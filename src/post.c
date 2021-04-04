/*
 * POST.C:
 *
 *	These routines are for the game post office.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */
#include "mstruct.h"
#include "mextern.h"


/************************************************************************/
/*								postsend								*/
/************************************************************************/
/* This function allows a player to send a letter to another player if	*/
/* he/she is in a post office.											*/

int postsend(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	room	*rom_ptr;
	char	file[80];
	int		fd, ff;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(!F_ISSET(rom_ptr, RPOSTO) && ply_ptr->class < CARETAKER) {
		print(fd, "This is not a post office.\n");
		return(0);
	}

	if(cmnd->num < 2) {
		print(fd, "Mail whom?\n");
		return(0);
	}

	cmnd->str[1][0] = up(cmnd->str[1][0]);
	sprintf(file, "%s/%s", PLAYERPATH, cmnd->str[1]);
	ff = open(file, O_RDONLY, 0);

	if(ff < 0) {
		print(fd, "That player does not exist.  [Check the spelling.]\n");
		return(0);
	}

	close(ff);

	ANSI(fd, YELLOW)
	ANSI(fd, BLUEBG)

	print(fd, "Enter your message now.  Use an empty line to finish.\n");
	print(fd, "This is NOT a word processor... so use the enter key, \n");
	print(fd, "as each line should be NO LONGER THAN 80 CHARACTERS.\n-: ");

	ANSI(fd, NORMAL)
	ANSI(fd, WHITE)

	sprintf(Ply[fd].extr->tempstr[0], "%s/%s", POSTPATH, cmnd->str[1]);

	F_SET(Ply[fd].ply, PREADI);
	output_buf();
	Ply[fd].io->intrpt &= ~1;
	Ply[fd].io->fn = postedit;
	Ply[fd].io->fnparam = 1;

	ANSI(fd, BOLD)
	return(DOPROMPT);

}

/************************************************************************/
/*							postedit									*/
/************************************************************************/
/* This function is called when a player is editing a message to send	*/
/* to another player.  Its major flaw is that it sends the line just	*/
/* typed to the target's mail file immediately.  So if two people were	*/
/* simultaneously mailing the same person, it could get messy.			*/

void postedit(fd, param, str)
int		fd;
int		param;
char	*str;
{
	char	outstr[85], datestr[40];
	long	t;
	int		ff;

	if(!str[0]) {
		F_CLR(Ply[fd].ply, PREADI);
		ANSI(fd, BOLD)
		print(fd, "Mail sent.\n");
		RETURN(fd, command, 1);
	}

	ff = open(Ply[fd].extr->tempstr[0], O_CREAT | O_APPEND | O_RDWR, ACC);

	if(ff < 0)
		merror("postedit", FATAL);

	if(param == 1) {
		time(&t);
		strcpy(datestr, (char *)ctime(&t));
		datestr[strlen(datestr)-1] = 0;
		sprintf(outstr, "\n---\nMail from %s (%s):\n\n",
		    Ply[fd].ply->name, datestr);
		write(ff, outstr, strlen(outstr));
	}

	ANSI(fd, NORMAL);
	ANSI(fd, WHITE);
	ANSI(fd, BLUEBG);

	strncpy(outstr, str, 79);
	outstr[79] = 0;
	strcat(outstr, "\n");

	write(ff, outstr, strlen(outstr));

	close(ff);

	print(fd, "-: ");

	output_buf();
	Ply[fd].io->intrpt &= ~1;
	RETURN(fd, postedit, 2);
}

/************************************************************************/
/*								postread								*/
/************************************************************************/
/* This function allows a player to read his or her mail.				*/

int postread(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	room	*rom_ptr;
	char 	file[80];
	int		fd;
	
	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(!F_ISSET(rom_ptr, RPOSTO) && ply_ptr->class < CARETAKER) {
		print(fd, "This is not a post office.\n");
		return(0);
	}

	strcpy(file, POSTPATH);

	strcat(file, "/");
	strcat(file, ply_ptr->name);
	ANSI(fd, NORMAL)
	ANSI(fd, WHITE)
	ANSI(fd, BLUEBG)
	view_file(fd, 1, file);
	return(DOPROMPT);

}

/************************************************************************/
/*								postdelete								*/
/************************************************************************/
/* This function allows a player to delete his or her mail.				*/

int postdelete(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;
{
	room	*rom_ptr;
	char	file[80];
	int		fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(!F_ISSET(rom_ptr, RPOSTO) && ply_ptr->class < CARETAKER) {
		print(fd, "This is not a post office.\n");
		return(0);
	}

	sprintf(file, "%s/%s", POSTPATH, ply_ptr->name);

	print(fd, "Mail deleted.\n");

	unlink(file);

	return(0);
}
/************************************************************************/
/*                              notepad                                 */
/************************************************************************/
/*  This is for DM's only.  It provides a notepad that can be written	*/
/*  to or cleared.  Things-to-do...										*/

int notepad(ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;
{
    room    *rom_ptr;
    char    file[80];
    int 	fd, ff;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;
 
    if(ply_ptr->class < CARETAKER)
        return(PROMPT);
 
    sprintf(file, "%s/%s_pad", POSTPATH, ply_ptr->name);
 
    if(cmnd->num == 2) {
        if(low(cmnd->str[1][0]) == 'a') {
            strcpy(Ply[fd].extr->tempstr[0],file);
            print(fd, "DM notepad:\n->");
            F_SET(Ply[fd].ply, PREADI);
            output_buf();
            Ply[fd].io->intrpt &= ~1;
            Ply[fd].io->fn = noteedit;
            Ply[fd].io->fnparam = 1;
            return(DOPROMPT);
        }
        else if(low(cmnd->str[1][0]) == 'd') {
            unlink(file);
            print(fd,"Clearing your notepad.\n");
            return(PROMPT);
        }
        else {
            print(fd,"invalid option.\n");
            return(PROMPT);
        }
    }
    else {
        view_file(fd, 1, file);
        return(DOPROMPT);
    }
}
 
/************************************************************************/
/*                              noteedit                                */
/************************************************************************/
 
void noteedit(fd, param, str)
int 	fd;
int 	param;
char    *str;
{
    char    outstr[85], tmpstr[40];
    long    t;
    int 	ff;
 
    if(str[0] == '.') {
        F_CLR(Ply[fd].ply, PREADI);
        print(fd, "Message appended.\n");
        RETURN(fd, command, 1); 
    }
 
    ff = open(Ply[fd].extr->tempstr[0], O_RDONLY, 0);
    if(ff < 0) {
        ff = open(Ply[fd].extr->tempstr[0], O_CREAT | O_RDWR, ACC);
        sprintf(tmpstr,"            %s\n\n","=== DM Notepad ===");
        write(ff, tmpstr,strlen(tmpstr));
    }
    close(ff);
 
    ff = open(Ply[fd].extr->tempstr[0], O_CREAT | O_APPEND | O_RDWR, ACC);
    if(ff < 0)
        merror("noteedit", FATAL);
 
    strncpy(outstr, str, 79);
    outstr[79] = 0;
    strcat(outstr, "\n");
 
    write(ff, outstr, strlen(outstr));
 
    close(ff);
 
    print(fd, "->");
 
    output_buf();
    Ply[fd].io->intrpt &= ~1;
    RETURN(fd, noteedit, 2);
}


/****************************************************************************/
/*                              newspaper edit                              */
/****************************************************************************/
/*  This is for DM's only.  It allows the DM to either append text to the   */
/*  Mordor newspaper, or to clear it, but previously written text cannot 	*/
/*  be edited on-line.         												*/ 

int edit_news(ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;

{
    room    *rom_ptr;
    char    file[80];
    int 	fd, ff;
 
    fd = ply_ptr->fd;
    rom_ptr = ply_ptr->parent_rom;
 
    if(ply_ptr->class < CARETAKER)
        return(PROMPT);
 
    sprintf(file, "%s/%s", LOGPATH, "/newspaper");
 
    if(cmnd->num == 2) {
        if(low(cmnd->str[1][0]) == 'a') {
            strcpy(Ply[fd].extr->tempstr[0], file);
            print(fd, "append:\n->");
            F_SET(Ply[fd].ply, PREADI);
            output_buf();
            Ply[fd].io->intrpt &= ~1;
            Ply[fd].io->fn = newsedit;
            Ply[fd].io->fnparam = 1;
            return(DOPROMPT);
        }
        else if(low(cmnd->str[1][0]) == 'd') {
            unlink(file);
            print(fd, "Clearing the newspaper.\n");
            return(PROMPT);
        }
        else if(low(cmnd->str[1][0]) == 'r') {
			view_file(fd, 1, file);
			return(DOPROMPT);
        }
        else {
            print(fd, "invalid option.\n");
            return(PROMPT);
        }
    }
    else {
		ANSI(fd, YELLOW);
		print(fd, "\nThere are two options:  *news [a|d]\n");
		ANSI(fd, WHITE);
		print(fd, "Option a allows text to be appended to the end of the newspaper.\n");
		print(fd, "The process is terminated with a single period on a new line.\n");
		print(fd, "Option d deletes the current news in the newspaper.\n");
 
        return(PROMPT);
    }
}
 
/************************************************************************/
/*                              newsedit                                */
/************************************************************************/
void newsedit(fd, param, str)
int 	fd;
int 	param;
char    *str;

{
    char    outstr[85], tmpstr[40], file[80];
    long    t;
    int 	ff;
 
    if(str[0] == '.') {
        F_CLR(Ply[fd].ply, PREADI);
        print(fd, "Text appended.\n");
        RETURN(fd, command, 1); 
    }
 
    ff = open(Ply[fd].extr->tempstr[0], O_RDONLY, 0);
    if(ff < 0) {
        ff = open(Ply[fd].extr->tempstr[0], O_CREAT | O_RDWR, ACC);

        sprintf(tmpstr,"\n%s\n", 
"============================ Mordor Newspaper =========================");
        write(ff, tmpstr, strlen(tmpstr));
        sprintf(tmpstr,"%s\n", 
"    This newspaper is for your use.  If you have some interesting news,");
        write(ff, tmpstr, strlen(tmpstr));
        sprintf(tmpstr,"%s\n", 
"    or want to advertise something, send mudmail to Styx.");
        write(ff, tmpstr, strlen(tmpstr));
        sprintf(tmpstr,"%s\n\n", 
"=======================================================================");
        write(ff, tmpstr, strlen(tmpstr));

    }
    close(ff);
 
    ff = open(Ply[fd].extr->tempstr[0], O_CREAT | O_APPEND | O_RDWR, ACC);
    if(ff < 0)
        merror("newsedit", FATAL);
 
    strncpy(outstr, str, 79);
    outstr[79] = 0;
    strcat(outstr, "\n");
 
    write(ff, outstr, strlen(outstr));
 
    close(ff);
 
    print(fd, "->");
 
    output_buf();
    Ply[fd].io->intrpt &= ~1;
    RETURN(fd, newsedit, 2);
}


/**************************************************************************/
/*								news								      */
/**************************************************************************/
/* This function allows a player to view the newspaper file				  */	

int news(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	char 	file[80];
	int		fd, c = 0, match = 0, num = 0;
	
	fd = ply_ptr->fd;


	strcpy(file, LOGPATH);

		strcat(file, "/newspaper");
		ANSI(fd, NORMAL);
		ANSI(fd, BLUEBG);
		ANSI(fd, WHITE);
		view_file(fd, 1, file);
		return(DOPROMPT);
}
