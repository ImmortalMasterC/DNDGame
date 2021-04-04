/*
 * COMMAND6.C:
 *
 *	Additional user routines.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"

/************************************************************************/
/*								move							        */
/************************************************************************/
/* This function takes the player using the socket descriptor specified */
/* in the first parameter, and attempts to move him in a specified		*/
/* direction if that exit is not prohibited to the player.				*/

int move(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room			*rom_ptr, *old_rom_ptr;
	creature		*crt_ptr;
	char			tempstr[10];
	char			*str;
	ctag			*cp, *temp;
	xtag			*xp;
	int				pick, fd, old_rom_num, fall;
	int				agility, dmg, n, i;
    int				found = 0, num = 0, count = 0;
	long			t;
	extra			*extr;
	extern char		ext[5][8];
	object			*obj_ptr;
	otag			*op;

	rom_ptr = ply_ptr->parent_rom;
	fd = ply_ptr->fd;
	str = cmnd->str[0];


	agility = compute_agility(ply_ptr);
	if((agility < 20 && mrand(1, 100) <= 2)) {
		count = player_inv(ply_ptr);	
		if(count && mrand(1, 100) < 10) {
			pick = mrand(1, count);
			op = ply_ptr->first_obj;
			while(op) {
				num++;
				if(num == pick) {
					obj_ptr = op->obj;
					break;
				}
				op = op->next_tag;
			}
			del_obj_crt(obj_ptr, ply_ptr);

    		print(fd, "You stumbled and dropped %1i.\n", obj_ptr);
    		if(F_ISSET(ply_ptr, PSHRNK))
        		print(fd, "It returns to normal size as you release it.\n");

			if(!strcmp(ply_ptr->name, DMNAME)) {
    			broadcast_rom(fd, rom_ptr->rom_num, 
					"%M stumbled and dropped %1i.", ply_ptr, obj_ptr);
			}

    		if(!F_ISSET(rom_ptr, RDUMPR) && !F_ISSET(rom_ptr, RSLIME))
        		add_obj_rom(obj_ptr, rom_ptr);
			savegame(ply_ptr, 0);
			return(0);
		}
		else {
			print(fd, "You stumbled.\n");
			if(!strcmp(ply_ptr->name, DMNAME)) {
    			broadcast_rom(fd, rom_ptr->rom_num, "%M stumbled.", ply_ptr);
			}
			return(0);
		}
	}


	if(weight_ply(ply_ptr) > max_weight(ply_ptr)) {
		print(fd, "You're holding more than you can carry, and cannot move.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PFROZE)) {
		print(fd, 
			"A freeze spell has been cast on you, and you cannot move.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		F_SET(ply_ptr, PNOLDS);
		ANSI(fd, RED);
		print(fd, "You're blind.\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
	}

	t = time(0);
	if(ply_ptr->lasttime[LT_MOVED].ltime == t) {
		if(++ply_ptr->lasttime[LT_MOVED].misc > 3) {
			please_wait(fd, 1);
			return(0);
		}
	}
	else if(ply_ptr->lasttime[LT_ATTCK].ltime + 3 > t) {
		please_wait(fd, 3 - t + ply_ptr->lasttime[LT_ATTCK].ltime);
		return(0);
	}
	else if(ply_ptr->lasttime[LT_SPELL].ltime + 3 > t) {
		please_wait(fd, 3 - t + ply_ptr->lasttime[LT_SPELL].ltime);
		return(0);
	}
	else {
		ply_ptr->lasttime[LT_MOVED].ltime = t;
		ply_ptr->lasttime[LT_MOVED].misc = 1;
	}

		
	if(!strcmp(str, "sw") || !strncmp(str, "southw", 6))
		strcpy(tempstr, "southwest");
	else if(!strcmp(str, "nw") || !strncmp(str, "northw", 6))
		strcpy(tempstr, "northwest");
	else if(!strcmp(str, "se") || !strncmp(str, "southe", 6))
		strcpy(tempstr, "southeast");
	else if(!strcmp(str, "ne") || !strncmp(str, "northe", 6))
		strcpy(tempstr, "northeast");
	else if(!strcmp(str, "dis") || !strncmp(str, "dismount", 6))
		strcpy(tempstr, "dismount");
	else if(!strcmp(str, "lef") || !strncmp(str, "left", 6))
		strcpy(tempstr, "left");
	else if(!strcmp(str, "fo") || !strncmp(str, "forw", 6))
		strcpy(tempstr, "forward");
	else if(!strcmp(str, "af") || !strncmp(str, "aft", 6))
		strcpy(tempstr, "aft");
	else if(!strcmp(str, "po") || !strncmp(str, "port", 6))
		strcpy(tempstr, "port");
	else if(!strcmp(str, "star") || !strncmp(str, "starb", 6))
		strcpy(tempstr, "starboard");
	else if(!strcmp(str, "exi") || !strncmp(str, "exit", 6))
		strcpy(tempstr, "out");
	else {
		switch(str[0]) {
			case 'n': strcpy(tempstr, "north");		break;
			case 's': strcpy(tempstr, "south");		break;
			case 'e': strcpy(tempstr, "east");  	break;
			case 'w': strcpy(tempstr, "west"); 		break;
			case 'u': strcpy(tempstr, "up");   		break;
			case 'd': strcpy(tempstr, "down");  	break;
			case 'o': strcpy(tempstr, "out");   	break;
			case 'm': strcpy(tempstr, "mount");   	break;
			case 'r': strcpy(tempstr, "right");   	break;
			case 'f': strcpy(tempstr, "forward");   break;
			case 'b': strcpy(tempstr, "back");   	break;
			case 'a': strcpy(tempstr, "aft");   	break;
			case 'p': strcpy(tempstr, "port");   	break;
		}
	}

	xp = rom_ptr->first_ext;
	while(xp) {
		if(!strcmp(xp->ext->name, tempstr) && !F_ISSET(xp->ext, XNOSEE)) {
			found = 1;
			break;
		}
		xp = xp->next_tag;
	}

	if(!found) {
		print(fd, "You can't go that way.\n");
		return(0);
	}

	if(F_ISSET(xp->ext, XNGOOD)) {
		if(!F_ISSET(xp->ext, XNGOOD + (ply_ptr->alignment > 100))) {
			ANSI(fd, RED);
			print(fd, "A ward of evil overlooks that portal.\n");
			ANSI(fd, YELLOW);
			print(fd, "Your goodness prevents your entry or exit.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			return(0);
		}
	}

	if(F_ISSET(xp->ext, XNOEVL)) {
		if(!F_ISSET(xp->ext, XNOEVL + (ply_ptr->alignment < -100))) {
			ANSI(fd, CYAN);
			print(fd, "A ward of goodness overlooks that portal.\n");
			ANSI(fd, YELLOW);
			print(fd, "Begone, foul and evil creature!\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			return(0);
		}
	}

	if((F_ISSET(xp->ext, XPLSEL) || F_ISSET(xp->ext, XRACES)) &&
		ply_ptr->class >= CARETAKER) 
			print(fd, " DM note:  Class or Race selective exit.\n");

	if(F_ISSET(xp->ext, XPLSEL)) {
		if(!F_ISSET(xp->ext, XPLSEL + ply_ptr->class) && 
			ply_ptr->class < CARETAKER) {
				print(fd, "Your class cannot go there.\n");
				return(0);
		}
	}

	if(F_ISSET(xp->ext, XRACES)) {
		if(!F_ISSET(xp->ext, XRACES + ply_ptr->race) && 
			ply_ptr->class < CARETAKER) {
				print(fd, "Your race cannot go there.\n");
				return(0);
		}
	}

	if(F_ISSET(xp->ext, XLOCKD)) {
		print(fd, "It's locked.\n");
		return(0);
	}
	else if(F_ISSET(xp->ext, XCLOSD)) {
		print(fd, "You have to open it first.\n");
		return(0);
	}

	if(F_ISSET(xp->ext, XFLYSP) && !F_ISSET(ply_ptr, PFLYSP)) {
		print(fd, "You must fly to get there.\n");
		return(0);
	}
 
	t = Time%24L;
	if(F_ISSET(xp->ext, XNGHTO) && (t > 6 && t < 20)) {
 		print(fd, "That exit can be used only at night.\n");
       	return(0);
   	}          

	if(F_ISSET(xp->ext, XDAYON) && (t < 6 || t > 20)) {
       	print(fd, "You cannot use that exit at night.\n");
       	return(0);
   	}          
 
    if(F_ISSET(xp->ext, XPGUAR)) {
        cp = rom_ptr->first_mon;
        while(cp) {
            if(F_ISSET(cp->crt, MPGUAR)) {
           		if(!F_ISSET(ply_ptr, PINVIS) && ply_ptr->class < CARETAKER) {
            	   	print(fd, "%M blocks your exit.\n", cp->crt);
            	   	return(0);
           		}
           		if(!F_ISSET(ply_ptr, PINVIS) && ply_ptr->class >= CARETAKER) {
					ANSI(fd, YELLOW);
            	   	print(fd, "DM Note:  %M guards this exit.\n", cp->crt);
					ANSI(fd, WHITE);
           		}
	   			if(F_ISSET(cp->crt, MDINVI) && ply_ptr->class < CARETAKER) {
            	   	print(fd, 
						"%M senses your invisibility and blocks your exit.\n", 
							cp->crt);
           			return(0);
        		}	    
	    	}
            cp = cp->next_tag;
        }
    }         

	if(F_ISSET(xp->ext, XMOUSE) && !F_ISSET(ply_ptr, PSHRNK)) {
		print(fd, "Only a mouse could enter that tiny place.\n");
		return(0); 
	}

	if(F_ISSET(xp->ext, XPLDGK)) 
	 if (!F_ISSET(ply_ptr, PPLDGK)) {
   		print(fd, "You may only go there after you have pledged your "
					"allegiance to a prince.\n");
        	return(0);
	}
	else if(BOOL(F_ISSET(xp->ext,XKNGDM)) !=  
		BOOL(F_ISSET(ply_ptr, PKNGDM))) {
        	print(fd, "You must be pledged to Prince %s to go there.\n", 
				F_ISSET(ply_ptr, PKNGDM) ? "Ragnar" : "Rutger");
        	return(0);
	}          

	if(F_ISSET(xp->ext, XFEMAL) && F_ISSET(ply_ptr, PMALES)) {
		print(fd, "Sorry, only females are allowed to go there.\n");
		return(0); 
	}

	if(F_ISSET(xp->ext, XMALES) && !F_ISSET(ply_ptr, PMALES)) {
		print(fd, "Sorry, only males are allowed to go there.\n");
		return(0); 
	}           

	if(F_ISSET(xp->ext, XSMALL) && !F_ISSET(ply_ptr, PSHRNK) &&
		ply_ptr->race != DWARF && ply_ptr->race != GNOME && 
			ply_ptr->race != HOBBIT) {
				print(fd, "Only a very small person can squeeze in there.\n");
				return(0); 
	}           

	if(F_ISSET(xp->ext, XLARGE) && (ply_ptr->race != OGRE &&
			ply_ptr->race != CENTAUR && ply_ptr->race != HALFGIANT ||
				F_ISSET(ply_ptr, PSHRNK))) {
					print(fd, "You are too small to go there.\n");
					return(0); 
	}           

	if(F_ISSET(xp->ext, XNOINV) && F_ISSET(ply_ptr, PINVIS)) {
		print(fd,
			"A ward drops a curtain of shadows falls across the %s "
				"and you cannot enter.\n", tempstr);
		return(0);
	}

	if(F_ISSET(xp->ext, XNOBIG) && (ply_ptr->race == OGRE ||
		ply_ptr->race == CENTAUR || ply_ptr->race == HALFGIANT)) {
			print(fd, "You are much too large to go there.\n");
			return(0); 
	}           

	if(F_ISSET(xp->ext, XNAKED) && weight_ply(ply_ptr)) {
		print(fd, "You cannot bring anything through that exit.\n");
		return(0);
	}

	if((F_ISSET(xp->ext, XCLIMB) || F_ISSET(xp->ext, XREPEL)) &&
		(!F_ISSET(ply_ptr, PLEVIT) && !F_ISSET(ply_ptr, PFLYSP))) {
			fall = (F_ISSET(xp->ext, XDCLIM) ? 50:0) + 50 - fall_ply(ply_ptr);
			if(mrand(1, 100) < fall) {
				dmg = mrand(5, (15 + fall/10));
				if(ply_ptr->hpcur <= dmg) {
					ANSI(fd, REVERSE);
					print(fd, "You fell to your death.\n");
					ANSI(fd, NORMAL);
					ANSI(fd, BOLD);
					ANSI(fd, WHITE);
					ply_ptr->hpcur = 0;
					broadcast_rom(fd, ply_ptr->rom_num, 
						"%M died from the fall.\n", ply_ptr);
					die(ply_ptr, ply_ptr);
					return(0);
				}
				ply_ptr->hpcur -= dmg;
				print(fd, "You fell and hurt yourself for %d damage.\n", dmg);
				broadcast_rom(fd, ply_ptr->rom_num, "%M fell down.", ply_ptr);
				if(F_ISSET(xp->ext, XCLIMB))
					return(0);
			}
	}

	F_CLR(ply_ptr, PHIDDN);

	cp = rom_ptr->first_mon;
	while(cp) {
		if(F_ISSET(cp->crt, MBLOCK) && is_enm_crt(ply_ptr->name, cp->crt)
			&& !F_ISSET(ply_ptr, PINVIS) && ply_ptr->class < CARETAKER) {
				ANSI(fd, YELLOW);
				print(fd, "%M blocks your exit.\n", cp->crt);
				ANSI(fd, WHITE);
				return(0);
		}
		if(F_ISSET(cp->crt, MBLOCK) && is_enm_crt(ply_ptr->name, cp->crt)
			&& ply_ptr->class >= CARETAKER) {
				ANSI(fd, YELLOW);
				print(fd, "DM Note:  %M blocks players from exit.\n", cp->crt);
				ANSI(fd, WHITE);
		}

		cp = cp->next_tag;
	}

	if(!F_ISSET(rom_ptr, RPTRAK))
		strcpy(rom_ptr->track, tempstr);

	old_rom_num = rom_ptr->rom_num;
	old_rom_ptr = rom_ptr;

	load_rom(xp->ext->room, &rom_ptr);
	if(rom_ptr == old_rom_ptr) {
		print(fd, "Off the map in that direction.\n");
		print(fd, "This is room number %d.\n", rom_ptr->rom_num);
		print(fd, 
			"Please note the room number and send mudmail to Styx, the DM.\n");
		return(0);
	}

	n = count_vis_ply(rom_ptr);

	if(rom_ptr->lolevel > ply_ptr->level) {
		print(fd, "You must be at least level %d to go that way.\n",
		      rom_ptr->lolevel);
		if(ply_ptr->class < CARETAKER) 
		return(0);
	}
	else if(ply_ptr->level > rom_ptr->hilevel && rom_ptr->hilevel) { 
		print(fd, "Only players under level %d may go that way.\n",
	    	rom_ptr->hilevel + 1);
		if(ply_ptr->class < CARETAKER) 
			return(0);
	}
	else if((F_ISSET(rom_ptr, RONEPL) && n > 0) || 
		(F_ISSET(rom_ptr, RTWOPL) && n > 1) || 
			(F_ISSET(rom_ptr, RTHREE) && n > 2)) {
				print(fd, "That place is full.\n");
				if(ply_ptr->class < CARETAKER) 
					return(0);
	}

	for(i = 0, found = 0; i < 5 && found == 0; i++) {
		if(strcmp(tempstr, ext[i]) == 0)
			found = 1;
	}

	if(!F_ISSET(ply_ptr, PDMINV) && 
		!F_ISSET(ply_ptr, PCLOAK) &&
		!F_ISSET(ply_ptr, PINVIS)) {
		if(found == 0) {
			broadcast_rom(fd, old_rom_ptr->rom_num, "%M goes to the %s.", 
				ply_ptr, tempstr);
		}
		else {
			broadcast_rom(fd, old_rom_ptr->rom_num, "%M goes %s.", 
			  	ply_ptr, tempstr);
		}
	}

	if(F_ISSET(ply_ptr, PALIAS) && ply_ptr->type == PLAYER) { 
        del_crt_rom(Ply[ply_ptr->fd].extr->alias_crt, ply_ptr->parent_rom);
		if(found == 0) {
            broadcast_rom(ply_ptr->fd, ply_ptr->rom_num, 
				"%M just went to the %s.", Ply[ply_ptr->fd].extr->alias_crt, 
					tempstr);
           	add_crt_rom(Ply[ply_ptr->fd].extr->alias_crt, rom_ptr, 1);
		}
		else if(found == 1) {
            broadcast_rom(ply_ptr->fd, ply_ptr->rom_num, "%M just went %s.", 
		 		Ply[ply_ptr->fd].extr->alias_crt, tempstr);
           	add_crt_rom(Ply[ply_ptr->fd].extr->alias_crt, rom_ptr, 1);
		}
    }

    if(ply_ptr->type == PLAYER) {
		del_ply_rom(ply_ptr, ply_ptr->parent_rom);
		add_ply_rom(ply_ptr, rom_ptr);
	}

	cp = ply_ptr->first_fol;
	while(cp) {
		if(cp->crt->rom_num == old_rom_num && cp->crt->type != MONSTER)
			move(cp->crt, cmnd);
		cp = cp->next_tag;
	}

	if(is_rom_loaded(old_rom_num)) {
		cp = old_rom_ptr->first_mon;
		while(cp) {
			if(cp->crt->special == CRT_PORTR && F_ISSET(cp->crt, MFOLLO)) {
				temp = cp->next_tag;
				if(!strcmp(cp->crt->key[1], ply_ptr->password)) {
					if(cp->crt->mpcur > 0) {
						crt_ptr = cp->crt;
						del_crt_rom(crt_ptr, old_rom_ptr);
						add_crt_rom(crt_ptr, rom_ptr, 1);
						add_active(crt_ptr);
						crt_ptr->mpcur--;
						if(crt_ptr->mpcur < 0)
							crt_ptr->mpcur = 0;
						broadcast_rom(fd, old_rom_num, "%M's %s followed %s.", 
							ply_ptr, crt_ptr, F_ISSET(ply_ptr, PMALES) ? "him":"her");
					}
					else if(cp->crt->mpcur == 0)
						print(fd, "Your porter refuses to follow without more pay.\n");
				}
				cp = temp;
				continue;
			}
			cp = cp->next_tag;
		}
	}

	if(is_rom_loaded(old_rom_num)) {
		cp = old_rom_ptr->first_mon;
		while(cp) {
			if((!F_ISSET(cp->crt, MFOLLO) || F_ISSET(cp->crt, MDMFOL)) &&
				cp->crt->type == MONSTER) {
					cp = cp->next_tag;
					continue;
			}
			if(!cp->crt->first_enm) {
				cp = cp->next_tag;
				continue;
			}
			if(strcmp(cp->crt->first_enm->enemy, ply_ptr->name)) {
				cp = cp->next_tag;
				continue;
			}
			if(mrand(1, 20) > 15 - ply_ptr->dexterity + cp->crt->dexterity) {
				cp = cp->next_tag;
				continue;
			}

			if(!F_ISSET(ply_ptr->parent_rom, RHEALR)) {
				temp = cp->next_tag;
				print(fd, "%M followed you.\n", cp->crt);
				broadcast_rom(fd, old_rom_num, "%M follows %m.", cp->crt, ply_ptr);
				crt_ptr = cp->crt;
				if(F_ISSET(crt_ptr, MPERMT))
					die_perm_crt(crt_ptr);
				del_crt_rom(crt_ptr, old_rom_ptr);
				add_crt_rom(crt_ptr, rom_ptr, 1);
				add_active(crt_ptr);
				F_CLR(crt_ptr, MPERMT);
				cp = temp;
			}
			else {
				print(fd, "%M could not follow you here.\n", cp->crt);
				cp = cp->next_tag;
			}
		}
	}

	check_traps(ply_ptr, rom_ptr);
	return(0);
}



/************************************************************************/
/*								go								        */
/************************************************************************/
/* This function allows a player to go through an exit, other than one  */
/* of the 6 cardinal directions.				       					*/

int go(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room		*rom_ptr, *old_rom_ptr;
	creature	*crt_ptr;
	ctag		*cp, *temp;
	exit_		*ext_ptr;
	long		i, t;
	int			fd, old_rom_num, fall, dmg, n, s, p, j, found;
	char		*str;
	extra		*extr;

	extern	char		ext[5][8];

	rom_ptr = ply_ptr->parent_rom;
	fd = ply_ptr->fd;

	if(F_ISSET(ply_ptr, PFROZE)) {
		print(fd, "A freeze spell binds you, and you cannot move.\n");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M tried to leave, but is frozen in place.\n", ply_ptr);
		return(0);
	}

	if(weight_ply(ply_ptr) > max_weight(ply_ptr)) {
        print(fd, "You're holding more than you can carry, and cannot move.\n");        
		return(0);
    }

	if(cmnd->num < 2) {
		print(fd, "Go where?\n");
		return(0);
	}

	str = cmnd->str[0];

	if(!strcmp(cmnd->str[1], "ex") || !strcmp(cmnd->str[1], "exit"))
		strcpy(cmnd->str[1], "out");

	ext_ptr = find_ext(ply_ptr, rom_ptr->first_ext, 
		cmnd->str[1], cmnd->val[1]);

	if(ext_ptr && !strcmp(str, "climb")) {
		if(strcmp(ext_ptr->name, "up") && 
			strcmp(ext_ptr->name, "down") &&
			strcmp(ext_ptr->name, "rope") &&
			strcmp(ext_ptr->name, "ladder") &&
			strcmp(ext_ptr->name, "staircase") &&
			strcmp(ext_ptr->name, "stairs")) {
				print(fd, "You can't climb that.\n");
				return(0);
		}
	}

	if(!ext_ptr) {
		print(fd, "I don't see that exit.\n");
		return(0);
	}

	if(F_ISSET(ext_ptr, XNOEVL)) {
		if(!F_ISSET(ext_ptr, XNOEVL + (ply_ptr->alignment < -100))) {
			ANSI(fd, CYAN);
			print(fd, "A ward of goodness protects that portal.\n");
			print(fd,  "Begone, foul and evil creature!  You cannot enter "
						"or leave through there.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			if(ply_ptr->class < CARETAKER || !F_ISSET(ply_ptr, PDMINV)) {
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M found a ward of goodness protecting the portal.", 
						ply_ptr);
				return(0);
			}
		}
	}

	if(F_ISSET(ext_ptr, XNGOOD)) {
		if(!F_ISSET(ext_ptr, XNGOOD + (ply_ptr->alignment > 100))) {
			ANSI(fd, YELLOW);
			print(fd, "An evil ward protects that portal.\n");
			print(fd, "Your goodness prevents your entry or exit.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			if(ply_ptr->class < CARETAKER || !F_ISSET(ply_ptr, PDMINV)) {
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M found an evil ward protecting the portal.", ply_ptr);
				return(0);
			}
		}
	}

	if(F_ISSET(ext_ptr, XNOINV) && F_ISSET(ply_ptr, PINVIS)) {
		print(fd,
			"A ward drops a curtain of shadows falls across the %s "
				"and you cannot enter.\n", ext_ptr->name);
		if(ply_ptr->class < CARETAKER || !F_ISSET(ply_ptr, PDMINV)) {
			broadcast_rom(fd, ply_ptr->rom_num, 
				"Someone attempted to enter the %s, but could not.", 
					ext_ptr->name);
			return(0);
		}
	}



	if(F_ISSET(ext_ptr, XLOCKD)) {
		print(fd, "It's locked.\n");
		if(ply_ptr->class < CARETAKER || !F_ISSET(ply_ptr, PDMINV)) {
			broadcast_rom(fd, 
				ply_ptr->rom_num, "%M tried a locked exit.\n", ply_ptr);
		}
		return(0);
	}
	else if(F_ISSET(ext_ptr, XCLOSD)) {
		print(fd, "You have to open it first.\n");
		return(0);
	}

	if(F_ISSET(ext_ptr, XFLYSP) && !F_ISSET(ply_ptr, PFLYSP)) {
		print(fd, "You must fly to get there.\n");
		return(0);
	}

	if(F_ISSET(ext_ptr, XPLSEL) && ply_ptr->class >= CARETAKER) {
		ANSI(fd, YELLOW);
		print(fd, "Class selective exit.\n");
		ANSI(fd, WHITE);
	}

	if(F_ISSET(ext_ptr, XPLSEL)) {
		if(!F_ISSET(ext_ptr, XPLSEL + ply_ptr->class) && 
			ply_ptr->class < CARETAKER) {
				print(fd, "Your class is not allowed to go there.\n");
				return(0);
		}
	}

	if(F_ISSET(ext_ptr, XRACES) && ply_ptr->class >= CARETAKER) {
		ANSI(fd, YELLOW);
		print(fd, "Race selective exit.\n");
		ANSI(fd, WHITE);
	}

	
	if(F_ISSET(ext_ptr, XRACES)) {
		if(!F_ISSET(ext_ptr, XRACES + ply_ptr->race) && 
			ply_ptr->class < CARETAKER) {
				print(fd, "Your race is not allowed to go there.\n");
				return(0);
		}
	}

	t = Time%24L;
	if(F_ISSET(ext_ptr, XNGHTO) && (t > 6 && t < 20)) {
		print(fd, "That exit is not open during the day.\n");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M tried an exit that is closed during the day.\n", ply_ptr);
       	return(0);
    }          

	if(F_ISSET(ext_ptr, XDAYON) && (t < 6 || t > 20)) {
        print(fd, "That exit is closed for the night.\n");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M tried an exit that is closed during the night.\n", ply_ptr);
        return(0);
    }          
 
    if(F_ISSET(ext_ptr, XPGUAR)) {
        cp = rom_ptr->first_mon;
        while(cp) {
            if(F_ISSET(cp->crt, MPGUAR)) {
                if(!F_ISSET(ply_ptr, PINVIS)) { 
                   	print(fd, "%M%s uses force to block you from going "
						"that way.\n", cp->crt, (F_ISSET(cp->crt, MHIDDN) ? 
							", hidden in the shadows," : ""));
					if(ply_ptr->class < CARETAKER)
                    	return(0);
            	}
            	else if(F_ISSET(cp->crt, MDINVI)) {
					ANSI(fd, MAGENTA);
					print(fd, "%M%s detects your invisibility!\n", cp->crt,
						(F_ISSET(cp->crt, MHIDDN) ? 
						", hidden in the shadows," : ""));
					ANSI(fd, WHITE);
                    print(fd, 
						"%M sees your transparent form and blocks the way.\n", 
							cp->crt);
                    return(0);
	    		}	
	    	}	

            cp = cp->next_tag;
        }
    }         

	if(F_ISSET(ext_ptr, XMOUSE) && !F_ISSET(ply_ptr, PSHRNK)) {
		print(fd, "Only a mouse could enter that tiny place.\n");
		return(0); 
	}

	if(F_ISSET(ext_ptr, XPLDGK))
		if(!F_ISSET(ply_ptr, PPLDGK)) {
       		print(fd, "Sorry, you have not pledged allegience to a Prince.\n");
        	return(0);
		}
	else if(BOOL(F_ISSET(ext_ptr, XKNGDM)) !=  BOOL(F_ISSET(ply_ptr, PKNGDM))) {
        print(fd, "Only loyalists to Prince %s may go there.\n",
			F_ISSET(ply_ptr, PKNGDM) ? "Ragnar":"Rutger");	
		if(ply_ptr->class < CARETAKER)
        	return(0);
	}

	if(F_ISSET(ext_ptr, XNAKED) && weight_ply(ply_ptr)) {
		print(fd, "You cannot bring anything through that exit.\n");
		if(ply_ptr->class < CARETAKER)
			return(0);
	}
 
	if(F_ISSET(ext_ptr, XFEMAL) && F_ISSET(ply_ptr, PMALES)) {
		print(fd, "Sorry, only females are allowed to go there.\n");
		if(ply_ptr->class < CARETAKER)
			return(0); 
	}

	if(F_ISSET(ext_ptr, XMALES) && !F_ISSET(ply_ptr, PMALES)) {
		print(fd, "Sorry, only males are allowed to go there.\n");
		if(ply_ptr->class < CARETAKER)
			return(0); 
	}           

	if(F_ISSET(ext_ptr, XSMALL) && 
		!F_ISSET(ply_ptr, PSHRNK) &&
		ply_ptr->race != DWARF &&
		ply_ptr->race != GNOME && 
		ply_ptr->race != HOBBIT) {
			print(fd, "Only a very small person can squeeze in there.\n");
			if(ply_ptr->class < CARETAKER)
				return(0); 
	}           

	if(F_ISSET(ext_ptr, XLARGE) && 
		(ply_ptr->race != OGRE &&
		ply_ptr->race != CENTAUR && 
		ply_ptr->race != HALFGIANT ||
		F_ISSET(ply_ptr, PSHRNK))) {
			print(fd, "Only someone very large can go there.\n");
			if(ply_ptr->class < CARETAKER)
				return(0); 
	}           

	if(F_ISSET(ext_ptr, XNOBIG) && 
		(ply_ptr->race == OGRE ||
		 ply_ptr->race == CENTAUR || 
		 ply_ptr->race == HALFGIANT)) {
			print(fd, "Large races are not allowed to go there.\n");
			if(ply_ptr->class < CARETAKER)
				return(0); 
	}           

	if((F_ISSET(ext_ptr, XCLIMB) || F_ISSET(ext_ptr, XREPEL)) &&
	   (!F_ISSET(ply_ptr, PLEVIT)) && !F_ISSET(ply_ptr, PFLYSP)) {
			fall = (F_ISSET(ext_ptr, XDCLIM) ? 50:0) + 50 - fall_ply(ply_ptr);
			if(mrand(1,100) < fall) {
				dmg = mrand(5, (15 + fall/10));
				if(ply_ptr->hpcur <= dmg) {
				   ANSI(fd, REVERSE);
        		   print(fd, "You fell to your death.\n");
				   ANSI(fd, NORMAL);
				   ANSI(fd, BLUEBG);
				   ANSI(fd, BOLD);
				   ANSI(fd, WHITE);
	    	       ply_ptr->hpcur=0;
				   broadcast_rom(fd, ply_ptr->rom_num, 
						"%M died from the fall.\n", ply_ptr);
        	       die(ply_ptr, ply_ptr);
        	       return(0);
        	    }

				ply_ptr->hpcur -= dmg;
				print(fd, "You fell and hurt yourself for %d damage.\n", dmg);
				broadcast_rom(fd, 
					ply_ptr->rom_num, "%M fell down.\n", ply_ptr);
	
				if(ply_ptr->hpcur < 1) { 
					print(fd, "You fell to your death.\n");
					die(ply_ptr, ply_ptr); 
				}  

				if(F_ISSET(ext_ptr, XCLIMB))
				return(0);
			}
	}

	i = LT(ply_ptr, LT_ATTCK);
	s = LT(ply_ptr, LT_SPELL);
	t = time(0);

	if(t < i || t < s) {
		p = MAX(i, s);
		please_wait(fd, p - t);
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	cp = rom_ptr->first_mon;
	while(cp) {
		if(F_ISSET(cp->crt, MBLOCK) && 
		   is_enm_crt(ply_ptr->name, cp->crt) &&
		   		!F_ISSET(ply_ptr, PINVIS) && ply_ptr->class < CARETAKER) {
					print(fd, "%M quickly blocks your exit.\n", cp->crt);
					return(0);
		}
		if(F_ISSET(cp->crt, MBLOCK) && 
		   is_enm_crt(ply_ptr->name, cp->crt) && ply_ptr->class >= CARETAKER) 
					print(fd, "DM NOTE: %M sighs and waves goodbye.\n", 
						cp->crt);
					cp = cp->next_tag;
	}

	if(!F_ISSET(rom_ptr, RPTRAK))
		strcpy(rom_ptr->track, ext_ptr->name);

	old_rom_num = rom_ptr->rom_num;
	old_rom_ptr = rom_ptr;

	load_rom(ext_ptr->room, &rom_ptr);
	if(rom_ptr == old_rom_ptr) {
		print(fd, "Off the map in that direction.\n");
		print(fd, "This is room number %d.\n", rom_ptr->rom_num);
		print(fd, 
			"Please note the room number and send mudmail to Styx, the DM.\n");
		return(0);
	}

	n = count_vis_ply(rom_ptr);

	if((rom_ptr->lolevel || rom_ptr->hilevel) && ply_ptr->class >= CARETAKER) {
		ANSI(fd, YELLOW);
		print(fd, "Boundary levels set...\n");
		print(fd, "Low Level = %d      High level = %d\n", 
			rom_ptr->lolevel, rom_ptr->hilevel);
		ANSI(fd, WHITE);
	}

	if(rom_ptr->lolevel > ply_ptr->level && ply_ptr->class < CARETAKER) {
		print(fd, "You must be at least level %d to go that way.\n",
			rom_ptr->lolevel);
		return(0);
	}
	else if(ply_ptr->level > rom_ptr->hilevel && rom_ptr->hilevel &&
		ply_ptr->class < CARETAKER) {
			print(fd, "Only players under level %d may go that way.\n",
				rom_ptr->hilevel+1);
			return(0);
	}
	else if((F_ISSET(rom_ptr, RONEPL) && n > 0) ||
		(F_ISSET(rom_ptr, RTWOPL) && n > 1) ||
		(F_ISSET(rom_ptr, RTHREE) && n > 2)) {
			print(fd, "That place is full.\n");
			return(0);
	}

	for(j = 0, found = 0; j < 5 && found == 0; j++)
		if(strcmp(ext_ptr->name, ext[j])  == 0)
			found = 1;

	if(found && !F_ISSET(ply_ptr, PDMINV) && !F_ISSET(ply_ptr, PCLOAK)) {
		broadcast_rom(fd, old_rom_ptr->rom_num, "%M went %s.", 
		    ply_ptr, ext_ptr->name);
	}
	else if(!F_ISSET(ply_ptr, PDMINV) && !F_ISSET(ply_ptr, PCLOAK)) { 
		broadcast_rom(fd, old_rom_ptr->rom_num, "%M went to the %s.", 
			ply_ptr, ext_ptr->name);
	}

	if(F_ISSET(ply_ptr, PALIAS)) {
    	del_crt_rom(Ply[ply_ptr->fd].extr->alias_crt, ply_ptr->parent_rom);
		if(strcmp(ext_ptr->name, "up") 
			|| strcmp(ext_ptr->name, "down") 
			|| strcmp(ext_ptr->name, "out") 
			|| strcmp(ext_ptr->name, "back") 
			|| strcmp(ext_ptr->name, "aft") 
			|| strcmp(ext_ptr->name, "forward")) 
        		broadcast_rom(ply_ptr->fd, ply_ptr->rom_num,
			 		"%M just went %s.", Ply[ply_ptr->fd].extr->alias_crt, 
						ext_ptr->name);
		else
        	broadcast_rom(ply_ptr->fd, ply_ptr->rom_num,
				"%M just went to the %s.", Ply[ply_ptr->fd].extr->alias_crt, 
					ext_ptr->name);
            add_crt_rom(Ply[ply_ptr->fd].extr->alias_crt, rom_ptr, 1);
	}

	del_ply_rom(ply_ptr, ply_ptr->parent_rom);
	add_ply_rom(ply_ptr, rom_ptr);

	cp = ply_ptr->first_fol;
	while(cp) {
		if(cp->crt->rom_num == old_rom_num && cp->crt->type != MONSTER)
			go(cp->crt, cmnd);
		cp = cp->next_tag;
	}

   if(is_rom_loaded(old_rom_num)) {
        cp = old_rom_ptr->first_mon;
        while(cp) {
            if(cp->crt->special == CRT_PORTR) {
                temp = cp->next_tag;
                if(!strcmp(cp->crt->key[1], ply_ptr->password)) {
                    if(cp->crt->mpcur > 0) {
                        crt_ptr = cp->crt;
                        del_crt_rom(crt_ptr, old_rom_ptr);
                        add_crt_rom(crt_ptr, rom_ptr, 1);
                        add_active(crt_ptr);
                        crt_ptr->mpcur--;
                        if(crt_ptr->mpcur < 0)
                            crt_ptr->mpcur = 0;
                        broadcast_rom(fd, old_rom_num, "%M's %m followed %s.",
                            ply_ptr, crt_ptr, F_ISSET(ply_ptr, PMALES) ? "him":"her");
                    }
                    else if(cp->crt->mpcur == 0)
                        print(fd, "Your porter refuses to follow without more pay.\n");
                }
                cp = temp;
                continue;
            }
            cp = cp->next_tag;
        }
    }

	if(is_rom_loaded(old_rom_num)) {
		cp = old_rom_ptr->first_mon;
		while(cp) {
			if(!F_ISSET(cp->crt, MFOLLO) || F_ISSET(cp->crt, MDMFOL)) {
				cp = cp->next_tag;
				continue;
			}
			if(!cp->crt->first_enm) {
				cp = cp->next_tag;
				continue;
			}
			if(strcmp(cp->crt->first_enm->enemy, ply_ptr->name)) {
				cp = cp->next_tag;
				continue;
			}
			if(mrand(1, 20) > 10 - ply_ptr->dexterity + cp->crt->dexterity) {
				cp = cp->next_tag;
				continue;
			}
			if(!F_ISSET(ply_ptr->parent_rom, RHEALR)) {
				print(fd, "%M followed you.\n", cp->crt);
				broadcast_rom(fd, old_rom_num, "%M follows %m.",
					cp->crt, ply_ptr);
				temp = cp->next_tag;
				crt_ptr = cp->crt;
				del_crt_rom(crt_ptr, old_rom_ptr);
				add_crt_rom(crt_ptr, rom_ptr, 1);
				add_active(crt_ptr); 
				F_CLR(crt_ptr, MPERMT);
				cp = temp;
			}
			else {
				print(fd, "%M could not follow you here.\n", cp->crt);
				cp = cp->next_tag;
			}
		}
	}

/*
	if(F_ISSET(ply_ptr, PFLYSP) || F_ISSET(ply_ptr, PLEVIT))
		print(fd, "You are %s.\n", 
			F_ISSET(ply_ptr, PFLYSP) ? "flying" : "levitating");
*/

	compute_agility(ply_ptr);
	check_traps(ply_ptr, rom_ptr);
	return(0);
}

/**********************************************************************/
/*								openexit			 				  */
/**********************************************************************/
/* This function allows a player to open a door if it is not already  */
/* open and if it is not locked.				     				  */

int openexit(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	exit_	*ext_ptr;
	room	*rom_ptr;
	int		fd;

	rom_ptr = ply_ptr->parent_rom;
	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Open what?\n");
		return(0);
	}

	ext_ptr = find_ext(ply_ptr, rom_ptr->first_ext, cmnd->str[1], cmnd->val[1]);

	if(!ext_ptr) {
		print(fd, "I don't see that exit.\n");
		return(0);
	}

	if(F_ISSET(ext_ptr, XLOCKD)) {
		print(fd, "It's locked.\n");
		return(0);
	}

	if(!F_ISSET(ext_ptr, XCLOSD)) {
		print(fd, "It's already open.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	F_CLR(ext_ptr, XCLOSD);
	ext_ptr->ltime.ltime = time(0);

	print(fd, "You open the %s.\n", ext_ptr->name);
	if(!F_ISSET(ply_ptr, PDMINV)) {
		if(!F_ISSET(ply_ptr, PCLOAK)) {
			broadcast_rom(fd, ply_ptr->rom_num, "%M opens the %s.", 
				ply_ptr, ext_ptr->name);
		}
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
		broadcast_rom(fd, ply_ptr->rom_num, 
			"A shadow flits across the %s, and it opens.", 
				ext_ptr->name);
	}

	compute_agility(ply_ptr);
	return(0);

}

/*********************************************************************/
/*							close exit								 */
/*********************************************************************/
/* This function allows a player to close a door, if the door is not */
/* already closed, and if indeed it is a door.					     */

int closeexit(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	exit_	*ext_ptr;
	room	*rom_ptr;
	int		fd;

	rom_ptr = ply_ptr->parent_rom;
	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Close what?\n");
		return(0);
	}

	ext_ptr = find_ext(ply_ptr, rom_ptr->first_ext, cmnd->str[1], cmnd->val[1]);

	if(!ext_ptr) {
		print(fd, "I don't see that exit.\n");
		return(0);
	}

	if(F_ISSET(ext_ptr, XCLOSD)) {
		print(fd, "It's already closed.\n");
		return(0);
	}

	if(!F_ISSET(ext_ptr, XCLOSS)) {
		print(fd, "You can't close that.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);
	F_SET(ext_ptr, XCLOSD);

	print(fd, "You close the %s.\n", ext_ptr->name);
	if(!F_ISSET(ply_ptr, PDMINV) && !F_ISSET(ply_ptr, PCLOAK)) {
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M closes the %s.", ply_ptr, ext_ptr->name);
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
		broadcast_rom(fd, ply_ptr->rom_num, 
			"A shadow covers the %s, and it closes.", ext_ptr->name);
	}

	compute_agility(ply_ptr);
	return(0);
}

/************************************************************************/
/*								unlock						   			*/
/************************************************************************/
/* This function allows a player to unlock a door if he has the correct */
/* key, and it is locked.												*/

int unlock(ply_ptr, cmnd, use)
creature	*ply_ptr;
cmd			*cmnd;
int			use;

{
	room	*rom_ptr;
	object	*obj_ptr;
	exit_	*ext_ptr;
	int		fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	
	if(use != 1)
		use = 0;

	if(cmnd->num < 2) {
		print(fd, "Unlock what?\n");
		return(0);
	}

	if(use) {
		ext_ptr = find_ext(ply_ptr, rom_ptr->first_ext,
			cmnd->str[2], cmnd->val[2]);
	}
	else {
		ext_ptr = find_ext(ply_ptr, rom_ptr->first_ext,
			cmnd->str[1], cmnd->val[1]);
	}

	if(!ext_ptr) {
		print(fd, "Can't find that exit!\n");
		return(0);
	}

	if(!F_ISSET(ext_ptr, XLOCKD)) {
		print(fd, "It's not locked.\n");
		return(0);
	}

	if(cmnd->num < 3) {
		print(fd, "Unlock it with what?\n");
		return(0);
	}

	if(use) {
		obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, 
			cmnd->str[1], cmnd->val[1]);
	}
	else {
		obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, 
			cmnd->str[2], cmnd->val[2]);
	}

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(obj_ptr->type != KEY) {
		print(fd, "That's not a key.\n");
		return(0);
	}

	if(obj_ptr->shotscur < 1) {
		print(fd, "%I is broken.\n", obj_ptr);
		return(0);
	}

	if(obj_ptr->ndice != ext_ptr->key) {
		print(fd, "Wrong key.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);
	F_CLR(ext_ptr, XLOCKD);

	ext_ptr->ltime.ltime = time(0);
	if(!F_ISSET(obj_ptr, ONUSED))
		obj_ptr->shotscur--;

	if(obj_ptr->use_output[0])
		print(fd, "%s\n", obj_ptr->use_output);
	else
		print(fd, "Click.\n");

	if(!F_ISSET(ply_ptr, PDMINV) && 
		!F_ISSET(ply_ptr, PINVIS) &&
		!F_ISSET(ply_ptr, PCLOAK)) {
		broadcast_rom(fd, ply_ptr->rom_num, "Click.  %M unlocks the %s.",
			ply_ptr, ext_ptr->name);
	}

	if(F_ISSET(ply_ptr, PINVIS)) {
		broadcast_rom(fd, ply_ptr->rom_num, 
			"Click.  The sound comes from the %s.", ext_ptr->name);
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
	broadcast_rom(fd, ply_ptr->rom_num, 
		"Click.  A shadowy figure unlocks the %s.", ext_ptr->name);
	}

	return(0);
}

/**********************************************************************/
/*								lock							      */
/**********************************************************************/
/* This function allows a player to lock a door with the correct key. */

int lock(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room	*rom_ptr;
	object	*obj_ptr;
	exit_	*ext_ptr;
	int		fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	
	if(cmnd->num < 2) {
		print(fd, "Lock which exit?\n");
		return(0);
	}

	ext_ptr = find_ext(ply_ptr, rom_ptr->first_ext,
		cmnd->str[1], cmnd->val[1]);

	if(!ext_ptr) {
		print(fd, "Lock which exit?\n");
		return(0);
	}

	if(F_ISSET(ext_ptr, XLOCKD)) {
		print(fd, "It's already locked.\n");
		return(0);
	}

	if(cmnd->num < 3) {
		print(fd, "Lock it with what?\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[2], cmnd->val[2]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(obj_ptr->type != KEY) {
		print(fd, "%I is not a key.\n", obj_ptr);
		return(0);
	}

	if(!F_ISSET(ext_ptr, XLOCKS)) {
		print(fd, 
			"Sorry, it doesn't have a lock on it and cannot be locked.\n");
		return(0);
	}

	if(!F_ISSET(ext_ptr, XCLOSD)) {
		print(fd, "You have to close it first.\n");
		return(0);
	}

	if(obj_ptr->shotscur < 1) {
		print(fd, "%I is broken.\n", obj_ptr);
		return(0);
	}

	if(obj_ptr->ndice != ext_ptr->key) {
		print(fd, "Wrong key.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	F_SET(ext_ptr, XLOCKD);

	print(fd, "Click.\n");
	if(!F_ISSET(ply_ptr, PDMINV) && 
		!F_ISSET(ply_ptr, PINVIS) &&
		!F_ISSET(ply_ptr, PCLOAK)) {
			broadcast_rom(fd, ply_ptr->rom_num, "%M locks the %s.",
				ply_ptr, ext_ptr->name);
	}

	if(F_ISSET(ply_ptr, PINVIS)) {
		broadcast_rom(fd, ply_ptr->rom_num, "Click.  The %s is locked.", 
			ext_ptr->name); 
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
		broadcast_rom(fd, ply_ptr->rom_num, "Click."); 
		broadcast_rom(fd, ply_ptr->rom_num, 
			"A shadow covers the %s, as it locks.", ext_ptr->name);
	}

	return(0);
}

/***********************************************************************/
/*								picklock						       */
/***********************************************************************/
/* This function is called when a thief or monk attempts to pick a 	   */
/* lock.  If the lock is pickable, there is a chance (depending on the */
/* player's level and luck) that the lock will be picked.              */

int picklock(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room	*rom_ptr;
	exit_	*ext_ptr;
	long	i, t;
	int		fd, chance;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->class != THIEF && ply_ptr->class != MONK && 
		ply_ptr->class < CARETAKER) {
			print(fd, "Only thieves and monks may pick locks.\n");
			return(0);
	}

	if(cmnd->num < 2) {
		print(fd, "Pick what?\n");
		return(0);
	}

	ext_ptr = find_ext(ply_ptr, rom_ptr->first_ext,
		cmnd->str[1], cmnd->val[1]);

	if(!ext_ptr) {
		print(fd, "I don't see that here.\n");
		return(0);
	}
	if(F_ISSET(ply_ptr, PBLIND) && Ply[ply_ptr->fd].extr->luck < 65) {
		print(fd, "You can't see to pick that lock.\n");
		return(0);
	}
	if(!F_ISSET(ext_ptr, XLOCKD)) {
		print(fd, "It's not locked.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	i = LT(ply_ptr, LT_PICKL);
	t = time(0);

	if(t < i) {
		please_wait(fd, i - t);
		return(0);
	}

	ply_ptr->lasttime[LT_PICKL].ltime = t;
	ply_ptr->lasttime[LT_PICKL].interval = 10;

	chance = (ply_ptr->class == THIEF) ? 10 * ply_ptr->level : 5*ply_ptr->level;
	chance += bonus[ply_ptr->dexterity] * 2 + Ply[ply_ptr->fd].extr->luck/60;

	if(F_ISSET(ext_ptr, XUNPCK)) {
		print(fd,
			"That lock is protected by a magic spell.  You cannot pick it.\n");
		return(PROMPT);
	}

	if(!F_ISSET(ply_ptr, PDMINV) && 
		!F_ISSET(ply_ptr, PINVIS) && 
		!F_ISSET(ply_ptr, PCLOAK)) {
			broadcast_rom(fd, ply_ptr->rom_num, "%M attempts to pick the %s.", 
				ply_ptr, ext_ptr->name);
	}
	else if(F_ISSET(ply_ptr, PCLOAK)) {
		broadcast_rom(fd, ply_ptr->rom_num, "Shadows swirl about the %s.", 
			ext_ptr->name);
	}

	if(mrand(1, 100) <= chance) {
		if(F_ISSET(ply_ptr, PBLIND) && Ply[ply_ptr->fd].extr->luck > 65) 
			print(fd, 
				"Even as blind as you are, your fantastic luck is holding.\n");
			print(fd, 
				"You picked the lock and gained %d experience points.\n", 
					1 + ply_ptr->level/2);
			ply_ptr->experience += 1 + ply_ptr->level/2;
			F_CLR(ext_ptr, XLOCKD);
			if(!F_ISSET(ply_ptr, PDMINV) && 
				!F_ISSET(ply_ptr, PINVIS) &&
				!F_ISSET(ply_ptr, PCLOAK)) {
					broadcast_rom(fd, ply_ptr->rom_num, "Click.  %s succeeded.",
						F_ISSET(ply_ptr, PMALES) ? "He":"She");
			}

			if(F_ISSET(ply_ptr, PINVIS)) {
				broadcast_rom(fd, ply_ptr->rom_num, "Click."); 
			}

			if(F_ISSET(ply_ptr, PCLOAK)) {
				broadcast_rom(fd, ply_ptr->rom_num, "Click."); 
				broadcast_rom(fd, ply_ptr->rom_num, 
					"A shadow moves over the %s, as it unlocks.", 
						ext_ptr->name);
			}
	}
	else
		print(fd, "You failed.\n");

	return(0);
}

/***********************************************************************/
/*								flee							       */
/***********************************************************************/
/* This function allows a player to flee from an enemy.  If successful */
/* the player will drop his readied weapon and run through one of the  */
/* visible exits.  Paladins lose 15 x level.						   */

int flee(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room	*rom_ptr;
	xtag	*xp;
	ctag	*cp, *temp;
	char 	found = 0;
	char	tempstr[40];
	int		fd, n, mon = 0;
	long	i, t;
	extra	*extr;

	extern char		ext[5][8];

	rom_ptr = ply_ptr->parent_rom;
	fd = ply_ptr->fd;

	if(F_ISSET(ply_ptr, PFROZE)) {
		print(fd, "You can't flee!  A freeze spell has been cast on you.\n");
		return(0);
	}

	if(compute_agility(ply_ptr) <= 15) {
		print(fd, "You're too clumsy and you stumbled!\n");
		return(0);
	}

	mon = count_mon(rom_ptr, 0) + count_mon(rom_ptr, 1);	
	if(mon == 0) { 
		print(fd, "What are you afraid of?  There's nothing to flee from!\n");
		return(0);
	}

	t = time(0);
	i = MAX(ply_ptr->lasttime[LT_ATTCK].ltime,
		ply_ptr->lasttime[LT_SPELL].ltime) + 4L;

	if(t < i && !F_ISSET(ply_ptr, PFEARS)) {
		please_wait(fd, i - t);
		return(0);
	}

    t = Time%24L;
	xp = rom_ptr->first_ext;
	if(xp) do {
		if(F_ISSET(xp->ext, XCLOSD)) continue;
		if((F_ISSET(xp->ext, XCLIMB) || F_ISSET(xp->ext, XDCLIM)) &&  
			(!F_ISSET(ply_ptr, PLEVIT))) continue; 
		if(F_ISSET(xp->ext, XNAKED) && weight_ply(ply_ptr)) continue;
        if(F_ISSET(xp->ext, XFEMAL) && F_ISSET(ply_ptr, PMALES)) continue;
        if(F_ISSET(xp->ext, XMALES) && !F_ISSET(ply_ptr, PMALES)) continue; 
		if(F_ISSET(xp->ext, XFLYSP) && !F_ISSET(ply_ptr, PFLYSP)) continue;
		if(!F_ISSET(ply_ptr, PDINVI) && F_ISSET(xp->ext, XINVIS)) continue;
		if(F_ISSET(xp->ext, XSECRT)) continue;
		if(F_ISSET(xp->ext, XNOSEE)) continue;
		if(F_ISSET(xp->ext, XPLDGK)) 
			if(!F_ISSET(ply_ptr, PPLDGK)) continue;
			else if(BOOL(F_ISSET(xp->ext, XKNGDM)) !=  
				BOOL(F_ISSET(ply_ptr, PKNGDM))) continue;
		if(F_ISSET(xp->ext, XNGHTO) && (t > 6 && t < 20)) continue;
		if(F_ISSET(xp->ext, XDAYON) && (t < 6 || t > 20))  continue;
		if(F_ISSET(xp->ext, XPLSEL) && 
			!F_ISSET(xp->ext, XPLSEL + ply_ptr->class)) continue;		
    	if(F_ISSET(xp->ext, XPGUAR)) {
    	    cp = rom_ptr->first_mon;
    	    while(cp) {
    	        if(F_ISSET(cp->crt, MPGUAR) && 
    		        !F_ISSET(ply_ptr, PINVIS) && ply_ptr->class < CARETAKER) {
						found = 1;
						break;
    	        }
    	        cp = cp->next_tag;
    	    }
			if(found) continue;
    	}         
		if(mrand(1, 100) < (65 + bonus[ply_ptr->dexterity] * 5)) 
			break;
	} 

	while(xp = xp->next_tag);

	if(xp && F_ISSET(xp->ext, 52) && mrand(1, 15) < 3 && 
		!F_ISSET(ply_ptr, PFEARS))
			xp = 0;

	if(!xp) {
		ANSI(fd, BOLD);
		ANSI(fd, YELLOW);
		print(fd, "You tried to flee but failed to escape!\n");
		ANSI(fd, WHITE);
		return(0);
	}

	if(ply_ptr->ready[WIELD - 1] &&
		!F_ISSET(ply_ptr->ready[WIELD - 1], OCURSE)) {
			ANSI(fd, YELLOW);
			print(fd, "You drop your %s as you flee!\n", 
				ply_ptr->ready[WIELD - 1]);
			if(!F_ISSET(rom_ptr, RSLIME))
				add_obj_rom(ply_ptr->ready[WIELD - 1], rom_ptr);
			ply_ptr->ready[WIELD - 1] = 0;
			compute_thaco(ply_ptr);
			compute_ac(ply_ptr);
			compute_agility(ply_ptr);
			if(F_ISSET(rom_ptr, RSLIME)) {
				ANSI(fd, GREEN);
				print(fd, 
					"The slime beneath your feet dissolves everything.\n");
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
			}
			savegame(ply_ptr, 0);
	}
	else if(ply_ptr->ready[HELD - 1] &&
		!F_ISSET(ply_ptr->ready[HELD - 1], OCURSE)) {
			ANSI(fd, YELLOW);
			print(fd, "Your %s fell as you fled!\n",
				ply_ptr->ready[HELD - 1]);
			if(!F_ISSET(rom_ptr, RSLIME))
				add_obj_rom(ply_ptr->ready[HELD - 1], rom_ptr);
			ply_ptr->ready[HELD - 1] = 0;
			compute_thaco(ply_ptr);
			compute_ac(ply_ptr);
			compute_agility(ply_ptr);
			if(F_ISSET(rom_ptr, RSLIME)) {
				ANSI(fd, GREEN);
				print(fd, 
					"The slime beneath your feet dissolves everything.\n");
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
			}
			savegame(ply_ptr, 0);
	}
	else {
		if(!F_ISSET(ply_ptr, PFEARS)) {
			ANSI(fd, YELLOW);
			print(fd, "You run like a chicken.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
		}
	}

	F_CLR(ply_ptr, PHIDDN);

	if(F_ISSET(ply_ptr, PFEARS)) {
		ANSI(fd, YELLOW);
		print(fd, "Your fear causes you to run screaming in terror!\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		F_CLR(ply_ptr, PFEARS);
	}

	if(!F_ISSET(rom_ptr, RPTRAK))
		strcpy(rom_ptr->track, xp->ext->name);
		strcpy(tempstr, xp->ext->name);
		for(i = 0, found = 0; i < 5 && found == 0; i++) {
			if(strcmp(tempstr, ext[i]) == 0)
			found = 1;
		}

	if(found == 0) {
		print(fd, "You fled to the %s.\n", tempstr);
		if(!F_ISSET(ply_ptr, PCLOAK)) {
			broadcast_rom(fd, rom_ptr->rom_num, "%M flees to the %s.", 
				ply_ptr, tempstr);
		}
		else {
			broadcast_rom(fd, rom_ptr->rom_num, "A shadow flees to the %s.", 
				tempstr);
		}
	}
	else {
		print(fd, "You flee %s.\n", tempstr);
		if(!F_ISSET(ply_ptr, PCLOAK)) {
			broadcast_rom(fd, rom_ptr->rom_num, "%M flees %s.", 
				ply_ptr, tempstr);
		}
		else {
			broadcast_rom(fd, rom_ptr->rom_num, "A shadow flees %s.", 
				tempstr);
		}
	}

	if(ply_ptr->class == PALADIN && ply_ptr->level > 5) {
		n = ply_ptr->level*15;
		n = MIN(ply_ptr->experience, n);
		print(fd,"You lose %d experience for your cowardly retreat.\n", n);
		ply_ptr->experience -= n;
		lower_prof(ply_ptr,n);
	}

	load_rom(xp->ext->room, &rom_ptr);

	if(rom_ptr->lolevel > ply_ptr->level || (ply_ptr->level > 
		rom_ptr->hilevel && rom_ptr->hilevel)) {
			ANSI(fd, MAGENTA);
			print(fd, "You are thrown back by an invisible force.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			if(!F_ISSET(ply_ptr, PCLOAK)) {
				broadcast_rom(fd, rom_ptr->rom_num, "%M just arrived.",
					ply_ptr);
			}
			return(0);
	}

	n = count_vis_ply(rom_ptr);
	if((F_ISSET(rom_ptr, RONEPL) && n > 0) ||
	   (F_ISSET(rom_ptr, RTWOPL) && n > 1) ||
	   (F_ISSET(rom_ptr, RTHREE) && n > 2)) {
			print(fd, "The place you fled to was full!\n");
			if(!F_ISSET(ply_ptr, PCLOAK)) {
				broadcast_rom(fd, rom_ptr->rom_num, "%M just arrived.",
					ply_ptr);
			}
			return(0);
	}

	if(F_ISSET(ply_ptr, PALIAS)) {
		del_crt_rom(Ply[ply_ptr->fd].extr->alias_crt, ply_ptr->parent_rom);
		add_crt_rom(Ply[ply_ptr->fd].extr->alias_crt, rom_ptr, 1);
	}
	
	ANSI(fd, BOLD);
	ANSI(fd, WHITE);
	savegame(ply_ptr, 0);
	del_ply_rom(ply_ptr, ply_ptr->parent_rom);
	add_ply_rom(ply_ptr, rom_ptr);
	check_traps(ply_ptr, rom_ptr);

	return(0);

}


/*************************************************************************/
/*								track							         */
/*************************************************************************/
/* This function is the routine that allows rangers, assassins and       */
/* druids to search for tracks in a room.  If successful, the player     */
/* will be told the departed direction for the previous person who was   */
/* in the room.	       													 */

int track(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	long	i, t;
	int		fd, chance, found = 0;

	extern char		ext[5][8];

	fd = ply_ptr->fd;

	if(ply_ptr->class != RANGER && 
		ply_ptr->class != DRUID && 
		ply_ptr->class != ASSASSIN && 
		ply_ptr->class < CARETAKER) {
			print(fd, "Only rangers, assassins and druids can track.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	t = time(0);
	i = LT(ply_ptr, LT_TRACK);

	if(t < i) {
		please_wait(fd, i - t);
		return(0);
	}

	ply_ptr->lasttime[LT_TRACK].ltime = t;
	ply_ptr->lasttime[LT_TRACK].interval = 5 - bonus[ply_ptr->dexterity];
	
	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "You're blind...how can you see tracks?\n");
		return(0);
	}
	chance = 25 + (bonus[ply_ptr->dexterity] + ply_ptr->level) * 5;

	if(mrand(1, 100) > chance) {
		print(fd, "You fail to find any tracks.\n");
		return(0);
	}

	if(!ply_ptr->parent_rom->track[0]) {
		print(fd, "There are no tracks in this room.\n");
		return(0);
	}


	for(i = 0, found = 0; i < 5 && found == 0; i++) {
		if(strcmp(ply_ptr->parent_rom->track, ext[i]) == 0)
			found = 1;
	}
	if(found == 1)	
		print(fd, "You find tracks leading %s.\n",
			ply_ptr->parent_rom->track);
	else
		print(fd, "You find tracks leading to the %s.\n",	
			ply_ptr->parent_rom->track);

	if(!F_ISSET(ply_ptr, PDMINV) &&
		(!F_ISSET(ply_ptr, PCLOAK) &&
		!F_ISSET(ply_ptr, PINVIS))) {
			broadcast_rom(fd, ply_ptr->rom_num, "%M finds tracks.", ply_ptr);
	}

	return(0);
}

/************************************************************************/
/*                           sneak                                      */
/************************************************************************/
/*	This allows a hidden player to move while hidden. 			   		*/ 

int sneak(ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;

{
    room        *rom_ptr, *old_rom_ptr;
    creature    *crt_ptr;
    ctag        *cp, *temp;
    exit_       *ext_ptr;
    long        i, t;
    int     	fd, old_rom_num, fall, dmg, n, j;
    int         chance, found = 0;
	extra		*extr;

	extern char		ext[5][8];

    rom_ptr = ply_ptr->parent_rom;
    fd = ply_ptr->fd;

	if(ply_ptr->class != ASSASSIN && ply_ptr->class != THIEF && 
		ply_ptr->class != MONK && ply_ptr->class < CARETAKER) {
			print(fd, "Only thieves, monks and assassins may sneak.\n");
			return(0);
	}                  

	if(F_ISSET(ply_ptr, PFROZE)) {
		print(fd, 
			"A freeze spell has been cast on you, and you cannot move.\n" );
		return(0);
	}                  

	if(!F_ISSET(ply_ptr, PHIDDN)) {
		print(fd, "You need to hide before you can sneak.\n");
		return(0);
	}                  

	if(cmnd->num < 2) {
		print(fd, "Sneak where?\n");
		return(0);
	}

	if(!strcmp(cmnd->str[1], "ex") || !strcmp(cmnd->str[1], "exit"))
		strcpy(cmnd->str[1], "out");

	ext_ptr = find_ext(ply_ptr, rom_ptr->first_ext, 
		cmnd->str[1], cmnd->val[1]);

	if(!ext_ptr) {
		print(fd, "Where is that exit?\n");
		return(0);
	}

	compute_agility(ply_ptr);
	if(Ply[ply_ptr->fd].extr->agility < 20) {
		print(fd, "You cannot sneak while carrying so much weight.\n");	
		return(0);
	}

	if(F_ISSET(ext_ptr, XNGOOD)) {
		if(!F_ISSET(ext_ptr, XNGOOD + (ply_ptr->alignment > 100))) {
			ANSI(fd, YELLOW);
			print(fd, "Although that exit is protected by a ward of evil,\n");
			print(fd, "you have avoided detection by sneaking.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
		}
	}

	if(F_ISSET(ext_ptr, XNOEVL)) {
		if(!F_ISSET(ext_ptr, XNOEVL + (ply_ptr->alignment < -100))) {
			ANSI(fd, YELLOW);
			print(fd, 
				"Although that exit is protected by a ward of goodness,\n");
			print(fd, "you have avoided detection by sneaking.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
		}
	}

	if(F_ISSET(ext_ptr, XNOINV)) {
		print(fd,
			"A flicker of light falls across the %s and you cannot enter.\n", 
				ext_ptr->name);
		return(0);
	}

    if(F_ISSET(ext_ptr, XMOUSE) && !F_ISSET(ply_ptr, PSHRNK)) {
        print(fd, "You would have to be the size of a mouse to go there.\n");
        return(0);
    }

    if(F_ISSET(ext_ptr, XLOCKD)) {
        print(fd, "It's locked.\n");
        return(0);
    }
    else if(F_ISSET(ext_ptr, XCLOSD)) {
        print(fd, "You have to open it first.\n");
        return(0);
    }

    if(F_ISSET(ext_ptr, XFLYSP) && !F_ISSET(ply_ptr, PFLYSP)) {
        print(fd, "You must fly to get there.\n");
        return(0);
    }

    if(F_ISSET(ext_ptr, XNAKED) && weight_ply(ply_ptr)) {
        print(fd, "Nothing can be taken through that exit.\n");
        return(0);
    }

	if(F_ISSET(ext_ptr, XSMALL) && !F_ISSET(ply_ptr, PSHRNK) &&
		ply_ptr->race != DWARF && ply_ptr->race != GNOME && 
			ply_ptr->race != HOBBIT) {
				print(fd, "Only a very small person can sneak in there.\n");
				return(0); 
	}           

	if(F_ISSET(ext_ptr, XLARGE) && (ply_ptr->race != OGRE &&
		ply_ptr->race != CENTAUR && ply_ptr->race != HALFGIANT ||
			F_ISSET(ply_ptr, PSHRNK))) {
				print(fd, "You have to be very large to go there.\n");
				return(0); 
	}           

	if(F_ISSET(ext_ptr, XNOBIG) && (ply_ptr->race == OGRE ||
		ply_ptr->race == CENTAUR || ply_ptr->race == HALFGIANT)) {
			print(fd, "You are much too large to sneak in there.\n");
			return(0); 
	}           

    if(F_ISSET(ext_ptr, XPLDGK)) 
     if (!F_ISSET(ply_ptr, PPLDGK)) {
     	print(fd, 
			"You are not pledged to the prince that controls that place.\n");
        return(0);
     }
     else if(BOOL(F_ISSET(ext_ptr, XKNGDM)) != BOOL(F_ISSET(ply_ptr, PKNGDM))) {
        print(fd, "Your pledged allegience is wrong to go there.\n");
        return(0);
     }          

    t = Time%24L;
    if(F_ISSET(ext_ptr, XNGHTO) && (t > 6 && t < 20)) {
        print(fd, "That exit is not open during the day.\n");
        return(0);
    }          

    if(F_ISSET(ext_ptr, XDAYON) && (t < 6 || t > 20)) {
        print(fd, "That exit is closed for the night.\n");
        return(0);
    }          
 
    if(F_ISSET(ext_ptr, XPGUAR)) {
        cp = rom_ptr->first_mon;
        while(cp) {
            if(F_ISSET(cp->crt, MPGUAR)) { 
            	if(!F_ISSET(ply_ptr, PINVIS) && ply_ptr->class < CARETAKER) {
                	print(fd, "%M forcefully blocks your exit.\n", cp->crt);
                	return(0);
				}
            	if(!F_ISSET(ply_ptr, PINVIS) && ply_ptr->class >= CARETAKER)
					print(fd, "%M winks and waves you through.\n", cp->crt);

     			if(F_ISSET(cp->crt, MDINVI) && ply_ptr->class < CARETAKER) {
       				print(fd, 
						"%M detects your invisibility and blocks your exit.\n", 
							cp->crt);
                	return(0);
				}
     			if(F_ISSET(cp->crt, MDINVI) && ply_ptr->class >= CARETAKER) 
				print(fd, 
					"%M detects your invisibility and waves you through.\n", 
						cp->crt);
            }
            cp = cp->next_tag;
        }
    }         
 
    if(F_ISSET(ext_ptr, XFEMAL) && F_ISSET(ply_ptr, PMALES)) {
        print(fd, "Sorry, only females are allowed to go there.\n");
        return(0); 
    }
    if(F_ISSET(ext_ptr, XMALES) && !F_ISSET(ply_ptr, PMALES)) {
        print(fd, "Sorry, only males are allowed to go there.\n");
        return(0); 
    }           

    if((F_ISSET(ext_ptr, XCLIMB) || F_ISSET(ext_ptr, XREPEL)) &&
		(!F_ISSET(ply_ptr, PLEVIT) && !F_ISSET(ply_ptr, PFLYSP))) {
			fall = (F_ISSET(ext_ptr, XDCLIM) ? 50:0) + 50 - fall_ply(ply_ptr);
        	if(mrand(1, 100) < fall) {
            	dmg = mrand(5, 15 + fall/10);
            		if(ply_ptr->hpcur <= dmg) {
						ANSI(fd, REVERSE);
						ANSI(fd, BLINK);
            		    print(fd, "You fell to your death.\n");
						ANSI(fd, NORMAL);
						ANSI(fd, BLUEBG);
						ANSI(fd, BOLD);
						ANSI(fd, WHITE);
            		    ply_ptr->hpcur = 0;
            		    broadcast_rom(fd, ply_ptr->rom_num, 
							"%s died from the fall.\n", ply_ptr->name);
            	    	die(ply_ptr, ply_ptr);
            	    	return(0);
            		}
            	ply_ptr->hpcur -= dmg;
            	print(fd, "You fell and hurt yourself for %d damage.\n", dmg);
            	broadcast_rom(fd, ply_ptr->rom_num, "%M fell down.", ply_ptr);

				if(ply_ptr->hpcur < 1) {
					ANSI(fd, REDBG);
					ANSI(fd, REVERSE);
					ANSI(fd, BLINK);
        		    print(fd, "You died from your fall injuries.\n");
					ANSI(fd, NORMAL);
					ANSI(fd, BLUEBG);
					ANSI(fd, BOLD);
					ANSI(fd, WHITE);
        		   	broadcast_rom(fd, ply_ptr->rom_num, 
						"%M fell to %s death.", ply_ptr, 
							F_ISSET(ply_ptr, PMALES) ? "his":"her");
        		    die(ply_ptr, ply_ptr);
				}

            	if(F_ISSET(ext_ptr, XCLIMB))
            	    return(0);
        	}
    }

    i = LT(ply_ptr, LT_ATTCK);
    t = time(0);
    if(t < i) {
        please_wait(fd, i - t);
        return(0);
    }

    chance = MIN(85, 5 + 6 * ply_ptr->level + 3 * bonus[ply_ptr->dexterity]);
	if(F_ISSET(ply_ptr, PBLIND))
		chance = MIN(20, chance);

    if(mrand(1, 100) > chance) {
        print(fd,
			"You tripped over your own feet and clumsily failed to sneak.\n"); 
        F_CLR(ply_ptr, PHIDDN);

        cp = rom_ptr->first_mon;
        while(cp) {
            if(F_ISSET(cp->crt, MBLOCK) && 
            	is_enm_crt(ply_ptr->name, cp->crt) && 
					!F_ISSET(ply_ptr, PINVIS) && 
					ply_ptr->class < CARETAKER) {
               			print(fd, "%M speedily blocks your exit.\n", cp->crt);
               			return(0);
            }
            cp = cp->next_tag;
        }
        return(0);
    }

    if(!F_ISSET(rom_ptr, RPTRAK))
        strcpy(rom_ptr->track, ext_ptr->name);

    old_rom_num = rom_ptr->rom_num;
    old_rom_ptr = rom_ptr;

    load_rom(ext_ptr->room, &rom_ptr);
    if(rom_ptr == old_rom_ptr) {
        print(fd, "Off the map in that direction.\n");
        print(fd, "This is room number %d.\n", rom_ptr->rom_num);
        print(fd, 
			"Please note the room number and send mudmail to Styx, the DM.\n");
        return(0);
    }

    n = count_vis_ply(rom_ptr);

    if((rom_ptr->lolevel || rom_ptr->hilevel) && ply_ptr->class >= CARETAKER) 
		print(fd, "Level selective exit.\n");

    if(rom_ptr->lolevel > ply_ptr->level && ply_ptr->class < CARETAKER) {
        print(fd, "You must be at least level %d to go that way.\n",
        	rom_ptr->lolevel);
        return(0);
    }
    else if(ply_ptr->level > rom_ptr->hilevel && 
		rom_ptr->hilevel && ply_ptr->class < CARETAKER) {
        	print(fd, "Only players under level %d may go that way.\n",
            	rom_ptr->hilevel + 1);
        	return(0);
    }
    else if((F_ISSET(rom_ptr, RONEPL) && n > 0) ||
        	(F_ISSET(rom_ptr, RTWOPL) && n > 1) ||
        	(F_ISSET(rom_ptr, RTHREE) && n > 2)) {
        		print(fd, "That area is full.\n");
        		return(0);
    }

    del_ply_rom(ply_ptr, ply_ptr->parent_rom);
    add_ply_rom(ply_ptr, rom_ptr);

/*
	if(F_ISSET(ply_ptr, PFLYSP) || F_ISSET(ply_ptr, PLEVIT))
		print(fd, "You are %s.\n",
			F_ISSET(ply_ptr, PFLYSP) ? "flying" : "levitating");
*/

    check_traps(ply_ptr, rom_ptr);
    return(0);
}
     
/**************************************************************************/
/*								home								      */
/**************************************************************************/
/* This function allows a player to determine his home room				  */	

int home(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	int		fd; 
	fd = ply_ptr->fd;
	print(fd, "Your home is %s.\n", Ply[fd].extr->home_rom->name);	
}


/*************************************************************************/
/*								follow							         */
/*************************************************************************/
/* This command allows a player (or a monster) to follow another player. */
/* Follow loops are not allowed; i.e. you cannot follow someone who is   */
/* following you.  Also, you cannot follow yourself.			 		 */

int follow(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*old_ptr, *new_ptr;
	room		*rom_ptr;
	ctag		*cp, *pp, *prev;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Follow whom?\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PFROZE)) {
		print(fd, 
			"You can't follow anyone.  "
				"A freeze spell has been cast on you.\n");
		return(0);
	}
 
	lowercize(cmnd->str[1], 1);
	new_ptr = find_crt(ply_ptr, rom_ptr->first_ply, cmnd->str[1], cmnd->val[1]);

	if(!new_ptr) {
		print(fd, "No one here by that name.\n");
		return(0);
	}

	if(new_ptr == ply_ptr && !ply_ptr->following) {
		print(fd, "You can't follow yourself.\n");
		return(0);
	}

	if(new_ptr->following == ply_ptr) {
		print(fd, "You can't.  %s's following you.\n",
			F_ISSET(new_ptr, PMALES) ? "He":"She");
		return(0);
	}

	if(F_ISSET(new_ptr, PNOFOL)) {
		print(fd, "You can't.  %s's not allowing anyone to follow %s.\n",
			F_ISSET(new_ptr, PMALES) ? "He":"She",
			F_ISSET(new_ptr, PMALES) ? "him":"her");
		return(0);
	}

	compute_agility(ply_ptr);
	if(Ply[ply_ptr->fd].extr->agility < 12) {
		print(fd, 
			"You cannot follow and keep up while carrying so much weight.\n");	
    	broadcast_rom(fd, rom_ptr->rom_num, 
			"%M stumbles and cannot follow anyone.", ply_ptr);
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	if(ply_ptr->following) {
		old_ptr = ply_ptr->following;
		cp = old_ptr->first_fol;
		if(cp->crt == ply_ptr) {
			old_ptr->first_fol = cp->next_tag;
			free(cp);
		}
		else while(cp) {
			if(cp->crt == ply_ptr) {
				prev->next_tag = cp->next_tag;
				free(cp);
				break;
			}
			prev = cp;
			cp = cp->next_tag;
		}
		ply_ptr->following = 0;
		print(fd, "You stop following %s.\n", old_ptr->name);
		if(!F_ISSET(ply_ptr, PDMINV))
			print(old_ptr->fd, "%M stops following you.\n", ply_ptr);
	}

	if(ply_ptr == new_ptr)
		return(0);

	ply_ptr->following = new_ptr;

	pp = (ctag *)malloc(sizeof(ctag));
	if(!pp)
		merror("follow", FATAL);
	pp->crt = ply_ptr;
	pp->next_tag = 0;

	if(!new_ptr->first_fol)
		new_ptr->first_fol = pp;
	else {
		pp->next_tag = new_ptr->first_fol;
		new_ptr->first_fol = pp;
	}

	print(fd, "You start following %s.\n", new_ptr->name);
	if(!F_ISSET(ply_ptr, PDMINV) && !F_ISSET(ply_ptr, PCLOAK)) {
		print(new_ptr->fd, "%M starts following you.\n", ply_ptr);
		broadcast_rom2(fd, new_ptr->fd, ply_ptr->rom_num,
			"%M follows %m.", ply_ptr, new_ptr);
	}		

	if(F_ISSET(ply_ptr, PCLOAK)) {
		print(new_ptr->fd, "A shadowy figure follows you.\n");
	}		


	return(0);
}

/**********************************************************************/
/*								lose							      */
/**********************************************************************/
/* This function allows a player to lose another player who might be  */
/* following him.  When successful, that player will no longer be     */
/* following.							     						  */

int lose(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	ctag		*cp, *prev;
	int			fd;

	fd = ply_ptr->fd;

	if(cmnd->num == 1) {
		if(ply_ptr->following == 0) {
			print(fd, "You're not following anyone.\n");
			return(0);
		}
		crt_ptr = ply_ptr->following;
		cp = crt_ptr->first_fol;
		if(cp->crt == ply_ptr) {
			crt_ptr->first_fol = cp->next_tag;
			free(cp);
		}
		else while(cp) {
			if(cp->crt == ply_ptr) {
				prev->next_tag = cp->next_tag;
				free(cp);
				break;
			}
			prev = cp;
			cp = cp->next_tag;
		}
        ply_ptr->following = 0; 
		print(fd, "You stop following %m.\n", crt_ptr);
		if(!F_ISSET(ply_ptr, PDMINV))
			print(crt_ptr->fd, "%M stops following you.\n", ply_ptr);
		return(0);
	}                   

	F_CLR(ply_ptr, PHIDDN);

	lowercize(cmnd->str[1], 1);
	crt_ptr = find_crt(ply_ptr, ply_ptr->first_fol,
		cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		print(fd, "That person is not following you.\n");
		return(0);
	}

	if(crt_ptr->following != ply_ptr) {
		print(fd, "That person is not following you.\n");
		return(0);
	}

	cp = ply_ptr->first_fol;
	if(cp->crt == crt_ptr) {
		ply_ptr->first_fol = cp->next_tag;
		free(cp);
	}
	else while(cp) {
		if(cp->crt == crt_ptr) {
			prev->next_tag = cp->next_tag;
			free(cp);
			break;
		}
		prev = cp;
		cp = cp->next_tag;
	}
	crt_ptr->following = 0;

	print(fd, "You lose %s.\n", F_ISSET(crt_ptr, PMALES) ? "him":"her");
	if(!F_ISSET(ply_ptr, PDMINV)) {
		print(crt_ptr->fd, "%M loses you.\n", ply_ptr);
		broadcast_rom2(fd, crt_ptr->fd, "%M loses %m.", ply_ptr, crt_ptr);
	}

	return(0);
}

/************************************************************************/
/*								group							        */
/************************************************************************/
/* This function allows you to see who is in a group or party of people */
/* who are following you.												*/

int group(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	ctag	*cp;
	char	str[2048];
	char	hpstr[5];
	int		fd, found = 0;

	fd = ply_ptr->fd;

	cp = ply_ptr->first_fol;
	if(!cp) {
		print(fd, "No one is following you.\n");
		return(0);
	}

	strcpy(str, "People in your group: ");
	while(cp) {
		if(!F_ISSET(cp->crt, PDMINV)) {
			strcat(str, crt_str(cp->crt, 0, 0));
			strcat(str, "(");
			sprintf(hpstr, "%d", cp->crt->hpcur);
			strcat(str, hpstr);
			sprintf(hpstr, "/%dH ", cp->crt->hpmax);
			strcat(str, hpstr);
			sprintf(hpstr, "%d", cp->crt->mpcur);
			strcat(str, hpstr);
			sprintf(hpstr, "/%dM)", cp->crt->mpmax);
			strcat(str, hpstr);
			strcat(str, ", ");
			found = 1;
		}
		cp = cp->next_tag;
	}

	if(!found) {
		print(fd, "No one is following you.\n");
		return(0);
	}

	str[strlen(str) - 2] = 0;
	print(fd, "%s.\n", str);

	return(0);
}

