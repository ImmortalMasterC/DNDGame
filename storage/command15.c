/*
 * COMMAND15.C:
 * 
 *   Additional user routines for Druids:
 *	(c) 1996  Brooke Paul & Eric Krichbaum
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

/************************************************************************/
/*							Prepare herbs								*/
/************************************************************************/

int prep_herb(ply_ptr,cmnd)

creature *ply_ptr;
cmd      *cmnd;

{
	object	*obj_ptr;
	long	t;
	int		fd;

	fd = ply_ptr->fd;

	if(ply_ptr->class != DRUID && ply_ptr->class < CARETAKER) {
		prepare(ply_ptr, cmnd);
		return(0);
	}   

	if(cmnd->num < 2) {
		print(fd, "Prepare what herb?\n");
		return(0);
	}
   
	t = time(0);
	if(ply_ptr->lasttime[LT_PREPN].interval + 
		ply_ptr->lasttime[LT_PREPN].ltime > t) {
			print(fd, "You cannot prepare any more herbs right now.\n");
			return(0);
	}

	if(LT(ply_ptr, LT_ATTCK) > t)
		print(fd, "You cannot prepare herbs while fighting!\n");

	obj_ptr = ply_ptr->ready[HELD - 1];
	if(!obj_ptr) {
		print(fd, "You must be holding an herb.\n");
		return(0);
	}

   if(obj_ptr->type != HERB) {
     print(fd, "%s is not an herb.\n", obj_ptr);
     return(0);
   }

   if(ply_ptr->ready[WIELD - 1]) {
	print(fd, "You're hands are too full to do that.\n");
	return(0);
   }

   if(obj_ptr->shotscur < 1) {
		print(fd, "That herb is used up.\n");
		return(0);
   }   

	F_SET(obj_ptr, ONPREP);
	F_SET(ply_ptr, PPREPN);
	ply_ptr->lasttime[LT_PREPN].ltime = t; 

	if(F_ISSET(obj_ptr, OHBREW)) {	
		print(fd, "You begin to brew the %s for drinking.\n", obj_ptr);
		broadcast_rom(fd, ply_ptr->rom_num,
			"%M begins to brew a %s for drinking.", ply_ptr, obj_ptr->name);
		ply_ptr->lasttime[LT_PREPN].interval = 60L;
	}

	if(F_ISSET(obj_ptr, OHNGST)) {	
		print(fd, "You begin to prepare a %s for eating.\n", obj_ptr->name);
		broadcast_rom(fd,ply_ptr->rom_num,
			"%M begins to prepare a %s for eating.", ply_ptr, obj_ptr->name);
		ply_ptr->lasttime[LT_PREPN].interval = 10L;
	}

   if(F_ISSET(obj_ptr, OHAPLY)) {	
     print(fd, "You begin to soften a %s into a cream.\n", obj_ptr->name);
     broadcast_rom(fd, ply_ptr->rom_num,
		   "%M begins to soften a %s into a cream.", ply_ptr, obj_ptr->name);
     ply_ptr->lasttime[LT_PREPN].interval = 10L;
   }

   if(F_ISSET(obj_ptr, OHPAST)) {	
     print(fd, "You begin to mix a %s into a paste.\n", obj_ptr->name);
     broadcast_rom(fd, ply_ptr->rom_num,
		   "%M begins to mix a %s into a paste.", ply_ptr, obj_ptr->name);
     ply_ptr->lasttime[LT_PREPN].interval = 15L;
   }

   return(PROMPT);
}

/************************************************************************/
/*							Apply herbs									*/
/************************************************************************/

int apply_herb(ply_ptr, cmnd)
creature *ply_ptr;
cmd      *cmnd;

{
	object	*obj_ptr;
	long	t;
	int		fd;

	fd = ply_ptr->fd;

   
	if(cmnd->num < 2) {
		print(ply_ptr->fd, "Apply what?\n");
		return(0);
	}

	t = time(0);

	if(LT(ply_ptr,LT_PREPN) > t) {
		print(ply_ptr->fd, "You are still preparing the herb.\n");
		return(0);
	}

	if(LT(ply_ptr,LT_ATTCK) > t) {
		print(ply_ptr->fd, "You can not apply herbs while in combat!\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't seem to have the %s.\n", cmnd->str[1]);
		return(0);
	}

	if(obj_ptr->type != HERB) {
		print(fd, "That is not an herb.\n");
		return(0);
	}

	if(!F_ISSET(obj_ptr, ODPREP)) {
		print(fd, "The %s has not been prepared properly.\n", obj_ptr->name);
		return(0);
	}

	if(!F_ISSET(obj_ptr, OHAPLY)) {
		print(fd, "You cannot apply %s.\n", obj_ptr->name);
		return(0);
	}

	print(fd, "You apply the herb to your skin.\n");
	broadcast_rom(fd, ply_ptr->rom_num, "%M applies some %s.", 
		ply_ptr, obj_ptr->name);
	use_herb(ply_ptr, obj_ptr, obj_ptr->special);
	return(PROMPT);
}

/************************************************************************/
/*							Ingest herbs								*/
/************************************************************************/

int ingest_herb(ply_ptr,cmnd)
creature *ply_ptr;
cmd      *cmnd;

{
	object	*obj_ptr;
	long	t;
	int		fd;

	fd = ply_ptr->fd;
   
	if(cmnd->num < 2) {
		print(fd, "Ingest what?\n");
		return(0);
	}

	t = time(0);
	if(LT(ply_ptr, LT_PREPN) > t) {
		print(ply_ptr->fd, "You are still preparing the herb.\n");
		return(0);
	}
 

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(obj_ptr->type != HERB) {
		print(ply_ptr->fd, "That is not an herb.\n");
		return(0);
	}

	if(!F_ISSET(obj_ptr, ODPREP)) {
		print(fd, "The %s has not been prepared properly.\n", obj_ptr->name);
		return(0);
	}

	if(!F_ISSET(obj_ptr, OHBREW)) {
		print(fd, "You cannot drink a %s.\n", obj_ptr->name);
		return(0);
	}

	print(fd,"You drink the herb's broth.\n");
	broadcast_rom(fd, ply_ptr->rom_num, "%M drinks some %s.",
		ply_ptr, obj_ptr->name);
	use_herb(ply_ptr, obj_ptr, obj_ptr->special);

	return(PROMPT);
}

/************************************************************************/
/*							Eat herbs									*/
/************************************************************************/

int eat_herb(ply_ptr,cmnd)
creature *ply_ptr;
cmd      *cmnd;

{
	object	*obj_ptr;
	long	t;
	int		fd;

	fd = ply_ptr->fd;
   
	if(cmnd->num < 2) {
		print(fd, "Eat what?\n");
		return(0);
	}

	t = time(0);

	if(LT(ply_ptr, LT_PREPN) > t) {
		print(fd, "You are still preparing the herb.\n");
		return(0);
	}
  
	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(obj_ptr->type != HERB) {
		print(fd, "That is not an herb.\n");
		return(0);
	}

	if(!F_ISSET(obj_ptr, ODPREP)) {
		print(fd, "The %s has not been prepared properly.\n", 
			obj_ptr->name);
		return(0);
	}

	if(!F_ISSET(obj_ptr, OHNGST)) {
		print(fd, "You cannot eat a %s.\n", obj_ptr->name);
		return(0);
	}

	print(fd, "You eat the herb.\n");
	broadcast_rom(fd, ply_ptr->rom_num, "%M eats some %s.",
		ply_ptr, obj_ptr->name);
	use_herb(ply_ptr, obj_ptr, obj_ptr->special);

	return(PROMPT);
}

/************************************************************************/
/*							Paste herbs									*/
/************************************************************************/

int paste_herb(ply_ptr,cmnd)
creature *ply_ptr;
cmd      *cmnd;

{
	object	*obj_ptr;
	long	t;
	int		fd;

	fd = ply_ptr->fd;
   
	if(cmnd->num < 2) {
		print(fd, "Paste what?\n");
		return(0);
	}

	t = time(0);

	if(LT(ply_ptr, LT_PREPN) > t) {
		print(fd, "You are still preparing the herb.\n");
		return(0);
	}

	if(LT(ply_ptr, LT_ATTCK) > t) {
		print(fd, "You cannot apply herbs while in combat!\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(obj_ptr->type != HERB) {
		print(fd, "That is not an herb.\n");
		return(0);
	}

	if(!F_ISSET(obj_ptr, ODPREP)) {
		print(fd, "The %s has not been prepared properly.\n", obj_ptr->name);
		return(0);
	}

	if(!F_ISSET(obj_ptr, OHPAST)) {
		print(fd,"You cannot apply a %s.\n", obj_ptr->name);
		return(0);
	}

	print(fd,"You apply the herb.\n");
	broadcast_rom(fd, ply_ptr->rom_num, "%M applies some %s.",
		ply_ptr, obj_ptr->name);
	use_herb(ply_ptr, obj_ptr, obj_ptr->special);

	return(PROMPT);
}

/************************************************************************/
/*							use herbs									*/
/************************************************************************/

int use_herb(ply_ptr,obj_ptr,herb_type)
creature *ply_ptr;
object   *obj_ptr;
int      herb_type;

{
	int		dmg, fd;
   
	switch(herb_type) {
		case SP_HERB_HEAL:
			print(fd,"A warmth flows through your body.\n");
			dmg = mdice(obj_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
			free_obj(obj_ptr);
			ply_ptr->hpcur += dmg;
			if(ply_ptr->hpcur > ply_ptr->hpmax)
			ply_ptr->hpcur = ply_ptr->hpmax;
			break;

		case SP_HERB_POISON:
			print(fd, "Your blood begins to burn as the poison spreads!\n");
			F_SET(ply_ptr, PPOISN);
			dmg = mdice(obj_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
			free_obj(obj_ptr);
			ply_ptr->hpcur -= dmg;
			if(ply_ptr->hpcur < 1)
			die(ply_ptr, ply_ptr);
			break;

		case SP_HERB_HARM:
			print(fd, "Your body begins to jerk madly!\n");
			dmg = mdice(obj_ptr);
			del_obj_crt(obj_ptr, ply_ptr);
			free_obj(obj_ptr);
			ply_ptr->hpcur -= dmg;
			if(ply_ptr->hpcur < 1)
			die(ply_ptr, ply_ptr);
			break;

		case SP_HERB_DISEASE:
			print(fd,
				"A puss-dripping rash appears on your arms and face.\n");
			F_SET(ply_ptr, PDISEA);
			break;

		case SP_HERB_CURE_POISON:
			print(fd, "You feel your blood cool.\n");
			F_CLR(ply_ptr, PPOISN);
			del_obj_crt(obj_ptr, ply_ptr);
			free_obj(obj_ptr);
			break;

		case SP_HERB_CURE_DISEASE:
			print(fd, "A clean feeling flows through your body.\n");
			F_CLR(ply_ptr, PDISEA);
			del_obj_crt(obj_ptr, ply_ptr);
			free_obj(obj_ptr);
			break;

		default:
			print(ply_ptr->fd, "The herb went bad.\n");
			del_obj_crt(obj_ptr, ply_ptr);
			free_obj(obj_ptr);
			break;
	}
	return(PROMPT);
}



/************************************************************************/
/*							Eat											*/
/*  This function handes all attempts to eat any type object.			*/
/************************************************************************/

int eat(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{

	object 	*obj_ptr;
	int		fd;

	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Eat what?\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, cmnd->str[1], cmnd->val[1]);
	
	if(!obj_ptr) {
		print(fd, "You dont have that.\n");
		return(0);
	}

	switch(obj_ptr->type) {
		case HERB:
			if(F_ISSET(obj_ptr, OHBREW)) 
				ingest_herb(ply_ptr, cmnd);
			if(F_ISSET(obj_ptr, OHNGST)) 
				eat_herb(ply_ptr, cmnd);			
			break;

		case DRINK:
			if(ply_ptr->talk[5] < 1) {
				print(fd, "You can't drink anymore.\n");
			break;
			}

			if(obj_ptr->shotscur) {
				obj_ptr->shotscur -= 1;			
				ply_ptr->talk[5] -= obj_ptr->ndice;
				if(ply_ptr->talk[5] < 0)
					ply_ptr->talk[5] = 0;
				broadcast_rom(fd, ply_ptr->rom_num, "%M drinks from a %s.", 
					ply_ptr, obj_ptr);
				print(fd, "Ahh, refreshing!\n");
				break;
			}
			else 
				print(fd, "The %s is empty.\n", obj_ptr);
				break;
	
		case FOOD:
			if(ply_ptr->talk[6] < 1) {
				print(fd, "You cannot eat another bite.\n");
				break;
			}
			if(obj_ptr->shotscur) {
				obj_ptr->shotscur -= 1;
				ply_ptr->talk[6] -= obj_ptr->ndice;
				if(ply_ptr->talk[6] < 0)
					ply_ptr->talk[6] = 0;
				if(!obj_ptr->shotscur) {			
					del_obj_crt(obj_ptr, ply_ptr);
					free(obj_ptr);	
				}
				broadcast_rom(fd, ply_ptr->rom_num, "%M munches on a %s.", 
					ply_ptr, obj_ptr);
				print(fd, "Yummy!\n");
				break;
			}
			else
				del_obj_crt(obj_ptr, ply_ptr);
			free(obj_ptr);
			print(fd, "I don't see that here.\n");
			break;

		default:
			print(fd, "You can't do that.\n");
	}
	if(ply_ptr->talk[5] < 4 && ply_ptr->talk[6] < 4)
	F_CLR(ply_ptr, PNSUSN);

	return(0);
}
