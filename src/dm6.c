/*
 * DM6.C:
 *
 *  DM functions
 *
 *  Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *  Copyright (C) 1995 Brooke Paul
 *
 */

#include "mstruct.h"
#include "mextern.h"

/**********************************************************************/
/*								dm_dust							      */
/**********************************************************************/
/* This function allows a DM to destroy a player.				      */

int dm_dust(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
	creature	*crt_ptr;
	int			cfd, fd;
	char		str[IBUFSIZE + 1]; 
	char		file[80];

	if(ply_ptr->class < DM)
		return(PROMPT);

	if(cmnd->num < 2) {
		print(ply_ptr->fd, "\nDust whom?\n");
		return(PROMPT);
	}

	lowercize(cmnd->str[1], 1);
	crt_ptr = find_who(cmnd->str[1]);
	if(!crt_ptr) {
		print(ply_ptr->fd, "%s is not on.\n", cmnd->str[1]);
		return(0);
	}

	if(crt_ptr->class > CARETAKER) {
		ANSI(crt_ptr->fd, RED);
		print(crt_ptr->fd, "%s tried to dust you!\n", ply_ptr->name);
		ANSI(crt_ptr->fd, BOLD);
		ANSI(crt_ptr->fd, WHITE);
		return(0);
	}

	cfd = crt_ptr->fd;
	if(!(Ply[cfd].io->fn == command && Ply[cfd].io->fnparam == 1)) {
		print(ply_ptr->fd, "Can't dust %s right now.\n", cmnd->str[1]);
		return(0);
	}
	ANSI(cfd, MAGENTA);
	write(cfd, "Lightning comes down from on high!  You have angered the "
				"Dungeonmaster!\n", 72);
	ANSI(cfd, BOLD);
	ANSI(cfd, WHITE);
	broadcast_rom(cfd, crt_ptr->rom_num,
	    "A bolt of lightning strikes %s from on high.\n", crt_ptr->name);
	broadcast("\n### %s has been turned to dust by a Dungeonmaster!  "
						"We'll miss %s dearly.",
	    Ply[cfd].ply->name, F_ISSET(Ply[cfd].ply, PMALES) ? "him":"her");
	sprintf(file, "%s/%s", PLAYERPATH, Ply[cfd].ply->name);
	disconnect(cfd);
	broadcast("### Ominous thunder rumbles in the distance.\n");
	broadcast("### Beware the lightning of the Dungeonmasters.\n");
	unlink(file);

	return(0);
}

/**********************************************************************/
/*                              dm_alias                              */
/**********************************************************************/
/*	This function allows a DM to possess a monster. 		    	  */

int dm_alias (ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	ctag		*pp, *cp, *prev;
	int			fd;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->class < CARETAKER)
		return(PROMPT);	

	if(cmnd->num < 2) {
		print (fd, "syntax: *possess <creature>\n");
		return(0);
	}
	
	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		print (fd, "Can't seem to locate that creature here.\n");
		return (0);
	}
	
	if(crt_ptr->type != MONSTER) {
		print (fd, "Their soul refuses to be displaced.\n");
		return (0);
	}

	if(F_ISSET(crt_ptr, MPERMT)) {
		print (fd, "Their soul refuses to be displaced.\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PALIAS) && crt_ptr != Ply[fd].extr->alias_crt) {
		print (fd, "You may only possess one creature at a time.\n");
		return(0);
	}

	if(F_ISSET(crt_ptr, MDMFOL)) {
		if(crt_ptr != Ply[fd].extr->alias_crt) {
			print (fd, "Their soul belongs to another.\n");
			return(0);
		}
		F_CLR(crt_ptr, MDMFOL);
		F_CLR(ply_ptr, PALIAS);
		print (fd, "You release %1m's body.\n", crt_ptr->name);
		/* *cfollow code */
		cp = ply_ptr->first_fol;
		if(cp->crt == crt_ptr) {
			ply_ptr->first_fol = cp->next_tag;
			free(cp);
		}
		else while(cp) {
			if(cp->crt == crt_ptr) {
				prev->next_tag = cp->next_tag;
				free(cp);
				break;
			}
		    prev = cp;
		    cp = cp->next_tag;
		} 
		crt_ptr->following = 0;	
		Ply[fd].extr->alias_crt = 0;
		return(0);
	}

	/* *cfollow code */

	crt_ptr->following = ply_ptr;
	pp = 0;
	pp = (ctag *)malloc(sizeof(ctag));
	if(!pp)
		merror("dmalias", FATAL);
	pp->crt = crt_ptr;
	pp->next_tag = 0;
	
	if(!ply_ptr->first_fol) {
		ply_ptr->first_fol = pp;
		pp->next_tag = 0;
	}
	else {
		pp->next_tag = ply_ptr->first_fol;
		ply_ptr->first_fol = pp;
	} 

	Ply[fd].extr->alias_crt = crt_ptr;
	F_SET(ply_ptr, PALIAS);	
	F_SET(crt_ptr, MDMFOL);

	ply_ptr->strength = crt_ptr->strength;
	ply_ptr->dexterity = crt_ptr->dexterity;
	ply_ptr->constitution = crt_ptr->constitution;
	ply_ptr->intelligence = crt_ptr->intelligence;
	ply_ptr->piety = crt_ptr->piety;
	ply_ptr->level = crt_ptr->level;
	ply_ptr->ndice = crt_ptr->ndice;
	ply_ptr->sdice = crt_ptr->sdice;
	ply_ptr->pdice = crt_ptr->pdice;
	
	if(F_ISSET(crt_ptr, MMALES))
		F_SET(ply_ptr, PMALES);
	else
		F_CLR(ply_ptr, PMALES);

	print (fd, "You possess %1m.\n", crt_ptr);

	return (0);
}

/************************************************************************/
/*								dm_attack								*/
/************************************************************************/
/* 	This function allows a DM to make a monster attack a given player 	*/

int dm_attack (ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
	creature	*atr_ptr, *atd_ptr;
	room		*rom_ptr;
	ctag		*pp, *cp, *prev;
	int			fd, inroom = 1;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	if(ply_ptr->class < CARETAKER)
		return(PROMPT);	

	if(cmnd->num < 3) {
		print (fd, "syntax: *attack <monster> <defender>\n");
		return(0);
	}

	atr_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[1],
		cmnd->val[1]);

	if(!atr_ptr) {
		print (fd, "Can't seem to locate that attacker here.\n");
		return (0);
	}

	if(F_ISSET(atr_ptr, MPERMT)) {
		print (fd, "Perms can't do that.\n");
		return(0);
	}

  	atd_ptr = find_crt(ply_ptr, rom_ptr->first_mon,cmnd->str[2], cmnd->val[2]);

	if(!atd_ptr) {
		lowercize(cmnd->str[2], 1);
		atd_ptr = find_who(cmnd->str[2]);
		inroom=0;
	}

	if(!atd_ptr) {
		print (fd, "Can't seem to locate that victim here.\n");
			print(fd, "Please use full names.\n");
			return (0);
	}

	if(F_ISSET(atd_ptr, MPERMT)) {
		print (fd, "Perms can't do that.\n");
		return(0);
	}

	print(fd, "Adding %s to attack list of %s.\n", atd_ptr, atr_ptr);
	add_enm_crt(atd_ptr->name, atr_ptr);
	attack_crt(atr_ptr, atd_ptr); 
	if(inroom) {
		broadcast_rom(atd_ptr->fd, atd_ptr->rom_num, "%M attacks %m.", 
			atr_ptr, atd_ptr);
		print (atd_ptr->fd, "%M attacked you!\n", atr_ptr); 
	}

	return(0);
}

/***************************************************************************/
/*								list_enm								   */ 
/***************************************************************************/
/*	This function lists the enemy list of a given monster.		   		   */

int list_enm(ply_ptr, cmnd)
creature 	*ply_ptr;
cmd     	*cmnd;

{
	etag		*first_enm;
	etag    	*ep;
	room		*rom_ptr;
	int			fd, n=0;
	creature	*crt_ptr;

	if (ply_ptr->class < CARETAKER)
		return(0);
	
	rom_ptr= ply_ptr->parent_rom;
	fd = ply_ptr->fd;

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, 
		cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		print(fd, "Not here.\n");
		return(0);
	}

	print(ply_ptr->fd,"Enemy list for %s:\n", crt_ptr->name);
    ep = crt_ptr->first_enm;

    while(ep) {
		n += 1;
		print (fd,"%s.\n", ep->enemy);
        ep = ep->next_tag;
    }

    if(!n)
		print(fd, "None.\n");

    return(0);
}

/**********************************************************************/
/*                              list_charm                            */
/**********************************************************************/
/*      This function allows a DM to see a given player's charm list   */
                
int list_charm (ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
                        
creature        *crt_ptr;
int             fd, cfd, n = 0;
ctag            *cp;
                                
	if(ply_ptr->class < DM)
		return(PROMPT);
                         
	if(cmnd->num < 2) {
		print(ply_ptr->fd, "See whose charm list?\n");
		return(PROMPT);
	}
                
	fd = ply_ptr->fd; 
	lowercize(cmnd->str[1], 1);
	crt_ptr = find_who(cmnd->str[1]);
	if(!crt_ptr) {
		print(ply_ptr->fd, "%s is not on.\n", cmnd->str[1]);
		return(0);
	}
        
	cfd = crt_ptr->fd;
	cp = Ply[cfd].extr->first_charm;
	print (fd, "Charm list for %s:\n", crt_ptr->name);

	while(cp) {
		n += 1;
		print(fd,"%s.\n", cp->crt->name); 
		cp = cp->next_tag;
	}

	if(!n)
		print(fd, "Nobody.\n");

	return(0);
}

/**********************************************************************/
/*                              dm_auth                               */
/**********************************************************************/
/*      This function allows a DM to authorize a character online.    */
        
int dm_auth(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
creature        *crt_ptr;
int				fd;
                
	if(ply_ptr->class < DM)
		return(PROMPT);
	
	fd =ply_ptr->fd;

	lowercize(cmnd->str[1], 1);
	crt_ptr = find_who(cmnd->str[1]);
	if(!crt_ptr) {
		print(ply_ptr->fd, "%s is not on.\n", cmnd->str[1]);
		return(0);
	}
	
	if(!F_ISSET(crt_ptr, PAUTHD)) {
		F_SET(crt_ptr, PAUTHD);
		print(fd, "%s given authorization.\n", crt_ptr);
		return(0);
	}
	else {
		F_CLR(crt_ptr, PAUTHD);
		F_CLR(crt_ptr, PSECOK);
		print(fd, "Authorization removed from %s.\n", crt_ptr);
		return(0);
	}
}

/**********************************************************************/
/*								dm_flash						      */
/**********************************************************************/
/*  This function allows a DM to output a string to an individual     */
/*  player's screen. 						      					  */

int dm_flash(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr = 0;
	int			spaces = 0, i, fd, j, len;

	fd = ply_ptr->fd;

	if(ply_ptr->class < DM)
		return(PROMPT);

	if(cmnd->num < 2) {
		print(fd, "DM flash to whom?\n");
		return 0;
	}

	cmnd->str[1][0] = up(cmnd->str[1][0]);
	for(i = 0; i < Tablesize; i++) {
		if(!Ply[i].ply) continue;
		if(Ply[i].ply->fd == -1) continue;
		if(!strncmp(Ply[i].ply->name, cmnd->str[1], strlen(cmnd->str[1])))
			crt_ptr = Ply[i].ply;
		if(!strcmp(Ply[i].ply->name, cmnd->str[1]))
			break;
	}

	if(!crt_ptr) {
		print(fd, "Send to whom?\n");
		return(0);
	}

	len = strlen(cmnd->fullstr);
	for(j = 0; j < len && j < 256; j++) {
		if(cmnd->fullstr[j] == ' ' && cmnd->fullstr[j + 1] != ' ')
			spaces++;
		if(spaces == 2) break;
	}
	cmnd->fullstr[255] = 0;
                
	if(spaces < 2 || strlen(&cmnd->fullstr[j+1]) < 1) {
		print(fd, "Send what?\n");
		return(0);
	}

	if(F_ISSET(ply_ptr, PLECHO)) {
        ANSI(fd, CYAN);
        print(fd, "You flashed: \"%s\" to %M.\n", &cmnd->fullstr[j+1], 
			crt_ptr);
        ANSI(fd, BOLD)
        ANSI(fd, WHITE)
	}
	else
		print(fd, "Message flashed to %s.\n", crt_ptr->name);

	print(crt_ptr->fd, "%s.\n", &cmnd->fullstr[j+1]);

	return(0);
}
