/*
 * COMMAND1.C:
 *
 *	Command handling/parsing routines.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include "mtype.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "AddFiles/time.h"


/**************************************************************************/
/*				login				      	  */
/**************************************************************************/
/* This function is the first function that gets input from a player when */
/* he logs in.  It asks for the player's name and password, and performs  */
/* the appropiate function calls.					  */

void login(fd, param, str)
int 	fd;
int		param;
char	*str;

{
	int			i, match = 0;
	extern int	Numplayers;
	char		tempstr[20], str2[50];
	long 		t, x;
	creature	*ply_ptr;

	switch(param) {
	case 0:
		if(strcmp(Ply[fd].extr->tempstr[0], str)) {
			disconnect(fd);
			return;
		}
		COLR(fd, BLUEBG);
		COLR(fd, WHITE);
		print(fd, "\nWelcome to Darbonne...");
		print(fd, "\nPlease enter your character's name: ");
		RETURN(fd, login, 1);

	case 1:
		if(!isalpha(str[0])) {
			print(fd, "Please enter a name (no numbers): ");
			RETURN(fd, login, 1);
		}

		if(strlen(str) < 3) {
			COLR(fd, BLUEBG);
			COLR(fd, WHITE);
			print(fd, 
				"A name must be at least 3 characters with no spaces.\n\n");
			print(fd, "Please enter your character's name: ");
			RETURN(fd, login, 1);
		}

		if(strlen(str) >= 20) {
			COLR(fd, BLUEBG);
			COLR(fd, WHITE);
			print(fd, "A name must be less than 20 characters.\n\n");
			print(fd, "Please enter your character's name: ");
			RETURN(fd, login, 1);
		}

		for(i = 0; i < strlen(str); i++)
			if(!isalpha(str[i])) {
				COLR(fd, BLUEBG);
				COLR(fd, WHITE);
				print(fd, "A name must be alphabetic with no spaces.\n\n");
				print(fd, "Please enter your character's name: ");
				RETURN(fd, login, 1);
			}

		lowercize(str, 1);
		str[25] = 0;
		if(load_ply(str, &ply_ptr) < 0) {
			if(read_forbidden_name(str) < 0) {	
				strcpy(Ply[fd].extr->tempstr[0], str);
				COLR(fd, BLUEBG);
				COLR(fd, WHITE);
				print(fd, "\n%s? Did I get that right? ", str);
				RETURN(fd, login, 2);
			}
			else if(read_forbidden_name(str) > 0) {	
				COLR(fd, BLUEBG);
				COLR(fd, YELLOW);
				print(fd, "Sorry, that name is not permitted.\n\n");
				COLR(fd, WHITE);
				print(fd, "Please enter a character's name: ");
				RETURN(fd, login, 1);
			}
		}
		else {
			ply_ptr->fd = -1;
			Ply[fd].ply = ply_ptr;
			print(fd, "%c%c%cPlease enter your password: ", 255, 251, 1); 
			RETURN(fd, login, 3);
		}

	case 2:
		if(str[0] != 'y' && str[0] != 'Y') {
			Ply[fd].extr->tempstr[0][0] = 0;
			COLR(fd, BLUEBG);
			COLR(fd, WHITE);
			print(fd, "Please enter your character's name: ");
			RETURN(fd, login, 1);
		}
		else {
			print(fd, "\nHit return: ");
			RETURN(fd, create_ply, 1);
		}

	case 3:
		if(strcmp(str, Ply[fd].ply->password)) {
			COLR(fd, YELLOW);
			write(fd, 
				"\255\252\1\n\rIncorrect for specified player name.\n\r", 44);
			disconnect(fd);
			return;
		}
		else {
			print(fd, "%c%c%c\n\r", 255, 252, 1);
			strcpy(tempstr, Ply[fd].ply->name);
			for(i = 0; i < Tablesize; i++)
				if(Ply[i].ply && i != fd)
					if(!strcmp(Ply[i].ply->name, Ply[fd].ply->name))
						disconnect(i);	
			free_crt(Ply[fd].ply);

			if(load_ply(tempstr, &Ply[fd].ply) < 0) {
				write(fd, "Player no longer exists!\n\r", 24);
        	    t = time(0);
        	    strcpy(str2, (char *)ctime(&t));
        	    str2[strlen(str2) - 1] = 0;
        	    logn("sui_crash","%s: %s (%s) suicided.\n", 
				str2, Ply[fd].ply->name, Ply[fd].io->address);         
				disconnect(fd);
				return;
			}

			Ply[fd].ply->fd = fd;
			init_ply(Ply[fd].ply);
			RETURN(fd, command, 1);
		}
	}
}

/*********************************************************************/
/*			create_ply	 			     */
/*********************************************************************/
/* This function allows a new player to create his or her character. */

void create_ply(fd, param, str)
int		fd;
int		param;
char	*str;

{
	int 	i, k, l, n, sum, num[5];

	switch(param) {
	case 1:
		print(fd,"\n\n");
		Ply[fd].ply = (creature *)malloc(sizeof(creature));
		if(!Ply[fd].ply)
			merror("create_ply", FATAL);
		zero(Ply[fd].ply, sizeof(creature));
		Ply[fd].ply->fd = -1;
		Ply[fd].ply->rom_num = 1;
		print(fd, "Male or Female: ");
		RETURN(fd, create_ply, 2);

	case 2:
		if(low(str[0]) != 'm' && low(str[0]) != 'f') {
			print(fd, "Male or Female: ");
			RETURN(fd, create_ply, 2);
		}
		if(low(str[0]) == 'm')
			F_SET(Ply[fd].ply, PMALES);
		print(fd, "\nAvailable classes:\n");
		COLR(fd, YELLOW);
		print(fd, "\tAlchemist\tAssassin\tBarbarian\tBard\n");
		print(fd, "\tCleric   \tDruid   \tFighter  \tMage\n");
		print(fd, "\tMonk     \tPaladin \tRanger   \tThief\n");
		COLR(fd, WHITE);
		print(fd, "Choose one: ");
		RETURN(fd, create_ply, 3);

	case 3:
		switch(low(str[0])) {
			case 'a': switch(low(str[1])) {
				case 'l':Ply[fd].ply->class = ALCHEMIST; break;
				case 's':Ply[fd].ply->class = ASSASSIN; break;
				}
				break;
			case 'b': switch(low(str[3])) {
				case 'b':Ply[fd].ply->class = BARBARIAN; break;
				case 'd':Ply[fd].ply->class = BARD; break;
				}
				break;
			case 'c': Ply[fd].ply->class = CLERIC; break;
			case 'd': Ply[fd].ply->class = DRUID; break;
			case 'f': Ply[fd].ply->class = FIGHTER; break;
			case 'm': switch(low(str[1])) {
				case 'a':Ply[fd].ply->class = MAGE; break;
				case 'o':Ply[fd].ply->class = MONK; break;
			}
			break;
			case 'p': Ply[fd].ply->class = PALADIN; break;
			case 'r': Ply[fd].ply->class = RANGER; break;
			case 't': Ply[fd].ply->class = THIEF; break;
			default: print(fd, "Choose one: "); 
			RETURN(fd, create_ply, 3);
		}
		if(!Ply[fd].ply->class) {
			print(fd, "What?  Invalid selection: %s", str);
			print(fd, "\nChoose one: ");
			RETURN(fd, create_ply, 3);
		}
		COLR(fd, BLUEBG);
		print(fd, "\nAvailable races:\n");
		COLR(fd, YELLOW);
		print(fd, "\tCentaur   \tDwarf     \tDark-elf\n"); 
		print(fd, "\tElf       \tGnome     \tHalf-elf\n");
		print(fd, "\tHalf-giant\tHalf-orc  \tHobbit\n");
		print(fd, "\tHuman     \tOgre      \tOrc\n");
		print(fd, "\tSatyr\n");
		COLR(fd, WHITE);
		print(fd, "Choose one:  ");
		RETURN(fd, create_ply, 4);

	case 4:
		switch(low(str[0])) {
		case 'd': switch(low(str[1])) {
 			case 'a': Ply[fd].ply->race = DARKELF; break;
			case 'w': Ply[fd].ply->race = DWARF; break;
		}
		break;
		case 'c': Ply[fd].ply->race = CENTAUR; break;
		case 'e': Ply[fd].ply->race = ELF; break;
		case 'g': Ply[fd].ply->race = GNOME; break;
		case 'o': switch(low(str[1])) {
			case 'g': Ply[fd].ply->race = OGRE; break;
			case 'r': Ply[fd].ply->race = ORC; break;
		}
		break;
		case 's': Ply[fd].ply->race = SATYR; break;
		case 'h': switch(low(str[1])) {
			case 'a': switch(low(str[5])) {
				case 'e': Ply[fd].ply->race = HALFELF; break;
				case 'g': Ply[fd].ply->race = HALFGIANT; break;
				case 'o': Ply[fd].ply->race = HALFORC; break;
			}
			break;
			case 'o': Ply[fd].ply->race = HOBBIT; break;
			case 'u': Ply[fd].ply->race = HUMAN; break;
			}
			break;
		}

		if(!Ply[fd].ply->race) {
			print(fd, "\nChoose one: ");
			RETURN(fd, create_ply, 4);
		}

		print(fd, 
			"\nYou have 54 points to distribute among your 5 statistics.\n");
		print(fd, "No statistic may be smaller than 3 or larger than 18.\n");
		print(fd, "Please enter your five numbers in the following order:\n");
		print(fd, "Strength, Dexterity, Constitution, Intelligence, Piety.\n");
		print(fd, "\nUse the following format:   ## ## ## ## ##\n");
		print(fd, "         Your statistics:   ");
		RETURN(fd, create_ply, 5);

	case 5:
		n = strlen(str); l = 0; k = 0;
		for(i = 0; i <= n; i++) {
			if(str[i] == ' ' || str[i] == 0) {
				str[i] = 0;
				num[k++] = atoi(&str[l]);
				l = i+1;
			}
			if(k > 4) break;
		}
		if(k < 5) {
			COLR(fd, YELLOW);
			print(fd, "\nPlease enter all 5 numbers.\n\n");
			COLR(fd, WHITE);
		print(fd, "Strength, Dexterity, Constitution, Intelligence, Piety.\n");
			print(fd, "\nUse the following format:   ## ## ## ## ##\n");
			print(fd, "         Your statistics:   ");
			RETURN(fd, create_ply, 5);
		}
		sum = 0;
		for(i = 0; i < 5; i++) {
			if(num[i] < 3 || num[i] > 18) {
			COLR(fd, YELLOW);
			print(fd, "\nNo stats < 3 or > 18 please.\n\n");
			COLR(fd, WHITE);
			print(fd, "Strength, Dexterity, Constitution, Intelligence, Piety.\n");
			print(fd, "\nUse the following format:   ## ## ## ## ##\n");
			print(fd, "         Your statistics:   ");
				RETURN(fd, create_ply, 5);
			}
			sum += num[i];
		}
		if(sum != 54) {
			COLR(fd, YELLOW);
			print(fd, "\nTotal of statistics must equal 54.\n");
			print(fd, "Your total is %d.\n\n", sum);
			COLR(fd, WHITE);
			print(fd, "Strength, Dexterity, Constitution, Intelligence, Piety.\n");
			print(fd, "\nUse the following format:   ## ## ## ## ##\n");
			print(fd, "         Your statistics:   ");
			RETURN(fd, create_ply, 5);
		}
		Ply[fd].ply->strength = num[0];
		Ply[fd].ply->dexterity = num[1];
		Ply[fd].ply->constitution = num[2];
		Ply[fd].ply->intelligence = num[3];
		Ply[fd].ply->piety = num[4];

		if(F_ISSET(Ply[fd].ply, PMALES))
			Ply[fd].ply->strength++; 
		else
			Ply[fd].ply->dexterity++; 
			
		switch(Ply[fd].ply->race) {
		case CENTAUR:
			Ply[fd].ply->constitution += 2;
			Ply[fd].ply->piety--;
			break;
		case DARKELF:
			Ply[fd].ply->intelligence++;
			Ply[fd].ply->dexterity++; 
			Ply[fd].ply->constitution--;
			break;
		case DWARF: 
			Ply[fd].ply->strength++; 
			Ply[fd].ply->constitution++;
			Ply[fd].ply->piety--; 
			break;
		case ELF: 
			Ply[fd].ply->intelligence += 2;
			Ply[fd].ply->constitution--;
			break;
		case GNOME:
			Ply[fd].ply->constitution++;
			Ply[fd].ply->piety++;
			Ply[fd].ply->strength--;
			break;
		case HALFELF: 
			Ply[fd].ply->intelligence++; 
			Ply[fd].ply->piety++;
			Ply[fd].ply->constitution--; 
			break;
		case HALFGIANT: 
			Ply[fd].ply->strength += 2; 
			Ply[fd].ply->intelligence--; 
			break;
		case HALFORC:
			Ply[fd].ply->constitution++;
			Ply[fd].ply->dexterity++; 
			Ply[fd].ply->piety--; 
			break;			
		case HOBBIT: 
			Ply[fd].ply->dexterity += 2; 
			Ply[fd].ply->strength--; 
			break;
		case HUMAN: 
			Ply[fd].ply->constitution++; 
			break;
		case OGRE:
			Ply[fd].ply->strength++;
			Ply[fd].ply->constitution++; 
			Ply[fd].ply->piety--;
			break;
		case ORC: 
			Ply[fd].ply->strength++; 
			Ply[fd].ply->dexterity++; 
			Ply[fd].ply->intelligence--;
			break;
		case SATYR: 
			Ply[fd].ply->dexterity++; 
			Ply[fd].ply->piety++;
			Ply[fd].ply->constitution--; 
			break;
		}

		print(fd, "\nChoose a favorite type of weapon:\n");
		print(fd, "Sharp, Thrusting, Blunt, Pole, Missile.\n");
		print(fd, ": ");
		RETURN(fd, create_ply, 6);

	case 6:
		switch(low(str[0])) {
			case 's': Ply[fd].ply->proficiency[0] = 1024; break;
			case 't': Ply[fd].ply->proficiency[1] = 1024; break;
			case 'b': Ply[fd].ply->proficiency[2] = 1024; break;
			case 'p': Ply[fd].ply->proficiency[3] = 1024; break;
			case 'm': Ply[fd].ply->proficiency[4] = 1024; break;
			default: print(fd, "What?  Try again, please.\n: ");
			RETURN(fd, create_ply, 6);
		}

		if(Ply[fd].ply->class == FIGHTER) {
			for(i = 0; i < 5; i++)
				if(Ply[fd].ply->proficiency[i] == 0)
					Ply[fd].ply->proficiency[i] = 1024;
				else	Ply[fd].ply->proficiency[i] = 1268;
		}

		if(Ply[fd].ply->class == PALADIN) {
			for(i = 0; i < 5; i++)
				if(Ply[fd].ply->proficiency[i] == 1024)
					Ply[fd].ply->proficiency[i] = 1268;
			if(Ply[fd].ply->proficiency[0] == 0)
				Ply[fd].ply->proficiency[0] = 1024;
			else
				Ply[fd].ply->proficiency[1] = 1024;

		}

		print(fd, 
			"\nLawful players cannot attack or steal from other players,\n"); 
		print(fd, 
			"nor can they be attacked or stolen from.  A lawful player\n");
		print(fd, 
			"can become chaotic later in the game, but at a price.\n");
		print(fd, 
			"\nChaotic players may attack and/or steal from each other.\n");
		print(fd, 
			"They may also choose to become lawful later.  Generally, no\n"); 
		print(fd, 
			"sanctions are involved for this change.  Choose carefully.\n");
		print(fd, 
			"\nNow, select an alignment, Chaotic or Lawful:  ");

		RETURN(fd, create_ply, 7);

	case 7:
		if(low(str[0]) == 'c')
			F_SET(Ply[fd].ply, PCHAOS);
		else if(low(str[0]) == 'l')
			F_CLR(Ply[fd].ply, PCHAOS);
		else {
			print(fd, "What?  Chaotic or Lawful: ");
			RETURN(fd, create_ply, 7);
		}
		print(fd, "\nChoose a password (up to 14 chars): ");
		RETURN(fd, create_ply, 8);

	case 8:
		if(strlen(str) > 14) {
			print(fd, "Too long.\nChoose a password: ");
			RETURN(fd, create_ply, 8);
		}
		if(strlen(str) < 3) {
			print(fd, "Too short.\nChoose a password: ");
			RETURN(fd, create_ply, 8);
		}
		strncpy(Ply[fd].ply->password, str, 14);
		strcpy(Ply[fd].ply->name, Ply[fd].extr->tempstr[0]);
		up_level(Ply[fd].ply);
		Ply[fd].ply->fd = fd;
		init_ply(Ply[fd].ply);
		save_ply(Ply[fd].ply->name, Ply[fd].ply);
		print(fd, "\n");
		COLR(fd, YELLOW);
		print(fd, "Type 'welcome' to get more information about the game.\n");
		COLR(fd, WHITE);

		RETURN(fd, command, 1);
	}
}

/************************************************************************/
/*				command			 	        */
/************************************************************************/
/* This function handles the main prompt commands, and calls the   	*/
/* appropriate function, depending on what service is requested by the 	*/
/* player.																*/

void command(fd, param, str)
int		fd;
int		param;
char	*str;

{
	cmd	cmnd;
	int	n;
	unsigned char ch;

#ifdef RECORD_ALL
/*
this logn command will print out all the commands entered by players.
It should be used in extreme cases when trying to isolate a players
input which may be causing a crash.
*/

logn("all_cmd","\n%s-%d (%d): %s\n",
	Ply[fd].ply->name,fd, Ply[fd].ply->rom_num, str);  
#endif /* RECORD_ALL */ 

	switch(param) {
	case 1:

		if(!strcmp(str, "!"))
			strncpy(str, Ply[fd].extr->lastcommand, 79);

		if(str[0]) {
			for(n = 0; str[n] && str[n] == ' '; n++) ;
			strncpy(Ply[fd].extr->lastcommand, &str[n], 79);
		}

		strncpy(cmnd.fullstr, str, 255);
		lowercize(str, 0);
		parse(str, &cmnd); n = 0;

		if(cmnd.num)
			n = process_cmd(fd, &cmnd);
		else
			n = PROMPT;

		if(n == DISCONNECT) {
			write(fd, "Goodbye!\n\r\n\r", 12);
			COLR(fd, BLACKBG); 		/* reset Background */
			COLR(fd, WHITE); 		/* reset Foreground */
			COLR(fd, NORMAL);		/* reset Attribut */
			disconnect(fd);
			return;
		}
		else if(n == PROMPT) {
			if(F_ISSET(Ply[fd].ply, PPROMP)) {
				sprintf(str, "(%d H %d M): ", 
					Ply[fd].ply->hpcur, 
						Ply[fd].ply->mpcur);
			}
			else
				strcpy(str, ": ");
			write(fd, str, strlen(str));
		}

		if(n != DOPROMPT) {
			RETURN(fd, command, 1);
		}
		else
			return;
	}
}

/***********************************************************************/
/*				parse				       */
/***********************************************************************/
/* This function takes the string in the first parameter and breaks    */
/* it up into its component words, stripping out useless words.  The   */
/* resulting words are stored in a command structure pointed to by     */
/* the second argument. 					       */

void parse(str, cmnd)
char	*str;
cmd		*cmnd;

{
	int		i, j, l, m, n, o, art;
	char	tempstr[25];

	l = m = n = 0;
	j = strlen(str);

	for(i = 0; i <= j; i++) {
		if(str[i] == ' ' || str[i] == '#' || str[i] == 0) {
			str[i] = 0;	/* tokenize */

			/* Strip extra white-space */
			while((str[i + 1] == ' ' || str[i] == '#') && i < j + 1)
				str[++i] = 0;

			strncpy(tempstr, &str[l], 24); tempstr[24] = 0;
			l = i + 1;
			if(!strlen(tempstr)) continue;

			/* Ignore article/useless words */
			o = art = 0;
			while(article[o][0] != '@') {
				if(!strcmp(article[o++], tempstr)) {
					art = 1;
					break;
				}
			}
			if(art) continue;

			/* Copy into command structure */
			if(n == m) {
				strncpy(cmnd->str[n++], tempstr, 20);
				cmnd->val[m] = 1L;
			}
			else if(isdigit(tempstr[0]) || (tempstr[0] == '-' &&
				isdigit(tempstr[1]))) {
				cmnd->val[m++] = atol(tempstr);
			}
			else {
				strncpy(cmnd->str[n++], tempstr, 20);
				cmnd->val[m++] = 1L;
			}

		}
		if(m >= COMMANDMAX) {
			n = 5;
			break;
		}
	}

	if(n > m)
		cmnd->val[m++] = 1L;
	cmnd->num = n;

}

/*************************************************************************/
/*			process_cmd				     	 */
/*************************************************************************/
/* This function takes the command structure of the person at the socket */
/* in the first parameter and interprets the person's command.		 */

int process_cmd(fd, cmnd)
int		fd;
cmd		*cmnd;

{
	int	match = 0, cmdno = 0, c = 0, n;

	do {
		if(!strcmp(cmnd->str[0], cmdlist[c].cmdstr)) {
			match = 1;
			cmdno = c;
			break;
		}
		else if(!strncmp(cmnd->str[0], cmdlist[c].cmdstr, 
			strlen(cmnd->str[0]))) {
			match++;
			cmdno = c;
		}
		c++;
	} while(cmdlist[c].cmdno);

	if(match == 0) {
		print(fd, "The command \"%s\" does not exist.\n",
		      cmnd->str[0]);
		RETURN(fd, command, 1);
	}

	else if(match > 1) {
		print(fd, "Command is not unique.\n");
		RETURN(fd, command, 1);
	}

	if(cmdlist[cmdno].cmdno < 0)
		return(special_cmd(Ply[fd].ply, 0-cmdlist[cmdno].cmdno, cmnd));
	
	return((*cmdlist[cmdno].cmdfn)(Ply[fd].ply, cmnd));

}


/*********************************************************************/
/*                          passwd                            	     */
/*********************************************************************/
/* The passwd function calls the necessary function to allow 	     */
/* a player to change their password. 				     */

int passwd (ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;   

{
int			fd;
extern void chpasswd();
	fd = ply_ptr->fd;

	/* do not flash output until player hits return */
	F_SET(Ply[fd].ply, PREADI); 
	chpasswd(fd, 0, "");
	return(DOPROMPT);
                         
}
/******************************************************************/
/*                       chpasswd                                 */
/******************************************************************/
/* The chpasswd command handles  the procedure involved in        */
/* changing a player's password.  A player first must enter the   */
/* correct current password, then the new password, and re-enter  */
/* the new password to comfirm it.  If the player enters the      */
/* wrong password  or an invalid password (too short or long),    */
/* the password will not be changed and the procedure is aborted. */

void chpasswd(fd, param, str)
int		fd;
int		param;
char	 *str;

{
	creature	*ply_ptr;


	ply_ptr = Ply[fd].ply;
	output_buf();
	switch (param) {
		case 0:
			print(fd, "Current password: ");
 			output_buf();
    	    Ply[fd].io->intrpt &= ~1; 
			RETURN(fd, chpasswd, 1);
			break;

		case 1:
			if(!strcmp(ply_ptr->password, str)) {
				print(fd, "%c%c%c\n\r", 255, 252, 1);
				print(fd, "New password: ");
		 		output_buf();
    	        Ply[fd].io->intrpt &= ~1; 
				RETURN(fd, chpasswd, 2);
			}
			else {
				print(fd, "%c%c%c\n\r", 255, 252, 1);
				print(fd, "Imposter!  Incorrect password.\n");
				print(fd, "Aborting.\n");
    	        F_CLR(Ply[fd].ply, PREADI);
				RETURN(fd, command, 1);
			}

		case 2:
			if(strlen(str) < 3) {
				print(fd, "%c%c%c\n\r", 255, 252, 1);
				print(fd, "Password too short.\n");
                print(fd, "Aborting.\n");
                F_CLR(Ply[fd].ply, PREADI);
				RETURN(fd, command, 1);
			}
			else if(strlen(str) > 14) {
				print(fd, "%c%c%c\n\r", 255, 252, 1);
				print(fd, "Password too long.\n");
				print(fd, "Aborting.\n");
            	F_CLR(Ply[fd].ply, PREADI);
				RETURN(fd, command, 1);
			}
			else {
				strcpy(Ply[fd].extr->tempstr[1], str);
				print(fd, "%c%c%c\n\r", 255, 252, 1);
				print(fd, "Re-enter password: ");
	 			output_buf();
            	Ply[fd].io->intrpt &= ~1; 
				RETURN(fd, chpasswd, 3);
			}
				break;

		case 3:
			if(!strcmp(Ply[fd].extr->tempstr[1],str)) {
				strcpy(ply_ptr->password, str);
				print(fd, "%c%c%c\n\r", 255, 252, 1);
				print(fd,"Password changed.\n");
        	   	F_CLR(Ply[fd].ply, PREADI);
				save_ply(ply_ptr->name, ply_ptr);
				RETURN(fd, command, 1);
			}
			else {
				print(fd, "%c%c%c\n\r", 255, 252, 1);
				print(fd, "Different passwords given.\n");
				print(fd, "Aborting.\n");
        	   	F_CLR(Ply[fd].ply, PREADI);
        	    RETURN(fd, command, 1);
			}
			break;
	}

}

#ifdef CHECKDOUBLE

int checkdouble(name)
char *name;
{
	char	path[128], tempname[80];
	FILE 	*fp;
	int		found = 0;

	sprintf(path, "%s/simul/%s", PLAYERPATH, name);
	fp = fopen(path, "r");
	if(!fp)
		return(0);

	while(!feof(fp)) {
		fgets(tempname, 80, fp);
		tempname[strlen(tempname) - 1] = 0;
		if(!strcmp(tempname, name))
			continue;
		if(find_who(tempname)) {
			found = 1;
			break;
		}
	}

	fclose(fp);
	return(rtn);
}
#endif /* CHECKDOUBLE */


/**********************************************************************/
/*           	   ply_suicide 					      */
/**********************************************************************/
/* This function is called whenever the player explicitly asks to     */
/* commit suicide.  It then calls the suicide() function which takes  */
/* over that player's input.               			      */

int ply_suicide(ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;
{
    suicide(ply_ptr->fd, 1, "");
    return(DOPROMPT);
}

/***********************************************************************/
/* 		   suicide 		 		 	       */
/***********************************************************************/
/* This function allows a player to kill himself, thus erasing his     */
/* entire player file.  			                       */

void suicide(fd, param, str)
int 	fd;
int 	param;
char    *str;

{
    char    file[80];
#ifdef SUICIDE
    long t;
    char str2[50];
#endif

    switch(param) {
        case 1:
	    ANSI(fd, RED);
            print(fd, "This will completely erase your player.\n");
            print(fd, "Are you sure (Y/N)\n");
 	    ANSI(fd, BOLD);
	    ANSI(fd, WHITE);
            RETURN(fd, suicide, 2);
        case 2:
            if(low(str[0]) == 'y') {
                broadcast("### %s committed suicide! We'll miss %s dearly.", 
					Ply[fd].ply->name, F_ISSET(Ply[fd].ply, PMALES) ? 
						"him":"her");
                sprintf(file, "%s/%s", PLAYERPATH, Ply[fd].ply->name);

#ifdef SUICIDE
	t = time(0);
        strcpy(str2, (char *)ctime(&t));
        str[strlen(str2)-1] = 0;
	logn("SUICIDE","%s (%s@%s):%s\n", Ply[fd].ply->name, 
		Ply[fd].io->userid, Ply[fd].io->address, str2); 
#endif
		
				disconnect(fd);
                unlink(file);

				/* remove mail file if it exists */
				sprintf(file, "%s/%s", POSTPATH, Ply[fd].ply->name);
				unlink(file);
				return;

            }
            else {
                print(fd, "Aborted.\n");
                RETURN(fd, command, 1);
            }
    }
}

