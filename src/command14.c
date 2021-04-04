/*
 * COMMAND14.C:
 * 
 *
 *	(c) 1996  Brooke Paul & Eric Krichbaum
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "/app/mud/darbonne/lib/time.h"

/**********************************************************************/
/*							emote									  */
/**********************************************************************/
/* This command allows a player to echo a message unaccompanied by    */
/* any message format, except for the players name at the beginning   */

int emote(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room	*rom_ptr;
	int	index = -1, j, i, fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	for(i = 0; i < strlen(cmnd->fullstr) && i < 256; i++) {
		if(cmnd->fullstr[i] == ' ') {
			index = i + 1;
			break;
		}
	}
	cmnd->fullstr[255] = 0;
	/* Check for modem escape code */
	for(j = 0; j < strlen(cmnd->fullstr) && j < 256; j++) {
		if(cmnd->fullstr[j] == '+' && cmnd->fullstr[j + 1] == '+') {
			index = -1;
			break;
		}
	}

	if(index == -1 || strlen(&cmnd->fullstr[index]) < 1) {
		print(fd, "Emote what?\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PSILNC)) {
		print(fd, "You are mute and unable to do that right now.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	if(F_ISSET(ply_ptr, PLECHO)) {
		ANSI(fd, CYAN);
		print(fd, "You emote: %s\n", &cmnd->fullstr[index]);
		ANSI(fd, WHITE);
	}
	else
		print(fd, "Ok.\n");

	broadcast_rom(fd, rom_ptr->rom_num, "|%M %s.", 
			ply_ptr, &cmnd->fullstr[index]);

	return(0);
}

/**************************************************************************/
/*									channelemote		             	  */
/**************************************************************************/
/* This command allows a player to broadcast a message unaccompanied by   */
/* any message format, except for the player's name at the beginning. 	  */

int channelemote(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room	*rom_ptr;
	int		index = -1, j, i, fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	for(i = 0; i < strlen(cmnd->fullstr) && i < 256; i++) {
		if(cmnd->fullstr[i] == ' ') {
			index = i + 1;
			break;
		}
	}

	cmnd->fullstr[255] = 0;
	/* Check for modem escape code */
	for(j = 0; j < strlen(cmnd->fullstr) && j < 256; j++) {
		if(cmnd->fullstr[j] == '+' && cmnd->fullstr[j + 1] == '+') {
			index = -1;
			break;
		}
	}

	if(index == -1 || strlen(&cmnd->fullstr[index]) < 1) {
		print(fd, "Emote what?\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PSILNC)) {
		print(fd, "You are unable to do that right now.\n");
		return(0);
	}

	if(!dec_daily(&ply_ptr->daily[DL_BROAE]) && ply_ptr->class < CARETAKER) {
		print(fd,"You've used up all your broadcasts today.\n");                        
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);
	if(F_ISSET(ply_ptr, PLECHO)) {
		ANSI(fd, CYAN);
		print(fd, "You emote: %s\n", &cmnd->fullstr[index]);
		ANSI(fd, BLUEBG);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
	}
	else
		print(fd, "Ok.\n");

	ANSI(fd, YELLOW);
	broadcast("### |%M %s.", ply_ptr, &cmnd->fullstr[index]);
	ANSI(fd, BLUEBG);
	ANSI(fd, BOLD);
	ANSI(fd, WHITE);

	return(0);
}
/************************************************************************/
/*									classemote						  	*/
/************************************************************************/
/* This command allows a player to broadcast a message to other players	*/ 
/* of the same class.  The message is unaccompanied by any message 		*/
/* format, except for the player's name at the beginning. 				*/

int classemote(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	room	*rom_ptr;
	int		index = -1, j, i, fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	for(i = 0; i < strlen(cmnd->fullstr) && i < 256; i++) {
		if(cmnd->fullstr[i] == ' ') {
			index = i + 1;
			break;
		}
	}
	cmnd->fullstr[255] = 0;
	/* Check for modem escape code */
	for(j = 0; j < strlen(cmnd->fullstr) && j < 256; j++) {
		if(cmnd->fullstr[j] == '+' && cmnd->fullstr[j + 1] == '+') {
			index = -1;
			break;
		}
	}

	if(index == -1 || strlen(&cmnd->fullstr[index]) < 1) {
		print(fd, "Emote what?\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PSILNC)) {
		print(fd, "You are unable to do that right now.\n");
		return(0);
	}

	F_CLR(ply_ptr, PHIDDN);

	if(F_ISSET(ply_ptr, PLECHO)) {
		ANSI(fd, CYAN);
		print(fd, "You emote: %s\n", &cmnd->fullstr[index]);
		ANSI(fd, BLUEBG);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
	}
	else
		print(fd, "Ok.\n");
		broadcast_class(ply_ptr, "### |%s %s.", ply_ptr->name, 
			&cmnd->fullstr[index]);
		broadcast_eaves("--- (%s): |%s %s.", class_str[ply_ptr->class], 
			ply_ptr->name, &cmnd->fullstr[index]);

	return(0);
}

/************************************************************************/
/*							class_send									*/
/************************************************************************/
/* This function allows a player to send messages to other players of	*/ 
/* the same class.  It is similar to a broadcast, but is unlimited.		*/

int class_send(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	int		i, fd, found = 0;

	fd = ply_ptr->fd;

	for(i = 0; i < strlen(cmnd->fullstr); i++) {
		if(cmnd->fullstr[i] == ' ' && cmnd->fullstr[i+1] != ' ') {
			found++;
			break;
		}
	}

	if(found < 1 || strlen(&cmnd->fullstr[i + 1]) < 1) {
		print(fd, "Send what?\n");
		return(0);
	}

	print(fd, "Ok.\n");
	broadcast_class(ply_ptr, "### %s sent, \"%s\".", ply_ptr->name, 
		&cmnd->fullstr[i + 1]);
	broadcast_eaves("--- %s class sent, \"%s\".", ply_ptr->name, 
		&cmnd->fullstr[i + 1]);

	return(0);

}
