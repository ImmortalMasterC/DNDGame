/*
 * TALK.C:
 *
 *
 *	The Talk-Action Routine Written by Steve Smith
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>
#include <stdlib.h>


/**************************************************************************/
/*				talk				          */
/**************************************************************************/
/* This function allows players to speak with monsters if the monster 	  */
/* has a talk string set and is not silenced.			      	  */

int talk(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room		*rom_ptr;
	creature	*crt_ptr;
	ttag		*tp;
	char		str[160];
	int		fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(F_ISSET(ply_ptr, PHIDDN)) {
		F_CLR(ply_ptr, PHIDDN);
		print(fd, "You step from the shadows as you speak.\n");
		broadcast_rom(fd, ply_ptr->rom_num, 
			"%M steps away from the shadows.\n", ply_ptr); 
	}

	if(cmnd->num < 2) {
		print(fd, "Talk to whom?\n");
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		print(fd, "I don't see that here.\n");
		return(0);
	}


	if(cmnd->num == 2 || !F_ISSET(crt_ptr, MTALKS)) {
		broadcast_rom(fd, ply_ptr->rom_num, "%M talks with %m.", 
			ply_ptr, crt_ptr);

		if(!crt_ptr->talk[0]) {
			print(fd, "%M has nothing to say.\n", crt_ptr);
			return(PROMPT);
		}
 
		if(F_ISSET(crt_ptr, MTLKAG))
			add_enm_crt(ply_ptr->name, crt_ptr);

		if(crt_ptr->talk[0] && 
			!F_ISSET(crt_ptr, MSILNC) && 
			!F_ISSET(crt_ptr, MTALKR) &&
			!F_ISSET(crt_ptr, MTALKD)) {
				if(!F_ISSET(crt_ptr, MFROZE)) {
					broadcast_rom(fd, ply_ptr->rom_num,
						"%M says to %m, \"%s\"\n", crt_ptr, ply_ptr, 
								crt_ptr->talk);
					print(fd, "%M says to you, \"%s\"\n", crt_ptr, 
								crt_ptr->talk);
				}
				else {
					broadcast_rom(fd, ply_ptr->rom_num,
						"%M tries to talk to %m but is blue with cold.\n", 
							crt_ptr, ply_ptr);
					print(fd, "%M is too frozen to speak.\n", crt_ptr);
				}
		}
		else {
			if(F_ISSET(crt_ptr, MSILNC)) {
				broadcast_rom(-1, ply_ptr->rom_num, 
					"%M doesn't say anything to %m.", crt_ptr, ply_ptr);
				broadcast_rom(-1, ply_ptr->rom_num,
				   "Someone has cast a silence spell on %M.\n", crt_ptr);
				if(F_ISSET(crt_ptr, MTLKAG) && ply_ptr->class != RANGER)
					add_enm_crt(ply_ptr->name, crt_ptr);
				return(PROMPT);
			}

			if(crt_ptr->talk[0] && F_ISSET(crt_ptr, MTALKD) && 
				(ply_ptr->class == DRUID || 
				 ply_ptr->class == RANGER || 
				 ply_ptr->class >= CARETAKER)) {
				broadcast_rom(-1, ply_ptr->rom_num, 
					"%M says something quietly in a strange tongue.", crt_ptr);
				print(fd, "%M says to you, \"%s\"\n", crt_ptr, crt_ptr->talk);
				return(PROMPT);
			}
			else if(crt_ptr->talk[0] && F_ISSET(crt_ptr, MTALKD)) {
				print(fd, "%M says something in the %s language.\n", 
					crt_ptr, crt_ptr->name);
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M says something in an odd language to %m.\n",
						crt_ptr, ply_ptr);
				if(F_ISSET(crt_ptr, MTLKAG) && ply_ptr->class != RANGER)
					add_enm_crt(ply_ptr->name, crt_ptr);
			}
			
			if(crt_ptr->talk[0] && F_ISSET(crt_ptr, MTALKR) && 
				(ply_ptr->class == RANGER || ply_ptr->class >= CARETAKER)) {
				broadcast_rom(-1, ply_ptr->rom_num, 
					"%M replies in a strange tongue.", crt_ptr);
				print(fd, "%M says to you, \"%s\"\n", crt_ptr, crt_ptr->talk);
				return(PROMPT);
			}
			else if(crt_ptr->talk[0] && F_ISSET(crt_ptr, MTALKR)) {
				print(fd, "%M mumbles in an odd language.\n", crt_ptr);
				broadcast_rom(fd, ply_ptr->rom_num, 
					"%M mumbles something in a strange language to %m.\n",
						crt_ptr, ply_ptr);
				if(F_ISSET(crt_ptr, MTLKAG) && ply_ptr->class != RANGER)
					add_enm_crt(ply_ptr->name, crt_ptr);
			}
			return(PROMPT);

		}
	}
	else {
		if(!crt_ptr->first_tlk)
			if(!load_crt_tlk(crt_ptr))
				return(PROMPT);

		if(F_ISSET(crt_ptr, MPASSW)) {
			print(fd, "You speak softly to %m.\n", crt_ptr);
			broadcast_rom(fd, ply_ptr->rom_num, 
				"There is whispering between %M and %m.", crt_ptr, ply_ptr);
		}

		else {
			broadcast_rom(fd, ply_ptr->rom_num, "%M asks %m about \"%s\".",
				ply_ptr, crt_ptr, cmnd->str[2]);
		}

		if(F_ISSET(crt_ptr, MFROZE)) {
			print(fd, 
				"%M is suffering from a freeze spell and cannot answer you.\n");
			broadcast_rom(fd, ply_ptr->rom_num, 
				"%M has turned blue from a freeze spell, and cannot answer %m.",
					crt_ptr, ply_ptr); 
			return(PROMPT);
		}

		if(!F_ISSET(crt_ptr, MSILNC)) {
			tp = crt_ptr->first_tlk;
			while(tp) {
				if(!strcmp(cmnd->str[2], tp->key)) {
					broadcast_rom(fd, ply_ptr->rom_num, 
						"%M says to %m, \"%s\"\n", crt_ptr, ply_ptr, 
							tp->response);
					print(fd, "%M says to you, \"%s\"\n", crt_ptr, 
						tp->response);
					talk_action(ply_ptr, crt_ptr, tp);
					return(0);
				}
				tp = tp->next_tag;
			}
		}

		broadcast_rom(-1, ply_ptr->rom_num, "%M shrugs.", crt_ptr);
		if(F_ISSET(crt_ptr, MSILNC)) {
			broadcast_rom(-1, ply_ptr->rom_num,
			   "Someone has cast a silence spell on %M.\n", crt_ptr);
		}
		if(F_ISSET(crt_ptr, MTLKAG) && ply_ptr->class != RANGER)
			add_enm_crt(ply_ptr->name, crt_ptr);
	}

	return(0);
}

/************************************************************************/
/*                    		  talk_action                           */
/************************************************************************/
/* The talk_action function handles a monster's action when a player    */
/* asks the monster about a key word.  The action is defined in the 	*/
/* monster's talk file.  A monster can attack, cast spells, give items  */
/* or perform social commands which are triggered by key words given by */
/* a player.			  												*/


void talk_action(ply_ptr, crt_ptr,tt)

creature	*ply_ptr;
creature	*crt_ptr;
ttag		*tt;

{
	room		*rom_ptr;
	cmd			cm;
	int 		i, n, qst = 0, splno = 0;
	object  	*obj_ptr;
	int			(*fn)();

	for (i = 0; i < COMMANDMAX; i++) {
		cm.str[i][0]  = 0;
		cm.str[i][24] = 0;
		cm.val[i]     = 0;
	}
	cm.fullstr[0] = 0;
	cm.num = 0;


	switch(tt->type) {
		case 1: 			/*attack */
			add_enm_crt(ply_ptr->name, crt_ptr);
			F_CLR(crt_ptr, MHIDDN);
			broadcast_rom(ply_ptr->fd, ply_ptr->rom_num, "%M attacks %M\n",
				crt_ptr, ply_ptr);
			print(ply_ptr->fd,"%M attacks you.\n", crt_ptr);
			if(F_ISSET(crt_ptr, MSILNC)) {
				broadcast_rom(-1, ply_ptr->rom_num,
				   "Someone has cast a silence spell on %M.\n", crt_ptr);
				return;
			}

		case 2:				/*action command */
			if(action) {
				strncpy(cm.str[0], tt->action, 25);
				strcat(cm.fullstr, tt->action);
				cm.val[0]  = 1;
				cm.num = 1;
				if(tt->target)
					if(!strcmp(tt->target, "PLAYER")) {
						strcpy(cm.str[1], ply_ptr->name);
						strcat(cm.fullstr, " ");
						strcat(cm.fullstr, ply_ptr->name);
						cm.val[1]  = 1;
						cm.num = 2;
					}
				if(F_ISSET(crt_ptr, MSILNC)) {
					broadcast_rom(-1, ply_ptr->rom_num,
					   "Someone has cast a silence spell on %M.\n", crt_ptr);
					return;
				}
				action(crt_ptr, &cm);
			}
			break;

		case 3:				/*cast */
			if(tt->action) {
				n =  crt_ptr->mpcur; 
				strcpy(cm.str[0], "cast");
				strncpy(cm.str[1], tt->action, 25);
				strcpy(cm.str[2], ply_ptr->name);
				cm.val[0]  = cm.val[1]  = cm.val[2]  = 1;
				cm.num = 3;
				sprintf(cm.fullstr, "cast %s %s",
				tt->action, ply_ptr->name);
		    	i = 0;
				do {
		   	   		if(!strcmp(tt->action, spllist[i].splstr)) {
           		   		splno = i;
           	 	   		break;
        		   	}
       	 			i++;
    			} while(spllist[i].splno != -1);

				if(spllist[i].splno == -1)
					return;
		 		fn = spllist[splno].splfn;
				if(fn == offensive_spell) {
					for(i = 0; ospell[i].splno != spllist[splno].splno; i++)
						if(ospell[i].splno == -1) return;
				 		(*fn)(crt_ptr, &cm, CAST, spllist[splno].splstr,
            				&ospell[i]);
    			}
				else if(is_enm_crt(ply_ptr->name, crt_ptr)) {
					print(ply_ptr->fd,"%M refuses to cast any spells on you.\n",
						crt_ptr);
					return;
				}	
				else
			  		(*fn)(crt_ptr, &cm, CAST);
			
				if(spllist[i].splno  && n < crt_ptr->mpcur)
					print(ply_ptr->fd,
						"%M apologizes that %s cannot currently cast that "
							"spell on you.\n", crt_ptr, 
								(F_ISSET(crt_ptr, MMALES)) ? "he" : "she");
			}
			break;

		case 4:				/* give item to player */
			i = atoi(tt->action);
			if(i > 0) {
				n = load_obj(i, &obj_ptr);

            if(n > -1) {
                if(F_ISSET(obj_ptr, ORENCH))
                    rand_enchant(obj_ptr);
				if(weight_ply(ply_ptr) + weight_obj(obj_ptr) > 
					max_weight(ply_ptr)) {
						print(ply_ptr->fd, "You can't hold anymore.\n");
						break;
				}    
	        	if(obj_ptr->questnum && Q_ISSET(ply_ptr, obj_ptr->questnum-1)) {
               		print(ply_ptr->fd, "You may not have that. %s.\n",
               			"You have already fulfilled that quest");
					qst = 1;
					break;
                } 
	     		if(obj_ptr->questnum) {
					ANSI(ply_ptr->fd, YELLOW);
   	                print(ply_ptr->fd, "Quest fulfilled!\n");
   	                Q_SET(ply_ptr, obj_ptr->questnum-1);
   	                ply_ptr->experience += quest_exp[obj_ptr->questnum-1];
   	                print(ply_ptr->fd, "%ld experience granted.\n",
                     	quest_exp[obj_ptr->questnum-1]);
   	                add_prof(ply_ptr, quest_exp[obj_ptr->questnum-1]);
					ANSI(ply_ptr->fd, BOLD);
					ANSI(ply_ptr->fd, WHITE);
   	             } 
				add_obj_crt(obj_ptr, ply_ptr);
				print(ply_ptr->fd, "%M gives you %i.\n",crt_ptr, obj_ptr);
				broadcast_rom(ply_ptr->fd, ply_ptr->rom_num,
					"%M gives %m %i.\n", crt_ptr, ply_ptr, obj_ptr);
				if(obj_ptr->questnum) 
					print(ply_ptr->fd, "Don't drop it.  You won't be able "
										"to pick it up again.\n");
			}
			else
				print(ply_ptr->fd,"%M has nothing to give you.\n",crt_ptr);
        }  
		break;
		default:
		break;
	}

	if(crt_ptr->special == CRT_EXITS && qst == 0) {
		del_active(crt_ptr);
		del_crt_rom(crt_ptr, crt_ptr->parent_rom);
		print(ply_ptr->fd,
			"%M shakes your hand and is gone in a burst of energy.\n", 
				crt_ptr);
		broadcast_rom(ply_ptr->fd, ply_ptr->rom_num,
			"%M shakes the hand of %m and disappears in a flash of energy.\n", 
				crt_ptr, ply_ptr);
	}

return;
}

/**********************************************************************/
/*			say					      */
/**********************************************************************/
/* This function allows the player specified by the socket descriptor */
/* in the first parameter to say something to all the other people in */
/* the room.							  */

int say(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room		*rom_ptr;
	int		index = -1, i, j, len, fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	len = strlen(cmnd->fullstr);
	for(i = 0; i < len && i < 256; i++) {
		if(cmnd->fullstr[i] == ' ') {
			index = i + 1;
			break;
		}
	}

	cmnd->fullstr[255] = 0;
	/* Check for modem escape code */
	for(j = 0; j < len && j < 256; j++) {
		if(cmnd->fullstr[j] == '+' && cmnd->fullstr[j + 1] == '+') {
			index = -1;
			break;
		}
	}

	if(index == -1 || strlen(&cmnd->fullstr[index]) < 1) {
		print(fd, "Say what?\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PSILNC)) {
		print(fd, "Your lips move but no sound comes forth.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);
	if(F_ISSET(ply_ptr, PLECHO)) {
		ANSI(fd, CYAN);
		print(fd, "You say, \"%s\"\n", &cmnd->fullstr[index]);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
	}
	else
	print(fd, "Ok.\n");
	broadcast_rom(fd, rom_ptr->rom_num, "%M says, \"%s\"", 
		ply_ptr, &cmnd->fullstr[index]);

	return(0);

}

/**********************************************************************/
/*			yell					      */
/**********************************************************************/
/* This function allows a player to yell something that will be heard */
/* not only in his room, but also in all rooms adjacent to him.  In   */
/* the adjacent rooms, however, people will not know who yelled.      */

int yell(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room	*rom_ptr;
	xtag	*xp;
	char	str[300];
	int	index = -1, i, j, fd;
	int	len;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	len = strlen(cmnd->fullstr);
	for(i = 0; i < len; i++) {
		if(cmnd->fullstr[i] == ' ') {
			index = i+1;
			break;
		}
	}

/* Check for modem escape code */

	str[256] = 0;
	for(j = 0; j < len && j < 256; j++) {
		if(cmnd->fullstr[j] == '+' && cmnd->fullstr[j+1] == '+') {
			index = -1;
			break;
		}
	}

	if(F_ISSET(ply_ptr, PSILNC)) {
		print(fd, "Your voice is too weak to yell.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PFROZE)) {
		print(fd, "You're frozen and can't open your mouth enough to yell.\n");
		return(0);
	}

	if(index == -1 || strlen(&cmnd->fullstr[index]) < 1) {
		print(fd, "Yell what?\n");
		return(0);
	}

	print(fd, "YOU YELLED!\n");

	if(F_ISSET(ply_ptr, PHIDDN)) {
		F_CLR(ply_ptr, PHIDDN);
		print(fd, "You step from the shadows as you yell.\n");
	}

	broadcast_rom(fd, rom_ptr->rom_num, "%M yells, \"%s\"", 
		ply_ptr, &cmnd->fullstr[index]);

	sprintf(str, "Someone yells, \"%s\"", &cmnd->fullstr[index]);

	xp = rom_ptr->first_ext;
	while(xp) {
		if(is_rom_loaded(xp->ext->room))
			broadcast_rom(fd, xp->ext->room, "%s", str);
		xp = xp->next_tag;
	}

	return(0);
}


/************************************************************************/
/*                          Group_talk                                  */
/************************************************************************/
/* Group talk allows a player to broadcast a message to everyone in     */
/* their group.  gtalk first checks to see if the player is following   */
/* another player; in that case, the group talk message will be sent    */
/* every member in the leader's group.  If the player isn't following   */
/* then it is assumed the player is a leader and the message will be    */
/* sent to all the followers in the group. 				*/
 
int gtalk(ply_ptr, cmnd)
creature    *ply_ptr;
cmd    	    *cmnd;

{
    creature *leader;
    ctag    *cp;
    int     found = 0, i, j, fd;
    int     len;

    fd = ply_ptr->fd;

if(ply_ptr->following)
{
        leader = ply_ptr->following;
        cp = ply_ptr->following->first_fol;
}
    else
{
        leader = ply_ptr;
        cp = ply_ptr->first_fol;
}

    if(!cp) {
        print(fd, "You are not in a group.\n");
        print(fd, "You must either follow someone or they must follow you.\n");
        return(0);
    }

    len = strlen(cmnd->fullstr);
    for(i = 0; i < len && i < 256; i++) {
        if(cmnd->fullstr[i] == ' ' && cmnd->fullstr[i+1] != ' ')
            found++;
        if(found == 1) break;
    }
    cmnd->fullstr[255] = 0;
	/* Check for modem escape code */
	for(j = 0; j < len && j < 256; j++) {
		if(cmnd->fullstr[j] == '+' && cmnd->fullstr[j + 1] == '+') {
			found=0;
			break;
		}
	}

    if(found < 1 || strlen(&cmnd->fullstr[i + 1]) < 1) {
        print(fd, "Say what?\n");
        return(0);
    } 

    if(F_ISSET(ply_ptr, PSILNC)) {
        print(fd, "You can't seem to form the words.\n");
        return(0);
    } 

    found = 0;          /*if only inv dm in group */

    while(cp) {
        if((F_ISSET(cp->crt, PIGNOR) && (ply_ptr->class < CARETAKER)) &&
            !F_ISSET(cp->crt, PDMINV))
            	print(fd, "%s is ignoring everyone.\n", cp->crt->name);
        else
            print(cp->crt->fd, "### %M group mentioned, \"%s\"\n", 
                ply_ptr, &cmnd->fullstr[i + 1]);

        if(!F_ISSET(cp->crt, PDMINV))
            found = 1;
        cp = cp->next_tag;
    }

    if(!found) {
        print(fd, "You are not in a group.\n");
        return(0);
    }            
        
	if((F_ISSET(leader, PIGNOR) && (ply_ptr->class < CARETAKER)) &&
		!F_ISSET(leader, PDMINV))
			print(fd, "%s is ignoring everyone.\n", leader->name);
	else
		print(leader->fd, "### %M group mentioned, \"%s\"\n",
			ply_ptr, &cmnd->fullstr[i + 1]);     

	if(found)
		broadcast_eaves("--- %s group talked: \"%s\"", ply_ptr->name,
			&cmnd->fullstr[i + 1]);

	return(0);
}


/***********************************************************************/
/*			psend					       */
/***********************************************************************/
/* This function allows a player to send a message to another player.  */
/* If the other player is logged in, the message is sent successfully. */

int psend(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	creature	*crt_ptr = 0;
	etag		*ign;
	int		spaces = 0, i, j, len, fd;

	fd = ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Send to whom?\n");
		return 0;
	}

	cmnd->str[1][0] = up(cmnd->str[1][0]);
	for(i = 0; i < Tablesize; i++) {
		if(!Ply[i].ply) continue;
		if(Ply[i].ply->fd == -1) continue;
		if(F_ISSET(Ply[i].ply, PDMINV) && ply_ptr->class < CARETAKER) 
			continue;
		if(!strncmp(Ply[i].ply->name, cmnd->str[1], 
		   strlen(cmnd->str[1])))
			crt_ptr = Ply[i].ply;
		if(!strcmp(Ply[i].ply->name, cmnd->str[1]))
			break;
	}
	if(!crt_ptr) {
		print(fd, "Send to whom?\n");
		return(0);
	}
	if(ply_ptr->class < CARETAKER && (F_ISSET(crt_ptr, PINVIS) && 
		!F_ISSET(ply_ptr, PDINVI))) {
			print(fd, "Send to whom?\n");
			return(0);
	}
 	if(F_ISSET(crt_ptr, PIGNOR) && (ply_ptr->class < CARETAKER)) {
		print(fd, "%s is ignoring everyone.\n", crt_ptr->name);
		return(0);
	}

	ign = Ply[crt_ptr->fd].extr->first_ignore;
	while(ign) {
		if(!strcmp(ign->enemy, ply_ptr->name)) {
			print(fd, "%s is ignoring you.\n", crt_ptr->name);
			return(0);
		}
		ign = ign->next_tag;
	}
	
	len = strlen(cmnd->fullstr);
	for(i = 0; i < len && i < 256; i++) {
		if(cmnd->fullstr[i] == ' ' && cmnd->fullstr[i + 1] != ' ')
			spaces++;
		if(spaces == 2) break;
	}
	cmnd->fullstr[255] = 0;
/* Check for modem escape code */
	for(j = 0; j < len && j < 256; j++) {
		if(cmnd->fullstr[j] == '+' && cmnd->fullstr[j + 1] == '+') {
			spaces = 0;
			break;
		}
	}

	if(spaces < 2 || strlen(&cmnd->fullstr[i + 1]) < 1) {
		print(fd, "Send what?\n");
		return(0);
	}
	if(F_ISSET(ply_ptr, PSILNC)) {
		print(fd, "The cat has your tongue and you can't speak.\n");
		return(0);
	}
	if(F_ISSET(ply_ptr, PLECHO)) {
		ANSI(fd, CYAN);
		print(fd, "You sent: \"%s\" to %M.\n", &cmnd->fullstr[i + 1], crt_ptr);
		ANSI(fd, BOLD)
		ANSI(fd, WHITE)
	}
	else
		print(fd, "Message sent to %s.\n", crt_ptr->name);
	
	print(crt_ptr->fd, "### %M just flashed, \"%s\"\n", ply_ptr,
		&cmnd->fullstr[i + 1]);
	
	if(F_ISSET(ply_ptr, PDMINV) && crt_ptr->class < CARETAKER) {
		print(fd, "They will be unable to reply.\n");
		if(F_ISSET(ply_ptr, PALIAS)) 
			print(fd, "Sent from: %s.\n", Ply[fd].extr->alias_crt);
	}

	if(ply_ptr->class > CARETAKER || crt_ptr->class > CARETAKER)
		return(0);

	broadcast_eaves("--- %s sent to %s, \"%s\"", ply_ptr->name,
		crt_ptr->name, &cmnd->fullstr[i + 1]);

	return(0);
}

/**********************************************************************/
/*			broadsend				      */
/**********************************************************************/
/* This function is used by players to broadcast a message to every   */
/* player in the game.  Broadcasts by players are of course limited,  */
/* so the number used that day is checked before the broadcast is     */
/* allowed.							  */

int broadsend(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;
{
	int	i, j, len, found = 0, fd;
	room    *rom_ptr;
	fd = ply_ptr->fd;

	len = strlen(cmnd->fullstr);
	for(i = 0; i < len && i < 256; i++) {
		if(cmnd->fullstr[i] == ' ' && cmnd->fullstr[i + 1] != ' ')
			found++;
		if(found == 1) break;
	}
	cmnd->fullstr[255] = 0;

/* Check for modem escape code */
	for(j = 0; j < len && j < 256; j++) {
		if(cmnd->fullstr[j] == '+' && cmnd->fullstr[j + 1] == '+') {
			found = 0;
			break;
		}
	}


	if(found < 1 || strlen(&cmnd->fullstr[i + 1]) < 1) {
		print(fd, "Send what?\n");
		return(0);
	}

#ifdef SECURE
	if((!strcmp(Ply[fd].io->userid, "no_port") || 
		!strcmp(Ply[fd].io->userid, "unknown")) && 
			!F_ISSET(ply_ptr, PAUTHD)) {
				print(fd, "You are not authorized to broadcast.\n");
				return(0);
	}
#endif /* SECURE */

	if(!F_ISSET(ply_ptr, PSECOK)) {
		print(fd, "You may not do that yet.\n");
		return(0);
	}

	if(ply_ptr->class < CARETAKER) {
		if(!dec_daily(&ply_ptr->daily[DL_BROAD])) {
			print(fd, "You've used up all your broadcasts today.\n");
			return(0);
		}
	}

	if(F_ISSET(ply_ptr, PSILNC)) {
		print(fd, "Your voice is too weak to do that.\n");
		return(0);
	}


	rom_ptr = ply_ptr->parent_rom;
	if((F_ISSET(rom_ptr, RJAILR)) && ply_ptr->class < CARETAKER) {
        print(fd, "No one can hear you from this deep in the earth.\n");
        return(0);
    }


	print(fd, "Message broadcast.\n");
	broadcast("### %M broadcasted, \"%s\"", ply_ptr, &cmnd->fullstr[i + 1]);

	return(0);
}

/************************************************************************/
/*			ignore						*/
/************************************************************************/
/* This function allows players to ignore the flashes of other players. */
/* If used without an argument, it lists the names of players currently */
/* being ignored.  If included with a name, the player will be added	*/
/* to the ignore list if he is not already on it.  Otherwise the player */
/* will be taken off the list.  The ignore list lasts only for the	*/
/* duration of the player's connection.					*/

int ignore(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	int		n, fd;
	etag		*ign, *prev;
	char		outstr[1024], *name;
	creature	*crt_ptr;

	fd = ply_ptr->fd;

	if(cmnd->num == 1) {
		strcpy(outstr, "You are ignoring: ");
		ign = Ply[fd].extr->first_ignore;
		n = 0;
		while(ign) {
			strcat(outstr, ign->enemy);
			strcat(outstr, ", ");
			n = 1;
			ign = ign->next_tag;
		}
		if(!n)
			strcat(outstr, "No one.");
		else {
			outstr[strlen(outstr) - 2] = '.';
			outstr[strlen(outstr) - 1] = 0;
		}
		print(fd, "%s\n", outstr);
		return(0);
	}
	cmnd->str[1][0] = up(cmnd->str[1][0]);
	name = cmnd->str[1];
	n = 0;
	ign = Ply[fd].extr->first_ignore;
	if(!ign) {}
	else if(!strcmp(ign->enemy, name)) {
		Ply[fd].extr->first_ignore = ign->next_tag;
		free(ign);
		n = 1;
	}
	else {
		prev = ign;
		ign = ign->next_tag;
		while(ign) {
			if(!strcmp(ign->enemy, name)) {
				prev->next_tag = ign->next_tag;
				free(ign);
				n = 1;
				break;
			}
			prev = ign;
			ign = ign->next_tag;
		}
	}

	if(n) {
		print(fd, "%s removed from your ignore list.\n", name);
		return(0);
	}

	crt_ptr = find_who(name);

	if(crt_ptr && !F_ISSET(crt_ptr, PDMINV)) {
		ign = (etag *)malloc(sizeof(etag));
		strcpy(ign->enemy, name);
		ign->next_tag = Ply[fd].extr->first_ignore;
		Ply[fd].extr->first_ignore = ign;
		print(fd, "%s added to your ignore list.\n", name);
	}
	else
		print(fd, "That player is not on.\n");

	return(0);
}

/************************************************************************/
/*			load_crt_tlk					*/
/************************************************************************/
/* This function loads a creature's talk responses, if they exist.	*/

int load_crt_tlk(crt_ptr)
creature	*crt_ptr;

{
	char	crt_name[80], path[256];
	char	keystr[80], responsestr[1024];
	int	i, len1, len2;
	ttag	*tp, **prev;
	FILE	*fp;

	if(!F_ISSET(crt_ptr, MTALKS) || crt_ptr->first_tlk)
		return(0);

	strcpy(crt_name, crt_ptr->name);
	for(i = 0; crt_name[i]; i++)
		if(crt_name[i] == ' ')
			crt_name[i] = '_';

	sprintf(path, "%s/talk/%s-%d", MONPATH, crt_name, crt_ptr->level);
	fp = fopen(path, "r");
	if(!fp) return(0);

	i = 0;
	prev = &crt_ptr->first_tlk;
	while(!feof(fp)) {
		fgets(keystr, 80, fp);
		len1 = strlen(keystr);
		if(!len1) break;
		keystr[len1 - 1] = 0;
		fgets(responsestr, 1024, fp);
		len2 = strlen(responsestr);
		if(!len2) break;
		responsestr[len2 - 1] = 0;

		i++;

		tp = (ttag *)malloc(sizeof(ttag));
		if(!tp)
			merror("load_crt_tlk", FATAL);
		tp->key = (char *)malloc(len1);
		if(!tp->key)
			merror("load_crt_tlk", FATAL);
		tp->response = (char *)malloc(len2);
		if(!tp->response)
			merror("load_crt_tlk", FATAL);
		tp->next_tag = 0;

		strcpy(tp->key, keystr);
		talk_crt_act(keystr, tp);
		strcpy(tp->response, responsestr);

		*prev = tp;
		prev = &tp->next_tag;
	}

	fclose(fp);
	return(i);
}

/************************************************************************/
/*                          talk_crt_act                                */
/************************************************************************/
/*  the talk_crt_act function is passed the  key word line from a    	*/
/*  monster talk file, and parses the key word, as well as any action 	*/
/*  for the monster.  (i.e. cast a spell, attack, do a social command.) */
/*  The parsed information is then assigned to the fields of the       	*/
/*  monster talkstructure. 						*/

int talk_crt_act(str, tlk)
char 	*str;
ttag	*tlk;

{

	int 	index = 0, num = 0, i, n;
	char	*word[4];


	if(!str) {
		tlk->key = 0;
		tlk->action = 0;
		tlk->target = 0;
		tlk->type = 0;
		return (0);
	}	

	
	for(i = 0; i < 4; i++)
		word[i] = 0;

	for(n = 0; n < 4; n++) {
		i = 0;
		while(isalpha(str[i + index]) || isdigit(str[i + index]) || 
			str[i + index] == '-')
				i++;
		word[n] = (char *)malloc(sizeof(char) * i + 1);
		if(!word[n])
			merror("talk_crt_act", FATAL);
		memcpy(word[n], &str[index], i);
		word[n][i] = 0;
		while(isspace(str[index + i]))
			i++;
		index += i;
		num++;
		if(str[index] == 0)
			break;

	}

	tlk->key = word[0];

	if(num < 2) {
		tlk->action = 0;
		tlk->target = 0;
		tlk->type = 0;
		return(0);
	}

	if(!strcmp(word[1], "ATTACK")) {
		tlk->type = 1;
		tlk->target = 0;
		tlk->action = 0;
	}
	else if(!strcmp(word[1], "ACTION") && num > 2) {
		tlk->type = 2;
		tlk->action = word[2];
		tlk->target = word[3];
	}
	else if(!strcmp(word[1], "CAST") && num > 2) {
		tlk->type = 3;
		tlk->action = word[2];
		tlk->target = word[3];
	}
	else if(!strcmp(word[1], "GIVE")) {
		tlk->type = 4;
		tlk->action = word[2];
		tlk->target =  0;
	}
	else {
		tlk->type = 0;
		tlk->action = 0;
		tlk->target = 0;
	}

	return(0);
}

