/*
 * COMMAND5.C:
 *
 *  Additional user routines.
 *
 *  Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <ctype.h>
#include <string.h>

/************************************************************************/
/* 			search   			                */
/************************************************************************/
/* This function allows a player to search a room for hidden objects,	*/
/* exits, monsters and players.                	       			*/

int search(ply_ptr, cmnd)
creature    *ply_ptr;
cmd         *cmnd;
{
    room    *rom_ptr;
    xtag    *xp;
    otag    *op;
    ctag    *cp;
    long    i, t;
    int     fd, chance, found = 0;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    chance = 15 + 5 * bonus[ply_ptr->piety] + ply_ptr->level * 2;
    chance = MIN(chance, 90);
    if(ply_ptr->class == RANGER)
        chance += ply_ptr->level * 10;
    if(F_ISSET(ply_ptr, PBLIND))
	chance = MIN(chance, 20);
    if(ply_ptr->class >= CARETAKER)
        chance = 100;

    t = time(0);
    i = LT(ply_ptr, LT_SERCH);

    if(t < i) {
        please_wait(fd, i - t);
        return(0);
    }

    F_CLR(ply_ptr, PHIDDN);

    ply_ptr->lasttime[LT_SERCH].ltime = t;
    if(ply_ptr->class == RANGER || ply_ptr->class >= CARETAKER)
        ply_ptr->lasttime[LT_SERCH].interval = 3;
    else
        ply_ptr->lasttime[LT_SERCH].interval = 7;

    xp = rom_ptr->first_ext;
    while(xp) {
        if(F_ISSET(xp->ext, XSECRT) && mrand(1, 100) <= chance) 
           if((F_ISSET(xp->ext, XINVIS) && F_ISSET(ply_ptr, PDINVI) ||
            !F_ISSET(xp->ext, XINVIS)) && !F_ISSET(xp->ext, XNOSEE)) {
            found++;
	    	ANSI(fd, YELLOW);
            print(fd, "You found an exit: %s.\n", xp->ext->name);
           	ANSI(fd, BOLD);
	    	ANSI(fd, WHITE);
        }
        if(F_ISSET(xp->ext, XINVIS) && (F_ISSET(ply_ptr, PDINVI) ||
			ply_ptr->class >= CARETAKER)) {
				ANSI(fd, MAGENTA);
	    		print(fd,"You detect an invisible exit:  %s.\n",
					xp->ext->name);
            	ANSI(fd, BOLD);
	    		ANSI(fd, WHITE);
	    found++;
	    }
        xp = xp->next_tag;
    }

    op = rom_ptr->first_obj;
    while(op) {
        if(F_ISSET(op->obj, OHIDDN) && mrand(1, 100) <= chance) 
        if(F_ISSET(op->obj, OINVIS) && F_ISSET(ply_ptr, PDINVI) ||
            !F_ISSET(op->obj, OINVIS)) {
            found++;
            print(fd, "You found %1i.\n", op->obj);
        }

        if(F_ISSET(op->obj, OINVIS) && F_ISSET(ply_ptr, PDINVI)) { 
	    ANSI(fd, MAGENTA);
	    print(fd, "You detect %1i.  It's invisible.\n", op->obj);
            ANSI(fd, BOLD);
	    ANSI(fd, WHITE);
	    found++;
	    }
        op = op->next_tag;
    }

    cp = rom_ptr->first_ply;
    while(cp) {
        if(F_ISSET(cp->crt, PHIDDN) && !F_ISSET(cp->crt, PDMINV) &&
           mrand(1, 100) <= chance) 
        if(F_ISSET(cp->crt, PINVIS) && F_ISSET(ply_ptr, PDINVI) ||
            !F_ISSET(cp->crt, PINVIS)) {
            found++;
            print(fd, "You found %s hiding.\n", cp->crt->name);
        }

        if(F_ISSET(cp->crt, PINVIS) && F_ISSET(ply_ptr, PDINVI) &&
		cp->crt->name != ply_ptr->name) {
	    ANSI(fd, MAGENTA);
	    print(fd, "You detect %s hiding and invisible.\n",
	               cp->crt->name);
            ANSI(fd, BOLD);
	    ANSI(fd, WHITE);
	    found++;
	    }
        cp = cp->next_tag;
    }

    cp = rom_ptr->first_mon;
    while(cp) {
        if(F_ISSET(cp->crt, MHIDDN) && mrand(1, 100) <= chance) 
        	if(F_ISSET(cp->crt, MINVIS) && F_ISSET(ply_ptr, PDINVI) ||
            	!F_ISSET(cp->crt, MINVIS)) {
            	found++;
            	print(fd, "You found %1m hiding.\n", cp->crt);
        	}

        if(F_ISSET(cp->crt, MINVIS) && F_ISSET(ply_ptr, PDINVI)) {
	    ANSI(fd, MAGENTA);
            print(fd, "You found %1m hiding and invisible.\n", cp->crt);
            ANSI(fd, BOLD);
	    ANSI(fd, WHITE);
	    found++;
	    }	
        cp = cp->next_tag;
    }

	if(!F_ISSET(ply_ptr, PDMINV)) {
    	broadcast_rom(fd, ply_ptr->rom_num, "%M searches the area.", ply_ptr);
    	if(found)
        	broadcast_rom(fd, ply_ptr->rom_num, "%s found something!", 
				F_ISSET(ply_ptr, MMALES) ? "He":"She");
    	else
        	print(fd, "You didn't find anything.\n");
    }

    return(0);

}

/************************************************************************/
/*			      hide  	                  	        */
/************************************************************************/
/* This command allows a player to try and hide himself in the shadows 	*/
/* or it can be used to hide an object in a room.              		*/

int hide(ply_ptr, cmnd)
creature    *ply_ptr;
cmd    	    *cmnd;
{
    room    *rom_ptr;
    object  *obj_ptr;
    long    i, t;
    int	    fd, chance;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    i = LT(ply_ptr, LT_HIDES);
    t = time(0);

    if(i > t) {
        please_wait(fd, i - t);
        return(0);
    }

    ply_ptr->lasttime[LT_HIDES].ltime = t;
    ply_ptr->lasttime[LT_HIDES].interval = (ply_ptr->class == THIEF ||
        ply_ptr->class == ASSASSIN || ply_ptr->class == RANGER) ? 3:15;

    if(cmnd->num == 1) {
        if(ply_ptr->class == THIEF || ply_ptr->class == ASSASSIN)
            chance = MIN(90, 5 + 10 * ply_ptr->level + 
                3 * bonus[ply_ptr->dexterity]);

        else if(ply_ptr->class == RANGER || ply_ptr->class == DRUID) 
			chance = MIN(90, 5 + 6 * ply_ptr->level +
                3 * bonus[ply_ptr->dexterity]);
        else
            chance = MIN(90, 5 + 2 * ply_ptr->level +
                3 * bonus[ply_ptr->dexterity]);

		if(F_ISSET(ply_ptr, PBLIND))
	    	chance = MIN(chance, 20);
		
	    if(Ply[ply_ptr->fd].extr->luck > 45)
			chance += Ply[ply_ptr->fd].extr->luck/20;

		if(ply_ptr->class <= CARETAKER)
        	print(fd, "You attempt to hide in the shadows.\n");

		if(ply_ptr->class >= CARETAKER) {
            F_SET(ply_ptr, PHIDDN);
	    	print(fd, "You slip into the shadows unnoticed.\n");
	        return(0);
		}

        if(mrand(1, 100) <= chance) {
            F_SET(ply_ptr, PHIDDN);
	    		print(fd, "You slip into the shadows unnoticed.\n");
	    		if(Ply[ply_ptr->fd].extr->luck > 55)
					print(fd,"Your good luck is serving you well.\n") ;
		}
        else {
            F_CLR(ply_ptr, PHIDDN);
            broadcast_rom(fd, ply_ptr->rom_num,
            	"%M tries to hide in the shadows.", ply_ptr);
	   		if(Ply[ply_ptr->fd].extr->luck < 45)
				print(fd,"Your bad luck is hindering you.\n") ;
        }

        return(0);
    }

    obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
               cmnd->str[1], cmnd->val[1]);

    if(!obj_ptr) {
        print(fd, "I don't see that here.\n");
        return(0);
    }

    if(F_ISSET(obj_ptr, ONOTAK)) {
		print(fd,"You cannot hide that.\n");
		return (0);
   }

    if(ply_ptr->class == THIEF || ply_ptr->class == ASSASSIN)
        chance = MIN(90, 10 + 5 * ply_ptr->level + 
            5 * bonus[ply_ptr->dexterity]);

    else if(ply_ptr->class == RANGER)
        chance = 5 + 9 * ply_ptr->level +
            3 * bonus[ply_ptr->dexterity];
    else
        chance = MIN(90, 5 + 3 * ply_ptr->level + 
            3 * bonus[ply_ptr->dexterity]);

	if(ply_ptr->class < CARETAKER) {
    	print(fd, "You attempt to hide it.\n");
    	broadcast_rom(fd, ply_ptr->rom_num, "%M attempts to hide %1i.", 
			ply_ptr, obj_ptr);
	}

	if(ply_ptr->class >= CARETAKER) {
        F_SET(obj_ptr, OHIDDN);
		print(fd, "You hide %1i.\n", obj_ptr);
		return(0);
	}

    if(mrand(1, 100) <= chance)
        F_SET(obj_ptr, OHIDDN);
    else
        F_CLR(obj_ptr, OHIDDN);

    return(0);

}

/************************************************************************/
/*			Player Selectable Flags				*/
/************************************************************************/
/*  Display information on flags to given player.			*/

int flag_list(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;
{
	char	str[1024], temp[20];
	int	i, fd;

	str[0] = 0;
	temp[0] = 0;

	fd = ply_ptr->fd;

		ANSI(fd, GREEN);
		print(fd, "Flags currently set:\n");
		print(fd, "----------------------------------------------\n");
		if(F_ISSET(ply_ptr, PNOBRD)) strcat(str, "\tNo broadcast\n");
		if(F_ISSET(ply_ptr, PNOLDS)) strcat(str, "\tNo room description\n");
		if(F_ISSET(ply_ptr, PLECHO)) strcat(str, "\tCommunications echo\n");
		if(F_ISSET(ply_ptr, PNLOGN)) strcat(str, "\tNo Login messages\n");
		if(F_ISSET(ply_ptr, PPROMP)) strcat(str, "\tPrompt\n");
		if(F_ISSET(ply_ptr, PANSIC)) strcat(str, "\tColor output mode\n");
		if(F_ISSET(ply_ptr, PIGNOR)) strcat(str, "\tIgnore all sends\n");
		if(F_ISSET(ply_ptr, PNOSUM)) strcat(str, "\tNo summon\n");
        if(F_ISSET(ply_ptr, PAUTOA)) strcat(str, "\tAuto attack\n"); 
		if(F_ISSET(ply_ptr, PNOCOP)) strcat(str, "\tNo copulation\n");
		if(F_ISSET(ply_ptr, PSPLIT)) strcat(str, "\tAuto split gold\n");
		if(F_ISSET(ply_ptr, PNOFOL)) strcat(str, "\tNo followers\n");
		if(F_ISSET(ply_ptr, PWIMPY)) {
			sprintf(temp, "\tWimpy%d\n", ply_ptr->WIMPYVALUE);
			strcat(str, temp);
		}

		print(fd,"%s\n", str);

        print(fd, "Type 'help set' to see a complete list of flags.\n");
		ANSI(fd, WHITE);
    return(0);
}

/**************************************************************************/
/* 		             quit    			                  */
/**************************************************************************/
/* This function checks to see if a player is allowed to quit yet.  It    */
/* checks to make sure the player isn't in the middle of combat, and if   */
/* so, the player is not allowed to quit (and 0 is returned).             */
/* A player is not allowed to quit with more than 80 items in inventory,  */
/* as more than that would be lost. */

int quit(ply_ptr, cmnd)
creature    *ply_ptr;
cmd         *cmnd;
{
    long    i, t;
    int     fd, n, wearnumb = 0, inventory = 0;

    otag *op, *cop, *obj;
    object *obj_ptr, *obj_ptr2, *cnt_ptr;
    fd = ply_ptr->fd;

	if(ply_ptr->class == DM) {
    	update_ply(ply_ptr);
   		return(DISCONNECT);
	}

    t = time(0);
    i = LT(ply_ptr, LT_ATTCK) + 20;
	
    if(t < i) {
        please_wait(fd, i - t);
        return(0);
    }
    i = LT(ply_ptr, LT_SPELL) + 20;
    
    if(t < i) {
        please_wait(fd, i - t);
        return(0);
    }   

	for(n = 0; n < MAXWEAR; n++) {
		if(ply_ptr->ready[n])
			wearnumb++;
	}

	if((player_inv(ply_ptr) + wearnumb) > 80) {
		print(fd, "You have more than 80 items in your inventory.\n");	
        return(0);
	}
		
    update_ply(ply_ptr);
    return(DISCONNECT);
}


/*************************************************************************/
/*			steal					         */
/*************************************************************************/
/* This function allows a thief to steal an item from a monster within   */
/* the game.  Only thieves may steal, and the monster must have the      */
/* object which is being stolen within his inventory.		 	 */

int steal(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	object		*obj_ptr;
	long		i, t;
	int		fd, chance;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Steal what?\n");
		return(0);
	}

	if(cmnd->num < 3) {
		print(fd, "Steal what from whom?\n");
		return(0);
	}

	if(ply_ptr->class != THIEF && ply_ptr->class < CARETAKER) {
		print(fd, "Only thieves may steal.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	i = LT(ply_ptr, LT_STEAL);
	t = time(0);

	if(t < i) {
		please_wait(fd, i - t);
		return(0);
	}

	if(F_ISSET(ply_ptr, PINVIS)) {
		F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
		print(fd, "Your invisibility fades.\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
	}

	ply_ptr->lasttime[LT_STEAL].ltime = t;
	ply_ptr->lasttime[LT_STEAL].interval = 5;

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[2], cmnd->val[2]);

	if(!crt_ptr) {
		cmnd->str[2][0] = up(cmnd->str[2][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);

		if(!crt_ptr || crt_ptr == ply_ptr || F_ISSET(ply_ptr, PBLIND)) {
			print(fd, "I don't see that here.\n");
			return(0);
		}
	}

	if(crt_ptr->type == MONSTER) {
		if(F_ISSET(crt_ptr, MUNKIL)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
			print(fd, "You can do no harm to %s.\n",
				F_ISSET(crt_ptr, MMALES) ? "him":"her");
			}
			else
			print(fd, "You can do no harm to it.\n");
			return(0);
		}
		if(is_enm_crt(ply_ptr->name, crt_ptr)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "Not while %s's attacking you.\n",
			    	F_ISSET(crt_ptr, MMALES) ? "he":"she");
			}
			else
				print(fd, "Not while it's attacking you.\n");

			return(0);
		}
	}
	else {
		if(F_ISSET(rom_ptr, RNOKIL) && ply_ptr->class < DM) {
			print(fd, "Stealing is not allowed here.\n");
			return(0);
		}

		if((!F_ISSET(ply_ptr, PPLDGK) || !F_ISSET(crt_ptr, PPLDGK)) ||
			(BOOL(F_ISSET(ply_ptr, PKNGDM)) == 
				BOOL(F_ISSET(crt_ptr, PKNGDM))) || (! AT_WAR)) {
					if(!F_ISSET(ply_ptr, PCHAOS) && ply_ptr->class < DM) {
						print(fd, "Sorry, you're lawful.\n");
						return (0);
                	}
					if(!F_ISSET(crt_ptr, PCHAOS) && ply_ptr->class < DM) {
						print(fd, "Sorry, that player is lawful.\n");
						return (0);
					}     
		}
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "How do you do that?  You're blind.\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, crt_ptr->first_obj,
		cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		if(!F_ISSET(crt_ptr, MNOGEN)) {
		print(fd, "%s doesn't have that.\n",
			F_ISSET(crt_ptr, MMALES) ? "He":"She");
		}
		else
			print(fd, "It doesn't have that.\n");
		return(0);
	}

	chance = 5 * ply_ptr->level;
	chance += bonus[ply_ptr->dexterity] * 3;
	if(crt_ptr->level < ply_ptr->level)
		chance += 3 * (crt_ptr->level - ply_ptr->level);
	if(crt_ptr->level > ply_ptr->level)
		chance -= 15 * (crt_ptr->level - ply_ptr->level);
	if(obj_ptr->questnum) chance = 0;

	if(obj_ptr->questnum || F_ISSET(crt_ptr, MUNSTL))
		chance = 0;
	
	if(ply_ptr->class >= CARETAKER)
		chance = 100;

	if(mrand(1, 100) <= chance) {
		print(fd, "You succeeded and gained %d experience points.\n",
			ply_ptr->level);
		ply_ptr->experience += ply_ptr->level;
		del_obj_crt(obj_ptr, crt_ptr);
		add_obj_crt(obj_ptr, ply_ptr);
		if(crt_ptr->type == PLAYER) {
			ply_ptr->lasttime[LT_PLYKL].ltime = t;
			ply_ptr->lasttime[LT_PLYKL].interval = (long)mrand(7, 10) * 86400L;
		}
	}

	else {
		print(fd, "You failed and lost %d experience points.\n",
			ply_ptr->level);
		ply_ptr->experience -= ply_ptr->level;
		if(ply_ptr->experience < 0)
			ply_ptr->experience = 0;
		ANSI(fd, RED);
		print(fd, "\n%s", crt_ptr->type == MONSTER ? 
			"You are attacked for attempted theft.\n" : "");
		ANSI(fd, WHITE);
		if(!F_ISSET(ply_ptr, PDMINV)) {
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"%M tried to steal from %m.", ply_ptr, crt_ptr);
		}

		if(crt_ptr->type == PLAYER)
			print(crt_ptr->fd, "%M tried to steal %1i from you.\n",
				ply_ptr, obj_ptr);
		else
			add_enm_crt(ply_ptr->name, crt_ptr);
	}

	return(0);
}

/*************************************************************************/
/*			steal gold				         */
/*************************************************************************/
/* This function allows a thief to steal gold from a monster or player.  */

int stealgold(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	object		*obj_ptr;
	long		i, t;
	int		fd, chance;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 1) {
		print(fd, "Steal from whom?\n");
		return(0);
	}

	if(ply_ptr->class != THIEF && ply_ptr->class < CARETAKER) {
		print(fd, "Only thieves may steal.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	i = LT(ply_ptr, LT_STEAL);
	t = time(0);

	if(t < i) {
		please_wait(fd, i - t);
		return(0);
	}

	if(F_ISSET(ply_ptr, PINVIS)) {
		F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
		print(fd, "Your invisibility fades.\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
	}

	ply_ptr->lasttime[LT_STEAL].ltime = t;
	ply_ptr->lasttime[LT_STEAL].interval = 5;

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		cmnd->str[1][0] = up(cmnd->str[1][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[1], cmnd->val[1]);

		if(!crt_ptr || crt_ptr == ply_ptr || F_ISSET(ply_ptr, PBLIND)) {
			print(fd, "I don't see that here.\n");
			return(0);
		}
	}

	if(crt_ptr->type == MONSTER) {
		if(F_ISSET(crt_ptr, MUNKIL)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
			print(fd, "%s cannot be stolen from.\n",
				F_ISSET(crt_ptr, MMALES) ? "He":"She");
			}
			else
			print(fd, "It cannot be stolen from.\n");
			return(0);
		}
		if(is_enm_crt(ply_ptr->name, crt_ptr)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "Not while %s's attacking you.\n",
			    	F_ISSET(crt_ptr, MMALES) ? "he":"she");
			}
			else
				print(fd, "Not while it's attacking you.\n");

			return(0);
		}
	}
	else {
		if(F_ISSET(rom_ptr, RNOKIL) && ply_ptr->class < DM) {
			print(fd, "Stealing is not allowed here.\n");
			return(0);
		}

		if((!F_ISSET(ply_ptr, PPLDGK) || !F_ISSET(crt_ptr, PPLDGK)) ||
			(BOOL(F_ISSET(ply_ptr, PKNGDM)) == 
				BOOL(F_ISSET(crt_ptr, PKNGDM))) || (! AT_WAR)) {
					if(!F_ISSET(ply_ptr, PCHAOS) && ply_ptr->class < DM) {
						print(fd, "Sorry, you're lawful.\n");
						return (0);
                	}
					if(!F_ISSET(crt_ptr, PCHAOS) && ply_ptr->class < DM) {
						print(fd, "Sorry, that player is lawful.\n");
						return (0);
					}     
		}
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "How do you do that?  You're blind.\n");
		return(0);
	}


	chance =  4 * ply_ptr->level; 
	chance += bonus[ply_ptr->dexterity] * 3;
	if(crt_ptr->level < ply_ptr->level)
		chance += (ply_ptr->level - crt_ptr->level) * 3;
	if(crt_ptr->level > ply_ptr->level)
		chance -= 15 * (crt_ptr->level - ply_ptr->level);

	if(ply_ptr->class >= CARETAKER)
		chance = 100;

	if(mrand(1, 100) <= chance) {
		if(crt_ptr->gold) {
			print(fd, 
				"You stole %d gold pieces and gained %d experience points.\n",
					crt_ptr->gold, ply_ptr->level);
			ply_ptr->experience += ply_ptr->level/2;
			ply_ptr->gold = ply_ptr->gold + crt_ptr->gold;
			crt_ptr->gold = 0;
			if(crt_ptr->type == PLAYER) {
				ply_ptr->lasttime[LT_PLYKL].ltime = t;
				ply_ptr->lasttime[LT_PLYKL].interval = 
					(long)mrand(7, 10) * 86400L;
			}
		}
		else
			print(fd, "You were undetected, but %s is not carrying any gold.\n",
				F_ISSET(crt_ptr, MMALES) ? "he":"she");
	}

	else {
		print(fd, "You failed and lost %d experience points.\n",
			ply_ptr->level);
		ply_ptr->experience -= ply_ptr->level;
		if(ply_ptr->experience < 0)
			ply_ptr->experience = 0;
		ANSI(fd, RED);
		print(fd, "\n%s", crt_ptr->type == MONSTER ? 
			"You are attacked.\n" : "");
		ANSI(fd, WHITE);
		if(!F_ISSET(ply_ptr, PDMINV)) {
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"%M tried to steal %m's gold.", ply_ptr, crt_ptr);
		}

		if(crt_ptr->type == PLAYER)
			print(crt_ptr->fd, "%M tried to steal your gold from you.\n",
				ply_ptr);
		else
			add_enm_crt(ply_ptr->name, crt_ptr);
	}

	return(0);
}

/*************************************************************************/
/*			distract				         */
/*************************************************************************/
/* This function allows a thief to distract a monster which has the      */
/* GUARD flag set.  Only thieves may distract.                           */

int distract(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		i, t;
	int		fd, p, chance;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Distract whom?\n");
		return(0);
	}

	if(ply_ptr->class != THIEF && ply_ptr->class < CARETAKER) {
		print(fd, "Only thieves have that talent.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PHIDDN)) {
		F_CLR(ply_ptr, PHIDDN);
		print(fd, "You step from your hiding place.\n");
		broadcast_rom(fd, ply_ptr->rom_num, "%M steps from hiding.", ply_ptr);
	}

	i = LT(ply_ptr, LT_DISTR);
	t = time(0);

	if(t < i) {
		please_wait(fd, i - t);
		return(0);
	}

	if(F_ISSET(ply_ptr, PINVIS)) {
		F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
		print(fd, "Your invisibility fades.\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
	}

	ply_ptr->lasttime[LT_DISTR].ltime = t;
	ply_ptr->lasttime[LT_DISTR].interval = 5;

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		cmnd->str[1][0] = up(cmnd->str[1][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[1], cmnd->val[1]);

		if(!crt_ptr || crt_ptr == ply_ptr || F_ISSET(ply_ptr, PBLIND)) {
			print(fd, "I don't see that here.\n");
			return(0);
		}
	}

	if(crt_ptr->type == MONSTER) {
		if(F_ISSET(crt_ptr, MUNKIL)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
			print(fd, "You cannot distract %s.\n",
				F_ISSET(crt_ptr, MMALES) ? "him":"her");
			}
			else
			print(fd, "You cannot distract it.\n");
			return(0);
		}
		if(is_enm_crt(ply_ptr->name, crt_ptr)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "Not while %s's attacking you.\n",
			    	F_ISSET(crt_ptr, MMALES) ? "he":"she");
			}
			else
				print(fd, "Not while it's attacking you.\n");

			return(0);
		}
	}
	else {
			print(fd, "You cannot distract that person.\n");
			return(0);
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "How do you do that?  You're blind.\n");
		return(0);
	}

	chance = 4 * ply_ptr->level;
	chance += bonus[ply_ptr->dexterity] * 4;
	if(crt_ptr->level < ply_ptr->level)
		chance += 4 * (ply_ptr->level - crt_ptr->level);
	if(crt_ptr->level > ply_ptr->level)
		chance = 25;
	if(F_ISSET(crt_ptr, MPERMT))
		chance = 0;
	
	if(ply_ptr->class >= CARETAKER)
		chance = 100;

	if(mrand(1, 100) <= chance && F_ISSET(crt_ptr, MGUARD)) {
		print(fd, "You succeeded and gained %d experience points.\n",
			(ply_ptr->level * 2));
		ply_ptr->experience += ply_ptr->level * 2;
		F_CLR(crt_ptr, MGUARD);
	}

	else {
		print(fd, "You failed and lost %d experience points.\n",
			(ply_ptr->level * 2));
		ply_ptr->experience -= ply_ptr->level * 2;
		if(ply_ptr->experience < 0)
			ply_ptr->experience = 0;
		ANSI(fd, RED);
		print(fd, "You are attacked.\n");
		ANSI(fd, WHITE);
		if(!F_ISSET(ply_ptr, PDMINV)) {
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"%M tried to distract %m and failed.", ply_ptr, crt_ptr);
		}
		add_enm_crt(ply_ptr->name, crt_ptr);
	}

	return(0);
}





/*************************************************************************/
/*				snatch				         */
/*************************************************************************/
/* This function allows a thief to snatch an object either held or       */
/* wileded by another player.  Both the thief and the victim must be     */
/* chaotic.                                         		 	 */

int snatch(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;
{

	creature	*crt_ptr;
	room		*rom_ptr;
	object		*obj_ptr;
	long		i, t;
	int		fd, chance, found = 0, hold = 0, wield = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Snatch what?\n");
		return(0);
	}

	if(cmnd->num < 3) {
		print(fd, "Snatch what from whom?\n");
		return(0);
	}

	if(ply_ptr->class != THIEF && ply_ptr->class < CARETAKER) {
		print(fd, "Only thieves may snatch.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PHIDDN)) {
		F_CLR(ply_ptr, PHIDDN);
		print(fd, "You step from your hiding place.\n");
		broadcast_rom(fd, ply_ptr->rom_num, "%M steps from hiding.", ply_ptr);
	}

	if(F_ISSET(ply_ptr, PINVIS)) {
		F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
		print(fd, "Your invisibility fades.\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
	}

	i = LT(ply_ptr, LT_STEAL);
	t = time(0);

	if(t < i) {
		please_wait(fd, i - t);
		return(0);
	}

	ply_ptr->lasttime[LT_STEAL].ltime = t;
	ply_ptr->lasttime[LT_STEAL].interval = 5;

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[2], cmnd->val[2]);

	if(!crt_ptr) {
		cmnd->str[2][0] = up(cmnd->str[2][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "You grope blindly and fail.\n");
		return(0);
	}

	if(!crt_ptr) {
		print(fd, "I don't see that here.\n");
		return(0);
	}

	if(crt_ptr == ply_ptr) {
		print(fd, "How silly is that?\n");
		return(0);
	}

	if(crt_ptr->type == MONSTER) {
		print(fd, "%M is beyond your reach.\n", crt_ptr);
		return(0);
	}

	if(F_ISSET(rom_ptr, RNOKIL) && ply_ptr->class < DM) {
		print(fd, "That is forbidden here.\n");
		return(0);
	}

	if((!F_ISSET(ply_ptr, PPLDGK) || !F_ISSET(crt_ptr, PPLDGK)) ||
		(BOOL(F_ISSET(ply_ptr, PKNGDM)) == BOOL(F_ISSET(crt_ptr, PKNGDM))) || 
		(! AT_WAR)) {
			if(!F_ISSET(ply_ptr, PCHAOS) && ply_ptr->class < DM) {
				print(fd, "Sorry, you're lawful.\n");
				return (0);
           	}
			if(!F_ISSET(crt_ptr, PCHAOS) && ply_ptr->class < DM) {
				print(fd, "Sorry, that player is lawful.\n");
				return (0);
			}     
	}


	if(crt_ptr->ready[WIELD - 1]) {
		obj_ptr = crt_ptr->ready[WIELD - 1];
     	if(EQUAL(obj_ptr, cmnd->str[1])) {
			wield = 1;
			found = 1;
		}
		else
			free_obj(obj_ptr);
	}

	if(!found && crt_ptr->ready[HELD - 1]) {
		obj_ptr = crt_ptr->ready[HELD - 1];
     	if(EQUAL(obj_ptr, cmnd->str[1])) {
			hold = 1;
			found = 1;
		}
		else
			free_obj(obj_ptr);
	}

	if(!found) {
		print(fd, "%s is not handling that.\n",
			F_ISSET(crt_ptr, MMALES) ? "He":"She");
		return (0);
	}

	chance = 5 * ply_ptr->level;
	chance += bonus[ply_ptr->dexterity] * 3;
	if(crt_ptr->level < ply_ptr->level)
		chance += 3 * (crt_ptr->level - ply_ptr->level);
	if(crt_ptr->level > ply_ptr->level)
		chance -= 15 * (crt_ptr->level - ply_ptr->level);
	chance += ((ply_ptr->dexterity - crt_ptr->dexterity) * 3);
	if(obj_ptr->questnum) chance = 0;

	if(ply_ptr->class >= CARETAKER)
		chance = 100;

	if(mrand(1, 100) <= chance) {
		print(fd, "You succeeded in taking %1i.\n", obj_ptr);
		add_obj_crt(obj_ptr, ply_ptr);
		if(!hold && !wield)
			del_obj_crt(obj_ptr, crt_ptr);	
		else {
			if(wield) 
				crt_ptr->ready[WIELD - 1] = 0;
			if(hold) 
				crt_ptr->ready[HELD - 1] = 0;
		}

		compute_ac(crt_ptr);
		compute_agility(crt_ptr);

		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
			"%M snatched %1i from %m.", ply_ptr, obj_ptr, crt_ptr);
		print(crt_ptr->fd, "%M snatched %1i from you.\n",
			ply_ptr, obj_ptr, crt_ptr);

	}
	else {
		print(fd, "You failed and lost %d experience points.\n",
			ply_ptr->level * 4);
		ply_ptr->experience -= (ply_ptr->level * 4);
		if(ply_ptr->experience < 0)
			ply_ptr->experience = 0;
		if(!F_ISSET(ply_ptr, PDMINV)) {
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"%M tried to snatch something from %m.", ply_ptr, crt_ptr);
		}
		print(crt_ptr->fd, "%M tried to snatch %1i from you.\n",
			ply_ptr, obj_ptr);
	}

	return(0);
}
