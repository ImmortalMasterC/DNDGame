/*
 * DM8.C:
 *
 *  DM functions
 *
 *  1991, 1992, 1993 	Brett J. Vickers
 *  1994, 1995	   	Brooke Paul
 *  1996	   	Brooke Paul & Eric Krichbaum
 *  2000		Roy Wilson
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>

/************************************************************************/
/*			dm_advance					*/
/************************************************************************/

int dm_advance(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
   	int 		fd, lev, i;
	creature 	*crt_ptr = 0;
	 
	fd = ply_ptr->fd;
 
	if(ply_ptr->class < CARETAKER)
		return(PROMPT);

	if(cmnd->num < 2) {
		print(fd, "Advance whom to what level?\n");
		return(0);
	}

	cmnd->str[1][0] = up(cmnd->str[1][0]);
	for(i = 0; i < Tablesize; i++) {
		if(!Ply[i].ply) continue;
		if(Ply[i].ply->fd == -1) continue;
		if(!strncmp(Ply[i].ply->name, cmnd->str[1], strlen(cmnd->str[1])))
			crt_ptr = Ply[i].ply;
		if(!strcmp(Ply[i].ply->name, cmnd->str[1]))
		break;
	}

	if(!crt_ptr) {
		print(fd, "Advance whom?\n");
		return(0);
	}

	if(cmnd->val[1] <= 0 || cmnd->val[1] > MAXALVL) {
		print(fd, "Only levels between 1 and 25!\n");
		return(0);
	}

	lev = cmnd->val[1] - crt_ptr->level;
	if(lev == 0) {
		print(fd, "But %M is already level %d.\n", 
			crt_ptr, crt_ptr->level);
		return(0);
	}

	if(lev > 0) {
		for(i = 0; i < lev; i++) up_level(crt_ptr);
		print(fd, "%M has been raised to level %d.\n", 
			crt_ptr, crt_ptr->level);
	}

	if(lev < 0) {
		for(i = 0; i > lev; i--) down_level(crt_ptr);
		print(fd, "%M has been lowered to level %d.\n", 
			crt_ptr, crt_ptr->level);
	}
}

/************************************************************************/
/*                              dm_experience                           */
/************************************************************************/

int dm_exp(ply_ptr, cmnd)
creature    *ply_ptr;
cmd         *cmnd;

{
    int         fd, lev, i, not_logged;
	long		expneeded, diff;
    creature    *crt_ptr = 0;

    fd = ply_ptr->fd;

    if(ply_ptr->class < CARETAKER)
        return(PROMPT);

	if(cmnd->num < 2) {
        print(fd, "Determine the experience of which player?\n");
		return(0);
	}

  	cmnd->str[1][0] = up(cmnd->str[1][0]);
    	for(i = 0; i < Tablesize; i++) {
        	if(!Ply[i].ply) continue;
        	if(Ply[i].ply->fd == -1) continue;
        	if(!strncmp(Ply[i].ply->name, cmnd->str[1], strlen(cmnd->str[1])))
            	crt_ptr = Ply[i].ply;
        	if(!strcmp(Ply[i].ply->name, cmnd->str[1]))
       		break;
    	}

    if(!crt_ptr) {
        print(fd, "Player not found.\n");
        return(0);
    }


	if(!strcmp(crt_ptr->name, DMNAME)) {
		print(fd, "Styx has an infinite amount of experience.\n");
		return(0);
	}
	else {
		print(fd, "%M is level %d and has %d experience points.\n", 
			crt_ptr, crt_ptr->level, crt_ptr->experience);
		expneeded = needed_exp[crt_ptr->level - 1];
		diff = expneeded - crt_ptr->experience;
		if(diff >= 0) {
			print(fd, "%d still needed for level %d.\n", 
				diff, crt_ptr->level + 1); 
		}
		else {
			for(i = 0; i < 26; i++) {
				expneeded = needed_exp[crt_ptr->level + i];
				diff = crt_ptr->experience - expneeded;
				if(diff >= 0)
					print(fd, "%d excess for level %d.\n", 
						diff, (crt_ptr->level + 2 + i)); 
				else break;
			}
		}

	}

	return(0);
}



/************************************************************************/
/*                              dm_bank_balance                         */
/************************************************************************/

int dm_bal(ply_ptr, cmnd)
creature    *ply_ptr;
cmd         *cmnd;

{
    int         fd, i, not_logged;
    creature    *crt_ptr = 0;

    fd = ply_ptr->fd;

    if(ply_ptr->class < CARETAKER)
        return(PROMPT);

	if(cmnd->num < 2) {
        print(fd, "Determine the bank balance for which player?\n");
		return(0);
	}


   cmnd->str[1][0] = up(cmnd->str[1][0]);
        for(i = 0; i < Tablesize; i++) {
            if(!Ply[i].ply) continue;
            if(Ply[i].ply->fd == -1) continue;
            if(!strncmp(Ply[i].ply->name, cmnd->str[1], strlen(cmnd->str[1])))
                crt_ptr = Ply[i].ply;
            if(!strcmp(Ply[i].ply->name, cmnd->str[1]))
            break;
        }

    if(!crt_ptr) {
        print(fd, "Player not found.\n");
        return(0);
    }


	if(!strcmp(crt_ptr->name, DMNAME)) {
		print(fd, "Why do you want to know?\n");
		return(0);
	}
	else {
		if(crt_ptr->bank_bal > 0) {
			print(fd, "%M is level %d and has a bank balance of $%d and $%d in gold.\n", 
				crt_ptr, crt_ptr->level, crt_ptr->bank_bal, crt_ptr->gold);
		}
		if(crt_ptr->bank_bal == 0) {
			print(fd, "%M is level %d and does not have a bank account, but has $%d in gold.\n", 
				crt_ptr, crt_ptr->level, crt_ptr->gold);
		}
		if(crt_ptr->bank_bal < 0) {
			print(fd, "%M is level %d and has a bank loan of $%d and $%d in gold.\n", 
				crt_ptr, crt_ptr->level, crt_ptr->bank_bal, crt_ptr->gold);
		}
	}

	return(0);
}

