/*
 * COMMAND10.C:
 *
 *  Additional user routines.
 *
 *
 */

#include "mstruct.h"
#include "mextern.h"
     
/************************************************************************/
/*                              lower_prof                              */
/************************************************************************/
/* The lower_prof function adjusts a player's magical realm and weapon  */
/* proficiency after a player loses (exp) amount of experience          */

void lower_prof(ply_ptr, exp)
creature   *ply_ptr;
long        exp;

{
    long    profloss, total;
    int     n, below = 0;
             
	for(n = 0, total = 0L; n < 5; n++)
		total += ply_ptr->proficiency[n];

	for(n = 0; n < 4; n++)
		total += ply_ptr->realm[n];
 
	profloss = MIN(exp, total);
	while(profloss > 9L && below < 9) {
		below = 0;
		for(n = 0; n < 9; n++) {
			if(n < 5) {
				ply_ptr->proficiency[n] -= profloss/(9 - n);
				profloss -= profloss/(9 - n);
				if(ply_ptr->proficiency[n] < 0L) {
					below++;
					profloss -=
					ply_ptr->proficiency[n];
					ply_ptr->proficiency[n] = 0L;
				}
			}
			else {
				ply_ptr->realm[n - 5] -= profloss/(9 - n);
				profloss -= profloss/(9 - n);
				if(ply_ptr->realm[n - 5] < 0L) {
					below++;
					profloss -= ply_ptr->realm[n - 5];
					ply_ptr->realm[n - 5] = 0L;
				}
			}
		}
	}
	for(n = 1, total = 0; n < 5; n++)
	if(ply_ptr->proficiency[n] > ply_ptr->proficiency[total]) 
		total = n;
	if(ply_ptr->proficiency[total] < 1024L)
		ply_ptr->proficiency[total] = 1024L;

	return;
}

/*************************************************************************/
/*                              add_prof                                 */
/*************************************************************************/
/* The add_prof function adjusts a player's magical realm and weapon     */
/* proficiency after the player gains an unassigned amount of experience */
/* such as from a quest item or pkilling.  The exp gain is divided       */
/* between the player's 5 weapon proficiency and 4 magic realms.         */

void add_prof(ply_ptr, exp)
creature   *ply_ptr;
long        exp;

{
    int     n;
          
	if(exp) 
		exp = exp/9L;
	else 
		return;
	      
	for(n = 0; n < 5; n++) 
		ply_ptr->proficiency[n] += exp;
	for(n = 0; n < 4; n++) 
			ply_ptr->realm[n]  += exp;

	return;
}

/************************************************************************/
/*                              Pledge                                  */
/************************************************************************/
/* The pledge command allows a player to pledge allegiance to a given   */
/* monster. A pledged player may use selected items and exits, and may  */
/* kill players of the opposing kingdom for experience (regardless if   */
/*  one player is lawful.)                                              */
/* In order for a player to pledge, the player needs to be in a flagged */
/* room with a flagged monster.             							*/

int pledge(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
	object      *obj_ptr;
	creature    *crt_ptr;
	room        *rom_ptr;
	int         fd, amte, amtg;
 
	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
    
	amte = REWARD;
	amtg = REWARD * 5;
 
	if(cmnd->num < 2) {
		print(fd, "Pledge to which Prince?\n");
		return(PROMPT);
	}

	if(F_ISSET(ply_ptr, PPLDGK)) {
		print(fd, "You have already pledged.\n");
		return(PROMPT);
	}

	if(!F_ISSET(rom_ptr, RPLDGK)) {
		print(fd, "This is not the proper place to pledge your allegiance.\n");
		return(PROMPT);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);
 
	if(!crt_ptr) {
		print(fd, "You cannot pledge to %s.\n", cmnd->str[1]);
		return(PROMPT);
	}

	if(!F_ISSET(crt_ptr, MPLDGK)) {
		print(fd, "%m cannot accept a pledge.\n", crt_ptr);
		return(PROMPT);
	}

	F_CLR(ply_ptr, PHIDDN);
	F_CLR(ply_ptr, PINVIS);
	broadcast_rom(fd, ply_ptr->rom_num, "%M pledges %s allegiance to %m.", 
		ply_ptr, F_ISSET(crt_ptr, PMALES) ? "his":"her", crt_ptr);      
	print(fd, "You kneel before %m as you vow allegiance.\n", crt_ptr);
	ANSI(fd, YELLOW);
	print(fd, 
		"%M draws his sword and dubs you a valued member of his castle.\n",
		crt_ptr); 
	ANSI(fd, WHITE);
	print(fd,"The room erupts in cheers for the newest protector of %m.\n",
		crt_ptr); 
	print(fd, "You gain %d experience and %d gold!\n", REWARD, REWARD * 5); 
	ANSI(fd, YELLOW);
	print(fd, "%M teaches you the detect-invisible spell.\n", crt_ptr);
	ANSI(fd, WHITE);
	broadcast("### %M has pledged %s allegience to %m.", ply_ptr,
			F_ISSET(ply_ptr, PMALES) ? "his":"her", crt_ptr);
	ply_ptr->experience += amte;
	add_prof(ply_ptr, amte);
	ply_ptr->gold += amtg;
	ply_ptr->hpcur = ply_ptr->hpmax;
	ply_ptr->mpcur = ply_ptr->mpmax;
	S_SET(ply_ptr, SDINVI);		/*  detect-invisible spell  */

	F_SET(ply_ptr, PPLDGK);
	if(F_ISSET(crt_ptr, MKNGDM))
		F_SET(ply_ptr, PKNGDM);

return(PROMPT); 
 
}               

/************************************************************************/
/*                              rescind                                 */
/************************************************************************/
/* The rescind command allows a player to rescind allegiance to a given */
/* monster.  Once a player has rescinded his allegiance, he or she will */
/* lose all the privleges of the rescinded kingdom as well as a         */
/* specified amount of experience and gold. 							*/

int rescind(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
	object      *obj_ptr;
	creature    *crt_ptr;
	room        *rom_ptr;
	int         fd, amte, amtg;
 
	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	amte = REWARD * 2;
	amtg = REWARD * 2;
 
	if(cmnd->num < 2) {
		print(fd, "Rescind to whom?\n");
		return(PROMPT);
	}

	if(!F_ISSET(ply_ptr, PPLDGK)) {
		print(fd, "You are not presently pledged.\n");
		return(PROMPT);
	}

	if(!F_ISSET(rom_ptr, RRSCND)) {
		print(fd, "You cannot rescind your allegiance here.\n");
		return(PROMPT);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);
 
	if(!crt_ptr) {
		print(fd, "You cannot rescind to %s.\n", cmnd->str[1]);
		return(PROMPT);
	}

	if(BOOL(F_ISSET(crt_ptr, MKNGDM)) != BOOL(F_ISSET(ply_ptr, PKNGDM))) {
		print(fd, "SPY!!  You are in the wrong castle to rescind!\n");
		return(PROMPT);
	}

	if(ply_ptr->gold < amtg) {
		print(fd, "You do not have sufficient gold to rescind.\n");
		print(fd, 
			"Perhaps if you spend some time in prison your relatives or\n");
		print(fd, "friends can bring enough gold.  *spit*\n");
		return (PROMPT);
	}

	if(!F_ISSET(crt_ptr, MRSCND)) {
		print(fd, "You do not owe allegiance to %m.\n", crt_ptr);
		return(PROMPT);
	}

	broadcast_rom(fd, ply_ptr->rom_num, "%M rescinds %s allegiance to %m.", 
		ply_ptr, F_ISSET(crt_ptr, PMALES) ? "his":"her", crt_ptr);      
	ANSI(fd, YELLOW);
	print(fd,
		"%M scourns you and strips you of all your rights and privileges!\n", 
			crt_ptr);
	ANSI(fd, WHITE);
	print(fd,
		"The room fills with hisses as you are ostracized from %m's castle.\n", 
			crt_ptr); 
	amte = MIN(amte, ply_ptr->experience);
	print(fd, "You lose %ld experience and %d gold!\n", amte, amtg); 
	ANSI(fd, YELLOW);
	print(fd, "%M forces you to forget the detect-invisible spell.\n", 
		crt_ptr);
	ANSI(fd, WHITE);
	broadcast("### %M has renounced his allegience to Prince %m.", ply_ptr, 
			F_ISSET(ply_ptr, PMALES) ? "his":"her", crt_ptr);

	lower_prof(ply_ptr, amte);
	ply_ptr->experience -= amte;
	ply_ptr->gold -= amtg;
	ply_ptr->hpcur = ply_ptr->hpmax/3;
	ply_ptr->mpcur = 0;

	F_CLR(ply_ptr, PHIDDN);
	F_CLR(ply_ptr, PPLDGK);
	F_CLR(ply_ptr, PKNGDM);
	S_CLR(ply_ptr, 9);	/*  detect-invisible spell  */
 
    return(PROMPT); 
}               

/***********************************************************************/
/*      		        		lose_all				               */
/***********************************************************************/
/* lose_all causes the given player to lose all his or her possessions */
/* including any equipted items.                                       */

void lose_all(ply_ptr)
creature    *ply_ptr;

{
    object  *obj_ptr;
    room    *rom_ptr;
    otag    *op;
    int 	fd, i;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

/* remove all equipted items */
    for(i = 0; i < MAXWEAR; i++) {
        if(ply_ptr->ready[i] && !F_ISSET(ply_ptr->ready[i], OCURSE)) {
            F_CLR(ply_ptr->ready[i], OWEARS);
            add_obj_crt(ply_ptr->ready[i], ply_ptr);
            ply_ptr->ready[i] = 0;
        }
    }

    compute_ac(ply_ptr);
    compute_thaco(ply_ptr);
	compute_agility(ply_ptr);

/* remove and delete all possessions */
    op = ply_ptr->first_obj;
    while(op) {
		obj_ptr = op->obj;
		op = op->next_tag;
		del_obj_crt(obj_ptr, ply_ptr);
		free_obj(obj_ptr);
    }
}

/**********************************************************************/
/*                      dissolve_item                                 */
/**********************************************************************/
/* dissolve_item will randomly select one equipted (including held or */
/* wield) items on the given player and then delete it. The player    */
/* receives a message that the item was destroyed as well as who is   */
/* responsible for the deed.										  */

void dissolve_item (ply_ptr,crt_ptr)
creature    *ply_ptr;
creature    *crt_ptr;

{
    int     item_num;
    char    checklist[MAXWEAR];
    int     fd, numwear = 0, i;
 
	fd = ply_ptr->fd;

	for(i = 0; i < MAXWEAR; i++) {
		checklist[i] = 0;
		if(ply_ptr->ready[i])
		checklist[numwear++] = i + 1;
	}
 
	if(!numwear) 
		item_num = 0;
	else {
		i = mrand(0, numwear - 1);
		item_num = (int) checklist[i];
	}
            
	if(item_num) {
		broadcast_rom(ply_ptr->fd, ply_ptr->rom_num, "%M dissolves %m's %s.",
			crt_ptr, ply_ptr, ply_ptr->ready[item_num - 1]->name);
		ANSI(fd, YELLOW);
		print(fd, "%M dissolves your %s.\n", 
			crt_ptr, ply_ptr->ready[item_num - 1]->name); 
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		free_obj(ply_ptr->ready[item_num - 1]);
		ply_ptr->ready[item_num - 1] = 0;
		compute_ac(ply_ptr);
		compute_agility(ply_ptr);
	}
	return;
}            

/**********************************************************************/
/*							train								      */
/**********************************************************************/
/* This function allows a player to train if he is in the correct     */
/* training location and has enough gold and experience.  If so, the  */
/* character goes up a level.					      				  */

int train(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room	*rom_ptr;
	long	goldneeded, expneeded, lv;
	int		fd, i, fail = 0, bit[4];

	fd = ply_ptr->fd;
	
	if(F_ISSET(ply_ptr, PBLIND)) {
		ANSI(fd, RED);
		print(fd, "You can't... You're blind!\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		return(0);
	}

	rom_ptr = ply_ptr->parent_rom;

	bit[0] = ply_ptr->class & 1;
	bit[1] = ply_ptr->class & 2;
	bit[2] = ply_ptr->class & 4;
	bit[3] = ply_ptr->class & 8;

	for(i = 0; i < 4; i++) {
		if(!((bit[i] && F_ISSET(rom_ptr, RTRAIN + 3 - i)) ||
			(!bit[i] && !F_ISSET(rom_ptr, RTRAIN + 3 - i))))
				fail = 1;
	}

	if(fail) {
		print(fd, "This is not your training location.\n");
		return(0);
	}

	if(ply_ptr->level == MAXALVL) {
		print(fd, "Congratulations!\n");
		print(fd, "You have reached the highest level of success.\n");
		print(fd, "It is unfortunate, but you may not advance any further.\n");
	}	

	if(ply_ptr->level < MAXALVL)
		expneeded = needed_exp[ply_ptr->level - 1];
	else
        expneeded = (long)((needed_exp[MAXALVL - 1] * ply_ptr->level));   

	goldneeded = expneeded/2L;

	if(expneeded > ply_ptr->experience) {
		print(fd, "You need %ld more experience.\n",
		      expneeded - ply_ptr->experience);
		return(0);
	}

	if(ply_ptr->experience > needed_exp[ply_ptr->level]) {
		print(fd, "You have far more experience than you need for this\n");
		print(fd, "training location.  Send mudmail to Styx for help.\n");
        return(0);
    }

	if(((ply_ptr->level < 7) || (ply_ptr->class == BARBARIAN)) 
		&& (goldneeded > ply_ptr->gold)) {
			print(fd, "You don't have enough gold.\n");
			print(fd, "You need %ld gold to train.\n", goldneeded);
			return(0);
	}

	if((ply_ptr->level < 7) || (ply_ptr->class == BARBARIAN)) {
		ply_ptr->gold -= goldneeded;
		print(fd, "Your training fee is %ld gold, leaving you with %ld.\n", 
			goldneeded, ply_ptr->gold);
		up_level(ply_ptr);
		if(ply_ptr->level == 6 && ply_ptr->class != BARBARIAN)
			print(fd, "Next time, for level seven training and above, your "
				"training fee must be in the bank.\n");

	}
	else {
		if(ply_ptr->level > 9 && !F_ISSET(ply_ptr, PPLDGK)) {
			print(fd, "You must be pledged to either Prince Rutger or to "
				"Prince Ragnar if you wish to advance further.\n");
			return(0);
		}

		if(ply_ptr->class != BARBARIAN) {
			if(goldneeded > ply_ptr->bank_bal) {
				print(fd, "You don't have enough gold in the bank.\n");
				print(fd, "You need a bank balance of %ld gold to advance.\n", 
					goldneeded);
				return(0);
			}
			else {
				if(F_ISSET(ply_ptr, PLOANU)) {
					print(fd, "You must repay your bank loan first.\n");
					return(0);
				}
				if(!F_ISSET(ply_ptr, PLOANB))
					ply_ptr->bank_bal -= goldneeded;
				else 
					F_SET(ply_ptr, PLOANU);

				print(fd, "Your training fee of %ld gold has been taken "
					"from your bank account.\n", goldneeded);
				up_level(ply_ptr);
			}
		}
	}
		

	broadcast("### %s just made a level!", ply_ptr->name);
	ANSI(fd, NORMAL); 
	ANSI(fd, GREENBG); 
	ANSI(fd, BOLD); 
	ANSI(fd, YELLOW); 
	print(fd, "\nCongratulations, you made a level!");
	ANSI(fd, BLUEBG); 
	ANSI(fd, BOLD); 
	ANSI(fd, WHITE); 
	print(fd, "\n\n");

	return(0);
}
