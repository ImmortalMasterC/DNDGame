/*
 * COMMAND8.C:
 *
 *	Additional user routines.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include <stdlib.h>
#include <string.h>
#include "mstruct.h"
#include "mextern.h"
#include "AddFiles/time.h"

/**********************************************************************/
/*			give					      */
/**********************************************************************/
/* This function allows a player to give an item in his inventory to  */
/* another player or monster.				  	      */

int give(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;
{
	object		*obj_ptr;
	creature	*crt_ptr;
	room		*rom_ptr;
	int		fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 3) {
		print(fd, "Give what to whom?\n");
		return(0);
	}

	if(cmnd->str[1][0] == '$') {
		give_money(ply_ptr, cmnd);
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
			   cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	cmnd->str[2][0] = up(cmnd->str[2][0]);
	crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
		cmnd->str[2], cmnd->val[2]);

	if(!crt_ptr) {
		cmnd->str[2][0] = low(cmnd->str[2][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
			cmnd->str[2], cmnd->val[2]);
		if(!crt_ptr) {
			print(fd, "I don't see that person here.\n");
			return(0);
		}
	}

	if(crt_ptr == ply_ptr) {
		print(fd, "You can't give something to yourself.\n");
		return(0);
	}

	if(obj_ptr->questnum) {
		print(fd, "You can't give away a quest object.\n");
		return(0);
	}

	if(crt_ptr->type == PLAYER && (weight_ply(crt_ptr) + 
	   weight_obj(obj_ptr) > max_weight(crt_ptr))) {
		print(fd, "%s can't hold anymore.\n", crt_ptr->name);
		return(0);
	}

	del_obj_crt(obj_ptr, ply_ptr);
	add_obj_crt(obj_ptr, crt_ptr);
	savegame(ply_ptr, 0);
	savegame(crt_ptr, 0);

	print(fd, "You give %1i to %m.\n", obj_ptr, crt_ptr);
	if(F_ISSET(ply_ptr, PSHRNK) && !F_ISSET(crt_ptr, PSHRNK)) {
		print(fd, "It returns to normal size as you hand it to %s.\n",
			F_ISSET(crt_ptr, PMALES) ? "him":"her");
		print(crt_ptr->fd, "A mouse-sized %M gave %1i to you.\n", 
			ply_ptr, obj_ptr);
	}
	else {
		ANSI(crt_ptr->fd, YELLOW);
		print(crt_ptr->fd, "%M gave %1i to you.\n", ply_ptr, obj_ptr);
		ANSI(crt_ptr->fd, WHITE);
	}

	broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, "%M gave %1i to %m.",
		ply_ptr, obj_ptr, crt_ptr);

	return(0);

}

/***********************************************************************/
/*		give_money					       */
/***********************************************************************/
/* This function allows a player to give gold to another player or to  */
/* a monster.  To give gold, it must be preceded by a dollar sign.     */

void give_money(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;
{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		amt;
	int		fd;

	rom_ptr = ply_ptr->parent_rom;
	fd = ply_ptr->fd;

	amt = atol(&cmnd->str[1][1]);
	if(amt < 1) {
		print(fd, "Get real, cheater.  I should tell Styx.\n");
		return;
	}
	if(amt > ply_ptr->gold) {
		print(fd, "You don't have that much gold.\n");
		return;
	}

	cmnd->str[2][0] = up(cmnd->str[2][0]);
	crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			   cmnd->str[2], cmnd->val[2]);

	if(!crt_ptr || crt_ptr == ply_ptr) {
		cmnd->str[2][0] = low(cmnd->str[2][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
			  cmnd->str[2], cmnd->val[2]);

		if(!crt_ptr) {
			print(fd, "That person is not here.\n");
			return;
		}
	}

	if(F_ISSET(crt_ptr, MGATEK)) {
		print(fd, "%M says, \"Pay me, please.  I can't accept a gift of gold.\"\n", crt_ptr);
		return;
	}

	crt_ptr->gold += amt;
	ply_ptr->gold -= amt;
	savegame(ply_ptr, 0);
	savegame(crt_ptr, 0);

	print(fd, "Ok.  Now you have %ld gold pieces left.\n", ply_ptr->gold);
	if(F_ISSET(ply_ptr, PSHRNK) && !F_ISSET(crt_ptr, PSHRNK))
		print(fd, "The gold returns to normal size as you release it.\n");
		ANSI(crt_ptr->fd, YELLOW);
			print(crt_ptr->fd, "%M gave you %ld gold pieces.\n", ply_ptr, amt);
		ANSI(crt_ptr->fd, WHITE);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
		    "%M gave %m %ld gold pieces.", ply_ptr, crt_ptr, amt);

}

/**********************************************************************/
/*			repair					      */
/**********************************************************************/
/* This function allows a player to repair a weapon or armor if he is */
/* in a repair shoppe.  There is a chance of breakage.	      	      */

int repair(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;
{
	object	*obj_ptr;
	room	*rom_ptr;
	long	cost;
	int	fd, broke;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	F_CLR(ply_ptr, PHIDDN);

	if(cmnd->num < 2) {
		print(fd, "Repair what?\n");
		return(0);
	}

	if(!F_ISSET(rom_ptr, RREPAI)) {
		print(fd, "This is not a repair shop.\n");
		if(F_ISSET(rom_ptr, RSILVR))
			print(fd, "This shop is for renewing enchanted items.\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(F_ISSET(obj_ptr, OSILVR)) {
		print(fd, "This fine item should be renewed in a silver shop.\n");
		return(0);
	}

	if(F_ISSET(obj_ptr, ONOFIX)) {
		print(fd, "The smithy cannot repair that.\n");
		return(0);
	}

	if(obj_ptr->type > MISSILE && obj_ptr->type != ARMOR) { 
			print(fd, "He can only repair weapons and armor.\n");
			return(0);
	}

	if(obj_ptr->shotscur > MAX(3, obj_ptr->shotsmax/10)) {
		print(fd, "It's not broken yet.\n");
		return(0);
	}

	cost = obj_ptr->value/2;

	if(ply_ptr->gold < cost) {
		print(fd, "You don't have enough money.\n");
		return(0);
	}

	print(fd, "The smithy takes %ld gold pieces from you.\n", cost);
	ply_ptr->gold -= cost;

	broadcast_rom(fd, ply_ptr->rom_num, "%M has the smithy repair %1i.",
		      ply_ptr, obj_ptr);

	broke = mrand(1,100) + bonus[ply_ptr->piety];
	if((broke <= 15 && obj_ptr->shotscur < 1) ||
	   (broke <= 5 && obj_ptr->shotscur > 0)) {
		print(fd, "\"Darnitall!\" shouts the smithy, \"I broke another.  "
					"Sorry %s.\"\n", F_ISSET(ply_ptr, PMALES) ? "lad":"lass");
		broadcast_rom(fd, ply_ptr->rom_num, "Oops!  He broke it.");
		if(obj_ptr->shotscur > 0) {
			print(fd, "He gives your money back.\n");
			ply_ptr->gold += cost;
		}
		del_obj_crt(obj_ptr, ply_ptr);
		free_obj(obj_ptr);
		return(0);
	}

	if((F_ISSET(obj_ptr, OENCHA) || obj_ptr->adjustment) &&
		mrand(1, 50) > ply_ptr->piety) {
			print(fd, "\"It lost that nice sparkle,\" says the smithy.\n");
			if(obj_ptr->type == ARMOR && obj_ptr->wearflag == BODY)
				obj_ptr->armor = MAX(obj_ptr->armor - obj_ptr->adjustment*2, 0);
			else if(obj_ptr->type == ARMOR)
				obj_ptr->armor = MAX(obj_ptr->armor - obj_ptr->adjustment, 0);
			else if(obj_ptr->type <= MISSILE) {
				obj_ptr->shotsmax -= obj_ptr->adjustment * 10;
				obj_ptr->pdice = MAX(obj_ptr->pdice - obj_ptr->adjustment, 0);
			}
		obj_ptr->adjustment = 0;
		F_CLR(obj_ptr, OENCHA);
	}

	obj_ptr->shotscur = (obj_ptr->shotsmax * mrand(5, 9)) / 10;

	print(fd, "The smithy hands %i back to you, almost good as new.\n", 
		obj_ptr);
	if(F_ISSET(ply_ptr, PSHRNK)) {
		ANSI(fd, YELLOW);
		print(fd, "Your body returns to normal size as you accept %i.\n",
			obj_ptr);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		F_CLR(ply_ptr, PSHRNK);
	}
	savegame(ply_ptr, 0);
	return(0);
}

/**********************************************************************/
/*			renew					      */
/**********************************************************************/
/* This function allows a player to renew the power of an enchanted   */
/* silver weapon in a silversmith's shop.  It may break.              */

int  renew(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;
{
	object	*obj_ptr;
	room	*rom_ptr;
	long	cost;
	int	fd, broke;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Repair what?\n");
		return(0);
	}

	if(!F_ISSET(rom_ptr, RREPAI) && !F_ISSET(rom_ptr, RSILVR)) {
		print(fd, "This is not the shop of a silversmith.\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
			   cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	if(!F_ISSET(obj_ptr, OSILVR)) {
		if(obj_ptr->type == 11) {
			print(fd, "The silversmith says, \"I'm not a locksmith, and "
						"I don't repair keys.\"\n");
			return(0);
		}
		else {
			print(fd, 
				"The silversmith says, \"I'm sorry, but I cannot renew %i.\n",
					obj_ptr);
			print(fd, 
				"I only work with items containing enchanted silver.\"\n");
			print(fd, 
				"\"I recommend you take it to a blacksmith or a wizard for repair.\"\n");
			return(0);
		}
	}

	if(F_ISSET(obj_ptr, ONOFIX)) {
		print(fd, "The silversmith says it cannot be renewed.\n");
		return(0);
	}

	if(obj_ptr->shotscur > MAX(3, obj_ptr->shotsmax/10) &&
		F_ISSET(obj_ptr, OTMPEN)) {
			print(fd, "It doesn't need renewal yet.\n");
			return(0);
	}

	cost = obj_ptr->value * 2/3;

	if(ply_ptr->gold < cost) {
		print(fd, "You don't have enough money.\n");
		return(0);
	}

	print(fd, "The silversmith takes %ld gold pieces from you.\n", cost);
	ply_ptr->gold -= cost;

	broadcast_rom(fd, ply_ptr->rom_num, "%M has the silversmith repair %1i.",
		      ply_ptr, obj_ptr);

	broke = mrand(1, 100) + bonus[ply_ptr->piety];
	if((broke <= 15 && obj_ptr->shotscur < 1) ||
		(broke <= 5 && obj_ptr->shotscur > 0)) {
			print(fd, "\"Sorry,\" says the silversmith, \"I can mend it, "
						"but the enchantment may not hold.\"\n");
			print(fd, "He returns the mended %s and half your money.\n",
				obj_ptr);
			ply_ptr->gold += cost/2;
			F_CLR(obj_ptr, OTMPEN);
			if(F_ISSET(obj_ptr, OENCHA) || obj_ptr->adjustment) {
				obj_ptr->adjustment = 0;
				F_CLR(obj_ptr, OENCHA);
			}
		}
	F_SET(obj_ptr, OTMPEN);
	obj_ptr->shotscur = (obj_ptr->shotsmax * 9)/10;
	print(fd, "The silversmith hands %i back to you, almost good as new.\n", 
		obj_ptr);

	if(F_ISSET(ply_ptr, PSHRNK)) {
		ANSI(fd, YELLOW);
		print(fd, "Your body returns to normal size as you accept %i.\n", 
			obj_ptr);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		F_CLR(ply_ptr, PSHRNK);
	}
	savegame(ply_ptr, 0);
	return(0);
}

/**********************************************************************/
/*			prt_time				      */
/**********************************************************************/
/* This function outputs the time of day (realtime) to the player.    */

int prt_time(ply_ptr)
creature	*ply_ptr;
{
	char	*str;
	long	t;
	int	fd, daytime;

	fd = ply_ptr->fd;

	daytime = (int)(Time % 24L);
	print(fd, "Darbonne-time: %d o'clock %s.\n", daytime%12 == 0 ? 12 :
	      daytime%12, daytime > 11 ? "PM":"AM");

	t = time(0);
	str = ctime(&t);
	str[strlen(str) - 1] = 0;
	
	print(fd, "Real-Time: %s (PST).\n", str);

	return(0);

}

/***************************************************************************/
/*			savegame					   */        
/***************************************************************************/
/* This function saves a player's game.  Since items need to be un-readied */
/* before a player can be saved to a file, this function makes a duplicate */
/* of the player, unreadies everything on the duplicate, and then saves    */
/* the duplicate to the file.  Afterwards, the duplicate is freed from     */
/* memory.								   */

int savegame(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;
{
	creature	*dum_ptr;
	object		*obj[MAXWEAR];
	int		i, n = 0;

	dum_ptr = (creature *)malloc(sizeof(creature));
	if(!dum_ptr)
		merror("savegame", FATAL);

	*dum_ptr = *ply_ptr;

	for(i = 0; i < MAXWEAR; i++) {
		if(dum_ptr->ready[i]) {
			obj[n++] = dum_ptr->ready[i];
			add_obj_crt(dum_ptr->ready[i], dum_ptr);
			dum_ptr->ready[i] = 0;
		}
	}

	if(!dum_ptr->name[0]) return;

	if(save_ply(dum_ptr->name, dum_ptr) < 0)
		merror("ERROR - savegame", NONFATAL);

	for(i = 0; i < n; i++)
		del_obj_crt(obj[i], dum_ptr);

	free(dum_ptr);

	print(ply_ptr->fd, "Player saved.\n");

	return(0);

}



/**********************************************************************/
/*			fix 					      */
/**********************************************************************/
/* This function allows a player to fix an object if he is holding    */
/* an appropiate repair tool.  There is a chance of breakage, and     */
/* the item is never repaired to full strength.  The amount of repair */
/* will depend on the player's dexterity.                             */

int fix(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;
{
	object	*obj_ptr1;
	object	*obj_ptr2;
	room	*rom_ptr;
	int	fd, breaks, dex_bonus;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	F_CLR(ply_ptr, PHIDDN);

	if(cmnd->num < 3) {
		print(fd, "Fix something?  With what?\n");
		return(0);
	}

	if(ply_ptr->hpcur < 20) {
		print(fd, "You don't have enough hit points.\n");
		return(0);
	}


	obj_ptr1 = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr1) {
		print(fd, "You don't have that item.\n");
		return(0);
	}

	if(obj_ptr1->type == REPAIR ||
		obj_ptr1->type == CONTAINER ||
		obj_ptr1->type == SCROLL) {
			print(fd, "Some objects simply cannot be repaired.\n");
			return(0);
	}

	if(F_ISSET(obj_ptr1, OSILVR)) {
		print(fd, "This fine item should be renewed rather than repaired.\n");
		return(0);
	}

	if(F_ISSET(obj_ptr1, OCLOAK)) {
		print(fd, "The magic in %i can only be restored by a wizard.\n", obj_ptr1);
		return(0);
	}

	if(F_ISSET(obj_ptr1, ONTFIX) ||
		F_ISSET(obj_ptr1, ONOFIX)) {
		print(fd, "The item cannot be fixed.\n");
		return(0);
	}

	if(obj_ptr1->shotscur > MAX(3, obj_ptr1->shotsmax/10)) {
		print(fd, "It's not broken yet.\n");
		return(0);
	}

	obj_ptr2 = find_obj(ply_ptr, ply_ptr->first_obj,
        cmnd->str[2], cmnd->val[2]);

    if(!obj_ptr2) {
        print(fd, "You don't have one of those.\n");
        return(0);
    }
    if(obj_ptr2->type != REPAIR) {
        print(fd, "That's not a repair tool.\n");
        return(0);
    }

    if(obj_ptr2->shotscur <= 0) {
        print(fd, "Oops, %i is broken.\n", obj_ptr2);
        return(0);
    }

	broadcast_rom(fd, ply_ptr->rom_num, 
		"%M attempts to use a tool to fix %1i.", ply_ptr, obj_ptr1);

	breaks = mrand(1, 100) + bonus[ply_ptr->piety];
	if((breaks <= 25 && obj_ptr1->shotscur < 1) ||
	   (breaks <= 15 && obj_ptr1->shotscur > 0) ||
		(Ply[ply_ptr->fd].extr->luck < 35)) {
		  print(fd, "You're too clumsy and you shattered both %i and %i!\n",
                obj_ptr1, obj_ptr2);
		if(Ply[ply_ptr->fd].extr->luck < 35)
			print(fd, "Your luck is at a very low level.\n");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M was clumsy and shattered the %1i.", ply_ptr, obj_ptr1);
		del_obj_crt(obj_ptr1, ply_ptr);
		free_obj(obj_ptr1);
		del_obj_crt(obj_ptr2, ply_ptr);
		free_obj(obj_ptr2);
		return(0);
	}

	if((F_ISSET(obj_ptr1, OENCHA) || obj_ptr1->adjustment) &&
		mrand(1, 70) > ply_ptr->piety) {
			print(fd, "You pounded the enchantment out of it!\n");
			if(obj_ptr1->type == ARMOR && obj_ptr1->wearflag == BODY)
				obj_ptr1->armor = 
					MAX(obj_ptr1->armor - obj_ptr1->adjustment*2, 0);
			else if(obj_ptr1->type == ARMOR)
				obj_ptr1->armor = 
					MAX(obj_ptr1->armor - obj_ptr1->adjustment, 0);
			else if(obj_ptr1->type <= MISSILE) {
				obj_ptr1->shotsmax -= obj_ptr1->adjustment * 10;
				obj_ptr1->pdice = 
					MAX(obj_ptr1->pdice - obj_ptr1->adjustment, 0);
			}
		obj_ptr1->adjustment = 0;
		F_CLR(obj_ptr1, OENCHA);
	}

	dex_bonus = 3;
	if(ply_ptr->dexterity < 20)
		dex_bonus = 2;
	if(ply_ptr->dexterity < 15)
		dex_bonus = 1;
	if(ply_ptr->dexterity < 10)
		dex_bonus = 0;
	
	obj_ptr1->shotscur = (obj_ptr1->shotsmax * (mrand(2, 6) + dex_bonus))/10;
	obj_ptr2->shotscur--;

	if(obj_ptr1->shotscur > 0) {
		print(fd, "You repaired %i, but it still has a few knicks in it.\n", 
			obj_ptr1);
	}
	else
		print(fd, "The %s could not be fixed with %i.\n", 
			obj_ptr1, obj_ptr2);

		

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

