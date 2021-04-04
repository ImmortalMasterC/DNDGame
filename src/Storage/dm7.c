/*
 * DM7.C:
 *
 *  DM functions
 *
 *  Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *  Copyright (C) 1994, 1995, 1996 Brooke Paul
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>
#include <stdlib.h>

/*************************************************************************/
/*			save creature					 */
/*  dm_save_crt allows a dm to save modified creatures. 		 */ 
/*************************************************************************/

int dm_save_crt(ply_ptr, cmnd)
creature 	*ply_ptr;
cmd		*cmnd;

{
	creature  	*crt_ptr;
	ttag      	*tp, *tempt;
        char	  	file[80];
	int	  	fd, ff, n, index, i = 0, x, y, bad = 0;
	otag	  	*op, *otemp;
	ctag	  	*fol;

	fd = ply_ptr->fd;

	if(ply_ptr->class < DM)
		return(PROMPT);

	if(cmnd->num < 3) {
		print(fd, "Syntax: *save c [name] [number]\n");
		return(0);
	}
        
	crt_ptr = find_crt(ply_ptr, ply_ptr->parent_rom->first_mon, 
		cmnd->str[2], 1);    
 
	if(!crt_ptr) {
		print(fd,"Creature not found.\n");
		return(PROMPT);
	}

	if(cmnd->val[2] < 2 || cmnd->val[2] > CMAX) {
		print(fd, "Index error: creature number invalid.\n");
		return(PROMPT);   
	}

	index = cmnd->val[2];
	tp = crt_ptr->first_tlk;
	while(tp) {
		tempt = tp->next_tag;
		if(tp->key) 
			free(tp->key);
		if(tp->response) 
			free(tp->response);
		if(tp->action) 
			free(tp->action);
		if(tp->target) 
			free(tp->target);
		free(tp);
		tp = tempt;
	}
	crt_ptr->first_tlk = 0;
	op=crt_ptr->first_obj;
	if(!F_ISSET(crt_ptr, MTRADE)) { 
		while(op) {
			x=find_obj_num(op->obj);
			if(!x) {
				print(fd,"Unique object in inventory not saved.\n");
				op=op->next_tag;
				continue;
			}
			crt_ptr->carry[i]=(long)x;
			i++;
			if(i>9) {
				print(fd, "Only first 10 objects in inventory saved.\n");
				break;
			}		
			op = op->next_tag;
		}


		/* clear remaining carry slots */
	        while(i < 9) {
                crt_ptr->carry[i] = 0;
                i++;
        	}

	}

	/* flush inventory -- dont exactly know why this is necessary... */
	op=crt_ptr->first_obj;
	crt_ptr->first_obj = 0;
	while(op) {
		otemp=op->next_tag;
		free_obj(op->obj);
		free(op);
		op=otemp;
	}

	/* clean up possessed before save */
	if(F_ISSET (crt_ptr, MDMFOL)) { 		/* clear relevant follow lists */
		F_CLR(crt_ptr->following, PALIAS);
		F_CLR(crt_ptr, MDMFOL);
		Ply[crt_ptr->following->fd].extr->alias_crt = 0;
		print (crt_ptr->following->fd, "%1M's soul was saved.\n", crt_ptr);
		fol = crt_ptr->following->first_fol;
		if(fol->crt == crt_ptr) {
			crt_ptr->following->first_fol = fol->next_tag;
			free(fol);
		}
	}
        /* Update index */
	sprintf(crt_ptr->password, "%d", index);

	sprintf(file, "%s/m%02d", MONPATH, index/MFILESIZE);
	ff = open(file, O_RDONLY, 0);
	if(ff >= 0)
		print(fd, "Creature #%d might already exist.\n", index);
	close(ff);
       
	sprintf(file, "%s/m%02d", MONPATH, index/MFILESIZE);
	ff = open(file, O_WRONLY | O_CREAT, ACC);
	if(ff == -1) {
		close(ff);
		print(fd, "File open error.\n");
		return(0);
	}
	n = lseek(ff, (long)((index%MFILESIZE)*sizeof(creature)), 0);
     
	n = write(ff, crt_ptr, sizeof(creature));
	if(n < sizeof(creature))
		merror("write_crt", FATAL);
	close(ff);
	print(fd, "Creature #%d updated.\n", index);

	return(0);
}

/**********************************************************************/
/*			dm_add_crt				      */
/**********************************************************************/
/* This function creates a generic creature for a DM to work on.      */

int dm_add_crt(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	creature  *new_crt;
	int	  fd, n;

	fd = ply_ptr->fd;

	if(ply_ptr->class < DM)
		return(PROMPT);

	new_crt = (creature *)malloc(sizeof(creature));
	if(!new_crt) {
		print(fd, "Cannot allocate memory for creature.\n");
		merror("dm_add_crt", NONFATAL);
		return(PROMPT);
	}	       

	zero(new_crt, sizeof(creature));
	strcpy(new_crt->name, "clay form");
	strcpy(new_crt->description, "An unformed mass of clay is here.");
	strcpy(new_crt->key[0], "form");
	new_crt->level = 1;
	new_crt->type = MONSTER;
	new_crt->strength     = 10;
	new_crt->dexterity    = 10;
	new_crt->constitution = 10;
	new_crt->intelligence = 10;
	new_crt->piety        = 10;
	new_crt->hpmax = new_crt->hpcur = 12;
	new_crt->armor = 100;
	new_crt->thaco = 19;
	new_crt->experience = 10;
	new_crt->ndice = 1;
	new_crt->sdice = 4;
	new_crt->pdice = 1;
	new_crt->first_obj =  0;
	new_crt->first_fol =  0;
	new_crt->first_enm =  0;
	new_crt->first_tlk =  0;
	new_crt->parent_rom = 0;
	new_crt->following =  0;
	for(n=0; n < 20; n++)
		new_crt->ready[n] = 0; 
	add_crt_rom(new_crt, ply_ptr->parent_rom, 1);
	print(fd, "Creature created.\n");

return(0);
}

/***********************************************************************/
/*  			dm add object			 		*/
/***********************************************************************/

int dm_add_obj(ply_ptr,cmnd)
creature *ply_ptr;
cmd      *cmnd;

{
	object	*new_obj;
	int	fd;

	fd = ply_ptr->fd;

	if(ply_ptr->class < DM)
		return(PROMPT);

	if(cmnd->val[1] < 1) {
		print(fd, "Add what?\n");
		return(0);
	}

	new_obj = (object *)malloc(sizeof(object));
	if(!new_obj) {
		merror("dm_add_obj", NONFATAL);
        print(fd, "Cannot allocate object.\n");
		return(0);
	}
	       
	zero(new_obj, sizeof(object));
	strcpy(new_obj->name, "small clay ball");
	strcpy(new_obj->description, 
		"You see a plain grey ball.  Perhaps you can make something of it.");
	strcpy(new_obj->key[0], "ball");
	strcpy(new_obj->key[1], "clay");
	strcpy(new_obj->use_output, "You squeeze a clay ball in your hand.");
	new_obj->wearflag = HELD;
	new_obj->type = MISC;
	new_obj->first_obj = 0;
	new_obj->parent_obj = 0;
	new_obj->parent_rom = 0;
	new_obj->parent_crt = 0;
	add_obj_crt(new_obj, ply_ptr);
	print(fd, "Generic object added to your inventory.\n");

	return(0);
}

/************************************************************************/
/*			dm save object					*/
/************************************************************************/

int dm_save_obj(ply_ptr, cmnd)
creature 	*ply_ptr;
cmd		*cmnd;

{
	object    	*obj_ptr, *po;
	otag      	*fo;
	room      	*rom;
	creature  	*pc;
	char	  	file[80];
	int		fd, ff, n, index;
        
	fd = ply_ptr->fd;

	if(ply_ptr->class < DM)
		return(PROMPT);

	if(cmnd->num < 3) {
		print(fd, "Syntax: *objsave o [object] [index]\n");
		return(0);
	}
     
	obj_ptr = find_obj(ply_ptr, ply_ptr->parent_rom->first_obj, 
		cmnd->str[2], 1);    
 
	if(!obj_ptr)
		obj_ptr = find_obj(ply_ptr, ply_ptr->first_obj, 
		cmnd->str[2], 1);
  
	if(!obj_ptr) {
		print(fd, "Object not found.\n");
		return(PROMPT);
	}
       
	if(cmnd->val[2] < 2 || cmnd->val[2] > OMAX) {
		print(fd, "Index error: object number invalid.\n");
		return(PROMPT);  
	}       

	index=cmnd->val[2];
	sprintf(file, "%s/o%02d", OBJPATH, index/OFILESIZE);
	ff = open(file, O_WRONLY, index);
	if(ff >= 0)
		print(fd, "Object might already exist.\n");
	close(ff);     

	sprintf(file, "%s/o%02d", OBJPATH, index/OFILESIZE);
	ff = open(file, O_WRONLY | O_CREAT, ACC);
	if(ff == -1) {
		close(ff);
		print(fd, "File open error.\n");
		return(0);
	}
	n = lseek(ff, (long)((index%OFILESIZE)*sizeof(object)), 0);
     
	rom = obj_ptr->parent_rom;
	po  = obj_ptr->parent_obj;
	fo  = obj_ptr->first_obj;
	pc  = obj_ptr->parent_crt;
	obj_ptr->parent_rom = 0;
	obj_ptr->parent_obj = 0;
	obj_ptr->first_obj =  0;
	obj_ptr->parent_crt = 0;    
	n = write(ff, obj_ptr, sizeof(object));
	if(n < sizeof(object))
		merror("write_obj", FATAL);
	close(ff);
	print(fd, "Object #%d updated.\n", index);
	obj_ptr->parent_rom = rom;
	obj_ptr->parent_obj = po;
	obj_ptr->first_obj = fo;
	obj_ptr->parent_crt = pc;    

	return(0);  
}

/************************************************************************/
/* 			Talk editing functions				*/ 
/************************************************************************/

int dm_add_talk(ply_ptr, cmnd)
creature *ply_ptr;
cmd      *cmnd;

{
	ttag     *talk;
	int      fd;
   
	fd = ply_ptr->fd;
   
	if(ply_ptr->class < DM)
		return(PROMPT);

	if(ply_ptr->first_tlk) {
		print(fd, "You have a talk defined already.\n");
		return(PROMPT);
	}

	talk = (ttag *) malloc(sizeof(ttag));
	if(!talk)
		merror("dm_add_talk", FATAL);   

	talk->next_tag = 0;
	talk->key      = 0;
	talk->response = 0;
	talk->type     = 0;
	talk->action   = 0;
	talk->target   = 0;
	ply_ptr->first_tlk = talk;
	print(fd, "Talk ready for input.\n");
   
	return(PROMPT);
}

/************************************************************************/
/* 			add talk key					*/ 
/************************************************************************/

int dm_add_talk_key(ply_ptr, cmnd)
creature *ply_ptr;
cmd      *cmnd;

{
	int      fd;
	char     *ptr;
   
	fd = ply_ptr->fd;
   
	if(ply_ptr->class < DM)
		return(PROMPT);

	if(!ply_ptr->first_tlk) {
		print(fd, "You must start a talk first.\n");
		return(PROMPT);
	}
   
	if(ply_ptr->first_tlk->key) {  
		print(fd, "Keyword replaced.\n");
		free(ply_ptr->first_tlk->key);
	}
	else
	print(fd, "Keyword set\n");

	ptr = cmnd->fullstr;
	for(; *ptr == ' '; ptr++);
	for(; *ptr != ' ' && *ptr; ptr++);
	for(; *ptr == ' ' && *ptr; ptr++);
	if(!*ptr) {
		print(fd, "*tkey <keyword [action] [action args]>");
		return 0;
	}

	ply_ptr->first_tlk->key = (char *)malloc(strlen(ptr)+1);
	if(!ply_ptr->first_tlk->key)
		merror("dm_add_talk_key", FATAL);
	strcpy(ply_ptr->first_tlk->key, ptr);
	return(PROMPT);
}

/************************************************************************/
/* 				add talk response			*/ 
/************************************************************************/

int dm_add_talk_resp(ply_ptr, cmnd)
creature *ply_ptr;
cmd      *cmnd;

{
	int      fd, append;
	char     *ptr;
	char     buf[1024];
   
	fd = ply_ptr->fd;
   
	if(ply_ptr->class < DM)
		return(PROMPT);

	if(!ply_ptr->first_tlk) {
		print(fd, "You must start a talk first.\n");
		return(PROMPT);
	}
  
	ptr = cmnd->fullstr;
	for(; *ptr == ' '; ptr++);
	for(; ; ptr++) {
		if(!*ptr) {
			print(fd, "*tres +/- <line>\n");
			return(PROMPT);	  
		}
		if(*ptr == '+') { 
			append = 1;
			break;
		}
		if(*ptr == '-') { 
			append = 0;
			break;
		}
	}
   
	if(!*(ptr+1))
		return PROMPT;
   
	if(append && ply_ptr->first_tlk->response) {
		strcpy(buf, ply_ptr->first_tlk->response);
		free(ply_ptr->first_tlk->response);
		strcat(buf, ++ptr);
	}
	else {	
		free(ply_ptr->first_tlk->response);
		strcpy(buf, ++ptr);
	}

	ply_ptr->first_tlk->response = (char *) malloc(strlen(buf)+1);
	if(!ply_ptr->first_tlk->response)
		merror("dm_add_talk_resp", FATAL);
	strcpy(ply_ptr->first_tlk->response, buf);
   
	return(PROMPT);
}

/************************************************************************/
/*			add talk show					*/
/************************************************************************/

int dm_add_talk_show(ply_ptr, cmnd)
creature *ply_ptr;
cmd      *cmnd;

{
	int	fd;
   
	fd = ply_ptr->fd;
   
	if(ply_ptr->class < DM)
		return(PROMPT);

	if(!ply_ptr->first_tlk) {
		print(fd, "You must start a talk first.\n");
		return(PROMPT);
	}

	if(ply_ptr->first_tlk->key) {
		print(fd, "Keyword: ");
		print(fd, ply_ptr->first_tlk->key);
		print(fd, "\n");
	}

	if(ply_ptr->first_tlk->response) {
		print(fd, "Response:\n");
		print(fd, ply_ptr->first_tlk->response);
		print(fd, "\n");
	}

	return(0);
}


/************************************************************************/
/*			add talk save					*/
/************************************************************************/

int dm_add_talk_save(ply_ptr, cmnd)
creature *ply_ptr;
cmd      *cmnd;

{
	int      fd, i;
	char     file[256];
	char     crt_name[80];
	FILE     *fp;
	creature *crt_ptr;
   
	fd = ply_ptr->fd;
   
	if(ply_ptr->class < DM)
		return(PROMPT);

	if(!ply_ptr->first_tlk) {
		print(fd, "You must start a talk first.\n");
		return(PROMPT);
	}

	if(!ply_ptr->first_tlk->key) {
		print(fd, "You must define a keyword.\n");
		return(PROMPT);
	}

	if(!ply_ptr->first_tlk->response) {
		print(fd, "You must supply a response.\n");
		return(PROMPT);
	}

	cmnd->val[1] = 1;
	crt_ptr = find_crt(ply_ptr,ply_ptr->parent_rom->first_mon, cmnd->str[1],
		cmnd->val[1]);    
 
	if(!crt_ptr) {
		print(fd, "Creature not found in the room.\n");
		return(PROMPT);
	}  

	strcpy(crt_name, crt_ptr->name);
	for(i = 0; crt_name[i]; i++)
		if(crt_name[i] == ' ')
			crt_name[i] = '_';
	sprintf(file,"%s/talk/%s-%d", MONPATH, crt_name, crt_ptr->level);
	fp = fopen(file, "a");
	fprintf(fp,"%s\n", ply_ptr->first_tlk->key);
	fprintf(fp,"%s\n", ply_ptr->first_tlk->response);
	fclose(fp);
	print(fd,"Talk saved.\n");

	return 0;
}

/************************************************************************/
/*	This function finds an object's or creature's number from the	*/
/*	database.  							*/
/************************************************************************/

int dm_find_db(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	int 		fd, n;
	object		*obj_ptr;
	creature	*crt_ptr;

	if(ply_ptr->class < CARETAKER)
		return(PROMPT);

	fd=ply_ptr->fd;

	if(cmnd->num < 3) {
		print(fd, "Syntax: *find o|c name [#]\n");
		return(0);
	}

	switch(low(cmnd->str[1][0])) {

		case 'o':
			obj_ptr=find_obj(ply_ptr, ply_ptr->first_obj, cmnd->str[2], 
				cmnd->val[2]);	
			if(!obj_ptr)
				obj_ptr=find_obj(ply_ptr, ply_ptr->parent_rom->first_obj, 
					cmnd->str[2], cmnd->val[2]);
			if(!obj_ptr) {
				print(fd, "Cannot locate object.\n");
				return(0);
			}
			n=find_obj_num(obj_ptr);
			if(n)
				print(fd, "Object is #%d.\n", n);
			else
				print(fd, "Object is unique.\n");

			break;	

		case 'm':

		case 'c':
			crt_ptr=find_crt(ply_ptr, ply_ptr->parent_rom->first_mon,
				cmnd->str[2], cmnd->val[2]);
			if(!crt_ptr) {
				print(fd, "Cannot locate creature.\n");
				return(0);
			}
			n = find_crt_num(crt_ptr);
			if(n)
				print(fd, "Creature is #%d.\n", n);
			else
				print(fd, "Creature is unique.\n");
			break;

		default:
			print(fd, "Syntax: *find o|c name [#]\n");
			return(0);
	}
}

/************************************************************************/
/*			remove from database				*/
/************************************************************************/
/*  This function allows the removal of an object/creature from the 	*/
/*  database.															*/

int dm_remove_db(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	int 		fd, ff, index,n;
	creature	*crt_ptr;
	object		*obj_ptr;
	char      	file[80];

	if(ply_ptr->class < DM)
		return(PROMPT);

	fd=ply_ptr->fd;

	if(cmnd->num < 2) {
		print(fd, "Syntax: *remove [o|c] [index]\n");
		return(0);
	}

	if(!strcmp(cmnd->str[1], "c") || !strcmp(cmnd->str[1], "m")) {
		if(cmnd->val[1] < 2 || cmnd->val[1] > CMAX) {
			print(fd, "Index error.\n");
			return(0);
		}
		index = cmnd->val[1];
		sprintf(file, "%s/m%02d", MONPATH, index/MFILESIZE);
       	ff = open(file, O_WRONLY | O_BINARY, ACC);
       	if(ff == -1) {
        	close(ff);
        	print(fd, "File open error.\n");
        	return(0);
        }
		crt_ptr=(creature *)malloc(sizeof(creature));
		zero(crt_ptr, sizeof(creature));
		n = lseek(ff, (long)((index%MFILESIZE) * sizeof(creature)), 0);
        n = write(ff, crt_ptr, sizeof(creature));
        if(n < sizeof(creature))
           	merror("dm_remove_db", FATAL);
        close(ff);
		print(fd,"Creature #%d removed from database.\n", index);
		return(0);
	}
	else {
		if(strcmp(cmnd->str[1], "o")) {	
           	print(fd, "Syntax: *remove [o|c] [index].\n");
           	return(0);
		}

		if(cmnd->val[1] < 2 || cmnd->val[1] > OMAX) {
			print(fd, "Index error.\n");
			return(0);
		}
	
		index = cmnd->val[1];
		sprintf(file, "%s/o%02d", OBJPATH, index/OFILESIZE);
		ff = open(file, O_WRONLY | O_BINARY, ACC);
		if(ff == -1) {
			close(ff);
			print(fd, "File open error.\n");
			return(0);
		}

		obj_ptr=(object *)malloc(sizeof(object));
		zero(obj_ptr, sizeof(object));
		n = lseek(ff, (long)((index%OFILESIZE) * sizeof(object)), 0);
		n = write(ff, obj_ptr, sizeof(object));
		if(n < sizeof(object))
			merror("dm_remove_db", FATAL);
		close(ff);
		print(fd, "Object #%d removed.\n", index);

		return(0);
	}
}
