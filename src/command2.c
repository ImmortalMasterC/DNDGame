/*
 * COMMAND2.C:
 *
 *	Addition user command routines.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"

/**********************************************************************/
/*									get							      */
/**********************************************************************/
/* This function allows players to get things lying on the floor or   */
/* inside another object.					      					  */

int get(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room		*rom_ptr, *home_rom;
	creature    *crt_ptr;
	creature    *ply_ptr2;
	creature    *guard;
	object		*obj_ptr, *cnt_ptr;
	otag	    *op;
	ctag		*cp;
	int			fd, n, match = 0, guarded = 0;
	int			splitnum = 1, oddgold = 0;
	char		tempstr[2];

	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Get what?\n");
		return(0);
	}

	rom_ptr = ply_ptr->parent_rom;
	home_rom = Ply[fd].extr->home_rom;

	F_CLR(ply_ptr, PHIDDN);

	if(cmnd->num == 2) {
		op = rom_ptr->first_obj;
		while(op) {
			if(F_ISSET(op->obj, OGUARD))
				break;
			op = op->next_tag;
		}
		cp = rom_ptr->first_mon;
		while(cp) {
			if(F_ISSET(cp->crt, MGDOBJ))
				break;
			cp = cp->next_tag;
		}
		if(cp && op) {
			guarded = 1;
			guard = cp->crt;
  			if(EQUAL(op->obj, cmnd->str[1])) {
				print(fd, "%M stands between you and the %s.\n", 
					cp->crt, op->obj);
				broadcast_rom(fd, rom_ptr->rom_num, 
					"%M guards the %s and won't let %m take it.", 
						cp->crt, op->obj, ply_ptr);
				return(0);
			}
		}

		if(F_ISSET(ply_ptr, PBLIND)) {
			ANSI(fd, RED);
			print(fd, "You can't see to do that!\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			return(0);
		}

		cp = rom_ptr->first_mon;
		while(cp) {
			if(F_ISSET(cp->crt, MGUARD))
				break;
			cp = cp->next_tag;
		}

		if(cp && ply_ptr->class < CARETAKER) {
			if(strcmp(cp->crt->key[1], ply_ptr->password)) {
				print(fd, "%M won't let you take anything.\n", cp->crt);
				broadcast_rom(fd, rom_ptr->rom_num, 
					"%M won't let %m take anything.", cp->crt, ply_ptr);
				return(0);
			}
		}

		if(cp && ply_ptr->class >= CARETAKER && strcmp(cp->crt->key[1], ply_ptr->password)) 
			print(fd, "%M winks and stands aside for the Dungeonmaster.\n", cp->crt);

		if(cp)
			strcpy(cp->crt->key[1], "");

		if(!strcmp(cmnd->str[1], "all")) {
			if(!guarded) {
				get_all_rom(ply_ptr);
				return(0);
			}
			else {
				print(fd, "You can't.  The %s is guarded by the %s.\n", 
					op->obj, guard);
				return(0);
			}
		}


		crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, 
			cmnd->str[1], cmnd->val[1]);
		if(crt_ptr) {
			print(fd, 
				"%M smiles and says to you, \"Maybe later, you pervert...\"\n", 
				crt_ptr);
			return(0);
		}
		if(!crt_ptr) {
			tempstr[1] = cmnd->str[1][0];
			cmnd->str[1][0] = up(cmnd->str[1][0]);
			crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply, 
				cmnd->str[1], cmnd->val[1]);
		}		
		if(crt_ptr) {
			print(fd, "You want to get %m?  No way.\n", crt_ptr);
			print(crt_ptr->fd, "%M wants your body.\n", ply_ptr);
			return(0);
		}
		else
			cmnd->str[1][0] = tempstr[1];
			

		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj, cmnd->str[1], 
			cmnd->val[1]);


		if(!obj_ptr) {
			print(fd, "That isn't here.\n");
			return(0);
		}

		if(ply_ptr->class < CARETAKER) {
			if(F_ISSET(obj_ptr, OINVIS) && !F_ISSET(ply_ptr, PDINVI)) {
				print(fd, "You don't see that here.\n");
				return(0);
			}
		}

		if(F_ISSET(obj_ptr, ONOTAK) || F_ISSET(obj_ptr, OSCENE)) {
			print(fd, "You can't take that!\n");
			return(0);
		}

		if(max_weight(ply_ptr) < weight_obj(obj_ptr)) { 
			print(fd, "That object is too heavy for you.");
			print(fd, "  You can't carry it.\n");
			return(0);
		}

		if(weight_ply(ply_ptr) + weight_obj(obj_ptr) > max_weight(ply_ptr) ||
			(player_inv(ply_ptr) == 80)) {
				print(fd, "You can't carry anymore.  "
					"Maybe if you drop something first.\n");
			return(0);
		}

		if(obj_ptr->questnum && Q_ISSET(ply_ptr, obj_ptr->questnum - 1)) {
			print(fd, "You may not take that.\n",
				"You have already fulfilled that quest");
			return(0);
		}

		if(F_ISSET(ply_ptr, PSHRNK)) {
			ANSI(fd, YELLOW);
			print(fd, 
				"Your body returns to normal size so you can carry it.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			F_CLR(ply_ptr, PSHRNK);
		}

		if(F_ISSET(obj_ptr, OTEMPP)) {
			F_CLR(obj_ptr, OPERM2);
			F_CLR(obj_ptr, OTEMPP);
		}

		if(F_ISSET(obj_ptr, OPERMT)) {
			get_perm_obj(obj_ptr);
		}

		F_CLR(obj_ptr, OHIDDN);
		del_obj_rom(obj_ptr, rom_ptr);
		print(fd, "You get %1i.\n", obj_ptr);
		if(obj_ptr->questnum) {
			print(fd, "Quest fulfilled!  Don't drop it.\n");
			print(fd, "You won't be able to pick it up again.\n");			
			savegame(ply_ptr, 0);
			Q_SET(ply_ptr, obj_ptr->questnum - 1);
			ply_ptr->experience += quest_exp[obj_ptr->questnum - 1];
			print(fd, "%ld experience granted.\n",
				quest_exp[obj_ptr->questnum - 1]);
			add_prof(ply_ptr, quest_exp[obj_ptr->questnum - 1]);
		}
		broadcast_rom(fd, rom_ptr->rom_num, "%M gets %1i.", ply_ptr, obj_ptr);

		if(obj_ptr->type == MONEY) {
			cp = ply_ptr->first_fol;
			if(cp && F_ISSET(ply_ptr, PSPLIT) && obj_ptr->value > 1) {
				while(cp) {
					splitnum++;
					cp = cp->next_tag;
				}
			}
			cp = ply_ptr->first_fol;
			if(cp && F_ISSET(ply_ptr, PSPLIT) && obj_ptr->value > 1) {
				oddgold = obj_ptr->value % splitnum;
				ply_ptr->gold += oddgold;
				ply_ptr->gold += (obj_ptr->value)/splitnum;
				while(cp) {
					cp->crt->gold += obj_ptr->value/splitnum;
					cp = cp->next_tag;
				}
				print(fd, "You split the gold %d ways.\n", splitnum);
				broadcast_rom(fd, rom_ptr->rom_num, 
					"%M splits %1i %d ways with %s group.", 
						ply_ptr, obj_ptr, splitnum, 
						F_ISSET(ply_ptr, PMALES) ? "his" : "her");
				free_obj(obj_ptr);
			}
			else {
				ply_ptr->gold += obj_ptr->value;
				free_obj(obj_ptr);
				print(fd, "You now have %-ld gold pieces.\n", ply_ptr->gold);
			}
		}
		else {
			add_obj_crt(obj_ptr, ply_ptr);
			if(obj_ptr->special == SP_HORNS)
				print(fd, "%s\n", obj_ptr->use_output);
		}
		compute_agility(ply_ptr);
		return(0);
	}
	else {
		cnt_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
			cmnd->str[2], cmnd->val[2]);

		if(!cnt_ptr)
			cnt_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
				cmnd->str[2], cmnd->val[2]);

		if(!cnt_ptr || !cmnd->val[2]) {
			for(n = 0; n < MAXWEAR; n++) {
				if(!ply_ptr->ready[n]) continue;
				if(EQUAL(ply_ptr->ready[n], cmnd->str[2]))
					match++;
				else continue;
				if(match == cmnd->val[2] || !cmnd->val[2]) {
					cnt_ptr = ply_ptr->ready[n];
					break;
				}
			}
		}

		if(!cnt_ptr) {
			print(fd, "That's not here.\n");
			return(0);
		}

		if(!F_ISSET(cnt_ptr, OCONTN)) {
			print(fd, "That isn't a container.\n");
			return(0);
		}

		if(cnt_ptr->type == CONTAINER && F_ISSET(cnt_ptr, OFASTD)) {
			print(fd, "That container is fastened shut.\n");
			return(0);
		}


		if(!strcmp(cmnd->str[1], "all")) {
			get_all_obj(ply_ptr, cnt_ptr);
			return(0);
		}

		obj_ptr = find_obj(ply_ptr, cnt_ptr->first_obj,
			cmnd->str[1], cmnd->val[1]);

		if(!obj_ptr) {
			print(fd, "That isn't in there.\n");
			return(0);
		}

		if(weight_ply(ply_ptr) + weight_obj(obj_ptr) > max_weight(ply_ptr) && 
			cnt_ptr->parent_rom || (player_inv(ply_ptr) == 80)) {
				print(fd, "You can't carry anymore.\n");
				return(0);
		}

		if(F_ISSET(obj_ptr, OPERMT)) {
			get_perm_obj(obj_ptr);
		}

		cnt_ptr->shotscur--;
		del_obj_obj(obj_ptr, cnt_ptr);
		print(fd, "You get %1i from %1i.\n", obj_ptr, cnt_ptr);
		broadcast_rom(fd, rom_ptr->rom_num, "%M gets %1i from %1i.",
			ply_ptr, obj_ptr, cnt_ptr);

		if(obj_ptr->type == MONEY) {
			ply_ptr->gold += obj_ptr->value;
			free_obj(obj_ptr);
			print(fd, "You now have %-ld gold pieces.\n", ply_ptr->gold);
			compute_agility(ply_ptr);
		}
		else
			add_obj_crt(obj_ptr, ply_ptr);
	
		if(cnt_ptr->special == SP_ENCHT) {
			ANSI(fd, YELLOW);
			print(fd, "\n\n\n\n");
			print(fd, "The magic in the room overwhelms you, and you are ");
			print(fd, "hurled through a void!\n\n");
			ANSI(fd, WHITE);
			broadcast_rom(fd, rom_ptr->rom_num, 
				"%M is overwhelmed by the magic and disappears.", ply_ptr);
			del_ply_rom(ply_ptr, rom_ptr);
			add_ply_rom(ply_ptr, home_rom);
			compute_agility(ply_ptr);
			return(0);
		}

	}

	return(0);
}

/*************************************************************************/
/*								get_all_rom						      	 */
/*************************************************************************/
/* This function will cause the player pointed to by the first parameter */
/* to get everything he is able to see in the room.			 			 */

void get_all_rom(ply_ptr)
creature	*ply_ptr;

{
	room	*rom_ptr;
	object	*obj_ptr, *last_obj;
	otag	*op;
	ctag	*cp;
	char	str[2048];
	int 	fd, n = 1, found = 0, heavy = 0, dogoldmsg = 0;
	int		splitnum = 1, totalgold = 0, oddgold = 0;

	last_obj = 0; str[0] = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(F_ISSET(ply_ptr, PSHRNK)) {
		ANSI(fd, YELLOW);
		print(fd, 
			"Your body returns to normal size so you can carry everything.\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		F_CLR(ply_ptr, PSHRNK);
	}

	op = rom_ptr->first_obj;

    if(op){
	while(op) {
		if(!F_ISSET(op->obj, OSCENE) && 
		    !F_ISSET(op->obj, ONOTAK) && 
			!F_ISSET(op->obj, OHIDDN) && 
			(F_ISSET(ply_ptr, PDINVI) ? 1:!F_ISSET(op->obj, OINVIS))) {
				found++;
				obj_ptr = op->obj;
				op = op->next_tag;
				if(player_inv(ply_ptr) == 80) {
					continue;
				}
				if(weight_ply(ply_ptr) + weight_obj(obj_ptr) >
					max_weight(ply_ptr)) {
						heavy++;
						continue;
				}
				if(obj_ptr->questnum && 
					Q_ISSET(ply_ptr, obj_ptr->questnum - 1)) {
						heavy++;
						continue;
				}

				if(F_ISSET(obj_ptr, OTEMPP)) {
					F_CLR(obj_ptr, OPERM2);
					F_CLR(obj_ptr, OTEMPP);
				}

				if(F_ISSET(obj_ptr, OPERMT))
					get_perm_obj(obj_ptr);

				F_CLR(obj_ptr, OHIDDN);
				F_CLR(obj_ptr, OINVIS);

				if(obj_ptr->questnum) {
					print(fd,"Quest fulfilled!  Don't drop it.\n");
					print(fd,"You won't be able to pick it up again.\n");		
					Q_SET(ply_ptr, obj_ptr->questnum-1);
					ply_ptr->experience += quest_exp[obj_ptr->questnum-1];
					print(fd, "%ld experience granted.\n",
						quest_exp[obj_ptr->questnum-1]);
					add_prof(ply_ptr,quest_exp[obj_ptr->questnum-1]);
				}
				del_obj_rom(obj_ptr, rom_ptr);
				if(last_obj && !strcmp(last_obj->name, obj_ptr->name) &&
					last_obj->adjustment == obj_ptr->adjustment)
						n++;
				else if(last_obj) {
					strcat(str, obj_str(last_obj, n, 0));
					strcat(str, ", ");
					n = 1;
				}
				if(obj_ptr->type == MONEY) {
					strcat(str, obj_str(obj_ptr, 1, 0));
					strcat(str, ", ");
					if(F_ISSET(ply_ptr, PSPLIT))
						totalgold += obj_ptr->value;
					else
						ply_ptr->gold += obj_ptr->value;
					free_obj(obj_ptr);
					last_obj = 0;
					dogoldmsg = 1;
				}
				else {
					add_obj_crt(obj_ptr, ply_ptr);
					last_obj = obj_ptr;
				}
		}
		else
			op = op->next_tag;
	}
	}

	if(found && last_obj)
		strcat(str, obj_str(last_obj, n, 0));
	else if(!found) {
		print(fd, "There's nothing here that you can take.\n");
		return;
	}

	if(dogoldmsg && !last_obj)
		str[strlen(str) - 2] = 0;

	if(heavy) {
		print(fd, "You are not able to carry everything.\n");
		if(heavy == found) {
			compute_agility(ply_ptr);
			return;
		}
	}

	if(!strlen(str)) {
		compute_agility(ply_ptr);
		return;
	}

	print(fd, "You get %s.\n", str);
	broadcast_rom(fd, rom_ptr->rom_num, "%M gets %s.", ply_ptr, str);
	if(dogoldmsg) {
		if(F_ISSET(ply_ptr, PSPLIT) && totalgold > 1) {
			cp = ply_ptr->first_fol;
			while(cp) {
				splitnum++;
				cp = cp->next_tag;
			}
			oddgold = totalgold % splitnum;
			ply_ptr->gold += oddgold;
			ply_ptr->gold += totalgold/splitnum;
			cp = ply_ptr->first_fol;
			if(cp && F_ISSET(ply_ptr, PSPLIT)) {
				while(cp) {
					cp->crt->gold += totalgold/splitnum;
					cp = cp->next_tag;
				}
				print(fd, "You split the gold %d ways.\n", splitnum);
				print(fd, "You now have %-ld gold pieces.\n", ply_ptr->gold);
				broadcast_rom(fd, rom_ptr->rom_num, "%M gets %s.", 
					ply_ptr, str);
				broadcast_rom(fd, rom_ptr->rom_num, 
					"%M splits the gold %d ways with %s group.", ply_ptr, 
						splitnum, F_ISSET(ply_ptr, PMALES) ? "his" : "her"); 
			}
		}
		else {
			if(totalgold == 1)
				ply_ptr->gold++;
			print(fd, "You now have %-ld gold pieces.\n", ply_ptr->gold);
			broadcast_rom(fd, rom_ptr->rom_num, "%M gets %s.", ply_ptr, str);
		}
	}
	compute_agility(ply_ptr);
	return;
}

/**********************************************************************/
/*								get_all_obj						      */
/**********************************************************************/
/* This function allows a player to get the entire contents from a 	  */
/* container object.  The player is pointed to by the first parameter */
/* and the container by the second.						  			  */

void get_all_obj(ply_ptr, cnt_ptr)
creature	*ply_ptr;
object		*cnt_ptr;

{
	room	*rom_ptr, *home_rom;
	object	*obj_ptr, *last_obj;
	otag	*op;
	char	str[2048];
	int		fd, n = 1, found = 0, heavy = 0;

	last_obj = 0; str[0] = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	home_rom = Ply[fd].extr->home_rom;

	op = cnt_ptr->first_obj;

	while(op) {
		if(!F_ISSET(op->obj, OSCENE) && 
		   !F_ISSET(op->obj, ONOTAK) && 
		   !F_ISSET(op->obj, OHIDDN) && 
		   ((F_ISSET(ply_ptr, PDINVI) ||
	       ply_ptr->class >= CARETAKER) ? 1:!F_ISSET(op->obj, OINVIS))) {
				found++;
				obj_ptr = op->obj;
				op = op->next_tag;

				if(weight_ply(ply_ptr) + weight_obj(obj_ptr) >
					max_weight(ply_ptr)) {
						heavy++;
						continue;
				}

				if(F_ISSET(obj_ptr, OTEMPP)) {
					F_CLR(obj_ptr, OPERM2);
					F_CLR(obj_ptr, OTEMPP);
				}

				if(F_ISSET(obj_ptr, OPERMT)) {
					get_perm_obj(obj_ptr);
				}

				cnt_ptr->shotscur--;
				del_obj_obj(obj_ptr, cnt_ptr);

				if(last_obj && !strcmp(last_obj->name, obj_ptr->name) &&
					last_obj->adjustment == obj_ptr->adjustment)
						n++;

				else if(last_obj) {
					strcat(str, obj_str(last_obj, n, 0));
					strcat(str, ", ");
					n = 1;
				}

				if(obj_ptr->type == MONEY) {
					ply_ptr->gold += obj_ptr->value;
					free_obj(obj_ptr);
					last_obj = 0;
					print(fd, "You now have %-ld gold pieces.\n", 
						ply_ptr->gold);
				}
				else {
					add_obj_crt(obj_ptr, ply_ptr);
					last_obj = obj_ptr;
				}
		}
		else
			op = op->next_tag;
	}

	if(found && last_obj)
		strcat(str, obj_str(last_obj, n, 0));

	else if(!found) {
		print(fd, "There's nothing in it.\n");
		return;
	}

	if(heavy) {
		print(fd, "You weren't able to carry everything.\n");
		compute_agility(ply_ptr);
		if(heavy == found) return;
	}

	if(!strlen(str)) return;

	broadcast_rom(fd, rom_ptr->rom_num, "%M gets %s from %1i.",
		ply_ptr, str, cnt_ptr);
	print(fd, "You get %s from %1i.\n", str, cnt_ptr);

	if(cnt_ptr->special == SP_ENCHT) {
		ANSI(fd, YELLOW);
		print(fd, "\n\n\n\n");
		print(fd, "The magic in the room overwhelms you, and you are ");
		print(fd, "hurled through a void!\n\n");
		ANSI(fd, WHITE);
		broadcast_rom(fd, rom_ptr->rom_num, 
			"%M is overwhelmed by the magic and disappears.", ply_ptr);
		del_ply_rom(ply_ptr, rom_ptr);
		add_ply_rom(ply_ptr, home_rom);
		compute_agility(ply_ptr);
		return;
	}
}

/**********************************************************************/
/*								inventory						      */
/**********************************************************************/
/* This function outputs the contents of a player's inventory.        */

int inventory(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	otag		*op;
	char		str[2048];
	int			m, n, fd, flags = 0;

	fd = ply_ptr->fd;

	if(F_ISSET(ply_ptr, PBLIND)) {
		ANSI(fd, RED);
		print(fd, "You're blind as a bat...how can you do that?\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		return(0);
	}		

	if(F_ISSET(ply_ptr, PDINVI) || (ply_ptr->class >= CARETAKER))
		flags |= INV;

	if(F_ISSET(ply_ptr, PDMAGI) || (ply_ptr->class >= CARETAKER))
		flags |= MAG;

	op = ply_ptr->first_obj; n = 0; str[0] = 0;
	strcat(str, "You have: ");

	if(!op) {
		strcat(str, "nothing.");
		print(fd, "%s\n", str);
		return(0);
	}

	while(op) {
		if((F_ISSET(ply_ptr, PDINVI) || 
			ply_ptr->class >= CARETAKER) ?  1:!F_ISSET(op->obj, OINVIS)) {
			m = 1;

			while(op->next_tag) {

				if(!strcmp(op->next_tag->obj->name, op->obj->name) &&
				   op->next_tag->obj->adjustment == op->obj->adjustment &&
				   (F_ISSET(ply_ptr, PDINVI) ? 
				    1:!F_ISSET(op->next_tag->obj, OINVIS))) {
						m++;
						op = op->next_tag;
				}
				else break;
			}

			strcat(str, obj_str(op->obj, m, flags));
			strcat(str, ", ");
			n++;
		}

		op = op->next_tag;
	}

	if(n) {
		str[strlen(str) - 2] = 0;
		print(fd, "%s.\n", str);
	}

	return(0);
}

/**********************************************************************/
/*								drop						   	      */
/**********************************************************************/
/* This function allows the player pointed to by the first parameter  */
/* to drop an object in the room at which he is located.              */

int drop(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room		*rom_ptr;
	object		*obj_ptr, *cnt_ptr;
	int			fd, n, match = 0, gold = 0;

	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Drop what?\n");
		return(0);
	}

	rom_ptr = ply_ptr->parent_rom;
	F_CLR(ply_ptr, PHIDDN);

	if(cmnd->num == 2) {
		if(!strcmp(cmnd->str[1], "all")) {
			drop_all_rom(ply_ptr);
			compute_agility(ply_ptr);
			return(0);
		}



	/* drop gold */
	if(cmnd->str[1][0] == '$' && F_ISSET(rom_ptr, RDUMPR)) {
		print(fd, 
			"Now wouldn't that be a nice way to make money?  Forget it.\n");
		return(0);		
	} 

	if(cmnd->str[1][0] == '$' && !F_ISSET(rom_ptr, RDUMPR)) {
		gold = atoi(cmnd->str[1] + 1);

		if(gold > 0 && gold <= ply_ptr->gold) {
			load_obj(0, &obj_ptr);

			if(gold == 1)
				sprintf(obj_ptr->name, "%-ld gold coin", gold);
			else 
				sprintf(obj_ptr->name, "%-ld gold coins", gold);
			obj_ptr->value = gold;
			ply_ptr->gold -= gold;
			compute_agility(ply_ptr);
		} 

		else { 
			if(gold > 0 && gold > ply_ptr->gold) { 
				print(fd, "You don't have that much!\n");
				return(0);
			}

			if(gold < 0) {	
				ANSI(fd, RED);
				print(fd, "SHAME!  You just used the magic negative number!\n");
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
				ply_ptr->gold = ply_ptr->gold/2;
				if(ply_ptr->gold < 100)
					ply_ptr->gold = 0;
				print(fd, "You have %-ld gold coins left.\n", ply_ptr->gold);
				savegame(ply_ptr, 0);
				compute_agility(ply_ptr);
			}

			return(0);
		}
	}
	else 
		obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, 
			cmnd->str[1], cmnd->val[1]);
		
	if(!obj_ptr) {
	    print(fd, "You don't have that.\n");
		return(0);
	}

	if(cmnd->num < 3) {
		if(F_ISSET(rom_ptr, RDUMPR) && 
		   F_ISSET(obj_ptr, OCONTN) &&
		   obj_ptr->shotscur > 0) {
				print(fd, "The %s is not empty.\n", obj_ptr->name);
				return(0);
		}
	}

    if(F_ISSET(obj_ptr, OCONTN) && obj_ptr->shotscur > 0) {
		del_obj_crt(obj_ptr, ply_ptr);
        print(fd, "Clunk.  There's something in the %s you dropped.\n", 
			obj_ptr->name);
		if(!F_ISSET(rom_ptr, RDUMPR) && !F_ISSET(rom_ptr, RSLIME))
			add_obj_rom(obj_ptr, rom_ptr);
		return(0);
	}

	if(!gold)
		del_obj_crt(obj_ptr, ply_ptr);
	print(fd, "You drop %1i.\n", obj_ptr);

	if(F_ISSET(ply_ptr, PSHRNK))
	    print(fd, "It returns to normal size as you release it.\n");
	broadcast_rom(fd, rom_ptr->rom_num, "%M dropped %1i.", ply_ptr, obj_ptr);

	if(!F_ISSET(rom_ptr, RDUMPR) && !F_ISSET(rom_ptr, RSLIME))
		add_obj_rom(obj_ptr, rom_ptr);

	else if(F_ISSET(rom_ptr, RDUMPR) && !F_ISSET(rom_ptr, RSLIME)) {
		free_obj(obj_ptr);
		ply_ptr->gold += 5;
		print(fd, 
			"Thanks for recycling.\nYou have %-ld gold.\n", ply_ptr->gold);
	}

	else if(F_ISSET(rom_ptr, RSLIME)) {
		ANSI(fd, GREEN);
		print(fd, "The slime beneath your feet dissolves %1i.\n", obj_ptr);
		free_obj(obj_ptr);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
	}

	compute_agility(ply_ptr);
	savegame(ply_ptr, 0);
	return(0);
	}

	else {
		cnt_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
			cmnd->str[2], cmnd->val[2]);

		if(!cnt_ptr)
			cnt_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
				cmnd->str[2], cmnd->val[2]);

		if(!cnt_ptr || !cmnd->val[2]) {
			for(n = 0; n < MAXWEAR; n++) {
				if(!ply_ptr->ready[n]) continue;
				if(EQUAL(ply_ptr->ready[n], cmnd->str[2]))
					match++;
				else continue;
				if(match == cmnd->val[2] || !cmnd->val[2]) {
					cnt_ptr = ply_ptr->ready[n];
					break;
				}
			}
		}
		
		if(!cnt_ptr) {
			print(fd, "You don't see that here.\n");
			return(0);
		}

		if(!F_ISSET(cnt_ptr, OCONTN)) {
			print(fd, "That isn't a container.\n");
			return(0);
		}

		if(ply_ptr->class < CARETAKER) {
			if(cnt_ptr->special == SP_ENCHT && 
				ply_ptr->class != MAGE &&
				ply_ptr->class != ALCHEMIST) {
					print(fd, "Only a mage or an alchemist has the knowledge "
						"to use that container.\n");
				return(0);
			}
		}

		if(!strcmp(cmnd->str[1], "all")) {
			drop_all_obj(ply_ptr, cnt_ptr);
			compute_agility(ply_ptr);
			return(0);
		}

		obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		   cmnd->str[1], cmnd->val[1]);

		if(!obj_ptr) {
			print(fd, "You don't have that.\n");
			return(0);
		}

		if(obj_ptr == cnt_ptr) {
			print(fd, "You can't put something in itself!\n");
			return(0);
		}

		if(cnt_ptr->shotscur >= cnt_ptr->shotsmax) {
			print(fd, "%I can't hold anymore.\n", cnt_ptr);
			return(0);
		}

		if(F_ISSET(obj_ptr, OCONTN)) {
			print(fd, "You can't put containers into containers.\n");
			return(0);
		}

		if(F_ISSET(cnt_ptr, OFASTD)) {
			print(fd, "You can't.  That container is fastened close.\n");
			return(0);
		}


    	if(F_ISSET(cnt_ptr, OCNDES)) { 
			if(!F_ISSET(obj_ptr, OSOMEA))
				print(fd, "%1i disappears forever in %1i!\n", obj_ptr, cnt_ptr);
			else
				print(fd, "%1i disappear forever in %1i!\n", obj_ptr, cnt_ptr);
			broadcast_rom(fd, rom_ptr->rom_num, "%M put %1i in %1i.",
				ply_ptr, obj_ptr, cnt_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
			free(obj_ptr);
			compute_agility(ply_ptr);
			return(0);
		}

		del_obj_crt(obj_ptr, ply_ptr);
		add_obj_obj(obj_ptr, cnt_ptr);
		cnt_ptr->shotscur++;
		print(fd, "You put %1i in %1i.\n", obj_ptr, cnt_ptr);
		broadcast_rom(fd, rom_ptr->rom_num, "%M put %1i in %1i.",
			ply_ptr, obj_ptr, cnt_ptr);

		if(cnt_ptr->special == SP_ENCHT && ply_ptr->level < 7) {
			print(fd, "A foul smelling cloud of acid dissolves the %s.\n", 
				obj_ptr->name);
			print(fd, "You are not high enough in level to do this!\n");
			del_obj_obj(obj_ptr, cnt_ptr);
			cnt_ptr->shotscur--;
			free(obj_ptr);
			broadcast_rom(fd, rom_ptr->rom_num,
				"%1i dissolves everything.", cnt_ptr);
			return(0);
		}

		if(cnt_ptr->special == SP_ENCHT && ply_ptr->level >= 7) {

			if(F_ISSET(obj_ptr, OENCHA) || F_ISSET(obj_ptr, OTMPEN)) {
				print(fd, "It was already enchanted and begins to dissolve.\n");
				print(fd, "Now its magical energy is added to the vat's.\n");
				del_obj_obj(obj_ptr, cnt_ptr);
				cnt_ptr->shotscur--;
				free(obj_ptr);
				broadcast_rom(fd, rom_ptr->rom_num, 
					"A white cloud fills the room!");
				return(0);
			}

			if(obj_ptr->type > 5) { 
				print(fd, "The %s cannot be enchanted and dissolves.\n",
					obj_ptr->name);
				print(fd, "Its energy has been added to the vat's magicke.\n");
				del_obj_obj(obj_ptr, cnt_ptr);
				cnt_ptr->shotscur--;
				free(obj_ptr);
				broadcast_rom(fd, rom_ptr->rom_num, 
					"A black cloud forms in the room!");
				return(0);
			}

			else {
				print(fd, "The silvery liquid suddenly boils and seethes!\n");
				print(fd, "Just as quickly, it returns to a placid pool.\n");
				F_SET(obj_ptr, OENCHA);
				if(obj_ptr->type < 5)
					obj_ptr->adjustment += 2;
				if(obj_ptr->type == 5) 
					obj_ptr->armor += 3;
				broadcast_rom(fd, rom_ptr->rom_num, 
					"The liquid in %1i boils as the magic takes place.",
						cnt_ptr);
			}
		}
		return(0);
	}
}

/**********************************************************************/
/*								drop_all_rom					      */
/**********************************************************************/
/* This function is called when a player wishes to drop his entire    */
/* inventory into the the room.					      				  */

void drop_all_rom(ply_ptr)
creature	*ply_ptr;

{
	object	*obj_ptr;
	room	*rom_ptr;
	otag	*op;
	char	str[2048], str2[2048];
	int		fd, found;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	found = list_obj(str, ply_ptr, ply_ptr->first_obj);

	if(!found) {
		print(fd, "You don't have anything.\n");
		return;
	}

	op = ply_ptr->first_obj;
	while(op) {

		if(F_ISSET(ply_ptr, PDINVI) ? 1:!F_ISSET(op->obj, OINVIS)) {
			obj_ptr = op->obj;
			op = op->next_tag;
			del_obj_crt(obj_ptr, ply_ptr);

			if(!F_ISSET(rom_ptr, RDUMPR) && !F_ISSET(rom_ptr, RSLIME))
				add_obj_rom(obj_ptr, rom_ptr);

			else if(F_ISSET(rom_ptr, RDUMPR) && !F_ISSET(rom_ptr, RSLIME)) {
				free_obj(obj_ptr);
				ply_ptr->gold+=5;
			}

			else if(F_ISSET(rom_ptr, RSLIME))
				free_obj(obj_ptr);
		}

		else
			op = op->next_tag;
	}

	broadcast_rom(fd, rom_ptr->rom_num, "%M dropped %s.", ply_ptr, str);
	print(fd, "You drop %s.\n", str);

	if(F_ISSET(ply_ptr, PSHRNK))
		print(fd, "Everything you drop returns to normal size.\n");

	if(F_ISSET(rom_ptr, RDUMPR) && !F_ISSET(rom_ptr, RSLIME))
		print(fd, "Thanks for recycling.\nYou have %-ld gold.\n",
			ply_ptr->gold);

	savegame(ply_ptr, 0);

	if(F_ISSET(rom_ptr, RSLIME)) {
		broadcast_rom(fd, rom_ptr->rom_num, "Slime dissolves %s.", str);
		ANSI(fd, GREEN);
		print(fd, "Slime at your feet dissolves %s.\n", str);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
	}			
	compute_agility(ply_ptr);
}

/*************************************************************************/
/*							drop_all_obj						         */
/*************************************************************************/
/* This function drops all the items in a player's inventory into a	 	 */
/* container object, if possible.  The player is pointed to by the first */
/* parameter, and the container by the second.				 			 */

void drop_all_obj(ply_ptr, cnt_ptr)
creature	*ply_ptr;
object		*cnt_ptr;

{
	object	*obj_ptr, *last_obj;
	room	*rom_ptr;
	otag	*op;
	char	str[2048];
	int		fd, n = 1, found = 0, full = 0;

	fd = ply_ptr->fd;

	last_obj = 0; str[0] = 0;
	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->class < CARETAKER) {
		if(cnt_ptr->special == SP_ENCHT && ply_ptr->class != MAGE) {
			print(fd, "Only a mage has the knowledge to use that container.\n");
			return;
		}
	}

	op = ply_ptr->first_obj;

	while(op) {

		if((F_ISSET(ply_ptr, PDINVI) ? 1:!F_ISSET(op->obj, OINVIS)) &&
		   op->obj != cnt_ptr) {
			found++;
			obj_ptr = op->obj;
			op = op->next_tag;

			if(F_ISSET(obj_ptr, OCONTN)) {
				full++;
				continue;
			}

			if(F_ISSET(cnt_ptr, OCNDES)) {
				full++;
				del_obj_crt(obj_ptr, ply_ptr);
				free(obj_ptr);
				continue;
			}

			if(cnt_ptr->shotscur >= cnt_ptr->shotsmax) {
				full++;
				continue;
			}
			cnt_ptr->shotscur++;
			del_obj_crt(obj_ptr, ply_ptr);
			add_obj_obj(obj_ptr, cnt_ptr);

			if(last_obj && !strcmp(last_obj->name, obj_ptr->name) &&
				last_obj->adjustment == obj_ptr->adjustment)
					n++;

			else if(last_obj) {
				strcat(str, obj_str(last_obj, n, 0));
				strcat(str, ", ");
				n = 1;
			}
			last_obj = obj_ptr;
		}

		else
			op = op->next_tag;
	}

	if(found && last_obj)
		strcat(str, obj_str(last_obj, n, 0));

	else {
		print(fd, "You don't have anything that will go into it.\n");
		return;
	}

	if(full)
		print(fd, "%I couldn't hold everything.\n", cnt_ptr);

	if(full == found) 
		return;

	broadcast_rom(fd, rom_ptr->rom_num, "%M put %s into %1i.", ply_ptr,
		str, cnt_ptr);
	print(fd, "You put %s into %1i.\n", str, cnt_ptr);
	savegame(ply_ptr, 0);
	return;


/*============== Container is able to enchant armor or weapons ============*/

	if(cnt_ptr->special == SP_ENCHT && ply_ptr->level < 7) {
		print(fd, "A foul smelling cloud of acid dissolves the %s.\n", 
			last_obj->name);
		print(fd, "You don't have enough experience to use this!\n");
		del_obj_obj(last_obj, cnt_ptr);
		cnt_ptr->shotscur--;
		free(last_obj);
		compute_agility(ply_ptr);
		return;
	}
		
	if(cnt_ptr->special == SP_ENCHT && ply_ptr->level >= 7) {

		if(F_ISSET(last_obj, OENCHA) || F_ISSET(last_obj, OTMPEN)) {
			print(fd, "It was already enchanted and begins to dissolve.\n");
			print(fd, "Now its magical energy is added to %1i.\n", cnt_ptr);
			del_obj_obj(last_obj, cnt_ptr);
			cnt_ptr->shotscur--;
			free(last_obj);
			return;
		}

		else {
			print(fd, "The silvery liquid suddenly boils and churns!\n");
			F_SET(last_obj, OENCHA);
			if(last_obj->type < 5)
				last_obj->adjustment += 2;
			if(last_obj->type == 5) 
				last_obj->armor += 3;
			return;
		}
	}

}

/**********************************************************************/
/*								discard						   	      */
/**********************************************************************/
/* This function allows the player pointed to by the first parameter  */
/* to discard an object, freeing it from memory.                      */

int discard(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room		*rom_ptr;
	object		*obj_ptr;
	int			fd, n, match = 0;

	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Discard what?\n");
		return(0);
	}

	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num == 2) {
		if(!strcmp(cmnd->str[1], "all")) {
			print(fd, "You can only discard a single item at a time.\n");
			return(0);
		}

		if(cmnd->str[1][0] == '$') {
			print(fd, "You cannot discard gold.\n");
			return(0);
		} 
		else 
			obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, 
				cmnd->str[1], cmnd->val[1]);
		
		if(!obj_ptr) {
	    	print(fd, "You don't have that.\n");
			return(0);
		}

		if(F_ISSET(obj_ptr, OCONTN)) {
   			if(obj_ptr->shotscur > 0) {
				print(fd, "Container is not empty.\n");
				return(0);
			}
		}

		if(!F_ISSET(ply_ptr, PDMINV))
			broadcast_rom(fd, rom_ptr->rom_num, 
				"%M discards %1i in a wisp of smoke.", ply_ptr, obj_ptr);
		print(fd, "You discard %1i in a wisp of smoke.\n", obj_ptr);
		del_obj_crt(obj_ptr, ply_ptr);
		free_obj(obj_ptr);
		compute_agility(ply_ptr);
		savegame(ply_ptr, 0);
		return(0);
	}
}
