/*
 *  util2.c
 *
 *  more functions needed for psearch
 *
 *  History:
 *  APR22 '96 <pdtelford@ucdavis.edu>
 *  Functions blatantly copied from various mordor files.
 */

#include "lib/util.h"


static csparse	Crt[CMAX];
static osparse	Obj[OMAX];

	/* Queue header and tail pointers */
static qtag	*Crthead = 0;
static qtag	*Crttail = 0;
static qtag	*Objhead = 0;
static qtag	*Objtail = 0;

		/* Queue sizes */
int	Csize = 0;
int	Osize = 0;




/**********************************************************************/
/*						load_obj								      */
/**********************************************************************/

/* This function loads the object specified by the first parameter, and */
/* returns a pointer to it in the second parameter.  If the object has  */
/* already been loaded before, then a pointer is merely returned.       */
/* Otherwise, the object is loaded into memory and the pointer is       */
/* returned.  If there are too many objects in memory, then the least   */
/* recently used objects are freed from memory.				*/

int load_obj(index, obj_ptr)
int	index;
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
		fd = open(file, O_RDONLY, 0);
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


/**********************************************************************/
/*						put_queue								      */
/**********************************************************************/

/* put_queue places the queue tag pointed to by the first paramater onto */
/* a queue whose head and tail tag pointers are the second and third     */
/* parameters.  If parameters 2 & 3 are 0, then a new queue is created.  */
/* The fourth parameter points to a queue size counter which is 	 */
/* incremented.							         */

void put_queue(qt, headptr, tailptr, sizeptr)
qtag	**qt;
qtag	**headptr;
qtag	**tailptr;
int	*sizeptr;
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

/**********************************************************************/
/*							pull_queue							      */
/**********************************************************************/

/* pull_queue removes the last queue tag on the queue specified by the */
/* second and third parameters and returns that tag in the first       */
/* parameter.  The fourth parameter points to a queue size counter     */
/* which is decremented.					       */

void pull_queue(qt, headptr, tailptr, sizeptr)
qtag 	**qt;
qtag	**headptr;
qtag	**tailptr;
int	*sizeptr;
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

/**********************************************************************/
/*						front_queue								      */
/**********************************************************************/

/* front_queue removes the queue tag pointed to by the first parameter */
/* from the queue (specified by the second and third parameters) and   */
/* places it back at the head of the queue.  The fourth parameter is a */
/* pointer to a queue size counter, and it remains unchanged.          */

void front_queue(qt, headptr, tailptr, sizeptr)
qtag	**qt;
qtag	**headptr;
qtag	**tailptr;
int	*sizeptr;
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


