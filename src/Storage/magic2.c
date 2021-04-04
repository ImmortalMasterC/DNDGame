/*
 * MAGIC2.C:
 *
 *  Additional user routines dealing with magic spells.  
 *
 *  Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>

/************************************************************************/
/* 		            vigor		                        */
/************************************************************************/
/* This function will cause the vigor spell to be cast on a player or   */
/* another monster.  It heals 1d8 hit points plus any bonuses for       */
/* intelligence.  If the healer is a cleric then there is an additional */
/* point of healing for each level of the cleric.           		*/

int vigor(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd    	    *cmnd;
int         how;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    int     	fd, heal;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    if(!S_ISSET(ply_ptr, SVIGOR) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

    if(ply_ptr->mpcur < 2 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }

    if(ply_ptr->class == BARBARIAN) {
    	if(spell_fail(ply_ptr, how)) {
        	if(how == CAST)
               	ply_ptr->mpcur -= 2;
        	return(0);
    	}  
    }
    /* Vigor self */
    if(cmnd->num == 2) {
        if(how == CAST) {
            heal = MAX(bonus[ply_ptr->intelligence], 
				bonus[ply_ptr->piety]) + 
            	((ply_ptr->class == DRUID) ? ply_ptr->level/2 +
              	mrand(1, 1 + ply_ptr->level) : 0) + 
            	((ply_ptr->class == CLERIC) ? ply_ptr->level/2 +
              	mrand(1, 1 + ply_ptr->level) : 0) + 
            	((ply_ptr->class == PALADIN) ? ply_ptr->level/3 +
              	mrand(1, 1 + ply_ptr->level/2) : 0) +
	    		((ply_ptr->class == MONK) ? ply_ptr->level/4 +
        		mrand(1, 1 + ply_ptr->level/3) : 0) +
            	mrand(1, 8);
            		ply_ptr->mpcur -= 2;
	    			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
						heal += mrand(1, 3);
						print(fd,
							"The magic in the air increase the "
								"power of your spell.\n");
	    			}
		}
        else
            heal = mrand(3, 8);

        heal = MAX(1, heal);

        ply_ptr->hpcur += heal;

        if(ply_ptr->hpcur > ply_ptr->hpmax && !F_ISSET(ply_ptr, PSTRNG))
            ply_ptr->hpcur = ply_ptr->hpmax;
        if(ply_ptr->hpcur > ply_ptr->hpmax && F_ISSET(ply_ptr, PSTRNG)) {
            ply_ptr->hpcur = ply_ptr->hpmax + ply_ptr->level;
			compute_thaco(ply_ptr);
		}

        if(how == CAST || how == SCROLL) {
            print(fd, "Vigor spell cast.\n");
            broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts a vigor spell on %sself.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
            return(1);
        }
        else {
            print(fd, "You feel better.\n");
            return(1);
        }
    }

    /* Cast vigor on another player or monster */
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
				cmnd->str[2], cmnd->val[2]);

            if(!crt_ptr) {
                print(fd, "That person is not here.\n");
                return(0);
            }
        }

        if(how == CAST) {
            heal = MAX(bonus[ply_ptr->intelligence], 
            bonus[ply_ptr->piety]) + 
            ((ply_ptr->class == DRUID) ? ply_ptr->level/2 +
            mrand(1, 1 + ply_ptr->level/3) : 0) + 
            ((ply_ptr->class == CLERIC) ? ply_ptr->level/2 +
            mrand(1, 1 + ply_ptr->level/2) : 0) + 
            ((ply_ptr->class == PALADIN) ? ply_ptr->level/3 +
            mrand(1, 1 + ply_ptr->level/4) : 0) +
	    	((ply_ptr->class == MONK) ? ply_ptr->level/3 +
            mrand(1, 1 + ply_ptr->level/5) : 0) +
            mrand(1, 8);
            	ply_ptr->mpcur -= 2;
	    		if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
					heal += mrand(2, 4);
					print(fd, "The magic in the air increase the "
								"power of your spell.\n");
	    		}
		}
        else
            heal = mrand(3, 8);
        heal = MAX(1, heal);
		if(ply_ptr->class >= CARETAKER)
			heal = 8;

        crt_ptr->hpcur += MAX(1, heal);
		if(crt_ptr->hpcur > (crt_ptr->hpcur + 1))
        	crt_ptr->hpcur = crt_ptr->hpcur + 1;

        if(crt_ptr->hpcur > crt_ptr->hpmax && !F_ISSET(crt_ptr, PSTRNG))
            crt_ptr->hpcur = crt_ptr->hpmax;

        if(crt_ptr->hpcur > crt_ptr->hpmax && F_ISSET(crt_ptr, PSTRNG)) {
            crt_ptr->hpcur = crt_ptr->hpmax + crt_ptr->level;
			compute_thaco(ply_ptr);
		}
        if(how == CAST || how == SCROLL || how == WAND) {
            print(fd, "Vigor spell of %d HP cast on %m.\n", heal, crt_ptr);
            print(crt_ptr->fd, "%M casts a vigor spell on you.\n", ply_ptr);
            broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
            	"%M casts a vigor spell on %m.", ply_ptr, crt_ptr);
            if(crt_ptr->hpcur >= crt_ptr->hpmax) {
				print(fd, "%M is at full strength.\n", crt_ptr);
            	return(1);
			}
            if(crt_ptr->hpcur >= crt_ptr->hpmax/2) {
				print(fd, "%M is above half-strength now.\n", crt_ptr);
            	return(1);
        	}
            if(crt_ptr->hpcur < crt_ptr->hpmax/2) {
				print(fd, "%M is still below half-strength.\n", crt_ptr);
            	return(1);
        	}
           	return(1);
        }
    }

    return(1);
}

/*************************************************************************/
/*              	cure-poison  				         */
/*************************************************************************/
/* This function allows a player to cast a cure-poison spell on himself, */
/* another player or a monster.  It will remove any poison that is in    */
/* that player's system.                        			 */

int curepoison(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd     	*cmnd;
int     	how;

{
    room        *rom_ptr;
    creature    *crt_ptr;
    int     	fd;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    if(!S_ISSET(ply_ptr, SCUREP) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

    if(ply_ptr->mpcur < 6 && how == CAST && 
		ply_ptr->class != DRUID && ply_ptr->class != CLERIC) {
    	   	print(fd, "Not enough magic points.\n");
    	   	return(0);
    }

    if(ply_ptr->mpcur < 1 && how == CAST && ply_ptr->class == DRUID) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }

    if(ply_ptr->mpcur < 3 && how == CAST && ply_ptr->class == CLERIC) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }

    /* Cure-poison self */
    if(cmnd->num == 2) {
    	if(how == CAST && ply_ptr->class == DRUID) {
    	    print(fd, "Druids cannot be poisoned.  Don't waste your time.\n");
    	    return(0);
    	}

		if(how == CAST && ply_ptr->class == CLERIC)
			ply_ptr->mpcur -= 3;
		if(how == CAST && ply_ptr->class != CLERIC)
			ply_ptr->mpcur -= 6;

		if(how == CAST || how == SCROLL || how == WAND) {
			print(fd, "Cure-poison spell cast on yourself.\n");
			if(F_ISSET(ply_ptr, PPOISN))
				print(fd, "You feel much better as the poison subsides.\n");
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts cure-poison on %sself.", 
					ply_ptr, F_ISSET(ply_ptr, PMALES) ? "him":"her");
		}
		else if(how == POTION && F_ISSET(ply_ptr, PPOISN))
			print(fd, "You feel the poison subside.\n");
		else if(how == POTION && !F_ISSET(ply_ptr, PPOISN))
			print(fd, "Your stomach gurgles, but nothing else happens.\n");

		F_CLR(ply_ptr, PPOISN);

	}

    /* Cure a monster or player */
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
                print(fd, "That's not here.\n");
                return(0);
            }
        }

        if(how == CAST && ply_ptr->class == DRUID) 
            ply_ptr->mpcur -= 1;
        if(how == CAST && ply_ptr->class == CLERIC) 
            ply_ptr->mpcur -= 3;
        if(how == CAST && ply_ptr->class != DRUID && 
			ply_ptr->class != CLERIC) 
	    		ply_ptr->mpcur -=6;	
        if(how == CAST && crt_ptr->type == PLAYER && 
			F_ISSET(crt_ptr, PPOISN)) {
	            ply_ptr->experience += 2 * (ply_ptr->level);	
		    	print(fd,
					"You gain %d experience points as life returns to %m.\n", 
		    	   		2 * (ply_ptr->level), crt_ptr);	
			}

        if(how == CAST || how == SCROLL || how == WAND) {
            print(fd, "Cure-poison cast on %m.\n", crt_ptr);
            broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts cure-poison on %m.", ply_ptr, crt_ptr);
            print(crt_ptr->fd, "%M casts cure-poison on you.\n", ply_ptr);
			if(F_ISSET(crt_ptr, PPOISN)) {
				ANSI(crt_ptr->fd, YELLOW);
            	print(crt_ptr->fd, "You feel life return to your limbs.\n");
				ANSI(crt_ptr->fd, WHITE);
        	}
        }
        F_CLR(crt_ptr, PPOISN);
    }

    return(1);
}

/****************************************************************************/
/*          		  light	   			             	    */
/****************************************************************************/
/* This spell allows a player to cast a light spell which will illuminate   */
/* any darkened room for a period of time (depending on level).       	    */

int light(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd    	    *cmnd;
int         how;

{
    int	fd;

    fd = ply_ptr->fd;
    
    if(!S_ISSET(ply_ptr, SLIGHT) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

    if(how == CAST) {
    if(ply_ptr->mpcur < 2 && how == CAST && ply_ptr->class == MAGE) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }

    if(ply_ptr->mpcur < 5 && how == CAST && ply_ptr->class != MAGE) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }

    	if (F_ISSET(ply_ptr->parent_rom, RPMEXT))
			print(fd,
				"The magic in the air increase the power of your spell.\n");
		if(ply_ptr->class == MAGE)
        	ply_ptr->mpcur -= 1;
		else
        	ply_ptr->mpcur -= 5;
    }

    F_SET(ply_ptr, PLIGHT);
    ply_ptr->lasttime[LT_LIGHT].ltime = time(0);
    ply_ptr->lasttime[LT_LIGHT].interval = 300L + ply_ptr->level * 300L +
		(F_ISSET(ply_ptr->parent_rom, RPMEXT)) ? 600L : 0;
    ANSI(fd, YELLOW);
    print(fd, "You cast a light spell.\n");
    ANSI(fd, WHITE);
    broadcast_rom(fd, ply_ptr->rom_num, "%M casts a light spell.", ply_ptr);

    return(1);
}

/**************************************************************************/
/*         		 protection   				          */
/**************************************************************************/
/* This function allows a spellcaster to cast a protection spell either   */
/* on himself or on another player, improving the armor class by a        */
/* score of 10.                             				  */

int protection(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd    	    *cmnd;
int    	    how;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    long        t;
    int     fd, heal;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;
    t = time(0);

    if(!S_ISSET(ply_ptr, SPROTE) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

    if(ply_ptr->mpcur < 10 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }

    if(spell_fail(ply_ptr, how) && ply_ptr->class != CLERIC &&
		ply_ptr->class != PALADIN && how != POTION) {
        if(how == CAST)
			ply_ptr->mpcur -= 10;
        return(0);
    }

    /* Cast protection on self */
    if(cmnd->num == 2) {
        F_SET(ply_ptr, PPROTE);
        compute_ac(ply_ptr);
        ply_ptr->lasttime[LT_PROTE].ltime = t;
        if(how == CAST) {
            ply_ptr->lasttime[LT_PROTE].interval = MAX(300, 1200 + 
                bonus[ply_ptr->intelligence] * 600);
            if(ply_ptr->class == CLERIC || ply_ptr->class == PALADIN)
                ply_ptr->lasttime[LT_PROTE].interval += 60 * ply_ptr->level;
            ply_ptr->mpcur -= 10;
    		if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
			print(fd,
				"The magic in the air increase the power of your spell.\n");
	    		ply_ptr->lasttime[LT_PROTE].interval += 800L;
			}
        }
        else
            ply_ptr->lasttime[LT_PROTE].interval = 1200;

        if(how == CAST || how == SCROLL || how == WAND)  {
            print(fd,"Protection spell cast.  You feel watched.\n");
            broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts a protection spell on %sself.", ply_ptr,
                      F_ISSET(ply_ptr, PMALES) ? "him":"her");
        }
        else if(how == POTION)
            print(fd, 
				"You feel a warm feeling as a protective aura envelope you.\n");
        return(1);
    }

    /* Cast protection on another player */
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

        F_SET(crt_ptr, PPROTE);
        compute_ac(crt_ptr);

        crt_ptr->lasttime[LT_PROTE].ltime = t;
        if(how == CAST) {
            crt_ptr->lasttime[LT_PROTE].interval = MAX(300, 1200 + 
                bonus[ply_ptr->intelligence]*600);
            if(ply_ptr->class == CLERIC || ply_ptr->class == PALADIN)
                crt_ptr->lasttime[LT_PROTE].interval += 60*ply_ptr->level;
            ply_ptr->mpcur -= 10;
    		if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
				print(fd,
					"The magic in the air increase the power of your spell.\n");
	    		crt_ptr->lasttime[LT_PROTE].interval += 800L;
			}
        }
        else
            crt_ptr->lasttime[LT_PROTE].interval = 1200;

        if(how == CAST || how == SCROLL || how == WAND) {
            print(fd, "Protection cast on %s.\n", crt_ptr->name);
            print(crt_ptr->fd, "%M casts a protection spell on you.\n%s",
				ply_ptr, "You feel watched.\n");
            broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts a protection spell on %m.", ply_ptr, crt_ptr);
            return(1);
        }
    }

    return(1);
}

/************************************************************************/
/*              	mend   				                */
/************************************************************************/
/* This function will cause the mend spell to be cast on a player or    */
/* another monster.  It heals 2d8 hit points plus any bonuses for       */
/* intelligence.  If the healer is a cleric or druid then there is an   */
/* additional point of healing for each level of the player.            */

int mend(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd    	    *cmnd;
int    	    how;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    int     fd, heal;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    if(!S_ISSET(ply_ptr, SMENDW) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

    if(ply_ptr->class != CLERIC && ply_ptr->mpcur < 4 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }
    if(ply_ptr->class == CLERIC && ply_ptr->mpcur < 3 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }

    if(ply_ptr->class == BARBARIAN || ply_ptr->class == FIGHTER || 
	    ply_ptr->class == MONK) {
	        if(spell_fail(ply_ptr, how)) {
	            if(how == CAST && ply_ptr->class != CLERIC)
        	        ply_ptr->mpcur -= 4;
	            if(how == CAST && ply_ptr->class == CLERIC)
        	        ply_ptr->mpcur -= 3;
        		return(0);
			}
    }

    /* Mend self */
    if(cmnd->num == 2) {
        if(how == CAST) {
            heal = MAX(bonus[ply_ptr->intelligence], bonus[ply_ptr->piety]) + 
            ((ply_ptr->class == CLERIC) ? ply_ptr->level + 
            mrand(1, 1 + ply_ptr->level/2) : 0) + 
            ((ply_ptr->class == DRUID) ? ply_ptr->level + 
            mrand(1, 1 + ply_ptr->level/3) : 0) + 
            ((ply_ptr->class == PALADIN) ? ply_ptr->level/2 +
            mrand(1, 1 + ply_ptr->level/3) : 0) +
	    	((ply_ptr->class == BARD) ? ply_ptr->level/4 +
            mrand(1, 1 + ply_ptr->level/5) : 0) + dice(2, 6, 0);

	    	if(ply_ptr->class == CLERIC)
        		ply_ptr->mpcur -= 3;
	    	else
				ply_ptr->mpcur -= 4;

	    	if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
				heal += mrand(2, 8) + 1;
				print(fd,
					"The magic in the air increase the power of your spell.\n");
	    	}
		}
        else
            heal = dice(2, 8, 0);

        heal = MAX(1, heal);

        ply_ptr->hpcur += MAX(1, heal);

        if(ply_ptr->hpcur > ply_ptr->hpmax && !F_ISSET(ply_ptr, PSTRNG))
            ply_ptr->hpcur = ply_ptr->hpmax;
        if(ply_ptr->hpcur > ply_ptr->hpmax && F_ISSET(ply_ptr, PSTRNG)) {
            ply_ptr->hpcur = ply_ptr->hpmax + ply_ptr->level;
			compute_thaco(ply_ptr);
		}

        if(how == CAST || how == SCROLL) {
            print(fd, "Mend-wounds spell cast.\n");
            broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts a mend-wounds spell on %sself.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
            return(1);
        }
        else {
            print(fd, "You feel better.\n");
            return(1);
        }
    }

    /* Cast mend on another player or monster */
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
				cmnd->str[2], cmnd->val[2]);

            if(!crt_ptr) {
                print(fd, "That person is not here.\n");
                return(0);
            }
        }

        if(how == CAST) {
            heal = MAX(bonus[ply_ptr->intelligence], bonus[ply_ptr->piety]) + 
            ((ply_ptr->class == CLERIC) ? ply_ptr->level + 
            mrand(1, 1 + ply_ptr->level/2) : 0) + 
            ((ply_ptr->class == DRUID) ? ply_ptr->level/2 +
            mrand(1, 1 + ply_ptr->level/3) : 0) +
            ((ply_ptr->class == PALADIN) ? ply_ptr->level/2 +
            mrand(1, 1 + ply_ptr->level/3) : 0) +
            dice(2, 6, 0);

	    	if(ply_ptr->class == CLERIC)
                ply_ptr->mpcur -= 3;
	    	else
                ply_ptr->mpcur -= 4;

	    	if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
				heal += mrand(4, 9) + 1;
				print(fd,
					"The magic in the air increase the power of your spell.\n");
	    	}
        }
        else
            heal = dice(4, 9, 0);

        heal = MAX(1, heal);

        crt_ptr->hpcur += MAX(1, heal);

        if(crt_ptr->hpcur > crt_ptr->hpmax && !F_ISSET(crt_ptr, PSTRNG))
            crt_ptr->hpcur = crt_ptr->hpmax;
        if(crt_ptr->hpcur > crt_ptr->hpmax && F_ISSET(crt_ptr, PSTRNG)) {
            crt_ptr->hpcur = crt_ptr->hpmax + crt_ptr->level;
			compute_thaco(ply_ptr);
		}

        if(how == CAST || how == SCROLL || how == WAND) {
            print(fd, "Mend-wounds spell of %d HP cast on %m.\n", 
				heal, crt_ptr);
            print(crt_ptr->fd, 
            	"%M casts a mend-wounds spell on you.\n", ply_ptr);
            broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts a mend-wounds spell on %m.", ply_ptr, crt_ptr);
            if(crt_ptr->hpcur >= crt_ptr->hpmax) {
				print(fd, "%M is at full strength.\n", crt_ptr);
            	return(1);
			}
            if(crt_ptr->hpcur >= crt_ptr->hpmax/2) {
				print(fd, "%M is above half-strength now.\n", crt_ptr);
            	return(1);
        	}
            if(crt_ptr->hpcur < crt_ptr->hpmax/2) {
				print(fd, "%M is still below half-strength.\n", crt_ptr);
            	return(1);
        	}
            return(1);
        }
    }

    return(1);
}
