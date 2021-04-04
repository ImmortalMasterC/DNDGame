/*
 *  util.c
 *
 *  Stuff copied from various files.  These are the functions
 *  that are needed by gstat and plist.
 *
 *  History:
 *  APR23 '96  <pdtelford@ucdavis.edu>
 *  All functions were blatantly copied from the Mordor distribution
 *  to be used here.
 *
 */

#include "lib/util.h"


/* strings needed for plist */

char class_str[][15] = { "None", "Assassin", "Barbarian", "Cleric",
	"Fighter", "Mage", "Paladin", "Ranger", "Thief", "Bard", 
	"Monk", "Druid", "Alchemist", "Caretaker", "Dungeonmaster" };

char race_str[][15] = { "Unknown", "Dwarf", "Elf", "Half-elf", "Hobbit",
	"Human", "Orc", "Half-giant", "Gnome", "Satyr", "Half-orc", "Ogre",
	"Dark-elf", "Centaur" };



/***********************************************************************/
/*							load_ply							       */
/***********************************************************************/

/* This function loads the player specified by the string in the first */
/* parameter, and returns the player in the second parameter.	       */

int load_ply(str, ply_ptr)
char		*str;
creature	**ply_ptr;
{
	char	file[80];
	int	fd, n;
#ifdef COMPRESS
	char	*a_buf, *b_buf;
	int	size;
#endif

	sprintf(file, "%s/%s", PLAYERPATH, str);
	fd = open(file, O_RDONLY, 0);
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


/**********************************************************************/
/*                              free_obj                              */
/**********************************************************************/

/* This function is called to release the object pointed to by the first */
/* parameter from memory.  All objects contained within it are also      */
/* released from memory.  *ASSUMPTION*:  This function will only be      */
/* called from free_rom() or free_crt().  Otherwise there may be         */
/* unresolved links in memory which would cause a game crash.            */

void free_obj(obj_ptr)
object  *obj_ptr;
{
        otag    *op, *temp;

        op = obj_ptr->first_obj;
        while(op) {
                temp = op->next_tag;
                free_obj(op->obj);
                free(op);
                op = temp;
        }
        free(obj_ptr);
}

/**********************************************************************/
/*                              read_crt                              */
/**********************************************************************/

/* Loads a creature from an open and positioned file.  The creature is */
/* loaded at the mem location specified by the second parameter.  In   */
/* addition, all the creature's objects have memory allocated for them */
/* and are loaded as well.  Returns -1 on fail.                        */

int read_crt(fd, crt_ptr)
int             fd;
creature        *crt_ptr;
{
        int             n, cnt, error=0;
        otag            *op;
        otag            **prev;
        object          *obj;

        n = read(fd, crt_ptr, sizeof(creature));
        if(n < sizeof(creature))
                error = 1;

        crt_ptr->first_obj = 0;
        crt_ptr->first_fol = 0;
        crt_ptr->first_enm = 0;
        crt_ptr->first_tlk = 0;
        crt_ptr->parent_rom = 0;
        crt_ptr->following = 0;
        for(n=0; n<MAXWEAR; n++)
                crt_ptr->ready[n] = 0;
        if(crt_ptr->mpcur > crt_ptr->mpmax)
                crt_ptr->mpcur = crt_ptr->mpmax;
        if(crt_ptr->hpcur > crt_ptr->hpmax)
                crt_ptr->hpcur = crt_ptr->hpmax;

        n = read(fd, &cnt, sizeof(int));
        if(n < sizeof(int)) {
                error = 1;
                cnt = 0;
        }

        prev = &crt_ptr->first_obj;
        while(cnt > 0) {
                cnt--;
                op = (otag *)malloc(sizeof(otag));
                if(op) {
                        obj = (object *)malloc(sizeof(object));
                        if(obj) {
                                if(read_obj(fd, obj) < 0)
                                        error = 1;
                                obj->parent_crt = crt_ptr;
                                op->obj = obj;
                                op->next_tag = 0;
                                *prev = op;
                                prev = &op->next_tag;
                        }
                        else
                                merror("read_crt", FATAL);
                }
                else
                        merror("read_crt", FATAL);
        }

        if(error)
                return(-1);
        else
                return(0);
}

/************************************************************************/
/*                              merror                                  */
/************************************************************************/

/* merror is called whenever an error message should be output to the   */
/* log file.  If the error is fatal, then the program is aborted        */

void merror(str, errtype)
char    *str;
char    errtype;
{
        long t;
        char bugstr[256];

        t = time(0);
        sprintf(bugstr, "Error occured in %s. %s", str, ctime(&t));
        logf(bugstr);
        if(errtype == FATAL)
                exit(-1);
}

/************************************************************************/
/*                              logf                                    */
/************************************************************************/

/* This function writes a formatted printf string to a logfile called   */
/* "log" in the player directory.                                       */

void logf(fmt, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10)
char    *fmt;
int     i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
{
        char    file[80];
        char    str[1024];
        int     fd;

        sprintf(file, "%s/log", LOGPATH);
        fd = open(file, O_RDWR, 0);
        if(fd < 0) {
                fd = open(file, O_RDWR | O_CREAT, ACC);
                if(fd < 0) return;
        }
        lseek(fd, 0L, 2);

        sprintf(str, fmt, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10);

        write(fd, str, strlen(str));

        close(fd);

}



static ctag *first_active;

/**********************************************************************/
/*                              free_crt                              */
/**********************************************************************/

/* This function releases the creature pointed to by the first parameter */
/* from memory.  All items that creature has readied or carries will     */
/* also be released.  *ASSUMPTION*:  This function will only be called   */
/* from free_rom().  If it is called from somewhere else, unresolved     */
/* links may remain and cause a game crash.  *EXCEPTION*: This function  */
/* can be called independently to free a player's information from       */
/* memory (but not a monster).                                           */

void free_crt(crt_ptr)
creature        *crt_ptr;
{
        otag    *op, *tempo;
        etag    *ep, *tempe;
        ctag    *cp, *tempc;
        ttag    *tp, *tempt;
        int     i;
        for(i=0; i<MAXWEAR; i++)
                if(crt_ptr->ready[i]) {
                        free_obj(crt_ptr->ready[i]);
                        crt_ptr->ready[i] = 0;
                }

        op = crt_ptr->first_obj;
        while(op) {
                tempo = op->next_tag;
                free_obj(op->obj);
                free(op);
                op = tempo;
        }

        cp = crt_ptr->first_fol;
        while(cp) {
                tempc = cp->next_tag;
                free(cp);
                cp = tempc;
        }

        ep = crt_ptr->first_enm;
        while(ep) {
                tempe = ep->next_tag;
                free(ep);
                ep = tempe;
        }

        tp = crt_ptr->first_tlk;
        while(tp) {
                tempt = tp->next_tag;
                if(tp->key) free(tp->key);
                if(tp->response) free(tp->response);
                if(tp->action) free(tp->action);
                if(tp->target) free(tp->target);
                free(tp);
                tp = tempt;
        }

        del_active(crt_ptr);
        free(crt_ptr);
}


/**********************************************************************/
/*              del_active                */
/**********************************************************************/

/* This function removes a monster from the active-monster list.  The */
/* parameter contains a pointer to the monster which is to be removed */

void del_active(crt_ptr)
creature    *crt_ptr;
{
    ctag    *cp, *prev;

    if(!(cp = first_active)) return;
    if(cp->crt == crt_ptr) {
        first_active = cp->next_tag;
        free(cp);
        return;
    }

    prev = cp;
    cp = cp->next_tag;
    while(cp) {
        if(cp->crt == crt_ptr) {
            prev->next_tag = cp->next_tag;
            free(cp);
            return;
        }
        prev = cp;
        cp = cp->next_tag;
    }

}


/************************************************************************/
/*				read_obj_from_mem			*/
/************************************************************************/

/* Loads the object from memory, returns the number of bytes read,	*/
/* and also loads every object which it might contain.  Returns -1 if	*/
/* there was an error.							*/

int read_obj_from_mem(buf, obj_ptr)
char	*buf;
object 	*obj_ptr;
{
	int 		n, cnt, error=0;
	char		*bufstart;
	otag		*op;
	otag		**prev;
	object 		*obj;

	bufstart = buf;

	memcpy(obj_ptr, buf, sizeof(object));
	buf += sizeof(object);

	obj_ptr->first_obj = 0;
	obj_ptr->parent_obj = 0;
	obj_ptr->parent_rom = 0;
	obj_ptr->parent_crt = 0;
	if(obj_ptr->shotscur > obj_ptr->shotsmax)
		obj_ptr->shotscur = obj_ptr->shotsmax;

	memcpy(&cnt, buf, sizeof(int));
	buf += sizeof(int);

	prev = &obj_ptr->first_obj;
	while(cnt > 0) {
		cnt--;
		op = (otag *)malloc(sizeof(otag));
		if(op) {
			obj = (object *)malloc(sizeof(object));
			if(obj) {
				if((n = read_obj_from_mem(buf, obj)) < 0)
					error = 1;
				else
					buf += n;
				obj->parent_obj = obj_ptr;
				op->obj = obj;
				op->next_tag = 0;
				*prev = op;
				prev = &op->next_tag;
			}
			else
				merror("read_obj", FATAL);
		}
		else
			merror("read_obj", FATAL);
	}

	if(error)
		return(-1);
	else
		return(buf - bufstart);
}

/************************************************************************/
/*				read_crt_from_mem			*/
/************************************************************************/

/* Loads a creature from memory & returns bytes read.  The creature is	*/
/* loaded at the mem location specified by the second parameter.  In	*/
/* addition, all the creature's objects have memory allocated for them	*/
/* and are loaded as well.  Returns -1 on fail.				*/

int read_crt_from_mem(buf, crt_ptr)
char		*buf;
creature 	*crt_ptr;
{
	int 		n, cnt, error=0;
	char		*bufstart;
	otag		*op;
	otag		**prev;
	object 		*obj;

	bufstart = buf;

	memcpy(crt_ptr, buf, sizeof(creature));
	buf += sizeof(creature);

	crt_ptr->first_obj = 0;
	crt_ptr->first_fol = 0;
	crt_ptr->first_enm = 0;
	crt_ptr->parent_rom = 0;
	crt_ptr->following = 0;
	for(n=0; n<MAXWEAR; n++)
		crt_ptr->ready[n] = 0;
	if(crt_ptr->mpcur > crt_ptr->mpmax)
		crt_ptr->mpcur = crt_ptr->mpmax;
	if(crt_ptr->hpcur > crt_ptr->hpmax)
		crt_ptr->hpcur = crt_ptr->hpmax;

	memcpy(&cnt, buf, sizeof(int));
	buf += sizeof(int);

	prev = &crt_ptr->first_obj;
	while(cnt > 0) {
		cnt--;
		op = (otag *)malloc(sizeof(otag));
		if(op) {
			obj = (object *)malloc(sizeof(object));
			if(obj) {
				if((n = read_obj_from_mem(buf, obj)) < 0)
					error = 1;
				else
					buf += n;
				obj->parent_crt = crt_ptr;
				op->obj = obj;
				op->next_tag = 0;
				*prev = op;
				prev = &op->next_tag;
			}
			else
				merror("read_crt", FATAL);
		}
		else
			merror("read_crt", FATAL);
	}

	if(error)
		return(-1);
	else
		return(buf - bufstart);
}



/**********************************************************************/
/*				read_obj			      */
/**********************************************************************/

/* Loads the object from the open and positioned file described by fd */
/* and also loads every object which it might contain.  Returns -1 if */
/* there was an error.						      */

int read_obj(fd, obj_ptr)
int 	fd;
object 	*obj_ptr;
{
	int 		n, cnt, error=0;
	otag		*op;
	otag		**prev;
	object 		*obj;

	n = read(fd, obj_ptr, sizeof(object));
	if(n < sizeof(object))
		error = 1;

	obj_ptr->first_obj = 0;
	obj_ptr->parent_obj = 0;
	obj_ptr->parent_rom = 0;
	obj_ptr->parent_crt = 0;
	if(obj_ptr->shotscur > obj_ptr->shotsmax)
		obj_ptr->shotscur = obj_ptr->shotsmax;

	n = read(fd, &cnt, sizeof(int));
	if(n < sizeof(int)) {
		error = 1;
		cnt = 0;
	}

	prev = &obj_ptr->first_obj;
	while(cnt > 0) {
		cnt--;
		op = (otag *)malloc(sizeof(otag));
		if(op) {
			obj = (object *)malloc(sizeof(object));
			if(obj) {
				if(read_obj(fd, obj) < 0)
					error = 1;
				obj->parent_obj = obj_ptr;
				op->obj = obj;
				op->next_tag = 0;
				*prev = op;
				prev = &op->next_tag;
			}
			else
				merror("read_obj", FATAL);
		}
		else
			merror("read_obj", FATAL);
	}

	if(error)
		return(-1);
	else
		return(0);
}

