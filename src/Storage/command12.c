/*
 * 	COMMAND12.C:
 *
 *	(C) Brett Vickers 1992, 1994
 *	(C) Brooke Paul 1995
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>

/***********************************************************************/
/*                              bard_song                             */
/***********************************************************************/

int bard_song(ply_ptr, cmnd)
creature    *ply_ptr;
cmd         *cmnd;

{          
	char	*sp;
    int     c = 0, fd, i, t, heal;
	ctag	*cp;
	ctag	*cp_tmp;

	fd = ply_ptr->fd;
	
	if(ply_ptr->class != BARD && ply_ptr->class < CARETAKER) {
		print(fd, "Only bards have that skill.\n");
		return(PROMPT);
	}

	if(ply_ptr->level < 4 && ply_ptr->class < CARETAKER) {
		print(fd, "You have not practiced enough to do that yet.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PSILNC)) {
		print( fd, "A spell has taken your voice, and you cannot sing.\n");
		return(0);
	}		
	
	i = ply_ptr->lasttime[LT_SINGS].ltime+ply_ptr->lasttime[LT_SINGS].interval;
	t = time(0);
	if(i > t) {
		if((i - t) > 60) 
			print(fd, "Please wait %d:%02d minutes.\n", (i-t)/60L, (i-t)%60L);
		else
			print(fd, "Please wait %d seconds.\n", i - t);
		return(0);
	}
	
	ply_ptr->lasttime[LT_SINGS].ltime = t;
	ply_ptr->lasttime[LT_SINGS].interval = 120L;

	cp = ply_ptr->parent_rom->first_ply;

	print(fd,"Your musical voice rejuvenates everyone in the room.\n");
	heal = mrand(2, 5) + ply_ptr->level;            
	if(F_ISSET(ply_ptr->parent_rom, RPMEXT)) {
		heal += mrand(3, 6);
		print(fd,
			"The magic in the air increase the power of your song.\n");
	}		
	while(cp) {
		cp_tmp = cp->next_tag;
		if(cp->crt->type != MONSTER) {
			if(cp->crt != ply_ptr) {
				print(cp->crt->fd,"%M's song rejuvinates your spirits.\n",
					ply_ptr);
			}
			cp->crt->hpcur += heal;
			cp->crt->hpcur = MIN(cp->crt->hpmax, cp->crt->hpcur);
		}
		cp = cp_tmp;
	}
	return(0);
}

/**********************************************************************/
/*      	       			 Charm				                      */
/**********************************************************************/
int bard_song2(ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;

{
    room        *rom_ptr;
    creature    *crt_ptr;
    int     	fd, n, dur, chance;

   	fd = ply_ptr->fd;
	
	if(ply_ptr->class != BARD && ply_ptr->class < CARETAKER) {
		print(fd, "Only bards have that skill.\n");
		return(0);
	}
    	
	if(F_ISSET(ply_ptr, PSILNC)) {
       	print(fd, 
			"A spell of silence has taken your voice and you cannot sing.\n");
       	return(0);
   	}

	if(cmnd->num < 2) {
        print(fd, "Charm whom?\n");
        return(0);
	}

 	if(!dec_daily(&ply_ptr->daily[DL_CHARM]) && ply_ptr->class < CARETAKER) {
		print (fd, "You have charmed enough today.\n");
		return(0);
	}   	
	rom_ptr = ply_ptr->parent_rom;

	dur =  300 + 
			mrand(1, 30) * 10  + 
			bonus[ply_ptr->constitution] * 30 + 
			ply_ptr->level * 5;

	cmnd->str[1][0] = up(cmnd->str[1][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		cmnd->str[1][0] = low(cmnd->str[1][0]);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
			cmnd->str[1], cmnd->val[1]);
		if(!crt_ptr) {
			print(fd, "That's not here.\n");
			return(0);
		}
	}
	if(F_ISSET(crt_ptr, MUNKIL)) {
		print(fd, "%M cannot be harmed.\n", crt_ptr);
		return(0);
	}

	chance = MIN(90, ((ply_ptr->level)-(crt_ptr->level)) * 20 +
		(4 * bonus[ply_ptr->piety]));

	if(F_ISSET(crt_ptr, MUNDED) || 	F_ISSET(crt_ptr, MPERMT) || 
		chance < mrand(1, 100)) {
			print(fd, "Your charm has no effect on %m.\n", crt_ptr);
			print(fd, "You lose %d experience points.\n", ply_ptr->level);
			ply_ptr->experience -= ply_ptr->level;
			broadcast_rom(fd, "%M sings way off key.\n", ply_ptr);
			if(crt_ptr->type == MONSTER) {
               	add_enm_crt(ply_ptr->name, crt_ptr);
				return(0);
			}
			ANSI(crt_ptr->fd, RED);
			print(crt_ptr->fd, "%M tried to charm you.\n", ply_ptr);
			ANSI(crt_ptr->fd, WHITE);
			return(0);
		}		
	
	if((crt_ptr->type == PLAYER && F_ISSET(crt_ptr, PRMAGI)) ||
		(crt_ptr->type != PLAYER && F_ISSET(crt_ptr, MRMAGI)))
			dur /= 2;
	if(crt_ptr->type == PLAYER && F_ISSET(crt_ptr, PRMAGI))
		print(crt_ptr->fd, 
			"You feel your Resist-Magic spell glow warmly around you.\n"); 

	print(fd, "Your song charms %m.\n", crt_ptr);
	print(fd, "You gain %d experience points.\n", ply_ptr->level);
	ply_ptr->experience += ply_ptr->level;
	broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, "%M sings to %m.",
		ply_ptr, crt_ptr);
	print(crt_ptr->fd, "%M's song charms you.\n", ply_ptr);
	add_charm_crt(crt_ptr, ply_ptr);

    crt_ptr->lasttime[LT_CHRMD].ltime = time(0);
        crt_ptr->lasttime[LT_CHRMD].interval = dur;

    if(crt_ptr->type == PLAYER)
    	F_SET(crt_ptr, PCHARM);
    else 
		F_SET(crt_ptr, MCHARM);

    return(0);
}


/********************************************************************/
/*							meditate							    */
/********************************************************************/
/* This command is for monks.										*/

int meditate(ply_ptr)
creature	*ply_ptr;

{
int		fd, chance;
long	i, t;

	fd = ply_ptr->fd;

	if(ply_ptr->class != MONK && ply_ptr->class < CARETAKER) {
		print(fd, "Only monks may meditate.\n");
		return(0);
	}

	if(ply_ptr->alignment > -100 && ply_ptr->alignment < 100) {
		print(fd, "Neither Alith or Zeth favor a neutral monk.\n");
		print(fd, "You must commit to being good or evil before you "
			"can meditate.\n");
		return(0);
	}

	i = ply_ptr->lasttime[LT_MEDIT].ltime+ply_ptr->lasttime[LT_MEDIT].interval;
        t = time(0);
        if(i > t) {
			if((i - t) > 60)
				print(fd, 
					"Please wait %d:%02d minutes.\n", (i-t)/60L, (i-t)%60L);
			else
				print(fd,"Please wait %d seconds.\n", i - t);

			return(0);
		}

	chance = MIN(70, ply_ptr->level * 10 + bonus[ply_ptr->piety]);

	if(mrand(1, 100) <= chance) {
		if(ply_ptr->alignment > 100)
			print(fd, "Alith rewards you.  ");
		else
			print(fd, "Zeth rewards you.  ");
		print(fd, "You feel at one with the universe.\n");
		broadcast_rom(fd, ply_ptr->rom_num, "%M meditates.", ply_ptr);
		ply_ptr->hpcur += mrand(4, 15) + ply_ptr->level;
		ply_ptr->hpcur = MIN(ply_ptr->hpmax, ply_ptr->hpcur);
		ply_ptr->lasttime[LT_MEDIT].ltime = t;
		ply_ptr->lasttime[LT_MEDIT].interval = 120L;
	}
	else {
		print(fd, "Your spirit is not at peace.\n");
		broadcast_rom(fd, ply_ptr->rom_num, "%M meditates.", ply_ptr);
		ply_ptr->lasttime[LT_MEDIT].ltime = t;
		ply_ptr->lasttime[LT_MEDIT].interval = 5L;
	}

	return(0);
}
