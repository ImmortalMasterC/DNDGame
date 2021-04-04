/*
 *		DM5.C
 *
 *		These files are useful tools for the DM	
 *		Written by:  Roy Wilson  -  styx@styx.darbonne.com
 *		May, 1998										
 *		If you modify or improve on them, I would appreciate a copy.
*/

#include "mstruct.h"
#include "mextern.h"
#include <string.h>

/**************************************************************************/
/*							dm_inventory								  */
/**************************************************************************/
/*  This function allows a DM to list the inventory, including the		  */
/*  contents of any bags or containers, of a specified player.		  	  */ 		

int dm_inventory(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	int			i, n, wt, armwgt, fd, num = 0, len = 0; 
    otag 		*obj, *cnt;
	otag 		*temp, *prev;
	char		numb[16], str[2048];
	object		*obj_ptr;

	fd = ply_ptr->fd;
	str[0] = 0;

	if(ply_ptr->class < CARETAKER)
		return(PROMPT); 

    if(cmnd->num < 2) {
        print(fd,
			"Syntax: *inventory <player> [add <number> | delete <name><#>]\n");
        return(PROMPT);
    }

	lowercize(cmnd->str[1], 1);
    crt_ptr = find_who(cmnd->str[1]);

	if(!crt_ptr) {
		print(fd, "Player not found.\n");
		return(PROMPT);
	}

    if(cmnd->num < 3) {
		if(!strcmp(crt_ptr->name, DMNAME)) {
			ANSI(crt_ptr->fd, RED);
			print(crt_ptr->fd, "%s just listed your inventory.\n", ply_ptr);
			ANSI(crt_ptr->fd, WHITE);
		}

		ANSI(fd, YELLOW);
		print(fd,"%s has %ld gold.  %s inventory is:\n", 
			crt_ptr->name, crt_ptr->gold, 
				F_ISSET(crt_ptr, PMALES) ? "His":"Her");
		ANSI(fd, WHITE);

		obj = crt_ptr->first_obj; 

		if(!obj) { 
			print(fd, "%s isn't holding anything.\n",
		      F_ISSET(crt_ptr, PMALES) ? "He":"She");
			return(PROMPT);
		}

		n = strlen(str);
		if(list_obj(&str[n], ply_ptr, crt_ptr->first_obj) > 0)
			print(fd, "%s.\n", str);

		while(obj) {
			num++;

			if(F_ISSET(obj->obj, OCONTN)) {
				ANSI(fd, GREEN);
				print(fd,"%s: ", obj->obj->name);
				cnt = obj->obj->first_obj;
				ANSI(fd, WHITE);

				if(!cnt) 
					print(fd,"empty.\n");

				else {
					while(cnt) {
						print(fd,"%s%s", cnt->obj->name,
							(cnt->next_tag) ? ", " : ".");
						len += strlen(cnt->obj->name);
						if(len > 40) {
							print(fd,"\n");
							len = 0; 
						}
						cnt = cnt->next_tag;
					}
					len = 0; 
					print(fd,"\n");
				}
			}
			obj = obj->next_tag;    
		}

		armwgt = equip_weight(crt_ptr);
		if(armwgt)
		print(fd, "Weight of armor and wielded weapons is %d pounds.\n", 
			armwgt);

		wt = weight_ply(crt_ptr);

		if(num == 1)
			print(fd, "One item carried, and its weight is %d %s.\n", wt,
				(wt == 1) ? "pound":"pounds");

		else {
			if(num < 21) {
				sprintf(numb, "%s", number[num]);
				numb[0] = up(numb[0]);
			}
			else
				sprintf(numb, "%d", num);
			print(fd, "%s items carried, and total weight is %d pounds.\n", 
				numb, wt);
		}


		ANSI(fd, GREEN);
		print(fd, "Maximum weight %s can carry is %d pounds.\n", 
			crt_ptr, max_weight(crt_ptr));
		ANSI(fd, WHITE);
		return(PROMPT);
	}	

	else {
		if(cmnd->val[2] > 0 && cmnd->str[2][0] == 'a') {

			if(load_obj(cmnd->val[2], &obj_ptr) < 0) {
				print(ply_ptr->fd, "Error (%d)\n", cmnd->val[2]);
				return(0);
			}

			if(F_ISSET(obj_ptr, ORENCH))
				rand_enchant(obj_ptr);
			add_obj_crt(obj_ptr, crt_ptr);
			print(ply_ptr->fd, "%s added to %M's inventory.\n", 
				obj_ptr->name, crt_ptr);
			return(0);
		}

		else if(cmnd->str[2][0] == 'd') {

			if(!strcmp(crt_ptr->name, DMNAME)) {
				ANSI(fd, RED);
				print(crt_ptr->fd, 
					"%M tried to delete an object from your inventory.\n",
						ply_ptr);
				ANSI(fd, BOLD);
				ANSI(fd, WHITE);
				return(0);
			}

			obj_ptr = find_obj(ply_ptr, crt_ptr->first_obj,
				cmnd->str[3], cmnd->val[3]);

			if(!obj_ptr) {
				print(fd, "%s doesn't have a %s.\n",
					F_ISSET(crt_ptr, MMALES) ? "He":"She", cmnd->str[3]) ;
				return(0);
			}

			print(ply_ptr->fd, "%s deleted from %M's inventory.\n", 
				obj_ptr->name, crt_ptr);
			del_obj_crt(obj_ptr, crt_ptr);
			free_obj(obj_ptr);
		}
	}

	return(PROMPT);
}

/****************************************************************************/
/*							dm_spells										*/
/****************************************************************************/
/*  This function allows a DM to list the spells known by a player			*/

int dm_spells(ply_ptr, cmnd)

creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	int			i, j, n, fd;  
	char		numb[16], str[2048];
	char		spl[128][20];

	fd = ply_ptr->fd;
	str[0] = 0;

	if(ply_ptr->class < CARETAKER)
		return(PROMPT); 

    if(cmnd->num < 2) {
        print(fd,"syntax: *spells <player>\n");
        return(PROMPT);
    }

	if(cmnd->str[1][0] == '.')
		strcpy(cmnd->str[1], ply_ptr->name);
    lowercize(cmnd->str[1], 1);
    crt_ptr = find_who(cmnd->str[1]);

	if(!crt_ptr) {
		print(fd, "Player not found.\n");
		return(PROMPT);
	}

	strcpy(str, "\n  ");
	for(i = 0, j = 0; i < 128; i++)
		if(S_ISSET(crt_ptr, i))
			strcpy(spl[j++], spllist[i].splstr);

	if(!j)
		strcat(str, "None.");
	else {
		qsort((void *)spl, j, 20, strcmp);
		for(i = 0; i < j; i++) {
			strcat(str, spl[i]);
			strcat(str, ", ");
		}
		str[strlen(str) - 2] = '.';
		str[strlen(str) - 1] = 0;
	}

	if(j < 21) {
		sprintf(numb, "%s", number[j]);
		numb[0] = up(numb[0]);
	}
	else
		sprintf(numb, "%d", j);
	ANSI(fd, YELLOW);
	print(fd, " %s spells known by %s:", numb, crt_ptr);
	ANSI(fd, WHITE);
	print(fd, "%s\n", str);

	return(PROMPT);
}

/****************************************************************************/
/*							dm_quests										*/
/****************************************************************************/
/*  This function allows a DM to list the quests fulfilled by a player		*/

int dm_quests(ply_ptr, cmnd)

creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	int			i = 1, fd, quests = 0;  
	char		temp[16], qflags[2048], str[2048];

	fd = ply_ptr->fd;
	temp[0] = 0;
	qflags[0] = 0;
	str[0] = 0;

	if(ply_ptr->class < CARETAKER)
		return(PROMPT); 

    if(cmnd->num < 2) {
        print(fd,"syntax: *quests <player>\n");
        return(PROMPT);
    }

	if(cmnd->str[1][0] == '.')
		strcpy(cmnd->str[1], ply_ptr->name);
    lowercize(cmnd->str[1], 1);
    crt_ptr = find_who(cmnd->str[1]);

	if(!crt_ptr) {
		print(fd, "Player not found.\n");
		return(PROMPT);
    }

	ANSI(fd, YELLOW);
	print(fd, "Quest flags set for %s:  ", crt_ptr);
	ANSI(fd, WHITE);

	for(i = 0; i < 128; i++) {
		if(Q_ISSET(crt_ptr, i)) {
			sprintf(temp, "%d, ", i + 1);
			strcat(qflags, temp);
			temp[0] = 0;
			quests++;
    	}
    }

	qflags[strlen(qflags) - 2] = '.';
	qflags[strlen(qflags) - 1] = 0;
	print(fd, "%s\n", qflags);

	if(quests) {
		ANSI(fd, YELLOW);
		print(fd, "%d quests fulfilled.\n", quests);
		ANSI(fd, WHITE);
    }
	else
		print(fd, "None.\n");

	return(PROMPT);
}
