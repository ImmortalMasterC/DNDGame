/*
 * COMMAND7.C:
 *
 *	Additional user routines.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>
#include <stdlib.h>

/**********************************************************************/
/*			list					      */
/**********************************************************************/
/* This function allows a player to list the items for sale within a  */
/* shoppe.							      */

int list(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room	*rom_ptr, *dep_ptr;
	otag	*op;
	int	fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(F_ISSET(rom_ptr, RSHOPP)) {
		if(load_rom(rom_ptr->rom_num + 1, &dep_ptr) < 0) {
			print(fd, "Nothing to buy.\n");
			return(0);
		}

		add_permobj_rom(dep_ptr);
		print(fd, "You may buy:\n");
		op = dep_ptr->first_obj;

		while(op) {
			print(fd, "   Cost: %5ld\t%-30s", op->obj->value, 
				obj_str(op->obj, 1, CAP));
			if(!F_ISSET(op->obj, OSIZE2) && F_ISSET(op->obj, OSIZE1))
				print(fd, "   (medium)");
			else if(F_ISSET(op->obj, OSIZE2) && !F_ISSET(op->obj, OSIZE1))
				print(fd, "   (small)");
			else if(F_ISSET(op->obj, OSIZE2) && F_ISSET(op->obj, OSIZE1))
				print(fd, "   (large)");
			print(fd, "\n");
			op = op->next_tag;
		}
		print(fd, "\n");
	}
	else {
		print(fd, "This is not a shoppe.\n");
		print(fd, "There's nothing for sale here.\n");
	}

	return(0);
}

/**********************************************************************/
/*				buy				      */
/**********************************************************************/
/* This function allows a player to buy something from a shoppe.      */

int buy(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room	*rom_ptr, *dep_ptr;
	object	*obj_ptr, *obj_ptr2;
	int	fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(!F_ISSET(rom_ptr, RSHOPP)) {
		print(fd, "This is not a shop that sells items.\n");
		print(fd, "Did you mean to purchase from someone?\n");
		return(0);
	}

	if(cmnd->num < 2) {
		print(fd, "Buy what? Do you mean to ask for a list?\n");
		return(0);
	}

	if(load_rom(rom_ptr->rom_num + 1, &dep_ptr) < 0) {
		print(fd, "The shoppe's inventory is presently depleted.\n");
		print(fd, "It shouldn't be, so please send mudmail to Styx.\n");
		return(0);
	}

	obj_ptr = find_obj(ply_ptr, dep_ptr->first_obj, 
		cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "That's not being sold.\n");
		return(0);
	}

	if(ply_ptr->gold < obj_ptr->value) {
		print(fd, "You don't have enough gold.\n");
		return(0);
	}
 
	if(weight_ply(ply_ptr) + weight_obj(obj_ptr) > max_weight(ply_ptr) ||
		(player_inv(ply_ptr) == 80)) {
			print(fd, "You can't carry anymore.\n");
			return(0);
	}             

	obj_ptr2 = (object *)malloc(sizeof(object));
	if(!obj_ptr2)
		merror("buy", FATAL);
	*obj_ptr2 = *obj_ptr;
	F_CLR(obj_ptr2, OPERM2);
	F_CLR(obj_ptr2, OPERMT);
	F_CLR(obj_ptr2, OTEMPP);
	F_CLR(ply_ptr, PHIDDN);

	add_obj_crt(obj_ptr2, ply_ptr);
	ply_ptr->gold -= obj_ptr2->value;
	print(fd, "Bought.\n");
	broadcast_rom(fd, ply_ptr->rom_num, "%M bought %1i.", ply_ptr, obj_ptr2);

	return(0);
}

/************************************************************************/
/*				sell				        */
/************************************************************************/
/* This function will allow a player to sell an object in a pawn shoppe */

int sell(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room	*rom_ptr;
	object	*obj_ptr;
	int	gold, fd, poorquality = 0, used = 0;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(!F_ISSET(rom_ptr, RPAWNS) && F_ISSET(rom_ptr, RSHOPP)) {
		print(fd, "This shoppe will not buy your ill-gotten gains.\n");
		print(fd, "Why don't you go to a pawn shop with your bloody goods?\n");
		return(0);
	}

	if(!F_ISSET(rom_ptr, RPAWNS) && !F_ISSET(rom_ptr, RSHOPP)) {
		print(fd, "This is not a pawn shoppe.  "
			"You will have to go elsewhere to sell your goods.\n");
		return(0);
	}

	if(cmnd->num < 2) {
		print(fd, "Sell what?\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(obj_ptr->shotscur <= obj_ptr->shotsmax/2)
		used = 1;

	gold = obj_ptr->value/2;
	if(Ply[fd].extr->luck > 55) 
		gold += obj_ptr->value/12;
	if(Ply[fd].extr->luck > 65)
		gold += obj_ptr->value/12;
	if(used)
		gold = gold/2;
	gold = MIN(gold, 10000);  
	
	if((obj_ptr->type <= MISSILE || obj_ptr->type == ARMOR) &&
		obj_ptr->shotscur <= obj_ptr->shotsmax/8)
			poorquality = 1;

	if(obj_ptr->type == REPAIR && obj_ptr->shotscur < 2)
		poorquality = 1;

	if((obj_ptr->type == WAND || obj_ptr->type == KEY) && 
		obj_ptr->shotscur < 1)
			poorquality = 1;

	if((obj_ptr->type == WAND || obj_ptr->type == KEY) && 
		obj_ptr->shotscur < 2)
			used = 1;

	if(F_ISSET(obj_ptr, OCONTN) && obj_ptr->shotscur >= 1) {
		print(fd, 
			"The shopkeep shakes his head and gives %i back to you.", obj_ptr);
		print(fd, "\nHe says, \"It has something in it.\"\n");
		return;
	}

	if(Ply[fd].extr->luck < 35) {
		print(fd, "The shopkeep says, \"I don't like your looks and won't "
					"buy any of your crap.\"\n");
		print(fd, " \"You look damned unlucky to me.  Go away, it might be "
					"catching!\"\n");
		return(0);
	}

	if(gold < 20 || poorquality) {
		print(fd, "The shopkeep says, \"I won't buy that junk from you.\"\n");
		if(poorquality) 
			print(fd, "\"Can't you see its almost broken?\"\n");
		return(0);
	}

	if(obj_ptr->type == SCROLL || obj_ptr->type == POTION) {
		print(fd, "The clerk won't buy potions or scrolls.\n");
		return(0);
	}

	if(obj_ptr->special == SP_NOSEL) {
		print(fd, "The clerk backs away from the counter, and with a "
					"scared look says,\n");
		print(fd, "\"I dare not buy such an item.  You should take it "
					"to the Temple of Alith!\"\n");
		return(0);
	}
	
	if(used) {
		print(fd, "The clerk says, \"It's been used, so I can't " 
						"give you full value for it.\"\n");
	}

	broadcast_rom(fd, ply_ptr->rom_num, "%M sells %1i.", ply_ptr, obj_ptr);
	print(fd, "The shopkeep gives you %d gold pieces for %i.\n", gold, obj_ptr);
	if(Ply[fd].extr->luck > 55) 
		print(fd, "Your good luck gave you an extra measure of gold.\n");

	ply_ptr->gold += gold;
	del_obj_crt(obj_ptr, ply_ptr);
	free_obj(obj_ptr);

	return(0);

}

/************************************************************************/
/*				value		   		        */
/************************************************************************/
/* This function allows a player to find out the pawn-shop value of an 	*/
/* object, if he is in the pawn shop.					*/

int value(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room	*rom_ptr;
	object	*obj_ptr;
	long	value;
	int	fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(!F_ISSET(rom_ptr, RPAWNS) && !F_ISSET(rom_ptr, RREPAI)) {
		print(fd, "You must be in a pawn shoppe or repair facility.\n");
		return(0);
	}

	if(cmnd->num < 2) {
		print(fd, "Value what?\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[1], cmnd->val[1]);

	if(!obj_ptr) {
		print(fd, "You don't have that.\n");
		return(0);
	}

	if(F_ISSET(rom_ptr, RPAWNS)) {
		value = MIN(obj_ptr->value/2, 10000L);
		print(fd, "The shopkeep says, \"%I %s worth %ld.\"\n", obj_ptr, 
			(F_ISSET(obj_ptr, ODROPS) || F_ISSET(obj_ptr, OSOMEA)) 
				? "are":"is", value);
	}
	else {
		if(!F_ISSET(obj_ptr, OSILVR)) {
			value = obj_ptr->value/2;
			print(fd, 
				"The shopmaster says, \"%I costs %ld to be repaired.\"\n", 
		    		obj_ptr, value);
		}
		else {
			if(F_ISSET(rom_ptr, RSILVR)) {
				value = obj_ptr->value*2/3;
				print(fd, "The silversmith says, \"%I costs %ld to be "
							"renewed.\"\n", obj_ptr, value);
			}
			else
				print(fd, "The smith says, \"take it to a silversmith for "
							"appraisal, %s.\"\n", F_ISSET(ply_ptr, PMALES) ? 
								"lad":"lass");
		}
	}

	broadcast_rom(fd, ply_ptr->rom_num, "%M gets %i appraised.",
		      ply_ptr, obj_ptr);

	return(0);

}

/**********************************************************************/
/*                              purchase                              */
/**********************************************************************/
/* purchase allows a player to buy an item from a monster.  The       */
/* purchase item flag must be set, and the monster must have an       */
/* object to sell.  The object for sale is determined by the first    */
/* object listed in carried items. 				      */

int purchase(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
	creature    *crt_ptr;
	room        *rom_ptr;
	object	    *obj_ptr, *obj_list[10];
	int	    maxitem = 0, obj_num[10];
	long	    amt;
	int 	    fd, i, j, found = 0, match = 0;
 
	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
 
	if(cmnd->num < 2) {
		print(fd, "Purchase what?\n");
		return(PROMPT);
	}

	if(cmnd->num < 3 ) {
		print(fd, "Syntax: purchase <item> <monster>\n");
		return(PROMPT);
	}
 
	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[2], cmnd->val[2]);
 
	if(!crt_ptr) {
		print(fd, "You cannot purchase from %s.\n", cmnd->str[2]);
		return(PROMPT);
	}
	
	if(!F_ISSET(crt_ptr, MPURIT)) {
		print(fd, "%M cannot sell anything to you.\n", crt_ptr);
		return(PROMPT);
	}

	for(i = 0; i < 10; i++)
		if(crt_ptr->carry[i] > 0) {
		    found = 0;
		    for(j = 0; j < maxitem; j++)
				if(crt_ptr->carry[i] == obj_num[j])
					found = 1;
		    	if(!found) {
					maxitem++;
					obj_num[i] = crt_ptr->carry[i];
		    	}
		}
	if(!maxitem) {
		print(fd, "%M has nothing to sell.\n", crt_ptr);
		return(PROMPT);
	}

	found = 0;

	for(i = 0; i < maxitem; i++) {
		if(!(load_obj(crt_ptr->carry[i], &obj_ptr) < 0))
			if(EQUAL(obj_ptr, cmnd->str[1]) && (F_ISSET(ply_ptr, PDINVI) ?
				1:!F_ISSET(obj_ptr, OINVIS))) { 
					match++;
					if(match == cmnd->val[1]) {
						found = 1;
						break;
					}
			}
	}
 
	if(!found) {
		print(fd, "%M says, \"sorry, I don't have any of those to sell.\"\n",
			crt_ptr);
        return(PROMPT);
	}
 
	amt =  MAX(10, obj_ptr->value * 1);
	if(ply_ptr->gold < amt)
		print(fd, "%M says, \"the price is $%d, and not a gold piece less!\"\n",
			crt_ptr, amt);
	else {
		print(fd, "You pay %m %d gold pieces.\n", crt_ptr,amt);
		print(fd, "%M says, \"thank you very much.  Here is your %s.\"\n",
			crt_ptr, obj_ptr->name);
		broadcast_rom(fd, ply_ptr->rom_num, "%M pays %m $%d for %i.\n", 
			ply_ptr, crt_ptr, amt, obj_ptr);          
		ply_ptr->gold -= amt;
		add_obj_crt(obj_ptr, ply_ptr);  	
		if(crt_ptr->special == CRT_EXITS) {
			print(ply_ptr->fd,
				"%M shakes your hand and leaves quickly.\n", crt_ptr);
			broadcast_rom(ply_ptr->fd, ply_ptr->rom_num,
				"%M shakes the hand of %m and then leaves quickly.\n", 
					crt_ptr, ply_ptr);
			del_active(crt_ptr);
			del_crt_rom(crt_ptr, crt_ptr->parent_rom);
		}
	}

    return(PROMPT);
}                 


/**********************************************************************/
/*                              selection                             */
/**********************************************************************/
/* The selection command  lists all the items  a monster is selling.  */
/* The monster needs the MPURIT flag set to denote it can sell.   */

int selection(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    object	*obj_ptr[10];
	int	obj_list[10];
	int 	fd, i, j, found = 0, maxitem = 0;
 
	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
 
	if(cmnd->num < 2) {
		print(fd, "Ask whom for a selection?\n");
		return(PROMPT);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[1],
		cmnd->val[1]);
 
	if(!crt_ptr) {
		print(fd, "%s does not have a selection.\n", cmnd->str[1]);
		return(PROMPT);
	}

	if(!F_ISSET(crt_ptr, MPURIT)) {
		print(fd, "%M is not selling anything.\n", crt_ptr);
		return(PROMPT);
	}
	
	for (i = 0; i <  10; i++)
		if(crt_ptr->carry[i] > 0) {
		    found = 0;
		    for(j = 0; j < maxitem; j++)
			if(crt_ptr->carry[i] == obj_list[j])
				found = 1;
		    if(!found) {
				maxitem++;
				obj_list[i] = crt_ptr->carry[i];
		    }
		}

	if(!maxitem) {
		print(fd, "%M has nothing to sell.\n", crt_ptr);
		return(PROMPT);
	}

	print(fd, "%M will sell you the folling:\n", crt_ptr);
	for(i = 0; i < maxitem; i++)
		if((load_obj(crt_ptr->carry[i], &(obj_ptr[i])) < 0)  ||
			(crt_ptr->carry[i] == 0))
				print(fd,"%d) Ooops.  Out of it.\n", i + 1);
		else 
			print(fd,"%d) %-22s    %ld gp\n", i + 1, 
				(obj_ptr[i])->name, MAX(10, ((obj_ptr[i])->value)));
	print(fd,"\n"); 

   return(PROMPT);
}                 

/************************************************************************/
/*				trade				     	*/
/************************************************************************/
/*	This function allows a player to trade items with a monster.	*/
/*  The items accepted by the monster are in the first five fields 	*/
/*  of items carried, and the items given in return are in the second	*/
/*  five.								*/

int trade(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
    creature    *crt_ptr;
    room        *rom_ptr;
    object	*obj_ptr, *trd_ptr;
    int         obj_list[5][2];
    int         fd, i, j, found = 0, maxitem = 0;
 
	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
 
	if(cmnd->num < 2) {
		print(fd, "Trade what?\n");
		return(PROMPT);
	}
 
	if(cmnd->num < 3) {
		print(fd, "The proper way to trade is: trade <item> <monster>\n");
		return(PROMPT);
	}
 
	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[2], cmnd->val[2]);
 
	if(!crt_ptr) {
		print(fd, "You cannot trade with %s.\n", cmnd->str[2]);
		return(PROMPT);
	}
 
	if(!F_ISSET(crt_ptr, MTRADE)) {
		print(fd, "%m is not a trader and has nothing to trade.\n", crt_ptr);
		return(PROMPT);
	}
 
	obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj,
		cmnd->str[1], cmnd->val[1]);
 
	if(!obj_ptr) {
		print(fd, "Sorry, you don't have that.\n");
		return(PROMPT);
	}

	for(i = 0; i < 5; i++)
		if(crt_ptr->carry[i] > 0) {
			found = 0;
			for(j = 0; j < maxitem; j++)
				if(crt_ptr->carry[i] == obj_list[j][0])
					found = 1;
			if(!found) {
				maxitem++;
				obj_list[i][0] = crt_ptr->carry[i];
				obj_list[i][1] = crt_ptr->carry[i+5];
			}
		}
 
	if(!maxitem) {
		print(fd, "%M has nothing left to trade.  Perhaps another time.\n",
			crt_ptr);
		return(PROMPT);
	}
	found = 0;

	for(i = 0; i < maxitem; i++) {
		if(load_obj(obj_list[i][0], &trd_ptr) < 0)
			continue;
		if(!strcmp(obj_ptr->name, trd_ptr->name)) {
			found = i + 1;
			break;
		}
	} 

	if(!found || ((obj_ptr->shotscur <= obj_ptr->shotsmax/10)  &&
		(obj_ptr->type != MISC)))
			print(fd,"%M says, \"I have no need of that and don't want it!\"\n",
				crt_ptr);
	else {
		if(crt_ptr->carry[found + 4] == 0) {
			del_obj_crt(obj_ptr, ply_ptr);
			add_obj_crt(obj_ptr, crt_ptr); 
			print(fd, "%M says, \"Thank you!  I sorely desired %i.\n",
				crt_ptr, obj_ptr);
			print(fd, "Sorry, but I have nothing for you.\"\n");
			broadcast_rom(fd, ply_ptr->rom_num, "%M trades %m %i.\n",
				ply_ptr, crt_ptr,obj_ptr);
		}
		else if(!(load_obj((obj_list[found-1][1]), &trd_ptr) < 0)) { 
			if(trd_ptr->questnum && Q_ISSET(ply_ptr, trd_ptr->questnum-1)) {
				print(fd, "You have already fulfilled that quest.\n");
				return(PROMPT);
			}                    

			del_obj_crt(obj_ptr, ply_ptr);
			add_obj_crt(obj_ptr, crt_ptr); 
			add_obj_crt(trd_ptr, ply_ptr); 

			print(fd,"%M says, \"Thank you for retrieving %i for me.\n",
				crt_ptr, obj_ptr);
			print(fd,"For your deed I will reward you.\n");
			print(fd,"%M gives you %i.\n", crt_ptr, trd_ptr);

			if(F_ISSET(ply_ptr, PSHRNK))
				print(fd, "It shrinks as you accept it.\n");
			broadcast_rom(fd, ply_ptr->rom_num, "%M gives %m %i.\n",
				ply_ptr, crt_ptr, obj_ptr);

			if(trd_ptr->questnum) {
				ANSI(fd, YELLOW);
				print(fd, "Quest fulfilled!  Don't drop it.\n");
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
				print(fd, "You won't be able to pick it up again.\n");  
				Q_SET(ply_ptr, trd_ptr->questnum-1);
				ply_ptr->experience += quest_exp[trd_ptr->questnum-1];
				print(fd, "%ld experience granted.\n",
				quest_exp[trd_ptr->questnum-1]);
				add_prof(ply_ptr, quest_exp[trd_ptr->questnum-1]);
			}     
		}
	}

	return(PROMPT);	
}             

