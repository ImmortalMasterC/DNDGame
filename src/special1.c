/*
 * SPECIAL1.C:
 *
 *	User routines dealing with special routines.
 *
 *	Copyright (C) 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"

int	combo_box();

/************************************************************************/
/*							special_obj									*/
/************************************************************************/

#define SP_INVENTORY 0
#define SP_EQUIPMENT 1
#define SP_ROOM      2

int special_obj(ply_ptr, cmnd, special)
creature 	*ply_ptr;
cmd			*cmnd;
int			special;

{
	object	*obj_ptr;
	room	*rom_ptr;
	int		fd, n, where;
	char	str[80], str2[160], match = 0, i;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
	   cmnd->str[1], cmnd->val[1]);
	where = SP_INVENTORY;

	if(!obj_ptr || !cmnd->val[1]) {
		for(n = 0; n < MAXWEAR; n++) {
			if(!ply_ptr->ready[n]) continue;
			if(EQUAL(ply_ptr->ready[n], cmnd->str[1]))
				match++;
			else continue;
			if(match == cmnd->val[1] || !cmnd->val[1]) {
				obj_ptr = ply_ptr->ready[n];
				where = SP_EQUIPMENT;
				break;
			}
		}
	}

	if(!obj_ptr) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
		   cmnd->str[1], cmnd->val[1]);
		where = SP_ROOM;
	}

	if(!obj_ptr) {
		print(fd, "You don't see that.\n");
		return(-1);
	}

	if(obj_ptr->special != special)
		return(-2);

	switch(obj_ptr->special) {
		case SP_MAPSC:
			strcpy(str, obj_ptr->name);
			for(i = 0; i<strlen(str); i++)
				if(str[i] == ' ') str[i] = '_';
			sprintf(str2, "%s/%s", OBJPATH, str);
			view_file(fd, 1, str2);
			return(DOPROMPT);

		case SP_COMBO:
			return(combo_box(ply_ptr, obj_ptr));

		default:
			print(fd, "Nothing.\n");
	}

	return(0);
}

/************************************************************************/
/*								special_cmd								*/
/************************************************************************/

int special_cmd(ply_ptr, special, cmnd)
creature	*ply_ptr;
int			special;
cmd			*cmnd;

{
	int n, fd;

	fd = ply_ptr->fd;

	switch(special) {
		case SP_MAPSC:
		case SP_COMBO:
			if(cmnd->num < 2) {
				print(fd, "What?  Push who?  Push what?  Which one?  Where?\n");
				return(0);
			}
			n = special_obj(ply_ptr, cmnd, special);
			if(n == -1) 
				return(0);
			if(n == -2) {
				print(fd, "What are you mumbling about?\n");
				return(0);
			}
			else return(n);

		default:
			print(ply_ptr->fd, "Nothing happens.  Everything is quiet.\n");
			return(0);
	}
}

/************************************************************************/
/*							combo_box									*/
/************************************************************************/

int combo_box(ply_ptr, obj_ptr)
creature	*ply_ptr;
object		*obj_ptr;

{
	xtag	*xp;
	room	*rom_ptr, *new_rom;
	char	str[80];
	int		fd, dmg, i;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	str[0] = obj_ptr->sdice + '0'; 
	str[1] = 0;

	if(obj_ptr->ndice == 1 || strlen(Ply[fd].extr->tempstr[3]) > 70)
		strcpy(Ply[fd].extr->tempstr[3], str);
	else
		strcat(Ply[fd].extr->tempstr[3], str);
 
	print(fd, "Click.\n");
	broadcast_rom(fd, ply_ptr->rom_num, "Click.  %M presses something.", 
		ply_ptr);

	if(strlen(Ply[fd].extr->tempstr[3]) >= strlen(obj_ptr->use_output)) {
		if(strcmp(Ply[fd].extr->tempstr[3], obj_ptr->use_output)) {
			Ply[fd].extr->tempstr[3][0] = 0;
			if(ply_ptr->class == BARBARIAN) 
				print(fd, "You're a wimp.  Try again.\n");
			else {
				dmg = mrand(20, 40);
				ply_ptr->hpcur -= dmg;
				print(fd, "You were zapped for %d damage!\n", dmg);
				broadcast_rom(fd, ply_ptr->rom_num,
					"%M was zapped by %i!", ply_ptr, obj_ptr);
				if(ply_ptr->hpcur < 1) {
					ply_ptr->hpcur = 0;
					load_rom(1, &new_rom);
					del_ply_rom(ply_ptr, rom_ptr);
					add_ply_rom(ply_ptr, new_rom);
					print(fd, "You were vaporized.\n");
					broadcast_rom(fd, ply_ptr->rom_num,
						"%M disappeared in a swirling vapor!", ply_ptr);
					ply_ptr->experience -= 10 * ply_ptr->level;
					if(ply_ptr->experience < 0)
						ply_ptr->experience = 0;
				}
			}
		}
		else {
			for(i = 1, xp = rom_ptr->first_ext; xp && i < obj_ptr->pdice;
				i++, xp = xp->next_tag);
			if(!xp) 
				return(0);
			ANSI(fd, YELLOW);
			print(fd, "You opened the %s!\n", xp->ext->name);
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
			broadcast_rom(fd, ply_ptr->rom_num, "%M opened the %s!", 
				ply_ptr, xp->ext->name);
			F_CLR(xp->ext, XLOCKD);
			F_CLR(xp->ext, XCLOSD);
			if(F_ISSET(xp->ext, XNPRNT) && F_ISSET(xp->ext, XNOSEE)) {
				F_CLR(xp->ext, XNPRNT);
				F_CLR(xp->ext, XNOSEE);
				F_SET(xp->ext, XTPRNT);
			}
			xp->ext->ltime.ltime = time(0);
		}
	}
	return(0);
}
