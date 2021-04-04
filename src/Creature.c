/*
 * CREATURE.C:
 *
 *  Routines that act on creatures.
 *
 *  Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"

/************************************************************************/
/*  			            find_crt 		 			                */
/************************************************************************/
/* This function will attempt to locate a given creature within a given */
/* list.  The first parameter contains a pointer to the creature doing  */
/* the search, the second contains a tag pointer to the first tag in    */
/* the list.  The third contains the match string, and the fourth       */
/* contains the number of times to match the string.            		*/

creature *find_crt(ply_ptr, first_ct, str, val)
creature    *ply_ptr;
ctag        *first_ct;
char        *str;
int     	val;

{
    ctag    *cp;
    int match = 0, found = 0;

    if(!ply_ptr)
		return(0);

    if(!str)
	return(0);
	
    cp = first_ct;
    while(cp) {
        if(!cp->crt) {
		cp = cp->next_tag;
           	continue;
		}

		if(cp->crt->class >= CARETAKER && F_ISSET(cp->crt, PDMINV)) {
			cp = cp->next_tag;
			continue;
		}
		if(EQUAL(cp->crt, str) && ((F_ISSET(ply_ptr, PDINVI) || 
			ply_ptr->class >= CARETAKER) ?  1:!F_ISSET(cp->crt, MINVIS))) {
    	        match++;
    	        if(match == val) {
    	            found = 1;
    	            break;
    	        }
		}
        cp = cp->next_tag;
    }

    if(found)
        return(cp->crt);
    else
        return(0);
}

/***************************************************************************/
/* 		            			 add_enm_crt   		 			           */
/***************************************************************************/
/* This function adds a new enemy's name to the top of a creature's enemy  */
/* list.  The closer to the top of the list, the higher the chance the     */
/* player will be attacked.   				                   			   */

int add_enm_crt(enemy, crt_ptr)
char        *enemy;
creature    *crt_ptr;

{
    etag    *ep;
    int 	n;

    n = del_enm_crt(enemy, crt_ptr);

    ep = 0;
    ep = (etag *)malloc(sizeof(etag));
    if(!ep)
        merror("add_enm_crt", FATAL);
    
    strcpy(ep->enemy, enemy);
    ep->next_tag = 0;
    ep->damage = (n > -1) ? n : 0;

    if(!crt_ptr->first_enm) {
        crt_ptr->first_enm = ep;
        return(n);
    }

    ep->next_tag = crt_ptr->first_enm;
    crt_ptr->first_enm = ep;

    if(n < 0) crt_ptr->NUMHITS = 0;

	F_CLR(crt_ptr, MHIDDN);

    return(n);
}

/**********************************************************************/
/*  		           			 del_enm_crt 			              */
/**********************************************************************/
/* This function removes an enemy's name from his enemy list.         */

int del_enm_crt(enemy, crt_ptr)
char        *enemy;
creature    *crt_ptr;

{
    etag    *ep, *prev;
    int 	dmg;

    ep = crt_ptr->first_enm;
    if(!ep) return(-1);

    if(!strcmp(ep->enemy, enemy)) {
        crt_ptr->first_enm = ep->next_tag;
        dmg = ep->damage;
        free(ep);
        return(dmg);
    }

    while(ep) {
        if(!strcmp(ep->enemy, enemy)) {
            prev->next_tag = ep->next_tag;
            dmg = ep->damage;
            free(ep);
            return(dmg);
        }
        prev = ep;
        ep = ep->next_tag;
    }

    return(-1);
}

/*************************************************************************/
/*          		   			 end_enm_crt  	    			         */
/*************************************************************************/
/* This function moves an enemy name on a monster's enemy list to the    */
/* very end of the list.                         			 			 */

void end_enm_crt(enemy, crt_ptr)
char        *enemy;
creature    *crt_ptr;

{
    etag    *ep, *move;
    
    move = 0;
    move = (etag *)malloc(sizeof(etag));
    if(!move)
        merror("end_enm_crt", FATAL);

    strcpy(move->enemy, enemy);
    move->next_tag = 0;
    move->damage =  del_enm_crt(enemy, crt_ptr);

    ep = crt_ptr->first_enm;
    if(!ep) {
        crt_ptr->first_enm = move;
        return;
    }

    while(ep->next_tag)
        ep = ep->next_tag;

    ep->next_tag = move;
}

/**************************************************************************/
/*   		          			 add_enm_dmg 		  			          */
/**************************************************************************/
/* This function adds the amount of damage indicated by the third	  	  */
/* argument to the enemy's total for the creature pointed to by the	  	  */
/* second argument.  The first argument contains the name of the player	  */
/* who hit the creature.                        						  */

void add_enm_dmg(enemy, crt_ptr, dmg)
char        *enemy;
creature    *crt_ptr;
int     	dmg;

{
    room    *rom_ptr;
    etag    *ep;

    ep = crt_ptr->first_enm;
    while(ep) {
        if(!strcmp(enemy, ep->enemy))
            ep->damage += dmg;
        ep = ep->next_tag;
    }

    if(crt_ptr->hpcur <= crt_ptr->hpmax/10 && crt_ptr->hpcur >= 1) {
		rom_ptr = crt_ptr->parent_rom;
		if(!F_ISSET(crt_ptr, MNOPRE))
    		broadcast_rom(-1, rom_ptr->rom_num, "%1M screams in pain!\n",
	   			crt_ptr);
		else
       		broadcast_rom(-1, rom_ptr->rom_num, "%M screams in pain!\n",
	   			crt_ptr);
    }
}

/************************************************************************/
/*     					 		   is_enm_crt 	 		                */
/************************************************************************/
/* This function returns true if the name passed in the first parameter */
/* is in the enemy list of the creature pointed to by the second.       */

int is_enm_crt(enemy, crt_ptr)
char        *enemy;
creature    *crt_ptr;

{
    etag    *ep;

    ep = crt_ptr->first_enm;

    while(ep) {
        if(!strcmp(ep->enemy, enemy))
            return(1);
        ep = ep->next_tag;
    }

    return(0);
}

/************************************************************************/
/*          					    die   		  		                */
/************************************************************************/
/* This function is called whenever a player or a monster dies.  The    */
/* first parameter is a pointer to the creature that has died.  	    */
/* The second parameter is a pointer to the creature who did the dirty  */
/* deed.  Different chains of events are taken depending on whether     */
/* the deceased was a player or a monster.             		     	    */

void die(crt_ptr, att_ptr)
creature    *crt_ptr;
creature    *att_ptr;

{
    otag        *op, *temp;
    etag        *ep;
    ctag		*cp, *prev;
    creature    *ply_ptr;
    room        *rom_ptr;
    object      *obj_ptr;
    char        str[2048];
    long        i, t, profloss, total, xpv;
    int     	n, levels = 0, expdiv, lessthanzero = 0, wielding = 0;

   str[0] = 0;
/*----------------------------------------------------------------*/
/*           A monster dies                                       */
/*----------------------------------------------------------------*/

    if(crt_ptr->type == MONSTER) {
        ep = crt_ptr->first_enm;
        while(ep) {
            ply_ptr = find_who(ep->enemy);
            if(ply_ptr) 
				levels += ply_ptr->level;
            ep = ep->next_tag;
        }
		if(levels > 1)
     	   expdiv = crt_ptr->experience/levels;
        ep = crt_ptr->first_enm;
        while(ep) {
            ply_ptr = find_who(ep->enemy);
            if(ply_ptr) {
                expdiv = (crt_ptr->experience * ep->damage) /
                    MAX(crt_ptr->hpmax, 1);
                expdiv = MIN(expdiv, crt_ptr->experience);
				ANSI(ply_ptr->fd, CYAN);
                print(ply_ptr->fd,
            	    "You gained %d experience for the death of %m.\n",
               		     expdiv, crt_ptr);
				ANSI(ply_ptr->fd, BOLD);
				ANSI(ply_ptr->fd, WHITE);
                ply_ptr->experience += expdiv;
                ply_ptr->alignment -= crt_ptr->alignment/10;
                if(ply_ptr->alignment > 1000)
                    ply_ptr->alignment = 1000;
                if(ply_ptr->alignment < -1000)
                    ply_ptr->alignment = -1000;
            }
            ep = ep->next_tag;
        }

		if(!F_ISSET(crt_ptr, MTRADE)) {
        	sprintf(str, "%s dropped: ", crt_str(crt_ptr, 0, CAP|INV));
        	n = strlen(str);
        	i = list_obj(&str[n], att_ptr, crt_ptr->first_obj);
			if(F_ISSET(crt_ptr->parent_rom, RSLIME))  
   	    		broadcast_rom(-1, crt_ptr->rom_num,
   	        		"The slime on the floor writhes hungrily, ready to "
					"devour anything!");
		}

        if(F_ISSET(crt_ptr, MPERMT))
            die_perm_crt(crt_ptr);

        op = crt_ptr->first_obj;
        while(op) {
            temp = op->next_tag;
            obj_ptr = op->obj;
            del_obj_crt(obj_ptr, crt_ptr);
            if(!F_ISSET(crt_ptr, MTRADE) && 
				!F_ISSET(crt_ptr->parent_rom, RSLIME))  
	    	    	add_obj_rom(obj_ptr, crt_ptr->parent_rom);
	    	op = temp;
        }

        if(crt_ptr->gold) {
            load_obj(0, &obj_ptr);
			if(crt_ptr->gold > 1)
            	sprintf(obj_ptr->name, "%d gold coins", crt_ptr->gold);
			else 
            	sprintf(obj_ptr->name, "%d gold coin", crt_ptr->gold);
            if(i)
                strcat(str, ", ");
            strcat(str, obj_ptr->name);
            obj_ptr->value = crt_ptr->gold;
            if(!F_ISSET(crt_ptr->parent_rom, RSLIME))  
                add_obj_rom(obj_ptr, crt_ptr->parent_rom);
        }
	
		if(F_ISSET(crt_ptr, MDMFOL) && crt_ptr->type == MONSTER) {	
			ply_ptr = crt_ptr->following;
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

			Ply[ply_ptr->fd].extr->alias_crt = 0;
			F_CLR(Ply[ply_ptr->fd].ply, PALIAS);
			ANSI(ply_ptr->fd, GREEN);
			print (ply_ptr->fd, "%M's body has been destroyed.\n", crt_ptr);
        	ANSI(ply_ptr->fd, BOLD);
        	ANSI(ply_ptr->fd, WHITE);
			crt_ptr->following = 0;
		}

		if(is_charm_crt(crt_ptr->name, att_ptr))
			del_charm_crt(crt_ptr, att_ptr);
        del_crt_rom(crt_ptr, crt_ptr->parent_rom);
		del_active(crt_ptr);
        free_crt(crt_ptr);
        if(strlen(str) > n)
            print(att_ptr->fd, "%s.\n", str);
    }

/*----------------------------------------------------------------*/
/*           A player dies                                        */
/*----------------------------------------------------------------*/

    else if(crt_ptr->type == PLAYER) {
        rom_ptr = crt_ptr->parent_rom;
        i = LT(crt_ptr, LT_PLYKL);
        t = time(0);

		if(att_ptr->special != SHD) {
        	if(att_ptr->type != PLAYER || att_ptr == crt_ptr) {
            	if(crt_ptr->level < 4)
                	crt_ptr->experience -= crt_ptr->experience/10;
            	else {
                	crt_ptr->experience -= crt_ptr->experience/5;
                	n = crt_ptr->level - exp_to_lev(crt_ptr->experience);
					if(n > 1)
						if( crt_ptr->level < (MAXALVL + 2))
							crt_ptr->experience = 
							needed_exp[crt_ptr->level - 3];
					else
			    		crt_ptr->experience = 
						(long)((needed_exp[MAXALVL - 1]*(crt_ptr->level - 2)));
            	}
        	}
        }

        crt_ptr->lasttime[LT_PLYKL].ltime = 0L;
        crt_ptr->lasttime[LT_PLYKL].interval = 0L;

        if(att_ptr->type == PLAYER) {
            att_ptr->lasttime[LT_PLYKL].ltime = t;
            att_ptr->lasttime[LT_PLYKL].interval = 
                (long)mrand(7, 14) * 86400L;
	    	if(F_ISSET(att_ptr, PPLDGK) && F_ISSET(crt_ptr, PPLDGK) && 
				(crt_ptr->name != att_ptr->name))
				if(F_ISSET(att_ptr, PKNGDM) != F_ISSET(crt_ptr, PKNGDM)) {
		    		xpv = (crt_ptr->level * crt_ptr->level) * 10;
		    		print(crt_ptr->fd, "You have been bested and lose %d experience.\n", xpv);
		    		crt_ptr->experience -= xpv;
		    		if(crt_ptr->experience <= 0) crt_ptr->experience = 0;
		    			xpv /= 2;
		    		print(att_ptr->fd, "You have vanquished %m\n",crt_ptr);
		    		print(att_ptr->fd, "You gain %d for your heroic deed.\n", 
						xpv);
		    		print(att_ptr->fd, "Prince %s will be proud of you.\n",
						!F_ISSET(att_ptr, PKNGDM) ? "Ragnar" : "Rutger");
		    		att_ptr->experience += xpv;
		    		add_prof(att_ptr, xpv);
				}
				else {
		    		xpv = (crt_ptr->level * crt_ptr->level) * 5;
		    		xpv = MIN(att_ptr->experience, xpv);
		    		crt_ptr->experience -= xpv; 
		    		if(crt_ptr->experience <= 0) 
						crt_ptr->experience = 0;
		    		print(crt_ptr->fd, 
						"You are killed by a fellow pledge of Prince %s!\n", 
							!F_ISSET(crt_ptr, PKNGDM) ? "Ragnar" : "Rutger");
		    		print(crt_ptr->fd, "You lose %d experience!\n", xpv);
		    		xpv = (crt_ptr->level * crt_ptr->level) * 10;
		    		xpv = MIN(att_ptr->experience, xpv);
		    		print(att_ptr->fd, 
						"You killed a fellow pledge of Prince %s!\n",
							!F_ISSET(att_ptr, PKNGDM) ? "Ragnar" : "Rutger");
		    		print(att_ptr->fd, "You lose %d experience!\n", xpv);
		    		att_ptr->experience -= xpv;
		    		if(att_ptr->experience <= 0) 
						att_ptr->experience = 0;
		    		lower_prof(att_ptr, xpv);
				}
		}
        else
            del_enm_crt(crt_ptr->name, att_ptr);

        for(n = 0, total = 0L; n < 5; n++)
            total += crt_ptr->proficiency[n];

        for(n = 0; n < 4; n++)
            total += crt_ptr->realm[n];
        profloss = MAX(0L, total - crt_ptr->experience - 1024L);

        while(profloss > 9L && lessthanzero < 9) {
            lessthanzero = 0;
            for(n = 0; n < 9; n++) {
                if(n < 5) {
                    crt_ptr->proficiency[n] -= profloss/(9 - n);
                    profloss -= profloss/(9 - n);
                    if(crt_ptr->proficiency[n] < 0L) {
                        lessthanzero++;
                        profloss -= crt_ptr->proficiency[n];
                        crt_ptr->proficiency[n] = 0L;
                    }
                }
                else {
                    crt_ptr->realm[n - 5] -= profloss/(9 - n);
                    profloss -= profloss/(9 - n);
                    if(crt_ptr->realm[n - 5] < 0L) {
                        lessthanzero++;
                        profloss -= crt_ptr->realm[n - 5];
                        crt_ptr->realm[n - 5] = 0L;
                    }
                }
            }
        }

        for(n = 1, total = 0; n < 5; n++)
            if(crt_ptr->proficiency[n] >
                crt_ptr->proficiency[total]) 
					total = n;
        	if(crt_ptr->proficiency[total] < 1024L)
            	crt_ptr->proficiency[total] = 1024L;

        	n = exp_to_lev(crt_ptr->experience);
        	while(crt_ptr->level > n)
            	down_level(crt_ptr);

			if(att_ptr->type == PLAYER) {
				crt_ptr->hpcur = crt_ptr->hpmax;
        		crt_ptr->mpcur = MAX(crt_ptr->mpcur, (crt_ptr->mpmax)/10);
			}
			else {
				crt_ptr->hpcur = crt_ptr->hpmax;
        		crt_ptr->mpcur = crt_ptr->mpmax;
			}

			if(F_ISSET(crt_ptr, PSTRNG)){
				crt_ptr->strength -= 5;
			}
			if(F_ISSET(crt_ptr, PBERSK)){
				crt_ptr->strength -= 3;
				ply_ptr->dexterity -= 5;
			}

        	F_CLR(crt_ptr, PPOISN);
        	F_CLR(crt_ptr, PDISEA);
        	F_CLR(crt_ptr, PSHRNK);
        	F_CLR(crt_ptr, PRFLCT);
        	F_CLR(crt_ptr, PSTRNG);
        	F_CLR(crt_ptr, PBERSK);
        	F_CLR(crt_ptr, PEVEYE);
        	F_CLR(crt_ptr, PCHARM);
        	F_CLR(crt_ptr, PFROZE);
        	F_CLR(crt_ptr, PRFIRE);
        	F_CLR(crt_ptr, PRCOLD);
        	F_CLR(crt_ptr, PRACID);

        	if(crt_ptr->ready[WIELD-1] &&
           		!F_ISSET(crt_ptr->ready[WIELD - 1], OCURSE)) {
					wielding = 1;
            		add_obj_rom(crt_ptr->ready[WIELD - 1], crt_ptr->parent_rom);
            		temp_perm(crt_ptr->ready[WIELD - 1]);
            		crt_ptr->ready[WIELD - 1] = 0;
        	}

        	for(i = 0; i < MAXWEAR; i++) {
        	    if(crt_ptr->ready[i] && !F_ISSET(crt_ptr->ready[i], OCURSE)) {
                	if(att_ptr->type == PLAYER)
                    	add_obj_rom(crt_ptr->ready[i], crt_ptr->parent_rom);
                else
                    add_obj_crt(crt_ptr->ready[i], crt_ptr);
                crt_ptr->ready[i] = 0;
            	}
        	}

			if(!F_ISSET(rom_ptr, RJAILR)) {
        		compute_ac(crt_ptr);
        		compute_thaco(crt_ptr);
        		if(crt_ptr == att_ptr)
            		broadcast("### Sadly, %s died and is now in Limbo.", 
						crt_ptr->name);
        		else {
            		broadcast("### The bells toll slowly and sadly "
								"throughout the Land of Mordor,"); 
            		broadcast(" for %s was killed by %1m.", crt_ptr->name, 
						att_ptr);
				}

        		del_ply_rom(crt_ptr, rom_ptr);
        		load_rom(50, &rom_ptr);
        		add_ply_rom(crt_ptr, rom_ptr);
        		savegame(crt_ptr, 0);
				if(wielding)
        		print(crt_ptr->fd, "### Your weapon was dropped at the "
									"place of your demise...\n");
    		}
			else {
        		compute_ac(crt_ptr);
        		compute_thaco(crt_ptr);
				ANSI(crt_ptr->fd, YELLOW);
        		print(crt_ptr->fd, "### However, not even death will allow "
									"you to escape this prison.  So sad...\n");
				ANSI(crt_ptr->fd, WHITE);
            	broadcast("### %s has died an ignoble death in prison, "
							"where %s remains even now.",
            		crt_ptr->name, F_ISSET(crt_ptr, PMALES) ? "he":"she");
        		savegame(crt_ptr, 0);
    		}
    }
        
}

/*************************************************************************/
/*            				 temp_perm			 		                 */
/*************************************************************************/
void temp_perm(obj_ptr)
object  *obj_ptr;

{
    otag    *op;

    F_SET(obj_ptr, OPERM2);
    F_SET(obj_ptr, OTEMPP);
    op = obj_ptr->first_obj;
    while(op) {
        temp_perm(op->obj);
        op = op->next_tag;
    }
}

/*************************************************************************/
/*            				 die_perm_crt		 		                 */
/*************************************************************************/
/* This function is called whenever a permanent monster is killed.  The  */
/* room the monster was in has its permanent monster list checked to see */
/* if the monster was loaded from that room.  If it was, then the last-  */
/* time field for that permanent monster is updated.           		 	 */

void die_perm_crt(crt_ptr)
creature    *crt_ptr;

{
    creature    *temp_crt;
    room        *rom_ptr;
    long        t;
    int     	i;

    t = time(0);

    rom_ptr = crt_ptr->parent_rom;
    
    for(i = 0; i < 10; i++) {
        if(!rom_ptr->perm_mon[i].misc) continue;
        if(rom_ptr->perm_mon[i].ltime + rom_ptr->perm_mon[i].interval > t) 
			continue;
        if(load_crt(rom_ptr->perm_mon[i].misc, &temp_crt) < 0)
            continue;
        if(!strcmp(temp_crt->name, crt_ptr->name)) {
            rom_ptr->perm_mon[i].ltime = t;
            free_crt(temp_crt);
            break;
        }
        free_crt(temp_crt);
    }

	if(F_ISSET(crt_ptr, MDEATH)) {
	  	int     fd, n;
    	char    tmp[2048], file[80], name[80];

		strcpy(name, crt_ptr->name);
        for(i = 0; name[i]; i++)
            if(name[i] == ' ') 
				name[i] = '_';

    	sprintf(file,"%s/ddesc/%s_%d", OBJPATH, name, crt_ptr->level);
    	fd = open(file, O_RDONLY | O_BINARY, 0);
    	if(fd) {
		    n = read(fd, tmp, 2048);
    		tmp[n] = 0;
   			broadcast_rom(-1, crt_ptr->rom_num,"\n%s", tmp);
		}
    	close(fd);
	}
}

/*************************************************************************/
/*             				 check_for_flee  		 	                 */
/*************************************************************************/
/* This function will determine if a monster is about to flee to another */
/* room.  Only fleeing monsters will flee, and then only after they have */
/* less than 10% of their hit points.  Plus, they can only flee into     */
/* rooms that have already been loaded into memory.         		 	 */

void check_for_flee(crt_ptr)
creature    *crt_ptr;

{
    room    *rom_ptr;
    xtag    *xp;

    if(!F_ISSET(crt_ptr, MFLEER) || F_ISSET(crt_ptr, MDMFOL))
        return;

    if(crt_ptr->hpcur > crt_ptr->hpmax/5)
        return;

	if(crt_ptr->hpcur > 0) {     
    	rom_ptr = crt_ptr->parent_rom;

    	xp = rom_ptr->first_ext;
    	while(xp) {
        	if(!F_ISSET(xp->ext, XSECRT) && !F_ISSET(xp->ext, XCLOSD) &&
           		is_rom_loaded(xp->ext->room) && mrand(1, 100) < 75) {
            		broadcast_rom(-1, rom_ptr->rom_num, 
            			"%M flees to the %s.", crt_ptr, xp->ext->name);
            		die_perm_crt(crt_ptr);
            		del_crt_rom(crt_ptr, rom_ptr);
            		load_rom(xp->ext->room, &rom_ptr);
            		add_crt_rom(crt_ptr, rom_ptr, 1);
            	return;
        	}
        	xp = xp->next_tag;
    	}
	}
	else return;
}

/*************************************************************************/
/*     			        		 consider 				                 */
/*************************************************************************/
/* This function allows the player pointed to by the first argument to   */
/* consider how difficult it would be to kill the creature in the        */
/* second parameter.                              			 			 */

void consider(ply_ptr, crt_ptr)
creature    *ply_ptr;
creature    *crt_ptr;

{
    char    he[5], him[5];
    int 	fd, diff;

    fd = ply_ptr->fd;
    diff = ply_ptr->level - crt_ptr->level;
    diff = MAX(-4, diff);
    diff = MIN(4, diff);

   	sprintf(he, "%s", F_ISSET(crt_ptr, MMALES) ? "He":"She");
   	sprintf(him, "%s", F_ISSET(crt_ptr, MMALES) ? "him":"her");
	if(F_ISSET(crt_ptr, MNOGEN)) {
		sprintf(he, "It");
		sprintf(him, "it");
	}


    if(F_ISSET(crt_ptr, MUNKIL)) 
       print(fd, "%s cannot be harmed.\n", he);

    if(!F_ISSET(crt_ptr, MUNKIL)) {
		switch(diff) {
			case 0:
	   	         print(fd, "%s is a perfect match for you!\n", he);
	   	         break;

			case 1:
	   	         print(fd, "%s is not quite as good as you.\n", he);
	   	         break;

			case -1:
	   	         print(fd, "%s is a little better than you.\n", he);
	   	         break;

			case 2:
	   	         print(fd, "%s shouldn't be too tough to kill.\n", he);
	   	         break;

			case -2:
	   	         print(fd, "%s might be tough to kill.\n", he);
   		         break;

			case 3:
	   	         print(fd, "%s should be easy to kill.\n", he);
	   	         break;

			case -3:
	  	         print(fd, "%s should be really hard to kill.\n", he);
	   	         break;

			case 4:
	   	         print(fd, "You could kill %s with a needle.\n", him);
	   	         break;

			case -4:
	   	         print(fd, "%s could kill you with a needle.\n", he);
	   	         break;
		}
    }
}

/*************************************************************************/
/*                      is_charm_crt                                     */
/*************************************************************************/
/* This function returns true if the name passed in the first parameter  */
/* is in the charm list of the creature pointed to by the second.        */
        
int is_charm_crt(charmed, crt_ptr)
char        *charmed;
creature    *crt_ptr;

{
    ctag    *cp;  
    int     fd;
            
        fd = crt_ptr->fd;
        if(crt_ptr->type == MONSTER)
		return(0);

        cp = Ply[fd].extr->first_charm;
           
    while(cp) {
        if(!strcmp(cp->crt->name, charmed))
            return(1);
        cp = cp->next_tag;
    }

    return(0);
}                   

/************************************************************************/
/*  		            	add_charm_crt                               */
/************************************************************************/
/*      This function adds the creature pointed to by the first         */
/*  parameter to the charm list of the creature pointed to by the       */
/*  second parameter.                                                   */

            
int add_charm_crt(crt_ptr, ply_ptr)
creature    *crt_ptr;
creature    *ply_ptr;

{           
    ctag    *cp;
    int     fd, n;
            
    fd = ply_ptr->fd; 
     
    n = is_charm_crt(crt_ptr->name, ply_ptr);
	if(n)
		return(0);

	if(crt_ptr && crt_ptr->class < DM) {
		cp = 0;
		cp = (ctag *)malloc(sizeof(ctag));
		if(!cp)
		merror("add_charm_crt", FATAL);
		cp->crt = crt_ptr;
		cp->next_tag = Ply[fd].extr->first_charm;
		Ply[fd].extr->first_charm = cp;
	}       

	return(0);
}

/************************************************************************/
/*           		   		del_charm_crt                               */
/************************************************************************/
/*      This function deletes the creature pointed to by the first      */
/*  parameter from the charm list of the creature pointed to by the     */
/*  second parameter.                                                   */

int del_charm_crt(crt_ptr, ply_ptr)
creature    *crt_ptr;
creature    *ply_ptr;

{
    ctag    *cp, *prev;
    int     fd, n;
                
	fd = ply_ptr->fd;

	n = is_charm_crt(crt_ptr->name, ply_ptr);
	if(!n)   
		return(0);

	cp = Ply[fd].extr->first_charm;

	if(cp->crt == crt_ptr) {
		Ply[fd].extr->first_charm = cp->next_tag;
		free(cp);
		return(1);
	}
	else while(cp) {
		if(cp->crt == crt_ptr) {
			prev->next_tag = cp->next_tag;
			free(cp);
			return(1);
		}
		prev = cp;
		cp = cp->next_tag;
	}       
	return(0);
}


/**********************************************************************/
/*							attack_mon							      */
/**********************************************************************/
/*  This function does the attacking when it is two monsters 	      */
/*  attacking each other.					      					  */

int attack_mon(att_ptr, atd_ptr)
creature	*att_ptr;
creature	*atd_ptr;

{
int			n, t, m;	
	

	if(att_ptr->type != MONSTER || atd_ptr->type != MONSTER)
		return(0);
	
	t = time(0);
    	if(t < LT(att_ptr, LT_ATTCK))
			return(0);

    if(F_ISSET(att_ptr, MHIDDN)) {
		F_CLR(att_ptr, MHIDDN);
        broadcast_rom(-1, att_ptr->rom_num, "%M comes from hiding.", att_ptr);
    }

    if(F_ISSET(att_ptr, MINVIS)) {
       	F_CLR(att_ptr, MINVIS);
        broadcast_rom(-1, att_ptr->rom_num, "%M fades into view.", att_ptr);
    }

    att_ptr->lasttime[LT_ATTCK].ltime = t;
	if(F_ISSET(att_ptr, MBLIND)) {
		att_ptr->lasttime[LT_ATTCK].interval = 7;
    }

    if(F_ISSET(atd_ptr, MUNKIL)) {
        return(0);
    }

    if(add_enm_crt(att_ptr->name, atd_ptr) < 0) {
		broadcast_rom(-1, att_ptr->rom_num, "%M attacks %m.",
			att_ptr, atd_ptr);
	}

    n = att_ptr->thaco - atd_ptr->armor/10;
    if(F_ISSET(att_ptr, MFEARS))
		n += 5;

    if(F_ISSET(att_ptr, MBLIND))
		n += 5;

    if(mrand(1, 20) >= n) {
		n = mdice(att_ptr) + bonus[att_ptr->strength]; 
		n = MAX(1, n);
        if(mrand(1, 100) <= 5) {
            broadcast_rom(-1, att_ptr->rom_num, "%M made a critical hit.", 
				att_ptr);
            n *= mrand(3, 6);
        }
        else if(mrand(1, 100) <= 5) {
            broadcast_rom(-1, att_ptr->rom_num, 
                "%M fumbled %s weapon.", att_ptr,
                F_ISSET(att_ptr, MMALES) ? "his":"her");
        }
        m = MIN(atd_ptr->hpcur, n);
        atd_ptr->hpcur -= n;
        if(atd_ptr->hpcur < 1) {
            broadcast_rom(-1, att_ptr->rom_num, "%M killed %m.", 
				att_ptr, atd_ptr);
            die(atd_ptr, att_ptr);
            return(1);
        }
        else
            check_for_flee(atd_ptr);
    }

    return(0);
}

/************************************************************************/
/*							mobile_crt									*/
/************************************************************************/
/*	This function provides for self-moving monsters.  					*/
/*  If the MMOBIL flag is set the creature will move around.			*/
/*  NOTE:  The creature will only go to a loaded room.					*/

int mobile_crt(crt_ptr)
creature	*crt_ptr;

{
int		i, n = 0, d = 0, choice = 0;
room	*rom_ptr, *new_rom;
xtag	*xp, *xptemp;
char 	found = 0;

extern char		ext[5][8];

	if(crt_ptr->type != MONSTER || !F_ISSET(crt_ptr, MMOBIL) || 
		F_ISSET(crt_ptr, MPERMT) || F_ISSET(crt_ptr, MPGUAR))
			return(0);

	if(crt_ptr->first_enm)
		return(0);

	rom_ptr = crt_ptr->parent_rom;
	xptemp = rom_ptr->first_ext;
	while(xptemp) { /* Count up exits */
		if(!F_ISSET(xptemp->ext, XSECRT) && !F_ISSET(xptemp->ext, XNOSEE) && 
			!F_ISSET(xptemp->ext, XLOCKD) && !F_ISSET(xptemp->ext, XPGUAR) && 
				is_rom_loaded(xptemp->ext->room)) 
					d += 1;
		xptemp=xptemp->next_tag;
	}

	if(!d)
		return(0);

	choice = mrand(1, d);
	xp = rom_ptr->first_ext;
   	while(xp) {  
       	if(!F_ISSET(xp->ext, XSECRT) && !F_ISSET(xp->ext, XNOSEE) && 
			!F_ISSET(xp->ext, XLOCKD) && !F_ISSET(xp->ext, XPGUAR) && 
				is_rom_loaded(xp->ext->room)) {
       				choice -= 1;
    				if(!choice) {
						if(F_ISSET(xp->ext, XCLOSD) && 
							!F_ISSET(xp->ext, XLOCKD)) {
								broadcast_rom(-1, rom_ptr->rom_num, 
									"%M just opened the %s.", crt_ptr, 
										xp->ext->name);
								F_CLR(xp->ext, XCLOSD);
						}
						for(i = 0, found = 0; i < 5 && found == 0; i++) {
							if(strcmp(ext[i], xp->ext->name) == 0)
								found = 1;  
						}
						if(found) 
   							broadcast_rom(-1, rom_ptr->rom_num,
								"%M just went %s.", crt_ptr, xp->ext->name); 
						else
   							broadcast_rom(-1, rom_ptr->rom_num,
								"%M just went to the %s.", crt_ptr, 
									xp->ext->name);
						found = 0;
       					del_crt_rom(crt_ptr, rom_ptr);
    					load_rom(xp->ext->room, &new_rom); 
       					add_crt_rom(crt_ptr, new_rom , 1);
    					n = 1;
    					crt_ptr->lasttime[LT_MWAND].ltime=time(0);
    					crt_ptr->lasttime[LT_MWAND].interval=mrand(5,60);
    					break;
    				}
        }
        xp = xp->next_tag;
    }
	if(mrand(1, 100) > 80)
		F_CLR(crt_ptr, MMOBIL);

 	return(n);
}
/*************************************************************************/
/*							find creature number						 */	
/*************************************************************************/
/*       This function finds the creature number of the given creature   */	
/*       from the database												 */
        
int find_crt_num(crt_ptr) 
creature  *crt_ptr;

{
	int i;
	creature  *crt_src;

	for (i = 0; i < CMAX; i++) {
		if(load_crt_from_file(i, &crt_src) < 0)
			continue;
		if(!strcmp(crt_ptr->name, crt_src->name) && 
			crt_ptr->experience == crt_src->experience) {
				free(crt_src);
				break;
		}
		free(crt_src); 
	}

	if(i < CMAX)
		return(i);
	else
		return(0);
}
