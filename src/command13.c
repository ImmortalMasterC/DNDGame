/*
 * COMMAND13.C:
 *
 *	Additional user routines.
 *
 *	Written by Roy Wilson
 *	
 */

#include "mstruct.h"
#include "mextern.h"


/************************************************************************/
/*							unfasten container				   			*/
/************************************************************************/
/* This function allows a player to unfasten a container if he has the	*/
/* correct key, and it is locked.										*/

int unfasten_cont(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	room	*rom_ptr;
	object	*obj_ptr, *cnt_ptr;
	int		fd, on_person = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	
	if(cmnd->num < 2) {
		print(fd, "What would you like to unfasten?\n");
		return(0);
	}

	cnt_ptr = find_obj(ply_ptr, ply_ptr->first_obj, cmnd->str[1], cmnd->val[1]);
	if(cnt_ptr) {
		on_person = 1;
	}
	else {
		cnt_ptr = find_obj(ply_ptr, rom_ptr->first_obj, cmnd->str[1],
			 cmnd->val[1]);
		if(!cnt_ptr) {
			print(fd, "You don't have that, and it's not here.\n");
			return(0);
		}
	}

	if(cnt_ptr->type != CONTAINER) {
		print(fd, "It's not a container.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "You can't see to unfasten it.\n");
		return(0);
	}
	
	if(!F_ISSET(cnt_ptr, OFASTD)) {
		print(fd, "It's not fastened.\n");
		return(0);
	}

	if(cmnd->num < 3) {
		print(fd, "Unfasten it with what?\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[2], cmnd->val[2]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(obj_ptr->type != KEY) {
		print(fd, "That's not a key.\n");
		return(0);
	}

	if(obj_ptr->shotscur < 1) {
		print(fd, "%I is broken.\n", obj_ptr);
		return(0);
	}

	if(cnt_ptr->ndice != obj_ptr->ndice) {
		print(fd, "Wrong key.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	F_CLR(cnt_ptr, OFASTD);
	if(!F_ISSET(obj_ptr, ONUSED))
		obj_ptr->shotscur--;

	if(obj_ptr->use_output[0])
		print(fd, "%s\n", obj_ptr->use_output);
	else
		print(fd, "Snick.  You unfasten the %s.\n", cnt_ptr->name);
	if(!on_person)
		broadcast_rom(fd, ply_ptr->rom_num, "%M unfastens the %s.",
			ply_ptr, cnt_ptr->name);

	return(0);

}

/************************************************************************/
/*								fasten							      	*/
/************************************************************************/
/* This function allows a player to fasten a container if he has the    */
/* correct key. 														*/

int fasten_cont(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	room	*rom_ptr;
	object	*obj_ptr, *cnt_ptr;
	int		fd, on_person = 0; 

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	
	if(cmnd->num < 2) {
		print(fd, "Fasten what?\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "You can't see to fasten it.\n");
		return(0);
	}

	cnt_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		 cmnd->str[1], cmnd->val[1]);
	if(cnt_ptr)
		on_person = 1;

	if(!cnt_ptr) {
		cnt_ptr = find_obj(ply_ptr, rom_ptr->first_obj, cmnd->str[1],
			cmnd->val[1]);
		if(!cnt_ptr) {
			print(fd, "You don't have that, and it's not here.\n");
			return(0);
		}
	}

	if(cnt_ptr->type != CONTAINER) {
		print(fd, "It's not a container.\n");
		return(0);
	}

	if(F_ISSET(cnt_ptr, OFASTD)) {
		print(fd, "It's already fastened.\n");
		return(0);
	}

	if(cmnd->num < 3) {
		print(fd, "Fasten it with what?\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[2], cmnd->val[2]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(obj_ptr->type != KEY) {
		print(fd, "%I is not a key.\n", obj_ptr);
		return(0);
	}

	if(!F_ISSET(cnt_ptr, OFASTS)) {
		print(fd, "It doesn't have a fastener on it.\n");
		return(0);
	}

	if(obj_ptr->shotscur < 1) {
		print(fd, "%I is broken.\n", obj_ptr);
		return(0);
	}

	if(cnt_ptr->ndice != obj_ptr->ndice) {
		print(fd, "Wrong key.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	F_SET(cnt_ptr, OFASTD);
	print(fd, "Snick.  You fasten the %s.\n", cnt_ptr->name);
	if(!on_person)
		broadcast_rom(fd, ply_ptr->rom_num, "%M fastens the %s.",
			ply_ptr, cnt_ptr->name);

	return(0);
}

/************************************************************************/
/*								jimmy	  						        */
/************************************************************************/
/* This function is called when a thief attempts to jimmy a fastener    */
/* on a container.  There is only a small chance (depending on the 		*/
/* player's level and luck) that the lock will be jimmied.              */

int jimmylock(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room	*rom_ptr;
	object	*obj_ptr, *cnt_ptr;
	long	i, t;
	int		fd, chance, on_person = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->class != THIEF && ply_ptr->class < CARETAKER) {
		print(fd, "Only a thief can jimmy a locked container.\n");
		return(0);
	}

	if(cmnd->num < 2) {
		print(fd, "Jimmy what?\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "You're blind and can't see to jimmy it.\n");
		return(0);
	}
	cnt_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[1], cmnd->val[1]);
	if(cnt_ptr)
		on_person = 1;

	if(!cnt_ptr) {
		cnt_ptr = find_obj(ply_ptr, rom_ptr->first_obj, 
			cmnd->str[1], cmnd->val[1]);
		if(!cnt_ptr) {
			print(fd, "You don't have that, and it's not here either.\n");
			return(0);
		}
	}

	if(cnt_ptr->type != CONTAINER) {
		print(fd, "It's not a container.\n");
		return(0);
	}

	if(!F_ISSET(cnt_ptr, OFASTD)) {
		print(fd, "It's not fastened.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	if(F_ISSET(cnt_ptr, ONJIMY)) {
		print(fd, "The %s has a magic spell on it.  You cannot jimmy it.\n",
			cnt_ptr->name);
		return(PROMPT);
	}
	i = LT(ply_ptr, LT_JIMMY);
	t = time(0);
	if(t < i) {
		please_wait(fd, i - t);
		return(0);
	}
	ply_ptr->lasttime[LT_JIMMY].ltime = t;
	ply_ptr->lasttime[LT_JIMMY].interval = 10;

	chance = ply_ptr->level + 
		bonus[ply_ptr->dexterity] + 
			Ply[ply_ptr->fd].extr->luck/50;

	if(chance >= mrand(1, 100)) {
		print(fd, "Snick.  You successfully jimmied open the %s.\n",
			cnt_ptr->name);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M jimmied open the %s!", ply_ptr, cnt_ptr);
		F_CLR(cnt_ptr, OFASTD);
		print(fd, "You gained %d experience points.\n", ply_ptr->level);
		ply_ptr->experience += ply_ptr->level;
		return(0);
	}
	else {
		print(fd, "You failed.\n");
		if(!on_person)
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M failed to jimmy open the %s.", ply_ptr, cnt_ptr);
		return(0);
	}

	return(0);
}


/************************************************************************/
/*								store	  								*/
/************************************************************************/
/* 	This function allows a player to store an item in a container.	 	*/
/* 	The container cannot be in the player's inventory.				 	*/

int store_item(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
	room	*rom_ptr;
	object	*obj_ptr, *cnt_ptr;
	long	i, t;
	int		fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 2) {
		print(fd, "Store what?  And where do you want to store it?\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(cmnd->num < 3) {
		print(fd, "Where do you want to store it?\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PBLIND)) {
		print(fd, "You're blind and can't see to store it.\n");
		return(0);
	}

	cnt_ptr = find_obj(ply_ptr, rom_ptr->first_obj, cmnd->str[2], cmnd->val[2]);
	if(!cnt_ptr) {
		print(fd, "That's not here.\n");
		return(0);
	}

	if(cnt_ptr->type != CONTAINER) {
		print(fd, "The %s is not a container.\n", cnt_ptr->name);
		return(0);
	}

	if(F_ISSET(cnt_ptr, OFASTD)) {
		print(fd, "It's fastened.  You have to open it first.\n");
		return(0);
	}

	if(cnt_ptr->shotscur >= cnt_ptr->shotsmax) {
		print(fd, "%I can't hold anymore.\n", cnt_ptr);
		return(0);
	}

	if(F_ISSET(obj_ptr, OCONTN)) {
		print(fd, "You can't store a container in a container.\n");
		return(0);
	}

   	if(F_ISSET(cnt_ptr, OCNDES)) {
		print(fd, "You can't store anything in that!\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);
	F_SET(obj_ptr, OTEMPP);

	del_obj_crt(obj_ptr, ply_ptr);
	add_obj_obj(obj_ptr, cnt_ptr);
	cnt_ptr->shotscur++;

	print(fd, "You store %1i in %1i.\n", obj_ptr, cnt_ptr);
	broadcast_rom(fd, rom_ptr->rom_num, "%M stores %1i in %1i.",
		ply_ptr, obj_ptr, cnt_ptr);

	resave_all_rom(PERMONLY);
	return(0);
}
