/*
 * COMMAND11.C:
 * 
 *
 *   Additional user routines
 *
 */

#include "mstruct.h"
#include "mextern.h"


/**********************************************************************/
/*							look								      */
/**********************************************************************/
/* This function is called when a player tries to look at the room he */
/* is in, or at an object in the room or in his inventory.            */

int look(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room		*rom_ptr;
	object		*obj_ptr;
	creature	*crt_ptr;
	char		str[2048];
	int			fd, n, match = 0;
	int			count = 1, num_in_inventory = 0;
	int			inv = 0, wear = 0;

	fd = ply_ptr->fd;

	if(F_ISSET(ply_ptr, PBLIND)) {
		CLS(fd);
		ANSI(fd, RED);
		print(fd, "You're blind!\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		return(0);
	}


/*   LOOK AT THE ROOM	*/

	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		ANSI(fd, BLUEBG);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		display_rom(ply_ptr, rom_ptr);
		return(0);
	}


/*   LOOK AT OBJECTS: - INVENTORY, WEAR, ON THE FLOOR	*/

	while(obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, 
		cmnd->str[1], count)) {
			count++;
	}
	num_in_inventory = count - 1;

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, 
		cmnd->str[1], cmnd->val[1]);
	if(obj_ptr)
		inv = 1;

	if(!obj_ptr || !cmnd->val[1]) {
		for(n = 0; n < MAXWEAR; n++) {
			if(!ply_ptr->ready[n])
				continue;
			if(EQUAL(ply_ptr->ready[n], cmnd->str[1]))
				match++;
			else continue;
			if(cmnd->val[1] -num_in_inventory == match || !cmnd->val[1]) {
				obj_ptr = ply_ptr->ready[n];
				break;
			}
		}
		if(obj_ptr)
			wear = 1;
	}

	if(!obj_ptr && !num_in_inventory && !match) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
			cmnd->str[1], cmnd->val[1]);
	}
	else if(!obj_ptr && num_in_inventory && !match) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
			cmnd->str[1], cmnd->val[1] - num_in_inventory);
	}
	else if(!obj_ptr && !num_in_inventory && match) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
			cmnd->str[1], cmnd->val[1] - match);
	}
	else if(!obj_ptr && num_in_inventory && match) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
			cmnd->str[1], cmnd->val[1] - num_in_inventory - match);
	}

	if(obj_ptr) {
		if(obj_ptr->special) {
			n = special_obj(ply_ptr, cmnd, SP_MAPSC);
			if(n != -2) return(MAX(n, 0));
		}
		if(!F_ISSET(obj_ptr, OINVIS) && obj_ptr->description[0]) {
			print(fd, "\n");
			if(inv)
				print(fd, "+");
			if(wear)
				print(fd, "*");
			print(fd, "%s\n", obj_ptr->description);
			if((F_ISSET(obj_ptr, OENCHA) || F_ISSET(obj_ptr, OTMPEN)) &&
				obj_ptr->shotscur > 0)
				print(fd, "It looks enchanted.\n");
			if(F_ISSET(obj_ptr, OPLDGK)) {
				print(fd, "It belongs to %s kingdom.\n", 
					(F_ISSET(obj_ptr, OKNGDM) ? "Rutger's" : "Ragnar's")); 
			}
		}
		else if(!F_ISSET(obj_ptr, OINVIS) && !obj_ptr->description[0]) {
			print(fd, "\n");
			if(inv)
				print(fd, "+");
			if(wear)
				print(fd, "*");
			print(fd, "You see nothing special about it.\n");
			if(F_ISSET(obj_ptr, OENCHA) || F_ISSET(obj_ptr, OTMPEN))
				print(fd, "But it does look enchanted.\n");
			if(F_ISSET(obj_ptr, OPLDGK)) {
				print(fd, "It belongs to %s kingdom.\n", 
					(F_ISSET(obj_ptr, OKNGDM) ? "Rutger's" : "Ragnar's")); 
			}
		}

		if(F_ISSET(obj_ptr, OINVIS) && obj_ptr->description[0]) {
			if((F_ISSET(ply_ptr, PDINVI)) || (ply_ptr->class >= CARETAKER)) {   
				ANSI(fd, MAGENTA);
				print(fd, "You detect: %s.\n", obj_ptr->name);
				if(inv)
					print(fd, "+");
				if(wear)
					print(fd, "*");
				print(fd, "%s\n", obj_ptr->description);
				ANSI(fd, WHITE);
				if(F_ISSET(obj_ptr, OPLDGK)) {
					print(fd, "It belongs to %s kingdom.\n", 
						(F_ISSET(obj_ptr, OKNGDM) ? "Rutger's" : "Ragnar's")); 
				}
				ANSI(fd, WHITE);
			}
		}
		else if(F_ISSET(obj_ptr, OINVIS) && !obj_ptr->description[0]) {
			if((F_ISSET(ply_ptr, PDINVI)) || (ply_ptr->class >= CARETAKER)) {   
				ANSI(fd, MAGENTA);
				print(fd, "You detect: %s.\n", obj_ptr->name);
				if(inv)
					print(fd, "+");
				ANSI(fd, WHITE);
				if(wear)
					print(fd, "*");
				print(fd, "There is nothing special about it.\n");
				if(F_ISSET(obj_ptr, OPLDGK)) {
					print(fd, "It belongs to %s kingdom.", 
						(F_ISSET(obj_ptr, OKNGDM) ? "Rutger's" : "Ragnar's")); 
				}
				ANSI(fd, WHITE);
			}
		}

		if(F_ISSET(ply_ptr, PKNOWA)) {
			if(F_ISSET(obj_ptr, OGOODO))
				print(fd, "It has a blue aura.\n");
			if(F_ISSET(obj_ptr, OEVILO))
				print(fd, "It has a red aura.\n");
		}

		if(F_ISSET(obj_ptr, OLUCKY)) {
			if(obj_ptr->shotscur < 1)
				print(fd, "There's no more luck in it.\n");
			else
				print(fd, "It looks to be a lucky charm.\n");
		} 

		if(F_ISSET(obj_ptr, OCONTN) && 
			F_ISSET(obj_ptr, OFASTD) &&
			!F_ISSET(obj_ptr, OINVIS)) {
				print(fd, "It's closed with a fastener.\n");
				return(0);
		}

		if(F_ISSET(obj_ptr, OCONTN) && 
			F_ISSET(obj_ptr, OFASTS) &&
			!F_ISSET(obj_ptr, OFASTD) && 
			!F_ISSET(obj_ptr, OINVIS)) {
				print(fd, "It's open, but can be fastened.\n");
		}
		if(F_ISSET(obj_ptr, OCONTN) && 
			F_ISSET(obj_ptr, OFASTD) &&
			F_ISSET(obj_ptr, OINVIS) && 
			(F_ISSET(ply_ptr, PDINVI) ||
			ply_ptr->class >= CARETAKER)) {
				print(fd, "It's closed with a fastener.\n");
				return(0);
		}

		if(F_ISSET(obj_ptr, OCONTN) && 
			F_ISSET(obj_ptr, OFASTS) &&
			F_ISSET(obj_ptr, OINVIS) && 
			(F_ISSET(ply_ptr, PDINVI) ||
			ply_ptr->class >= CARETAKER)) {
				print(fd, "It's open, but can be fastened.\n");
		}

		if(F_ISSET(obj_ptr, OCONTN) && 
			!F_ISSET(obj_ptr, OFASTD) &&
			!F_ISSET(obj_ptr, OINVIS)) {
			strcpy(str, "It contains: ");
			n = list_obj(&str[13], ply_ptr, obj_ptr->first_obj);
			if(n)
				print(fd, "%s.\n", str);
			else {
				if(obj_ptr->special != SP_ENCHT)
					print(fd, "It's empty.\n");
				else
					print(fd, "It contains nothing but the magical fluid.\n");
			}
		}

		if(F_ISSET(obj_ptr, OCONTN) && 
			!F_ISSET(obj_ptr, OFASTD) &&
			F_ISSET(obj_ptr, OINVIS) && 
			(F_ISSET(ply_ptr, PDINVI) ||
			ply_ptr->class >= CARETAKER)) {
				strcpy(str, "It contains: ");
				n = list_obj(&str[13], ply_ptr, obj_ptr->first_obj);
				if(n)
					print(fd, "%s.\n", str);
				return(0);
		}

		if(obj_ptr->type == REPAIR &&
			F_ISSET(ply_ptr, PDINVI) &&
			F_ISSET(obj_ptr, OINVIS) && 
			(F_ISSET(ply_ptr, PDINVI) ||
			ply_ptr->class >= CARETAKER)) {
			    print(fd, "%I %s a ", obj_ptr, 
					F_ISSET(obj_ptr, OSOMEA) ? "are":"is");
				print(fd, "repair tool.\n");
		}

		if(obj_ptr->type == REPAIR && !F_ISSET(obj_ptr, OINVIS)) {
		    print(fd, "%I %s a ", obj_ptr, 
				F_ISSET(obj_ptr, OSOMEA) ? "are":"is");
			print(fd, "repair tool.\n");
        }

		if(obj_ptr->type <= MISSILE && 
			F_ISSET(ply_ptr, PDINVI) &&
			F_ISSET(obj_ptr, OINVIS) && 
			(F_ISSET(ply_ptr, PDINVI) ||
			ply_ptr->class >= CARETAKER)) {
			    print(fd, "%I %s a ", obj_ptr, 
					F_ISSET(obj_ptr, OSOMEA) ? "are":"is");
			    switch(obj_ptr->type) {
			    	case SHARP: print(fd, "sharp"); break;
			    	case THRUST: print(fd, "thrusting"); break;
				    case POLE: print(fd, "pole"); break;
				    case BLUNT: print(fd, "blunt"); break;
				    case MISSILE: print(fd, "missile"); break;
				}
				print(fd, " weapon.\n");
				if(F_ISSET(obj_ptr, OSILVR) && 
					F_ISSET(obj_ptr, OTMPEN))
						print(fd, "It glows with enchanted silver.\n");
		}

		if(obj_ptr->type <= MISSILE && !F_ISSET(obj_ptr, OINVIS)) { 
		    print(fd, "%I %s a ", obj_ptr, 
				F_ISSET(obj_ptr, OSOMEA) ? "are":"is");
			switch(obj_ptr->type) {
				case SHARP: print(fd, "sharp"); break;
				case THRUST: print(fd, "thrusting"); break;
				case POLE: print(fd, "pole"); break;
				case BLUNT: print(fd, "blunt"); break;
				case MISSILE: print(fd, "missile"); break;
			}
			print(fd, " weapon.\n");
			if(F_ISSET(obj_ptr, OSILVR) && F_ISSET(obj_ptr, OTMPEN))
				print(fd, "It glows with enchanted silver.\n");
		}

		if((obj_ptr->type <= MISSILE || 
			obj_ptr->type == ARMOR ||
			obj_ptr->type == REPAIR ||
			obj_ptr->type == LIGHTSOURCE || 
			obj_ptr->type == WAND ||
			obj_ptr->type == KEY) && 
			!F_ISSET(obj_ptr, OCONTN)) { 
				if(obj_ptr->shotscur < 1)
					print(fd, "It's broken or used up.\n");
				else if(obj_ptr->shotscur <= obj_ptr->shotsmax/10) {
					if(F_ISSET(obj_ptr, OSOMEA) || F_ISSET(obj_ptr, ODROPS))
						print(fd, "They look about ready to break.\n");
					else
						print(fd, "It looks about ready to break.\n");
					return(0);
				}
				else if(obj_ptr->shotscur <= obj_ptr->shotsmax/3) {
					if(F_ISSET(obj_ptr, OSOMEA) || F_ISSET(obj_ptr, ODROPS))
						print(fd, "They look well used and worn.\n");
					else
						print(fd, "It looks well used and worn.\n");
					return(0);
				}
				else if(obj_ptr->shotscur <= obj_ptr->shotsmax*2/3) {
					if(F_ISSET(obj_ptr, OSOMEA) || F_ISSET(obj_ptr, ODROPS))
						print(fd, "They look to be in good condition.\n");
					else
						print(fd, "It looks to be in good condition.\n");
					return(0);
				}
				else {
					if(F_ISSET(obj_ptr, OSOMEA) || F_ISSET(obj_ptr, ODROPS))
						print(fd, "They are in excellent condition.\n");
					else
						print(fd, "It's in excellent condition.\n");
				}
		}

		return(0);
	}

/*   LOOK AT MONSTERS IN THE ROOM	*/

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[1],
	   cmnd->val[1]);

	if(crt_ptr) {
		if(F_ISSET(crt_ptr, MHIDDN))
			print(fd, "There, hidden in the darkness...  ");
		print(fd, "You see %1m.\n", crt_ptr);
		if(crt_ptr->description[0])
			print(fd, "%s\n", crt_ptr->description);
		else
			print(fd, "There's not much to see, either.\n");

		if(F_ISSET(ply_ptr, PKNOWA)) {
			if(!F_ISSET(crt_ptr, MNOGEN))
				print(fd, "%s ", F_ISSET(crt_ptr, MMALES) ? "He":"She");
			else
				print(fd, "It ");
			if(crt_ptr->alignment <= -200)
				print(fd, "has a bright red aura!\n");
			else if(crt_ptr->alignment > -200 && crt_ptr->alignment < 0)
				print(fd, "has a red aura.\n");
			else if(crt_ptr->alignment == 0) 
				print(fd, "has a grey aura.\n");
			else print(fd, "has a blue aura.\n");
		}

		if(F_ISSET(crt_ptr, MFROZE))
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "%s has been frozen in place by a freeze spell.\n",
					F_ISSET(crt_ptr, MMALES) ? "He":"She");
			}
			else
				print(fd, "It has been frozen in place by a freeze spell.\n");

		if(F_ISSET(crt_ptr, MSHEAL))
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, 
					"%s has some silver-inflicted, slow mending wounds.\n",
						F_ISSET(crt_ptr, MMALES) ? "He":"She");
			}
			else
				print(fd, 
					"It has some silver-inflicted, slow mending wounds.\n");

		if(F_ISSET(crt_ptr, MEVEYE))
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "%s seems to be suffering from an evil eye spell.\n",
					F_ISSET(crt_ptr, MMALES) ? "He":"She");
			}
			else
				print(fd, "It seems to be suffering from an evil eye spell.\n");

		if(F_ISSET(crt_ptr, MBLIND))
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "%s seems to have been blinded recently.\n",
					F_ISSET(crt_ptr, MMALES) ? "He":"She");
			}
			else
				print(fd, "It seems to have been blinded recently.\n");

		if(crt_ptr->hpcur < (crt_ptr->hpmax*3)/10)
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "%s has some nasty wounds.\n",
					F_ISSET(crt_ptr, MMALES) ? "He":"She");
			}
			else
				print(fd, "It has some nasty wounds.\n");

		if(is_enm_crt(ply_ptr->name, crt_ptr)) 
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "%s is enraged by you.  ",
					F_ISSET(crt_ptr, MMALES) ? "He":"She");
			}
			else
				print(fd, "It is enraged by you.  ");

		if(crt_ptr->first_enm) {
			if(!strcmp(crt_ptr->first_enm->enemy, ply_ptr->name))
				if(!F_ISSET(crt_ptr, MNOGEN)) {
					print(fd, "%s is attacking you.\n",
						F_ISSET(crt_ptr, MMALES) ? "He":"She");
				}
				else
					print(fd, "It is attacking you.\n");
			else {
				if(!F_ISSET(crt_ptr, MNOGEN)) {
					print(fd, "%s is attacking %s.\n",
						F_ISSET(crt_ptr, MMALES) ? "He":"She",
							crt_ptr->first_enm->enemy);
				}
				else
					print(fd, "It is attacking %s.\n",
						crt_ptr->first_enm->enemy);
			}
		}

		consider(ply_ptr, crt_ptr);
		equip_list(fd, crt_ptr);
		return(0);
	}


/*   LOOK AT PLAYERS IN THE ROOM	*/

	cmnd->str[1][0] = up(cmnd->str[1][0]);
	crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply, cmnd->str[1],
		cmnd->val[1]);

	if(crt_ptr) {
		if(!F_ISSET(crt_ptr, PHIDDN) && !F_ISSET(crt_ptr, PCLOAK) ||
			ply_ptr->class >= CARETAKER) {
				print(fd, "You see %s the %s %s.\n", crt_ptr->name,
					race_adj[crt_ptr->race], title_ply(crt_ptr));
			if(crt_ptr->description[0])
				print(fd, "%s\n", crt_ptr->description);
			if(F_ISSET(crt_ptr, PSTRNG))
				print(fd, "%s flexes powerful, rippling muscles.\n", 
					crt_ptr->name);
			if(F_ISSET(crt_ptr, PSHRNK))
				print(fd, "%s is the size of a small mouse.\n", 
					crt_ptr->name);
			if(F_ISSET(crt_ptr, PFLYSP))
				print(fd, "%s is flying.\n", 
					F_ISSET(crt_ptr, PMALES) ? "He":"She");
			if(!F_ISSET(crt_ptr, PFLYSP) && F_ISSET(crt_ptr, PLEVIT))
				print(fd, "%s is levitating.\n", 
					F_ISSET(crt_ptr, PMALES) ? "He":"She");
			if(F_ISSET(crt_ptr, PFROZE))
				print(fd, "%s is frozen in place by a spell.\n", 
					F_ISSET(crt_ptr, PMALES) ? "He":"She");
			if(F_ISSET(ply_ptr, PKNOWA)) {
				print(fd, "%s ", F_ISSET(crt_ptr, PMALES) ? "He":"She");
				if(crt_ptr->alignment < -100)
					print(fd, "is %s and has a red aura.\n",
						F_ISSET(crt_ptr, PCHAOS) ? "chaotic":"lawful");
				else if(crt_ptr->alignment < 101)
					print(fd, "is %s and has a grey aura.\n",
						F_ISSET(crt_ptr, PCHAOS) ? "chaotic":"lawful");
				else print(fd, "is %s and has a blue aura.\n",
					F_ISSET(crt_ptr, PCHAOS) ? "chaotic":"lawful");
			}
			if(crt_ptr->hpcur < (crt_ptr->hpmax * 3)/10) {
				print(fd, "%s has some nasty wounds.\n",
			    	F_ISSET(crt_ptr, PMALES) ? "He":"She");
			}

		equip_list(fd, crt_ptr);
		return(0);
		}
	}
	else
		print(fd, "You don't see that here.\n");

	return(0);
}

/**********************************************************************/
/*							gaze								      */
/**********************************************************************/
/* This function is called when a player tries to look at a room      */
/* beyond the room in which he is standing.                           */

int gaze(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room		*rom_ptr;
	xtag		*xp;
	char		str[30], tempstr[10], namestr[240];
	int			i, fd, n = 0, found = 0, nogaze = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;


	if(ply_ptr->class < CARETAKER) {
		if(ply_ptr->class != MAGE) {
			if(ply_ptr->class != RANGER && ply_ptr->class != DRUID) {
				print(fd, "you are not a ranger nor a druid.\n");
				return(0);
			}	
		}	
	}	

	if(cmnd->num == 1 || cmnd->num > 2) {
		print(fd, "Where are you looking?\n");
		return(0);
	}	

	if(F_ISSET(ply_ptr, PBLIND)) {
		CLS(fd);
		ANSI(fd, RED);
		print(fd, "You're blind!\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		return(0);
	}

	if(ply_ptr->class == MAGE) {
    	if(ply_ptr->mpcur < 10) {
        	print(fd, "Not enough magic points.\n");
        	return(0);
		}
		ply_ptr->mpcur -= 10;	
   	}

	if((ply_ptr->class == RANGER || 
		ply_ptr->class == DRUID) &&
		(ply_ptr->level < 5)) {
        	print(fd, "You must be level five or higher.\n");
        	return(0);
	}

	strcpy(str, cmnd->str[1]);
	strcpy(tempstr, "");
	strcpy(namestr, "You discern:  ");

	if(!strcmp(str, "sw") || !strncmp(str, "southw", 6))
		strcpy(tempstr, "southwest");
	else if(!strcmp(str, "nw") || !strncmp(str, "northw", 6))
		strcpy(tempstr, "northwest");
	else if(!strcmp(str, "se") || !strncmp(str, "southe", 6))
		strcpy(tempstr, "southeast");
	else if(!strcmp(str, "ne") || !strncmp(str, "northe", 6))
		strcpy(tempstr, "northeast");
	else if(!strcmp(str, "lef") || !strncmp(str, "left", 6))
		strcpy(tempstr, "left");
	else if(!strcmp(str, "fo") || !strncmp(str, "forw", 6))
		strcpy(tempstr, "forward");
	else {
		if(strlen(str) == 1) {
			switch(str[0]) {
				case 'n': strcpy(tempstr, "north");		break;
				case 's': strcpy(tempstr, "south");		break;
				case 'e': strcpy(tempstr, "east");  	break;
				case 'w': strcpy(tempstr, "west"); 		break;
				case 'u': strcpy(tempstr, "up");   		break;
				case 'd': strcpy(tempstr, "down");  	break;
				case 'o': strcpy(tempstr, "out");   	break;
				case 'r': strcpy(tempstr, "right");   	break;
				case 'f': strcpy(tempstr, "forward");   break;
				case 'b': strcpy(tempstr, "back");   	break;
			}
		}
	}
	if(strlen(tempstr) == 0)
		strcpy(tempstr, str);

	xp = rom_ptr->first_ext;
	while(xp) {
		if(!strcmp(xp->ext->name, tempstr) && !F_ISSET(xp->ext, XNOSEE)) {
			found = 1;
			break;
		}
		xp = xp->next_tag;
	}

	if(found) {
		load_rom(xp->ext->room, &rom_ptr);
		rom_ptr->rom_num = xp->ext->room;
	}
	else
		nogaze = 1;
		
	ANSI(fd, BOLD);
	ANSI(fd, YELLOW);
	print(fd, "\n\n\n\n");
	print(fd, "You are gazing at the %s exit.\n", tempstr);
	ANSI(fd, NORMAL);
	ANSI(fd, WHITE);
	if(rom_ptr->rom_num != 200 && !nogaze) {
		print(fd, "\n%s\n\n", rom_ptr->name);
		print(fd, "%s\n", rom_ptr->short_desc);
		print(fd, "%s\n", rom_ptr->long_desc);
		for(i = 0; i < Tablesize; i++) {
			if(!Ply[i].ply) continue;
			if(Ply[i].ply->fd < 0) continue;
			if(Ply[i].ply->class >= CARETAKER) continue;
			if(Ply[i].ply->rom_num == rom_ptr->rom_num) {
				strcat(namestr, Ply[i].ply->name);
				strcat(namestr, ", ");
				n++;
			}
		}
		if(n) {
			namestr[strlen(namestr) - 2] = 0;
			print(fd, "%s.\n", namestr);
		}
	}
	else
		print(fd, "\n\n\nThere is only blackness.\n\n\n");

	print(fd, "\n\n\n");
	ANSI(fd, BOLD);
	ANSI(fd, WHITE);
	return(0);
}

/************************************************************************/
/*							describe									*/
/************************************************************************/
/*  This allows a player to set his/her description that is seen when   */
/*  another player looks at them.										*/

int describe_me(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	int 	fd, i;

	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Syntax: description [text|-d]\n.");
		return(0);
	}
	if(!strcmp(cmnd->str[1], "-d")) {
		print(fd, "Description cleared.\n");
		ply_ptr->description[0] = 0;
		return(0);
	}
	i = 0;
	while(!isspace(cmnd->fullstr[i]))
		i++;
	
	strncpy(ply_ptr->description,&cmnd->fullstr[i + 1], 78);
	ply_ptr->description[79] = 0;
	print(fd, "Description set.\n");

	return(0);
}

/************************************************************************/
/*					              set				                    */
/************************************************************************/
/* This function allows a player to set certain one-bit flags.  The     */
/* flags are settings for options that include both brief and verbose   */ 
/* display.  The clear function handles the turning off of these flags. */

int set(ply_ptr, cmnd)
creature    *ply_ptr;
cmd   	  	*cmnd;
{
    int 	fd;
	extra	*extr;
	room	*parent_rom;
    
    fd = ply_ptr->fd;

    if(cmnd->num == 1) {
		flag_list(ply_ptr, cmnd);
    	return(0);
    }

    if(!strcmp(cmnd->str[1], "nobroad")) {
        F_SET(ply_ptr, PNOBRD);
        print(fd, "Broadcast messages disabled.\n");
    }

    else if(!strcmp(cmnd->str[1], "long")) {
        F_CLR(ply_ptr, PNOLDS);
        print(fd, "Long description enabled.\n");
    }

    else if(!strcmp(cmnd->str[1], "nologin")) {
        F_SET(ply_ptr, PNLOGN);
        print(fd, "Login messages disabled.\n");
    } 

    else if(!strcmp(cmnd->str[1], "echo")) {
        F_SET(ply_ptr, PLECHO);
        print(fd, "Communications echo enabled.\n");
    }

    else if(!strcmp(cmnd->str[1], "wimpy")) {
        F_SET(ply_ptr, PWIMPY);
        print(fd, "Wimpy mode enabled.\n");
        ply_ptr->WIMPYVALUE = cmnd->val[1] == 1L ? 10 : cmnd->val[1];
        ply_ptr->WIMPYVALUE = MAX(ply_ptr->WIMPYVALUE, 2);
        print(fd, "Wimpy value set to (%d).\n", ply_ptr->WIMPYVALUE);
    }

    else if(!strcmp(cmnd->str[1], "prompt")) {
        F_SET(ply_ptr, PPROMP);
        print(fd, "Prompt in descriptive format.\n");
    }

    else if(!strcmp(cmnd->str[1], "ansi")) {
        F_SET(ply_ptr, PANSIC);
        print(fd, "Color ANSI mode on.\n");
    }

    else if(!strcmp(cmnd->str[1], "nosummon")) {
        F_SET(ply_ptr, PNOSUM);
        print(fd, "Nosummon flag on.\n");
    }

    else if(!strcmp(cmnd->str[1], "ignore")) {
        F_SET(ply_ptr, PIGNOR);
        print(fd, "Ignore all flag on.\n");
    }

    else if(!strcmp(cmnd->str[1], "auto")) {
        F_SET(ply_ptr, PAUTOA);
        print(fd, "Auto attack mode is enabled.\n");
    }

    else if(!strcmp(cmnd->str[1], "autoattack")) {
        F_SET(ply_ptr, PAUTOA);
        print(fd, "Auto attack mode is enabled.\n");
    }

    else if(!strcmp(cmnd->str[1], "nocop")) {
        F_SET(ply_ptr, PNOCOP);
        print(fd, "Won't allow copulation.\n");
    }

    else if(!strcmp(cmnd->str[1], "split")) {
        F_SET(ply_ptr, PSPLIT);
        print(fd, "You will split any gold with your group.\n");
    }

    else if(!strcmp(cmnd->str[1], "nofollow")) {
        F_SET(ply_ptr, PNOFOL);
        print(fd, "Others cannot follow you.\n");
    }


    else if(!strcmp(cmnd->str[1], "home")) {
		if(((F_ISSET(ply_ptr->parent_rom, RONEPL)) ||
			(F_ISSET(ply_ptr->parent_rom, RTWOPL)) ||
			(F_ISSET(ply_ptr->parent_rom, RTHREE)) ||
			(F_ISSET(ply_ptr->parent_rom, RNHOME)) ||
    		(F_ISSET(ply_ptr->parent_rom, RNOLOG))) && 
			ply_ptr->class < CARETAKER) {
        		print(fd, "You cannot call this place home.\n");
    			return(0);
    	}
		else {
			Ply[fd].extr->home_rom = ply_ptr->parent_rom;
        	print(fd, "This is now your home.\n");
    	}
    }

    else
        print(fd, "That's not something you can set.\n");

    return(0);
}

/************************************************************************/
/*             					 clear  	                            */
/************************************************************************/
/* Like set, this function allows a player to alter the value of a	 	*/
/* particular player flag.                         				  		*/

int clear(ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;
{
    int 	fd;
    
    fd = ply_ptr->fd;

    if(cmnd->num == 1) {
        print(fd, "Type help clear to see a list of flags.\n");
        return(0);
    }

    if(!strcmp(cmnd->str[1], "nobroad")) {
        F_CLR(ply_ptr, PNOBRD);
        print(fd, "Broadcast messages enabled.\n");
    }

    else if(!strcmp(cmnd->str[1], "echo")) {
        F_CLR(ply_ptr, PLECHO);
        print(fd, "Communications echo disabled.\n");
    }

    else if(!strcmp(cmnd->str[1], "nologin")) {
        F_CLR(ply_ptr, PNLOGN);
        print(fd, "Login messages enabled.\n");
    }

    else if(ply_ptr->level > 3 && !strcmp(cmnd->str[1], "long")) {
        F_SET(ply_ptr, PNOLDS);
        print(fd, "Long description disabled.\n");
    }

    else if(ply_ptr->level <= 3 && !strcmp(cmnd->str[1], "long")) {
        print(fd, "You must be above level 3 to clear the long description.\n");
    }

    else if(!strcmp(cmnd->str[1], "wimpy")) {
        F_CLR(ply_ptr, PWIMPY);
        ply_ptr->WIMPYVALUE = 0;
        print(fd, "Wimpy mode disabled.\n");
    }

    else if(!strcmp(cmnd->str[1], "prompt")) {
        F_CLR(ply_ptr, PPROMP);
        print(fd, "Set prompt to restore.\n");
    }

    else if(!strcmp(cmnd->str[1], "ansi")) {
        F_CLR(ply_ptr, PANSIC);
        print(fd, "Color ANSI off.\n");
    }

    else if(!strcmp(cmnd->str[1], "nosummon")) {
        F_CLR(ply_ptr, PNOSUM);
        print(fd, "Nosummon flag off.\n");
    }

    else if(!strcmp(cmnd->str[1], "ignore")) {
        F_CLR(ply_ptr, PIGNOR);
        print(fd, "Ignore all flag off.\n");
    }

    else if(!strcmp(cmnd->str[1], "auto")) {
        F_CLR(ply_ptr, PAUTOA);
        print(fd, "Auto attack mode disabled.\n");
    }

    else if(!strcmp(cmnd->str[1], "autoattack")) {
        F_CLR(ply_ptr, PAUTOA);
        print(fd, "Auto attack mode disabled.\n");
    }

    else if(!strcmp(cmnd->str[1], "nocop")) {
        F_CLR(ply_ptr, PNOCOP);
        print(fd, "Okay slut, copulation allowed.\n");
    }

    else if(!strcmp(cmnd->str[1], "split")) {
        F_CLR(ply_ptr, PSPLIT);
        print(fd, "You will not split any gold with your group.\n");
    }

    else if(!strcmp(cmnd->str[1], "nofollow")) {
        F_CLR(ply_ptr, PNOFOL);
        print(fd, "Other people can follow you.\n");
    }


    else
        print(fd, "Unknown flag.\n");

    return(0);

}

/**********************************************************************/
/*							show								      */
/**********************************************************************/
/* This function allows a player to show another player an item or    */
/* object in his inventory.                                           */

int show(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room		*rom_ptr;
	object		*obj_ptr;
	creature	*crt_ptr;
	char		str[2048];
	int			fd, n, match = 0;
	int			count = 1, num_in_inventory = 0;
	int			inv = 0, wear = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
        print(fd, "Show what?\n");
        return(0);
    }

    if(cmnd->num < 3) {
        print(fd, "Show what to whom?\n");
        return(0);
    }

	cmnd->str[2][0] = up(cmnd->str[2][0]);
	crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply, 
		cmnd->str[2], cmnd->val[2]);

	if(!crt_ptr) {
		cmnd->str[2][0] = low(cmnd->str[2][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[2], cmnd->val[2]);
	}

	if(!crt_ptr) {
		print(fd, "That person is not here.\n");
		return(0);
	}

	if(crt_ptr->type == MONSTER) {
		print(fd, "%M is not interested in looking at your crap.\n", crt_ptr);
		return(0);
	}

	if(F_ISSET(crt_ptr, PBLIND)) {
		print(fd, "%M is blind and cannot see anything.\n", crt_ptr);
		return(0);
	}

	while(obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, 
		cmnd->str[1], count)) {
			count++;
	}
	num_in_inventory = count - 1;

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, 
		cmnd->str[1], cmnd->val[1]);
	if(obj_ptr)
		inv = 1;

	if(!obj_ptr || !cmnd->val[1]) {
		for(n = 0; n < MAXWEAR; n++) {
			if(!ply_ptr->ready[n])
				continue;
			if(EQUAL(ply_ptr->ready[n], cmnd->str[1]))
				match++;
			else continue;
			if(cmnd->val[1] -num_in_inventory == match || !cmnd->val[1]) {
				obj_ptr = ply_ptr->ready[n];
				break;
			}
		}
		if(obj_ptr)
			wear = 1;
	}


	if(!obj_ptr && !num_in_inventory && !match) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
			cmnd->str[1], cmnd->val[1]);
	}
	else if(!obj_ptr && num_in_inventory && !match) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
			cmnd->str[1], cmnd->val[1] - num_in_inventory);
	}
	else if(!obj_ptr && !num_in_inventory && match) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
			cmnd->str[1], cmnd->val[1] - match);
	}
	else if(!obj_ptr && num_in_inventory && match) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
			cmnd->str[1], cmnd->val[1] - num_in_inventory - match);
	}

	if(obj_ptr) {
		if(obj_ptr->special) {
			print(fd, "%M finds the %s difficult to see in this poor light.\n",
				crt_ptr, obj_ptr->name);
			print(crt_ptr->fd,
				"%M tried to show you a %s but the light is too poor.\n", 
					ply_ptr, obj_ptr->name);
			return(0);
		}

		if(F_ISSET(obj_ptr, OINVIS) && (!F_ISSET(crt_ptr, PDINVI))) {
			print(fd, "%M cannot detect an invisible %s.\n", crt_ptr, obj_ptr);
			return(0);
		}

		if(obj_ptr->description[0]) {
			print(fd, "You show a %s to %m.\n", obj_ptr, crt_ptr);
			print(crt_ptr->fd, "%M shows you a %s.\n", ply_ptr, obj_ptr);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"%M shows a %s to %m.", ply_ptr, obj_ptr, crt_ptr);
			print(crt_ptr->fd, "%s\n", obj_ptr->description);
			if((F_ISSET(obj_ptr, OENCHA) || F_ISSET(obj_ptr, OTMPEN)) &&
				obj_ptr->shotscur > 0)
				print(crt_ptr->fd, "It looks enchanted.\n");
			if(F_ISSET(obj_ptr, OPLDGK)) {
				print(crt_ptr->fd, "It belongs to %s kingdom.\n", 
					(F_ISSET(obj_ptr, OKNGDM) ? "Rutger's" : "Ragnar's")); 
			}
		}
		else if(!obj_ptr->description[0]) {
			print(crt_ptr->fd, "You see nothing special about it.\n");
			if(F_ISSET(obj_ptr, OENCHA) || F_ISSET(obj_ptr, OTMPEN))
				print(crt_ptr->fd, "But it does look enchanted.\n");
			if(F_ISSET(obj_ptr, OPLDGK)) {
				print(crt_ptr->fd, "It belongs to %s kingdom.\n", 
					(F_ISSET(obj_ptr, OKNGDM) ? "Rutger's" : "Ragnar's")); 
			}
		}

		if(F_ISSET(obj_ptr, OINVIS) && obj_ptr->description[0]) {
			if((F_ISSET(crt_ptr, PDINVI)) || (crt_ptr->class >= CARETAKER)) {   
				ANSI(crt_ptr->fd, MAGENTA);
				print(crt_ptr->fd, "You detect its invisiblity.\n");
				ANSI(crt_ptr->fd, WHITE);
				if(F_ISSET(obj_ptr, OPLDGK)) {
					print(crt_ptr->fd, "It belongs to %s kingdom.\n", 
						(F_ISSET(obj_ptr, OKNGDM) ? "Rutger's" : "Ragnar's")); 
				}
				ANSI(crt_ptr->fd, WHITE);
			}
		}
		else if(F_ISSET(obj_ptr, OINVIS) && !obj_ptr->description[0]) {
			if((F_ISSET(ply_ptr, PDINVI)) || (ply_ptr->class >= CARETAKER)) {   
				ANSI(crt_ptr->fd, MAGENTA);
				print(crt_ptr->fd, "You detect: %s.\n", obj_ptr->name);
				ANSI(crt_ptr->fd, WHITE);
				print(crt_ptr->fd, "There is nothing special about it.\n");
				if(F_ISSET(obj_ptr, OPLDGK)) {
					print(crt_ptr->fd, "It belongs to %s kingdom.", 
						(F_ISSET(obj_ptr, OKNGDM) ? "Rutger's" : "Ragnar's")); 
				}
				ANSI(crt_ptr->fd, WHITE);
			}
		}

		if(F_ISSET(ply_ptr, PKNOWA)) {
			if(F_ISSET(obj_ptr, OGOODO))
				print(crt_ptr->fd, "It has a blue aura.\n");
			if(F_ISSET(obj_ptr, OEVILO))
				print(crt_ptr->fd, "It has a red aura.\n");
		}

		if(F_ISSET(obj_ptr, OLUCKY)) {
			if(obj_ptr->shotscur < 1)
				print(crt_ptr->fd, "There's no more luck in it.\n");
			else
				print(crt_ptr->fd, "It looks to be a lucky charm.\n");
		} 

		if(F_ISSET(obj_ptr, OCONTN) && 
			F_ISSET(obj_ptr, OFASTD) &&
			!F_ISSET(obj_ptr, OINVIS)) {
				print(crt_ptr->fd, "It's closed with a fastener.\n");
				return(0);
		}

		if(F_ISSET(obj_ptr, OCONTN) && 
			F_ISSET(obj_ptr, OFASTS) &&
			!F_ISSET(obj_ptr, OFASTD) && 
			!F_ISSET(obj_ptr, OINVIS)) {
				print(crt_ptr->fd, "It's open, but can be fastened.\n");
		}
		if(F_ISSET(obj_ptr, OCONTN) && 
			F_ISSET(obj_ptr, OFASTD) &&
			F_ISSET(obj_ptr, OINVIS) && 
			(F_ISSET(ply_ptr, PDINVI) ||
			ply_ptr->class >= CARETAKER)) {
				print(crt_ptr->fd, "It's closed with a fastener.\n");
				return(0);
		}

		if(F_ISSET(obj_ptr, OCONTN) && 
			F_ISSET(obj_ptr, OFASTS) &&
			F_ISSET(obj_ptr, OINVIS) && 
			(F_ISSET(ply_ptr, PDINVI) ||
			ply_ptr->class >= CARETAKER)) {
				print(crt_ptr->fd, "It's open, but can be fastened.\n");
		}

		if(F_ISSET(obj_ptr, OCONTN) && 
			!F_ISSET(obj_ptr, OFASTD) &&
			!F_ISSET(obj_ptr, OINVIS)) {
			strcpy(str, "It contains: ");
			n = list_obj(&str[13], ply_ptr, obj_ptr->first_obj);
			if(n)
				print(crt_ptr->fd, "%s.\n", str);
			else {
				if(obj_ptr->special != SP_ENCHT)
					print(crt_ptr->fd, "It's empty.\n");
				else
					print(crt_ptr->fd, 
						"It contains nothing but the magical fluid.\n");
			}
		}

		if(F_ISSET(obj_ptr, OCONTN) && 
			!F_ISSET(obj_ptr, OFASTD) &&
			F_ISSET(obj_ptr, OINVIS) && 
			(F_ISSET(ply_ptr, PDINVI) ||
			ply_ptr->class >= CARETAKER)) {
				strcpy(str, "It contains: ");
				n = list_obj(&str[13], ply_ptr, obj_ptr->first_obj);
				if(n)
					print(crt_ptr->fd, "%s.\n", str);
				return(0);
		}

		if(obj_ptr->type <= MISSILE && 
			F_ISSET(ply_ptr, PDINVI) &&
			F_ISSET(obj_ptr, OINVIS) && 
			(F_ISSET(ply_ptr, PDINVI) ||
			ply_ptr->class >= CARETAKER)) {
			    print(crt_ptr->fd, "%I is a ", obj_ptr);
			    switch(obj_ptr->type) {
			    	case SHARP: print(crt_ptr->fd, "sharp"); break;
			    	case THRUST: print(crt_ptr->fd, "thrusting"); break;
				    case POLE: print(crt_ptr->fd, "pole"); break;
				    case BLUNT: print(crt_ptr->fd, "blunt"); break;
				    case MISSILE: print(crt_ptr->fd, "missile"); break;
				}
				print(crt_ptr->fd, " weapon.\n");
				if(F_ISSET(obj_ptr, OSILVR) && 
					F_ISSET(obj_ptr, OTMPEN))
						print(crt_ptr->fd, "It glows with enchanted silver.\n");
		}

		if(obj_ptr->type <= MISSILE && !F_ISSET(obj_ptr, OINVIS)) { 
			print(crt_ptr->fd, "%I is a ", obj_ptr);
			switch(obj_ptr->type) {
				case SHARP: print(crt_ptr->fd, "sharp"); break;
				case THRUST: print(crt_ptr->fd, "thrusting"); break;
				case POLE: print(crt_ptr->fd, "pole"); break;
				case BLUNT: print(crt_ptr->fd, "blunt"); break;
				case MISSILE: print(crt_ptr->fd, "missile"); break;
			}
			print(crt_ptr->fd, " weapon.\n");
			if(F_ISSET(obj_ptr, OSILVR) && F_ISSET(obj_ptr, OTMPEN))
				print(crt_ptr->fd, "It glows with enchanted silver.\n");
		}

		if((obj_ptr->type <= MISSILE || obj_ptr->type == ARMOR ||
			obj_ptr->type == LIGHTSOURCE || obj_ptr->type == WAND ||
			obj_ptr->type == KEY) && !F_ISSET(obj_ptr, OCONTN)) { 
				if(obj_ptr->shotscur < 1)
					print(crt_ptr->fd, "It's broken or used up.\n");
				else if(obj_ptr->shotscur <= obj_ptr->shotsmax/10) {
					if(F_ISSET(obj_ptr, OSOMEA) || F_ISSET(obj_ptr, ODROPS))
						print(crt_ptr->fd, "They look about ready to break.\n");
					else
						print(crt_ptr->fd, "It looks about ready to break.\n");
					return(0);
				}
				else if(obj_ptr->shotscur <= obj_ptr->shotsmax/3) {
					if(F_ISSET(obj_ptr, OSOMEA) || F_ISSET(obj_ptr, ODROPS))
						print(crt_ptr->fd, "They look well used and worn.\n");
					else
						print(crt_ptr->fd, "It looks well used and worn.\n");
					return(0);
				}
				else if(obj_ptr->shotscur <= obj_ptr->shotsmax*2/3) {
					if(F_ISSET(obj_ptr, OSOMEA) || F_ISSET(obj_ptr, ODROPS))
						print(crt_ptr->fd, 
							"They look to be in good condition.\n");
					else
						print(crt_ptr->fd, 
							"It looks to be in good condition.\n");
					return(0);
				}
				else {
					if(F_ISSET(obj_ptr, OSOMEA) || F_ISSET(obj_ptr, ODROPS))
						print(crt_ptr->fd, 
							"They are in excellent condition.\n");
					else
						print(crt_ptr->fd, "It's in excellent condition.\n");
				}
		}

		return(0);
	}
}

