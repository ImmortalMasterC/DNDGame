/*
 * MAGIC4.C:
 *
 *	Additional spell-casting routines.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"


/**************************************************************************/
/*							detectinvis							      	  */
/**************************************************************************/
/* This function allows players to cast the detect-invisible spell which  */
/* allows the spell-castee to see invisible items. 			 			  */

int detectinvis(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		t;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	t = time(0);


	if(ply_ptr->mpcur < 5 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	if(ply_ptr->mpcur < 10 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(!S_ISSET(ply_ptr, SDINVI) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(spell_fail(ply_ptr, how)) {
		if(how == CAST && ply_ptr->class != MAGE)
			ply_ptr->mpcur -= 10;
			return(0);
	}

	/* Cast detect-invisibility on self */
	if(cmnd->num == 2) {
		ply_ptr->lasttime[LT_DINVI].ltime = t;
		if(how == CAST) {
			ply_ptr->lasttime[LT_DINVI].interval = MAX(300, 1200 + 
				bonus[ply_ptr->intelligence] * 600);
			if(ply_ptr->class == MAGE)
				ply_ptr->lasttime[LT_DINVI].interval += 60 * ply_ptr->level;
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 5;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 10;
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
	            print(fd,
					"The magic in the air increase the power of your spell.\n");
	            ply_ptr->lasttime[LT_DINVI].interval += 600L;
	        }                                
		}
		else
			ply_ptr->lasttime[LT_DINVI].interval = 1200;

		if(how == CAST || how == SCROLL || how == WAND) {
			ANSI(fd, MAGENTA);
			print(fd,"Detect-invisible spell cast.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts detect-invisible on %sself.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
		}
		else if(how == POTION)
			print(fd, "Your eyes tingle as you see things more vividly.\n");

		F_SET(ply_ptr, PDINVI);

		return(1);
	}

	/* Cast detect-invisibility on another player */
	else {
		if(how == POTION) {
			print(fd, "You can only use a potion on yourself.\n");
			return(0);
		}

		cmnd->str[2][0] = up(cmnd->str[2][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);

		if(!crt_ptr) {
			print(fd, "That player is not here.\n");
			return(0);
		}

		crt_ptr->lasttime[LT_DINVI].ltime = t;
		if(how == CAST) {
			crt_ptr->lasttime[LT_DINVI].interval = MAX(300, 1200 + 
				bonus[ply_ptr->intelligence] * 600);
			if(ply_ptr->class == MAGE)
				crt_ptr->lasttime[LT_DINVI].interval += 60 * ply_ptr->level;
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 5;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 10;
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
	            print(fd,
					"The magic in the air increase the power of your spell.\n");
    	        crt_ptr->lasttime[LT_DINVI].interval += 600L;
    	    }                                
		}
		else
			crt_ptr->lasttime[LT_DINVI].interval = 1200;

		F_SET(crt_ptr, PDINVI);

		if(how == CAST || how == SCROLL || how == WAND) {
			print(fd, "Detect-invisible cast on %m.\n", crt_ptr);
			ANSI(crt_ptr->fd, MAGENTA); 
			print(crt_ptr->fd, "%M casts a detect-invisible spell on you.\n",
				ply_ptr);
			ANSI(crt_ptr->fd, BOLD); 
			ANSI(crt_ptr->fd, WHITE); 
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts detect-invisible on %m.", ply_ptr, crt_ptr);
			return(1);
		}
	}

	return(1);
}

/*************************************************************************/
/*							detectmagic							      	 */
/*************************************************************************/
/* This function allows players to cast the detect-magic spell which 	 */
/* allows the spell-castee to see magic items. 			     			 */

int detectmagic(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		t;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	t = time(0);

	
	if(ply_ptr->mpcur < 5 && how == CAST && ply_ptr->class == ALCHEMIST) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	else {	
		if(ply_ptr->mpcur < 10 && how == CAST) {
			print(fd, "Not enough magic points.\n");
			return(0);
		}
	}

	if(!S_ISSET(ply_ptr, SDMAGI) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	/* Cast detect-magic on self */
	if(cmnd->num == 2) {

		ply_ptr->lasttime[LT_DMAGI].ltime = t;
		if(how == CAST) {
			ply_ptr->lasttime[LT_DMAGI].interval = MAX(300, 1200 + 
				bonus[ply_ptr->intelligence] * 600);
			if(ply_ptr->class == ALCHEMIST)
				ply_ptr->mpcur -= 5;
			if(ply_ptr->class != ALCHEMIST)
				ply_ptr->mpcur -= 10;
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
            	print(fd,
					"The magic in the air increase the power of your spell.\n");
            	ply_ptr->lasttime[LT_DMAGI].interval += 600L;
        	}                                
		}
		else
			ply_ptr->lasttime[LT_DMAGI].interval = 1200;

		if(how == CAST || how == SCROLL || how == WAND) {
			print(fd,"Detect-magic spell cast.\n");
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts detect-magic on %sself.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
		}
		else if(how == POTION)
			print(fd, "Your eyes feel funny and you can see differently.\n");

		F_SET(ply_ptr, PDMAGI);

		return(1);
	}

	/* Cast detect-magic on another player */
	else {
		if(how == POTION) {
			print(fd, "You can only use a potion on yourself.\n");
			return(0);
		}

		cmnd->str[2][0] = up(cmnd->str[2][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);

		if(!crt_ptr) {
			print(fd, "That person is not here.\n");
			return(0);
		}

		crt_ptr->lasttime[LT_DMAGI].ltime = t;
		if(how == CAST) {
			crt_ptr->lasttime[LT_DMAGI].interval = MAX(300, 1200 + 
				bonus[ply_ptr->intelligence] * 600);
			if(ply_ptr->class == MAGE)
				crt_ptr->lasttime[LT_DMAGI].interval += 60 * ply_ptr->level;
			if(ply_ptr->class == MAGE || ply_ptr->class == ALCHEMIST)
				ply_ptr->mpcur -= 2;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 10;
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
	            print(fd,
					"The magic in the air increase the power of your spell.\n");
	            crt_ptr->lasttime[LT_DMAGI].interval += 600L;
	        }                                
		}
		else
			crt_ptr->lasttime[LT_DMAGI].interval = 1200;

		F_SET(crt_ptr, PDMAGI);

		if(how == CAST || how == SCROLL || how == WAND) {
			print(fd, "Detect-magic cast on %m.\n", crt_ptr);
			print(crt_ptr->fd, 
				"%M casts a detect-magic spell on you.\n", ply_ptr);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts detect-magic on %m.", ply_ptr, crt_ptr);
			return(1);
		}
	}

	return(1);
}

/************************************************************************/
/*								teleport			 			     	*/
/************************************************************************/
/* This function allows a player to teleport himself or another player  */
/* to another room randomly.					       					*/
/* An added feature - player may teleport to a home room. - Styx		*/

int teleport(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr, *new_rom, *home_rom;
	int			fd, rtn, rom_num;
	extra		*extr;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->type == PLAYER) { 
		home_rom = Ply[fd].extr->home_rom;

		if(ply_ptr->class == MAGE) {
			if((ply_ptr->mpcur < 15) && how == CAST) {
				print(fd, "Not enough magic points.\n");
				return(0);
			}
		}
		else {
			if((ply_ptr->mpcur < 20) && how == CAST) {
				print(fd, "Not enough magic points.\n");
				return(0);
			}
		}
	}

	if(ply_ptr->type != PLAYER) {
		if((ply_ptr->mpcur < 20) && how == CAST) {
				return(0);
		}
	}

	if(!S_ISSET(ply_ptr, STELEP) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(F_ISSET(rom_ptr, RNOTEL)) {
		print(fd, "SSSZZZzzzzz..... POP!  The spell fizzles.\n");
		if(how == CAST) {
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 15;
			else
				ply_ptr->mpcur -= 20;
		}
		return(0);
	}

	if(spell_fail(ply_ptr, how)) {
		if(how == CAST && ply_ptr->class != MAGE)
			ply_ptr->mpcur -= 20;
		return(0);
	}

	/* Cast teleport on self */
	if(cmnd->num == 2) {
		if(how == CAST) {
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 15;
			else
				ply_ptr->mpcur -= 20;
		}

		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M disappears in a swirling vortex.", ply_ptr);

		if(how == CAST || how == SCROLL) {
			ANSI(fd, YELLOW);
			print(fd, "The space around you spins and turns topsy-turvy!.\n");
			ANSI(fd, WHITE);
		}

		do {
			rom_num = mrand(1, RMAX - 1);
			rtn = load_rom(rom_num, &new_rom);
			if(rtn > -1 && F_ISSET(new_rom, RNOTEL))
				rtn = -1;
			else if(rtn > -1 && new_rom->lolevel > ply_ptr->level)
				rtn = -1;
			else if(rtn > -1 && ply_ptr->level > new_rom->hilevel &&
				new_rom->hilevel)
					rtn = -1;
		} while(rtn < 0);

		del_ply_rom(ply_ptr, rom_ptr);
		add_ply_rom(ply_ptr, new_rom);

		return(1);
	}

	/* Cast teleport on another player */
	else {
		if(how == POTION) {
			print(fd, "You can only use a potion on yourself.\n");
			return(0);
		}

		if(!strcmp(cmnd->str[2], "home")) {
			del_ply_rom(ply_ptr, rom_ptr);
			add_ply_rom(ply_ptr, home_rom);
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 15;
			else
				ply_ptr->mpcur -= 20;
			ANSI(fd, YELLOW);
			print(fd, "Be it ever so humble, there's no place like home.\n");
			ANSI(fd, WHITE);
			return(1);
		}

		cmnd->str[2][0] = up(cmnd->str[2][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);

		if(!crt_ptr) {
			print(fd, "That player is not here.\n");
			return(0);
		}
		
		if(ply_ptr->level < 5) {
			print(fd, "ZZZzzz:...ppp.    The spell fizzles.\n");
			print(fd, 
				"Maybe when you have more experience, the spell will work.\n");
			if(how == CAST) {
				if(ply_ptr->class == MAGE)
					ply_ptr->mpcur -= 15;
				else
					ply_ptr->mpcur -= 20;
			}
			return(0);	
		}
		

		if(F_ISSET(crt_ptr, PRMAGI) && (mrand(1, 60) +
			(ply_ptr->level-crt_ptr->level) * 10) > 80) {
				print(fd, "%m is under a resist-magic spell.\n", crt_ptr);
				print(fd, "Your magic is too weak to teleport %s.\n",
					F_ISSET(crt_ptr, PMALES) ? "him":"her");
				print(crt_ptr->fd, 
					"%M tried to cast teleport on you, but failed.\n", ply_ptr);
				if(how == CAST) {
					if(ply_ptr->class == MAGE)
						ply_ptr->mpcur -= 15;
					else
						ply_ptr->mpcur -= 20;
				}
			return(0);
		}

		if(how == CAST) {
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 15;
			else
				ply_ptr->mpcur -= 20;
		}
	
		if(how == CAST || how == SCROLL || how == WAND) {
			print(fd, "Teleport cast on %m.\n", crt_ptr);
			print(crt_ptr->fd, "%M casts a teleport spell on you.\n", ply_ptr);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts teleport on %m.", ply_ptr, crt_ptr);

			do {
				rom_num = mrand(1, RMAX - 1);
				rtn = load_rom(rom_num, &new_rom);
				if(rtn > -1 && F_ISSET(new_rom, RNOTEL))
					rtn = -1;
				else if(rtn > -1 && 
					new_rom->lolevel > ply_ptr->level)
					rtn = -1;
				else if(rtn > -1 && 
					ply_ptr->level > new_rom->hilevel && new_rom->hilevel)
					rtn = -1;
			} while(rtn < 0);

			del_ply_rom(crt_ptr, rom_ptr);
			add_ply_rom(crt_ptr, new_rom);

			return(1);
		}
	}

	return(1);
}

/************************************************************************/
/*								enchant							        */
/************************************************************************/
/* This function allows mages to enchant weapons at a cost of 25 magic  */
/* points.  They can only do it 3 times a day.			       			*/

int enchant(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	object		*obj_ptr;
	room		*rom_ptr;
	int			fd, adj;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(how == CAST && ply_ptr->class != MAGE && ply_ptr->class < CARETAKER) {
		print(fd, "Only mages are able to cast an enchant spell.\n");
		return(0);
	}

	if(ply_ptr->mpcur < 25 && how == CAST) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(!S_ISSET(ply_ptr, SENCHA) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(cmnd->num < 3) {
		print(fd, "Cast the spell on what?\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[2], cmnd->val[2]);

	if(!obj_ptr) {
		print(fd, "You don't have that in your inventory.\n");
		return(0);
	}

	if(F_ISSET(obj_ptr, OENCHA)) {
		print(fd, "That object is already enchanted.\n");
		return(1);
	}

	if(F_ISSET(obj_ptr, OSILVR) && obj_ptr->type < 5) {
		print(fd, "Silver weapons can only be enchanted by an alchemist.\n");
		return(1);
	}

	if(how == CAST) {
		if(!dec_daily(&ply_ptr->daily[DL_ENCHA]) && 
			ply_ptr->class < CARETAKER) {
				print(fd, "You have exhausted your enchantments for today.\n");
				return(0);
		}
		ply_ptr->mpcur -= 25;
	}

	if((ply_ptr->class == MAGE || ply_ptr->class >= CARETAKER) && 
		how == CAST) {
			adj = (ply_ptr->level - 5)/5 + 1;
			adj = MIN(3, adj);
	}
	else
		adj = 1;

	obj_ptr->adjustment = MAX(adj, obj_ptr->adjustment);

	if(obj_ptr->type == ARMOR && obj_ptr->wearflag == BODY)
		obj_ptr->armor += adj * 2;
	else if(obj_ptr->type == ARMOR)
		obj_ptr->armor += adj;
	else if(obj_ptr->type <= MISSILE) {
		obj_ptr->shotsmax += adj * 10;
		obj_ptr->shotscur += adj * 10;
		obj_ptr->pdice += adj;
	}
	obj_ptr->value += 500 * adj;

	F_SET(obj_ptr, OENCHA);

	print(fd, "%I begins to sparkle.\n", obj_ptr);
	broadcast_rom(fd, rom_ptr->rom_num, "%M enchants %1i.", ply_ptr, obj_ptr);

	return(1);
}
