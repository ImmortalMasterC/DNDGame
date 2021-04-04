/*
 * COMMAND15.C:
 *
 *
 */

#include "mstruct.h"
#include "mextern.h"


/************************************************************************/
/*									Pay 						        */
/************************************************************************/
/* This function allows a player to hire a porter or pay a gatekeeper.  */


int pay(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
    object      *obj_ptr;
    creature    *crt_ptr;
    room        *rom_ptr;
    int         fd, amt, num, found = 0;
	long		amount;
	char		numb[16];
	xtag        *xp;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;

	if(cmnd->num < 3) {
		print(fd, "Pay how much and to whom?\n");
		return(0);
	}

	if(cmnd->str[1][0] != '$') {
		print(fd, "You need to pay with gold.\n");
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, 
		cmnd->str[2], cmnd->val[2]);
	if(!crt_ptr) {
		print(fd, "There is no one here as you describe.\n");
		return(0);
	}



/*--------------------------------------------------------------------*/
/* This routine allows the player to pay a gate keeper an amount of   */
/* money equal to it's gold so that it will unlock and open an exit.  */


	if(F_ISSET(crt_ptr, MGATEK)) {
		amount = atol(&cmnd->str[1][1]);
		if(amount < 1) {
			print(fd, "Get real, cheapskate.\n");
			return(0);
		}
		if(amount > ply_ptr->gold) {
			print(fd, "You don't have that much gold.\n");
			return(0);
		}

		if(amount < crt_ptr->gold) {
			print(fd, "%M ignores the small amount you have offered.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M sneers at %m and mutters, \"cheapskate.\"\n", crt_ptr, ply_ptr);
			return(0);
		}

		else {
			xp = rom_ptr->first_ext;
			while(xp) {
				if(F_ISSET(xp->ext, XMONEY)) {
					found = 1;
					break;
				}
				xp = xp->next_tag;
			}
		}

		if(!found) {
			print(fd, "%M looks confused and ignores your offer.\n", crt_ptr);
			return(0);
		}

 		if(F_ISSET(xp->ext, XLOCKD)) {
			print(fd, "%M unlocks and opens the %s for you.\"\n", crt_ptr, xp->ext->name);
			print(fd, "The %s swings open.\n", xp->ext->name);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M unlocks and opens the %s for %m.", crt_ptr, xp->ext->name, ply_ptr);
			broadcast_rom(fd, ply_ptr->rom_num,
				"The %s swings open.\n", xp->ext->name);
			F_CLR(xp->ext, XLOCKD);
			F_CLR(xp->ext, XCLOSD);
			ply_ptr->gold -= amount;
		}
		else
			print(fd, "%M says, \"The %s is open.  Please go in.\"\n",
				crt_ptr, xp->ext->name);

		return(0);
	}

/*--------------------------------------------------------------------*/
/* This routine allows the player to hire a porter to carry items     */

	if(crt_ptr->special != CRT_PORTR) {
		print(fd, "%M is not for hire.\n", crt_ptr);
		return(0);
	}

	amt = atol(&cmnd->str[1][1]);
    if(amt < 1) {
        print(fd, "Cheater!  I should tell Styx.\n");
        return;
    }
    if(amt > ply_ptr->gold) {
        print(fd, "You don't have that much gold.\n");
        return;
    }
    if(amt < 5) {
        print(fd, "The porter's guild has a minimum charge of $5 gold.\n");
        return;
    }
	if(crt_ptr->mpcur == crt_ptr->mpmax) {
	    print(fd, "%M says, \"Thank you, but I've already been paid my full fee.\"\n", crt_ptr);
        return;
    }

	if(F_ISSET(crt_ptr, MFOLLO)) {
		if(strcmp(crt_ptr->key[1], ply_ptr->password)) {
	    	print(fd, "%M has already been hired.\n", crt_ptr);
        	return;
    	}
    }

	if(amt/5 < crt_ptr->mpmax) { 
		crt_ptr->mpcur += amt/5;
		if(crt_ptr->mpcur > crt_ptr->mpmax)
			crt_ptr->mpcur = crt_ptr->mpmax;
	}
	else {
		crt_ptr->mpcur = crt_ptr->mpmax;
		print(fd, "%M says, \"Thanks for the extra tip!\"\n", crt_ptr);
	}
	num = crt_ptr->mpcur;
	sprintf(numb, "%s", number[num]);

	if(strcmp(crt_ptr->key[1], ply_ptr->password)) {
		print(fd, "%M says, \"Give me the items you want me to carry.\"\n", crt_ptr); 
		if(num == 1)
			print(fd, "%M says, \"For a measly $5 I will follow you once.\"\n", crt_ptr); 
		else {
			print(fd, "%M says, \"For $%d I will follow you ", crt_ptr, amt);
			if(num < 21) 
				print(fd, "%s places.\"\n", numb); 
			else
				print(fd, "%d places.\"\n", num);
		}
		print(fd, "For return of your goods, whisper your password to %m.\n", crt_ptr);
	}
	else {
		print(fd, "%M says, \"Now I will follow you ", crt_ptr);
			if(num < 21) 
				print(fd, "%s places.\"\n", numb); 
			else
				print(fd, "%d places.\"\n", num);
	}
	broadcast_rom(fd, ply_ptr->rom_num, "%M pays %m to follow him.\n", ply_ptr, crt_ptr);

	strcpy(crt_ptr->key[1], ply_ptr->password);
	F_SET(crt_ptr, MNOEXT);
	crt_ptr->gold += amt;
    ply_ptr->gold -= amt;

    print(fd, "You have %ld gold pieces left.\n", ply_ptr->gold);
    if(F_ISSET(ply_ptr, PSHRNK) && !F_ISSET(crt_ptr, PSHRNK))
        print(fd, "The gold returns to normal size as you release it.\n");
	if(!F_ISSET(ply_ptr, PDMINV) && (!F_ISSET(ply_ptr, PINVIS) && 
		(!F_ISSET(ply_ptr, PCLOAK))))
        broadcast_rom(fd, ply_ptr->rom_num,
            "%M gave %m %ld gold pieces.", ply_ptr, crt_ptr, amt);
	else
		broadcast_rom(fd, ply_ptr->rom_num,
            "An invisible presence speaks to %m.", crt_ptr);

	F_CLR(ply_ptr, PHIDDN);

	return(0);

}


/**********************************************************************/
/*                                  Porter_return                     */
/**********************************************************************/

int porter_return(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
	otag        *op, *temp;
    object      *obj_ptr;
    creature    *crt_ptr;
    room        *rom_ptr;
    int         fd;

    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;


    if(cmnd->num < 3) {
        print(fd, "Whisper what to whom?\n");
        return(0);
    }

    crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
        cmnd->str[2], cmnd->val[2]);
    if(!crt_ptr) {
        print(fd, "I don't see that here.\n");
        return(0);
    }

    if(crt_ptr->special != CRT_PORTR) {
        print(fd, "%M does not understand you.\n", crt_ptr);
        return(0);
    }

    if(!strcmp(cmnd->str[1], "follow")) {
		if(!strcmp(crt_ptr->key[1], ply_ptr->password)) {
			F_SET(crt_ptr, MFOLLO);
			print(fd, "%M nods his head.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M tells %m to follow %s.\n", ply_ptr, crt_ptr, 
					F_ISSET(ply_ptr, PMALES) ? "him":"her"); 
    	}
		else
			print(fd, "%s is not your porter.\n", F_ISSET(crt_ptr, MMALES) ? "He":"She");
       	return(0);
    }

    if(!strcmp(cmnd->str[1], "wait")) {
		if(!strcmp(crt_ptr->key[1], ply_ptr->password)) {
			F_CLR(crt_ptr, MFOLLO);
			print(fd, "%M sits down to wait.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M sits down to wait for %m.\n", crt_ptr, ply_ptr); 
    	}
		else
			print(fd, "%s is not your porter.\n", F_ISSET(crt_ptr, MMALES) ? "He":"She");
       	return(0);
    }

    if(strcmp(cmnd->str[1], ply_ptr->password)) {
        print(fd, "Try again.\n");
        return(0);
    }

	if(!strcmp(crt_ptr->key[1], ply_ptr->password)) {
		F_CLR(crt_ptr, MFOLLO);
		F_CLR(crt_ptr, MNOEXT);
		crt_ptr->mpcur = 0;
		print(fd, "%M tips his cap and drops his burden.\n", crt_ptr);
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M drops baggage belonging to %m.\n", crt_ptr, ply_ptr); 

		op = crt_ptr->first_obj;
		while(op) {
			temp = op->next_tag;
			obj_ptr = op->obj;
			del_obj_crt(obj_ptr, crt_ptr);
			if(!F_ISSET(crt_ptr->parent_rom, RSLIME))
			add_obj_rom(obj_ptr, crt_ptr->parent_rom);
			op = temp;
		}
/*
		del_crt_rom(crt_ptr, rom_ptr);
		del_active(crt_ptr);
		free_crt(crt_ptr);
*/
	}
	else if(F_ISSET(crt_ptr, MFOLLO)){
		print(fd, "%M shakes his head and says, \"That's not your password.\"\n", crt_ptr);
	}
	else {
		print(fd, "%M says, \"Sorry, I'm not your porter and don't have your baggage.\"\n", 
			crt_ptr);
	}


	return(0);
}

