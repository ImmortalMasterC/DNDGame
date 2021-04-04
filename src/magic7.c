/*
 * MAGIC7.C:
 *
 *  Additional spell-casting routines.
 */

#include "mstruct.h"
#include "mextern.h"


/************************************************************************/
/*   		 		             resist_cold		 		            */
/************************************************************************/
/* This function allows players to cast the resist cold spell.  It will */
/* allow the player to resist cold breathed on them by dragons and  	*/
/* other breathers, and will negate a freeze spell.						*/

int resist_cold(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd     	*cmnd;
int     	how;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    int     	fd;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    if(!S_ISSET(ply_ptr, SRCOLD) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

	if(ply_ptr->mpcur < 6 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	if(ply_ptr->mpcur < 12 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

    if(spell_fail(ply_ptr, how)) {
    	if(how==CAST && ply_ptr->class != MAGE)
   			ply_ptr->mpcur -= 12;
        return(0);
    }

    if(cmnd->num == 2) {
        ply_ptr->lasttime[LT_RBRTH].ltime = time(0);
        F_SET(ply_ptr, PRCOLD);
        broadcast_rom(fd, ply_ptr->rom_num, "%M resists cold.", ply_ptr);
        if(how == CAST) {
            print(fd, "You cast a resist-cold spell.\n");
			print(fd, "A warm aura surrounds you.\n");
			if(F_ISSET(ply_ptr, PFROZE)) {
				print(fd, "You can feel the freeze spell dissipate.\n");
				F_CLR(ply_ptr, PFROZE);
			}
			if(F_ISSET(ply_ptr, PRFIRE)) {
				  print(fd, "Your resist-fire spell dissipates.\n");
                F_CLR(ply_ptr, PRFIRE);
            }
			if(F_ISSET(ply_ptr, PRACID)) {
            	print(fd, "Your resist-acid spell dissipates.\n");
            	F_CLR(ply_ptr, PRACID);
        	}

			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 6;
			if(ply_ptr->class != MAGE)
			    ply_ptr->mpcur -= 12;
            ply_ptr->lasttime[LT_RBRTH].interval = MAX(300, 1200 +
                bonus[ply_ptr->intelligence] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
            	print(fd,
					"The magic in the air increase the power of your spell.\n");
            	ply_ptr->lasttime[LT_RBRTH].interval += 800L;
        	}                                
        }
        else {
            print(fd, "A warm aura surrounds you.\n");
            ply_ptr->lasttime[LT_RBRTH].interval = 1200L;
        }
        return(1);
    }
    else {
        if(how == POTION) {
            print(fd, "You can only use a potion on yourself.\n");
            return(0);
        }

        cmnd->str[2][0] = up(cmnd->str[2][0]);
        crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);

        if(!crt_ptr) {
            print(fd, "I don't see that player here.\n");
            return(0);
        }

        F_SET(crt_ptr, PRCOLD);
        crt_ptr->lasttime[LT_RBRTH].ltime = time(0);
        broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
            "%M casts a resist-cold spell on %m.", ply_ptr, crt_ptr);
        print(crt_ptr->fd, "%M casts a resist-cold spell on you.\n", ply_ptr);
		if(F_ISSET(crt_ptr, PFROZE)) {
			print(crt_ptr->fd, "You can feel the freeze spell dissipate.\n");
			F_CLR(crt_ptr, PFROZE);
		}	
 		if(F_ISSET(crt_ptr, PRFIRE)) {
        	print(crt_ptr->fd, "Your resist-fire spell dissipates.\n");
            F_CLR(crt_ptr, PRFIRE);
		}
 		if(F_ISSET(crt_ptr, PRACID)) {
        	print(crt_ptr->fd, "Your resist-acid spell dissipates.\n");
            F_CLR(crt_ptr, PRACID);
		}


        if(how == CAST) {
            print(fd, "You cast a resist-cold spell on %s.\n", crt_ptr);
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 6;
			if(ply_ptr->class != MAGE)
			    ply_ptr->mpcur -= 12;
        	    crt_ptr->lasttime[LT_RBRTH].interval = MAX(300, 1200 +
        	        bonus[ply_ptr->intelligence] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
            	print(fd,
					"The magic in the air increase the power of your spell.\n");
        	    crt_ptr->lasttime[LT_RBRTH].interval += 800L;
        	}                                
		}
        else {
            print(fd, "%M resists cold.\n", crt_ptr);
            crt_ptr->lasttime[LT_RBRTH].interval = 1200L;
        }

        return(1);
    }
}


/*************************************************************************/
/*                          Breathe-water                                */
/*************************************************************************/
/* This function allows players to cast the breath water spell.  It will */
/* allow the player to resist damage in a room with the player harming	 */
/* and water realm flags set.											 */

int breathe_water(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd     	*cmnd;
int     	how;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    int     	fd;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    if(!S_ISSET(ply_ptr, SBRWAT) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

	if(ply_ptr->mpcur < 6 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	if(ply_ptr->mpcur < 12 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

    if(spell_fail(ply_ptr, how)) {
    	if(how==CAST && ply_ptr->class != MAGE)
    		ply_ptr->mpcur -= 12;
    	return(0);
    }

    if(cmnd->num == 2) {
        ply_ptr->lasttime[LT_BRWAT].ltime = time(0);
        F_SET(ply_ptr, PBRWAT);
        broadcast_rom(fd, ply_ptr->rom_num, "%M breathes water.", ply_ptr);
        if(how == CAST) {
            print(fd, "You cast a breathe-water spell.  "
						"Your lungs increase in size.\n");
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 6;
			if(ply_ptr->class != MAGE)
	    	    ply_ptr->mpcur -= 12;
        	ply_ptr->lasttime[LT_BRWAT].interval = MAX(300, 1200 +
        	    bonus[ply_ptr->intelligence] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
				print(fd, "The magic in the air increases the "
					"power of your spell.\n");
        	    ply_ptr->lasttime[LT_BRWAT].interval += 800L;
        	}                                
		}
        else {
            print(fd, "Your lungs increase in size.\n");
            ply_ptr->lasttime[LT_BRWAT].interval = 1200L;
        }
        return(1);
    }
    else {
        if(how == POTION) {
            print(fd, "You can only use a potion on yourself.\n");
            return(0);
        }

        cmnd->str[2][0] = up(cmnd->str[2][0]);
        crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);
        if(!crt_ptr) {
            print(fd, "I don't see that player here.\n");
            return(0);
        }

        F_SET(crt_ptr, PBRWAT);
        crt_ptr->lasttime[LT_BRWAT].ltime = time(0);
        broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
            "%M casts a breathe-water spell on %m.", ply_ptr, crt_ptr);
        print(crt_ptr->fd, "%M casts breathe-water on you.\n", ply_ptr);

        if(how == CAST) {
            print(fd, "You cast a breathe-water spell on %s.\n", crt_ptr);
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 6;
			if(ply_ptr->class != MAGE)
			    ply_ptr->mpcur -= 12;
        	    crt_ptr->lasttime[LT_BRWAT].interval = MAX(300, 1200 +
        	        bonus[ply_ptr->intelligence] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
				print(fd,
					"The magic in the air increase the power of your spell.\n");
        	    crt_ptr->lasttime[LT_BRWAT].interval += 800L;
        	}                                
        }
        else {
            print(fd, "%M breathe-water.\n", crt_ptr);
            crt_ptr->lasttime[LT_BRWAT].interval = 1200L;
        }

        return(1);
    }
}


/*************************************************************************/
/*                      Stone-Shield                                     */
/*************************************************************************/
/* This function allows players to cast the earth shield spell.  It will */
/* allow the player to resist earth-realm offensive spells cast on them  */

int earth_shield(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd     	*cmnd;
int     	how;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    int     	fd;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    if(!S_ISSET(ply_ptr, SSSHLD) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

	if(ply_ptr->mpcur < 6 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	if(ply_ptr->mpcur < 12 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

    if(spell_fail(ply_ptr, how)) {
		if(how==CAST && ply_ptr->class != MAGE)
			ply_ptr->mpcur -= 12;
		return(0);
    }

    if(cmnd->num == 2) {
        ply_ptr->lasttime[LT_SSHLD].ltime = time(0);
        F_SET(ply_ptr, PSSHLD);
        broadcast_rom(fd, ply_ptr->rom_num, "%M casts earth shield.", ply_ptr);
        if(how == CAST) {
            print(fd, 
				"You cast an earth-shield spell.  Your body is protected.\n");
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 6;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 12;
            ply_ptr->lasttime[LT_SSHLD].interval = MAX(300, 1200 +
                bonus[ply_ptr->intelligence] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
				print(fd,
					"The magic in the air increase the power of your spell.\n");
    	        ply_ptr->lasttime[LT_SSHLD].interval += 800L;
    	    }                                
        }
        else {
            print(fd, "Your body is shielded from earth spells.\n");
            ply_ptr->lasttime[LT_SSHLD].interval = 1200L;
        }
        return(1);
    }
    else {
        if(how == POTION) {
            print(fd, "You can only use a potion on yourself.\n");
            return(0);
        }

        cmnd->str[2][0] = up(cmnd->str[2][0]);
        crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
        	cmnd->str[2], cmnd->val[2]);
        if(!crt_ptr) {
            print(fd, "I don't see that player here.\n");
            return(0);
        }

        F_SET(crt_ptr, PSSHLD);
        crt_ptr->lasttime[LT_SSHLD].ltime = time(0);
        broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
            "%M casts an earth-shield spell on %m.", ply_ptr, crt_ptr);
        print(crt_ptr->fd, "%M casts earth-shield on you.\n", ply_ptr);

        if(how == CAST) {
            print(fd, "You cast an earth-shield spell on %s.\n", crt_ptr);
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 6;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 12;
        	    crt_ptr->lasttime[LT_SSHLD].interval = MAX(300, 1200 +
        	        bonus[ply_ptr->intelligence] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
				print(fd,
					"The magic in the air increase the power of your spell.\n");
        	    crt_ptr->lasttime[LT_SSHLD].interval += 800L;
        	}                                
        }
        else {
            print(fd, "%M casts earth-shield.\n", crt_ptr);
            crt_ptr->lasttime[LT_SSHLD].interval = 1200L;
        }

        return(1);
    }
}

/*************************************************************************/
/*      		            mind-meld	                                 */
/*************************************************************************/
/*	This function allows a player to cast a one-time spell which allows	 */
/*  the player to 'see' the room which another player occupies.			 */

int locate_player(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd     	*cmnd;
int     	how;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    int     	fd, chance;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    if(fd < 0) return(0);

    if(!S_ISSET(ply_ptr, SLOCAT) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }
    
	if(ply_ptr->class != MAGE) {
    	if(ply_ptr->mpcur < 12 && how == CAST) {
        	print(fd, "Not enough magic points.\n");
        	return(0);
    	}
    }
    
	else {
		if(ply_ptr->mpcur < 6 && how == CAST) {
        	print(fd, "Not enough magic points.\n");
        	return(0);
    	}
    }
 
    if(cmnd->num < 3) {
        print(fd, "Link with whom?\n");
        return(0);
    }
 
    lowercize(cmnd->str[2], 1);
    crt_ptr = find_who(cmnd->str[2]);
 
    if(!crt_ptr || F_ISSET(crt_ptr, PDMINV) ||
       (F_ISSET(crt_ptr, PINVIS) && !F_ISSET(ply_ptr, PDINVI))) {
        	print(fd, "That person cannot be located.\n");
        	return(0);
    }
 
    if(crt_ptr->class >= CARETAKER) {
		ANSI(fd, REDBG);
        print(fd, "Now you are really in trouble.\n");
        print(fd, "A glimpse into %s mind would drive you insane!!\n",
			F_ISSET(crt_ptr, PMALES) ? "his":"her");
		ANSI(fd, BLUEBG);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
        return(0);
	} 

	broadcast_rom(fd, ply_ptr->rom_num, "%M casts a mind-meld spell.\n", 
		ply_ptr);
	if(how == CAST) {
		print(fd, "Your mind attempts to meld with %m.\n", crt_ptr);
		if(ply_ptr->class == MAGE)	
			ply_ptr->mpcur -= 6;
		else
			ply_ptr->mpcur -= 12;
	}

	if(ply_ptr->class >= CARETAKER)
		chance = 100;
	else if(ply_ptr->class == MAGE)
		chance = 90 + (ply_ptr->level)/3;
	else if(ply_ptr->class == DRUID || ply_ptr->class == CLERIC || 
		ply_ptr->class == PALADIN)
			chance = 75 + ply_ptr->level;
	else
		chance = 75;	

	if(mrand(1, 100) < chance) {
		ANSI(crt_ptr->fd, YELLOW);
        	print(crt_ptr->fd, "%M briefly looks through your eyes.\n", 
				ply_ptr);
		ANSI(crt_ptr->fd, BOLD);
		ANSI(crt_ptr->fd, WHITE);
		ANSI(fd, NORMAL);
		ANSI(fd, REDBG);
		ANSI(fd, YELLOW);
       	display_rom(ply_ptr, crt_ptr->parent_rom);   
		ANSI(fd, BLUEBG);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
	}
    else {
        print(fd,"%m is having a disturbing sexual fantasy right now...\n", 
			crt_ptr);
        print(fd,"Your mind is unable to connect.  Try again later.\n");
        print(crt_ptr->fd, 
			"%M tried to make a mind-meld connection with you,\n", ply_ptr);
        print(crt_ptr->fd, "but your disturbing sexual fantasy intruded...\n");
    }
 
    return(1);
}
 
/************************************************************************/
/*                          drain_exp                              	    */
/************************************************************************/
/* The spell drain_exp causes a player to lose a selected amout of 	 	*/
/* exp.  When a player loses exp, the player's magical realm and   	 	*/
/* weapon procifiency will reflect the change.  This spell is not  	 	*/
/* intended to be learned or cast by a player.  The 4th parameter	 	*/
/* if given, will base the exp loss on the DnS damage of the object	 	*/

int drain_exp(ply_ptr, cmnd, how, obj_ptr)
creature    *ply_ptr;
cmd     	*cmnd;
int     	how;
object  	*obj_ptr;

{
    room        *rom_ptr;
    creature    *crt_ptr;
    int     	fd;
    long    	loss;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;
 
    if(how == CAST && ply_ptr->class < CARETAKER) {
        print(fd, "That spell cannot be cast.\n");
        return(0);
        }      

    if(how ==  SCROLL) {
        print(fd, "That spell cannot be cast.\n");
        return(0);
        }      

    if(!S_ISSET(ply_ptr, SDREXP) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        print(fd, "(And besides, it can't be cast anyway.");
        return(0);
    }
 
    /* drain exp on self */
    if(cmnd->num == 2) {
        if(how == POTION || how == WAND) 
            loss= dice(ply_ptr->level, ply_ptr->level, (ply_ptr->level) * 10);

        else if(how == CAST)
            loss = dice(ply_ptr->level, ply_ptr->level, 1);
            loss = MIN(loss, ply_ptr->experience);

        if(how == CAST || how == WAND) {
            print(fd, "You cast a life-draining spell on yourself.\n");
            print(fd, "You lose %d experience.\n",loss);
            broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts a life-draining spell on %sself.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
        }
        else if(how == POTION) {
            print(fd, "You feel your experience slipping away.\n");
            print(fd, "You lose %d experience.\n",loss);
        }
        ply_ptr->experience -= loss;    
	    lower_prof(ply_ptr, loss);
    }
 
    /* life-drain a monster or player */
    else {
        if(how == POTION || how == WAND) 
            loss = mdice(obj_ptr);
        else if(how == CAST)
            loss = dice(ply_ptr->level, ply_ptr->level,1);

        if(how == POTION) {
            print(fd, "You can only use a potion on yourself.\n");
            return(0);
        }
 
        cmnd->str[2][0] = up(cmnd->str[2][0]);
        crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);
 
        if(!crt_ptr) {
            cmnd->str[2][0] = low(cmnd->str[2][0]);
            crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
				cmnd->str[2], cmnd->str[2]);
 
            if(!crt_ptr) {
                print(fd, "That's not here.\n");
                return(0);
            }
        }
 
        loss = MIN(loss, crt_ptr->experience);
        if(how == CAST || how == WAND) {
           print(fd, "You cast a life draining spell on %m.\n", crt_ptr);
           broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
           		"%M casts a life draining spell on %m.", ply_ptr, crt_ptr);
           print(crt_ptr->fd, "%M casts life draining spell on you.\n", 
				ply_ptr);
           print(crt_ptr->fd,"You lose %d experience.\n",loss);
           print(fd,"%M loses %d experience.\n", crt_ptr, loss);
        }
 
        crt_ptr->experience -= loss;    
	    lower_prof(crt_ptr, loss);
    }
 
    return(1);
}
               
/************************************************************************/
/*              		cure disease				                	*/
/************************************************************************/
/*	Player can cast a cure-disease spell on self or others				*/
 
int rm_disease(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd     	*cmnd;
int     	how;

{
    room        *rom_ptr;
    creature    *crt_ptr;
    int     	fd;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;
 
 
    if(ply_ptr->class != CLERIC && ply_ptr->class != PALADIN &&
    	ply_ptr->class != DRUID && ply_ptr->class < CARETAKER && how == CAST) {
           print(fd,
				"Only clerics, druids and paladins may cast that spell.\n");
           return(0);
    }                         

    if(!S_ISSET(ply_ptr, SRMDIS) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }
 
	if(ply_ptr->class == PALADIN && ply_ptr->mpcur < 12 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }

    if(ply_ptr->class == CLERIC && ply_ptr->mpcur < 6 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }

    if(ply_ptr->class == DRUID && ply_ptr->mpcur < 3 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }
 
    if(cmnd->num == 2) {
		if((how == CAST || how == SCROLL || how == WAND)) { 
		    if(ply_ptr->class == PALADIN)
    	        ply_ptr->mpcur -= 12;
		    else if(ply_ptr->class == CLERIC)
    	        ply_ptr->mpcur -= 6;
		    else if(ply_ptr->class == DRUID)
    	        ply_ptr->mpcur -= 3;
    	    print(fd, "Cure disease spell cast on yourself.\n");
	    	if(F_ISSET(ply_ptr, PDISEA))
            	print(fd, "Your fever subsides.\n");
	    	else
				print(fd, 
					"There was nothing wrong with you, you hypochondriac!\n");
            broadcast_rom(fd, ply_ptr->rom_num, 
               	"%M casts cure disease on %sself.", ply_ptr,
               		F_ISSET(ply_ptr, PMALES) ? "him":"her");
		}
       	else if(how == POTION && F_ISSET(ply_ptr, PDISEA))
           print(fd, "The potion immediately makes your fever subside.\n");
        else if(how == POTION && !F_ISSET(ply_ptr, PDISEA))
            print(fd, "Potion wasted.  There was nothing wrong with you.\n");

        F_CLR(ply_ptr, PDISEA);
	} 
    else {
        if(how == POTION) {
            print(fd, "You can only use a potion on yourself.\n");
            return(0);
        }
 
        cmnd->str[2][0] = up(cmnd->str[2][0]);
        crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);
 
        if(!crt_ptr) {
            cmnd->str[2][0] = low(cmnd->str[2][0]);
            crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
				cmnd->str[2], cmnd->str[2]);
 
            if(!crt_ptr) {
                print(fd, "No one's here by that name.\n");
                return(0);
            }
        }

        if(how == CAST || how == SCROLL || how == WAND) {
            print(fd, "You cast a cure disease spell on %m.\n", crt_ptr);
            broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts a cure disease spell on %m.", ply_ptr, crt_ptr);
            print(crt_ptr->fd, "%M casts a cure disease spell on you.\n", 
				ply_ptr);
	    	if(crt_ptr->class != DRUID  && F_ISSET(crt_ptr, PDISEA)) {
				ANSI(crt_ptr->fd, YELLOW);
	    	    print(crt_ptr->fd, "You feel your body take on new life.\n");
				ANSI(crt_ptr->fd, WHITE);
				ply_ptr->experience += (ply_ptr->level) * 2;
        		print(fd, "You gain %d experience points.\n", 
					(ply_ptr->level) * 2);
        	    F_CLR(crt_ptr, PDISEA);
	    	}
	    	else
	    		print(crt_ptr->fd, "Of course, it has no effect on you.\n");
        }

	    if(ply_ptr->class == PALADIN)
            ply_ptr->mpcur -= 12;
	    else if(ply_ptr->class == CLERIC)
            ply_ptr->mpcur -= 6;
	    else if(ply_ptr->class == DRUID)
            ply_ptr->mpcur -= 3;

		
	} 
    return(1);
}

/**************************************************************************/
/*                           object_send                                  */
/**************************************************************************/
/* The send object spell allows a mage or DM/Caretaker to magically       */
/* transport an object to another login (detectable) player.  The maxium  */
/* weight of the object being transported is based on the player's        */
/* level and intellignece. 												  */

int object_send(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd     	*cmnd;
int     	how;

{
    creature    *crt_ptr;
    object      *obj_ptr;
    room        *rom_ptr;
    int         fd, cost;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    if(ply_ptr->class != MAGE && ply_ptr->class < CARETAKER && how == CAST) {
       print(fd, "Only a mage is capable of casting that spell.\n");
       return(0);
    }                         

    if(!S_ISSET(ply_ptr, STRANO) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

    if(ply_ptr->level < 5) {
        print(fd, "Your level is not high enough to cast that.\n");
        return(0);
    }
 
    if(ply_ptr->mpcur < 3 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }

    if(spell_fail(ply_ptr, how)) {
    	if(how == CAST)
   	   		ply_ptr->mpcur -= 3;
        	return(0);
    }

    if(cmnd->num < 4) {
        print(fd, "Teleport what to whom?\n");
        return(0);
    }

    lowercize(cmnd->str[3], 1);
    crt_ptr = find_who(cmnd->str[3]);
 
    if(!crt_ptr || F_ISSET(crt_ptr, PDMINV) ||
       (F_ISSET(crt_ptr, PINVIS) && !F_ISSET(ply_ptr, PDINVI))) {
        	print(fd, "That person cannot be located.\n");
        	return(0);
    }
 
    obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
        cmnd->str[2], cmnd->val[2]);
 
    if(!obj_ptr) {
        print(fd, "You're confused.  You don't have that object.\n");
        return(0);
    }

	if(obj_ptr->questnum) {
		print(fd, "You can't transport a quest object.\n");
		return(0);
	}

    cost = 3 + bonus[ply_ptr->intelligence] + (ply_ptr->level - 5) * 2;
    if(obj_ptr->weight > cost) {
        print(fd, "%I is too heavy to transport at your current level.\n", 
			obj_ptr);
        return(0);
    }

    cost = 3 + (obj_ptr->weight)/4;
    if(ply_ptr->mpcur < cost && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    } 
	else if(how == CAST) {
		ply_ptr->mpcur -= cost;
	}
    
    if(spell_fail(ply_ptr, how) && ply_ptr->class != MAGE)
		return(0);

    if (max_weight(crt_ptr) < weight_ply(crt_ptr) + obj_ptr->weight) {
        print(fd, "%M is unable to hold %1i.\n", crt_ptr, obj_ptr);
        return(0);
    }

    del_obj_crt(obj_ptr, ply_ptr);
    add_obj_crt(obj_ptr, crt_ptr);

    if(!F_ISSET(ply_ptr, PDMINV))
	    broadcast_rom(fd, ply_ptr->rom_num, 
			"%M magically transports an object to someone.\n", ply_ptr);
    print(fd, "%I dissappears in a blinding flash of light.\n", obj_ptr);
    print(fd, "You sucessfully transport %1i to %m.\n", obj_ptr, crt_ptr);
    if(!F_ISSET(ply_ptr, PDMINV)) {
		ANSI(crt_ptr->fd, YELLOW);
		print(crt_ptr->fd, "%M magically transports %1i to you.\n",
			ply_ptr, obj_ptr);
		ANSI(crt_ptr->fd, BOLD);
		ANSI(crt_ptr->fd, WHITE);
	}
    return(1);
}
