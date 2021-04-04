/*
 * FILES2.C:
 *
 *	Additional file routines, including memory management.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"


rsparse	Rom[RMAX];	/* Pointer array declared */
csparse	Crt[CMAX];
osparse	Obj[OMAX];

static qtag	*Romhead = 0;	/* Queue header and tail pointers */
static qtag	*Romtail = 0;
static qtag	*Crthead = 0;
static qtag	*Crttail = 0;
static qtag	*Objhead = 0;
static qtag	*Objtail = 0;

int	Rsize = 0;		/* Queue sizes */
int	Csize = 0;
int	Osize = 0;

/**************************************************************************/
/*								load_rom							      */
/**************************************************************************/
/* This function accepts a room number as its first argument and then     */
/* returns a dblpointer to that room's data in the second parameter.      */
/* If the room has already been loaded, the pointer is simply returned.   */
/* Otherwise, the room is loaded into memory.  If a maximal number of     */
/* rooms is already in the memory, then the least recently used room      */
/* is stored back to disk, and the memory is freed.		        		  */

int load_rom(index, rom_ptr)
int		index;
room	**rom_ptr;

{
	int		fd, strct;
	qtag	*qt;
	char	file[256], filebak[256], rmfrstprt[4];

	if(index >= RMAX || index < 0)
		return(-1);

	/* Check if room is already loaded, and if so return pointer */

	if(Rom[index].rom) {
		front_queue(&Rom[index].q_rom, &Romhead, &Romtail, &Rsize);
		*rom_ptr = Rom[index].rom;
	}

	/* Otherwise load the room, store rooms if queue size becomes */
	/* too big, and return a pointer to the newly loaded room     */

	else {
#ifdef HASHROOMS
	sprintf(rmfrstprt, "10/r");
	if(index < 10000) sprintf(rmfrstprt, "09/r");
	if(index < 9000) sprintf(rmfrstprt, "08/r");
	if(index < 8000) sprintf(rmfrstprt, "07/r");
	if(index < 7000) sprintf(rmfrstprt, "06/r");
	if(index < 6000) sprintf(rmfrstprt, "05/r");
	if(index < 5000) sprintf(rmfrstprt, "04/r");
	if(index < 4000) sprintf(rmfrstprt, "03/r");
	if(index < 3000) sprintf(rmfrstprt, "02/r");
	if(index < 2000) sprintf(rmfrstprt, "01/r");
	if(index < 1000) sprintf(rmfrstprt, "00/r");
	sprintf(file, "%s/r%s%05d", ROOMPATH, rmfrstprt, index);
#else
		sprintf(file, "%s/r%05d", ROOMPATH, index);
#endif
		fd = open(file, O_RDONLY | O_BINARY, 0);
		if(fd < 0)
			return(-1);
		*rom_ptr = (room *)malloc(sizeof(room));
		if(!*rom_ptr)
			merror("load_rom", FATAL);
		if(read_rom(fd, *rom_ptr) < 0) {
			close(fd);
			return(-1);
		}
		close(fd);

		(*rom_ptr)->rom_num = index;

		qt = (qtag *)malloc(sizeof(qtag));
		if(!qt)
			merror("load_rom", FATAL);
		qt->index = index;
		Rom[index].rom = *rom_ptr;
		Rom[index].q_rom = qt;
		put_queue(&qt, &Romhead, &Romtail, &Rsize);

		while(Rsize > RQMAX) {

			pull_queue(&qt, &Romhead, &Romtail, &Rsize);
			if(Rom[qt->index].rom->first_ply) {
				put_queue(&qt, &Romhead, &Romtail, &Rsize);
				continue;
			}
#ifdef HASHROOMS
			sprintf(rmfrstprt, "10/r");
			if(index < 10000) sprintf(rmfrstprt, "09/r");
			if(index < 9000) sprintf(rmfrstprt, "08/r");
			if(index < 8000) sprintf(rmfrstprt, "07/r");
			if(index < 7000) sprintf(rmfrstprt, "06/r");
			if(index < 6000) sprintf(rmfrstprt, "05/r");
			if(index < 5000) sprintf(rmfrstprt, "04/r");
			if(index < 4000) sprintf(rmfrstprt, "03/r");
			if(index < 3000) sprintf(rmfrstprt, "02/r");
			if(index < 2000) sprintf(rmfrstprt, "01/r");
			if(index < 1000) sprintf(rmfrstprt, "00/r");
			sprintf(file, "%s/r%s%05d", ROOMPATH, rmfrstprt, index);
#else
			sprintf(file, "%s/r%05d", ROOMPATH, qt->index);
#endif
			sprintf(filebak, "%s~", file);
			rename(file, filebak);
			fd = open(file, O_RDWR | O_CREAT | O_BINARY, ACC);
			if(fd < 1)
				return(-1);
			if(!Rom[qt->index].rom)
				merror("ERROR - load_rom", NONFATAL);
			if(write_rom(fd, Rom[qt->index].rom, PERMONLY) < 0) {
				close(fd);
				unlink(file);
				rename(filebak, file);
				merror("ERROR - write_rom", NONFATAL);
				return(-1);
			}
			close(fd);
			unlink(filebak);
			free_rom(Rom[qt->index].rom);
			Rom[qt->index].rom = 0;
			free(qt);
		}
	}

	return(0);
}

/**************************************************************************/
/*							is_rom_loaded							      */
/**************************************************************************/
 
int is_rom_loaded(num)
int	num;
{
	return(Rom[num].rom != 0);
}

/**************************************************************************/
/*								reload_rom							      */
/**************************************************************************/
/* This function reloads a room from disk, if it's already loaded.  This  */
/* allows you to make changes to a room, and then reload it, even if it's */
/* already in the memory room queue.					 				  */

int reload_rom(num)
int	num;

{
	room	*rom_ptr;
	ctag	*cp;
	otag	*op;
	char	file[80], rmfrstprt[4];
	int		fd, index, strct;

	if(!Rom[num].rom)
		return(0);
#ifdef HASHROOMS
	index = num;
	sprintf(rmfrstprt, "10/r");
	if(index < 10000) sprintf(rmfrstprt, "09/r");
	if(index < 9000) sprintf(rmfrstprt, "08/r");
	if(index < 8000) sprintf(rmfrstprt, "07/r");
	if(index < 7000) sprintf(rmfrstprt, "06/r");
	if(index < 6000) sprintf(rmfrstprt, "05/r");
	if(index < 5000) sprintf(rmfrstprt, "04/r");
	if(index < 4000) sprintf(rmfrstprt, "03/r");
	if(index < 3000) sprintf(rmfrstprt, "02/r");
	if(index < 2000) sprintf(rmfrstprt, "01/r");
	if(index < 1000) sprintf(rmfrstprt, "00/r");
	sprintf(file, "%s/r%s%05d", ROOMPATH, rmfrstprt, index);
#else
	sprintf(file, "%s/r%05d", ROOMPATH, num);
#endif
	fd = open(file, O_RDONLY | O_BINARY, 0);
	if(fd < 0)
		return(-1);
	rom_ptr = (room *)malloc(sizeof(room));
	if(!rom_ptr)
		merror("reload_rom", FATAL);
	if(read_rom(fd, rom_ptr) < 0) {
		close(fd);
		return(-1);
	}
	close(fd);

	rom_ptr->first_ply = Rom[num].rom->first_ply;
	Rom[num].rom->first_ply = 0;

	add_permcrt_rom(rom_ptr);
	if(!rom_ptr->first_mon) {
		rom_ptr->first_mon = Rom[num].rom->first_mon;
		Rom[num].rom->first_mon = 0;
	}

	if(!rom_ptr->first_obj) {
		rom_ptr->first_obj = Rom[num].rom->first_obj;
		Rom[num].rom->first_obj = 0;
	}

	free_rom(Rom[num].rom);
	Rom[num].rom = rom_ptr;

	cp = rom_ptr->first_ply;
	while(cp) {
		cp->crt->parent_rom = rom_ptr;
		cp = cp->next_tag;
	}

	cp = rom_ptr->first_mon;
	while(cp) {
		cp->crt->parent_rom = rom_ptr;
		cp = cp->next_tag;
	}

	op = rom_ptr->first_obj;
	while(op) {
		op->obj->parent_rom = rom_ptr;
		op = op->next_tag;
	}

	return(0);
}

/**************************************************************************/
/*								resave_rom							      */
/**************************************************************************/
/* This function saves an already-loaded room back to memory without 	  */
/* altering its position on the queue.				     				  */

int resave_rom(num)
int	num;

{
	char	file[256], filebak[256], rmfrstprt[4];
	int		fd, index, strct;

	if(!Rom[num].rom)
		return(0);
#ifdef HASHROOMS
	index = num;
	sprintf(rmfrstprt, "10/r");
	if(index < 10000) sprintf(rmfrstprt, "09/r");
	if(index < 9000) sprintf(rmfrstprt, "08/r");
	if(index < 8000) sprintf(rmfrstprt, "07/r");
	if(index < 7000) sprintf(rmfrstprt, "06/r");
	if(index < 6000) sprintf(rmfrstprt, "05/r");
	if(index < 5000) sprintf(rmfrstprt, "04/r");
	if(index < 4000) sprintf(rmfrstprt, "03/r");
	if(index < 3000) sprintf(rmfrstprt, "02/r");
	if(index < 2000) sprintf(rmfrstprt, "01/r");
	if(index < 1000) sprintf(rmfrstprt, "00/r");
	sprintf(file, "%s/r%s%05d", ROOMPATH, rmfrstprt, index);
#else
	sprintf(file, "%s/r%05d", ROOMPATH, num);
#endif
	sprintf(filebak, "%s~", file);
	rename(file, filebak);
	fd = open(file, O_RDWR | O_CREAT | O_BINARY, ACC);
	if(fd < 1)
		return(-1);
	if(write_rom(fd, Rom[num].rom, PERMONLY) < 0) {
		close(fd);
		unlink(file);
		rename(filebak, file);
		return(-1);
	}
	close(fd);
	unlink(filebak);

	return(0);
}

/**************************************************************************/
/*								resave_all_rom						      */
/**************************************************************************/
/* This function saves all memory-resident rooms back to disk.  If the    */
/* permonly parameter is non-zero, then only permanent items in those     */
/* rooms are saved back.					       						  */

void resave_all_rom(permonly)
int	permonly;

{
	qtag 	*qt;
	char	file[80], rmfrstprt[4];
	int		fd, index, strct;

	qt = Romhead;
	while(qt) {
		if(!Rom[qt->index].rom) {
			qt = qt->next;
			continue;
		}
#ifdef HASHROOMS
		index = qt->index;	
		sprintf(rmfrstprt, "10/r");
		if(index < 10000) sprintf(rmfrstprt, "09/r");
		if(index < 9000) sprintf(rmfrstprt, "08/r");
		if(index < 8000) sprintf(rmfrstprt, "07/r");
		if(index < 7000) sprintf(rmfrstprt, "06/r");
		if(index < 6000) sprintf(rmfrstprt, "05/r");
		if(index < 5000) sprintf(rmfrstprt, "04/r");
		if(index < 4000) sprintf(rmfrstprt, "03/r");
		if(index < 3000) sprintf(rmfrstprt, "02/r");
		if(index < 2000) sprintf(rmfrstprt, "01/r");
		if(index < 1000) sprintf(rmfrstprt, "00/r");
		sprintf(file, "%s/r%s%05d", ROOMPATH, rmfrstprt, index);
#else
		sprintf(file, "%s/r%05d", ROOMPATH, qt->index);
#endif
		fd = open(file, O_RDWR | O_CREAT | O_BINARY, ACC);
		if(fd < 1)
			return;
		if(write_rom(fd, Rom[qt->index].rom, permonly) < 0) {
			close(fd);
			return;
		}
		close(fd);
		qt = qt->next;
	}
}

/*************************************************************************/
/*							save_all_ply			 				     */
/*************************************************************************/
/* This function saves all players currently in memory.			         */

void save_all_ply()

{
	int i;

	for(i = 0; i < Tablesize; i++) {
		if(Ply[i].ply && Ply[i].io && Ply[i].ply->name[0])
			savegame(Ply[i].ply, 0);
	}
}

/**************************************************************************/
/*								flush_rom							      */
/**************************************************************************/
/* This function flushes out the room queue and clears the room sparse    */
/* pointer array, without saving anything to file.  It also clears all    */
/* memory used by loaded rooms.  Call this function before leaving the    */
/* program.							       								  */

void flush_rom()

{
	qtag *qt;

	while(1) {
		pull_queue(&qt, &Romhead, &Romtail, &Rsize);
		if(!qt) break;
		free_rom(Rom[qt->index].rom);
		Rom[qt->index].rom = 0;
		free(qt);
	}
}

/*************************************************************************/
/*								flush_crt			 				     */
/*************************************************************************/
/* This function flushes out the monster queue and clears the monster 	 */
/* sparse pointer array without saving anything to file.  It also     	 */
/* clears all memory used by loaded creatures.  Call this function    	 */
/* before leaving the program.					      					 */

void flush_crt()

{
	qtag *qt;

	while(1) {
		pull_queue(&qt, &Crthead, &Crttail, &Csize);
		if(!qt) break;
		free_crt(Crt[qt->index].crt);
		Crt[qt->index].crt = 0;
		free(qt);
	}
}

/**************************************************************************/
/*								flush_obj			    				  */
/**************************************************************************/
/* This function flushes out the object queue and clears the object 	  */
/* sparse pointer array without saving anything to file.  It also   	  */
/* clears all memory used by loaded objects.  Call this function    	  */
/* before leaving the program.				    						  */

void flush_obj()

{
	qtag *qt;

	while(1) {
		pull_queue(&qt, &Objhead, &Objtail, &Osize);
		if(!qt) break;
		free_obj(Obj[qt->index].obj);
		Obj[qt->index].obj = 0;
		free(qt);
	}
}

/**************************************************************************/
/*								load_crt							      */
/**************************************************************************/
/* This function returns a pointer to the monster given by the index in   */
/* the first parameter.  The pointer is returned in the second.  If the   */
/* monster is already in memory, then a pointer is merely returned.       */
/* Otherwise, the monster is loaded into memory and a pointer is re-      */
/* turned.  If there are too many monsters in memory, then the least      */
/* recently used one is freed from memory.								  */

int load_crt(index, mon_ptr)
int			index;
creature	**mon_ptr;

{
	int		fd;
	long	n;
	qtag	*qt;
	char	file[256];

	if(index >= CMAX || index < 0)
		return(-1);

	/* Check if monster is already loaded, and if so return pointer */

	if(Crt[index].crt) {
		front_queue(&Crt[index].q_crt, &Crthead, &Crttail, &Csize);
		*mon_ptr = (creature *)malloc(sizeof(creature));
		**mon_ptr = *Crt[index].crt;
	}

	/* Otherwise load the monster, erase monsters if queue size          */
	/* becomes too big, and return a pointer to the newly loaded monster */

	else {
		sprintf(file, "%s/m%02d", MONPATH, index/MFILESIZE);
		fd = open(file, O_RDONLY | O_BINARY, 0);
		if(fd < 0) {
			*mon_ptr = 0;
			return(-1);
		}
		*mon_ptr = (creature *)malloc(sizeof(creature));
		if(!*mon_ptr)
			merror("load_crt", FATAL);
		n = lseek(fd, (long)((index%MFILESIZE)*sizeof(creature)), 0);
		if(n < 0L) {
			free(*mon_ptr);
			close(fd);
			*mon_ptr = 0;
			return(-1);
		}
		n = read(fd, *mon_ptr, sizeof(creature));
		close(fd);
		if(n < sizeof(creature)) {
			free(*mon_ptr);
			*mon_ptr = 0;
			return(-1);
		}

		(*mon_ptr)->fd = -1;
		qt = (qtag *)malloc(sizeof(qtag));
		if(!qt)
			merror("load_crt", FATAL);
		qt->index = index;
		Crt[index].crt = (creature *)malloc(sizeof(creature));
		*Crt[index].crt = **mon_ptr;
		Crt[index].q_crt = qt;
		put_queue(&qt, &Crthead, &Crttail, &Csize);

		while(Csize > CQMAX) {
			pull_queue(&qt, &Crthead, &Crttail, &Csize);
			free_crt(Crt[qt->index].crt);
			Crt[qt->index].crt = 0;
			free(qt);
		}
	}

	sprintf((*mon_ptr)->password, "%d", index);
	(*mon_ptr)->lasttime[LT_HEALS].ltime = time(0);
	(*mon_ptr)->lasttime[LT_HEALS].interval = 60L;
	(*mon_ptr)->first_enm = 0;

	return(0);
}

/**************************************************************************/
/*								load_obj							      */
/**************************************************************************/
/* This function loads the object specified by the first parameter, and   */
/* returns a pointer to it in the second parameter.  If the object has    */
/* already been loaded before, then a pointer is merely returned.         */
/* Otherwise, the object is loaded into memory and the pointer is         */
/* returned.  If there are too many objects in memory, then the least     */
/* recently used objects are freed from memory.							  */

int load_obj(index, obj_ptr)
int		index;
object	**obj_ptr;

{
	int		fd;
	long	n;
	qtag	*qt;
	char	file[256];

	if(index >= OMAX || index < 0)
		return(-1);

	/* Check if object is already loaded, and if so return pointer */

	if(Obj[index].obj) {
		front_queue(&Obj[index].q_obj, &Objhead, &Objtail, &Osize);
		*obj_ptr = (object *)malloc(sizeof(object));
		**obj_ptr = *Obj[index].obj;
	}

	/* Otherwise load the object, erase objects if queue size           */
	/* becomes too big, and return a pointer to the newly loaded object */

	else {
		sprintf(file, "%s/o%02d", OBJPATH, index/OFILESIZE);
		fd = open(file, O_RDONLY | O_BINARY, 0);
		if(fd < 0)
			return(-1);
		*obj_ptr = (object *)malloc(sizeof(object));
		if(!*obj_ptr)
			merror("load_obj", FATAL);
		n = lseek(fd, (long)((index%OFILESIZE)*sizeof(object)), 0);
		if(n < 0L) {
			close(fd);
			return(-1);
		}
		n = read(fd, *obj_ptr, sizeof(object));
		close(fd);
		if(n < sizeof(object))
			return(-1);

		qt = (qtag *)malloc(sizeof(qtag));
		if(!qt)
			merror("load_obj", FATAL);
		qt->index = index;
		Obj[index].obj = (object *)malloc(sizeof(object));
		*Obj[index].obj = **obj_ptr;
		Obj[index].q_obj = qt;
		put_queue(&qt, &Objhead, &Objtail, &Osize);
		while(Osize > OQMAX) {
			pull_queue(&qt, &Objhead, &Objtail, &Osize);
			free_obj(Obj[qt->index].obj);
			Obj[qt->index].obj = 0;
			free(qt);
		}
	}

	return(0);
}

/****************************************************************************/
/*								save_ply							        */
/****************************************************************************/
/* This function saves the player specified by the string in the first 		*/
/* parameter, and uses the player in the second parameter.	       			*/

int save_ply(str, ply_ptr)
char		*str;
creature	*ply_ptr;

{
	char	file[256], filebak[256];
	int		fd, n;

#ifdef COMPRESS
	char	*a_buf, *b_buf;
	int		size;
#endif

	sprintf(file, "%s/%s", PLAYERPATH, str);
	sprintf(filebak, "%s~", file);
	rename(file, filebak);
	fd = open(file, O_RDWR | O_CREAT | O_BINARY, ACC);
	if(fd < 0) {
		rename(filebak, file);
		return(-1);
	}

#ifdef COMPRESS
	a_buf = (char *)malloc(100000);
	if(!a_buf) merror("Memory allocation", FATAL);
	n = write_crt_to_mem(a_buf, ply_ptr, 0);
	if(n > 100000) merror(ply_ptr->name, FATAL);
	b_buf = (char *)malloc(n);
	if(!b_buf) merror("Memory allocation", FATAL);
	size = compress(a_buf, b_buf, n);
	n = write(fd, b_buf, size);
	free(a_buf);
	free(b_buf);
#else
	n = write_crt(fd, ply_ptr, 0);
	if(n < 0) {
		close(fd);
		unlink(file);
		rename(filebak, file);
		return(-1);
	}
#endif

	close(fd);
	unlink(filebak);
	return(0);

}

/****************************************************************************/
/*								load_ply							        */
/****************************************************************************/
/* This function loads the player specified by the string in the first 		*/
/* parameter, and returns the player in the second parameter.	       		*/

int load_ply(str, ply_ptr)
char		*str;
creature	**ply_ptr;

{
	char	file[80];
	int		fd, n;

#ifdef COMPRESS
	char	*a_buf, *b_buf;
	int		size;
#endif

	sprintf(file, "%s/%s", PLAYERPATH, str);
	fd = open(file, O_RDONLY | O_BINARY, 0);
	if(fd < 0) 
		return(-1);

	*ply_ptr = (creature *)malloc(sizeof(creature));
	if(!*ply_ptr)
		merror("load_ply", FATAL);

#ifdef COMPRESS
	a_buf = (char *)malloc(50000);
	if(!a_buf) merror("Memory allocation", FATAL);
	size = read(fd, a_buf, 50000);
	if(size >= 50000) merror("Player too large", FATAL);
	if(size < 1) {
		close(fd);
		return(-1);
	}
	b_buf = (char *)malloc(100000);
	if(!b_buf) merror("Memory allocation", FATAL);
	n = uncompress(a_buf, b_buf, size);
	if(n > 100000) merror("Player too large", FATAL);
	n = read_crt_from_mem(b_buf, *ply_ptr, 0);
	free(a_buf);
	free(b_buf);
#else
	n = read_crt(fd, *ply_ptr);
	if(n < 0) {
		close(fd);
		return(-1);
	}
#endif

	close(fd);
	return(0);
}

/*************************************************************************/
/*									put_queue			 			     */
/*************************************************************************/

/* put_queue places the queue tag pointed to by the first paramater onto */
/* a queue whose head and tail tag pointers are the second and third     */
/* parameters.  If parameters 2 & 3 are 0, then a new queue is created.  */
/* The fourth parameter points to a queue size counter which is 	 	 */
/* incremented.							         						 */

void put_queue(qt, headptr, tailptr, sizeptr)
qtag	**qt;
qtag	**headptr;
qtag	**tailptr;
int		*sizeptr;

{
	*sizeptr = *sizeptr + 1;

	if(!*headptr) {
		*headptr = *qt;
		*tailptr = *qt;
		(*qt)->next = 0;
		(*qt)->prev = 0;
	}

	else {
		(*headptr)->prev = *qt;
		(*qt)->next = *headptr;
		(*qt)->prev = 0;
		*headptr = *qt;
	}
}

/**************************************************************************/
/*								pull_queue							      */
/**************************************************************************/
/* pull_queue removes the last queue tag on the queue specified by the 	  */
/* second and third parameters and returns that tag in the first       	  */
/* parameter.  The fourth parameter points to a queue size counter     	  */
/* which is decremented.					       						  */

void pull_queue(qt, headptr, tailptr, sizeptr)
qtag 	**qt;
qtag	**headptr;
qtag	**tailptr;
int		*sizeptr;

{
	if(!*tailptr)
		*qt = 0;
	else {
		*sizeptr = *sizeptr - 1;
		*qt = *tailptr;
		if((*qt)->prev) {
			(*qt)->prev->next = 0;
			*tailptr = (*qt)->prev;
		}
		else {
			*headptr = 0;
			*tailptr = 0;
		}
	}
}

/*************************************************************************/
/*								front_queue			 				     */
/*************************************************************************/
/* front_queue removes the queue tag pointed to by the first parameter   */
/* from the queue (specified by the second and third parameters) and     */
/* places it back at the head of the queue.  The fourth parameter is a   */
/* pointer to a queue size counter, and it remains unchanged.            */

void front_queue(qt, headptr, tailptr, sizeptr)
qtag	**qt;
qtag	**headptr;
qtag	**tailptr;
int		*sizeptr;

{
	if((*qt)->prev) {
		((*qt)->prev)->next = (*qt)->next;
		if(*qt == *tailptr)
			*tailptr = (*qt)->prev;
	}
	if((*qt)->next) {
		((*qt)->next)->prev = (*qt)->prev;
		if(*qt == *headptr)
			*headptr = (*qt)->next;
	}
	if(!(*qt)->prev && !(*qt)->next) {
		*headptr = 0;
		*tailptr = 0;
	}
	(*qt)->next = 0;
	(*qt)->prev = 0;
	*sizeptr = *sizeptr - 1;

	put_queue(qt, headptr, tailptr, sizeptr);
}

/************************************************************************/
/*						 read_forbidden_name							*/
/************************************************************************/
/*	Compare login names with forbidden names.  Forbidden names are some	*/
/*  vulgar words and the names of permanent monsters.  There is a list	*/
/*  of names in objmon/forbidden_name_list								*/

int read_forbidden_name(str)
char *str;

{
	int		n, found = 0;
	char	file[80];
	char	name[80];
	FILE	*crt_name_list;

	if(str[0] == 'F') {
		if(str[1] == 'u' && 
           str[2] == 'c' && 
           str[3] == 'k')
		return(1);
	}
		
	for(n = 0; n < (strlen(str) - 3); n++)
		if(str[n] == 'f') {
			if(str[n + 1] == 'u' && 
               str[n + 2] == 'c' && 
               str[n + 3] == 'k')
			return(1);
		}
		
	if(str[0] == 'P') {
		if(str[1] == 'r' && 
           str[2] == 'i' && 
           str[3] == 'c' && 
           str[4] == 'k')
		return(1);
	}
		
	for(n = 0; n < (strlen(str) - 3); n++)
		if(str[n] == 'p') {
			if(str[n + 1] == 'r' && 
               str[n + 2] == 'i' && 
               str[n + 3] == 'c' &&
               str[n + 4] == 'k')
			return(1);
		}
		
	if(str[0] == 'D') {
		if(str[1] == 'i' && 
           str[2] == 'c' && 
           str[3] == 'k')
		return(1);
	}
		
	for(n = 0; n < (strlen(str) - 3); n++)
		if(str[n] == 'd') {
			if(str[n + 1] == 'i' && 
               str[n + 2] == 'c' &&
               str[n + 3] == 'k')
			return(1);
		}
		
	if(str[0] == 'P') {
		if(str[1] == 'u' && 
           str[2] == 's' && 
           str[3] == 's' && 
           str[4] == 'y')
		return(1);
	}
		
	for(n = 0; n < (strlen(str) - 3); n++)
		if(str[n] == 'p') {
			if(str[n + 1] == 'u' && 
               str[n + 2] == 's' && 
               str[n + 3] == 's' &&
               str[n + 4] == 'y')
			return(1);
		}
		
	if(str[0] == 'S') {
		if(str[1] == 'h' && 
           str[2] == 'i' && 
           str[3] == 't')
		return(1);
	}
		
	for(n = 0; n < (strlen(str) - 3); n++)
		if(str[n] == 's') {
			if(str[n + 1] == 'h' && 
               str[n + 2] == 'i' && 
               str[n + 3] == 't')
			return(1);
		}
		
	if(str[0] == 'S') {
		if(str[1] == 'u' && 
           str[2] == 'c' && 
           str[3] == 'k')
		return(1);
	}
		
	for(n = 0; n < (strlen(str) - 3); n++)
		if(str[n] == 's') {
			if(str[n + 1] == 'u' && 
               str[n + 2] == 'c' && 
               str[n + 3] == 'k')
			return(1);
		}
		
	if(str[0] == 'C') {
		if(str[1] == 'o' && 
           str[2] == 'c' && 
           str[3] == 'k')
		return(1);
	}
		
	for(n = 0; n < (strlen(str) - 3); n++)
		if(str[n] == 'c') {
			if(str[n + 1] == 'o' && 
               str[n + 2] == 'c' && 
               str[n + 3] == 'k')
			return(1);
		}
		
	if(str[0] == 'W') {
		if(str[1] == 'i' && 
           str[2] == 'l' && 
           str[3] == 's' &&
           str[4] == 'o' &&
           str[5] == 'n')
		return(1);
	}
		
	for(n = 0; n < (strlen(str) - 3); n++)
		if(str[n] == 'w') {
			if(str[n + 1] == 'i' && 
               str[n + 2] == 'l' && 
               str[n + 3] == 's' &&
               str[n + 4] == 'o' &&
               str[n + 5] == 'n')
			return(1);
		}
		

	sprintf(file, "%s/forbidden_name_list", MONPATH);
	crt_name_list = fopen(file, "r");
	if(crt_name_list != NULL) {
	    while (!found && !(feof (crt_name_list))) {
			fflush(crt_name_list);
	        fgets(name, sizeof(name), crt_name_list);
			name[strlen(name) - 1] = 0;
			if(strcmp(name, str) == 0) {
			    found = 1;
		     	fclose(crt_name_list);
			    return(1);	
			}
		}			
	}

	fclose(crt_name_list);
	return(-1);	
}
