/*
 *      MAGIC9.C:
 *
 *      Additional spell-casting routines.
 *      Recharge Wands: 1996 Brooke Paul
 *      All others: 1996 Roy Wilson
 */

#include "mstruct.h"
#include "mextern.h"
#include <stdlib.h>

/****************************************************************************/
/*							recharge wand									*/
/****************************************************************************/
/*  This allows an Alchemist to recharge a wand provided s/he knows the		*/
/*  spell of the wand, is holding it and has enough gold.  Requires no MP.	*/

int recharge_wand(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	ctag	*cp;
	otag	*op; 
	object  *obj_ptr;
	int		fd, how = CAST, cost;

	fd = ply_ptr->fd;

	if(ply_ptr->class < CARETAKER && ply_ptr->class != ALCHEMIST) {
		print(fd, "Only an alchemist can do that.\n");
		return(0);
	}

	if(ply_ptr->level < 6 && ply_ptr->class < CARETAKER) {
		print(fd, "You are not experienced enough to do that yet.\n");
		return(0);
	}

	if(cmnd->num < 2) {
		print(fd, "Recharge what?\n");
		return(0);
	}

	if(!dec_daily(&ply_ptr->daily[DL_RCHRG]) && ply_ptr->class < CARETAKER) {
		print(fd, "You are exhausted from recharging wands for today.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "How do you do that?  You're blind.\n");
		return(0);
	}

	if(!ply_ptr->ready[HELD - 1]) {
		print(fd, "You can only recharge a wand if you are holding it.\n");
		return(0);
	}

	obj_ptr = ply_ptr->ready[HELD - 1];

	if(ply_ptr->ready[HELD - 1]->type != WAND) {
		print(fd, 
			"You are holding something, but it's not a wand.\n");
		return(0);
	}

	if(obj_ptr->shotscur) {
		if(obj_ptr->shotscur == 1)
			print(fd, "%I still has one magic spell left in it.\n", obj_ptr);
		else
			print(fd, "%I still has %d magic spells left in it.\n", obj_ptr, 
				obj_ptr->shotscur);
		return(0);
	}

	if(!S_ISSET(ply_ptr, obj_ptr->magicpower - 1)) {
		print(fd, "You dont know the spell of this wand.\n");
		return(0);
	}

	if(obj_ptr) {
		if(F_ISSET(obj_ptr, ONOFIX)) {
			print(fd, "This object cannot be repaired or recharged.\n");
			return(0);
		}

		cost = obj_ptr->value * 1/3;
		if(cost == 0)
			cost = 1;

		if((ply_ptr->gold - cost) < 0) {
			print(fd, "You don't have enough gold.\n");
			return(0);
		}

		ply_ptr->gold -= cost;
		obj_ptr->shotscur = obj_ptr->shotsmax;
		F_SET(obj_ptr, ONOFIX);
		ANSI(fd, GREEN);
		print(fd, "You recharged the %s, but it cannot be recharged again.\n", 
			obj_ptr->name);
		print(fd, "The process required %d gold pieces.\n", cost);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		return(0);
	}

	return(0);
}

/***********************************************************************/
/*								transmute						       */
/***********************************************************************/
/* This function will allow an alchemist to transmute objects to gold. */

int transmute(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room	*rom_ptr;
	object	*obj_ptr;
	int		gold, fd, poorquality = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->class < CARETAKER && ply_ptr->class != ALCHEMIST) {
		print(fd, "Only an alchemist knows the secret of transmuting "
						"base materials to gold.\n"); 
		return(0);
	}

	if(F_ISSET(rom_ptr, RNOMAG)) {
		print(fd, "There's an aura in this place that prevents you from "
						"doing that.\n");
		return(0);
	}

	if(cmnd->num < 2) {
		print(fd, "Transmute what?\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, 
				cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(obj_ptr->type == WAND || 
		obj_ptr->type == SCROLL || 
		obj_ptr->type == POTION) {
		print(fd, "Magic items cannot be transmuted.\n");
		return(0);
	}

	luck(ply_ptr);	
	gold = obj_ptr->value * ply_ptr->level/50 + 
				obj_ptr->value * ply_ptr->intelligence/100 + 
				mrand(0, 1) * obj_ptr->value/10 + 
				obj_ptr->value/5;
	gold = (gold * (Ply[fd].extr->luck)/50); 
	gold = MIN( gold, 10000);
	
	if((obj_ptr->type <= MISSILE || obj_ptr->type == ARMOR) &&
	   obj_ptr->shotscur <= obj_ptr->shotsmax/8) 
		poorquality = 1;

	if(gold < 20 || poorquality) {
		if(poorquality && gold > 20)
			gold = MAX(2, gold/4);
		print(fd, "The material was of very poor quality.\n");
	}

	ANSI(fd, YELLOW);
	print(fd, "You transmuted %i into %d gold pieces!.\n", obj_ptr, gold);
	ANSI(fd, BOLD);
	ANSI(fd, WHITE);
	broadcast_rom(fd, ply_ptr->rom_num, "%M transmuted %1i to gold!", 
		ply_ptr, obj_ptr);

	ply_ptr->gold += gold;
	del_obj_crt(obj_ptr, ply_ptr);
	free_obj(obj_ptr);

	return(0);

}

/**************************************************************************/
/*							Alchemist renewal							  */
/**************************************************************************/
/* This function enables an alchemist to renew an enchanted silver weapon */

int silver_renew(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	int 	fd, cost;
	object 	*obj_ptr;
 
	fd = ply_ptr->fd;

	if(ply_ptr->class != ALCHEMIST && ply_ptr->class < CARETAKER) {
		print(fd, "Only an alchemist can do that.\n");
		return(0);
	}

	if(ply_ptr->level < 7 && ply_ptr->class < CARETAKER) {
		print(fd, "You are not experienced enough to do that.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "You can't do that.  You're blind!\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, cmnd->str[1], cmnd->val[1]);	
	if(!obj_ptr) {
		print(fd, "That's not in your inventory.\n");
		return(0);
	}

	if(obj_ptr->type > 5 || !F_ISSET(obj_ptr, OSILVR)) { 
		print(fd, "That's not something that can be renewed.\n");
		return(0);
	}

	if(F_ISSET(obj_ptr, OTMPEN)) {
		print(fd, "The silver in %i is already active.\n", obj_ptr);
		return(0);
	}

	if(!dec_daily(&ply_ptr->daily[DL_ENCHA]) && 
		ply_ptr->class < CARETAKER) {
			print(fd, "Your powers are exhausted for the rest of the day.\n");
			return(0);
	}

	cost = obj_ptr->value/3;
	if(ply_ptr->gold < cost) {
		print(fd, "You don't have enough gold.\n");
		return(0);
	}

	print(fd, "The silver in %i begins to glow brightly.\n", obj_ptr);
	print(fd, "The process required %d in gold.\n", cost);
    broadcast_rom(fd, ply_ptr->rom_num, "%M enchants %1i.", ply_ptr, obj_ptr);
	F_SET(obj_ptr, OTMPEN);
	ply_ptr->gold -= cost;
	savegame(ply_ptr, 0);

	return(0);

}

/*************************************************************************/
/*								super_strength							 */
/*************************************************************************/
/* This function allows players to cast the super_strength magic spell.  */

int super_strength(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(F_ISSET(ply_ptr, PSTRNG)) {
		print(fd, "You are already under that spell!\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PBERSK)) {
		print(fd, "Not while you are going berserk!\n");
		return(0);
	}

	if(!S_ISSET(ply_ptr, SSTRNG) && how == CAST) {
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

	if(!dec_daily(&ply_ptr->daily[DL_STRNG]) && 
		ply_ptr->class < CARETAKER) {
			print(fd, "You can only do that three times in a day.\n");
			return(0);
		}
	
	if(cmnd->num == 2) {
		ply_ptr->lasttime[LT_STRNG].ltime = time(0);
		F_SET(ply_ptr, PSTRNG);
		compute_thaco(ply_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M's body ripples with muscle.", ply_ptr);
		if(how == CAST) {
			print(fd, "Your body flexes with massive muscles.\n");
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 6;
			else
				ply_ptr->mpcur -= 12;
			ply_ptr->hpcur = (ply_ptr->hpmax) + ply_ptr->level;
			ply_ptr->strength += 5;
			ply_ptr->lasttime[LT_STRNG].interval = MAX(300, 600 +
				bonus[ply_ptr->strength] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
            	print(fd,
					"The magic in the air increase the power of your spell.\n");
            	ply_ptr->lasttime[LT_STRNG].interval += 600L;
        	}                                
		}
		else {
			if(!dec_daily(&ply_ptr->daily[DL_STRNG])) {
					print(fd, 
						"Your stomach rebels against the strong drink.\n");
					return(0);
			}
			if(F_ISSET(ply_ptr, PBERSK) && how == CAST) {
        		print(fd, "Not while you are a berserker!\n");
        	return(0);
    }

			print(fd, "Your strength increases tremendously!\n");
			ply_ptr->hpcur = (ply_ptr->hpmax) + ply_ptr->level;
			ply_ptr->strength += 5;
			ply_ptr->lasttime[LT_STRNG].interval = 600L;
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
			print(fd, "That person is not here.\n");
			return(0);
		}

		if(!dec_daily(&ply_ptr->daily[DL_STRNG]) && 
			ply_ptr->class < CARETAKER) {
				print(fd, "You need the rest of the day to recuperate.\n");
				return(0);
		}


		if(F_ISSET(crt_ptr, PSTRNG)){
			print(fd, "%M is already under the super-strength spell!\n", crt_ptr);
			print(crt_ptr->fd, "%M attempted to cast a super strength spell on you.\n", ply_ptr);
			return(0);
		}
		F_SET(crt_ptr, PSTRNG);
		compute_thaco(crt_ptr);
		crt_ptr->lasttime[LT_STRNG].ltime = time(0);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
			"%M casts a super strength spell on %m.", ply_ptr, crt_ptr);
		print(crt_ptr->fd, "%M casts a super strength spell on you.\n", 
			ply_ptr);

		if(how == CAST) {
			print(fd, "You cast a super strength spell on %s.\n", crt_ptr);

			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 6;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 12;

			if(F_ISSET(crt_ptr, PBERSK)){
				print(fd, "The spell fizzles.\n");
				print(crt_ptr->fd, "The spell fizzles because you are berserk!\n"); 
				return(0);
			}
			crt_ptr->hpcur = (crt_ptr->hpmax) + crt_ptr->level;
			crt_ptr->strength += 5;
			crt_ptr->lasttime[LT_STRNG].interval = MAX(300, 600 +
				bonus[ply_ptr->strength] * 600);

			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
            	print(fd,
					"The magic in the air increase the power of your spell.\n");
            	crt_ptr->lasttime[LT_STRNG].interval += 600L;
        	}                                
		}
		else {
			print(fd, "%M's strength increases tremendously.\n", crt_ptr);
			crt_ptr->hpcur = (crt_ptr->hpmax) + crt_ptr->level;
			crt_ptr->strength += 5;
			crt_ptr->lasttime[LT_STRNG].interval = 600L;
		}

		return(1);
	}
}

/************************************************************************/
/*								shrink									*/
/************************************************************************/
/*	Player can shrink to mouse-size once each day						*/		

int shrink(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;


	if(ply_ptr->mpcur < 15 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	if(ply_ptr->mpcur < 30 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(!S_ISSET(ply_ptr, SSHRNK) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(!dec_daily(&ply_ptr->daily[DL_SHRNK]) && 
		ply_ptr->class < CARETAKER) {
			print(fd, "You can only do that once each day.\n");
			print(fd, "It's very hard on your system, you know.\n");
			return(0);
		}
	
	if(cmnd->num == 2) {
		ply_ptr->lasttime[LT_SHRNK].ltime = time(0);
		F_SET(ply_ptr, PSHRNK);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M's body shrinks to mouse size.", ply_ptr);
		if(how == CAST) {
			print(fd, "Your body shrinks to the size of a small mouse.\n");
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 10;
			else
				ply_ptr->mpcur -= 15;
			ply_ptr->lasttime[LT_SHRNK].interval = MAX(1800, 1200 +
				bonus[ply_ptr->dexterity] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
            	print(fd,
					"The magic in the air increase the power of your spell.\n");
            	ply_ptr->lasttime[LT_SHRNK].interval += 1200L;
        	}                                
		}
		else {
			if(!dec_daily(&ply_ptr->daily[DL_SHRNK]) && 
				ply_ptr->class < CARETAKER) {
					print(fd, 
						"Your stomach rebels against the strong drink.\n");
					return(0);
			}

			print(fd, "Your body shrinks to the size of a small mouse!\n");
			ply_ptr->lasttime[LT_SHRNK].interval = 1800L;
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
			print(fd, "I don't see that person here.\n");
			return(0);
		}

		if(!dec_daily(&ply_ptr->daily[DL_SHRNK]) && 
			ply_ptr->class < CARETAKER) {
				print(fd, "You need the rest of the day to recuperate.\n");
				return(0);
		}

		F_SET(crt_ptr, PSHRNK);
		crt_ptr->lasttime[LT_SHRNK].ltime = time(0);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
			"%M casts a shrink spell on %m.", ply_ptr, crt_ptr);
		print(crt_ptr->fd, "%M casts a shrink spell on you.\n", ply_ptr);
		print(crt_ptr->fd, "You get as small as a mouse!\n");

		if(how == CAST) {
			print(fd, "You cast a shrink spell on %s.\n", crt_ptr);

			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 15;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 30;

			crt_ptr->lasttime[LT_SHRNK].interval = MAX(300, 1200 +
				bonus[ply_ptr->dexterity] * 600);

			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
	            print(fd,
					"The magic in the air increase the power of your spell.\n");
	            crt_ptr->lasttime[LT_SHRNK].interval += 1200L;
	        }                                
		}
		else {
			print(fd, "%M's body shrinks to mouse size.\n", crt_ptr);
			crt_ptr->lasttime[LT_SHRNK].interval = 1200L;
		}

		return(1);
	}
}

/************************************************************************/
/*							reflection spell 							*/
/************************************************************************/
/* 	The reflection spell allows a player to reflect offensive			*/
/*	spells cast by other players.										*/

int reflect(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(!S_ISSET(ply_ptr, SRFLCT) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}


	if(ply_ptr->mpcur < 25 && how == CAST) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(cmnd->num == 1 && !S_ISSET(ply_ptr, PCHAOS) && how == CAST) {
		print(fd, "The spell cannot be cast on lawful members of society.\n");
		return(0);
	}

	if(ply_ptr->alignment < 0 && how == CAST) {
		ANSI(fd, YELLOW);
		print(fd, "ZZzzz... POP!  The spell fails.\n");
		ANSI(fd, WHITE);
		print(fd, "Alith, the Holy Goat, does not approve of your "
			"evil alignment.\n");
		print(fd, "You are surrounded by a dark and terrible smelling haze.\n");
		ply_ptr->mpcur -= 25;
		return(0);
	}

	if(cmnd->num == 2) {
		ply_ptr->lasttime[LT_RFLCT].ltime = time(0);
		F_SET(ply_ptr, PRFLCT);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"Alith, the Holy Goat, surrounds %M's body with a"
				" reflective, mirror-like haze.", ply_ptr);
		if(how == CAST) {
			print(fd, "Alith, the Holy Goat, surrounds your "
				"body with a reflective, mirror-like haze.\n");
			ply_ptr->mpcur -= 25;
			ply_ptr->lasttime[LT_RFLCT].interval = MAX(1200, 600 +
				bonus[ply_ptr->piety] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
        	    print(fd,
					"The magic in the air increase the power of your spell.\n");
        	    ply_ptr->lasttime[LT_RFLCT].interval += 1200L;
        	}                                
		}
		else {
			print(fd, "Alith, the Holy Goat, smiles and surrounds"
				" your body with a mirror-like haze.\n");
			ply_ptr->lasttime[LT_RFLCT].interval = 1800L;
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
			print(fd, "I don't see that person here.\n");
			return(0);
		}

		if(how == POTION && !F_ISSET(crt_ptr, PCHAOS)) {
			print(fd, "Only the chaotic may drink this.\n"); 
			print(fd, "Your stomach churns and you feel sick.\n");
			ply_ptr->hpcur -= 10;
			if(ply_ptr->hpcur < 0)
				ply_ptr->hpcur = 1;
			return(0);
		}

		if(how == POTION && ply_ptr->alignment < 0) {
			print(fd, "Your evil nature cannot abide Alith's potion.\n");
			print(fd, "Your stomach churns and you feel sick.\n");
			ply_ptr->hpcur -= 10;
			if(ply_ptr->hpcur < 0)
				ply_ptr->hpcur = 1;
			return(0);
		}

		if(how == CAST && !F_ISSET(crt_ptr, PCHAOS)) {
			print(fd, "That spell can be cast only on the chaotic.\n");
			return(0);
		}

		if(how == CAST && crt_ptr->alignment < 0) {
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"%M calls upon Alith to cast a reflection spell on %m.",
					ply_ptr, crt_ptr);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"The angry image of Alith floats briefly above the head of %m.",
					 crt_ptr);
			print(crt_ptr->fd, 
				"%M calls upon Alith to cast a reflection spell on you.\n", 
					ply_ptr);
			print(crt_ptr->fd, 
				"Alith does not like your alignment and the spell fails.\n");
			print(ply_ptr->fd, 
				"%M is evil and Alith, the Holy Goat, does not approve.\n", 
					crt_ptr);
			print(ply_ptr->fd, "The spell fails.\n"); 
			ply_ptr->mpcur -= 25;
				return(0);
		}

		F_SET(crt_ptr, PRFLCT);
		crt_ptr->lasttime[LT_RFLCT].ltime = time(0);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
			"%M calls upon Alith to cast a reflection spell on %m.",
				ply_ptr, crt_ptr);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
			"The smiling image of Alith floats briefly above the head of %m.",
				 crt_ptr);
		print(crt_ptr->fd, "%M calls upon Alith to cast a reflection "
			"spell on you.\n", ply_ptr);
		print(crt_ptr->fd, "Your body is surrounded by a reflective, "
			"mirror-like haze.\n");

		if(how == CAST) {
			print(fd, "You cast a reflection spell on %s.\n", crt_ptr);
			ply_ptr->mpcur -= 25;
			crt_ptr->lasttime[LT_RFLCT].interval = MAX(300, 1200 +
				bonus[ply_ptr->piety] * 600);

			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
        	    print(fd,
					"The magic in the air increase the power of your spell.\n");
        	    crt_ptr->lasttime[LT_RFLCT].interval += 1200L;
        	}                                
		}
		else {
			print(fd, "%M's body is surrounded by a protective,"
				" mirror-like haze.\n", crt_ptr);
			crt_ptr->lasttime[LT_RFLCT].interval = 1200L;
		}

		return(1);
	}
}

/************************************************************************/
/*							evil eye spell 								*/
/************************************************************************/
/* 	This allows a player to cast an evil eye spell on another player    */ 
/*  or a monster.  Monks are more proficient than other players.        */

int evil_eye(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int			penalty, fd;


	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(!S_ISSET(ply_ptr, SEVEYE) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(!F_ISSET(ply_ptr, PCHAOS)) {
		print(fd, "That spell is only for the chaotic.  You are lawful.\n");
		return(0);
	}
	
	if(ply_ptr->class == MONK) {
		if(ply_ptr->mpcur < 12 && how == CAST) {
			print(fd, "Not enough magic points.\n");
			return(0);
		}
	}
	else if(ply_ptr->mpcur < 25 && how == CAST) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(cmnd->num == 1 && !S_ISSET(ply_ptr, PCHAOS) && how == CAST) {
		print(fd, "The spell cannot be cast on lawful members of society.\n");
		return(0);
	}

	if(ply_ptr->alignment > 0 && how == CAST && ply_ptr->class < CARETAKER) {
		ANSI(fd, YELLOW);
		print(fd, "ZZzzz... POP!  The spell fails.\n");
		ANSI(fd, WHITE);
		print(fd, 
			"Zeth, the Unholy Goat, does not approve of your goodness.\n");
		print(fd, "You are surrounded by a dark and terrible smelling haze.\n");
		if(ply_ptr->class == MONK) 
			ply_ptr->mpcur -= 12;
		else
			ply_ptr->mpcur -= 25;
		ply_ptr->alignment -= 50;
		return(0);
	}

	if(cmnd->num == 2) {
		ply_ptr->lasttime[LT_EVEYE].ltime = time(0);
		F_SET(ply_ptr, PEVEYE);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"Zeth, the Unholy Goat, peers at %M with an evil eye," 
				" causing %s to tremble.\n", ply_ptr, 
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
		if(how == CAST) {
			print(fd, 
				"You stagger as the evil eye of Zeth peers at you.\n");
			if(ply_ptr->class == MONK) 
				ply_ptr->mpcur -= 12;
			else
				ply_ptr->mpcur -= 25;
			ply_ptr->lasttime[LT_EVEYE].interval = 300L;
			ply_ptr->alignment -= 50;
			compute_thaco(ply_ptr);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
        	    print(fd,
					"The magic in the air increase the power of your spell.\n");
        	    ply_ptr->lasttime[LT_EVEYE].interval += 600L;
        	}                                
			else
				ply_ptr->lasttime[LT_EVEYE].interval += 300L;
			return(1);
		}

		if(how == POTION && ply_ptr->alignment > 0) {
			ply_ptr->lasttime[LT_EVEYE].ltime = time(0);
			F_SET(ply_ptr, PEVEYE);
			print(fd, "Zeth's evil eye floats before your face.\n");
			if(ply_ptr->alignment < -250)
				print(fd, "Your evilness please him.\n");
			else {
				print(fd, 
					"The Unholy Goat makes you feel quite clumsy and dizzy,\n");
				print(fd, "and you start to stagger.\n");
			}
			ply_ptr->lasttime[LT_EVEYE].interval = 120L;
			ply_ptr->alignment -= 50;
			compute_thaco(ply_ptr);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
        	    print(fd,
					"The magic in the air increase the power of the potion.\n");
        	    ply_ptr->lasttime[LT_EVEYE].interval += 300L;
        	}                                
			else
				ply_ptr->lasttime[LT_EVEYE].interval += 180L;
			return(0);
		}
		else {
			ply_ptr->lasttime[LT_EVEYE].ltime = time(0);
			F_SET(ply_ptr, PBLESS);
			print(fd, "Zeth's evil eye floats before your face.\n");
			print(fd, "The Unholy Goat approves of your evil nature,\n");
			print(fd, "and you feel his power flow over you.\n");
			ply_ptr->lasttime[LT_BLESS].interval = 1200L;
			ply_ptr->alignment -= 50;
			compute_thaco(ply_ptr);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
        	   	print(fd,
					"The magic in the air increase the power of your spell.\n");
        	   	ply_ptr->lasttime[LT_BLESS].interval += 1200L;
        	}                                
			else
				ply_ptr->lasttime[LT_BLESS].interval += 300L;
		}

		return(1);
	}

	else {
		if(how == POTION) {
			print(fd, "You can't force anyone to drink a potion.\n");
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
		if(crt_ptr->type == PLAYER) {
			if(how == CAST && !F_ISSET(crt_ptr, PCHAOS)) { 
				print(fd, "That spell can only be used on the chaotic.\n");
				print(fd, "%M is lawful.\n", crt_ptr);
				return(0);
			}
			if(how == CAST) {
				F_SET(crt_ptr, PEVEYE);
				crt_ptr->lasttime[LT_EVEYE].ltime = time(0);
				broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
					"%M calls upon Zeth to cast an evil eye spell on %m.",
						ply_ptr, crt_ptr);
				broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
					"%M staggers as the evil eye of Zeth stares briefly at %s.",
						crt_ptr, F_ISSET(crt_ptr, PMALES) ? "him":"her");
				print(crt_ptr->fd, 
					"%M calls upon Zeth to cast an evil eye spell on you.\n", 
						ply_ptr);
				if(crt_ptr->alignment > 0) 
					print(crt_ptr->fd, 
						"You feel dizzy and clumsy, and start to stagger.\n");
				else
					print(crt_ptr->fd, 
						"You feel his evil power enter your body.\n");
				print(fd, "You cast an evil eye spell on %m.\n", crt_ptr);
				if(ply_ptr->class == MONK)
					ply_ptr->mpcur -= 12;
				else
					ply_ptr->mpcur -= 25;
				ply_ptr->alignment -= 50;
				compute_thaco(crt_ptr);
				crt_ptr->lasttime[LT_EVEYE].interval = MAX(600, 120 +
					bonus[ply_ptr->piety] * 60);
	
				if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
        	    	print(fd,
					"The magic in the air increase the power of your spell.\n");
       			    crt_ptr->lasttime[LT_EVEYE].interval += 300L;
       			}                                
				else
					crt_ptr->lasttime[LT_EVEYE].interval += 120L;
			}
		}	
		else if(crt_ptr->type != PLAYER) {
			if(how == CAST) {
				if(crt_ptr->alignment < 0) {
					print(fd,
						"%M is evil, and Zeth shuts his evil eye.\n", crt_ptr);
					if(!F_ISSET(crt_ptr, MNOGEN)) {
						print(fd, "The spell cannot be cast on %s.\n", 
							F_ISSET(crt_ptr, MMALES) ? "him":"her");
					}
					else
						print(fd, "The spell cannot be cast on it.\n"); 

					if(ply_ptr->class == MONK)
						ply_ptr->mpcur -= 12;
					else
						ply_ptr->mpcur -= 25;
					return(0);
				}
				F_SET(crt_ptr, MEVEYE);
				broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
					"%M calls upon Zeth to cast an evil eye spell on %m.",
						ply_ptr, crt_ptr);
				broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
					"%M staggers as the evil eye of Zeth stares briefly at %s.",
						crt_ptr, F_ISSET(crt_ptr, PMALES) ? "him":"her");
				print(fd, 
					"An evil eye spell is cast on %m, causing %s to stagger.\n",
						crt_ptr, F_ISSET(crt_ptr, PMALES) ? "him":"her");
				if(ply_ptr->class == MONK)
					ply_ptr->mpcur -= 12;
				else
					ply_ptr->mpcur -= 25;
				ply_ptr->alignment -= 50;
				compute_thaco(crt_ptr);
	
			}
		}

		return(1);
	}
}

/*************************************************************************/
/*								freeze									 */
/*************************************************************************/
/* This function allows players to cast a freeze spell.  				 */	

int freeze(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;


	if(!S_ISSET(ply_ptr, SFREEZ) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(ply_ptr->mpcur < 25 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	if(ply_ptr->mpcur < 30 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(cmnd->num == 2) {
		if(how == CAST || how == SCROLL || how == WAND || how == POTION) {
			if(F_ISSET(ply_ptr, PRCOLD)) {
				print(fd, 
					"Your resist-cold spell dissipates as it protects you.\n");
				F_CLR(ply_ptr, PRCOLD);
				return(0);
			}
			else if(how != POTION) {
				ply_ptr->lasttime[LT_FROZE].ltime = time(0);
				F_SET(ply_ptr, PFROZE);
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M's body has been frozen solid, and %s cannot move.", 
						ply_ptr, F_ISSET(ply_ptr, PMALES) ? "he":"she");
				print(fd, "Well, that was smart.\n");
				print(fd, "Now you are frozen stiff and you can't move.\n");
       	    	ply_ptr->lasttime[LT_FROZE].interval += 300L;
			}

			if(how == CAST) {
				if(ply_ptr->class == MAGE)
					ply_ptr->mpcur -= 25;
				else
					ply_ptr->mpcur -= 30;
			}

			if(how == POTION) {
				F_SET(ply_ptr, PFROZE);
				print(fd, 
					"It freezes you all the way down to your toes.\n");
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M stupidly drank a freezing potion and %s cannot move!", 
						ply_ptr, F_ISSET(ply_ptr, PMALES) ? "he":"she");
				ply_ptr->lasttime[LT_FROZE].interval += 300L;
			}
		}
		return(1);
	}

	if(how == POTION) {
		print(fd, "You can't force a potion on someone else.\n");
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
			print(fd, "I don't see that person here.\n");
			return(0);
		}
	}

	if(crt_ptr->type == PLAYER) {
		if(how == CAST || how == SCROLL || how == WAND) {
			if(F_ISSET(crt_ptr, PRFLCT) || F_ISSET(crt_ptr, PRMAGI)) {
				print(fd, "Too bad.  %M is protected by a powerful spell.\n", 
					crt_ptr);
				print(fd, "The spell makes the air around you very cold.\n", 
					crt_ptr);
				print(crt_ptr->fd, "%M tried to cast a freeze spell on you.\n", 
					ply_ptr);
				print(crt_ptr->fd, 
					"Your magical protection kept you from harm,"
						" but the air is now very cold.\n");
				broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
					"%M was unsuccessful in casting a freeze spell on %m.", 
						ply_ptr, crt_ptr);

				if(how == CAST) {
					crt_ptr->lasttime[LT_FROZE].interval += 300L;
					if(ply_ptr->class == MAGE)
						ply_ptr->mpcur -= 25;
					if(ply_ptr->class != MAGE)
						ply_ptr->mpcur -= 30;
				}
				return(1);
			}

			if(F_ISSET(crt_ptr, PRCOLD)) {
				print(fd, "%M is protected by a resist-cold spell.\n", 
					crt_ptr);
				print(crt_ptr->fd, "%M tried to cast a freeze spell on you.\n", 
					ply_ptr);
				print(crt_ptr->fd, 
					"Your resist-cold spell dissipates as it protects you.\n");
				F_CLR(crt_ptr, PRCOLD);
				broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
					"%M was unsuccessful in casting a freeze spell on %m.", 
						ply_ptr, crt_ptr);
			}
			else {
				F_SET(crt_ptr, PFROZE);
				crt_ptr->lasttime[LT_FROZE].ltime = time(0);
				print(fd, "You cast a freeze spell on %m.\n", crt_ptr);
				print(crt_ptr->fd, "%M casts a freeze spell on you.\n", 
					ply_ptr);
				broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
					"%M casts a freeze spell on %m.", ply_ptr, crt_ptr);
			}
		}

		if(how == CAST) {
			crt_ptr->lasttime[LT_FROZE].interval += 300L;
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 25;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 30;
		}
		return(1);
	}

	else if(crt_ptr->type != PLAYER) {
		if(F_ISSET(crt_ptr, MPERMT) && 
			(how == CAST || how == SCROLL || how == WAND)) {
				print(fd, "%M casually deflects your freeze spell.\n", 
					crt_ptr); 
			return(1);
		}
		if(how == CAST || how == SCROLL || how == WAND) {
			F_CLR(crt_ptr, MFLEER);
			F_CLR(crt_ptr, MFOLLO);
			F_CLR(crt_ptr, MBLOCK);
			F_CLR(crt_ptr, MPGUAR);
			F_SET(crt_ptr, MFROZE);
			del_enm_crt(ply_ptr->name, crt_ptr);
			crt_ptr->hpcur += 12;
			if(crt_ptr->hpcur > crt_ptr->hpmax)
				crt_ptr->hpcur = crt_ptr->hpmax;
			print(fd, "You cast a freeze spell on %m.\n", crt_ptr);
			if(!F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, 
					"Now %s cannot block you, flee from you nor follow you.\n", 
						F_ISSET(crt_ptr, MMALES) ? "he":"she");
			}
			else
				print(fd, 
				"Now it cannot block you, flee from you nor follow you.\n"); 
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts a freeze spell on %m.", ply_ptr, crt_ptr);

			if(how == CAST) {
				crt_ptr->lasttime[LT_FROZE].interval += 300L;
				if(ply_ptr->class == MAGE)
					ply_ptr->mpcur -= 25;
				if(ply_ptr->class != MAGE)
					ply_ptr->mpcur -= 30;
			}
		}
		return(1);
	}
}

/************************************************************************/
/*								hex										*/
/************************************************************************/
/*	The hex spell reduces a player's luck, or reduces a monster's THAC0 */		
/*  Only a mage, bard, monk, druid or alchemist can cast this.			*/

int hex(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int			fd, i, found = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;


	if(!S_ISSET(ply_ptr, SHEXSP) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if((ply_ptr->class == ASSASSIN)   ||
		(ply_ptr->class == BARBARIAN) ||
		(ply_ptr->class == CLERIC)    ||
		(ply_ptr->class == FIGHTER)   ||
		(ply_ptr->class == PALADIN)   ||
		(ply_ptr->class == RANGER)    ||
		(ply_ptr->class == THIEF)) {
		print(fd, "Your class cannot cast the hex spell.\n");
		return(0);
	}

	if(ply_ptr->class == MAGE && ply_ptr->level < 5) {
		print(fd, "A mage must be at least level five to cast a hex spell.\n");
		return(0);
	}
		 
	if(ply_ptr->class != MAGE && ply_ptr->level < 7) {
		print(fd, "You must be at least level seven to cast the hex spell.\n");
		return(0);
	}
		 
	if(ply_ptr->class == MAGE && ply_ptr->mpcur < 15) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(ply_ptr->class != MAGE && ply_ptr->mpcur < 20) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	
	if(cmnd->num == 2) {
		for(i = 0; i < MAXWEAR; i++) {
			if(ply_ptr->ready[i] && F_ISSET(ply_ptr->ready[i], OLUCKY)
				&& (ply_ptr->ready[i]->shotscur > 0)) {
					found = 1;
					ply_ptr->ready[i]->shotscur--;
			}
		}
		if(found) {
			print(fd, "It's a good thing you're holding something lucky.\n");
			print(fd, "You almost cast a hex spell on yourself.\n");
			return(0);
		}

		F_SET(ply_ptr, PHEXED);
		compute_thaco(ply_ptr);
		Ply[fd].extr->luck = 20;
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M just foolishly put a hex curse on %s.", ply_ptr),
				F_ISSET(ply_ptr, PMALES) ? "himself":"herself";
		print(fd, "You foolishly put a hex curse on yourself.\n");
		if(how == CAST) {
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 15;
			else
				ply_ptr->mpcur -= 20;
		}
		return(1);
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

	if(how == POTION) {
		print(fd, "You can only use a potion on yourself.\n");
		return(0);
	}

	if(how == CAST) {
		if(ply_ptr->class == MAGE)
			ply_ptr->mpcur -= 15;
		else
			ply_ptr->mpcur -= 20;
	}

	if(crt_ptr->type == PLAYER) {
		for(i = 0; i < MAXWEAR; i++) {
			if(crt_ptr->ready[i] && F_ISSET(crt_ptr->ready[i], OLUCKY)
				&& (crt_ptr->ready[i]->shotscur > 0)) {
					crt_ptr->ready[i]->shotscur--;
					found = 1;
			}
		}
		if(found) {
			print(fd, "%M is holding a lucky charm.  ", crt_ptr);
			print(fd, "The hex spell fails.\n");
			print(crt_ptr->fd, "%M tried to cast a hex spell on you.\n",
				ply_ptr);
			print(crt_ptr->fd, "Your lucky charm stopped the spell!\n");
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"%M tried to cast a hex spell on %m, but failed.", 
					ply_ptr, crt_ptr);
			return(0);
		}
			
		if(F_ISSET(crt_ptr, PRFLCT)) {
			F_SET(ply_ptr, PHEXED);
			Ply[ply_ptr->fd].extr->luck = 20;
			compute_thaco(ply_ptr);
			print(fd, "%M is protected by a powerful reflective spell!\n",
				crt_ptr);
			print(fd, "The hex spell causes a shadow to pass over you.\n");
			print(crt_ptr->fd, "%M tried to cast a hex spell on you.\n",
				ply_ptr);
			print(crt_ptr->fd, "Your reflective spell blew it back!\n");
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
				"%M tried to cast a hex spell on %m, but failed.", 
					ply_ptr, crt_ptr);
			return(0);
		}
		else {
			F_SET(crt_ptr, PHEXED);
			Ply[crt_ptr->fd].extr->luck = 20;
			compute_thaco(crt_ptr);
		}
	}
	else {
		if(!F_ISSET(crt_ptr, MHEXED)) {
			F_SET(crt_ptr, MHEXED);
			add_enm_crt(ply_ptr->name, crt_ptr);
			if(F_ISSET(crt_ptr, MPERMT)) 
				crt_ptr->thaco += 3;
			else
				crt_ptr->thaco += 5;
			if(crt_ptr->thaco > 20)
				crt_ptr->thaco = 20;
		}
		else {
			print(fd, 
				"%M has already been hexed, so your spell was a waste.\n", 
					crt_ptr);
			return(0);
		}
	}

	print(fd, "You cast a hex spell on %m.\n", crt_ptr);
	broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
		"%M casts a hex spell on %m.", ply_ptr, crt_ptr);
	broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
		"A shadow falls across %m as the spell settles.\n", crt_ptr);
	if(crt_ptr->type == PLAYER) {
		print(crt_ptr->fd, "A shadow falls on your face.  ");
		print(crt_ptr->fd, "%M has cast a hex spell on you.\n", ply_ptr);
	}

	return(1);
}

/**********************************************************************/
/*							conjure     						      */
/**********************************************************************/
/* This function allows a mage to create a creature that will appear  */
/* in the room in which he is located.  The mage must be chaotic.	  */
/* The creature will be aggressive to the target specified by the     */
/* mage, if the target is also chaotic.  If no target is specified,   */
/* then the creature will be aggressive to the conjurer.  No exp is   */
/* awarded for killing the creature, and no exp is lost when the      */
/* creature kills a player.                                           */

int conjure(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	creature	*ply_ptr2;
	room		*rom_ptr;
	int			fd, protector = 500;
	long 		t;
	ctag		*cp;

	fd = ply_ptr->fd;


	if(ply_ptr->class != MAGE && ply_ptr->class < CARETAKER) {
		print(fd, "Only a chaotic mage can conjure a protector.\n"); 
		return(PROMPT);
	}

	if(!F_ISSET(ply_ptr, PCHAOS)) {
		print(fd, "Only a chaotic mage can conjure a protector.\n"); 
		return(PROMPT);
	}

	if(ply_ptr->mpcur < 20) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(!dec_daily(&ply_ptr->daily[DL_CONJR]) && 
		ply_ptr->class < CARETAKER) {
			print(fd, "Your powers to conjure are exhausted for "
				"the rest of the day.\n");
			return(0);
	}

	if(cmnd->num > 2) {
		cmnd->str[2][0] = up(cmnd->str[2][0]);
		ply_ptr2 = find_crt(ply_ptr, ply_ptr->parent_rom->first_mon,
			cmnd->str[2], cmnd->val[2]);

		if(!ply_ptr2)
			ply_ptr2 = find_crt(ply_ptr, ply_ptr->parent_rom->first_ply,
				cmnd->str[2], cmnd->val[2]);

		if(!ply_ptr2)
			ply_ptr2 = find_who(cmnd->str[2]);

		if(!ply_ptr2) {
			print(fd, "The guardian refuses to appear.\n");
			return(PROMPT);
		}

		if(ply_ptr2->class >= CARETAKER) {
			print(fd, "The guardian refuses to appear.\n");
			return(PROMPT);
		}
	}

	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->level > 10)
		protector++;
	if(ply_ptr->level > 15)
		protector++;

	if(load_crt(protector, &crt_ptr) < 0) {
        print(fd, "The guardian refuses to appear.\n");
        return(0);
    }

	t = time(0);
	crt_ptr->lasttime[LT_ATTCK].ltime = t;
	crt_ptr->lasttime[LT_ATTCK].interval = 2;

	ply_ptr->mpcur -= 20;
	print(fd, "You have conjured a guardian %s.\n", crt_ptr);
	broadcast_rom(fd, ply_ptr->rom_num, 
		"%M has conjured a guardian %s to protect %s.", ply_ptr, crt_ptr, 
			F_ISSET(ply_ptr, MMALES) ? "himself" : "herself");

	add_crt_rom(crt_ptr, rom_ptr, 1);
	add_active(crt_ptr);

	if(cmnd->num > 2) {
		print(ply_ptr2->fd, "%M has conjured a guardian to attack you!\n", 
			ply_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, "%M points at %m.",
			ply_ptr, ply_ptr2); 
		if(ply_ptr2) {
			if(F_ISSET(ply_ptr2, PCHAOS)) {
				if(ply_ptr2->level > 5) {
					if(!F_ISSET(ply_ptr2, PRFLCT)) {
						print(fd, "The guardian will attack %m.\n", ply_ptr2);
						add_enm_crt(ply_ptr2->name, crt_ptr);
					}
					else {
						print(fd, "The guardian is confused because "
							"%m is protected by a reflection spell!\n", 
								ply_ptr2);
						add_enm_crt(ply_ptr->name, crt_ptr);
						broadcast_rom(fd, ply_ptr->rom_num, 
							"The guardian is confused by %m's " 
								"reflection spell and turns on %m.\n", 
									ply_ptr2, ply_ptr);
					}
				}
				else {
					print(fd, "The guardian looks at you angrily and "
						"refuses to attack one so young.\n");
					add_enm_crt(ply_ptr->name, crt_ptr);
					broadcast_rom(fd, ply_ptr->rom_num, "The guardian will "
						"not attack one so young and turns angrily on %m.\n",
							ply_ptr);

				}
			}
			else {
				print(fd, 
					"The guardian is enraged because %m is lawful!\n", 
						ply_ptr2);
				add_enm_crt(ply_ptr->name, crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num, "The guardian will not "
					"attack a lawful player and turns on the summoner!");
			}
		}

	}
	else {
		cp = rom_ptr->first_ply;
		while(cp) {
			if(cp->crt && !F_ISSET(cp->crt, PDMINV)) {
				if(F_ISSET(cp->crt, PCHAOS) && cp->crt->level > 4)
           			add_enm_crt(cp->crt->name, crt_ptr);
			}
			cp = cp->next_tag;
		}
	}

	return(0);
}



/*************************************************************************/
/*								entangle								 */
/*************************************************************************/
/* This function allows a druid or a mage to cast an entangle spell,     */
/* preventing monster from fleeing, following or blocking.				 */ 

int entangle(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;


	if(!S_ISSET(ply_ptr, SENTNG) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(how == CAST) {
		if(ply_ptr->class != DRUID && 
			ply_ptr->class != MAGE &&
			ply_ptr->class != ASSASSIN &&
			ply_ptr->class < CARETAKER) {
				print(fd, 
					"Only a druid, mage or assassin can "
						"cast that spell to entangle a foe.\n");
				return(0);
		}

		if(ply_ptr->mpcur < 12) {
			print(fd, "Not enough magic points.\n");
			return(0);
		}

 		if(cmnd->num == 2) {
			print(fd, "Entangle yourself?  Don't be silly.\n");
			return(0);
		}

		if(ply_ptr->class == ASSASSIN)	
			ply_ptr->mpcur -= 15;
		else
			ply_ptr->mpcur -= 10;

		cmnd->str[2][0] = up(cmnd->str[2][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);

		if(!crt_ptr) {
			cmnd->str[2][0] = low(cmnd->str[2][0]);
			crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
			cmnd->str[2], cmnd->val[2]);
		}
		if(!crt_ptr) {
			print(fd, "Creature not found.\n");
			return(0);
		}

		if(crt_ptr && crt_ptr->type != PLAYER) {
			if(F_ISSET(crt_ptr, MPERMT)) {
				print(fd, "%M casually deflects your entangle spell.\n", 
					crt_ptr); 
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M's spell failed to entangle %m.\n", ply_ptr, crt_ptr);
				return(1);
			}
			F_CLR(crt_ptr, MFLEER);
			F_CLR(crt_ptr, MFOLLO);
			F_CLR(crt_ptr, MBLOCK);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts an entangle spell on %m.\n", ply_ptr, crt_ptr);
			print(fd, "You cast an entangle spell on %m.\n", crt_ptr);
			if(F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, 
					"Now it cannot follow you, flee from you or block you.\n"); 
			}
			else {
				print(fd, 
					"Now %s cannot follow you, flee from you or block you.\n", 
						F_ISSET(crt_ptr, MMALES) ? "he":"she");
			}
		}
		else if(ply_ptr && ply_ptr->type == PLAYER) {
			print(fd, "You cannot entangle %m.\n", crt_ptr);
		}
	}

	if(how == SCROLL || how == WAND) {
 		if(cmnd->num == 2) {
			print(fd, "Entangle yourself?  Don't be silly.\n");
			return(0);

		}
		cmnd->str[2][0] = up(cmnd->str[2][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[2], cmnd->val[2]);

		if(!crt_ptr) {
			cmnd->str[2][0] = low(cmnd->str[2][0]);
			crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
			cmnd->str[2], cmnd->val[2]);
		}
		if(!crt_ptr) {
			print(fd, "Creature not found.\n");
			return(0);
		}

		if(crt_ptr && crt_ptr->type != PLAYER) {
			if(F_ISSET(crt_ptr, MPERMT)) {
				print(fd, "%M easily wards off your entangle spell.\n", 
					crt_ptr); 
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M deflected %m's entangle spell.\n", crt_ptr, ply_ptr);
				return(1);
			}
			F_CLR(crt_ptr, MFLEER);
			F_CLR(crt_ptr, MFOLLO);
			F_CLR(crt_ptr, MBLOCK);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts an entangle spell on %m.\n", ply_ptr, crt_ptr);
			print(fd, "You cast an entangle spell on %m.\n", crt_ptr);
			if(F_ISSET(crt_ptr, MNOGEN)) {
				print(fd, 
					"Now it cannot follow you, flee from you or block you.\n"); 
			}
			else {
				print(fd, 
					"Now %s cannot follow you, flee from you or block you.\n", 
						F_ISSET(crt_ptr, MMALES) ? "he":"she");
			}
		}
		else if(ply_ptr && ply_ptr->type == PLAYER) {
			print(fd, "You cannot entangle %m.\n", crt_ptr);
		}

	}

	return(PROMPT);
}

/***********************************************************************/
/*								knock   						       */
/***********************************************************************/
/* This function allows an bard to cast the knock spell on a locked    */
/* door.  If the lock is pickable, there is a chance (depending on the */
/* player's level and luck) that the door will be opened.              */

int knock(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room	*rom_ptr;
	exit_	*ext_ptr;
	long	i, t;
	int		how, fd, chance;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->class != BARD && ply_ptr->class < CARETAKER) {
			print(fd, "Only bards may use the knock spell.\n");
			return(0);
	}

	if(!S_ISSET(ply_ptr, SKNOCK) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(cmnd->num == 2) {
		print(fd, "Knock on which door?\n");
		return(0);
	}

	ext_ptr = find_ext(ply_ptr, rom_ptr->first_ext,
		cmnd->str[2], cmnd->val[2]);

	if(!ext_ptr) {
		print(fd, "That doesn't seem to be here.\n");
		return(0);
	}

	if(!F_ISSET(ext_ptr, XLOCKD)) {
		print(fd, "It's not locked.\n");
		return(0);
	}

	if(how == CAST) {
		if(ply_ptr->mpcur < 12) {
			print(fd, "Not enough magic points.\n");
			return(0);
		}
		ply_ptr->mpcur -= 12;
	}

	F_CLR(ply_ptr, PHIDDN);

	if(F_ISSET(ext_ptr, XUNPCK)) {
		print(fd,
			"That lock is protected by a magic spell.  It will not open.\n");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M knocked on a door protected by a magic spell.  "
				"It will not open.\n", ply_ptr);
		return(PROMPT);
	}

	if(!F_ISSET(ply_ptr, PDMINV)) {
		broadcast_rom(fd, ply_ptr->rom_num, "%M knocks on the %s.", 
			ply_ptr, ext_ptr->name);
	}

	chance =  5*ply_ptr->level;
	chance += bonus[ply_ptr->dexterity] * 2 + Ply[ply_ptr->fd].extr->luck/60;

	if(mrand(1, 100) <= chance) {
		print(fd, "The knock spell has opened it!\n"); 
		F_CLR(ext_ptr, XLOCKD);
		if(!F_ISSET(ply_ptr, PDMINV)) {
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M's knock spell has opened it!\n", ply_ptr);
		}
	}
	else {
		print(fd, 
			"The %s glows brightly then returns to normal.  You failed.\n",
				ext_ptr->name);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"The %s glows brightly then returns to normal.  "
				"It remains locked.\n", ext_ptr->name);
	}

	return(PROMPT);
	
}


/*************************************************************************/
/*								dispel      							 */
/*************************************************************************/
/* This function allows players to cast a dispel which resets all other  */
/* spells.                                                               */

int dispel(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(!S_ISSET(ply_ptr, SDISPL) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(ply_ptr->mpcur < 15 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	if(ply_ptr->mpcur < 20 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(!dec_daily(&ply_ptr->daily[DL_DISPL]) && 
		ply_ptr->class < CARETAKER) {
			print(fd, "You can only do that three times in a day.\n");
			return(0);
		}
	
	if(cmnd->num == 2) {
		compute_thaco(ply_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M has cleansed %s body of all magic spells.", 
				ply_ptr, F_ISSET(ply_ptr, PMALES) ? "his": "her");
		if(how == CAST) {
			print(fd, 
				"A charcoal haze envelops you, nullifying your spells.\n");
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 15;
			else
				ply_ptr->mpcur -= 20;
		}
		else {
			if(!dec_daily(&ply_ptr->daily[DL_DISPL]) && 
				ply_ptr->class < CARETAKER) {
					print(fd, 
						"You vomit the potion and nothing else happens.\n");
					return(0);
			}
			print(fd, "A dark haze surrounds you, nullifying your spells.\n");
		}
		F_CLR(ply_ptr, PLIGHT);  /* Light */
		F_CLR(ply_ptr, PBLESS);  /* Bless */
		F_CLR(ply_ptr, PPROTE);  /* Protect */
		F_CLR(ply_ptr, PINVIS);  /* Invisible */
		F_CLR(ply_ptr, PDINVI);  /* Detect Invisible */
		F_CLR(ply_ptr, PDMAGI);  /* Detect magic */
		F_CLR(ply_ptr, PLEVIT);  /* Levitate */
		F_CLR(ply_ptr, PRFIRE);  /* Resist fire */
		F_CLR(ply_ptr, PRCOLD);  /* Resist cold */
		F_CLR(ply_ptr, PRACID);  /* Resist acid */
		F_CLR(ply_ptr, PFLYSP);  /* Fly */
		F_CLR(ply_ptr, PRMAGI);  /* Resist magic */
		F_CLR(ply_ptr, PKNOWA);  /* Know alignment */
		F_CLR(ply_ptr, PBRWAT);  /* Breath water */
		F_CLR(ply_ptr, PSSHLD);  /* Stone shield */
		F_CLR(ply_ptr, PFEARS);  /* Fear */
		F_CLR(ply_ptr, PSILNC);  /* Silence */
		F_CLR(ply_ptr, PSTRNG);  /* Super strength */
		F_CLR(ply_ptr, PSHRNK);  /* Shrink */
		F_CLR(ply_ptr, PRFLCT);  /* Reflect */
		F_CLR(ply_ptr, PEVEYE);  /* Evil eye */
		F_CLR(ply_ptr, PFROZE);  /* Frozen */
		F_CLR(ply_ptr, PHEXED);  /* Hex */
		F_CLR(ply_ptr, PPREPA);  /* Prepared */
		F_CLR(ply_ptr, PNOFOL);  /* No follow */
		F_CLR(ply_ptr, PCHARM);  /* Charmed */
		F_CLR(ply_ptr, PDETLK);  /* Detect luck */
		compute_thaco(ply_ptr);
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
			print(fd, "That person is not here.\n");
			return(0);
		}

		if(!dec_daily(&ply_ptr->daily[DL_DISPL]) && 
			ply_ptr->class < CARETAKER) {
				print(fd, "You are too weak to sustain a dispel.\n");
				return(0);
		}

		compute_thaco(crt_ptr);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
			"%M casts a dispel on %m.", ply_ptr, crt_ptr);
		print(crt_ptr->fd, "%M casts a dispel on you.\n", 
			ply_ptr);

		if(how == CAST) {
			print(fd, "You cast a dispel on %s.\n", crt_ptr);
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 15;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 20;
		}
		F_CLR(crt_ptr, PLIGHT);  /* Light */
		F_CLR(crt_ptr, PBLESS);  /* Bless */
		F_CLR(crt_ptr, PPROTE);  /* Protect */
		F_CLR(crt_ptr, PINVIS);  /* Invisible */
		F_CLR(crt_ptr, PDINVI);  /* Detect Invisible */
		F_CLR(crt_ptr, PDMAGI);  /* Detect magic */
		F_CLR(crt_ptr, PLEVIT);  /* Levitate */
		F_CLR(crt_ptr, PRFIRE);  /* Resist fire */
		F_CLR(crt_ptr, PRCOLD);  /* Resist cold */
		F_CLR(crt_ptr, PRACID);  /* Resist acid */
		F_CLR(crt_ptr, PFLYSP);  /* Fly */
		F_CLR(crt_ptr, PRMAGI);  /* Resist magic */
		F_CLR(crt_ptr, PKNOWA);  /* Know alignment */
		F_CLR(crt_ptr, PBRWAT);  /* Breath water */
		F_CLR(crt_ptr, PSSHLD);  /* Stone shield */
		F_CLR(crt_ptr, PFEARS);  /* Fear */
		F_CLR(crt_ptr, PSILNC);  /* Silence */
		F_CLR(crt_ptr, PSTRNG);  /* Super strength */
		F_CLR(crt_ptr, PSHRNK);  /* Shrink */
		F_CLR(crt_ptr, PRFLCT);  /* Reflect */
		F_CLR(crt_ptr, PEVEYE);  /* Evil eye */
		F_CLR(crt_ptr, PFROZE);  /* Frozen */
		F_CLR(crt_ptr, PHEXED);  /* Hex */
		F_CLR(crt_ptr, PPREPA);  /* Prepared */
		F_CLR(crt_ptr, PNOFOL);  /* No follow */
		F_CLR(crt_ptr, PCHARM);  /* Charmed */
		F_CLR(crt_ptr, PDETLK);  /* Detect luck */
		compute_thaco(crt_ptr);
		return(1);
	}
}

/************************************************************************/
/*								resist_acid								*/
/************************************************************************/
/* This function allows players to cast the resist acid spell.  It will	*/
/* allow the player to resist acid breathed on them by dragons and 		*/
/* other breathers.														*/

int resist_acid(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd			*cmnd;
int			how;

{
	creature	*crt_ptr;
	room		*rom_ptr, *new_rom;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(!S_ISSET(ply_ptr, SRACID) && how == CAST) {
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
		if(how == CAST && ply_ptr->class != MAGE)
			ply_ptr->mpcur -= 12;
			return(0);
        }

	if(cmnd->num == 2) {
		ply_ptr->lasttime[LT_RBRTH].ltime = time(0);
		F_SET(ply_ptr, PRACID);
		broadcast_rom(fd, ply_ptr->rom_num, "%M resists acid.", ply_ptr);

		if(how == CAST) {
			print(fd, "You cast a resist-acid spell.  Your skin toughens.\n");
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 6;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 12;
			ply_ptr->lasttime[LT_RBRTH].interval = MAX(300, 1200 +
				bonus[ply_ptr->intelligence] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
				print(fd,
					"The magic in the air increase the power of your spell.\n");
            	  ply_ptr->lasttime[LT_RBRTH].interval += 900L;
        	}                                
		}
		else {
			print(fd, "You feel your skin toughen.\n");
			ply_ptr->lasttime[LT_RBRTH].interval = 1200L;
		}
		if(F_ISSET(ply_ptr, PRCOLD)) {
        	print(fd, "Your resist-cold spell dissipates.\n");
        	F_CLR(ply_ptr, PRCOLD);
        }
        if(F_ISSET(ply_ptr, PRFIRE)) {
        	print(fd, "Your resist-fire spell dissipates.\n");
        	F_CLR(ply_ptr, PRFIRE);
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
			print(fd, "I don't see that person here.\n");
			return(0);
		}

		F_SET(crt_ptr, PRACID);
		crt_ptr->lasttime[LT_RBRTH].ltime = time(0);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
			"%M casts a resist-acid spell on %m.", ply_ptr, crt_ptr);
		print(crt_ptr->fd, "%M casts resist-acid on you.\n", ply_ptr);
		if(F_ISSET(crt_ptr, PRCOLD)) {
        	print(crt_ptr->fd, "Your resist-cold spell dissipates.\n");
        	F_CLR(crt_ptr, PRCOLD);
        }
        if(F_ISSET(crt_ptr, PRFIRE)) {
        	print(crt_ptr->fd, "Your resist-fire spell dissipates.\n");
        	F_CLR(crt_ptr, PRFIRE);
        }

		if(how == CAST) {
			print(fd, "You cast a resist-acid spell on %s.\n", crt_ptr);
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 6;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 12;
			crt_ptr->lasttime[LT_RBRTH].interval = MAX(300, 1200 +
				bonus[ply_ptr->intelligence] * 600);
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
				print(fd,
					"The magic in the air increase the power of your spell.\n");
            	  crt_ptr->lasttime[LT_RBRTH].interval += 900L;
        	}                                
		}
		else {
			print(fd, "%M resists acid.\n", crt_ptr);
			crt_ptr->lasttime[LT_RBRTH].interval = 1200L;
		}

		return(1);
	}
}
