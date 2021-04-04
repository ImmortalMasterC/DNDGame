/*
 * 	MAGIC8.C:
 *
 *  	Additional spell-casting routines.
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>


/***********************************************************************/
/*                              room_vigor                             */
/***********************************************************************/

int room_vigor(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd         *cmnd;

{          
	object	*obj_ptr;
	char	*sp;
    	int     c = 0, fd, i, n, match = 0, heal;
	ctag	*cp;
	ctag	*cp_tmp;

	fd = ply_ptr->fd;
	
	if(how == POTION) {
		print(fd, "The spell fizzles and your stomach starts to cramp.\n");
		return(0);
	}

	if(!S_ISSET(ply_ptr, SRVIGO) && (how == CAST)) {
       	print(fd, "You don't know that spell.\n");
       	return(0);
   	}  

    if(how == CAST && ply_ptr->class != CLERIC && ply_ptr->class < CARETAKER) {
		print(fd, "Only clerics may cast that spell.\n");
		return(PROMPT);
	}

 	if(ply_ptr->mpcur < 8 && (how == CAST)) {
		print(fd, "Not enough magic points.\n");
		return(0);
    }

	if(how == CAST)
		ply_ptr->mpcur -= 8;
	if(how == WAND) {
    	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
			cmnd->str[1], cmnd->val[1]);

	    if(!obj_ptr || !cmnd->val[1]) {
    	    for(n = 0; n < MAXWEAR; n++) {
        	    if(!ply_ptr->ready[n]) continue;
            	if(EQUAL(ply_ptr->ready[n], cmnd->str[1]))
                	match++;
           		else continue;
            	if(match == cmnd->val[1] || !cmnd->val[1]) {
                	obj_ptr = ply_ptr->ready[n];
               		break;
            	}
        	}
    	}

	    if(!obj_ptr) {
    	    print(fd, "You don't have that.\n");
        	return(0);
    	}
		if(obj_ptr->shotscur < 1) {
			print(fd, "It's used up.\n");
        	return(0);
    	}
		else
			obj_ptr->shotscur--;
    }

	if(how == CAST || how == WAND) {
		cp = ply_ptr->parent_rom->first_ply;
		print(fd,"\nYou cast a vigor spell on everyone in the room.\n");
		if(how == CAST)
			broadcast_rom(fd, ply_ptr->rom_num, 
				"\n%M casts a vigor spell on everyone in the room.\n", ply_ptr);
		else
			broadcast_rom(fd, ply_ptr->rom_num, 
				"\n%M waves a wand and casts a vigor spell on everyone in the room.\n", ply_ptr);
		
		heal = mrand(5, 9) + bonus[ply_ptr->piety];            

		if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
    	   	heal += mrand(1, 3);
       		print(fd,
				"\nThe magic in the air increases the power of your spell\n");
   		}		

		while(cp) {
			cp_tmp = cp->next_tag;
			if(cp->crt->type != MONSTER) {
				if(cp->crt != ply_ptr) 
					print(cp->crt->fd, 
						"\nYou feel new strength flow through your body.\n", 
							ply_ptr);
				cp->crt->hpcur += heal;
				cp->crt->hpcur = MIN(cp->crt->hpmax, cp->crt->hpcur);
			}
			cp = cp_tmp;
		}
	}

	return(0);
}

/************************************************************************/
/*      	 	remove blindness	  		        */
/************************************************************************/
 
int rm_blind(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd    	    *cmnd;
int    	    how;

{
    room        *rom_ptr;
    creature    *crt_ptr;
    int     	fd;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;
 
    if(ply_ptr->mpcur < 12 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }
 
    if(ply_ptr->class != CLERIC && ply_ptr->class != PALADIN &&
       ply_ptr->class != DRUID && ply_ptr->class < CARETAKER && how == CAST) {
           print(fd, 
				"Only clerics, druids and paladins may cast that spell.\n");
           return(0);
    }                         
 
    if(!S_ISSET(ply_ptr, SRMBLD) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

    if(cmnd->num == 2) {
        if(how == CAST)
            ply_ptr->mpcur -= 12;
 
        if(how == CAST || how == SCROLL || how == WAND)  {
            print(fd, "Cure blindness spell cast on yourself.\n");
	    	if(F_ISSET(ply_ptr, PBLIND)) {
	    		ANSI(fd, YELLOW);
        	   	print(fd, "You see the light!.\n");
	    		ANSI(fd, BOLD);
	    		ANSI(fd, WHITE);
        	   	broadcast_rom(fd, ply_ptr->rom_num, 
					"%M casts cure blindness on %sself.", ply_ptr,
						F_ISSET(ply_ptr, PMALES) ? "him":"her");
            }
	    	else {
        	   	print(fd, 
					"hmm... That's a waste.  Nothing wrong with your eyes.\n");
        	   	broadcast_rom(fd, ply_ptr->rom_num, 
					"%M needlessly casts cure blindness on %sself.", ply_ptr,
						F_ISSET(ply_ptr, PMALES) ? "him":"her");
			}
        }
        else if(how == POTION && F_ISSET(ply_ptr, PBLIND)) {
	    	ANSI(fd, YELLOW);
            print(fd, "You feel your eyes begin to clear!\n");
	    	ANSI(fd, BLUEBG);
	    	ANSI(fd, BOLD);
	    	ANSI(fd, WHITE);
		}
        else if(how == POTION && !F_ISSET(ply_ptr, PBLIND))
            print(fd, "It tastes terrible, but nothing happens.\n");
 
        F_CLR(ply_ptr, PBLIND);
        F_CLR(ply_ptr, PNOLDS);
    }
    else {
        if(how == POTION) {
            print(fd, "You can only use a potion on yourself.\n");
            return(0);
        }

        crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
               cmnd->str[2], cmnd->val[2]);
 
        if(!crt_ptr) {
            cmnd->str[2][0] = up(cmnd->str[2][0]);
            crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
               cmnd->str[2], cmnd->val[2]);
 
			if(!crt_ptr || crt_ptr == ply_ptr || strlen(cmnd->str[2]) < 3) {
				print(fd, "That's not here.\n");
				return(0);
			}
        }
 
        if(how == CAST) 
            ply_ptr->mpcur -= 12;
		
		if(crt_ptr->type == PLAYER) {
        	if(how == CAST || how == SCROLL || how == WAND) {
            	print(fd, "You cast cure blindness on %m.\n", crt_ptr);
            	broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
					"%M casts cure blindness on %m.", ply_ptr, crt_ptr);
            	print(crt_ptr->fd, "%M casts cure blindness on you.\n", 
					ply_ptr);

	    		if(F_ISSET(crt_ptr, PBLIND)) {
	    			ANSI(crt_ptr->fd, YELLOW);
	    			print(crt_ptr->fd, 
						"You feel a veil lifted from your eyes.\n");
	    			ANSI(crt_ptr->fd, BOLD);
	    			ANSI(crt_ptr->fd, WHITE);
				}
	    		else {
					print(fd, "Now, why did you bother with that?\n");
					print(fd, "You already have two perfectly good eyes.\n");
       			}
       		}
        	F_CLR(crt_ptr, PBLIND);
       	}

		if(crt_ptr->type != PLAYER) {
        	if(how == CAST || how == SCROLL || how == WAND) {
            	print(fd, "You cast cure blindness on %m.\n", crt_ptr);
            	broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
					"%M casts cure blindness on %m.", ply_ptr, crt_ptr);
        		F_CLR(crt_ptr, MBLIND);
       		}
    	}
    }
 
    return(1);
}
 

/*************************************************************************/
/*                              blind                                 	 */
/*************************************************************************/
/* The blind spell prevents a player or monster from seeing. The spell   */
/* results  in a -5 penalty on attacks, and an inability look at objects */
/* players, rooms, or inventory.  Also a player or monster cannot read.  */
/* Druids are able to reflect the spell when it is cast against them.	 */

int blind(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd    	    *cmnd;
int    	    how;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    int         fd, dur;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;
 
	if(ply_ptr->mpcur < 25 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	if(ply_ptr->mpcur < 30 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

    if(!S_ISSET(ply_ptr, SBLIND) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }
 
    if(F_ISSET(ply_ptr, PINVIS)) {
        F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
        print(fd, "Your invisibility fades.\n");
		ANSI(fd, BLUEBG);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
        broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
    }
 
    if(how == CAST) {
		if(ply_ptr->class == MAGE)
	    	ply_ptr->mpcur -= 20;
		if(ply_ptr->class != MAGE)
		    ply_ptr->mpcur -= 25;
    }
 
    /* blind self */
   if(cmnd->num == 2) {
		F_SET(ply_ptr, PBLIND);
 
        if(how == CAST || how == SCROLL || how == WAND)  {
		    CLS(fd);	
		    ANSI(fd, RED);   	
            print(fd, "You have gone blind in both eyes and can no longer see!\n");
		    ANSI(fd, BOLD);
		    ANSI(fd, WHITE);   	
            broadcast_rom(fd, ply_ptr->rom_num, 
            	"%M casts blindness on %sself.", ply_ptr,
                   	F_ISSET(ply_ptr, PMALES) ? "him":"her");
        }
        else if(how == POTION) {
			CLS(fd);
            print(fd, "Everything goes dark.  You have gone blind in both eyes!\n");
    	}
    }
 
    /* blind a monster or player */
    else {
        if(how == POTION) {
            print(fd, "You can only use a potion on yourself.\n");
            return(0);
        }
 
        crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
               cmnd->str[2], cmnd->val[2]);
 
        if(!crt_ptr) {
            cmnd->str[2][0] = up(cmnd->str[2][0]);
            crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
               cmnd->str[2], cmnd->val[2]);
 
			if(!crt_ptr || crt_ptr == ply_ptr || strlen(cmnd->str[2]) < 3) {
				print(fd, "That's not here.\n");
				return(0);
			}
        }
 
        if(crt_ptr->type != PLAYER && F_ISSET(crt_ptr, MUNKIL)) {
            print(fd, "You cannot harm %s.\n",
                F_ISSET(crt_ptr, MMALES) ? "him":"her");
            return(0);
        }
 
	if(crt_ptr->type == PLAYER && crt_ptr->class != DRUID)
		F_SET(crt_ptr, PBLIND);
	else if(crt_ptr->type != PLAYER)
		F_SET(crt_ptr, MBLIND);
	if(crt_ptr->type == PLAYER && crt_ptr->class == DRUID)
		F_SET(ply_ptr, PBLIND);
 
	if(how == CAST || how == SCROLL || how == WAND) {
		print(fd, "Blindness spell cast on %s.\n", crt_ptr->name);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
			"%M casts a blindness spell on %m.", ply_ptr, crt_ptr);
	    CLS(crt_ptr->fd);
	    ANSI(fd, RED);
		print(crt_ptr->fd, "%M casts a blindness spell on you.\n", ply_ptr);
	    ANSI(fd, WHITE);
	}
        
	if((how == CAST || how == SCROLL || how == WAND) && 
		crt_ptr->class == DRUID) {
            print(fd, "%m reflects the blindness spell back on you!\n",
		 		crt_ptr->name);
            broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
            	"%M reflects the blindness spell back to %m.", 
					crt_ptr, ply_ptr);
            print(crt_ptr->fd, "You reflect the blindness spell back against %M.\n", ply_ptr);
        }
 
        if(crt_ptr->type != PLAYER)
		add_enm_crt(ply_ptr->name, crt_ptr);
    }
 
    return(1);
}            

/************************************************************************/
/*                      fear                                 		*/
/************************************************************************/
/* The fear spell causes the monster to have a high wimpy / flee   	*/
/* percentage and a penalty of -2 on all attacks 			*/
 
int fear(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd         *cmnd;
int         how;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    int         fd, dur;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;
 
	if(ply_ptr->mpcur < 7 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	if(ply_ptr->mpcur < 15 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

    if(!S_ISSET(ply_ptr, SFEARS) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }
 
    if(F_ISSET(ply_ptr, PINVIS)) {
        F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
        print(fd, "Your invisibility fades.\n");
		ANSI(fd, BLUEBG);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
        broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
    }
 
    if(how == CAST) {
        dur =  300 + mrand(1, 30) * 10  + bonus[ply_ptr->intelligence] * 12;
		if(ply_ptr->class == MAGE)
			ply_ptr->mpcur -= 7;
		if(ply_ptr->class != MAGE)
			ply_ptr->mpcur -= 15;
	}
    else if(how == SCROLL)
        dur =  300 + mrand(1, 15) * 10  + bonus[ply_ptr->intelligence] * 12;
    else 
        dur = 300 + mrand(1, 30) * 10;
 
	if(spell_fail(ply_ptr, how)) 
		return(0);
   
 /* fear on self */
   if(cmnd->num == 2) {
        if(F_ISSET(ply_ptr, PRMAGI))
        	dur /= 2;
 
        ply_ptr->lasttime[LT_FEARS].ltime = time(0);
        ply_ptr->lasttime[LT_FEARS].interval = dur;
		F_SET(ply_ptr, PFEARS);
 
        if(how == CAST || how == SCROLL || how == WAND)  {
            print(fd, "You begin to shake in terror.\n");
            broadcast_rom(fd, ply_ptr->rom_num, 
		"%M casts a fear spell on %sself.", 
			ply_ptr, F_ISSET(ply_ptr, PMALES) ? "him":"her");
        }
        else if(how == POTION)
            print(fd, "You begin to shake in terror.\n");
 
    }
 
    /* fear on a monster or player */
    else {
        if(how == POTION) {
            print(fd, "You can only use a potion on yourself.\n");
            return(0);
        }
 
        crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
			cmnd->str[2], cmnd->val[2]);
 
        if(!crt_ptr) {
            cmnd->str[2][0] = up(cmnd->str[2][0]);
            crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
				cmnd->str[2], cmnd->val[2]);
 
            if(!crt_ptr || crt_ptr == ply_ptr || strlen(cmnd->str[2]) < 3) {
                print(fd, "That's not here.\n");
                return(0);
            }
        }
 
        if(crt_ptr->type != PLAYER && F_ISSET(crt_ptr, MUNKIL)) {
            print(fd, "You cannot harm %s.\n",
                F_ISSET(crt_ptr, MMALES) ? "him":"her");
            return(0);
        }
 
        if(crt_ptr->type != PLAYER && F_ISSET(crt_ptr, MPERMT)) {
            print(fd, "%M seems unaffected by a fear spell.\n", crt_ptr);
            return(0);
        }

        if((crt_ptr->type == PLAYER && F_ISSET(crt_ptr, PRMAGI)) ||
           (crt_ptr->type != PLAYER && F_ISSET(crt_ptr, MRMAGI)))
            	dur /= 2;
 
        crt_ptr->lasttime[LT_FEARS].ltime = time(0);
        crt_ptr->lasttime[LT_FEARS].interval = dur;
		if(crt_ptr->type == PLAYER)
			F_SET(crt_ptr, PFEARS);
		else
			F_SET(crt_ptr, MFEARS);
 
        if(how == CAST || how == SCROLL || how == WAND) {
            print(fd, "A fear spell is cast on %s.\n", crt_ptr->name);
            broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts a fear spell on %m.", ply_ptr, crt_ptr);
            print(crt_ptr->fd, "%M casts a fear spell on you.\n", ply_ptr);
        }
 
        if(crt_ptr->type != PLAYER)
		add_enm_crt(ply_ptr->name, crt_ptr);
    }
 
    return(1);
}            

/************************************************************************/
/*                   silence                                 		*/
/************************************************************************/
/* Silence  causes a player or monster to lose their voice, making them	*/
/* unable to cast spells, use scrolls, speak, yell, or broadcast 	*/

int silence(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd    	    *cmnd;
int         how;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    int         fd, dur;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;
 
    if(ply_ptr->mpcur < 25 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }
 
    if(!S_ISSET(ply_ptr, SSILNC) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }
 
    if(F_ISSET(ply_ptr, PINVIS)) {
        F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
        print(fd, "Your invisibility fades.\n");
		ANSI(fd, BLUEBG);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
        broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
    }
 
    if(how == CAST) {
        dur =  300 + mrand(1, 30) * 10  + bonus[ply_ptr->intelligence] * 150;
        ply_ptr->mpcur -= 25;
	}
    else if(how == SCROLL)
        dur =  300 + mrand(1, 15) * 10  + bonus[ply_ptr->intelligence] * 75;
    else 
        dur = 300 + mrand(1, 15) * 10;
 
	if(spell_fail(ply_ptr, how)) 
		return(0);

    /* silence on self */
   if(cmnd->num == 2) {
        if(F_ISSET(ply_ptr, PRMAGI))
			dur /= 4;
 
        ply_ptr->lasttime[LT_SILNC].ltime = time(0);
        ply_ptr->lasttime[LT_SILNC].interval = dur;
		F_SET(ply_ptr, PSILNC);
 
        if(how == CAST || how == SCROLL || how == WAND)  {
            print(fd, "Your voice begins to fade.\n");
            broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts silence on %sself.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
        }
        else if(how == POTION)
            print(fd, "Your throat goes dry and you cannot speak.\n");
    }
 
    /* silence a monster or player */
    else {
        if(how == POTION) {
            print(fd, "You can only use a potion on yourself.\n");
            return(0);
        }
 
        crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
			cmnd->str[2], cmnd->val[2]);
 
        if(!crt_ptr) {
            cmnd->str[2][0] = up(cmnd->str[2][0]);
            crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
				cmnd->str[2], cmnd->val[2]);
 
            if(!crt_ptr || crt_ptr == ply_ptr || strlen(cmnd->str[2]) < 3) {
                print(fd, "That's not here.\n");
                return(0);
            }
        }
 
        if(crt_ptr->type != PLAYER && F_ISSET(crt_ptr, MUNKIL)) {
            print(fd, "You cannot harm %s.\n",
                F_ISSET(crt_ptr, MMALES) ? "him":"her");
            return(0);
        }
 
        if((crt_ptr->type == PLAYER && F_ISSET(crt_ptr, PRMAGI)) ||
           (crt_ptr->type != PLAYER && F_ISSET(crt_ptr, MRMAGI)))
           		dur /= 2;
 
        crt_ptr->lasttime[LT_SILNC].ltime = time(0);
        crt_ptr->lasttime[LT_SILNC].interval = dur;
		if(crt_ptr->type == PLAYER)
			F_SET(crt_ptr, PSILNC);
		else
			F_SET(crt_ptr, MSILNC);
 
        if(how == CAST || how == SCROLL || how == WAND) {
            print(fd, "Silence cast on %s.\n", crt_ptr->name);
            broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts a silence spell on %m.", ply_ptr, crt_ptr);
            print(crt_ptr->fd, "%M casts a silence spell on you.\n", ply_ptr);
        }
 
        if(crt_ptr->type != PLAYER)
            add_enm_crt(ply_ptr->name, crt_ptr);
    }
 
    return(1);
}            
              
/************************************************************************/
/*                      remove silence                              	*/
/************************************************************************/
 
int rm_silence(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd    	    *cmnd;
int    	    how;

{
    room        *rom_ptr;
    creature    *crt_ptr;
    int     	fd;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;
 
    if(ply_ptr->mpcur < 12 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }
 
        if(ply_ptr->class != CLERIC && ply_ptr->class != PALADIN &&
           ply_ptr->class < CARETAKER && how == CAST) {
                print(fd, "Only clerics and paladins may cast that spell.\n");
                return(0);
        }                         
 
    if(!S_ISSET(ply_ptr, SRMSIL) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }
 
    if(cmnd->num == 2) {
        if(how == CAST)
            ply_ptr->mpcur -= 12;
 
        if(how == CAST || how == SCROLL || how == WAND)  {
            print(fd, "Remove silence spell cast on yourself.\n");
  	    	ANSI(fd, YELLOW);	
            print(fd, "You can speak again!\n");
	    	ANSI(fd, BLUEBG);
	    	ANSI(fd, BOLD);
  	    	ANSI(fd, WHITE);	
            broadcast_rom(fd, ply_ptr->rom_num, 
				"%M cast remove silence on %sself.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
        }
        else if(how == POTION & F_ISSET(ply_ptr, PSILNC))
            print(fd, "Your silence is lifted!\n");
        else if(how == POTION)
            print(fd, "Hmmm, tastes bad, but nothing happens.\n");
 
        F_CLR(ply_ptr, PSILNC);
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
			cmnd->str[2], cmnd->val[2]);
 
            if(!crt_ptr) {
                print(fd, "That's not here.\n");
                return(0);
            }
        }
 
        if(how == CAST) 
            ply_ptr->mpcur -= 12;
 
		if (crt_ptr->type == PLAYER)
			F_CLR(crt_ptr, PSILNC);
		else
			F_CLR(crt_ptr, MSILNC);
 
        if(how == CAST || how == SCROLL || how == WAND) {
            print(fd, "You cast the remove silence spell on %m.\n", crt_ptr);
            broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M cast remove silence on %m.", ply_ptr, crt_ptr);
            print(crt_ptr->fd, "%M cast remove silence on you.\n", ply_ptr);
	    	print(crt_ptr->fd, "Your ability to speak returns.\n");
        }
    }
 
    return(1);
}

/************************************************************************/
/*   			    hypnosis                              	*/
/************************************************************************/
/*  This function may or may not be complete.  It was not included	*/
/*  in the version 3.0 release, and neither was remove silence.		*/
/*  There was an obvious error - remove blindness was included twice.	*/
/*  So, comment out and check later.	-styx-				*/
/*

int hypnosis(ply_ptr, cmnd, how)
creature    *ply_ptr;
cmd    	    *cmnd;
int	    how;

{
    room        *rom_ptr;
    creature    *crt_ptr;
    int     	fd, n, dur;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

    if(ply_ptr->mpcur < 12 && how == CAST) {
        print(fd, "Not enough magic points.\n");
        return(0);
    }
     
    if(!S_ISSET(ply_ptr, SHYPNO) && how == CAST) {
        print(fd, "You don't know that spell.\n");
        return(0);
    }

    if(how == CAST) {
        dur =  300 + mrand(1, 30) * 10  + bonus[ply_ptr->intelligence] * 150;
    }
    else if (how == SCROLL)
        dur =  100 + mrand(1, 15) * 10  + bonus[ply_ptr->intelligence] * 75;
    else
        dur = 100 + mrand(1, 15) * 10;
	
    if(cmnd->num == 2) {
        if(how == CAST)
            ply_ptr->mpcur -= 15;

		ply_ptr->lasttime[LT_CHRMD].ltime = time(0);
        ply_ptr->lasttime[LT_CHRMD].interval = dur;

        if(how == CAST || how == SCROLL || how == WAND)  {
            print(fd, "You feel so much better about yourself now.\n");
            broadcast_rom(fd, ply_ptr->rom_num,
				"%M casts hypnosis on %sself.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
        }
        else if(how == POTION)
            print(fd, "You feel so much better about yourself now.\n");
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
				cmnd->str[2], cmnd->val[2]);

            if(!crt_ptr) {
                print(fd, "That's not here.\n");
                return(0);
            }
        }

	if(F_ISSET(crt_ptr, MUNDED)) {
		print(fd, "Your spell has no effect on %m.\n", crt_ptr);
		add_enm_crt(ply_ptr->name, crt_ptr);
		return(0);
	}		
		
	if(how == CAST)
        ply_ptr->mpcur -= 15;

        if((crt_ptr->type == PLAYER && F_ISSET(crt_ptr, PRMAGI)) ||
           (crt_ptr->type != PLAYER && F_ISSET(crt_ptr, MRMAGI)))
            	dur /= 2;

        if(how == CAST || how == SCROLL || how == WAND) {
            print(fd, "You cast hypnosis on %m.\n", crt_ptr);
            broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts hypnosis on %m.", ply_ptr, crt_ptr);
            print(crt_ptr->fd, "%M hypnotizes you.\n", ply_ptr);
            add_charm_crt(crt_ptr, ply_ptr);

	    	crt_ptr->lasttime[LT_CHRMD].ltime = time(0);
        	crt_ptr->lasttime[LT_CHRMD].interval = dur;

	    	if(crt_ptr->type == PLAYER)
     	    	F_SET(crt_ptr, PCHARM);
	    	else 
				F_SET(crt_ptr, MCHARM);

		}
    }

    return(1);i			*/
/*  END of commented-out hypnosis code.		*/

/****************************************************************************/
/*			spell_fail					    */
/****************************************************************************/
/* This function returns 1 if the casting of a spell fails, and 0 if it is  */
/* sucessful.								    							*/

int spell_fail(ply_ptr, how)
creature	*ply_ptr;
int	 	how;

{

	int	chance, fd;

	if(how == POTION)
		return(0);

	if(ply_ptr->type != PLAYER || ply_ptr->class == MAGE)
		return(0);

        chance = ply_ptr->level + Ply[ply_ptr->fd].extr->luck;
	if(mrand(1, 25) > chance) {
		print(fd,"There's a flash of green light and your spell fails.\n");
		return(1);
	}
	else
		return(0);
}


/****************************************************************************/
/*                      fortune                                             */
/****************************************************************************/
/* This allows bards to tell the luck of a given player.		    */

int fortune(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd        	*cmnd;
int		how;

{

	int		fd, luck;
	creature	*crt_ptr;
	room		*rom_ptr;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	
	if(ply_ptr->class != BARD && ply_ptr->class < CARETAKER) {
		print(fd, "Only bards may do that.\n");
		return(0);
	}

	if(ply_ptr->mpcur < 2 && how == CAST) {
       	print(fd, "Not enough magic points.\n");
       	return(0);
	}

	if(!S_ISSET(ply_ptr, SFORTU) && how == CAST) {
       	print(fd, "You don't know that spell.\n");
       	return(0);
    	}
    	
    if(cmnd->num == 2) {
		if(how == CAST)
            ply_ptr->mpcur -= 2;

        if(how == CAST || how == SCROLL || how == WAND)  {
            print(fd, "Fortune spell cast on yourself.\n");

			F_SET(ply_ptr, PDETLK);

	    	if(Ply[ply_ptr->fd].extr->luck <= 35)
				print(fd, "Your luck is really bad!\n");
	
	    	if(Ply[ply_ptr->fd].extr->luck > 35 && 
				Ply[ply_ptr->fd].extr->luck <= 45)
					print(fd, "Your luck is not very good.\n");

	    	if(Ply[ply_ptr->fd].extr->luck > 45 && 
				Ply[ply_ptr->fd].extr->luck <= 50)
					print(fd, "Your luck is just below average.\n");

	    	if(Ply[ply_ptr->fd].extr->luck > 50 && 
				Ply[ply_ptr->fd].extr->luck <= 55)
					print(fd, "Your luck is just above average.\n");

	    	if(Ply[ply_ptr->fd].extr->luck > 55 && 
				Ply[ply_ptr->fd].extr->luck <= 65)
					print(fd, "Your luck is very good.\n");

	    	if(Ply[ply_ptr->fd].extr->luck > 65)
				print(fd, "Your luck is exceptionally good!\n");

			broadcast_rom(fd, ply_ptr->rom_num,
				"%M casts a spell to divine %s luck.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "his":"her");
       	}
        else if(how == POTION)
            print(fd, "Nothing happens.\n"); 
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
                print(fd, "Your friend is not here.\n");
                return(0);
            }
        }

		if(crt_ptr->type == MONSTER)
			luck = crt_ptr->alignment/10;
		else 
			luck = Ply[crt_ptr->fd].extr->luck/10;
		if(luck < 1)
			luck = 1;
	
		if(ply_ptr->mpcur < 4 && how == CAST) {
       		print(fd, "Not enough magic points.\n");
       		return(0);
		}
		ply_ptr->mpcur -= 4;

        print(fd, "Fortune spell cast on %s.\n", crt_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M divines the luck of %s.\n", ply_ptr, crt_ptr); 
		F_SET(crt_ptr, PDETLK);

		switch(luck) {
			case 1:
				print(fd, "%M has the worst of possible luck!\n", crt_ptr);
				print(crt_ptr->fd, "You have the worst of possible luck!\n", 
					crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num,
					"A terrible black pall hang over %m.", crt_ptr);
				break;

			case 2:
				print(fd, "%M has terrible luck!\n", crt_ptr);
				print(crt_ptr->fd, "You have terrible luck!\n", crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num,
					"A black pall hang over %m.", crt_ptr);
				break;

			case 3:
				print(fd, "%M has very bad luck!\n", crt_ptr);
				print(crt_ptr->fd, "You have very bad luck!\n", crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num,
					"A dark pall hang over %m.", crt_ptr);
				break;

			case 4:
				print(fd, "%M's luck is not very good.\n", crt_ptr);
				print(crt_ptr->fd, "Your luck is not very good.\n", crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num,
					"A gray pall hang over %m.", crt_ptr);
				break;

			case 5:
				print(fd, "%M has about average luck.\n", crt_ptr);
				print(crt_ptr->fd, "You have about average luck.\n", crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num,
					"A soft glow surrounds %m.", crt_ptr);
				break;

			case 6:
				print(fd, "%M has better than average luck.\n", crt_ptr);
				print(crt_ptr->fd, "You have better than average luck.\n", 
					crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num,
					"A warm light illuminates %m.", crt_ptr);
				break;

			case 7:
				print(fd, "%M has very good luck!\n", crt_ptr);
				print(crt_ptr->fd, "You have very good luck!\n", crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num,
					"A bright aura surrounds %m.", crt_ptr);
				break;

			case 8:
				print(fd, "%M has very good luck!\n", crt_ptr);
				print(crt_ptr->fd, "You have very good luck!\n", crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num,
					"A bright and protective aura surrounds %m.", crt_ptr);
				break;

			case 9:
				print(fd, "%M has exceptionally good luck!\n", crt_ptr);
				print(crt_ptr->fd, "You have exceptionally good luck!\n", 
					crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num,
					"A brilliant protective aura surrounds %m.", crt_ptr);
				break;

			case 10:
				print(fd, "%M has exceptionally good luck!\n", crt_ptr);
				print(crt_ptr->fd, "You have exceptionally good luck!\n", 
					crt_ptr);
				break;

			default:
				print(fd, "There is something mysterious here, "
					"and you cannot divine %M's luck.\n", crt_ptr);
				print(crt_ptr->fd, "For some mysterious reason, your luck "
					"cannot be determined right now.\n");
		}

		return(0);
	}
}
