/*
 * SCREEN.C:
 *
 *	 Screen handling routines.
 *
 *
 */
#include "mstruct.h"
#include "mextern.h"

/*************************************************************************/
void setcolor(fd, color)
	short	fd;
	int		color;

{
	if(!Ply[fd].ply)
		return;
	if(fd > -1)
		if(F_ISSET(Ply[fd].ply, PANSIC)) 
			print(fd, "%c[%dm", 27, color);
}

/*************************************************************************/
void rc(fd, x, y)
	short	fd;
	int		x;
	int		y;

{
	if(fd > -1) 
		print(fd, "%c[%d;%df", 27, x, y);
}


/*************************************************************************/
void scroll(fd, y1, y2)
	short	fd;
	int		y1;
	int		y2;

{
	if(fd > -1) 
		print(fd, "%c[%d;%dr", 27, y1, y2);
}

/*************************************************************************/
void delline(fd)
	short	fd;

{
	if(fd > -1) print(fd, "\033[K");
}

/*************************************************************************/
void clrscr(fd)
	short	fd;

{
	if(fd > -1) print(fd, "\033[2J");
}
	 
/*************************************************************************/
void save_cursor(fd)
	short	fd;

{
	if(fd > -1) print(fd, "\0337");
}

/*************************************************************************/
void load_cursor(fd)
	short	fd;

{
	if(fd > -1) print(fd, "\0338");
}



/************************************************************************/
/*                            clear_screen                              */
/************************************************************************/
/*   This function allows a player to clear his/her screen.             */

int clear_screen(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	int 	fd;
	fd = ply_ptr->fd;

	clrscr(fd);
}

