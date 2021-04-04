/*
 * MAGIC3.C:
 *
 *	Additional spell-casting routines.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>


/*************************************************************************/
/*			bless					     	 */
/*************************************************************************/
/* This function allows a player to cast a bless spell on himself or  	 */
/* on another player, reducing the target's thaco by 1. 		 */

int bless(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd		*cmnd;
int		how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		t;
	int		fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	t = time(0);

	if(ply_ptr->mpcur < 10 && how == CAST) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(!S_ISSET(ply_ptr, SBLESS) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(ply_ptr->class != CLERIC && ply_ptr->class != PALADIN && 
		spell_fail(ply_ptr, how)) {
			if(how == CAST)
				ply_ptr->mpcur -= 10;
			return(0);
	}

	/* Cast bless on self */
	if(cmnd->num == 2) {

		F_SET(ply_ptr, PBLESS);
		F_CLR(ply_ptr, PEVEYE);
		compute_thaco(ply_ptr);

		ply_ptr->lasttime[LT_BLESS].ltime = t;
		if(how == CAST) {
			ply_ptr->lasttime[LT_BLESS].interval = MAX(300, 1200 + 
				bonus[ply_ptr->intelligence] * 600);
			if(ply_ptr->class == CLERIC || ply_ptr->class == PALADIN)
				ply_ptr->lasttime[LT_BLESS].interval += 60 * ply_ptr->level;
			ply_ptr->mpcur -= 10;
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
			    print(fd,
					"The magic in the air increase the power of your spell.\n");
			    ply_ptr->lasttime[LT_BLESS].interval += 800L;
			}                                
		}
		else
			ply_ptr->lasttime[LT_BLESS].interval = 1200;
			if(how == CAST || how == SCROLL || how == WAND) {
				print(fd,"A Blessing spell is cast.\nYou feel the power of the Goat.\n");
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M casts a bless spell on %sself.", ply_ptr,
				    	F_ISSET(ply_ptr, PMALES) ? "him":"her");
		}
		else if(how == POTION)
			print(fd, "You feel the blessing of the White Goat.\n");

		return(1);
	}

	/* Cast bless on another player */
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

		F_SET(crt_ptr, PBLESS);
		F_CLR(ply_ptr, PEVEYE);
		compute_thaco(crt_ptr);

		crt_ptr->lasttime[LT_BLESS].ltime = t;
		if(how == CAST) {
			crt_ptr->lasttime[LT_BLESS].interval = MAX(300, 1200 + 
				bonus[ply_ptr->intelligence] * 600);

			if(ply_ptr->class == CLERIC || ply_ptr->class == PALADIN)
				crt_ptr->lasttime[LT_BLESS].interval += 60 * ply_ptr->level;
			ply_ptr->mpcur -= 10;

			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
			    print(fd,
					"The magic in the air increase the power of your spell.\n");
			    crt_ptr->lasttime[LT_BLESS].interval += 800L;
			}                                
		}
		else
			crt_ptr->lasttime[LT_BLESS].interval = 1200;

		if(how == CAST || how == SCROLL || how == WAND) {
			print(fd, "Bless cast on %s.\n", crt_ptr->name);
			print(crt_ptr->fd, "%M casts a bless spell on you.\n%s",
				ply_ptr, "You feel the power of the blessing flow over you.\n");
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts a bless spell on %m.", ply_ptr, crt_ptr);
			return(1);
		}
	}

	return(1);
}

/************************************************************************/
/*				turn				        */
/************************************************************************/
/* This function allows clerics and paladins to turn undead creatures.  */
/* If they succeed then the creature is either disintegrated or harmed  */
/* for approximately half of its hit points.				*/

int turn(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	long		i, t;
	int		chance, m, dmg, fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Turn whom?\n");
		return(0);
	}

	if(ply_ptr->class != CLERIC && ply_ptr->class != PALADIN &&
		ply_ptr->class < CARETAKER) {
			print(fd, "Only clerics and paladins may turn the undead.\n");
			return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		print(fd, "That isn't here.\n");
		return(0);
	}

	if(!F_ISSET(crt_ptr, MUNDED)) {
		print(fd, "You may only turn undead monsters.\n");
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

	i = LT(ply_ptr, LT_TURNS);
	t = time(0);

	if(i > t) {
		please_wait(fd, i - t);
		return(0);
	}

	if(crt_ptr->type != PLAYER && F_ISSET(crt_ptr, MUNKIL)) {
		print(fd, "You cannot harm %s.\n",
			F_ISSET(crt_ptr, MMALES) ? "him":"her");
		return(0);
	}

	add_enm_crt(ply_ptr->name, crt_ptr);

	ply_ptr->lasttime[LT_TURNS].ltime = t;
	ply_ptr->lasttime[LT_ATTCK].ltime = t;
	ply_ptr->lasttime[LT_TURNS].interval = 30L;

	chance = (ply_ptr->level - crt_ptr->level) * 20 +
		 bonus[ply_ptr->piety] * 5 + (ply_ptr->class == PALADIN ? 15:25);
	chance = MIN(chance, 80);

	if(mrand(1,100) > chance) {
		print(fd, "You failed to turn %m.\n", crt_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, "%M failed to turn %m.",
			ply_ptr, crt_ptr);
		return(0);
	}

	if(mrand(1,100) > 90 - bonus[ply_ptr->piety]) {
		print(fd, "You disintegrated %m.\n", crt_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, "%M disintegrated %m.",
			ply_ptr, crt_ptr);
		add_enm_dmg(ply_ptr->name, crt_ptr, crt_ptr->hpcur);
		die(crt_ptr, ply_ptr);
	}
	else {
		dmg = MAX(1, crt_ptr->hpcur/2);
		m = MIN(crt_ptr->hpcur, dmg);
		crt_ptr->hpcur -= dmg;
		add_enm_dmg(ply_ptr->name, crt_ptr, m);
		print(fd, "You turned %m for %d damage.\n", crt_ptr, dmg);
		broadcast_rom(fd, ply_ptr->rom_num, "%M turned %m.", ply_ptr, crt_ptr);
		if(crt_ptr->hpcur < 1) {
			ANSI(fd, YELLOW);
			print(fd, "You killed %m.\n", crt_ptr);
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			broadcast_rom(fd, ply_ptr->rom_num, "%M killed %m.",
				ply_ptr, crt_ptr);
			die(crt_ptr, ply_ptr);
		}
	}

	return(0);
}

/**************************************************************************/
/*				invisibility			      	  */
/**************************************************************************/
/* This function allows a player to cast an invisibility spell on himself */
/* or on another player. 						  */

int invisibility(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd		*cmnd;
int		how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	ctag		*cp;
	long		t;
	int		fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	t = time(0);

	if(ply_ptr->mpcur < 7 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}
	
	if(ply_ptr->mpcur < 15 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(!S_ISSET(ply_ptr, SINVIS) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}
        if(spell_fail(ply_ptr, how)) {
			if(how==CAST && ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 15;
			return(0);
        }

	cp = rom_ptr->first_mon;
	while(cp) {
		if(is_enm_crt(ply_ptr->name, cp->crt)) {
			print(fd, "Not in the middle of combat.\n");
			return(0);
		}
		cp = cp->next_tag;
	}

	/* Cast invisibility on self */
	if(cmnd->num == 2) {
		ply_ptr->lasttime[LT_INVIS].ltime = t;
		if(how == CAST) {
			ply_ptr->lasttime[LT_INVIS].interval = 1200 + 
				bonus[ply_ptr->intelligence] * 600;
			if(ply_ptr->class == MAGE)
				ply_ptr->lasttime[LT_INVIS].interval += 60 * ply_ptr->level;
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 7;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 15;
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
			    print(fd,
					"The magic in the air increase the power of your spell.\n");
			    ply_ptr->lasttime[LT_INVIS].interval += 600L;
			}                                
		}
		else
			ply_ptr->lasttime[LT_INVIS].interval = 1200;

		if(how == CAST || how == SCROLL || how == WAND) {
			ANSI(fd, MAGENTA);
			print(fd,"Invisibility spell cast.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%s cast invisibility on %sself.", ply_ptr->name,
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
		}
		else if(how == POTION) {
			ANSI(fd, MAGENTA);
			print(fd, "You become transparent as the liquid gurgles down your throat.\n");
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
		}
		F_SET(ply_ptr, PINVIS);

		return(1);
	}

	/* Cast invisibility on another player */
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

		crt_ptr->lasttime[LT_INVIS].ltime = t;
		if(how == CAST) {
			crt_ptr->lasttime[LT_INVIS].interval = 1200 + 
				bonus[ply_ptr->intelligence] * 600;
			if(ply_ptr->class == MAGE)
				crt_ptr->lasttime[LT_INVIS].interval += 60 * ply_ptr->level;
			if(ply_ptr->class == MAGE)
				ply_ptr->mpcur -= 7;
			if(ply_ptr->class != MAGE)
				ply_ptr->mpcur -= 15;
			if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
			    print(fd,
					"The magic in the air increase the power of your spell.\n");
			    crt_ptr->lasttime[LT_INVIS].interval += 600L;
			}                                
		}
		else
			crt_ptr->lasttime[LT_INVIS].interval = 1200;

		if(how == CAST || how == SCROLL || how == WAND) {
			print(fd, "Invisibility cast on %m.\n", crt_ptr);
			ANSI(crt_ptr->fd, MAGENTA);
			print(crt_ptr->fd, "%M casts an invisibility spell on you.\n",
				ply_ptr);
			ANSI(crt_ptr->fd, BOLD);
			ANSI(crt_ptr->fd, WHITE);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts an invisibility spell on %m.", ply_ptr, crt_ptr);
			F_SET(crt_ptr, PINVIS);
			return(1);
		}
	}

	return(1);
}

/**************************************************************************/
/*				restore				          */
/**************************************************************************/
/* This function allows a player to cast the restore spell using either   */
/* a potion or a wand.  Restore should not be a cast-able spell because   */
/* it can restore magic points to full.					  */

int restore(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd		*cmnd;
int		how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int		fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(how == CAST && ply_ptr->class < CARETAKER) {
		print(fd, "Only the highest of wizards can cast that spell.\n");
		return(0);
	}

	/* Cast restore on self */
	if(cmnd->num == 2) {
		ply_ptr->hpcur = ply_ptr->hpmax;
		ply_ptr->mpcur = ply_ptr->mpmax;

		if(how == CAST || how == WAND) {
			print(fd, "Restore spell cast.\n");
			broadcast_rom(fd, ply_ptr->rom_num, "%M casts restore on %sself.", 
				ply_ptr, F_ISSET(ply_ptr, PMALES) ? "him":"her");
		}
		else if(how == POTION)
			print(fd, "You are restored to full health.\n");

		return(1);
	}

	/* Cast restore on another player */
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

		crt_ptr->hpcur = crt_ptr->hpmax;
		crt_ptr->mpcur = crt_ptr->mpmax;

		if(how == CAST || how == WAND) {
			print(fd, "Restore spell cast on %m.\n", crt_ptr);
			print(crt_ptr->fd, "%M casts a restore spell on you.\n", ply_ptr);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts a restore spell on %m.", ply_ptr, crt_ptr);
			return(1);
		}
	}

	return(1);
}

/*****************************************************************************/
/*			befuddle					     */
/*****************************************************************************/
/*	This function allows a player to cast befuddle/stun on a monster or  */
/*	other player.  The monster/other player cannot respond for several   */
/*  seconds.							             */
	
int befuddle(ply_ptr, cmnd, how)
creature	*ply_ptr;
cmd		*cmnd;
int		how;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	int		fd, dur;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	luck(ply_ptr); /* luck for stun */
	
	if(!S_ISSET(ply_ptr, SBEFUD) && how == CAST) {
		print(fd, "You don't know that spell.\n");
		return(0);
	}

	if(ply_ptr->mpcur < 5 && how == CAST && ply_ptr->class == MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(ply_ptr->mpcur < 10 && how == CAST && ply_ptr->class != MAGE) {
		print(fd, "Not enough magic points.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PINVIS)) {
		F_CLR(ply_ptr, PINVIS);
		ANSI(fd, MAGENTA);
		print(fd, "You gradually become visible.\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		broadcast_rom(fd, ply_ptr->rom_num, "%M fades into view.", ply_ptr);
	}

	/* Befuddle self */
	if(cmnd->num == 2) {
		if(how == CAST) {
			dur = bonus[ply_ptr->intelligence] * 2 + 
			(Ply[fd].extr->luck/10) + (ply_ptr->class == MAGE ?
				ply_ptr->level/3 : 0);
			if(ply_ptr->class == MAGE) ply_ptr->mpcur -= 5;
			if(ply_ptr->class != MAGE) ply_ptr->mpcur -= 10;
		}
		else
			dur = dice(2, 6, 0);

		dur = MAX(5, dur);
		ply_ptr->lasttime[LT_ATTCK].ltime = time(0);
		ply_ptr->lasttime[LT_ATTCK].interval = dur;

		if(how == CAST || how == SCROLL || how == WAND)  {
			print(fd, "You're stunned and befuddled.\n");
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M casts a stun spell on %sself.", ply_ptr,
					F_ISSET(ply_ptr, PMALES) ? "him":"her");
		}
		else if(how == POTION)
			print(fd, "You feel dizzy and befuddled.\n");
	}

	/* Befuddle a monster or player */
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

		if(crt_ptr->type == PLAYER && ply_ptr->type == PLAYER) {
			if(!F_ISSET(ply_ptr, PCHAOS) && ply_ptr->class < DM) {
				print(fd, "Sorry, you're lawful.\n");
				return(0);
			}

			if(!F_ISSET(crt_ptr, PCHAOS) && ply_ptr->class < DM) {
				print(fd, "Sorry, that person is lawful.\n");
				return(0);
			}
		}
		
		if(how == CAST) {
			dur = bonus[ply_ptr->intelligence] * 2 + dice(2, 6, 0);
			if(ply_ptr->class == MAGE) ply_ptr->mpcur -= 5;
			if(ply_ptr->class != MAGE) ply_ptr->mpcur -= 10;
		}
		else
			dur = dice(3, 5, 0);

		if((crt_ptr->type == PLAYER && F_ISSET(crt_ptr, PRMAGI)) ||
		   (crt_ptr->type != PLAYER && (F_ISSET(crt_ptr, MRMAGI) || 
			F_ISSET(crt_ptr, MRBEFD))))
				dur = 3;
		else
			dur = MAX(8, dur);

		crt_ptr->lasttime[LT_ATTCK].ltime = time(0);
		crt_ptr->lasttime[LT_ATTCK].interval = dur;
		crt_ptr->lasttime[LT_SPELL].ltime = time(0);
		dur = MIN(10, dur);
		crt_ptr->lasttime[LT_SPELL].interval = dur;

		if(how == CAST || how == SCROLL || how == WAND) {
			print(fd, "Stun and befuddlement cast on %s.\n", crt_ptr->name);
			broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num,
				"%M casts stun on %m.", ply_ptr, crt_ptr);
			print(crt_ptr->fd, 
				"You are befuddled.  %M stunned you.\n", ply_ptr);
		}

		if(crt_ptr->type != PLAYER)
			add_enm_crt(ply_ptr->name, crt_ptr);
	}
	return(1);
}
