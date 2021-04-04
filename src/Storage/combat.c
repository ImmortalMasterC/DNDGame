/*
* COMBAT.C
*
*  These routines are called during all combats
*/ 

#include "mstruct.h"
#include "mextern.h"
#include <string.h>


int update_combat(crt_ptr)
creature *crt_ptr;

{
	creature *att_ptr;
	room     *rom_ptr;
	etag     *ep;
	ctag     *cp;
	char     *enemy;
	int      fd, damage = 0, rtn = 0, item_num = 0, p = 0, t = 0;
	int	 diff = 0, parry = 0, fear_factor, blnd = 0, rndm = 0;   

	rom_ptr = crt_ptr->parent_rom;
	ep = crt_ptr->first_enm;
	while(1) {
		enemy = ep->enemy;
		if(!enemy)
			ep = ep->next_tag;
		if(!ep)
			return (0);
		if(enemy)
			break;
	}
    
	att_ptr = find_crt(crt_ptr, rom_ptr->first_ply, enemy, 1);

	if(!att_ptr)
		att_ptr = find_crt(crt_ptr, rom_ptr->first_mon, enemy, 1);

	if(!att_ptr) {
		if(!find_who(enemy))
			del_enm_crt(enemy, crt_ptr);
		else
			end_enm_crt(enemy, crt_ptr);
		return (0);
	}
   
	if(is_charm_crt(crt_ptr->name, att_ptr) && F_ISSET(crt_ptr, MCHARM))
		p = 1;
	crt_ptr->NUMHITS++;
	damage = 20;
	if(F_ISSET(crt_ptr, MMAGIO))
		damage = crt_ptr->proficiency[0];
	         
	if(F_ISSET(crt_ptr, MMAGIC) && (mrand(1, 100) <= damage) && !p) {
		rtn = crt_spell(crt_ptr, att_ptr);
		if(rtn == 2) 
			return (1); 
		else if(rtn == 1)
			damage = 21;
		else 
			damage = crt_ptr->thaco - att_ptr->armor/10;
	}
	else {
		damage = crt_ptr->thaco - att_ptr->armor/10;
		damage = MAX(damage, 1);
	}
   
	diff = att_ptr->armor/10 - crt_ptr->thaco;
	rndm = mrand(1, 100);
	if(diff <= 0 && (rndm < 20) && !p) {
		ANSI(att_ptr->fd, CYAN);
		print(att_ptr->fd, "%M missed you.\n", crt_ptr);
		ANSI(att_ptr->fd, BOLD);
		ANSI(att_ptr->fd, WHITE);
		return (1);
	}
	else if(diff < -3 && (rndm < 25) && !p) {
		ANSI(att_ptr->fd, CYAN);
		print(att_ptr->fd, "%M missed you.\n", crt_ptr);
		ANSI(att_ptr->fd, BOLD);
		ANSI(att_ptr->fd, WHITE);
		return (1);
	}
	else if(diff < -6 && (rndm < 30) && !p) {
		ANSI(att_ptr->fd, CYAN);
		print(att_ptr->fd, "%M missed you.\n", crt_ptr);
		ANSI(att_ptr->fd, BOLD);
		ANSI(att_ptr->fd, WHITE);
		return (1);
	}
	else if(diff < -9 && (rndm < 35) && !p) {
		ANSI(att_ptr->fd, CYAN);
		print(att_ptr->fd, "%M missed you.\n", crt_ptr);
		ANSI(att_ptr->fd, BOLD);
		ANSI(att_ptr->fd, WHITE);
		return (1);
	}
	else if(diff < -15 && (rndm < 40) && !p) {
		ANSI(att_ptr->fd, CYAN);
		print(att_ptr->fd, "%M missed you.\n", crt_ptr);
		ANSI(att_ptr->fd, BOLD);
		ANSI(att_ptr->fd, WHITE);
		return (1);
	}

	if(mrand(1, 20) >= damage && !p) {
		fd = att_ptr->fd;
		if(F_ISSET(crt_ptr, MBRETH) && mrand(1, 100) < 20) {
			if(F_ISSET(crt_ptr, MBRWP1) && !F_ISSET(crt_ptr, MBRWP2))
				damage = bw_spit_acid(crt_ptr, att_ptr);
			else if(F_ISSET(crt_ptr, MBRWP1) && F_ISSET(crt_ptr, MBRWP2))
				damage = bw_poison(crt_ptr, att_ptr);
			else if(!F_ISSET(crt_ptr, MBRWP1) && F_ISSET(crt_ptr, MBRWP2))
				damage = bw_cone_frost(crt_ptr, att_ptr);
			else
				damage = bw_cone_fire(crt_ptr, att_ptr);
		}
		else if(F_ISSET(crt_ptr, MENEDR) && mrand(1, 100) < 25) 
			damage = power_energy_drain(crt_ptr, att_ptr);
		else
			damage = mdice(crt_ptr);		     

		if((F_ISSET(crt_ptr, MEVEYE) && crt_ptr->alignment > 0) || 
			F_ISSET(crt_ptr, MBLIND) || 
			F_ISSET(crt_ptr, MFEARS)) {
				if(blnd == 0)
				damage = mdice(crt_ptr) * 2/3;		     
				blnd = 1;
		}
    
		if(F_ISSET(crt_ptr, MEVEYE) && crt_ptr->alignment > 0) {		
			crt_ptr->thaco += 2;
			if(crt_ptr->thaco > 20);
				crt_ptr->thaco = 20;
		}

		if(F_ISSET(crt_ptr, MBLIND) && blnd == 0) {		
			crt_ptr->thaco += 2;
			if(crt_ptr->thaco > 20);
				crt_ptr->thaco = 20;
		}

		if(F_ISSET(crt_ptr, MFEARS)) {		
			crt_ptr->thaco += 2;
			if(crt_ptr->thaco > 20);
				crt_ptr->thaco = 20;
		}

		if(F_ISSET(crt_ptr, MFROZE)) {		
			crt_ptr->thaco += 5;
			if(crt_ptr->thaco > 20);
				crt_ptr->thaco = 20;
		}

		if(att_ptr->ready[HELD - 1] && (mrand(1, 100) < 20) &&
			(att_ptr->ready[HELD - 1]->type == 5)) {
				parry = 1;
				att_ptr->ready[HELD - 1]->shotscur -= 1;
				damage -= att_ptr->ready[HELD - 1]->armor * 5;
				if(damage < 0)
					damage = 0;
        		if(att_ptr->ready[HELD - 1]->shotscur <= 3 &&
        			(att_ptr->ready[HELD - 1]->shotscur > 0)) {
						ANSI(fd, YELLOW);
        		    	print(att_ptr->fd, "Your %s is almost broken.\n", 
							att_ptr->ready[HELD - 1]->name);
						ANSI(fd, BOLD);
						ANSI(fd, WHITE);
				}	
        		else if(att_ptr->ready[HELD - 1]->shotscur < 1) {
					parry = 0;
					ANSI(fd, YELLOW);
        		    print(att_ptr->fd, "Your %s is broken.\n", 
						att_ptr->ready[HELD - 1]->name);
					ANSI(fd, BOLD);
					ANSI(fd, WHITE);
            		broadcast_rom(fd, att_ptr->rom_num, "%M broke %s %s.", 
						att_ptr, F_ISSET(att_ptr, PMALES) ? "his":"her",
            	    		att_ptr->ready[HELD - 1]->name);
            		add_obj_crt(att_ptr->ready[HELD - 1], att_ptr);
            		att_ptr->ready[HELD - 1] = 0;
        		}
		}
			
		if(F_ISSET(att_ptr, PSTRNG) || (F_ISSET(crt_ptr, MEVEYE) &&
			crt_ptr->alignment > 0)) {
        		damage = damage/2;
		}

		if(F_ISSET(crt_ptr, MFROZE) || F_ISSET(crt_ptr, MHEXED)) {
        	damage = (3 * damage/4);
		}

		if(F_ISSET(att_ptr, PCLOAK)) {
        	damage = damage/3;
		}

		att_ptr->hpcur -= damage;
		if(att_ptr->hpcur < 1) {            /*  Monster kills player  */
			print(fd, "\n\n\n\n\n");
			ANSI(fd, REDBG);
			ANSI(fd, YELLOW);
			print(fd, 
				"%M killed you!  The Land of Mordor mourns your demise.", 
					crt_ptr);
			ANSI(fd, NORMAL);
			ANSI(fd, BLUEBG);
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			print(fd, "\n\n");
			BEEP(fd);
			BEEP(fd);
			die(att_ptr, crt_ptr);
			return (1);
		}

		if(damage) {
			if(F_ISSET(crt_ptr, MHIDDN)) {
				F_CLR(crt_ptr, MHIDDN);
				print(fd, "%M jumps from hiding.\n", crt_ptr);
				broadcast_rom(fd, att_ptr->rom_num, 
					"%M jumps from a hiding place.\n", crt_ptr);
			}
			if(F_ISSET(crt_ptr, MEVEYE) && mrand(1, 100) > 70) {
				F_CLR(crt_ptr, MEVEYE);
				print(fd, "%M staggers and lightly hits you for %d damage.\n", 
					crt_ptr, damage);
			}
			if(F_ISSET(att_ptr, PCLOAK)) {
				print(fd, "%M sees your shadow and grazes you for %d damage.\n",
					crt_ptr, damage);
			}
			else {
        		ANSI(fd, RED); 
				print(fd, "%M hit you for %d damage.\n", crt_ptr, damage);
        		ANSI(fd, BOLD); 
        		ANSI(fd, WHITE); 
				broadcast_rom(fd, att_ptr->rom_num, "%M hit %m.", 
					crt_ptr, att_ptr);
				if(att_ptr->hpcur <= att_ptr->hpmax/3)
					broadcast_rom(fd, att_ptr->rom_num, "%M hurts %m!", 
						crt_ptr, att_ptr);
				if(F_ISSET(att_ptr, PSTRNG)) {
					print(fd, "Your super strength reduced %M's damage.\n",
						crt_ptr);
					if(mrand(1, 100) < 20) {
						F_CLR(att_ptr, PSTRNG);
						att_ptr->strength -=5;
						print(fd, "Your super strength fades.\n");
					}
				}
			}
		}
		else { 
			if(parry) {
        		ANSI(fd, YELLOW); 
				print(fd, "You parry the blow with your %s.\n",
					att_ptr->ready[HELD - 1]);
			}
        	ANSI(fd, BOLD); 
        	ANSI(fd, WHITE); 
			print(fd, "%M brushed against you for no damage.\n", crt_ptr);
		}

/*------------------------------------------------------------------------*/
/* Output only when monster v. monster 					  */


		if(att_ptr->type == MONSTER && crt_ptr->type == MONSTER) { 
			F_CLR(att_ptr, MHIDDN);
			F_CLR(crt_ptr, MHIDDN);
			broadcast_rom2(fd, crt_ptr->fd, att_ptr->rom_num, "%M hits %m.", 
				crt_ptr, att_ptr);
			add_enm_crt(crt_ptr->name, att_ptr);
		}
/*------------------------------------------------------------------------*/

		add_enm_dmg(att_ptr->name, crt_ptr, damage);

		if(F_ISSET(crt_ptr, MFEARD) && mrand(1, 100) <= 20) {
			if((att_ptr->class != PALADIN) && att_ptr->class < CARETAKER) {
				ANSI(fd, YELLOW);
				print(fd, "%M makes you tremble with fear.\n", crt_ptr);
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
				F_SET(att_ptr, PFEARS);
			}
			if(att_ptr->class >= CARETAKER) {
				ANSI(fd, YELLOW);
				print(fd, "%M makes a face at you.\n", 
					crt_ptr);
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
			}
			else { 
				if(mrand(1, 100) <= 90) {
					if(!F_ISSET(att_ptr, PFEARS) && 
						(att_ptr->class == PALADIN ||
						att_ptr->class == CLERIC)) {
							ANSI(fd, GREEN);
							if(!F_ISSET(crt_ptr, MNOGEN)) {
	 	   						print(fd, "%M tried to make you fear %s,\n", 
									crt_ptr, F_ISSET(crt_ptr, MMALES) ? 
										"him":"her");
							}
							else {
	 	   						print(fd, "%M tried to make you fear it.\n", 
									crt_ptr); 
							}
	 	   					print(fd, "but your faith relieves your fear.\n");
		   					ANSI(fd, BOLD);
		   					ANSI(fd, WHITE); 
					}
				}
				else {
					ANSI(fd, YELLOW);
					print(fd, "%M makes you tremble with fear.\n", crt_ptr);
					ANSI(fd, BOLD);
					ANSI(fd, WHITE);
					F_SET(att_ptr, PFEARS);
				}
			}
		}
	
		if(F_ISSET(crt_ptr, MPOISS) && mrand(1, 100) <= 15) {
			if(att_ptr->class != DRUID && att_ptr->class < CARETAKER) {
				F_SET(att_ptr, PPOISN);
				ANSI(fd, RED);
				print(fd, "%M poisoned you.\n", crt_ptr);
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
			}
			else { 
				ANSI(fd, GREEN);
	 	   		print(fd, "%M tried to poison you,\n", crt_ptr);
		   		print(fd, "but it turns to energy in your body!\n");
		   		ANSI(fd, BOLD);
		   		ANSI(fd, WHITE); 
    			att_ptr->hpcur = 1 + att_ptr->hpcur + 
					(att_ptr->hpmax - att_ptr->hpcur)/3;
			}
    	} 
	
		if(F_ISSET(crt_ptr, MDISEA) && mrand(1, 100) <= 15) {
			if(att_ptr->class != DRUID && att_ptr->class < CARETAKER) {
		  		ANSI(fd, RED);
      			print(fd, "%M infects you.\n", crt_ptr);
      	    	F_SET(att_ptr, PDISEA);
		  		ANSI(fd, BOLD);
		  		ANSI(fd, WHITE);
			}
	  		else { 
	    		ANSI(fd, GREEN);
      			print(fd, "%M tries to infect you,\n", crt_ptr);
	    		print(fd, "but your body is revitalized instead!\n");
	    		ANSI(fd, BOLD);
	    		ANSI(fd, WHITE);
    			att_ptr->hpcur = 1 + att_ptr->hpcur + 
					(att_ptr->hpmax - att_ptr->hpcur)/3;
			} 
        }  
			
		if(F_ISSET(crt_ptr, MBLNDR) && mrand(1, 100) <= 15) {
		  	if(att_ptr->class != DRUID && att_ptr->class < CARETAKER) {
				ANSI(fd, RED);
    	    	print(fd, "%M  blinds your eyes.\n", crt_ptr);
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
    	    	F_SET(att_ptr, PBLIND);
			}
		  	else { 
 		  		print(fd, 
					"%M tried to blind you, but the spell is deflected!\n", 
						crt_ptr);
	  			if(mrand(1, 100) <= (10 + (2 * att_ptr->level))) {
					F_SET(crt_ptr, MBLIND);
					ANSI(fd, YELLOW);
					print(fd, "The deflected spell blinded %M.\n", crt_ptr);
					ANSI(fd, BOLD);
					ANSI(fd, WHITE);
					broadcast_rom(fd, att_ptr->rom_num,
						"%M is blinded by a deflected spell cast at %m.",
							crt_ptr, att_ptr);
					if(blnd == 0) {
						crt_ptr->thaco += 2;
						if(crt_ptr->thaco > 20)
							crt_ptr->thaco = 20;
					}
				}
     		}
		} 
 
       	if(F_ISSET(crt_ptr, MDISIT) && mrand(1, 100) <= 4) {
			if(!F_ISSET(att_ptr, PRACID)) {
				dissolve_item(att_ptr, crt_ptr);
       			item_num = choose_item(att_ptr);
       			if(item_num) {
					if(--att_ptr->ready[item_num - 1]->shotscur < 1) {
						print(fd,"Your %s dissolved into a slimy mess.\n",
							att_ptr->ready[item_num - 1]->name);
						broadcast_rom(fd, att_ptr->rom_num,
							"%M's %s dissolved into a slimy mess.",
								att_ptr, att_ptr->ready[item_num - 1]->name);
						add_obj_crt(att_ptr->ready[item_num - 1], att_ptr);
						att_ptr->ready[item_num - 1] = 0;
						compute_ac(att_ptr);
						compute_agility(att_ptr);
					}
				}	
			}	
			else {
				item_num = choose_item(att_ptr);
				if(item_num) {
                        print(fd,"Your resist-acid spell prevented your %s from dissolving!\n",
                        	att_ptr->ready[item_num - 1]->name);
                        broadcast_rom(fd, att_ptr->rom_num,
                            "%M's resist-acid spell puts sparks in the air!", att_ptr);
				}
			}
		}
       
       	if(att_ptr->type == PLAYER && F_ISSET(att_ptr, PAUTOA) && !p) {      
       	     rtn = attack_crt(att_ptr, crt_ptr);
       	     if(rtn) 
				return (1);
		}
		else {      
			if(LT(att_ptr, LT_ATTCK) < t) {				   
				rtn = attack_crt(att_ptr, crt_ptr);
				att_ptr->lasttime[LT_ATTCK].ltime = t;
				if(rtn) 
					return (1);
			}
		}

       	if(att_ptr->hpcur > 1 && att_ptr->hpcur < att_ptr->hpmax/5) {
       		ANSI(fd, YELLOW);
       		print(fd, "%M has slashed you to a bloody mess!\n", crt_ptr);
       		ANSI(fd, BOLD);
       		ANSI(fd, WHITE);
       		broadcast_rom(fd, att_ptr->rom_num,
			"\n%M is covered with blood!", att_ptr);
       	}
       	        
		if(F_ISSET(att_ptr, PFEARS)) {
			fear_factor = (40 + (att_ptr->hpcur/att_ptr->hpmax) * 40 +
				bonus[att_ptr->constitution] + 
				((att_ptr->class == PALADIN) ? 10 : 0));
			if(fear_factor < mrand(1, 100)) {
				flee(att_ptr);
				return (1);
			}
		}
		else if(F_ISSET(att_ptr, PWIMPY)) {
			if(att_ptr->hpcur <= att_ptr->WIMPYVALUE) {
				flee(att_ptr);
				return (1);
			}
		}

/*------------------------------------------------------------------------*/
/* Output only when monster vs. monster */

		if(att_ptr->type == MONSTER && crt_ptr->type == MONSTER) { 
			broadcast_rom2(att_ptr->fd, crt_ptr->fd, 
				att_ptr->rom_num, "%M misses %m.", crt_ptr, att_ptr);
				add_enm_crt(crt_ptr->name, att_ptr);
			}
/*------------------------------------------------------------------------*/

		ANSI(att_ptr->fd, WHITE);
		if(att_ptr->type == PLAYER && F_ISSET(att_ptr, PAUTOA)) {      
			rtn = attack_crt(att_ptr, crt_ptr);
			if(rtn) 
			return (1);
		}
		else {      
			if(LT(att_ptr, LT_ATTCK) < t) {			
			rtn = attack_crt(att_ptr, crt_ptr);
			att_ptr->lasttime[LT_ATTCK].ltime = t;
			if(rtn) 
				return (1);
			}
		}
	}

	return (0);
}

/***************************************************************************/
/*  			breath weapons 					   */	
/***************************************************************************/
/*  			acid breath					   */	
/***************************************************************************/

int bw_spit_acid(crt_ptr, vic_ptr)
creature *crt_ptr;
creature *vic_ptr;

{
	int 	n, fd;

	fd = vic_ptr->fd;
	print(fd, "%M spits acid on you!\n", crt_ptr);
	broadcast_rom(fd, vic_ptr->rom_num, "%M spits acid on %m!", 
		crt_ptr, vic_ptr);

	if(F_ISSET(vic_ptr, PRACID)) {
        ANSI(fd, GREEN);
        print(fd, "Your resist-acid spell kept you from harm!\n");
        ANSI(fd, BOLD);
        ANSI(fd, WHITE);
        n = 0;
    }
	else 
		n = dice(crt_ptr->level, 3, 0);
	return (n);
}

/***************************************************************************/
/*  			poison gas breath				   */	
/***************************************************************************/

int bw_poison(crt_ptr, vic_ptr)
creature *crt_ptr;
creature *vic_ptr;

{
	int 	n, fd;

	fd = vic_ptr->fd;
	print(fd, "%M breathes poisonous gas on you!\n", crt_ptr);
	broadcast_rom(fd, vic_ptr->rom_num,
       "%M breathes poisonous gas on %m!", crt_ptr, vic_ptr);

	n = dice(crt_ptr->level, 2, 1);

	if(vic_ptr->class != DRUID && vic_ptr->class < CARETAKER) {
	  	ANSI(fd, RED);
   		print(fd, "%M poisoned you.\n", crt_ptr);
	  	ANSI(fd, BOLD);
	  	ANSI(fd, WHITE);
   		F_SET(vic_ptr, PPOISN);
		broadcast_rom(fd, vic_ptr->rom_num,
       		"%M was poisoned by %m!\n", vic_ptr, crt_ptr);
	}
	else {
		broadcast_rom(fd, vic_ptr->rom_num,
       		"%M laughs and takes a deep breath as %m tries to breath poison gas on %s!\n", 
				vic_ptr, crt_ptr, F_ISSET(vic_ptr, PMALES) ? "him":"her");
	   	ANSI(fd, GREEN);
 	   	print(fd, 
			"You inhale deeply as the poison turns to energy in your lungs!\n");
	   	ANSI(fd, BOLD);
	   	ANSI(fd, WHITE); 
    	vic_ptr->hpcur = 1 + vic_ptr->hpcur + 
			(vic_ptr->hpmax - vic_ptr->hpcur)/3;
	}

	return (n);
}

/***************************************************************************/
/*  			frost breath 					   */	
/***************************************************************************/

int bw_cone_frost(crt_ptr, vic_ptr)
creature *crt_ptr;
creature *vic_ptr;

{
	int 	n, fd;

	fd = vic_ptr->fd;

	print(fd, "%M breathes frost on you!\n", crt_ptr);
	broadcast_rom(fd, vic_ptr->rom_num,"%M breathes frost on %m!",
		crt_ptr, vic_ptr);

    if(mrand(1, 100) < 10) {
		if(!F_ISSET(vic_ptr, PRCOLD)) {
			F_SET(vic_ptr, PFROZE);
			ANSI(fd, YELLOW);
			print(fd, 
				"You have been frozen where you stand, and cannot move.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
		}	
		else {
			ANSI(fd, GREEN);
			F_CLR(vic_ptr, PRCOLD);
			print(fd, 
				"Your resist-cold spell protects you from a freezing blast!\n");
			ANSI(fd, YELLOW);
			print(fd, "Unfortunately, the blast dissipated your resist-cold "
						"spell.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
		}	
		return (0);
	}	

	if(F_ISSET(vic_ptr, PRCOLD)) {
		ANSI(fd, GREEN);
		print(fd, "Your resist-cold spell kept you from harm!\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		n = 0;
	}	
	else 
		n = dice(crt_ptr->level, 4, 0);

	return (n);
}

/***************************************************************************/
/*  			fire breath 					   */	
/***************************************************************************/

int bw_cone_fire(crt_ptr, vic_ptr)
creature *crt_ptr;
creature *vic_ptr;

{
	int 	n, fd;

	fd = vic_ptr->fd;
	print(fd, "%M breathes fire on you!\n", crt_ptr);
	broadcast_rom(fd, vic_ptr->rom_num,"%M breathes fire on %m!", 
		crt_ptr, vic_ptr);

	if(F_ISSET(vic_ptr, PFROZE)) {
		F_CLR(vic_ptr, PFROZE);
		print(fd, 
			"%M's fiery breath dissipates the freeze spell holding you.\n",
				crt_ptr);
		n = 1;
		return (n);
	}

	if(F_ISSET(vic_ptr, PRFIRE)) {
		ANSI(fd, GREEN);
		print(fd, "Your resist-fire spell kept you from harm!\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		n = 0;
	}
	else
		n = dice(crt_ptr->level, 4, 0);

	return (n);
}
   
/*******************************************************************************/
/*  			energy drain  		    			       */	
/*	Certain monsters can drain experience away from players during combat. */
/*	The monster must have flag #31 set.		 		       */	
/*******************************************************************************/

int power_energy_drain(crt_ptr, vic_ptr)
creature *crt_ptr;
creature *vic_ptr;

{
	int 	n, fd;

	fd = vic_ptr->fd;
	n = dice(crt_ptr->level, 5, (crt_ptr->level)*3);
	n = MAX(0, MIN(n, vic_ptr->experience));

	print(fd, "%M drains your experience!\n", crt_ptr);
	broadcast_rom2(crt_ptr->fd, vic_ptr->fd, vic_ptr->rom_num,
		"%M drains experience away from %m!", crt_ptr, vic_ptr);

	vic_ptr->experience -= n;
	lower_prof(vic_ptr, n);
	print(fd, "%M drains you for %d experience.\n", crt_ptr, n);

	return (0);
}

/**************************************************************************/
/* 				 attack 		                  */
/**************************************************************************/
/* This function allows the player pointed to by the first parameter 	  */
/* to attack a monster.                    	  		          */

int attack(ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    long        i, t;
    int     	fd;


    fd = ply_ptr->fd;

    t = time(0);
    i = LT(ply_ptr, LT_ATTCK);

    if(t < i) {
        please_wait(fd, i - t);
        return(0);
    }

	if(!strcmp(cmnd->str[0], "stab") && (!ply_ptr->ready[WIELD - 1])) {
		print(fd, "You are not wielding a weapon that can stab.\n");
	    return(0);
    }

	if(!strcmp(cmnd->str[0], "stab") && 
		(ply_ptr->ready[WIELD - 1]->type != SHARP) &&
		ply_ptr->ready[WIELD - 1]->type != THRUST) {
			print(fd, "Stabbing requires a sharp or thrusting weapon.\n");
	    	return(0);
    }

	if(!strcmp(cmnd->str[0], "impale") && (!ply_ptr->ready[WIELD - 1])) {
		print(fd, "You are not wielding a weapon that can impale.\n");
	    return(0);
    }

	if(!strcmp(cmnd->str[0], "impale") &&
        (ply_ptr->ready[WIELD - 1]->type != THRUST)) {
            print(fd, "Only a thrusting weapon can be used to impale.\n");
            return(0);
    }

	if(!strcmp(cmnd->str[0], "smash") && (!ply_ptr->ready[WIELD - 1])) {
		print(fd, "You are not wielding a weapon that can smash.\n");
	    return(0);
    }

	if(!strcmp(cmnd->str[0], "smash") &&
        (ply_ptr->ready[WIELD - 1]->type != BLUNT)) {
            print(fd, "Only a blunt weapon can be used to smash.\n");
            return(0);
    }

	if(!strcmp(cmnd->str[0], "club") && (!ply_ptr->ready[WIELD - 1])) {
		print(fd, "You are not wielding a blunt weapon.\n");
	    return(0);
    }

	if(!strcmp(cmnd->str[0], "club") &&
        (ply_ptr->ready[WIELD - 1]->type != BLUNT)) {
            print(fd, "You can only club with a blunt weapon.\n");
            return(0);
    }

	if(!strcmp(cmnd->str[0], "bludgeon") && (!ply_ptr->ready[WIELD - 1])) {
		print(fd, "You are not wielding a blunt weapon.\n");
	    return(0);
    }

	if(!strcmp(cmnd->str[0], "bludgeon") &&
        (ply_ptr->ready[WIELD - 1]->type != BLUNT)) {
            print(fd, "You can only bludgeon with a blunt weapon.\n");
            return(0);
    }


    if(cmnd->num < 2) {
        print(fd, "Attack what?\n");
        return(0);
    }


    rom_ptr = ply_ptr->parent_rom;

    crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);

    if(!crt_ptr) {
        cmnd->str[1][0] = up(cmnd->str[1][0]);
        crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply, cmnd->str[1], cmnd->val[1]);
        if(!crt_ptr || crt_ptr == ply_ptr || strlen(cmnd->str[1]) < 3) {
            print(fd, "You don't see that here.\n");
            return(0);
        }
    }

    attack_crt(ply_ptr, crt_ptr);

    return(0);
}

/**************************************************************************/
/* 		            attack_crt 			                  */
/**************************************************************************/
/* This function does the actual attacking.  The first parameter contains */
/* a pointer to the attacker and the second contains a pointer to the     */
/* victim.  A 1 is returned if the attack results in death.          	  */

int attack_crt(ply_ptr, crt_ptr)
creature    *ply_ptr;
creature    *crt_ptr;

{
    long    i, t;
    int 	fd, m, n, p, lev, addprof, d, wt, lucky;

    fd = ply_ptr->fd;

    t = time(0);
    i = LT(ply_ptr, LT_ATTCK);

    if(t < i)
        return(0);
	if(crt_ptr->type == PLAYER) {
		if(is_charm_crt(ply_ptr->name, crt_ptr) && F_ISSET(ply_ptr, PCHARM)) {
			print(fd, "You like %s too much to do that.\n", crt_ptr->name);
			return(0);
		}
		else
			del_charm_crt(ply_ptr->name, crt_ptr);
    }

    if(F_ISSET(ply_ptr, PCLOAK)) {
       	print(fd, "You cannot attack while you are cloaked.\n");
		return(0);
	}

    if(F_ISSET(ply_ptr, PHIDDN)) {
    	F_CLR(ply_ptr, PHIDDN);
		if(Ply[ply_ptr->fd].extr->agility >= 20) {
        	print(fd, "You spring from your hiding place.\n");
			broadcast_rom(fd, ply_ptr->rom_num, "%M jumps from hiding.", 
				ply_ptr);
    	}
		else {
        	print(fd, "Your poor agility causes you to stumble from your "
				"hiding place.\n");
			broadcast_rom(fd, ply_ptr->rom_num, "%M stumbles from hiding.", 
				ply_ptr);
    	}
    }

    if(F_ISSET(ply_ptr, PINVIS)) {
        F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
        print(fd, "Your invisibility fades.\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		broadcast_rom(fd, ply_ptr->rom_num, "%M gradually emerges into view.", 
			ply_ptr);
    }

    ply_ptr->lasttime[LT_ATTCK].ltime = t;
    ply_ptr->lasttime[LT_ATTCK].interval = 3;
    if(F_ISSET(ply_ptr, PHASTE))
        ply_ptr->lasttime[LT_ATTCK].interval = 2;
	if(F_ISSET(ply_ptr, PBLIND) || F_ISSET(ply_ptr, PFROZE))
		ply_ptr->lasttime[LT_ATTCK].interval = 7;
	if(F_ISSET(ply_ptr, PEVEYE) && ply_ptr->alignment > 0)
		ply_ptr->lasttime[LT_ATTCK].interval = 5;
	if(F_ISSET(ply_ptr, PEVEYE) && ply_ptr->alignment < -250)
		ply_ptr->lasttime[LT_ATTCK].interval = 2;

    if(F_ISSET(crt_ptr, MFEARS))
        crt_ptr->lasttime[LT_ATTCK].interval = 4;

    if(F_ISSET(crt_ptr, PEVEYE)) {
		if(crt_ptr->alignment > 0)
        	crt_ptr->lasttime[LT_ATTCK].interval = 4;
		else if(crt_ptr->alignment > -250)
        	crt_ptr->lasttime[LT_ATTCK].interval = 3;
		else
        	crt_ptr->lasttime[LT_ATTCK].interval = 2;
	}

    if(F_ISSET(crt_ptr, MFROZE))
        crt_ptr->lasttime[LT_ATTCK].interval = 5;

	if(crt_ptr->type == MONSTER) {
		if(F_ISSET(crt_ptr, MUNKIL)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "You cannot harm %s.\n",
					F_ISSET(crt_ptr, MMALES) ? "him":"her");
			}
			else
				print(fd, "You cannot harm it.\n");
			return(0);
		}
		if(F_ISSET(ply_ptr, PALIAS)) {
			print(fd, "You attack %m.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M attacks %m.",
				ply_ptr, crt_ptr);
		}
		else if(add_enm_crt(ply_ptr->name, crt_ptr) < 0 ) {
	    	print(fd, "You attack %m.\n", crt_ptr);
        		broadcast_rom(fd, ply_ptr->rom_num, "%M attacks %m.",
					ply_ptr, crt_ptr);
        }
        if(F_ISSET(crt_ptr, MMGONL)) {
          	print(fd, "Your attack has no effect.\n");
           	print(fd, "Only offensive magic spells can harm %m.\n", crt_ptr);
            return(0);
        }
        if(F_ISSET(crt_ptr, MENONL) && ply_ptr->class != MONK) {
            if(!ply_ptr->ready[WIELD - 1] || 
                ply_ptr->ready[WIELD - 1]->adjustment < 1) {
                	if(ply_ptr->ready[WIELD - 1]) 
            		    print(fd, "Your attack has no effect.\n");
					else
            		    print(fd, "You are not wielding a weapon and "
            				"only an enchanted weapon can harm %m.\n", crt_ptr);
                	return(0);
            }
        }
        if(F_ISSET(crt_ptr, MENONL) && ply_ptr->class == MONK) {
			if(!F_ISSET(ply_ptr, PPRAYD)) {
				print(fd, "Your hands are not sanctified and have no effect "
					"on %m.\n", crt_ptr);
               	return(0);
			}  
		}  

    }
    else {
        if(F_ISSET(ply_ptr->parent_rom, RNOKIL) && ply_ptr->class < DM) {
            print(fd, 
				"A ward of goodness will not allow killing in this area.\n");
        	broadcast_rom(fd, ply_ptr->rom_num, "%M failed in attacking %m.",
				ply_ptr, crt_ptr);
        	broadcast_rom(fd, ply_ptr->rom_num, 
				"A ward of goodness protects everyone in this area.");
            return(0);
        }
		if((!F_ISSET(ply_ptr, PPLDGK) || !F_ISSET(crt_ptr, PPLDGK)) ||
			(BOOL(F_ISSET(ply_ptr, PKNGDM)) == BOOL(F_ISSET(crt_ptr, PKNGDM))) 
				|| (! AT_WAR)) {
					if(!F_ISSET(ply_ptr, PCHAOS) && ply_ptr->class < DM) {
						print(fd, "You can't.  You're lawful.\n");
						return (0);
					}
					if(!F_ISSET(crt_ptr, PCHAOS) && ply_ptr->class < DM) {
						print(fd, "You can't.  That person is lawful.\n");
						return (0);
					}     
		}
		ply_ptr->lasttime[LT_ATTCK].interval += 5;
		ANSI(crt_ptr->fd, RED);
		print(crt_ptr->fd, "%M attacks you!\n", ply_ptr);
		ANSI(crt_ptr->fd, WHITE);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
			"%M attacked %m!", ply_ptr, crt_ptr);
	}

    if(ply_ptr->ready[WIELD - 1]) {
        if(ply_ptr->ready[WIELD - 1]->shotscur < 3) 
            print(fd, "Your %s is about to break.\n", 
				ply_ptr->ready[WIELD - 1]->name);
	}

    if(ply_ptr->ready[WIELD - 1]) {
        if(ply_ptr->ready[WIELD - 1]->shotscur < 1) {
			ANSI(fd, YELLOW);
            print(fd, "Your %s is broken.\n", 
				ply_ptr->ready[WIELD - 1]->name);
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
            broadcast_rom(fd, ply_ptr->rom_num, "%M broke %s %s.", ply_ptr,
                F_ISSET(ply_ptr, PMALES) ? "his":"her",
                	ply_ptr->ready[WIELD - 1]->name);
            add_obj_crt(ply_ptr->ready[WIELD - 1], ply_ptr);
            ply_ptr->ready[WIELD - 1] = 0;
            return(0);
        }
    }

    n = ply_ptr->thaco - crt_ptr->armor/10;

    if(F_ISSET(ply_ptr, PFEARS))
		n -= 2;

    if(F_ISSET(ply_ptr, PBLIND))
		n -= 5;

    if(F_ISSET(ply_ptr, PEVEYE) && ply_ptr->alignment > 0)
		n -= 5;

    if(F_ISSET(ply_ptr, PEVEYE) && ply_ptr->alignment < -250)
		n += 2;

	if(mrand(1, 100) < 10) {
		if(Ply[ply_ptr->fd].extr->agility < 20) {
			n -= 2;
			print(fd, "Your poor agility reduces your ability to fight.\n");
		}
	}

    if(mrand(1, 20) >= n) {
        if(ply_ptr->ready[WIELD - 1])
            n = mdice(ply_ptr->ready[WIELD - 1]) + bonus[ply_ptr->strength];
        else
            n = mdice(ply_ptr) + bonus[ply_ptr->strength];
        n = MAX(1, n);

		if(F_ISSET(ply_ptr, PSTRNG)) {
			if(mrand(1, 100) < 25) {
				n += 3;
				compute_thaco(ply_ptr);
				compute_agility(ply_ptr);
            	print(fd, "Your super strength increases your damage.\n");
			}
		}

        if(ply_ptr->class == PALADIN && mrand(1, 100) < 90) {
            if(ply_ptr->alignment < 0) {
                n /= 2;
                print(fd, "Your evilness reduces your damage.\n");
            }
            else if(ply_ptr->alignment > 250) {
                n += mrand(2, 4);
				if(F_ISSET(ply_ptr, PFEARS)) {
					n /= 2;
					print(fd, "Fear reduces your fighting ability.\n");
				}
				else {
					ANSI(fd, CYAN);
                	print(fd, "Your goodness increases your damage.\n");
					ANSI(fd, BOLD);
					ANSI(fd, WHITE);
            	}
            }
        }

		if(ply_ptr->class == MONK) {
			if(ply_ptr->ready[WIELD - 1] && 
				!F_ISSET(ply_ptr->ready[WIELD - 1], OMONKO)) {
					print(fd, 
						"Your damage is less because your hands are full.\n");
					n /= 2;
			}
		}
		if(ply_ptr->class == DRUID) {
			if(!F_ISSET(ply_ptr, PFEARS)) {
				if(ply_ptr->alignment < 250 && ply_ptr->alignment > -250) {
					n += mrand(2, 3);
					ANSI(fd, CYAN);
					print(fd, "Your neutrality increases your damage.\n");
					ANSI(fd, BOLD);
					ANSI(fd, WHITE);
				} 
			} 
		} 
		if(F_ISSET(ply_ptr, PEVEYE) && !F_ISSET(ply_ptr, PFEARS)) {
			if(ply_ptr->alignment > 0) {
				n -= mrand(4, 6);
				if(n < 1)
					n = 1; 
				ANSI(fd, CYAN);
				print(fd, "The evil eye spell reduces your damage.\n");
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
			} 
			if(ply_ptr->alignment < -250) {
				n += mrand(4, 6);
				ANSI(fd, CYAN);
				print(fd, "The evil eye spell enhances your damage.\n");
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
			} 
		} 

		p = mod_profic(ply_ptr);
		wt = (weight_ply(ply_ptr)/max_weight(ply_ptr)) * 4;
		lucky = Ply[ply_ptr->fd].extr->luck; 
		if(lucky >= 65)
			lucky = 1;
		else
			lucky = 0;

/*-----------------  CRITICAL HIT -------------------------------------*/

		if(mrand(1, 100) <= (p + lucky) || (ply_ptr->ready[WIELD - 1] && 
			F_ISSET(ply_ptr->ready[WIELD - 1], OALCRT))) {
				ANSI(fd, GREEN);
				print(fd, "CRITICAL HIT!\n");
				ANSI(fd, WHITE);
				broadcast_rom(fd, ply_ptr->rom_num, "%M made a critical hit.", 
					ply_ptr);
				n *= mrand(3, 6);
				if(F_ISSET(ply_ptr, PFEARS)) {
					n /= 3;
					print(fd, "Fear reduces your fighting ability.\n");
				}
				if(ply_ptr->ready[WIELD - 1] && 
					(!F_ISSET(ply_ptr->ready[WIELD - 1], ONSHAT)) && 
						((mrand(1, 100) < (3 - lucky)) || 
						(F_ISSET(ply_ptr->ready[WIELD - 1], OALCRT)))) {
							ANSI(fd, RED);
							print(fd, "Your %s shatters.\n",
								ply_ptr->ready[WIELD - 1]->name);
							ANSI(fd, WHITE);
							print(fd, "The %s is in a thousand pieces "
										"scattered around your feet.\n", 
											ply_ptr->ready[WIELD - 1]->name);
							broadcast_rom(fd, ply_ptr->rom_num,
								"%s %s shattered.",
									F_ISSET(ply_ptr, PMALES) ? "His":"Her",
										ply_ptr->ready[WIELD - 1]->name);
							free_obj(ply_ptr->ready[WIELD - 1]);
							ply_ptr->ready[WIELD - 1] = 0;
				}
		}

/*  FUMBLE */

	else if(mrand(1, 100) <= (5 + wt - (p + lucky)) && 
		ply_ptr->ready[WIELD - 1] && 
		!F_ISSET(ply_ptr->ready[WIELD - 1], OCURSE) &&
		ply_ptr->class != FIGHTER) {
			ANSI(fd, GREEN);
			print(fd, "You FUMBLED your weapon.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			if(wt >= 3)
				print(fd, "Perhaps you are carrying too much weight.\n");
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M fumbled %s weapon.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "his":"her");
			n = 0;
			add_obj_crt(ply_ptr->ready[WIELD - 1], ply_ptr);
			ply_ptr->ready[WIELD - 1] = 0;
			compute_thaco(ply_ptr);
			compute_agility(ply_ptr);
	}
	else if(mrand(1, 100) <= (5 + wt - (p + lucky)) && 
		ply_ptr->ready[WIELD - 1] && 
		!F_ISSET(ply_ptr->ready[WIELD - 1], OCURSE) &&
		ply_ptr->class == FIGHTER && ply_ptr->level < 4) {
			ANSI(fd, GREEN);
			print(fd, "You FUMBLED your weapon.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			if(wt >= 3)
				print(fd, "Perhaps you are carrying too much weight.\n");
			broadcast_rom(fd, ply_ptr->rom_num, "%M fumbled %s weapon.", 
				ply_ptr, F_ISSET(ply_ptr, PMALES) ? "his":"her");
			add_obj_crt(ply_ptr->ready[WIELD - 1], ply_ptr);
			ply_ptr->ready[WIELD - 1] = 0;
			compute_thaco(ply_ptr);
			compute_agility(ply_ptr);
			n = 0;
	}

	m = MIN(crt_ptr->hpcur, n);
	crt_ptr->hpcur -= n;
	print(fd, "You attack for %d damage.\n", n);
	broadcast_rom(fd, ply_ptr->rom_num, "%M hits %m.", ply_ptr, crt_ptr);
	ANSI(crt_ptr->fd, RED);
	print(crt_ptr->fd, "%M hit you for %d damage.\n", ply_ptr, n); 
	ANSI(crt_ptr->fd, BOLD);
	ANSI(crt_ptr->fd, WHITE);
	if(ply_ptr->type == MONSTER && crt_ptr->type == MONSTER)
	    broadcast_rom2(crt_ptr->fd, fd, crt_ptr->rom_num, "%M hits %m!", 
			ply_ptr, crt_ptr);
	if(ply_ptr->ready[WIELD - 1] && !mrand(0, 3)) {
    	if(!F_ISSET(ply_ptr->ready[WIELD - 1], ONUSED))	
               ply_ptr->ready[WIELD - 1]->shotscur--;
	}

	if(crt_ptr->type != PLAYER) {
    	if(is_charm_crt(crt_ptr->name, ply_ptr))
			del_charm_crt(crt_ptr, ply_ptr);  
		if(ply_ptr->ready[WIELD - 1]) {
	    	if(F_ISSET(ply_ptr->ready[WIELD - 1], OSILVR) && 
				F_ISSET(ply_ptr->ready[WIELD - 1], OTMPEN) && 
				F_ISSET(crt_ptr, MUNDED)) {
			    	F_SET(crt_ptr, MSHEAL);
					ANSI(fd, YELLOW);
	    			print(fd, 
					"The silver weapon opens unhealable wounds!\n");
	    			print(fd, "Your weapon transfers its deadly glow to %m.\n", 
						crt_ptr);
	    			ANSI(fd, BOLD);
	    			ANSI(fd, WHITE);
    	    		broadcast_rom2(crt_ptr->fd, fd, crt_ptr->rom_num,
						"%M wounds %m with a glowing silver weapon!", 
							ply_ptr, crt_ptr);
    	    		F_CLR((ply_ptr->ready[WIELD - 1]), OTMPEN);
    		}
 		}
	
		add_enm_dmg(ply_ptr->name, crt_ptr, m);
		if(ply_ptr->ready[WIELD - 1]) {
			p = MIN(ply_ptr->ready[WIELD - 1]->type, 4);
			addprof = (m * crt_ptr->experience)/MAX(crt_ptr->hpmax, 1);
			addprof = MIN(addprof, crt_ptr->experience);
			ply_ptr->proficiency[p] += addprof;
		}
		else if(ply_ptr->class == MONK) { 
			/* give blunt prof for monk barehand */
			addprof = (m * crt_ptr->experience)/MAX(crt_ptr->hpmax, 1);
			addprof = MIN(addprof, crt_ptr->experience);
			ply_ptr->proficiency[2] += addprof;
		}
       	else
        	check_for_flee(crt_ptr);
	}
	} 
	else {
		if(F_ISSET(ply_ptr, PEVEYE) && ply_ptr->alignment > 0) { 
			ANSI(fd, YELLOW);
			print(fd, "Zeth's evil eye caused you to stumble and miss.\n");
			ANSI(fd, WHITE);
		} 
		else
			print(fd, "You missed.\n");
		print(crt_ptr->fd, "%M missed.\n", ply_ptr);
		if(ply_ptr->type == MONSTER && crt_ptr->type == MONSTER)
			broadcast_rom2(fd, crt_ptr->fd, crt_ptr->rom_num, "%M missed %m.",
				ply_ptr, crt_ptr);
	}
   	if(crt_ptr->hpcur < 1) {
		ANSI(fd, YELLOW);	
   	   	print(fd, "You killed %m.\n", crt_ptr);
   		ANSI(fd, WHITE);	
   		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, "%M killed %m.", 
			ply_ptr, crt_ptr);
       	die(crt_ptr, ply_ptr);
       	return(1);
	}

    return(0);
}

/**********************************************************************/
/*			backstab				      */
/**********************************************************************/
/* This function allows thieves and assassins to backstab a monster.  */
/* If successful, a damage multiplier is given to the player.  The    */
/* player must be successfully hidden for the backstab to work.  If   */
/* the backstab fails, then the player is forced to wait double the   */
/* normal amount of time for his next attack.			      */

int backstab(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		i, t;
	int			fd, m, n, p, addprof;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->class != THIEF && ply_ptr->class != ASSASSIN &&
	   ply_ptr->class < CARETAKER) {
		print(fd, "Only thieves and assassins may backstab.\n");
		return(0);
	}

	if(cmnd->num < 2 || F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "Backstab what?\n");
		return(0);
	}

	if(!ply_ptr->ready[WIELD - 1] || (ply_ptr->ready[WIELD - 1]->type != SHARP 
		&& ply_ptr->ready[WIELD - 1]->type != THRUST)) {
			print(fd, "You must have either a sharp or thrusting weapon.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
		print(fd, "Not while you are cloaked.\n");
		return(0);
	}

	t = time(0);
	i = LT(ply_ptr, LT_ATTCK);

	if(t < i) {
		please_wait(fd, i - t);
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		cmnd->str[1][0] = up(cmnd->str[1][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[1], cmnd->val[1]);

		if(!crt_ptr || crt_ptr == ply_ptr || strlen(cmnd->str[1]) < 3) {
			print(fd, "You don't see that here.\n");
			return(0);
		}
	}

	if(crt_ptr->type != PLAYER && is_enm_crt(ply_ptr->name, crt_ptr)) {
		if(!F_ISSET(crt_ptr, MNOGEN)) {
			print(fd, "Not while you're already fighting %s.\n",
		    	F_ISSET(crt_ptr, MMALES) ? "him":"her");
		}
		else
			print(fd, "Not while you're already fighting it.\n");
		return(0);
	}

	if(crt_ptr->type == PLAYER) {
		if(F_ISSET(rom_ptr, RNOKIL) && ply_ptr->class < DM) {
			print(fd, 
				"A ward of goodness prevents any killing in this place.\n");
			return(0);
		}
		if((!F_ISSET(ply_ptr, PPLDGK) || !F_ISSET(crt_ptr, PPLDGK)) ||
			(BOOL(F_ISSET(ply_ptr, PKNGDM)) == BOOL(F_ISSET(crt_ptr, PKNGDM))) 
				|| (! AT_WAR)) {
					if(!F_ISSET(ply_ptr, PCHAOS) && ply_ptr->class < DM) {
						print(fd, "Sorry, you're lawful.\n");
						return (0);
					}
					if(!F_ISSET(crt_ptr, PCHAOS) && ply_ptr->class < DM) {
						print(fd, "Sorry, that player is lawful.\n");
						return (0);
					}     
					if(is_charm_crt(ply_ptr->name, crt_ptr) &&
						F_ISSET(ply_ptr, PCHARM)) {
							print(fd, "Backstab your very good friend, %s?\n", 
								crt_ptr->name);
							return(0);
					}
		}
		print(crt_ptr->fd, "%M backstabs you.\n", ply_ptr);
	}
	else if(F_ISSET(crt_ptr, MUNKIL)) {
		if(!F_ISSET(crt_ptr, MNOGEN)) {
			print(fd, "You cannot harm %s.\n",
				F_ISSET(crt_ptr, MMALES) ? "him":"her");
		}
		else
			print(fd, "You cannot harm it.\n");

		return(0);
	}

	if(is_charm_crt(crt_ptr->name, ply_ptr) && F_ISSET(ply_ptr, PCHARM)) {
        print(fd, "You are too fond of %s to do that.\n", crt_ptr->name);
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

	ply_ptr->lasttime[LT_ATTCK].ltime = t;
	if(ply_ptr->dexterity > 18 || ply_ptr->class == ASSASSIN)
		ply_ptr->lasttime[LT_ATTCK].interval = 2;
	else
		ply_ptr->lasttime[LT_ATTCK].interval = 3;

	if(crt_ptr->type != PLAYER) {
		if(add_enm_crt(ply_ptr->name, crt_ptr) < 0) {
			print(fd, "You backstab %m.\n", crt_ptr);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
		       "%M backstabs %m.", ply_ptr, crt_ptr);
 			if(F_ISSET(crt_ptr, MMGONL)) {
       			print(fd, "Your attack has no effect on %m.\n", crt_ptr);
       			print(fd, "%M can only be harmed by magic.\n", crt_ptr);
       			return(0);
       		}
        	if(F_ISSET(crt_ptr, MENONL) && ply_ptr->class != MONK) {
				if(!ply_ptr->ready[WIELD - 1] || 
					ply_ptr->ready[WIELD - 1]->adjustment < 1) {
						if(!ply_ptr->ready[WIELD - 1]) 
							print(fd, "You are not wielding a weapon.\n");
						else {
							print(fd, "Your weapon is not enchanted.\n");
							print(fd, "It has no effect on %m.\n", crt_ptr);
						}
					return(0);
        		}  
			}
        	if(F_ISSET(crt_ptr, MENONL) && ply_ptr->class == MONK) {
				if(!F_ISSET(ply_ptr, PPRAYD)) {
					print(fd, "Your hands are not sanctified and have no "								"effect on %m.\n", crt_ptr);
					return(0);
				}  
			}  
		}
	}
	else {
		print(fd, "You backstab %m.\n", crt_ptr);
			ANSI(crt_ptr->fd, RED);
			print(crt_ptr->fd, "%M backstabs you.\n", ply_ptr);
			ANSI(crt_ptr->fd, BOLD);
			ANSI(crt_ptr->fd, WHITE);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"%M backstabs %m!", ply_ptr, crt_ptr);
	}

	if(ply_ptr->ready[WIELD - 1]) {
		if(ply_ptr->ready[WIELD - 1]->shotscur < 1) {
			ANSI(crt_ptr->fd, YELLOW);
			print(fd, "Your %s is broken.\n", ply_ptr->ready[WIELD-1]->name);
			ANSI(crt_ptr->fd, BOLD);
			ANSI(crt_ptr->fd, WHITE);
			add_obj_crt(ply_ptr->ready[WIELD - 1], ply_ptr);
			ply_ptr->ready[WIELD - 1] = 0;
			broadcast_rom(fd, ply_ptr->rom_num, "%s backstab failed.", 
				F_ISSET(ply_ptr, PMALES) ? "His":"Her");
			ply_ptr->lasttime[LT_ATTCK].interval *= 2;
			return(0);
		}
	}

	n = ply_ptr->thaco - crt_ptr->armor/10 + 2;
	if(!F_ISSET(ply_ptr, PHIDDN))
		n = 21;

	F_CLR(ply_ptr, PHIDDN);

	if(mrand(1, 20) >= n) {
		if(ply_ptr->ready[WIELD - 1])
			n = mdice(ply_ptr->ready[WIELD - 1]);
		else
			n = mdice(ply_ptr);
		if(ply_ptr->class == THIEF)
			n *= (mrand(20, 35)/10);
		else
			n *= 3;
			if(F_ISSET(ply_ptr, PFEARS))
				n /= 2;
		m = MIN(crt_ptr->hpcur, n);
		if(crt_ptr->type != PLAYER) {
			add_enm_dmg(ply_ptr->name, crt_ptr, m);
			if(ply_ptr->ready[WIELD - 1]) {
				p = MIN(ply_ptr->ready[WIELD - 1]->type, 4);
				addprof = (m * crt_ptr->experience) / MAX(crt_ptr->hpmax, 1);
				addprof = MIN(addprof, crt_ptr->experience);
				ply_ptr->proficiency[p] += addprof;
			}
           	else if(ply_ptr->class == MONK) { 
		       	/* give blunt prof for monk barehand */
        	    addprof = (m * crt_ptr->experience) / MAX(crt_ptr->hpmax, 1);
              	addprof = MIN(addprof, crt_ptr->experience);
                ply_ptr->proficiency[2] += addprof;
           	}
		}

		crt_ptr->hpcur -= n;

		print(fd, "You hit for %d damage.\n", n);
		print(crt_ptr->fd, "%M hit you for %d damage.\n", ply_ptr, n);
			broadcast_rom(fd, ply_ptr->rom_num, "%M struck %m.", 
				ply_ptr, crt_ptr);

		if(crt_ptr->hpcur < 1) {
			print(fd, "You killed %m.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num,
			    "%M killed %m.", ply_ptr, crt_ptr);
			if(ply_ptr->ready[WIELD - 1])
				if(!F_ISSET(ply_ptr->ready[WIELD - 1], ONUSED))
					ply_ptr->ready[WIELD - 1]->shotscur--;
			die(crt_ptr, ply_ptr);
		}
		else
			check_for_flee(crt_ptr);
	}
	else {
		print(fd, "You missed.\n");
		broadcast_rom(fd, ply_ptr->rom_num, "%s backstab failed.", 
			F_ISSET(ply_ptr, PMALES) ? "His":"Her");
		ply_ptr->lasttime[LT_ATTCK].interval *= 3;
	}

	return(0);
}


/**********************************************************************/
/*				throatslit			      */
/**********************************************************************/
/* This function allows assassins to slit the throat of a monster.    */
/* If successful, a damage multiplier is given to the player.  The    */
/* player must be successfully hidden for the action to work.  If     */
/* the action fails, then the player is forced to wait double the     */
/* normal amount of time for his next attack.         		      */

int throatslit(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		i, t;
	int			fd, m, n, p, addprof;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->class != ASSASSIN && ply_ptr->class < CARETAKER) {
		print(fd, "Only assassins may slit throats.\n");
		return(0);
	}

	if(cmnd->num < 2 || F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "Slit whose throat?\n");
		return(0);
	}

	if(!ply_ptr->ready[WIELD - 1] || 
		(ply_ptr->ready[WIELD - 1]->type != SHARP )) {
			print(fd, "You must have a sharp weapon.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
		print(fd, "Not while you are cloaked.\n");
		return(0);
	}

	t = time(0);
	i = LT(ply_ptr, LT_ATTCK);

	if(t < i) {
		please_wait(fd, i - t);
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		cmnd->str[1][0] = up(cmnd->str[1][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[1], cmnd->val[1]);

		if(!crt_ptr || crt_ptr == ply_ptr || strlen(cmnd->str[1]) < 3) {
			print(fd, "You don't see that here.\n");
			return(0);
		}
	}

	if(crt_ptr->type != PLAYER && is_enm_crt(ply_ptr->name, crt_ptr)) {
		if(!F_ISSET(crt_ptr, MNOGEN)) {
			print(fd, "Not while you're already fighting %s.\n",
		    	F_ISSET(crt_ptr, MMALES) ? "him":"her");
		}
		else
			print(fd, "Not while you're already fighting it.\n");
		return(0);
	}

	if(crt_ptr->type == PLAYER) {
		if(F_ISSET(rom_ptr, RNOKIL) && ply_ptr->class < DM) {
			print(fd, 
				"A ward of goodness prevents any killing in this place.\n");
			return(0);
		}
		if((!F_ISSET(ply_ptr, PPLDGK) || !F_ISSET(crt_ptr, PPLDGK)) ||
			(BOOL(F_ISSET(ply_ptr, PKNGDM)) == BOOL(F_ISSET(crt_ptr, PKNGDM))) 
				|| (! AT_WAR)) {
					if(!F_ISSET(ply_ptr, PCHAOS) && ply_ptr->class < DM) {
						print(fd, "Sorry, you're lawful.\n");
						return (0);
					}
					if(!F_ISSET(crt_ptr, PCHAOS) && ply_ptr->class < DM) {
						print(fd, "Sorry, that player is lawful.\n");
						return (0);
					}     
					if(is_charm_crt(ply_ptr->name, crt_ptr) && 
						F_ISSET(ply_ptr, PCHARM)) {
							print(fd, "Slit the throat of your very good friend, %s?\n", 
								crt_ptr->name);
							return(0);
					}
		}
		print(crt_ptr->fd, "%M slices at your throat.\n", ply_ptr);
	}
	else if(F_ISSET(crt_ptr, MUNKIL)) {
		if(!F_ISSET(crt_ptr, MNOGEN)) {
			print(fd, "You cannot harm %s.\n",
				F_ISSET(crt_ptr, MMALES) ? "him":"her");
		}
		else
			print(fd, "You cannot harm it.\n");

		return(0);
	}

	if(is_charm_crt(crt_ptr->name, ply_ptr) && F_ISSET(ply_ptr, PCHARM)) {
        print(fd, "You are too fond of %s to do that.\n", crt_ptr->name);
        return(0);
    }

	if(F_ISSET(ply_ptr, PINVIS)) {
		F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
		print(fd, "Your invisibility fades.\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		broadcast_rom(fd, ply_ptr->rom_num, "%M emerges into view.", ply_ptr);
	}

	ply_ptr->lasttime[LT_ATTCK].ltime = t;
	if(ply_ptr->dexterity > 18)
		ply_ptr->lasttime[LT_ATTCK].interval = 1;
	else if(ply_ptr->dexterity > 14)
		ply_ptr->lasttime[LT_ATTCK].interval = 2;
	else
		ply_ptr->lasttime[LT_ATTCK].interval = 3;

	if(crt_ptr->type != PLAYER) {
		if(add_enm_crt(ply_ptr->name, crt_ptr) < 0) {
			print(fd, "You slice at the throat of %m.\n", crt_ptr);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
		       "%M slices at %m's throat.", ply_ptr, crt_ptr);
 			if(F_ISSET(crt_ptr, MMGONL)) {
       			print(fd, "Your attack has no effect on %m.\n", crt_ptr);
       			print(fd, "%M can only be harmed by magic.\n", crt_ptr);
       			return(0);
       		}
        	if(F_ISSET(crt_ptr, MENONL) && ply_ptr->class != MONK) {
				if(!ply_ptr->ready[WIELD - 1] || 
					ply_ptr->ready[WIELD - 1]->adjustment < 1) {
						if(!ply_ptr->ready[WIELD - 1]) 
							print(fd, "You are not wielding a weapon.\n");
						else {
							print(fd, "Your weapon is not enchanted.\n");
							print(fd, "It has no effect on %m.\n", crt_ptr);
						}
						return(0);
				}
        	}  
        	if(F_ISSET(crt_ptr, MENONL) && ply_ptr->class == MONK) {
				if(!F_ISSET(ply_ptr, PPRAYD))
					print(fd, "Your hands are not sanctified and have no effect on %m.\n", 
						crt_ptr);
					return(0);
			}  
		}
	}
	else {
		print(fd, "You slice at the throat of %m.\n", crt_ptr);
			ANSI(crt_ptr->fd, RED);
			print(crt_ptr->fd, "%M slices at your throat.\n", ply_ptr);
			ANSI(crt_ptr->fd, BOLD);
			ANSI(crt_ptr->fd, WHITE);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"%M slices at the throat of %m!", ply_ptr, crt_ptr);
	}

	if(ply_ptr->ready[WIELD - 1]) {
		if(ply_ptr->ready[WIELD - 1]->shotscur < 1) {
			ANSI(crt_ptr->fd, YELLOW);
			print(fd, "Your %s is broken.\n", ply_ptr->ready[WIELD-1]->name);
			ANSI(crt_ptr->fd, BOLD);
			ANSI(crt_ptr->fd, WHITE);
			add_obj_crt(ply_ptr->ready[WIELD - 1], ply_ptr);
			ply_ptr->ready[WIELD - 1] = 0;
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%s attempt to slit a throat failed.", 
				F_ISSET(ply_ptr, PMALES) ? "His":"Her");
			ply_ptr->lasttime[LT_ATTCK].interval *= 2;
			return(0);
		}
	}

	n = (mrand(15, 50) + ply_ptr->level);

	if(!F_ISSET(ply_ptr, PHIDDN))
		n /= 2;
	if(F_ISSET(ply_ptr, PHIDDN)) {
		F_CLR(ply_ptr, PHIDDN);
		print(fd, "You step from your hiding place.\n");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M steps from hiding to slice at %m.", ply_ptr, crt_ptr);
	}

	if(crt_ptr->type == PLAYER) {
		if(crt_ptr->level < (ply_ptr->level + 3)) {
			print(fd, "Your bloody hand slipped!\n");
			n = 1;
		}
	}
	else {
		if(crt_ptr->level > (ply_ptr->level + 1)) {
			print(fd, "Your bloody hand slipped!\n");
			n = 1;
		}
	}

	if(mrand(1, 20) <= n) {
		if(F_ISSET(ply_ptr, PFEARS))
			n /= 2;
		m = MIN(crt_ptr->hpcur, n);
		if(crt_ptr->type != PLAYER) {
			add_enm_dmg(ply_ptr->name, crt_ptr, m);
			if(ply_ptr->ready[WIELD - 1]) {
				p = MIN(ply_ptr->ready[WIELD - 1]->type, 0);
				addprof = (m * crt_ptr->experience) / MAX(crt_ptr->hpmax, 1);
				addprof = MIN(addprof, crt_ptr->experience);
				ply_ptr->proficiency[p] += addprof;
			}
		}

		crt_ptr->hpcur -= n;

		print(fd, "You sliced for %d damage.\n", n);
		print(crt_ptr->fd, "%M sliced you for %d damage.\n", ply_ptr, n);
			broadcast_rom(fd, ply_ptr->rom_num, "%M sliced %m.", 
				ply_ptr, crt_ptr);

		if(crt_ptr->hpcur < 1) {
			print(fd, "You killed %m.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num,
			    "%M killed %m.", ply_ptr, crt_ptr);
			if(ply_ptr->ready[WIELD - 1])
				if(!F_ISSET(ply_ptr->ready[WIELD - 1], ONUSED))
					ply_ptr->ready[WIELD - 1]->shotscur--;
			die(crt_ptr, ply_ptr);
		}
		else
			check_for_flee(crt_ptr);
	}
	else {
		print(fd, "You missed.\n");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%s attempt to slit a throat failed.", 
			F_ISSET(ply_ptr, PMALES) ? "His":"Her");
		ply_ptr->lasttime[LT_ATTCK].interval *= 3;
	}

	return(0);
}

/***********************************************************************/
/*			touch_of_death				       */
/***********************************************************************/
/* This function allows monks to kill non-undead creatures.            */
/* If they succeed, then the creature is either killed or harmed       */
/* for approximately half of its hit points.  A neutral monk will not  */
/* be able to use the touch_of_death.                                  */

int touch_of_death(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		i, t;
	int			chance, m, dmg, fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Touch whom?\n");
		return(0);
	}

	if(ply_ptr->class != MONK && ply_ptr->class < CARETAKER) {
		print(fd, "Only monks may use that skill.\n");
		return(0);
	}

	if(ply_ptr->level < 7) {
		print(fd, "You are not experienced enough to do that.\n");
		return(0);
	}

	if(ply_ptr->alignment > -100 && ply_ptr->alignment < 100) {
		print(fd, 
			"You are neither good nor evil, thus cannot use the touch.\n");
		return(0);
	}

	if(ply_ptr->ready[WIELD - 1]) {
		print(fd, "Your hands are full.  How can you do that?\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
		print(fd, "Not while you are cloaked.\n");
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		cmnd->str[1][0] = up(cmnd->str[1][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			 cmnd->str[1], cmnd->val[1]);
				if(!crt_ptr) {
					print(fd, "They aren't here.\n");
					return(0);
				}
	}
	if((!F_ISSET(ply_ptr,PPLDGK) || !F_ISSET(crt_ptr,PPLDGK)) || 
		(BOOL(F_ISSET(ply_ptr, PKNGDM)) == BOOL (F_ISSET(crt_ptr, PKNGDM))) || 
			(!AT_WAR)) {
                if(!F_ISSET(ply_ptr, PCHAOS) && crt_ptr->type != MONSTER && 
					ply_ptr->class < DM) {
                    	print(fd, "Sorry, you're lawful.\n");
                    	return (0);
                }
                if(!F_ISSET(crt_ptr, PCHAOS) && ply_ptr->class < DM && 
					crt_ptr->type != MONSTER) {
                    	print(fd, "Sorry, that player is lawful.\n");
                    	return (0);
                }
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "How do you do that?  You're blind.\n");
		return(0);
	}

	if(F_ISSET(rom_ptr, RNOKIL) && ply_ptr->class < DM) {
		print(fd, "Not here.\n");
		return(0);
	}
	
	if(F_ISSET(crt_ptr, MUNDED) && crt_ptr->type == MONSTER) {
		print(fd, "That won't work on the undead.\n");
		add_enm_crt(ply_ptr->name, crt_ptr);		
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

	i = ply_ptr->lasttime[LT_TOUCH].ltime;
        t = time(0);

	if((t - i) < 600L) {
		print(fd, "Please wait %d:%02d more minutes.\n",
			(600L - t + i)/60L, (600L - t + i)%60L);
		return(0);
	}

	if(crt_ptr->type != PLAYER && F_ISSET(crt_ptr, MUNKIL)) {
		if(!F_ISSET(crt_ptr, MNOGEN)) {
			print(fd, "No one can harm %s.\n",
				F_ISSET(crt_ptr, MMALES) ? "him":"her");
		}
		else
			print(fd, "No one can harm it.\n");
		return(0);
	}
	
	if(crt_ptr->type == MONSTER)
		 add_enm_crt(ply_ptr->name, crt_ptr);

	ply_ptr->lasttime[LT_TOUCH].ltime = t;
	ply_ptr->lasttime[LT_ATTCK].ltime = t;
	ply_ptr->lasttime[LT_TOUCH].interval = 600L;

	chance = (ply_ptr->level - crt_ptr->level) * 20 
		+ bonus[ply_ptr->constitution] * 10;
	if(chance < 0)
		chance = 0;
	chance = MIN(chance, 85);

	if(mrand(1, 100) > chance) {
		print(fd, "You failed to harm %m.\n", crt_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M attempted the touch of death on %m and failed.\n", 
				ply_ptr, crt_ptr);
		return(0);
	}

	if((mrand(1, 100) > 80 - bonus[ply_ptr->constitution]) && 
		crt_ptr->type == MONSTER && !F_ISSET(crt_ptr, MPERMT)) {
			print(fd, "You fatally wound %m.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M fatally wounds %m.", ply_ptr, crt_ptr);
			add_enm_dmg(ply_ptr->name, crt_ptr, crt_ptr->hpcur);
			die(crt_ptr, ply_ptr);	
	}
	else {
		dmg = MAX(1, crt_ptr->hpcur/2);
		m = MIN(crt_ptr->hpcur, dmg);
		crt_ptr->hpcur -= dmg;
		add_enm_dmg(ply_ptr->name, crt_ptr, m);
		print(fd, "You touched %m for %d damage.\n", crt_ptr, dmg);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M uses the touch of death on %m.", ply_ptr, crt_ptr);
	    if(crt_ptr->hpcur < 1) {
			print(fd, "You killed %m.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M killed %m.", 
				ply_ptr, crt_ptr);
			die(crt_ptr, ply_ptr);
		}
	}

	return(0);
}

/***************************************************************************/
/*				rescue					   */
/***************************************************************************/
/* 	This function allows one player to rescue another.  The rescued    */
/* 	player's name is removed from the creature's enemy list, and the   */
/* 	rescuer's name is added.					   */ 

int rescue(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
	creature	*vic_ptr, *crt_ptr;
	room		*rom_ptr;
	ctag		*pp, *cp, *prev;
	int			fd, chance = 50;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 3) {
		print(fd, "syntax: rescue <victim> <monster>\n");
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[2],
		cmnd->val[2]);

	if(!crt_ptr) {
		print (fd, "Can't seem to locate that attacker here.\n");
		return (0);
	}

	lowercize(cmnd->str[1], 1);
	vic_ptr = find_who(cmnd->str[1]);

	if(!vic_ptr) {
		print (fd, "Can't seem to locate that person here.\n");
		print (fd, "Please use full names.\n");
		return (0);
	}

	if(vic_ptr == ply_ptr) {
		print(fd, "Nice try, but it won't work...\n");
		return(0);
	}

	if(ply_ptr->class < CARETAKER) {
		if(F_ISSET(ply_ptr, PINVIS)) {
			F_CLR(ply_ptr, PINVIS);
			print(fd, "You emerge from invisibility.\n");
			broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", 
				ply_ptr); 
		}

		if(F_ISSET(ply_ptr, PHIDDN)) {
			F_CLR(ply_ptr, PHIDDN);
			print(fd, "You come out of hiding.\n");
			broadcast_rom(fd, ply_ptr->rom_num, "%M comes out of hiding.", 
				ply_ptr); 
		}
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
		print(fd, "Not while you are cloaked.\n");
		return(0);
	}

	if(ply_ptr->class >= CARETAKER)
		chance = 100;
	if(ply_ptr->class == PALADIN)
		chance += ply_ptr->level + ply_ptr->alignment/100;
	if(F_ISSET(crt_ptr, MPERMT))
		chance = 0;

	if(chance < mrand(1, 70)) {
		print(fd, "You failed to rescue %m.\n", vic_ptr);
		print(vic_ptr->fd, "%M failed to rescue you.\n", ply_ptr);
		print(fd, "%M attacked you!\n", crt_ptr); 
		broadcast_rom2(vic_ptr->fd, ply_ptr->fd, vic_ptr->rom_num, 
			"%M failed to rescue %m from %m.", ply_ptr, vic_ptr, crt_ptr);
		add_enm_crt(ply_ptr->name, crt_ptr);
	}
	else {
		del_enm_crt(vic_ptr->name, crt_ptr);
		print(vic_ptr->fd, 
			"%M stops attacking as %m steps in to rescue you!\n", 
				crt_ptr, ply_ptr); 

		vic_ptr->lasttime[LT_ATTCK].ltime = time(0);
		vic_ptr->lasttime[LT_ATTCK].interval = 15;

		print(fd, "You rescue %M from %m.\n", vic_ptr, crt_ptr); 
		if(ply_ptr->class == PALADIN) {
			ply_ptr->experience += vic_ptr->level;
			print(fd, "You gained %d experience for your good deed.\n", 
				vic_ptr->level);
		}
		if(ply_ptr->alignment <= 0 && vic_ptr->alignment >= 0) {
			print(fd, "Your good deed changes your alignment!\n");
			ply_ptr->alignment += 250;
		}
		
		if(ply_ptr->class < CARETAKER) {
			print(fd, "%M attacked you!\n", crt_ptr); 
			add_enm_crt(ply_ptr->name, crt_ptr);
			broadcast_rom2(vic_ptr->fd, ply_ptr->fd, vic_ptr->rom_num, 
				"%M rescues %m from %m.", ply_ptr, vic_ptr, crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M attacks %m.", 
				crt_ptr, ply_ptr);
		}
	}

	return(0);
}

/**********************************************************************/
/*			circle					      */
/**********************************************************************/
/* This function allows fighters and barbarians to run circles about  */
/* an enemy, confusing it for several seconds.			      */

int circle(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		i, t;
	int			chance, delay, fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Circle whom?\n");
		return(0);
	}

	if(ply_ptr->class != FIGHTER && ply_ptr->class != BARBARIAN &&
	   ply_ptr->class < CARETAKER) {
		print(fd, "Only barbarians and fighters may circle.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
		print(fd, "Not while you are cloaked.\n");
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		cmnd->str[1][0] = up(cmnd->str[1][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[1], cmnd->val[1]);

		if(!crt_ptr || crt_ptr == ply_ptr || strlen(cmnd->str[1]) < 3) {
			print(fd, "That is not here.\n");
			return(0);
		}
	}

	if(crt_ptr->type == PLAYER) {
		if(F_ISSET(rom_ptr, RNOKIL) && ply_ptr->class < DM) {
			print(fd, "No killing allowed in this room.\n");
			return(0);
		}
		if((!F_ISSET(ply_ptr,PPLDGK) || !F_ISSET(crt_ptr,PPLDGK)) ||
			(BOOL(F_ISSET(ply_ptr,PKNGDM)) == BOOL(F_ISSET(crt_ptr,PKNGDM))) ||
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
		if(is_charm_crt(ply_ptr->name, crt_ptr) && F_ISSET(crt_ptr, PCHARM)) {
			print(fd, "You are too fond of %s to do that.\n", crt_ptr->name);
			return(0);
		}
	}

	i = LT(ply_ptr, LT_ATTCK);
	t = time(0);

	if(i > t) {
		please_wait(fd, i - t);
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);
	if(F_ISSET(ply_ptr, PINVIS)) {
		F_CLR(ply_ptr, PINVIS);
		print(fd, "Your invisibility fades.\n");
		broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
	}

	chance = 50 + (ply_ptr->level - crt_ptr->level) * 10 +
		 (bonus[ply_ptr->dexterity] - bonus[crt_ptr->dexterity]) * 2;
	if (crt_ptr->type == MONSTER && F_ISSET(crt_ptr, MUNDED))
		chance -= (5 + crt_ptr->level * 2);
	chance = MIN(80, chance);
	if(F_ISSET(crt_ptr, MNOCIR) || F_ISSET(ply_ptr, PBLIND))
		chance = 10;
		
	if(crt_ptr->type != PLAYER) {
		if(F_ISSET(crt_ptr, MUNKIL)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "You cannot harm %s.\n",
					F_ISSET(crt_ptr, MMALES) ? "him":"her");
			}
			else
				print(fd, "You cannot harm it.\n");
			return(0);
		}

		add_enm_crt(ply_ptr->name, crt_ptr);
	}

	if(mrand(1, 100) <= chance) {
		if(ply_ptr->class == BARBARIAN)
			delay = mrand(4, 6);
		else
			delay = mrand(5, 10);
		crt_ptr->lasttime[LT_ATTCK].ltime = t;
		crt_ptr->lasttime[LT_ATTCK].interval = delay;

		print(fd, "You circle %m.\n", crt_ptr);

		print(crt_ptr->fd, "%M circles you.\n", ply_ptr);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
	       "%M circles %m.", ply_ptr, crt_ptr);
		ply_ptr->lasttime[LT_ATTCK].interval = 2;
	}
	else {
		print(fd, "You failed to circle it.\n");
		print(crt_ptr->fd, "%M tried to circle you.\n", ply_ptr);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
			       "%M tried to circle %m.", ply_ptr, crt_ptr);
		ply_ptr->lasttime[LT_ATTCK].interval = 3;
	}

	ply_ptr->lasttime[LT_ATTCK].ltime = t;

	return(0);

}

/**********************************************************************/
/*			bash					      */
/**********************************************************************/
/* This function allows barbarians to "bash" an opponent,  it does    */
/* more damage than a normal attack, and also knocks the opponent     */
/* over for a few seconds, leaving him unable to attack back.	      */

int bash(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		i, t;
	int			m, n, chance, fd, p, addprof;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Bash whom?\n");
		return(0);
	}

	if(ply_ptr->class != BARBARIAN && ply_ptr->class < CARETAKER) {
		print(fd, "Only barbarians are capable of bashing their opponents.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
		print(fd, "Not while you are cloaked.\n");
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
			cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		cmnd->str[1][0] = up(cmnd->str[1][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[1], cmnd->val[1]);

		if(!crt_ptr || crt_ptr == ply_ptr || strlen(cmnd->str[1]) < 3) {
			print(fd, "That isn't here.\n");
			return(0);
		}
	}

	if(crt_ptr->type != PLAYER && is_enm_crt(ply_ptr->name, crt_ptr)) {
		if(!F_ISSET(crt_ptr, MNOGEN)) {
			print(fd, "Not while you're already fighting %s.\n",
		    	F_ISSET(crt_ptr, MMALES) ? "him":"her");
		}
		else
			print(fd, "Not while you're already fighting it.\n");
	
		return(0);
	}
	else if(crt_ptr->type == PLAYER) {
		if(F_ISSET(rom_ptr, RNOKIL)) {
			print(fd, "No killing allowed in this area.\n");
			return(0);
		}
		if((!F_ISSET(ply_ptr, PPLDGK) || !F_ISSET(crt_ptr, PPLDGK)) ||
			(BOOL(F_ISSET(ply_ptr, PKNGDM)) == BOOL(F_ISSET(crt_ptr, PKNGDM))) 
				|| (! AT_WAR)) {
                	if(!F_ISSET(ply_ptr, PCHAOS) && ply_ptr->class < DM) {
                	    print(fd, "Sorry, you're lawful.\n");
                	    return (0);
                	}
                	if(!F_ISSET(crt_ptr, PCHAOS) && ply_ptr->class < DM) {
                	    print(fd, "Sorry, that citizen is lawful.\n");
                	    return (0);
                	}     
		}
		if(is_charm_crt(ply_ptr->name, crt_ptr) && F_ISSET(crt_ptr, PCHARM)) {
			print(fd, "You like %s too much to do that.\n", crt_ptr->name);
			return(0);
		}
	}

	i = LT(ply_ptr, LT_ATTCK);
	t = time(0);

	if(i > t) {
		please_wait(fd, i - t);
		return(0);
	}

	ply_ptr->lasttime[LT_ATTCK].ltime = t;
	ply_ptr->lasttime[LT_ATTCK].interval = 3;
	ply_ptr->lasttime[LT_SPELL].ltime = t;
        ply_ptr->lasttime[LT_SPELL].interval = 5;
	
	F_CLR(ply_ptr, PHIDDN);
	if(F_ISSET(ply_ptr, PINVIS)) {
		F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
		print(fd, "Your invisibility fades.\n");
		ANSI(fd, WHITE);
		broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
	}

	if(crt_ptr->type != PLAYER) {
		if(F_ISSET(crt_ptr, MUNKIL)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "You cannot harm %s.\n",
					F_ISSET(crt_ptr, MMALES) ? "him":"her");
			}
			else
				print(fd, "You cannot harm it.\n");
			return(0);
		}
		if(F_ISSET(crt_ptr, MMGONL)) {
			print(fd, "Your attack has no effect on %m.\n", crt_ptr);
			return(0);
		}
        if(F_ISSET(crt_ptr, MENONL) && ply_ptr->class != MONK) {
			if(!ply_ptr->ready[WIELD - 1] || 
				ply_ptr->ready[WIELD - 1]->adjustment < 1) {
					if(!ply_ptr->ready[WIELD - 1])
						print(fd, "You are not wielding a weapon.\n");
					else 
						print(fd, "Your weapon is not enchanted and has "
									"no effect on %m.\n", crt_ptr);
				return(0);
			}
		}  
        if(F_ISSET(crt_ptr, MENONL) && ply_ptr->class == MONK) {
			if(!F_ISSET(ply_ptr, PPRAYD)) 
				print(fd, "Your hands are not sanctified and have no effect "
					"on %m.\n", crt_ptr);
				return(0);
		}  


	/*	if(is_charm_crt(crt_ptr->name, ply_ptr))
			del_charm_crt(crt_ptr, ply_ptr); */
		
		add_enm_crt(ply_ptr->name, crt_ptr);
	}

	chance = 50 + (ply_ptr->level - crt_ptr->level) * 10 +
		bonus[ply_ptr->strength] * 3 +
		 	(bonus[ply_ptr->dexterity] - bonus[crt_ptr->dexterity]) * 2;
	chance = MIN(85, chance);
	if(F_ISSET(ply_ptr, PBLIND))
		chance = MIN(20, chance);

	if(mrand(1, 100) <= chance) {
		if(ply_ptr->ready[WIELD - 1]) {
			if((ply_ptr->ready[WIELD - 1]->shotscur < 4) && 
				(ply_ptr->ready[WIELD - 1]->shotscur > 1)) {  
					print(fd, "Your %s is almost broken.\n", 
				    	ply_ptr->ready[WIELD - 1]->name);
					return(0);
			}
			if(ply_ptr->ready[WIELD - 1]->shotscur < 1) {
				ANSI(fd, YELLOW);
				print(fd, "Your %s is broken.\n", 
				      ply_ptr->ready[WIELD - 1]->name);
				ANSI(fd, WHITE);
				add_obj_crt(ply_ptr->ready[WIELD - 1], ply_ptr);
				ply_ptr->ready[WIELD - 1] = 0;
				return(0);
			}
		}

		n = ply_ptr->thaco - crt_ptr->armor/10;
		if(mrand(1, 20) >= n) {
			crt_ptr->lasttime[LT_ATTCK].ltime = t;
			crt_ptr->lasttime[LT_ATTCK].interval = mrand(5, 8);
			crt_ptr->lasttime[LT_SPELL].ltime = t;
				crt_ptr->lasttime[LT_SPELL].interval = mrand(7, 10);
			
			if(ply_ptr->ready[WIELD - 1])
				n = mdice(ply_ptr->ready[WIELD - 1]) * 2;
			else
				n = mdice(ply_ptr) + 4;

			if(mrand(1, 100) > 90)
				n = (n * 3)/2;

			m = MIN(crt_ptr->hpcur, n);
			if(crt_ptr->type != PLAYER) {
				add_enm_dmg(ply_ptr->name, crt_ptr, m);
				if(ply_ptr->ready[WIELD - 1]) {
					p = MIN(ply_ptr->ready[WIELD - 1]->type, 4);
					addprof = (m * crt_ptr->experience)/MAX(crt_ptr->hpmax, 1);
					addprof = MIN(addprof, crt_ptr->experience);
					ply_ptr->proficiency[p] += addprof;
				}
			}
			crt_ptr->hpcur -= n;

			ANSI(fd, YELLOW);
			print(fd, "You bash for %d damage.\n", n);
			ANSI(fd, WHITE);
			broadcast_rom2(fd, crt_ptr->fd, crt_ptr->rom_num, 
				"%M bashes %m.", ply_ptr, crt_ptr);
			ANSI(crt_ptr->fd, RED);
			print(crt_ptr->fd, "%M bashed you for %d damage.\n", ply_ptr, n);
			ANSI(crt_ptr->fd, WHITE);

			ply_ptr->lasttime[LT_ATTCK].interval = 2;

			if(crt_ptr->hpcur < 1) {
				ANSI(fd, YELLOW);
				print(fd, "You killed %m.\n", crt_ptr);
				ANSI(fd, WHITE);
				broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
					"%M killed %m.", ply_ptr, crt_ptr);
				die(crt_ptr, ply_ptr);
			}
			else
				check_for_flee(crt_ptr);
		}
		else {
			print(fd, "Your bash failed.\n");
			ANSI(crt_ptr->fd, CYAN);
			print(crt_ptr->fd, "%M tried to bash you.\n", ply_ptr);
			ANSI(crt_ptr->fd, WHITE);
			broadcast_rom2(fd, crt_ptr->fd, crt_ptr->rom_num, 
				"%M tried to bash %m but failed.", ply_ptr, crt_ptr);
		}
	}

	else {
		print(fd, "Your bash failed.\n");
		ANSI(crt_ptr->fd, CYAN);
		print(crt_ptr->fd, "%M tried to bash you.\n", ply_ptr);
		ANSI(crt_ptr->fd, WHITE);
		broadcast_rom2(fd, crt_ptr->fd, crt_ptr->rom_num, 
			"%M tried to bash %m but failed.", ply_ptr, crt_ptr);
	}

	return(0);

}

/**********************************************************************/
/*			cripple					      */
/**********************************************************************/
/* This function allows fighters and assassins to cripple an opponent */
/* prior to an attack.  It does not do much damage, but it does slow  */
/* the opponent down, making it easier to attack.	              */ 

int cripple(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		i, t;
	int			m, n, chance, fd, p, addprof;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Cripple whom?\n");
		return(0);
	}

	if(ply_ptr->class != FIGHTER && 
		ply_ptr->class != ASSASSIN && 
		ply_ptr->class < CARETAKER) {
			print(fd, "Only fighters and assassins know that technique.\n");
			return(0);
	}

	if(F_ISSET(ply_ptr, PCLOAK)) {
		print(fd, "Not while you are cloaked.\n");
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
			cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		cmnd->str[1][0] = up(cmnd->str[1][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[1], cmnd->val[1]);

		if(!crt_ptr || crt_ptr == ply_ptr || strlen(cmnd->str[1]) < 3) {
			print(fd, "That isn't here.\n");
			return(0);
		}
	}

	if(crt_ptr->type == PLAYER) {
		if(F_ISSET(rom_ptr, RNOKIL)) {
			print(fd, "No harm is allowed in this area.\n");
			return(0);
		}
		if((!F_ISSET(ply_ptr, PPLDGK) || !F_ISSET(crt_ptr, PPLDGK)) ||
			(BOOL(F_ISSET(ply_ptr, PKNGDM)) == BOOL(F_ISSET(crt_ptr, PKNGDM))) 
				|| (! AT_WAR)) {
                	if(!F_ISSET(ply_ptr, PCHAOS) && ply_ptr->class < DM) {
                	    print(fd, "Sorry, you're lawful.\n");
                	    return (0);
                	}
                	if(!F_ISSET(crt_ptr, PCHAOS) && ply_ptr->class < DM) {
                	    print(fd, "Sorry, that citizen is lawful.\n");
                	    return (0);
                	}     
		}
		if(is_charm_crt(ply_ptr->name, crt_ptr) && F_ISSET(crt_ptr, PCHARM)) {
			print(fd, "You like %s too much to do that.\n", crt_ptr->name);
			return(0);
		}
	}

	i = LT(ply_ptr, LT_ATTCK);
	t = time(0);

	if(i > t) {
		please_wait(fd, i - t);
		return(0);
	}

	ply_ptr->lasttime[LT_ATTCK].ltime = t;
	ply_ptr->lasttime[LT_ATTCK].interval = 3;
	ply_ptr->lasttime[LT_SPELL].ltime = t;
        ply_ptr->lasttime[LT_SPELL].interval = 5;
	
	F_CLR(ply_ptr, PHIDDN);
	if(F_ISSET(ply_ptr, PINVIS)) {
		F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
		print(fd, "Your invisibility fades.\n");
		ANSI(fd, WHITE);
		broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
	}

	if(crt_ptr->type != PLAYER) {
		if(F_ISSET(crt_ptr, MUNKIL)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "You cannot harm %s.\n",
					F_ISSET(crt_ptr, MMALES) ? "him":"her");
			}
			else
				print(fd, "You cannot harm it.\n");
			return(0);
		}
		if(F_ISSET(crt_ptr, MMGONL)) {
			print(fd, "Your attack has no effect on %m.\n", crt_ptr);
			return(0);
		}
        if(F_ISSET(crt_ptr, MENONL) && ply_ptr->class != MONK) {
			if(!ply_ptr->ready[WIELD - 1] || 
				ply_ptr->ready[WIELD - 1]->adjustment < 1) {
					if(!ply_ptr->ready[WIELD - 1])
						print(fd, "You are not wielding a weapon.\n");
					else 
						print(fd, "Your weapon is not enchanted and has "
									"no effect on %m.\n", crt_ptr);
					return(0);
			}
		}  
        if(F_ISSET(crt_ptr, MENONL) && ply_ptr->class == MONK) {
			if(!F_ISSET(ply_ptr, PPRAYD))
				print(fd, "Your hands are not sanctified and have no effect "
					"on %m.\n", crt_ptr);
				return(0);
		}  

		add_enm_crt(ply_ptr->name, crt_ptr);
	}

	chance = 60 + (ply_ptr->level - crt_ptr->level) * 5 +
		bonus[ply_ptr->strength] * 3 +
		 	(bonus[ply_ptr->dexterity] - bonus[crt_ptr->dexterity]) * 2;
	chance = MIN(40, chance);
	if(F_ISSET(ply_ptr, PBLIND))
		chance = MIN(20, chance);
	if(ply_ptr->class >= CARETAKER)
		chance = MIN(80, chance);


	if(mrand(1, 100) <= chance) {
		if(ply_ptr->ready[WIELD - 1]) {
			if((ply_ptr->ready[WIELD - 1]->shotscur < 4) && 
				(ply_ptr->ready[WIELD - 1]->shotscur > 1)) {  
					print(fd, "Your %s is almost broken.\n", 
				    	ply_ptr->ready[WIELD - 1]->name);
					return(0);
			}
			if(ply_ptr->ready[WIELD - 1]->shotscur < 1) {
				ANSI(fd, YELLOW);
				print(fd, "Your %s is broken.\n", 
				      ply_ptr->ready[WIELD - 1]->name);
				ANSI(fd, WHITE);
				add_obj_crt(ply_ptr->ready[WIELD - 1], ply_ptr);
				ply_ptr->ready[WIELD - 1] = 0;
				return(0);
			}
		}

		n = ply_ptr->thaco - crt_ptr->armor/10;
		if(mrand(1, 20) >= n) {
			crt_ptr->lasttime[LT_ATTCK].ltime = t;
			crt_ptr->lasttime[LT_ATTCK].interval = mrand(6, 9);
			crt_ptr->lasttime[LT_SPELL].ltime = t;
				crt_ptr->lasttime[LT_SPELL].interval = mrand(7, 10);
			
			if(ply_ptr->ready[WIELD - 1])
				n = mdice(ply_ptr->ready[WIELD - 1])/2;
			else
				n = mdice(ply_ptr);

			m = MIN(crt_ptr->hpcur, n);
			if(crt_ptr->type != PLAYER) {
				add_enm_dmg(ply_ptr->name, crt_ptr, m);
				if(ply_ptr->ready[WIELD - 1]) {
					p = MIN(ply_ptr->ready[WIELD - 1]->type, 4);
					addprof = (m * crt_ptr->experience)/MAX(crt_ptr->hpmax, 1);
					addprof = MIN(addprof, crt_ptr->experience);
					ply_ptr->proficiency[p] += addprof;
				}
			}
			crt_ptr->thaco += 1;
			if(crt_ptr->thaco > 20)
				crt_ptr->thaco = 20;
			crt_ptr->hpcur -= n;

			ANSI(fd, YELLOW);
			print(fd, "You cripple %m, inflicting %d damage.\n", crt_ptr, n);
			ANSI(fd, WHITE);
			broadcast_rom2(fd, crt_ptr->fd, crt_ptr->rom_num, 
				"%M cripples %m.", ply_ptr, crt_ptr);
			ANSI(crt_ptr->fd, RED);
			print(crt_ptr->fd, "%M crippled you and inflicted %d damage.\n", 
				ply_ptr, n);
			ANSI(crt_ptr->fd, WHITE);

			ply_ptr->lasttime[LT_ATTCK].interval = 2;

			if(crt_ptr->hpcur < 1) {
				ANSI(fd, YELLOW);
				print(fd, "Your crippling attack killed %m.\n", crt_ptr);
				ANSI(fd, WHITE);
				broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
					"%M's crippling attack killed %m.", ply_ptr, crt_ptr);
				die(crt_ptr, ply_ptr);
			}
			else
				check_for_flee(crt_ptr);
		}
		else {
			print(fd, "Your attempt to cripple %m failed.\n", crt_ptr);
			ANSI(crt_ptr->fd, CYAN);
			print(crt_ptr->fd, "%M tried to cripple you.\n", ply_ptr);
			ANSI(crt_ptr->fd, WHITE);
			broadcast_rom2(fd, crt_ptr->fd, crt_ptr->rom_num, 
				"%M tried to cripple %m but failed.", ply_ptr, crt_ptr);
		}
	}

	else {
		print(fd, "Your cripple attempt failed.\n");
		ANSI(crt_ptr->fd, CYAN);
		print(crt_ptr->fd, "%M tried to cripple you.\n", ply_ptr);
		ANSI(crt_ptr->fd, WHITE);
		broadcast_rom2(fd, crt_ptr->fd, crt_ptr->rom_num, 
			"%M tried to cripple %m but failed.", ply_ptr, crt_ptr);
	}
	return(0);

}

