/*
 * COMMAND3.C:
 *
 *	Additional user routines.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>


/**********************************************************************/
/*				wear				      */
/**********************************************************************/
/* This function allows the player pointed to by the first parameter  */
/* to wear an item specified in the pointer to the command structure  */
/* in the second parameter.  That is, if the item is wearable.        */

int wear(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room	*rom_ptr;
	object	*obj_ptr;
	int	fd, i, cantwear = 0, allfingers = 0;

	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Wear what?\n");
		return(0);
	}

	rom_ptr = ply_ptr->parent_rom;
	F_CLR(ply_ptr, PHIDDN);

	if(cmnd->num > 1) {
		if(!strcmp(cmnd->str[1], "all")) {
			wear_all(ply_ptr);
			return(0);
		}

		obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
				   cmnd->str[1], cmnd->val[1]);

		if(!obj_ptr) {
			print(fd, "You don't have that.\n");
			return(0);
		}

		if(!obj_ptr->wearflag || obj_ptr->wearflag == WIELD ||
			obj_ptr->wearflag == HELD) {
				print(fd, "You can't wear that.\n");
				print(fd, "Maybe you can hold it or use it.\n");
				return(0);
		}

		if(obj_ptr->type == ARMOR && F_ISSET(obj_ptr, ONOMAG) &&
			ply_ptr->class == MAGE) {
				print(fd, "Mages may not wear or hold that.\n");
				print(fd, "Is it made of iron?  If so, it will "
							"destroy your magic.\n");
				return(0);
		}

		if(obj_ptr->type == ARMOR && F_ISSET(obj_ptr, ONOFEM) &&
			!F_ISSET(ply_ptr, PMALES)) {
				print(fd, "Only males may wear that.\n");
				return(0);
		}

		if(obj_ptr->type == ARMOR && F_ISSET(obj_ptr, ONOMAL) &&
			F_ISSET(ply_ptr, PMALES)) {
				print(fd, "Only females may wear that.\n");
				return(0);
		}

		if(obj_ptr->wearflag == NECK && ply_ptr->ready[NECK1 - 1] && 
			ply_ptr->ready[NECK2 - 1]) {
				print(fd, "You can't wear anymore on your neck.\n");
				return(0);
		}

		if(obj_ptr->wearflag == NECK && F_ISSET(obj_ptr, OCLOAK) &&
			(ply_ptr->ready[NECK1 - 1] || ply_ptr->ready[NECK2 - 1])) { 
				print(fd, "You already have something around your neck.\n");
				return(0);
		}

		if(obj_ptr->wearflag == FINGER && (obj_ptr->armor > 1) &&
			(ply_ptr->ready[FINGER1 - 1] || ply_ptr->ready[FINGER2 - 1] || 
			 ply_ptr->ready[FINGER3 - 1] || ply_ptr->ready[FINGER4 - 1] || 
			 ply_ptr->ready[FINGER5 - 1] || ply_ptr->ready[FINGER6 - 1] || 
			 ply_ptr->ready[FINGER7 - 1] || ply_ptr->ready[FINGER8 - 1])) {
				print(fd, "You can wear the %s only if you remove all others "
					"rings.\n", obj_ptr);
				return(0);
		}

		if(obj_ptr->wearflag == FINGER && (ply_ptr->ready[FINGER1 - 1])) {
			if(ply_ptr->ready[FINGER1 - 1]->armor > 1) {
				print(fd, "You are already wearing a very protective ring.\n");
				return(0);
			}
		}
		
		if(ply_ptr->ready[FINGER1 - 1] && ply_ptr->ready[FINGER2 - 1] && 
		   ply_ptr->ready[FINGER3 - 1] && ply_ptr->ready[FINGER4 - 1] && 
		   ply_ptr->ready[FINGER5 - 1] && ply_ptr->ready[FINGER6 - 1] && 
		   ply_ptr->ready[FINGER7 - 1] && ply_ptr->ready[FINGER8 - 1]) {
				allfingers = 1;
		}

		if(obj_ptr->wearflag == FINGER && allfingers) {
				print(fd, "You don't have any more fingers left.\n");
				return(0);
		}

		if(obj_ptr->wearflag != NECK && obj_ptr->wearflag != FINGER &&
			ply_ptr->ready[obj_ptr->wearflag - 1]) {
				print(fd, "You're already wearing something there.\n");
				return(0);
		}

		if(obj_ptr->shotscur < 1 && !F_ISSET(obj_ptr, OCONTN)) {
			if(!F_ISSET(obj_ptr, OCLOAK)) {
				print(fd, "You can't.  It's no longer useable.\n");
				return(0);
			}
			else {
				print(fd, "You can't.  Its powers are lost.\n");
				return(0);
			}
		}

		if(F_ISSET(obj_ptr, OGOODO) && ply_ptr->alignment < -50) {
			print(fd, "%I is good and jumps from your hand.\n", obj_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M is evil and is shocked by %i.", ply_ptr, obj_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
			add_obj_rom(obj_ptr, ply_ptr->parent_rom);
			return(0);
		}

		if(F_ISSET(obj_ptr, OEVILO) && ply_ptr->alignment > 50) {
			if(F_ISSET(obj_ptr, OSOMEA)) {
				print(fd, "%I are evil and shock you. You drop them.\n", 
					obj_ptr);
			}
			else {
				print(fd, "%I is evil and shocks you. You drop it.\n", obj_ptr);
			}
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M is good and is shocked by %i.", ply_ptr, obj_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
			add_obj_rom(obj_ptr, ply_ptr->parent_rom);
			return(0);
		}

		if(F_ISSET(obj_ptr, OPLDGK) && 
			(BOOL(F_ISSET(obj_ptr, OKNGDM)) 
			!= BOOL(F_ISSET(ply_ptr, PKNGDM)))) {
        	print(fd, "Your pledge prevents you from wearing %i.\n", obj_ptr);
        	print(fd, "Do you not see the hated insignia of Prince %s?\n",
				F_ISSET(ply_ptr, PKNGDM) ? "Ragnar":"Rutger");
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M tried to wear a %s belonging to the kingdom of Prince %s.", 
					ply_ptr, obj_ptr, F_ISSET(ply_ptr, PKNGDM) ? 
						"Ragnar":"Rutger");
        	return(0);
    	}              

      	if(F_ISSET(obj_ptr, OCLSEL))
			if(!F_ISSET(obj_ptr, (OCLSEL + ply_ptr->class)) && 
				(ply_ptr->class < CARETAKER)) {
        			print(fd, "Sorry, %i cannot be worn by your class.\n",
						obj_ptr);
        			return(0);
		}

/*
		if(!F_ISSET(obj_ptr, (OCLSEL + ply_ptr->class)) && 
			obj_ptr->type > 4 && 
			(ply_ptr->class == MONK || 
			ply_ptr->class == MAGE)) {
               	print(fd, 
					"Your class prevents you from wearing %i.\n", obj_ptr);
               	print(fd, 
					"Your magicke is affected by the presence of iron.\n");
                return(0);
        }
*/

		if((obj_ptr->wearflag == FINGER||obj_ptr->wearflag == SHIELD) && 
			ply_ptr->class == MONK) {
                print(fd, "Sorry, %i may not be worn by Monks.\n", obj_ptr);
				print(fd, "Your hands must be always be free.\n");
                return(0);
        }

		i = (F_ISSET(obj_ptr, OSIZE1) ? 1:0) * 2 
			+
			(F_ISSET(obj_ptr, OSIZE2) ? 1:0);

		switch(i) {
		case 1:
			if(ply_ptr->race != GNOME &&
			   ply_ptr->race != HOBBIT &&
			   ply_ptr->race != DWARF) cantwear = 1;
			break;

		case 2:
			if(ply_ptr->race != HUMAN &&
			   ply_ptr->race != ELF &&
			   ply_ptr->race != HALFELF &&
			   ply_ptr->race != DARKELF &&
			   ply_ptr->race != HALFORC &&
			   ply_ptr->race != SATYR &&
			   ply_ptr->race != ORC) cantwear = 1;
			break;

		case 3:
			if(ply_ptr->race != HALFGIANT && 
			   ply_ptr->race != OGRE && 
			   ply_ptr->race != CENTAUR) cantwear = 1;
			break;
		}

		if(cantwear && ply_ptr->class < CARETAKER) {
			print(fd, "%I %s fit you.\n", obj_ptr, 
				(F_ISSET(obj_ptr, ODROPS) ||
				(F_ISSET(obj_ptr, OSOMEA)) ? "don't" : "doesn't"));
			return(0);
		}

		del_obj_crt(obj_ptr, ply_ptr);
		
		switch(obj_ptr->wearflag) {
			case BODY:
			case ARMS:
			case LEGS:
			case HANDS:
			case HEAD:
			case FEET:
			case FACE:
			case HELD:
			case SHIELD:
				ply_ptr->ready[obj_ptr->wearflag - 1] = obj_ptr;
				break;
			case NECK:
				if(ply_ptr->ready[NECK1 - 1])
					ply_ptr->ready[NECK2 - 1] = obj_ptr;
				else
					ply_ptr->ready[NECK1 - 1] = obj_ptr;
				break;
			case FINGER:
				for(i = FINGER1; i < FINGER8 + 1; i++) {
					if(!ply_ptr->ready[i - 1]) {
						ply_ptr->ready[i - 1] = obj_ptr;
						break;
					}
				}
				break;
		}
	}

	compute_ac(ply_ptr);
	compute_agility(ply_ptr);

	if(F_ISSET(ply_ptr, PCLOAK)) {
		print(fd, "You cannot do that while you are cloaked.\n");
		return(0);
	}

	print(fd, "You wear %1i.\n", obj_ptr);
	broadcast_rom(fd, rom_ptr->rom_num, "%M wore %1i.", ply_ptr, obj_ptr);
	if(obj_ptr->use_output[0])
		print(fd, "%s\n", obj_ptr->use_output);

	F_SET(obj_ptr, OWEARS);
	if(F_ISSET(obj_ptr, OCURSE))
		F_SET(obj_ptr, OCURSW);
	if(F_ISSET(obj_ptr, OINVIS)) {
		if(F_ISSET(obj_ptr, ODROPS) || F_ISSET(obj_ptr, OSOMEA)) {
			ANSI(fd, MAGENTA);
			print(fd, "The %s emerge from invisibility.\n", obj_ptr);
		}
		else {
			ANSI(fd, MAGENTA);
			print(fd, "The %s emerges from invisibility.\n", obj_ptr);
		}
		F_CLR(obj_ptr, OINVIS);
		ANSI(fd, WHITE);
	}

	if(F_ISSET(obj_ptr, OCLOAK)) {
			F_SET(ply_ptr, PCLOAK);
			print(fd, "The shadows conceal you as you don the %s.\n", 
				obj_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M is lost in the shadows.", 
				ply_ptr);
			obj_ptr->shotscur--;
			if(obj_ptr->shotscur <= 0) {
				obj_ptr->shotscur = 0;
			}
		}

	compute_ac(ply_ptr);
	compute_agility(ply_ptr);
	compute_thaco(ply_ptr);
	return(0);
}

/**********************************************************************/
/*			wear_all				      */
/**********************************************************************/
/* This function allows a player to wear everything in his inventory  */
/* that he possibly can.					  */

void wear_all(ply_ptr)
creature	*ply_ptr;

{
	object	*obj_ptr;
	otag	*op, *temp;
	char	str[2048], str2[85];
	int	fd, i, found = 0, cantwear = 0;

	str[0] = 0;
	fd = ply_ptr->fd;

	op = ply_ptr->first_obj;

	if(F_ISSET(ply_ptr, PCLOAK)) {
		print(fd, "You cannot do that while you are cloaked.\n");
		return;
	}

	while(op) {
		temp = op->next_tag;
		if((F_ISSET(ply_ptr, PDINVI) ?  1:!F_ISSET(op->obj, OINVIS)) && 
			op->obj->wearflag && op->obj->wearflag != HELD && 
			op->obj->wearflag != WIELD) {

			obj_ptr = op->obj;

			if(obj_ptr->type == ARMOR && F_ISSET(obj_ptr, ONOMAG) &&
	   			ply_ptr->class == MAGE) {
					op = temp;
					continue;
			}

			if(obj_ptr->type == ARMOR && F_ISSET(obj_ptr, ONOFEM) &&
   				!F_ISSET(ply_ptr, PMALES)) {
					op = temp;
					continue;
			}

			if(obj_ptr->type == ARMOR && F_ISSET(obj_ptr, ONOMAL) &&
	   			F_ISSET(ply_ptr, PMALES)) {
					op = temp;
					continue;
			}


			if(obj_ptr->wearflag == NECK && ply_ptr->ready[NECK1 - 1] 
				&& ply_ptr->ready[NECK2 - 1]) {
					op = temp;
					continue;
			}

			if(obj_ptr->wearflag == NECK && F_ISSET(obj_ptr, OCLOAK) &&
            (ply_ptr->ready[NECK1 - 1] || ply_ptr->ready[NECK2 - 1])) {
					op = temp;
					continue;
			}

			if(obj_ptr->wearflag == FINGER && (obj_ptr->armor > 0)) {
					op = temp;
					continue;
			}

			if(obj_ptr->wearflag == FINGER &&
			   ply_ptr->ready[FINGER1 - 1] &&
			   ply_ptr->ready[FINGER2 - 1] && 
			   ply_ptr->ready[FINGER3 - 1] &&
			   ply_ptr->ready[FINGER4 - 1] && 
			   ply_ptr->ready[FINGER5 - 1] && 
			   ply_ptr->ready[FINGER6 - 1] && 
			   ply_ptr->ready[FINGER7 - 1] &&
			   ply_ptr->ready[FINGER8 - 1]) {
					op = temp;
					continue;
			}

			if(obj_ptr->wearflag != NECK && obj_ptr->wearflag != FINGER &&
   				ply_ptr->ready[obj_ptr->wearflag - 1]) {
					op = temp;
					continue;
			}
	
			if(obj_ptr->shotscur < 1) {
				op = temp;
				continue;
			}

			if(F_ISSET(obj_ptr, OPLDGK) && 
				(BOOL(F_ISSET(obj_ptr, OKNGDM)) != 
				BOOL(F_ISSET(ply_ptr, PKNGDM)))) {
					op = temp;
					continue;
	   		}              

      		if(F_ISSET(obj_ptr, OCLSEL))
				if(!F_ISSET(obj_ptr, (OCLSEL + ply_ptr->class)) && 
					(ply_ptr->class < CARETAKER)) {
						op = temp;
						continue;
				}

		/*			if(!F_ISSET(obj_ptr, (OCLSEL + ply_ptr->class)) && 
						(ply_ptr->class== MONK || ply_ptr->class == MAGE) && 
		 					obj_ptr->armor > 5) {
                                op = temp;
                                continue;
        			} */

			if((obj_ptr->wearflag == FINGER || 
				obj_ptr->wearflag == SHIELD) && 
				ply_ptr->class == MONK) {
					op = temp;
					continue;
			}

			if(F_ISSET(obj_ptr, OGOODO) && ply_ptr->alignment < -50) {
				op = temp;
				continue;
			}

			if(F_ISSET(obj_ptr, OEVILO) && ply_ptr->alignment > 50) {
				op = temp;
				continue;
			}

			i = (F_ISSET(obj_ptr, OSIZE1) ? 1:0) * 2 
				+ (F_ISSET(obj_ptr, OSIZE2) ? 1:0);

			switch(i) {
				case 1:
					if(ply_ptr->race != GNOME &&
		   				ply_ptr->race != HOBBIT &&
		   				ply_ptr->race != DWARF) cantwear = 1;
						break;

				case 2:
					if(ply_ptr->race != HUMAN &&
						ply_ptr->race != ELF &&
						ply_ptr->race != DARKELF &&
						ply_ptr->race != HALFELF &&
					    ply_ptr->race != DARKELF &&
						ply_ptr->race != HALFORC &&
						ply_ptr->race != SATYR &&
						ply_ptr->race != ORC) cantwear = 1;
						break;

				case 3:
					if(ply_ptr->race != CENTAUR && 
						ply_ptr->race != HALFGIANT && 
						ply_ptr->race != OGRE) cantwear = 1;
						break;
			}

			if(cantwear) {
				op = temp;
				cantwear = 0;
				continue;
			}

			if(obj_ptr->wearflag == NECK) {
				if(ply_ptr->ready[NECK1 - 1])
					ply_ptr->ready[NECK2 - 1] = obj_ptr;
				else
					ply_ptr->ready[NECK1 - 1] = obj_ptr;
				F_SET(obj_ptr, OWEARS);
			}
			else if(obj_ptr->wearflag == FINGER && ply_ptr->class != MONK) {
				for(i = FINGER1; i < FINGER8 + 1; i++) {
					if(!ply_ptr->ready[i - 1]) {
						ply_ptr->ready[i - 1] = obj_ptr;
						F_SET(obj_ptr, OWEARS);
						break;
					}
				}
			}
			else {
				ply_ptr->ready[obj_ptr->wearflag - 1] = obj_ptr;
				if(obj_ptr->use_output[0])
					print(fd, "%s\n", obj_ptr->use_output);
				F_SET(obj_ptr, OWEARS);
			}

			sprintf(str2, "%s, ", obj_str(obj_ptr, 1, 0));
			strcat(str, str2);
			del_obj_crt(obj_ptr, ply_ptr);
			if(F_ISSET(obj_ptr, OCURSE))
	           	F_SET(obj_ptr, OCURSW);
			found = 1;
		}
		op = temp;
	}

	if(found) {
		compute_ac(ply_ptr);
		compute_agility(ply_ptr);
		str[strlen(str) - 2] = 0;
		print(fd, "You wear %s.\n", str);
		broadcast_rom(fd, ply_ptr->rom_num, "%M wears %s.", ply_ptr, str);
		if(F_ISSET(obj_ptr, OCLOAK)) {
			F_SET(ply_ptr, PCLOAK);
			obj_ptr->shotscur--;
			if(obj_ptr->shotscur <= 0) {
				obj_ptr->shotscur = 0;
			}
			print(fd, "The shadows conceal you as you don the %s.\n", obj_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M fades into the shadows.", ply_ptr);
		}
	}
	else {
		print(fd, "You have nothing you can wear.\n");
	}

	compute_agility(ply_ptr);
	compute_thaco(ply_ptr);
}

/************************************************************************/
/*			remove					        */
/************************************************************************/
/* This function allows the player pointed to by the first parameter to */
/* remove the item specified by the command structure in the second     */
/* parameter from those things which he is wearing.			*/

int remove_obj(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room	*rom_ptr;
	object	*obj_ptr;
	otag	*op;
	int	i, fd, found = 0, match = 0;

	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Remove what?\n");
		return(0);
	}

	rom_ptr = ply_ptr->parent_rom;
	F_CLR(ply_ptr, PHIDDN);

	i = 0;
	if(cmnd->num > 1) {
		if(!strcmp(cmnd->str[1], "all")) {
			remove_all(ply_ptr);
			return(0);
		}

		while(i < 20) {
			obj_ptr = ply_ptr->ready[i];
			if(!obj_ptr) {
				i++;
				continue;
			}
			if(EQUAL(obj_ptr, cmnd->str[1]) && (F_ISSET(ply_ptr, PDINVI) ?
			   1:!F_ISSET(obj_ptr, OINVIS))) {
					match++;
					if(match == cmnd->val[1]) {
						found = 1;
						break;
					}
			}
			i++;
		}

		if(!found) {
			print(fd, "You aren't using that.\n");
			return(0);
		}

		if(F_ISSET(obj_ptr, OCURSE)) {
			if(obj_ptr->shotscur > 1) {
				ANSI(fd, RED);
				print(fd, "You can't.  It's cursed!\n");
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
				return(0);
			}
			else {
				ANSI(fd, RED);
				print(fd, "It's unusable, and its curse is removed.\n");
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
				return(0);
			}
				
		}
		

		if(F_ISSET(obj_ptr, OLUCKY)) {
			Ply[ply_ptr->fd].extr->luck -= ply_ptr->ready[HELD - 1]->ndice;
			if(obj_ptr->shotscur > 0)
				print(fd, "Your luck wanes as you remove the %s.\n", 
					obj_ptr);
			else
				print(fd, "The luck in the %s is used up.\n", obj_ptr);
		}

		add_obj_crt(obj_ptr, ply_ptr);
		ply_ptr->ready[i] = 0;
		compute_ac(ply_ptr);
		compute_thaco(ply_ptr);
		compute_agility(ply_ptr);

		print(fd, "You removed %1i.\n", obj_ptr);
		broadcast_rom(fd, rom_ptr->rom_num, "%M removed %1i.",
			ply_ptr, obj_ptr);
		F_CLR(obj_ptr, OWEARS);
	}

	if(F_ISSET(obj_ptr, OCLOAK)) {
		F_CLR(ply_ptr, PCLOAK);
		F_CLR(ply_ptr, PHIDDN);
		F_CLR(obj_ptr, OWEARS);
		print(fd, "Shadows fall away as you remove the %s.\n", obj_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, "Shadows fall away from %m.", 
			ply_ptr);
		if(obj_ptr->shotscur <= 0) {
			obj_ptr->shotscur = 0;
			print(fd,
				"The %s is worn out and turns to dust!\n", obj_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
    		free_obj(obj_ptr);
		}
	}

	compute_agility(ply_ptr);
	compute_thaco(ply_ptr);

	return(0);
}

/**********************************************************************/
/*			remove_all				      */
/**********************************************************************/
/* This function allows the player pointed to in the first parameter  */
/* to remove everything he is wearing all at once.     		  */

void remove_all(ply_ptr)
creature	*ply_ptr;

{
	char	str[2048], str2[85];
	object	*obj_ptr;
	int	fd, i, found = 0;

	fd = ply_ptr->fd;
	str[0] = 0;

	for(i = 0; i < MAXWEAR; i++) {
		if(ply_ptr->ready[i] && F_ISSET(ply_ptr->ready[i], OLUCKY)) {
			Ply[fd].extr->luck -= ply_ptr->ready[i]->ndice;
		}
			
		if(ply_ptr->ready[i] && F_ISSET(ply_ptr->ready[i], OCLOAK)) {
			F_CLR(ply_ptr, PCLOAK);
			F_CLR(ply_ptr, PHIDDN);
			print(fd, "Shadows fall away as you remove the %s.\n", ply_ptr->ready[i] );
 			if(ply_ptr->ready[i]->shotscur <= 0) {
            	print(fd,
                	"The %s turns a dull gray in color.\n", ply_ptr->ready[i]);
            	broadcast_rom(fd, ply_ptr->rom_num,
                	"The %s removed by %m turns a dull gray in color.\n", 
						ply_ptr->ready[i], ply_ptr);
        	}
		}

		if(ply_ptr->ready[i] && !F_ISSET(ply_ptr->ready[i], OCURSE)) {
			sprintf(str2,"%s, ", obj_str(ply_ptr->ready[i], 1, 0));
			strcat(str, str2);
			F_CLR(ply_ptr->ready[i], OWEARS);
			add_obj_crt(ply_ptr->ready[i], ply_ptr);
			ply_ptr->ready[i] = 0;
			found = 1;
		}
	}

	if(!found) {
		print(fd,"You aren't wearing anything that can be removed.\n");
		return;
	}

	compute_ac(ply_ptr);
	compute_thaco(ply_ptr);
	compute_agility(ply_ptr);
	compute_thaco(ply_ptr);

	str[strlen(str) - 2] = 0;
	broadcast_rom(fd, ply_ptr->rom_num, "%M removes %s.", ply_ptr, str);
	print(fd, "You remove %s.\n", str);

	return;
}

/*********************************************************************/
/*			equipment	 			     */
/*********************************************************************/
/* This function outputs to the player all of the equipment that he  */
/* is wearing/wielding/holding on his body.    			 */

int equipment(crt_ptr, cmnd)
creature	*crt_ptr;
cmd		*cmnd;

{
	int	fd = crt_ptr->fd;
	int	i, found = 0;

	for(i = 0; i < MAXWEAR; i++)
		if(crt_ptr->ready[i])
			found = 1;

	if(!found) {
		print(fd, "You aren't wearing anything.\n");
		return(0);
	}
	if(F_ISSET(crt_ptr, PBLIND)) {
		ANSI(fd, RED);
		print(fd, "You can't see anything...You're Blind!\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		return(0);
	}	
	equip_list(fd, crt_ptr);
	return(0);
}

/**********************************************************************/
/*			equip_list				      */
/**********************************************************************/
void equip_list(fd, crt_ptr)
int			fd;
creature	*crt_ptr;

{
int		wgt = 0;

	if(crt_ptr->ready[BODY - 1]) {
		print(fd, "On body:   %1i\n", crt_ptr->ready[BODY - 1]);
	}
	if(crt_ptr->ready[ARMS - 1]) {
		print(fd, "On arms:   %1i\n", crt_ptr->ready[ARMS - 1]);
	}
	if(crt_ptr->ready[LEGS - 1]) {
		print(fd, "On legs:   %1i\n", crt_ptr->ready[LEGS - 1]);
	}
	if(crt_ptr->ready[NECK1 - 1]) {
		print(fd, "On neck:   %1i\n", crt_ptr->ready[NECK1 - 1]);
	}
	if(crt_ptr->ready[NECK2 - 1]) {
		print(fd, "On neck:   %1i\n", crt_ptr->ready[NECK2 - 1]);
	}
	if(crt_ptr->ready[HANDS - 1]) {
		print(fd, "On hands:  %1i\n", crt_ptr->ready[HANDS - 1]);
	}
	if(crt_ptr->ready[HEAD - 1]) {
		print(fd, "On head:   %1i\n", crt_ptr->ready[HEAD - 1]);
	}
	if(crt_ptr->ready[FEET - 1]) {
		print(fd, "On feet:   %1i\n", crt_ptr->ready[FEET - 1]);
	}
	if(crt_ptr->ready[FACE - 1]) {
		print(fd, "On face:   %1i\n", crt_ptr->ready[FACE - 1]);
	}
	if(crt_ptr->ready[FINGER1 - 1]) {
		print(fd, "On finger: %1i\n", crt_ptr->ready[FINGER1 - 1]);
	}
	if(crt_ptr->ready[FINGER2 - 1]) {
		print(fd, "On finger: %1i\n", crt_ptr->ready[FINGER2 - 1]);
	}
	if(crt_ptr->ready[FINGER3 - 1]) {
		print(fd, "On finger: %1i\n", crt_ptr->ready[FINGER3 - 1]);
	}
	if(crt_ptr->ready[FINGER4 - 1]) {
		print(fd, "On finger: %1i\n", crt_ptr->ready[FINGER4 - 1]);
	}
	if(crt_ptr->ready[FINGER5 - 1]) {
		print(fd, "On finger: %1i\n", crt_ptr->ready[FINGER5 - 1]);
	}
	if(crt_ptr->ready[FINGER6 - 1]) {
		print(fd, "On finger: %1i\n", crt_ptr->ready[FINGER6 - 1]);
	}
	if(crt_ptr->ready[FINGER7 - 1]) {
		print(fd, "On finger: %1i\n", crt_ptr->ready[FINGER7 - 1]);
	}
	if(crt_ptr->ready[FINGER8 - 1]) {
		print(fd, "On finger: %1i\n", crt_ptr->ready[FINGER8 - 1]);
	}
	if(crt_ptr->ready[HELD - 1]) {
		print(fd, "Holding:   %1i\n", crt_ptr->ready[HELD - 1]);
	}
	if(crt_ptr->ready[SHIELD - 1]) {
		print(fd, "Shield:    %1i\n", crt_ptr->ready[SHIELD - 1]);
	}
	if(crt_ptr->ready[WIELD - 1]) {
		print(fd, "Wielded:   %1i\n", crt_ptr->ready[WIELD - 1]);
	}
}

/************************************************************************/
/*			ready					        */
/************************************************************************/
/* This function allows the player pointed to by the first parameter to */
/* ready a weapon specified in the second, if it is a weapon.		*/

int ready(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	object	*obj_ptr;
	otag	*op;
	int	fd, i, cantwear = 0;

	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Wield what?\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	if(cmnd->num > 1) {
		obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
			cmnd->str[1], cmnd->val[1]);

		if(!obj_ptr) {
			print(fd, "You don't have that.\n");
			return(0);
		}

		if(obj_ptr->wearflag != WIELD) {
			print(fd, "You can't wield that.\n");
			return(0);
		}

		if(ply_ptr->ready[WIELD - 1]) {
			print(fd, "You're already wielding something.\n");
			return(0);
		}

		if(F_ISSET(obj_ptr, OPLDGK) && (BOOL(F_ISSET(obj_ptr, OKNGDM)) != 
			BOOL(F_ISSET(ply_ptr, PKNGDM)))) {
        		print(fd, "Your Prince does not want you to wield %i.\n",
					obj_ptr);
        		print(fd, "There is an insignia of Prince %s on it!\n",
					F_ISSET(ply_ptr, PKNGDM) ? "Ragnar":"Rutger");
        		return(0);
    	}              

      	if(F_ISSET(obj_ptr, OCLSEL))
			if(!F_ISSET(obj_ptr, (OCLSEL + ply_ptr->class)) && 
				(ply_ptr->class < CARETAKER)) {
        			print(fd, "Your class prevents you from using %i.\n",
						obj_ptr);
        			return(0);
		}

		if(F_ISSET(obj_ptr, OGOODO) && ply_ptr->alignment < -50) {
			print(fd, "%I shocks you and you drop it.\n", obj_ptr);
			print(fd, "You are evil, and %I is good.\n", obj_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M is shocked by %i.", 
				ply_ptr, obj_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
			add_obj_rom(obj_ptr, ply_ptr->parent_rom);
			return(0);
		}

		if(F_ISSET(obj_ptr, OEVILO) && ply_ptr->alignment > 50) {
			print(fd, "%I shocks you and you drop it.\n", obj_ptr);
			print(fd, "You are good, and %I is evil.\n", obj_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M is shocked by %i.", 
				ply_ptr, obj_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
			add_obj_rom(obj_ptr, ply_ptr->parent_rom);
			return(0);
		}

		i = (F_ISSET(obj_ptr, OSIZE1) ? 1:0) * 2 +
				(F_ISSET(obj_ptr, OSIZE2) ? 1:0);

		switch(i) {
			case 1:
				if(ply_ptr->race != GNOME &&
				   ply_ptr->race != HOBBIT &&
				   ply_ptr->race != DWARF) cantwear = 1;
				break;

			case 2:
				if(ply_ptr->race != HUMAN &&
			   	   ply_ptr->race != ELF &&
			       ply_ptr->race != HALFELF &&
			       ply_ptr->race != DARKELF &&
                   ply_ptr->race != HALFORC &&
                   ply_ptr->race != SATYR &&
			       ply_ptr->race != ORC) cantwear = 1;
				break;

			case 3:
				if(ply_ptr->race != CENTAUR && 
			   	   ply_ptr->race != HALFGIANT && 
			   	   ply_ptr->race != OGRE) cantwear = 1;
				break;
		}

		if(cantwear && ply_ptr->class < CARETAKER) {
			print(fd, "%I isn't the right size for you.\n", obj_ptr);
			return(0);
		}

		ply_ptr->ready[WIELD-1] = obj_ptr;
		compute_thaco(ply_ptr);
		del_obj_crt(obj_ptr, ply_ptr);
		compute_agility(ply_ptr);

		print(fd, "You wield %1i.\n", obj_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, "%M wields %1i.", ply_ptr, obj_ptr);

		print(fd, "%s\n", obj_ptr->use_output);
		if(F_ISSET(obj_ptr, OSILVR) && F_ISSET(obj_ptr, OTMPEN))
			print(fd, "The undead cannot abide the touch of glowing silver.\n");
		F_SET(obj_ptr, OWEARS);
		if(F_ISSET(obj_ptr, OCURSE))
            F_SET(obj_ptr, OCURSW);

		if(F_ISSET(obj_ptr, OINVIS)) {
			if(F_ISSET(obj_ptr, ODROPS) || F_ISSET(obj_ptr, OSOMEA)) {
				ANSI(fd, MAGENTA);
				print(fd, "The %s emerge from invisibility.\n", obj_ptr);
			}
			else {
				ANSI(fd, MAGENTA);
				print(fd, "The %s emerges from invisibility.\n", obj_ptr);
			}
			F_CLR(obj_ptr, OINVIS);
			ANSI(fd, WHITE);
		}
	}
	return(0);
}

/************************************************************************/
/*			hold					        */
/************************************************************************/
/* This function allows a player to hold an item if it is designated 	*/
/* as a hold-able item.							*/

int hold(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	object	*obj_ptr;
	otag	*op;
	int	fd, found = 0, match = 0;

	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Hold what?\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	if(cmnd->num > 1) {
		obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
			cmnd->str[1], cmnd->val[1]);

		if(!obj_ptr) {
			print(fd, "You don't have that.\n");
			return(0);
		}

		if(obj_ptr->wearflag != HELD && obj_ptr->wearflag != WIELD) {
			print(fd, "You can't hold that.\n");
			return(0);
		}

		if(ply_ptr->ready[HELD - 1]) {
			print(fd, "You're already holding something.\n");
			return(0);
		}

		if(F_ISSET(obj_ptr, OPLDGK) && 
			(BOOL(F_ISSET(obj_ptr, OKNGDM)) != 
			BOOL(F_ISSET(ply_ptr, PKNGDM)))) {
        		print(fd, "Your pledge makes you unable to hold %i.\n",
					obj_ptr);
        		return(0);
    	}              

      	if(F_ISSET(obj_ptr, OCLSEL))
			if(!F_ISSET(obj_ptr, (OCLSEL + ply_ptr->class)) && 
				(ply_ptr->class < CARETAKER)) {
        			print(fd, "Your class prevents you from holding %i.\n",
						obj_ptr);
        			return(0);
		}

		if(F_ISSET(obj_ptr, OGOODO) && ply_ptr->alignment < -50) {
			print(fd, "%I shocks you and you drop it.\n", obj_ptr);
			print(fd, "%I is good and you are evil.\n", obj_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M is shocked by %i.", 
				ply_ptr, obj_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
			add_obj_rom(obj_ptr, ply_ptr->parent_rom);
			return(0);
		}

		if(F_ISSET(obj_ptr, OEVILO) && ply_ptr->alignment > 50) {
			print(fd, "%I shocks you and you drop it.\n", obj_ptr);
			print(fd, "%I is evil and you are good.\n", obj_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, "%M is shocked by %i.", 
				ply_ptr, obj_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
			add_obj_rom(obj_ptr, ply_ptr->parent_rom);
			return(0);
		}

		ply_ptr->ready[HELD - 1] = obj_ptr;
		del_obj_crt(obj_ptr, ply_ptr);

		print(fd, "You hold %1i.\n", obj_ptr);
		if(obj_ptr->wearflag == HELD && obj_ptr->type == ARMOR) {
			compute_ac(ply_ptr);
			compute_agility(ply_ptr);
		}
		broadcast_rom(fd, ply_ptr->rom_num, "%M holds %1i.", ply_ptr, obj_ptr);

/*-------------------------------------------------------------------------------
If the object is special number 5, check to see if it is in it's convergence
room.  Such objects will display their use_output only if in that room.
-------------------------------------------------------------------------------*/

		if((obj_ptr->special == SP_ENCHT)) {
			if(obj_message(ply_ptr, cmnd)) {
  				if(!F_ISSET (obj_ptr, ONUSED)) {
        			obj_ptr->shotscur--;
        			if(obj_ptr->shotscur <= 0) {
						obj_ptr->shotscur = 0;
            			print(fd, 
							"There is a smell of ozone as %i turns to a dull gray dust.\n",
                				obj_ptr);
            			broadcast_rom(fd, ply_ptr->rom_num, 
							"The %s held by %m disintegrated into dust.\n",
                				obj_ptr, ply_ptr);
            			del_obj_crt(obj_ptr, ply_ptr);
            			free_obj(obj_ptr);
            			return(0);
        			}
    			}

			}
		}

		if(obj_ptr->special != SP_ENCHT) {
			if(obj_ptr->use_output[0] && obj_ptr->type != POTION && 
				obj_ptr->shotscur > 0)
					print(fd, "%s\n", obj_ptr->use_output);
		}

		if(F_ISSET(obj_ptr, OCURSE))
           	F_SET(obj_ptr, OCURSW);
		F_SET(obj_ptr, OWEARS);

		if(F_ISSET(obj_ptr, OINVIS)) {
			if(F_ISSET(obj_ptr, ODROPS) || F_ISSET(obj_ptr, OSOMEA)) {
				ANSI(fd, MAGENTA);
				print(fd, "The %s emerge from invisibility.\n", obj_ptr);
			}
			else {
				ANSI(fd, MAGENTA);
				print(fd, "The %s emerges from invisibility.\n", obj_ptr);
			}
			F_CLR(obj_ptr, OINVIS);
			ANSI(fd, WHITE);
		}

		if(F_ISSET(obj_ptr, OLUCKY)) {
			if(obj_ptr->shotscur > 0) {
				Ply[ply_ptr->fd].extr->luck += ply_ptr->ready[HELD - 1]->ndice;
				print(fd, "Your luck is improving.\n");
				if(F_ISSET(ply_ptr, PHEXED)) {
					print(fd, "The hex spell on you fades.\n");
					Ply[fd].extr->luck = 50 + ply_ptr->ready[HELD - 1]->ndice;
					F_CLR(ply_ptr, PHEXED);
					obj_ptr->shotscur--;
					compute_thaco(ply_ptr);
				}
			}
			else
				print(fd, "The luck in the %s is used up.\n", obj_ptr); 
		}

		if(obj_ptr->wearflag == WIELD) {
			print(fd, "But if you want to use it, you must wield it. \n");
			return(0);
		}

/*----------------------------------------------------------------------*/
/*		Player may change from Lawful to Chaotic		*/
/*                  The process makes the player evil                   */
/*----------------------------------------------------------------------*/

		if(obj_ptr->special == SP_HORNS && ply_ptr->experience >
			(needed_exp[ply_ptr->level - 2] + ply_ptr->experience/10)) {
				ply_ptr->experience -= ply_ptr->experience/10;
				F_SET(ply_ptr, PCHAOS);
				ply_ptr->alignment = -1000;
				del_obj_crt(obj_ptr, ply_ptr);
				CLS(fd);
				ANSI(fd, REDBG);
				ANSI(fd, YELLOW);
				print(fd, "\nZeth accepts your tribute!\n");
				print(fd, "Your head spins and the temple goes dark as "
                          "a great roar fills the air.\n");
				print(fd, "Now you are one of his Chaotic followers, a "
                          "trusted servant of the Black Goat.\n");
				ANSI(fd, BLUEBG);
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
				print(fd, "\nThe Great Horns vanish in a cold flame.\n");
		}
		else if(obj_ptr->special == SP_HORNS) { 
			CLS(fd);
			ANSI(fd, YELLOW);
			print(fd, "Zeth rejects the offer of your soul as you have "
                      "too little experience.\n");
			print(fd, "However, he gladly accepts all of your gold as a "
                      "tithing offer.\n");
			print(fd, "Your head spins and the temple goes dark as a silence "
                       "fills the air.\n");
			print(fd, "You see the Great Horns vanish into darkness, and your "
                      "evilness is purged.\n");
			print(fd, "Now begone, fool.\n");
			del_obj_crt(obj_ptr, ply_ptr);
			ply_ptr->gold = 0;
			ply_ptr->alignment = 1000;
			ANSI(fd, BLUEBG);
			ANSI(fd, BOLD);
			ANSI(fd, WHITE);
		}
	}
	return(0);
}

/**********************************************************************/
/*			equip_weight				      */
/**********************************************************************/
int equip_weight(crt_ptr)

creature	*crt_ptr;

{
int	wgt = 0;

	if(crt_ptr->ready[BODY - 1]) {
		wgt += weight_obj(crt_ptr->ready[BODY - 1]);
	}
	if(crt_ptr->ready[ARMS - 1]) {
		wgt += weight_obj(crt_ptr->ready[ARMS - 1]);
	}
	if(crt_ptr->ready[LEGS - 1]) {
		wgt += weight_obj(crt_ptr->ready[LEGS - 1]);
	}
	if(crt_ptr->ready[NECK1 - 1]) {
		wgt += weight_obj(crt_ptr->ready[NECK1 - 1]);
	}
	if(crt_ptr->ready[NECK2 - 1]) {
		wgt += weight_obj(crt_ptr->ready[NECK2 - 1]);
	}
	if(crt_ptr->ready[HANDS - 1]) {
		wgt += weight_obj(crt_ptr->ready[HANDS - 1]);
	}
	if(crt_ptr->ready[HEAD - 1]) {
		wgt += weight_obj(crt_ptr->ready[HEAD - 1]);
	}
	if(crt_ptr->ready[FEET - 1]) {
		wgt += weight_obj(crt_ptr->ready[FEET - 1]);
	}
	if(crt_ptr->ready[FACE - 1]) {
		wgt += weight_obj(crt_ptr->ready[FACE - 1]);
	}
	if(crt_ptr->ready[FINGER1 - 1]) {
		wgt += weight_obj(crt_ptr->ready[FINGER1 - 1]);
	}
	if(crt_ptr->ready[FINGER2 - 1]) {
		wgt += weight_obj(crt_ptr->ready[FINGER2 - 1]);
	}
	if(crt_ptr->ready[FINGER3 - 1]) {
		wgt += weight_obj(crt_ptr->ready[FINGER3 - 1]);
	}
	if(crt_ptr->ready[FINGER4 - 1]) {
		wgt += weight_obj(crt_ptr->ready[FINGER4 - 1]);
	}
	if(crt_ptr->ready[FINGER5 - 1]) {
		wgt += weight_obj(crt_ptr->ready[FINGER5 - 1]);
	}
	if(crt_ptr->ready[FINGER6 - 1]) {
		wgt += weight_obj(crt_ptr->ready[FINGER6 - 1]);
	}
	if(crt_ptr->ready[FINGER7 - 1]) {
		wgt += weight_obj(crt_ptr->ready[FINGER7 - 1]);
	}
	if(crt_ptr->ready[FINGER8 - 1]) {
		wgt += weight_obj(crt_ptr->ready[FINGER8 - 1]);
	}
	if(crt_ptr->ready[HELD - 1]) {
		wgt += weight_obj(crt_ptr->ready[HELD - 1]);
	}
	if(crt_ptr->ready[SHIELD - 1]) {
		wgt += weight_obj(crt_ptr->ready[SHIELD - 1]);
	}
	if(crt_ptr->ready[WIELD - 1]) {
		wgt += weight_obj(crt_ptr->ready[WIELD - 1]);
	}
	return(wgt);
}

/**********************************************************************/
/*			Special Message				      */
/**********************************************************************/
/* This function allows an object to give a special message if the    */
/* object held by the player has a like object in room+1 	      */
/* NOTE:  The object referenced in this section of code pertains to   */
/* the object in the store room, not the object held by the player.   */
/**********************************************************************/

int obj_message(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room	*rom_ptr, *dep_ptr;
	object	*obj_ptr;
	int	fd, clue_given = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(rom_ptr->special != 5) {
		return(0);
	}

/*=========================================================*/
/*           Room+1 must have a permanent like object      */
/*=========================================================*/
  
	if(load_rom(rom_ptr->rom_num + 1, &dep_ptr) < 0) {
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, dep_ptr->first_obj, 
		cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "Aaaaiiieeeeeeee!\n");
		return(0);
	}

	if(obj_ptr->shotscur <= 0) {
		print(fd, "There is only silence.\n");
		return(0);
	}
  	if(!obj_ptr->use_output[0]) {
		print(fd, "The %s is mute.\n", obj_ptr);
		return(0);
	}

  	if(obj_ptr->use_output[0] && obj_ptr->type != POTION)
		print(fd, "%s\n", obj_ptr->use_output);

	broadcast_rom(fd, ply_ptr->rom_num, "The %s whispers to it's holder.\n", 
		obj_ptr);

	clue_given = 1;

	return(clue_given);
}

