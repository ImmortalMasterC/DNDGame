/*
 * COMMAND9.C:
 *
 *	Additional user routines.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"

/**********************************************************************/
/*									bribe						      */
/**********************************************************************/
/* This function allows players to bribe monsters.  If they give the  */
/* monster enough money, it will leave the room.  If not, the monster */
/* keeps the money and stays.	      				      */
/* If the monster is a passive guard for an exit, it will open the    */
/* exit.                                                              */

int bribe(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	exit_		*ext_ptr;
	xtag		*xp;
	long		amount;
	int			fd, found = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Bribe whom?\n");
		return(0);
	}

	if(cmnd->num < 3 || cmnd->str[2][0] != '$') {
		print(fd, "Syntax: bribe <monster> $<amount>\n");
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[1],
	   cmnd->val[1]);

	if(!crt_ptr) {
		print(fd, "That is not here.\n");
		return(0);
	}

	if(F_ISSET(crt_ptr, MNOBRB) || F_ISSET(crt_ptr, MTIPOK)) {
		print(fd, "%M ignores your bribery attempt.\n", crt_ptr);
		return(0);
	}

	amount = atol(&cmnd->str[2][1]);
	if(amount < 1 || amount > ply_ptr->gold) {
		print(fd, "Get real, Charly.\n");
		return(0);
	}
	if(F_ISSET(crt_ptr, MPBRIB)) {
		if(amount < (crt_ptr->gold)) {
			print(fd, "%M takes the gold, but sneers at the amount.\n", 
				crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M tried to bribe %m.", ply_ptr, crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M sneers at %m and mutters, \"cheapskate.\"\n", 
					crt_ptr, ply_ptr);
			ply_ptr->gold -= amount;
			crt_ptr->gold += amount;
		}
		else {
			print(fd, "%M takes your money, laughs at you, and leaves.\n", 
				crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M bribed %m.",
				ply_ptr, crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M grins broadly, gives everyone the finger and leaves.\n",
					crt_ptr);
			ply_ptr->gold -= amount;
			del_crt_rom(crt_ptr, rom_ptr);
			del_active(crt_ptr);
			free_crt(crt_ptr);
		}
		return(0);
	}

	if(!F_ISSET(crt_ptr, MPGUAR)) {
		if(amount < (crt_ptr->level * crt_ptr->level * 49L) || 
			F_ISSET(crt_ptr, MPERMT)) {
				print(fd, "%M takes the gold, but stays in your face.\n", 
					crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M tried to bribe %m.", ply_ptr, crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M sneers at %m and mutters, \"cheapskate.\"\n", 
						crt_ptr, ply_ptr);
				ply_ptr->gold -= amount;
				crt_ptr->gold += amount;
		}
		else {
			print(fd, "%M takes your money and leaves.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M bribed %m.",
				ply_ptr, crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M grins broadly, gives everyone the finger and leaves.\n",
					crt_ptr);
			ply_ptr->gold -= amount;
			del_crt_rom(crt_ptr, rom_ptr);
			del_active(crt_ptr);
			free_crt(crt_ptr);
		}
	}

/* -------------------------------*/
/*  Monster will open door        */

	else {
		xp = rom_ptr->first_ext;
		while(xp) {
			if(F_ISSET(xp->ext, XMONEY)) {
				found = 1;
				break;
			}
			xp = xp->next_tag;
		}
		if(!found) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, 
					"%M doesn't seem to be able to find the key.  Sorry.\n",
						crt_ptr);
			}
			else
				print(fd, 
					"It doesn't seem to be able to find the key.  Sorry.\n");

			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "%s reluctantly returns your gold to you.\n", 
					F_ISSET(crt_ptr, MMALES) ? "He":"She");
			}
			else
				print(fd, "It reluctantly returns your gold to you.\n"); 
			return(0);
		}

		if(amount < (xp->ext->ltime.misc)) {
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, "%M shakes %s head, \"It takes more than that,\n", 
					crt_ptr, F_ISSET(crt_ptr, MMALES) ? "his":"her");
			}
			else
				print(fd, 
					"It shakes it's head, \"It takes more than that,\n"); 
			print(fd, "but I'll just keep this for my trouble.\"\n", 
				crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M tried to bribe %m.", ply_ptr, crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M sneers at %m and mutters, \"cheapskate.\"\n", 
					crt_ptr, ply_ptr);
			ply_ptr->gold -= amount;
			crt_ptr->gold += amount;
		}
		else {
			ply_ptr->gold -= amount;
			crt_ptr->gold += amount;
			print(fd, 
				"%M winks and says, \"Thanks, let me open the %s for you.\"\n",
					crt_ptr, xp->ext->name);
			print(fd, "The %s swings open.\n", xp->ext->name);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M accepts a bribe and opens the %s for %m.", 
					crt_ptr, xp->ext->name, ply_ptr);
			F_CLR(xp->ext, XLOCKD);
			F_CLR(xp->ext, XCLOSD);
		}
	}

	return(0);

}

/**********************************************************************/
/*								haste							      */
/**********************************************************************/
/* This function allows rangers to hasten themselves every 10 minutes */
/* or so, giving themselves 5 extra dexterity points.  The haste will */
/* last for at least 4 minutes.				      					  */

int haste(ply_ptr, cmnd)
creature 	*ply_ptr;
cmd			*cmnd;
{
	long	i, t;
	int		chance, fd, agility;

	fd = ply_ptr->fd;

	if(ply_ptr->class != RANGER && ply_ptr->class < CARETAKER) {
		print(fd, "Only rangers may use haste.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PHASTE)) {
		print(fd, "You're already hastened.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PBERSK)) {
		print(fd, "You can't be a berserker and hasten also.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PSTRNG)) {
		print(fd, "You would explode while under a super-strengh spell!\n");
		return(0);
	}


	i = ply_ptr->lasttime[LT_HASTE].ltime;
	t = time(0);

	if(t - i < 600L) {
		print(fd, "Please wait %d:%02d more minutes.\n", 
		      (600L - t + i)/60L, (600L - t + i)%60L);
		return(0);
	}

	chance = MIN(85, ply_ptr->level * 20 + bonus[ply_ptr->dexterity]);

	if(mrand(1, 100) <= chance) {
		print(fd, "You feel yourself moving faster.\n");
		broadcast_rom(fd, ply_ptr->rom_num, "%M hastens.", ply_ptr);
		F_SET(ply_ptr, PHASTE);
		ply_ptr->dexterity += 5;
		ply_ptr->lasttime[LT_HASTE].ltime = t;
		ply_ptr->lasttime[LT_HASTE].interval = 240L + 60L * (ply_ptr->level/5);
		agility = compute_agility(ply_ptr);
	}
	else {
		print(fd, "Your attempt to hasten failed.\n");
		broadcast_rom(fd, ply_ptr->rom_num, "%M tried to hasten.", ply_ptr);
		ply_ptr->lasttime[LT_HASTE].ltime = t - 590L;
	}

	return(0);

}

/**********************************************************************/
/*								pray							      */
/**********************************************************************/
/* This function allows clerics and paladins to pray every 10 minutes */
/* to increase their piety for a duration of 5 minutes.	 Evil clerics */
/* and paladins cannot pray.  Monks may pray to have their hands	  */
/* blessed so that they may kill monsters that can only be killed by  */
/* an enchanted weapon.                                               */

int pray(ply_ptr, cmnd)
creature 	*ply_ptr;
cmd			*cmnd;

{
	long	i, t;
	int		aura, chance, fd;

	fd = ply_ptr->fd;

	if(ply_ptr->class != CLERIC && 
		ply_ptr->class != PALADIN && 
		ply_ptr->class != MONK && 
		ply_ptr->class != DM && 
		ply_ptr->class != CARETAKER) {
			print(fd, "Only clerics, paladins and monks may pray.\n");
			return(0);
	}


	if(F_ISSET(ply_ptr, PPRAYD)) {
		print(fd, "You've already prayed.\n");
		return(0);
	}

	aura = ply_ptr->alignment;
	if((aura < -50) && ply_ptr->class != MONK) { 
		print(fd, "Alith will not listen to the prayers of an evil one.\n");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M is evil and Alith will not answer %s prayers.", ply_ptr,
				(F_ISSET(ply_ptr, PMALES)) ? "his" : "her");
		return(0);
	}

	i = ply_ptr->lasttime[LT_PRAYD].ltime;
	t = time(0);

	if((t - i) < 600L) {
		print(fd, "Please wait %d:%02d more minutes.\n", 
			(600L - t + i)/60L, (600L - t + i)%60L);
		return(0);
	}

	if((ply_ptr->class == CLERIC || ply_ptr->class == PALADIN) || 
		(ply_ptr->class >= CARETAKER)) {
			chance = MIN(85, ply_ptr->level * 20 + bonus[ply_ptr->piety]);
			if(mrand(1, 100) <= chance) {
				print(fd, "You feel extremely pious.\n");
				broadcast_rom(fd, ply_ptr->rom_num, "%M prays to Alith.", 
					ply_ptr);
				F_SET(ply_ptr, PPRAYD);
				if(ply_ptr->class == CLERIC)
					ply_ptr->piety += 6;
				else
					ply_ptr->piety += 5;
				ply_ptr->lasttime[LT_PRAYD].ltime = t;
				ply_ptr->lasttime[LT_PRAYD].interval = 300L;
			}
			else {
				print(fd, "Alith did not hear your prayers.\n");
				broadcast_rom(fd, ply_ptr->rom_num, "%M prays to Alith.", 
					ply_ptr);
				ply_ptr->lasttime[LT_PRAYD].ltime = t - 590L;
			}
	}

	if(ply_ptr->class == MONK) { 
		if(aura < 50 && aura > -50) {
			print(fd, "You are neutral so both Alith and Zeth ignore you.\n");
			return(0);
		}
		chance = 20;
		if(aura < -600) 
			chance = 100;
		if(aura < -400) 
			chance = 80;
		if(aura < -200) 
			chance = 60;
		if(aura > 600) 
			chance = 100;
		if(aura > 400) 
			chance = 80;
		if(aura > 200) 
			chance = 60;
        if(mrand(1, 100) <= chance) {
                print(fd, "Your hands have been sanctified by %m.\n",
					(aura < 0) ? "Zeth":"Alith");
				if(aura < 0)
                	broadcast_rom(fd, ply_ptr->rom_num, "%M prays to Zeth.",
                    	ply_ptr);
				else
                	broadcast_rom(fd, ply_ptr->rom_num, "%M prays to Alith.",
                    	ply_ptr);
                F_SET(ply_ptr, PPRAYD);
                ply_ptr->piety += 5;
                ply_ptr->lasttime[LT_PRAYD].ltime = t;
                ply_ptr->lasttime[LT_PRAYD].interval = 300L;
		}
		else {
			if(aura > 50)
               	print(fd, "Alith did not hear your prayers.\n");
			if(aura < -50)
               	print(fd, "Zeth did not hear your prayers.\n");
			broadcast_rom(fd, ply_ptr->rom_num, "%M prays to a holy Goat.",
				ply_ptr);
			ply_ptr->lasttime[LT_PRAYD].ltime = t - 590L;
		}
	}
	return(0);
}

/***********************************************************************/
/*								prepare							       */
/***********************************************************************/
/* This function allows players to prepare themselves for traps that   */
/* might be in the next room they enter.  This way, they can hope to   */
/* avoid a trap that they already know is there.		     		   */

int prepare(ply_ptr, cmnd)
creature 	*ply_ptr;
cmd			*cmnd;

{
	long	i, t;
	int		fd;

	fd = ply_ptr->fd;

	if(F_ISSET(ply_ptr, PPREPA)) {
		print(fd, "You've already prepared and are at high alert.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		F_CLR(ply_ptr, PPREPA);
		print(fd, "You don't see well enough to prepare for traps.\n");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M blindly tried to prepare for traps, but failed.", ply_ptr);
		return(0);
	}

	t = time(0);
	i = LT(ply_ptr, LT_PREPA);

	if(t < i) {
		please_wait(fd, i - t);
		return(0);
	}

	ply_ptr->lasttime[LT_PREPA].ltime = t;
	ply_ptr->lasttime[LT_PREPA].interval = ply_ptr->class == DM ? 0:15;
	print(fd, "You prepare yourself for traps.\n");
	broadcast_rom(fd, ply_ptr->rom_num, "%M prepares for traps.", ply_ptr);
	F_SET(ply_ptr, PPREPA);

	return(0);
}

/************************************************************************/
/*								use										*/
/************************************************************************/
/* This function allows a player to use an item without specifying the	*/
/* special command for its type.  Use determines which type of item		*/
/* it is, and calls the appropriate functions.							*/

int use(ply_ptr, cmnd)
creature 	*ply_ptr;
cmd			*cmnd;

{
	object	*obj_ptr;
	object	*obj_ptr2;
	room	*rom_ptr;
	int		fd, n, rtn, onfloor = 0;
	int		breaks = 0, dex_bonus = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Use what?\n");
		return(0);
	}

	if(!strcmp(cmnd->str[1], "all"))
		return(wear(ply_ptr, cmnd));

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj, cmnd->str[1],
			cmnd->val[1]);
		if(!obj_ptr || !F_ISSET(obj_ptr, OUSEFL)) {
			print(fd, "Use what?\n");
			return(0);
		}
		else if(F_ISSET(obj_ptr, OUSEFL)) {
			onfloor = 1;
			cmnd->num = 2;
		}
	}

/***************-------------******************--------------***********/
	obj_ptr2 = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[2], cmnd->val[2]);

	if(obj_ptr->type == REPAIR && obj_ptr2) {
		if(ply_ptr->hpcur < 20) {
			print(fd, "You don't have enough hit points.\n");
			return(0);
		}

		if(obj_ptr2->type == REPAIR ||
        	obj_ptr2->type == CONTAINER ||
        	obj_ptr2->type == SCROLL) {
            	print(fd, "Some objects simply cannot be repaired.\n");
            	return(0);
   	 	}

    	if(F_ISSET(obj_ptr2, OSILVR)) {
        	print(fd, 
				"This fine item should be renewed rather than repaired.\n");
        	return(0);
    	}

    	if(F_ISSET(obj_ptr2, ONTFIX) ||
        	F_ISSET(obj_ptr2, ONOFIX)) {
        	print(fd, "The item cannot be fixed.\n");
        	return(0);
    	}

		if(obj_ptr2->shotscur > MAX(3, obj_ptr2->shotsmax/10)) {
			print(fd, "It's not broken yet.\n");
			return(0);
		}

		if(obj_ptr->shotscur <= 0) {
        	print(fd, "Oops, %i is broken.\n", obj_ptr);
        	return(0);
    	}

    	broadcast_rom(fd, ply_ptr->rom_num,
        	"%M attempts to use a tool to fix %1i.", ply_ptr, obj_ptr2);

    	breaks = mrand(1, 100) + bonus[ply_ptr->piety];
    	if((breaks <= 25 && obj_ptr2->shotscur < 1) ||
       		(breaks <= 15 && obj_ptr2->shotscur > 0) ||
			(Ply[ply_ptr->fd].extr->luck < 35)) {
        	print(fd, "You're too clumsy and you shattered both %i and %i!\n",
				obj_ptr, obj_ptr2);
			if(Ply[ply_ptr->fd].extr->luck < 35)
            	print(fd, "Your luck is at a very low level.\n");
        	broadcast_rom(fd, ply_ptr->rom_num,
            	"%M was clumsy and shattered the %1i.", ply_ptr, obj_ptr2);
        	del_obj_crt(obj_ptr2, ply_ptr);
        	free_obj(obj_ptr2);
        	del_obj_crt(obj_ptr, ply_ptr);
        	free_obj(obj_ptr);
        	return(0);
    	}
		if((F_ISSET(obj_ptr2, OENCHA) || obj_ptr2->adjustment) &&
        	mrand(1, 70) > ply_ptr->piety) {
            	print(fd, "It's fixed, but the enchantment is gone!\n");
            	if(obj_ptr2->type == ARMOR && obj_ptr2->wearflag == BODY)
                	obj_ptr2->armor =
                    	MAX(obj_ptr2->armor - obj_ptr2->adjustment*2, 0);
            	else if(obj_ptr2->type == ARMOR)
                	obj_ptr2->armor =
                    	MAX(obj_ptr2->armor - obj_ptr2->adjustment, 0);
            	else if(obj_ptr2->type <= MISSILE) {
                	obj_ptr2->shotsmax -= obj_ptr2->adjustment * 10;
                	obj_ptr2->pdice =
                    	MAX(obj_ptr2->pdice - obj_ptr2->adjustment, 0);
           		}
        	obj_ptr2->adjustment = 0;
        	F_CLR(obj_ptr2, OENCHA);
    	}

		dex_bonus = 3;
    	if(ply_ptr->dexterity < 20)
        	dex_bonus = 2;
    	if(ply_ptr->dexterity < 15)
        	dex_bonus = 1;
    	if(ply_ptr->dexterity < 10)
        	dex_bonus = 0;

    	obj_ptr2->shotscur = (obj_ptr2->shotsmax * 
			(mrand(2, 6) + dex_bonus))/10;
    	obj_ptr->shotscur--;

    	if(obj_ptr2->shotscur > 0) {
        	print(fd, 
				"You repaired %i, but it still has a few knicks in it.\n",
            		obj_ptr2);
    	}
    	else
        	print(fd, "The %s could not be fixed with %i.\n",
            	obj_ptr2, obj_ptr);

	 	if(F_ISSET(ply_ptr, PSHRNK)) {
        	ANSI(fd, YELLOW);
        	print(fd, "Your body returns to normal size.");
        	ANSI(fd, BOLD);
        	ANSI(fd, WHITE);
        	F_CLR(ply_ptr, PSHRNK);
    	}

    	ply_ptr->hpcur -= 20;
    	print(fd, "The repair process saps your strength.\n");

    	savegame(ply_ptr, 0);
    	return(0);
	}
	else if(obj_ptr->type == REPAIR && !obj_ptr2) {
		print(fd, "What do you want to fix?\n");
		return(0);
	}
/***************-------------******************--------------***********/

	F_CLR(ply_ptr, PHIDDN);

	if(F_ISSET(obj_ptr, OPLDGK) && 
		(BOOL(F_ISSET(obj_ptr, OKNGDM)) != BOOL(F_ISSET(ply_ptr, PKNGDM)))) {
       	print(fd, "Your pledge prevents you from using %i.\n", obj_ptr);
       	print(fd, "It has the insignia of Prince %s!\n",
			F_ISSET(ply_ptr, PKNGDM) ? "Ragnar":"Rutger");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M tried to use a %s belonging to the kingdom of Prince %s.", 
				ply_ptr, obj_ptr, F_ISSET(ply_ptr, PKNGDM) ? 
					"Ragnar":"Rutger");
       	return(0);
   	}              

	switch(obj_ptr->type) {
		case SHARP:
		case THRUST:
		case BLUNT:
		case POLE:
		case MISSILE:
			rtn = ready(ply_ptr, cmnd); break;
		case ARMOR:
			rtn = wear(ply_ptr, cmnd); break;
		case POTION:
			rtn = drink(ply_ptr, cmnd); break;
		case SCROLL:
			rtn = readscroll(ply_ptr, cmnd); break;
		case WAND:
			if(onfloor) {
				cmnd->num = 2;
				if(!F_ISSET(obj_ptr, ONUSED))
					obj_ptr->shotscur -= 1; 
				rtn = zap_obj(ply_ptr, obj_ptr, cmnd);
			}
			else
				rtn = zap(ply_ptr, cmnd); break;
		case KEY:
			rtn = unlock(ply_ptr, cmnd, 1); break;
		case LIGHTSOURCE:
			rtn = hold(ply_ptr, cmnd); break;
		default:
			print(fd, "How does one use that?\n");
			rtn = 0;
	}

	return(rtn);
}

/**********************************************************************/
/*								plead							      */
/**********************************************************************/
/* This function allows players to plead for mercy.  If the plea is   */ 
/* granted the monster will stop attacking and leave the room.        */
/* It costs the player about one percent of their experience points.  */
/**********************************************************************/

int plead(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int			fd, chance, loss;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Plead with whom?\n");
		return(0);
	}

		

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[1],
	   cmnd->val[1]);

	if(!crt_ptr) {
		print(fd, "That creature is not here.\n");
		return(0);
	}

	chance = 70 + (crt_ptr->level - ply_ptr->level) * 3;

	if(F_ISSET(crt_ptr, MNPLEA) || 
	   F_ISSET(crt_ptr, MGAGGR) || 
	   F_ISSET(crt_ptr, MEAGGR) || 
	   	 chance < mrand(1, 100) ||
		 ply_ptr->level < 3) {
			print(fd, "%M ignores your plea for mercy.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M ignored %m's plea for mercy.", crt_ptr, ply_ptr);
			return(0);
	}

	if(is_enm_crt(ply_ptr, crt_ptr)) {
		print(fd, "%M grants you mercy and turns away in contempt.\n", crt_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M granted mercy to %m.", crt_ptr, ply_ptr);
		if(ply_ptr->level < 10)
			loss = ply_ptr->experience * .02;
		else
			loss = ply_ptr->experience * .01;
		ply_ptr->experience -= loss;
		print(fd, "You lost %d points in experience.\n", loss);
		del_enm_crt(ply_ptr->name, crt_ptr);
		print(fd, "Alith casts a full restore on %m for the act of mercy.\n", 
			crt_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"Alith restores %m fully for the act of goodness.\n.", 
				crt_ptr, ply_ptr);
		crt_ptr->hpcur = crt_ptr->hpmax;
		crt_ptr->mpcur = crt_ptr->mpmax;
		F_CLR(crt_ptr, MAGGRE);
		return(0);
	}
	else {
		print(fd, "%M is not attacking you.\n", crt_ptr);
		return(0);
	}

}


/**********************************************************************/
/*								berserk							      */
/**********************************************************************/
/* This function allows barbarians to go berserk every 10 minutes     */
/* or so, giving themselves 5 extra dexterity points and increased    */
/* +3 strength.			      					  				      */


int berserk(ply_ptr, cmnd)
creature 	*ply_ptr;
cmd			*cmnd;
{
	long	i, t; 
	int		chance, fd, agility;

	fd = ply_ptr->fd;

	if(ply_ptr->class != BARBARIAN && ply_ptr->class < CARETAKER) {
		print(fd, "Only barbarians are crazy enough to go berserk.\n");
		return(0);
	}

	if(ply_ptr->level < 7) {
  		print(fd, "Grunts and Tribesmen are too dumb to go berserk.\n");
        return(0);
    }

	if(F_ISSET(ply_ptr, PBERSK)) {
		print(fd, "You're already going crazy!\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PSTRNG)) {
		print(fd, "You can't go berserk while under the super-strength spell!\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PHASTE)) {
		print(fd, "You can't use berserk if you are already using haste.\n");
		return(0);
	}


	i = ply_ptr->lasttime[LT_HASTE].ltime;
	t = time(0);

	if(t - i < 600L) {
		print(fd, "Not for another %d:%02d minutes.\n", 
		      (600L - t + i)/60L, (600L - t + i)%60L);
		return(0);
	}

	chance = MIN(65, ply_ptr->level * 20 + bonus[ply_ptr->dexterity]);

	if(mrand(1, 100) <= chance) {
		print(fd, "You feel yourself going berserk.\n");
		broadcast_rom(fd, ply_ptr->rom_num, "%M goes berserk.", ply_ptr);
		F_SET(ply_ptr, PBERSK);
		ply_ptr->dexterity += 5;
		ply_ptr->strength += 3;
		ply_ptr->lasttime[LT_HASTE].ltime = t;
		ply_ptr->lasttime[LT_HASTE].interval = 240L + 60L * (ply_ptr->level/5);
		agility = compute_agility(ply_ptr);
	}
	else {
		print(fd, "You're overwhelmed and cannot go berserk yet.\n");
		broadcast_rom(fd, ply_ptr->rom_num, "%M tried to go berserk and failed.", ply_ptr);
		ply_ptr->lasttime[LT_HASTE].ltime = t - 590L;
	}

	return(0);

}

