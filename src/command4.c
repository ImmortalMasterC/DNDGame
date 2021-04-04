/*
 * COMMAND4.C:
 *
 *	Additional user routines
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

/***********************************************************************/
/*								health/score					       */
/***********************************************************************/
/* This function shows a player his current hit points, magic points,  */
/* experience, gold, luck and level and spells under.		      	   */

char		str[2048];
int 		health(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	int			fd, l = 1, agility;
	extra		*extr;
	long		i, t;

	fd = ply_ptr->fd;
	
	update_ply(ply_ptr);

	if(F_ISSET(ply_ptr, PBLIND)) {
		CLS(fd);
		ANSI(fd, RED);
		print(fd, "You're obviously blind!\n");
		ANSI(fd, BLUEBG);
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		if(ply_ptr->class < CARETAKER)
			return(0);
	}

	ANSI(fd, BLUEBG);
	ANSI(fd, BLUE);
	print(fd, "\n\n\n___________________________________________"
              "_____________________________\n");
	ANSI(fd, YELLOW);
	print(fd, "%s the %s (level %d),", ply_ptr->name, 
		title_ply(ply_ptr), ply_ptr->level);
	print(fd,"  %-3d years old.", 
		18 + ply_ptr->lasttime[LT_HOURS].interval/86400L);

	if(F_ISSET(ply_ptr, PDETLK)) {
		print(fd, "   Luck: "); 
		if(Ply[ply_ptr->fd].extr->luck < 35) {
			ANSI(fd, RED);
			print(fd, "Very bad.");
		}
		else if(Ply[ply_ptr->fd].extr->luck >= 35 && 
			Ply[ply_ptr->fd].extr->luck < 45) {
				ANSI(fd, RED);
				print(fd, "Bad.");
		}
		else if(Ply[ply_ptr->fd].extr->luck >= 45 && 
			Ply[ply_ptr->fd].extr->luck < 55) {
				ANSI(fd, CYAN);
				print(fd, "Average.");
		}
		else if(Ply[ply_ptr->fd].extr->luck >= 55 && 
			Ply[ply_ptr->fd].extr->luck < 65) {
				ANSI(fd, CYAN);
				print(fd, "Good.");
		}
		else if(Ply[ply_ptr->fd].extr->luck >= 65) {
			ANSI(fd, YELLOW);
			print(fd, "Very good.");
		}
	}

	print(fd, "\n");
	if(F_ISSET(ply_ptr, PHIDDN)) {
		ANSI(fd, CYAN);
		print(fd, " *Hidden* ");
	}
	if(F_ISSET(ply_ptr, PCHARM)) {
		ANSI(fd, CYAN);
		print(fd, " *Charmed* ");
	}
	if(F_ISSET(ply_ptr, PPOISN)) {
		ANSI(fd, RED);	
		print(fd, " *Poisoned* ");
	}
	if(F_ISSET(ply_ptr, PDISEA)) {
		ANSI(fd, RED);	
		print(fd," *Diseased* ");
	}
	if(F_ISSET(ply_ptr, PEVEYE)) {
		ANSI(fd, RED);	
		print(fd," *Evil Eye* ");
	}
	if(F_ISSET(ply_ptr, PFEARS)) {
		ANSI(fd, RED);
		print(fd, " *Fearful* ");
	}
	if(F_ISSET(ply_ptr, PSILNC)) {
		ANSI(fd, RED);
		print(fd, " *Mute* ");
	}
	if(F_ISSET(ply_ptr, PFROZE)) {
		ANSI(fd, RED);
		print(fd, " *Frozen* ");
	}
	if(F_ISSET(ply_ptr, PHEXED)) {
		ANSI(fd, RED);
		print(fd, " *Hexed* ");
	}
	if(F_ISSET(ply_ptr, PPREPA)) {
		ANSI(fd, RED);
		print(fd, " *Prepared* ");
	}

	ANSI(fd, GREEN);
	print(fd, "\n  Experience:  ");
	ANSI(fd, CYAN);
	print(fd, "%-7ld  -  ", ply_ptr->experience);
	ANSI(fd, GREEN);
	print(fd, " Gold Pieces:  ");
	ANSI(fd, CYAN);
	print(fd, "%-7ld\n",  ply_ptr->gold);
	ANSI(fd, GREEN);
	print(fd, "   Alignment:  ");
	ANSI(fd, CYAN);
	if(ply_ptr->alignment > 100)
		print(fd, "Good (blue aura)\n");
	if(ply_ptr->alignment <= 100 && ply_ptr->alignment >= -100) {
		ANSI(fd, NORMAL);
		ANSI(fd, BLUEBG);
		ANSI(fd, WHITE);
		print(fd, "Neutral (gray aura)\n");
		ANSI(fd, BOLD);
	}
	if(ply_ptr->alignment < -100) {
		ANSI(fd, RED);
		print(fd, "Evil (red aura)\n");
	}
	ANSI(fd, GREEN);
	print(fd, " Wimpy Value:  ");
	ANSI(fd, CYAN);
	print(fd, "%-3d", ply_ptr->WIMPYVALUE);
	if(F_ISSET(ply_ptr, PWIMPY))
		print(fd, "\n");
	else {
		ANSI(fd, RED);
		print(fd, " (not set)\n");
	}
	ANSI(fd, GREEN);
	print(fd, " Armor Class:  ");
	ANSI(fd, CYAN);
	print(fd, "%-3d      -  ", ply_ptr->armor/10);
	ANSI(fd, GREEN);
	print(fd, " HP/MP:  ");
	ANSI(fd, CYAN);
	print(fd, "%3d/%-3d %3d/%-3d\n", 
		ply_ptr->hpcur, ply_ptr->hpmax, ply_ptr->mpcur, ply_ptr->mpmax);

	ANSI(fd, GREEN);
	print(fd, "        Home:  ");
	ANSI(fd, CYAN);
	print(fd, "%s", Ply[fd].extr->home_rom->name);
	if(ply_ptr->class < CARETAKER)
		print(fd, "\n");
	else
		print(fd, " Room No %d\n", Ply[fd].extr->home_rom->rom_num);

	if(ply_ptr->class == RANGER || ply_ptr->class >= CARETAKER) {
		ANSI(fd, GREEN);
		print(fd, "       Haste:  ");
		ANSI(fd, CYAN);
		t = time(0);
		i = ply_ptr->lasttime[LT_HASTE].ltime;
		if(((600L - t + i)/60L <= 0) && ((600L -t + i)%60L) <= 0) 
			print(fd, "Ready\n");
		else
			print(fd, "Wait %d:%02d minutes\n", 
				(600L - t + i)/60L, (600L - t + i)%60L); 
	}

	if((ply_ptr->class == CLERIC) || (ply_ptr->class == PALADIN) ||
		(ply_ptr->class >= CARETAKER)) {
		ANSI(fd, GREEN);
		print(fd, "        Pray:  ");
		ANSI(fd, CYAN);
		print(fd, "%s\n", (F_ISSET(ply_ptr, PPRAYD)) ? "Yes":"No");
	}

	if((ply_ptr->class == BARBARIAN) || (ply_ptr->class >= CARETAKER)) {
		ANSI(fd, GREEN);
		print(fd, "     Berserk:  ");
		ANSI(fd, CYAN);
		print(fd, "%s\n", (F_ISSET(ply_ptr, PBERSK)) ? "Yes":"No");
	}

	ANSI(fd, GREEN);
	print(fd, "     Agility:  ");
	ANSI(fd, CYAN);
	agility = compute_agility(ply_ptr);
	if(agility > 30)
		strcpy(str, "Excellent");
	else if(agility > 25)
		strcpy(str, "Good");
	else if(agility >= 20)
		strcpy(str, "Normal");
	else
		strcpy(str, "Poor");
	print(fd, "%s\n", str);

	strcpy(str, "  ");
	if(F_ISSET(ply_ptr, PBLESS)) strcat(str, "bless, ");
	if(F_ISSET(ply_ptr, PBRWAT)) strcat(str, "breathe-water, ");
	if(F_ISSET(ply_ptr, PDINVI)) strcat(str, "detect-invisible, ");
	if(F_ISSET(ply_ptr, PDMAGI)) strcat(str, "detect-magic, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PSSHLD)) strcat(str, "earth-shield, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PEVEYE)) strcat(str, "evil-eye, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n              ");
		l++;
	}
	if(F_ISSET(ply_ptr, PFLYSP)) strcat(str, "fly, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PINVIS)) strcat(str, "invisibility, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PKNOWA)) strcat(str, "know-aura, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PLIGHT)) strcat(str, "light, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PLEVIT)) strcat(str, "levitation, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PPROTE)) strcat(str, "protection, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PRFIRE)) strcat(str, "resist-fire, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PRMAGI)) strcat(str, "resist-magic, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PRCOLD)) strcat(str, "resist-cold, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PRACID)) strcat(str, "resist-acid, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PSHRNK)) strcat(str, "shrink, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PRFLCT)) strcat(str, "spell-reflect, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PFROZE)) strcat(str, "freeze, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PHEXED)) strcat(str, "hex, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PSTRNG)) strcat(str, "super-strength, ");
	if(strlen(str) <= 2)
		strcat(str, "None.");
	else {
		str[strlen(str) - 2] = '.';
		str[strlen(str) - 1] = 0;
    }

	ANSI(fd, GREEN);
	print(fd, "Spells under:");
	ANSI(fd, YELLOW);
	print(fd, "%s\n", str);


	ANSI(fd, BLUE);
	print(fd, "--------------------------------------------"
              "-----------------------------\n");
	ANSI(fd, BOLD);
	ANSI(fd, WHITE);
   
    print(fd, "Daily broadcasts:  %d/%d\n",
		ply_ptr->daily[DL_BROAD].cur, ply_ptr->daily[DL_BROAD].max);

return(0);

}

/****************************************************************************/
/*								help								   	    */
/****************************************************************************/
/* This function allows a player to get help in general, or help for a 		*/
/* specific command.  If help is typed by itself, a list of commands   		*/
/* is produced.  Otherwise, help is supplied for the command specified 		*/

int help(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	char 	file[80];
	int		fd, c = 0, match = 0, num = 0;
	
	fd = ply_ptr->fd;
	strcpy(file, DOCPATH);

	if(cmnd->num < 2) {
		strcat(file, "/helpfile");
		view_file(fd, 1, file);
		return(DOPROMPT);
	}
	if(!strcmp(cmnd->str[1], "spells")) {
		strcat(file, "/spellfile");
		view_file(fd, 1, file);
		return(DOPROMPT);
	}

	if(!strcmp(cmnd->str[1], "policy")) {
		strcat(file, "/policy");
		view_file(fd, 1, file);
		return(DOPROMPT);
	}

	if(!strcmp(cmnd->str[1], "welcome")) {
		strcat(file, "/welcome");
		view_file(fd, 1, file);
		return(DOPROMPT);
	}

	if(!strcmp(cmnd->str[1], "smile") || !strcmp(cmnd->str[1], "smiles")) {
		strcat(file, "/smiles");
		view_file(fd, 1, file);
		return(DOPROMPT);
	}

	do {
		if(!strcmp(cmnd->str[1], cmdlist[c].cmdstr)) {
			match = 1;
			num = c;
			break;
		}
		else if(!strncmp(cmnd->str[1], cmdlist[c].cmdstr, 
			strlen(cmnd->str[1]))) {
			match++;
			num = c;
		}
		c++;
	} while(cmdlist[c].cmdno);

	if(num > 300 && ply_ptr->class < CARETAKER) {
                print(fd, "Command not found.\n");
                return(0);
        }

	if(match == 1) {
		sprintf(file, "%s/help.%d", DOCPATH, cmdlist[num].cmdno);
		view_file(fd, 1, file);
		return(DOPROMPT);
	}
	else if(match > 1) {
		print(fd, "Command is not unique.\n");
		return(0);
	}

	c = num = 0;
	do {
		if(!strcmp(cmnd->str[1], spllist[c].splstr)) {
			match = 1;
			num = c;
			break;
		}
		else if(!strncmp(cmnd->str[1], spllist[c].splstr, 
			strlen(cmnd->str[1]))) {
			match++;
			num = c;
		}
		c++;
	} while(spllist[c].splno != -1);

	if(match == 0) {
		print(fd, "That command does not exist.\n");
		return(0);
	}
	else if(match > 1) {
		print(fd, "Sorry, that spell name is not unique.  Try more letters.\n");
		return(0);
	}

	sprintf(file, "%s/spell.%d", DOCPATH, spllist[num].splno);
	view_file(fd, 1, file);
	return(DOPROMPT);
}

/**********************************************************************/
/*								welcome							      */
/**********************************************************************/
/* Outputs welcome file to user, giving him/her info on how to play   */
/* the game 							      						  */

int welcome(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	char	file[80];
	int		fd;

	fd = ply_ptr->fd;

	sprintf(file, "%s/welcome", DOCPATH);

	view_file(fd, 1, file);
	return(DOPROMPT);
}

/**********************************************************************/
/*								info							      */
/**********************************************************************/
/* This function displays a player's entire list of information, 	  */
/* including player stats, proficiencies, level, class and spells.	  */

int info(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	char	supspell[14];
	char	alstr[8];
	int 	fd, cnt;
	long	expneeded, lv;

	fd = ply_ptr->fd;

	update_ply(ply_ptr);

	if(ply_ptr->level < MAXALVL)
		expneeded = needed_exp[ply_ptr->level - 1];
	else
		expneeded = (long)((needed_exp[MAXALVL - 1] * ply_ptr->level));   

	if(F_ISSET(ply_ptr, PSHRNK))
		strcpy(supspell, "(Shrink)");
	else strcpy(supspell, "");

	if(F_ISSET(ply_ptr, PSTRNG))
		strcpy(supspell, "(Super str)");
	else strcpy(supspell, "");

	if(ply_ptr->alignment < -100)
		strcpy(alstr, "Evil");
	else if(ply_ptr->alignment < 101)
		strcpy(alstr, "Neutral");
	else
		strcpy(alstr, "Good");

	for(lv = 0, cnt = 0; lv < MAXWEAR; lv++)
		if(ply_ptr->ready[lv]) cnt++;
	cnt += count_inv(ply_ptr, 0);

	CLS(fd);
	ANSI(fd, BLUEBG);
	ANSI(fd, YELLOW);
	print(fd, "%s the %s %s (level %d),", ply_ptr->name, 
		F_ISSET(ply_ptr, PCHAOS) ? "chaotic" : "lawful",
			title_ply(ply_ptr), ply_ptr->level);

	print(fd," %-3d years old.\n", 
		18 + ply_ptr->lasttime[LT_HOURS].interval/86400L);

	ANSI(fd, GREEN);
	print(fd, "Desc:  ");
	ANSI(fd, WHITE);
	print(fd, "%s.\n", ply_ptr->description); 

	ANSI(fd, GREEN);
	print(fd, "Race: ");
	ANSI(fd, BOLD);
	ANSI(fd, WHITE);
	print(fd, "%s    ", race_str[ply_ptr->race]);

	ANSI(fd, GREEN);
	print(fd, "Class: ");
	ANSI(fd, WHITE);
	print(fd, "%-14s\n\n", class_str[ply_ptr->class]);

	ANSI(fd, BLUE);
	print(fd, "---------------------------------------------"
              "---------------------------\n");
	ANSI(fd, GREEN);
	print(fd, "Strength   Dexterity    Constitution    "
              "Intelligence     Piety\n");
	ANSI(fd, CYAN);
	print(fd, "   %2d         %2d	    %2d		    %2d		   %2d\n\n", 
		ply_ptr->strength, 
		ply_ptr->dexterity, 
		ply_ptr->constitution,
		ply_ptr->intelligence, 
		ply_ptr->piety);

	ANSI(fd, GREEN);
	print(fd, "  Hit Points:  ");
	ANSI(fd, CYAN);
	print(fd, "%4d/%-4d", ply_ptr->hpcur, ply_ptr->hpmax);

	ANSI(fd, GREEN);
	print(fd, "\t\t     Wimpy Value:  ");
	ANSI(fd, CYAN);
	print(fd, "%-3d", ply_ptr->WIMPYVALUE);
	if(ply_ptr->WIMPYVALUE>0)
		print(fd, "\n");
	else {
		ANSI(fd, RED);
		print(fd, " (not set)\n");
	}
	ANSI(fd, GREEN);
	print(fd, "Magic Points:  ");
	ANSI(fd, CYAN);
	print(fd,"%4d/%-4d", ply_ptr->mpcur, ply_ptr->mpmax);

	ANSI(fd, GREEN);
	print(fd, "\t\t       Inventory:  " );
	ANSI(fd, CYAN);
	if(cnt == 0)
		print(fd, "Nothing.\n");
	if(cnt == 1) { 
	   	if(weight_ply(ply_ptr) == 1)
			print(fd, "%d item, %d lb.\n", cnt, weight_ply(ply_ptr));
		else
			print(fd, "%d item, %d lbs.\n", cnt, weight_ply(ply_ptr));
	}
	if(cnt > 1) 
	print(fd, "%d items, %d lbs.\n", cnt, weight_ply(ply_ptr));

	ANSI(fd, GREEN);
	print(fd, " Armor Class:  ");
	ANSI(fd, CYAN);
	print(fd, "%-4d", ply_ptr->armor/10);

	ANSI(fd, GREEN);
	print(fd, "\t\t                    Gold:  ");
	ANSI(fd, YELLOW);
	print(fd, "%-7d\n", ply_ptr->gold);

	ANSI(fd, GREEN);
	print(fd, "  Experience:  ");
	ANSI(fd, WHITE);
	print(fd, "%lu ", ply_ptr->experience);
	ANSI(fd, CYAN);
	if((expneeded - ply_ptr->experience) > 1)
		print(fd, " (%lu more needed)", expneeded - ply_ptr->experience);
	else
		print(fd, " (no more needed)");

	ANSI(fd, GREEN);
	if(F_ISSET(ply_ptr, PDETLK)) {
		print(fd, "\t\tYour luck is:  "); 
			ANSI(fd, RED);
			if(Ply[ply_ptr->fd].extr->luck < 35)
				print(fd, "Very bad.");
			if(Ply[ply_ptr->fd].extr->luck > 35 && 
				Ply[ply_ptr->fd].extr->luck < 45)
					print(fd, "Bad.");
			ANSI(fd, CYAN);
			if(Ply[ply_ptr->fd].extr->luck > 45 && 
				Ply[ply_ptr->fd].extr->luck < 55)
					print(fd, "Average.");
			if(Ply[ply_ptr->fd].extr->luck > 55 && 
				Ply[ply_ptr->fd].extr->luck < 65)
					print(fd, "Good.");
			ANSI(fd, YELLOW);
			if(Ply[ply_ptr->fd].extr->luck > 65)
				print(fd, "Very good.");
		print(fd, "\n"); 
	}
	else
		print(fd, "\n"); 

	if(F_ISSET(ply_ptr, PPLDGK)) {
		ANSI(fd, GREEN);
		print(fd, "  Pledged to:");
		ANSI(fd, CYAN);
		print(fd, "  %s\n", F_ISSET(ply_ptr, PKNGDM) ? "Rutger":"Ragnar");
	}
	ANSI(fd, BLUE);
	print(fd, "-------------------------------------------"
              "-----------------------------\n");
	ANSI(fd, WHITE);
	print(fd, "Weapon Proficiencies:\n");
	ANSI(fd, GREEN);
	print(fd, "Sharp        Thrust        Blunt        Pole        Missile\n");
	ANSI(fd, CYAN);
	print(fd, "  %2d%%\t        %2d%%\t     %2d%%\t %2d%%\t        %2d%%\n",
		profic(ply_ptr, SHARP), 
		profic(ply_ptr, THRUST),
		profic(ply_ptr, BLUNT),
		profic(ply_ptr, POLE), 
		profic(ply_ptr, MISSILE));

	ANSI(fd, WHITE);
	print(fd, "Magical Realms:\n");
	ANSI(fd, GREEN);
	print(fd, "Earth          Wind         Fire       Water\n");
	ANSI(fd, CYAN);
	print(fd, "  %2d%%\t        %2d%%\t     %2d%%\t %2d%%\n\n",
		mprofic(ply_ptr, EARTH), 
		mprofic(ply_ptr, WIND),
		mprofic(ply_ptr, FIRE), 
		mprofic(ply_ptr, WATER));

	ANSI(fd, BLUE);
	print(fd, "-----------------------------------------"
              "-------------------------------\n");
	ANSI(fd, WHITE);
	F_SET(ply_ptr, PREADI);
	print(fd, "[Q to Quit, else Return key]: ");
	output_buf();
	Ply[fd].io->intrpt &= ~1;
	Ply[fd].io->fn = info_2;
	Ply[fd].io->fnparam = 1;
	return(DOPROMPT);
}

/************************************************************************/
/*								info_2									*/
/************************************************************************/
/* This function is the second half of info which outputs spells		*/

void info_2(fd, param, instr)
int 	fd, param;
char 	*instr;

{
	char		str[2048];
	char		spl[128][20];
	int			i, j, l = 1;
	creature	*ply_ptr;

	ply_ptr = Ply[fd].ply;

	if(instr[0]) {
		print(fd, "Aborted.\n");
		F_CLR(ply_ptr, PREADI);
		RETURN(fd, command, 1);
	}

	strcpy(str, "\n  ");
	for(i = 0, j = 0; i < 128; i++)
		if(S_ISSET(ply_ptr, i))
			strcpy(spl[j++], spllist[i].splstr);

	if(!j)
		strcat(str, "None.");
	else {
		qsort((void *)spl, j, 20, strcmp);
		for(i = 0; i < j; i++) {
			strcat(str, spl[i]);
			strcat(str, ", ");
		}
		str[strlen(str) - 2] = '.';
		str[strlen(str) - 1] = 0;
	}
	ANSI(fd, BLUEBG);
	ANSI(fd, GREEN);
	print(fd, "Spells known:");
	ANSI(fd, BOLD);
	ANSI(fd, WHITE);
	print(fd, "%s\n", str);


	strcpy(str, "  ");
	if(F_ISSET(ply_ptr, PBLESS)) strcat(str, "bless, ");
	if(F_ISSET(ply_ptr, PBRWAT)) strcat(str, "breathe-water, ");
	if(F_ISSET(ply_ptr, PDINVI)) strcat(str, "detect-invisible, ");
	if(F_ISSET(ply_ptr, PDMAGI)) strcat(str, "detect-magic, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PSSHLD)) strcat(str, "earth-shield, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PEVEYE)) strcat(str, "evil-eye, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n              ");
		l++;
	}
	if(F_ISSET(ply_ptr, PFLYSP)) strcat(str, "fly, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PFROZE)) strcat(str, "freeze, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PHEXED)) strcat(str, "hex, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PINVIS)) strcat(str, "invisibility, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PKNOWA)) strcat(str, "know-aura, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PLIGHT)) strcat(str, "light, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PLEVIT)) strcat(str, "levitation, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PPROTE)) strcat(str, "protection, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PRFIRE)) strcat(str, "resist-fire, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PRMAGI)) strcat(str, "resist-magic, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PRCOLD)) strcat(str, "resist-cold, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PRACID)) strcat(str, "resist-acid, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PSHRNK)) strcat(str, "shrink, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PRFLCT)) strcat(str, "spell-reflect, ");
	if(strlen(str)/(l*57) == 1) {
		strcat(str, "\n               ");
		l++;
	}
	if(F_ISSET(ply_ptr, PSTRNG)) strcat(str, "super-strength, ");
	if(strlen(str) <= 2)
		strcat(str, "None.");
	else {
		str[strlen(str) - 2] = '.';
		str[strlen(str) - 1] = 0;
	}
	ANSI(fd, BLUEBG);
	ANSI(fd, GREEN);
	print(fd, "Spells under:");
	ANSI(fd, YELLOW);
	print(fd, "%s\n\n", str);
	ANSI(fd, BOLD);
	ANSI(fd, WHITE);

	F_CLR(Ply[fd].ply, PREADI);
	RETURN(fd, command, 1);
}

/************************************************************************/
/*								peek							        */
/************************************************************************/
/* This function allows a thief or assassin to peek at the inventory    */
/* of another player.  If successful they will be able to see it, and   */
/* another roll is made to see if they get caught.						*/

int peek(ply_ptr, cmnd)
creature	*ply_ptr;
cmd			*cmnd;

{
	creature	*crt_ptr;
	room		*rom_ptr;
	char		str[2048];
	long		i, t;
	int			fd, n, chance;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;
	str[0] = 0;

	if(cmnd->num < 2) {
		print(fd, "Peek at whom?\n");
		return(0);
	}

	if((ply_ptr->class != THIEF && ply_ptr->class != ASSASSIN) && 
		ply_ptr->class < CARETAKER) {
		print(fd, "Only thieves and assassins know how to peek.\n");
		return(0);
	}
	
	if(F_ISSET(ply_ptr, PBLIND)) {
		ANSI(fd, RED);
		print(fd, "You can't do that...You're blind!\n");
		ANSI(fd, BOLD);
		ANSI(fd, WHITE);
		return(0);
	}

	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
		cmnd->str[1], cmnd->val[1]);

	if(!crt_ptr) {
		lowercize(cmnd->str[1], 1);
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			cmnd->str[1], cmnd->val[1]);

		if(!crt_ptr) {
			print(fd, "That person is not here.\n");
			return(0);
		}
	}

	i = LT(ply_ptr, LT_PEEKS);
	t = time(0);

	if(i > t  && ply_ptr->class < CARETAKER) {
		please_wait(fd, i - t);
		return(0);
	}

	ply_ptr->lasttime[LT_PEEKS].ltime = t;
	ply_ptr->lasttime[LT_PEEKS].interval = 5;

	if((F_ISSET(crt_ptr, MUNSTL) || F_ISSET(crt_ptr, MTRADE) || 
		F_ISSET(crt_ptr, MPURIT)) && ply_ptr->class < CARETAKER) {        
			print(fd, "That person is watching you.\n");  
			return(0);		    
	} 
		
	chance = (25 + ply_ptr->level * 10) - (crt_ptr->level * 5);
	if(chance < 0) 
		chance = 0;
	if(ply_ptr->class >= CARETAKER) 
		chance = 100;
	if(mrand(1, 100) > chance) {
		print(fd, "You failed.\n");
		return(0);
	}

	chance = MIN(90, 15 + ply_ptr->level * 5);

	if(mrand(1, 100) > chance && ply_ptr->class < CARETAKER) {
		print(crt_ptr->fd, "%s peeked at your inventory.\n", ply_ptr);
		broadcast_rom2(fd, crt_ptr->fd, ply_ptr->rom_num, 
	       "%M peeked at %m's inventory.", ply_ptr, crt_ptr);
	}

	if (!F_ISSET(crt_ptr, MNOGEN)) {
		sprintf(str, "%s is carrying: ", F_ISSET(crt_ptr, PMALES) ? "He":"She");
		n = strlen(str);
		if(list_obj(&str[n], ply_ptr, crt_ptr->first_obj) > 0)
			print(fd, "%s\n", str);
		else
			print(fd, "%s isn't holding anything.\n",
				F_ISSET(crt_ptr, PMALES) ? "He":"She");

		return(0);
	}
	else
		print(fd, "It isn't holding anything.\n");
}


/************************************************************************/
/*      			  		      who  	 		               			*/
/************************************************************************/
/* This function outputs a list of all the players who are				*/ 
/* currently logged into the game. 										*/

int who(ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;
{
    char    str[15];
    int 	fd, i, j;

    fd = ply_ptr->fd;

	 if(F_ISSET(ply_ptr, PBLIND)) {
                ANSI(fd, RED);
                print(fd, "You're blind!\n");
                ANSI(fd, BOLD);
                ANSI(fd, WHITE);
                return(0);
        }

    ANSI (fd, YELLOW);
    print(fd, "\n %-20s %-16s %-14s %-14s %-7s", 
		"Player", "Title", "Class", "Race", "Status");
    ANSI(fd, BOLD);
    ANSI(fd, WHITE);
    print(fd, "\n---------------------------------------------------------------------------\n");
    for(i = 0; i < Tablesize; i++) {
        if(!Ply[i].ply) continue;
        if(Ply[i].ply->fd == -1) continue;
        if(!strcmp(Ply[i].ply->name, DMNAME)) continue;
        if(F_ISSET(Ply[i].ply, PDMINV) && Ply[i].ply->class == DM &&
        	ply_ptr->class < DM) continue;
        if(F_ISSET(Ply[i].ply, PINVIS) && !F_ISSET(ply_ptr, PDINVI) &&
        	ply_ptr->class < CARETAKER) continue;
        if(F_ISSET(Ply[i].ply, PDMINV) && ply_ptr->class < DM) continue;
        print(fd, "%s", (F_ISSET(Ply[i].ply, PDMINV)) ? "#":""); 
        print(fd, "%s%-20s ", F_ISSET(Ply[i].ply, PINVIS) ? "*":" ", 
			Ply[i].ply->name); 
		print(fd, "%-16s ", title_ply(Ply[i].ply));
		print(fd, "%-14s ", class_str[Ply[i].ply->class]);  
        print(fd, "%-15s", race_str[Ply[i].ply->race]);
		if(F_ISSET(Ply[i].ply, PASSHL))
        	print(fd, "%-8s\n", "Flag 8");
		else
        	print(fd, "Normal\n");
    }
    print(fd, "\n");  

    return(0);
}
 
 
/**********************************************************************/
/*                              whois                                 */
/**********************************************************************/
/* The whois function displays a selected player's name, class, level */
/* title, age, alignment and gender 								  */
      
int whois(ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;
{
    creature    *crt_ptr;
    int     	fd;
 
    fd = ply_ptr->fd;
 
    if(cmnd->num < 2) {
        print(fd, "Whois whom?\n");
        return(0);
    }
 
    lowercize(cmnd->str[1], 1);
    crt_ptr = find_who(cmnd->str[1]);
 
	if(ply_ptr->class < CARETAKER) {
    	if(!crt_ptr || F_ISSET(crt_ptr, PDMINV) || F_ISSET(ply_ptr, PBLIND) ||
       		(F_ISSET(crt_ptr, PINVIS) && !F_ISSET(ply_ptr, PDINVI))) {
        		print(fd, "That person cannot be found.\n");
        		return(0);
    	}
    }
	else if(!crt_ptr || F_ISSET(crt_ptr, PDMINV)) {
        print(fd, "That person cannot be found.\n");
        return(0);
    }
 
	ANSI(fd, YELLOW); 
        print(fd, "Player");
		print(fd, "         Class");
		print(fd, " Title");
		print(fd, "            Pledge");
		print(fd, "   Align");
		print(fd, "    Race");
		print(fd, "      Gen");
		print(fd, " Age");
		print(fd, " Level\n"); 

 	ANSI(fd, BOLD);
	ANSI(fd, WHITE);
        print(fd, "-------------------------------------------------------------------------------\n");

        print(fd, "%-15s", crt_ptr->name);
		if(crt_ptr->class < CARETAKER)
			print(fd, "%-6.5s", class_str[crt_ptr->class]);
		if(crt_ptr->class == CARETAKER)
			print(fd, "CT    ");
		if(crt_ptr->class == DM)
			print(fd, "DM    ");
		print(fd, "%-15s", title_ply(crt_ptr)); 
		if(F_ISSET(crt_ptr, PPLDGK))
			print(fd, "  %-7s", F_ISSET(crt_ptr, PKNGDM) ? "Rutger":"Ragnar");
		else
			print(fd, "  %-7s", "Neutral");
		print(fd, "  %-9s", F_ISSET(crt_ptr, PCHAOS) ? "Chaotic":"Lawful");  
		print(fd, "%-11s", race_str[crt_ptr->race]);
		print(fd, "%-3s", F_ISSET(crt_ptr, PMALES) ? "M ":"F ");  

		print(fd,"%-4d",
        	18 + crt_ptr->lasttime[LT_HOURS].interval/86400L);

       	print(fd, "%3d", crt_ptr->level);
		print(fd, "\n");
	
	return(0);
}


/************************************************************************/
/*								vote									*/
/************************************************************************/
/*	This file allows a player to vote on issues 						*/

int vote (ply_ptr, cmnd)
creature    *ply_ptr;
cmd     	*cmnd;   

{
	int		fd;
	int 	i, n, number;
	int		match = 0;
	char	str[80], tmp[256];
	FILE	*fp;
extern void vote_cmnd();
	
    fd = ply_ptr->fd;
	str[0] = 0; 
	if((18 + ply_ptr->lasttime[LT_HOURS].interval/86400L) < 19) {
		print(fd, "You're too young to vote in this election.\n");
		print(fd, "You must be at least 19 years old.\n");
		return(0);
	}
	if(!F_ISSET(ply_ptr->parent_rom, RELECT)) {
		print(fd, "This is not an election booth.\n");
		return(0);
	}

	sprintf(Ply[fd].extr->tempstr[0],"%s/ISSUE",POSTPATH);
	fp = fopen(Ply[fd].extr->tempstr[0], "r");
	if(!fp) {
		print(fd, "There are currently no issues for you to vote on.\n");
		return(0);
	}
	
	fgets(tmp, 256, fp);
	if(feof(fp)) {
		print(fd, "Currently there are no issues for you to vote on.\n");
		return(0);
	}

	fclose(fp);

	n =  sscanf(tmp, "%d %s", &number, str);

	lowercize(str, 1);
	if(n < 1) {
		print(fd, "There are no issues for you to vote on at this time.\n");
		return(0);
	} 
	else if(n == 2 && str) {
		if(!strcmp(str, "Prince0") && F_ISSET(ply_ptr, PPLDGK) && 
			!F_ISSET(ply_ptr, PKNGDM))
				match = 1;
		else if(!strcmp(str, "Prince1") && F_ISSET(ply_ptr, PPLDGK)
			&& F_ISSET(ply_ptr, PKNGDM))
				match = 1;
		else
			for(i = 1; i < CARETAKER; i++)
				if(!strcmp(str, class_str[i])) {
					if(i == ply_ptr->class)
						match = 1;
					break;
				}

		if(!match) {
			print(fd, "Sorry, you may not vote in this election.\n");
			return(0);
		}
	}

	Ply[fd].extr->tempstr[1][0] = MIN(79, number);
	Ply[fd].extr->tempstr[1][1] = 0;
	
	
 	F_SET(Ply[fd].ply, PREADI); 
    vote_cmnd(fd, 0, "");

    return(DOPROMPT);
}

/************************************************************************/
/*						vote-cmnd										*/
/************************************************************************/

void vote_cmnd(fd, param, str)
int     fd;
int     param;
char 	*str;

{
    creature	*ply_ptr;
		int		i, n = 0;
		char	tmp[1024], c;
		FILE	*fp;
	
	ply_ptr = Ply[fd].ply;
	output_buf();

	switch (param) {
		case 0:
			sprintf(tmp, "%s/vote/%s_v", PLAYERPATH, ply_ptr->name);
			fp = fopen(tmp, "r");
			if(!fp) {
				fp = fopen(Ply[fd].extr->tempstr[0], "r");
				if(!fp) {
					F_CLR(Ply[fd].ply, PREADI);
					RETURN(fd, command, 1);
				}
				fgets(tmp, 1024, fp);
				fgets(tmp, 1024, fp);
				tmp[1023] = 0;
				fclose(fp);
				print(fd, "\n%s", tmp);
				print(fd, "How do you cast your vote? ");
				output_buf();

				Ply[fd].extr->tempstr[1][1] = 1;
            	Ply[fd].io->intrpt &= ~1; 
				RETURN(fd, vote_cmnd, 2);
			}
			else {
				fclose(fp);
				print(fd, "You have already voted.\n");
				print(fd, "Do you wish to change your vote?  (y/n): ");
				output_buf();
            	Ply[fd].io->intrpt &= ~1; 
				RETURN(fd, vote_cmnd, 1);
			}
        	break;

        case 1:
			if(str[0] == 'y' || str[0] == 'Y') {
				sprintf(tmp, "%s/vote/%s_v", PLAYERPATH, ply_ptr->name);
				unlink(tmp);
				fp = fopen(Ply[fd].extr->tempstr[0], "r");
				if(!fp) {
					F_CLR(Ply[fd].ply, PREADI);
					RETURN(fd, command, 1);
				}
				fgets(tmp, 1024, fp);
				fgets(tmp, 1024, fp);
				tmp[1023] = 0;
				fclose(fp);
				print(fd, "\n%s", tmp);
				print(fd, "How do you cast your vote? ");
				output_buf();

				Ply[fd].extr->tempstr[1][1] = 1;
            	Ply[fd].io->intrpt &= ~1; 
				RETURN(fd,vote_cmnd, 2);
			}
            print(fd, "Aborting.\n");
            F_CLR(Ply[fd].ply, PREADI);
            RETURN(fd, command, 1);
			break;

		case 2:
			c = low(str[0]);
			if(c != 'a' && c != 'b' && c != 'c' && c != 'd' && 
				c != 'e' && c != 'f' && c != 'g') {
					print(fd,"Invalid selection.  Aborting.\n");
					F_CLR(Ply[fd].ply, PREADI);
					RETURN(fd, command, 1);
			}
			n = MAX(0,Ply[fd].extr->tempstr[1][1] -1);
			Ply[fd].extr->tempstr[2][n] = up(c);
			if(Ply[fd].extr->tempstr[1][1] >= Ply[fd].extr->tempstr[1][0]) {
				vote_cmnd(fd, 3, "");
				RETURN(fd, command, 1);
			}
			else {
				fp = fopen(Ply[fd].extr->tempstr[0], "r");
				if(!fp) {
					F_CLR(Ply[fd].ply, PREADI);
					RETURN(fd, command, 1);
				}
				fgets(tmp, 1024, fp);
				if(feof(fp)) {
					F_CLR(Ply[fd].ply, PREADI);
					RETURN(fd, command, 1);
				}
				n = 0;
				while(!feof(fp)) {
					fgets(tmp, 1024, fp);
					tmp[1023] = 0;
					n++;
					if( n  == Ply[fd].extr->tempstr[1][1]+1)
						break;
				}
				print(fd,"\n%s", tmp);
				print(fd, "How do you cast your vote? ");
				output_buf();

				Ply[fd].extr->tempstr[1][1] = n;
				fgets(tmp, 1024, fp);
				if(feof(fp))
					Ply[fd].extr->tempstr[1][0] = Ply[fd].extr->tempstr[1][1];
				fclose(fp);
	
   	        	Ply[fd].io->intrpt &= ~1; 
				RETURN(fd, vote_cmnd, 2);
			}
				break;

		case 3:
			n = Ply[fd].extr->tempstr[1][0] ;
			sprintf(tmp, "%s/vote/%s_v", PLAYERPATH, ply_ptr->name);
			Ply[fd].extr->tempstr[2][n] = 0;
			i = open(tmp, O_CREAT | O_RDWR, ACC);
			if(i < 0)
                merror("vote_cmnd", FATAL);
			write(i, Ply[fd].extr->tempstr[2], n);
			close(i);
			print(fd, "voted.\n");
			F_CLR(Ply[fd].ply, PREADI);
            RETURN(fd, command, 1);
			break;
	}
}

/*************************************************************************/
/*                              pfinger                                  */
/*************************************************************************/

int pfinger(ply_ptr, cmnd) 
creature	*ply_ptr;
cmd			*cmnd;

{
    struct stat f_stat;
    creature    *player;
	char		tmp[80];
	int			fd;

	fd = ply_ptr->fd; 
    if(cmnd->num < 2) {
        print(fd, "Finger whom?\n");
        return(0);
    }

	cmnd->str[1][0] = up(cmnd->str[1][0]);
	player = find_who(cmnd->str[1]);

	if(!player) {
		if(load_ply(cmnd->str[1], &player) < 0) {
			print(fd, "Person does not exist.\n");
			return (0);
    	}             
		if(!strcmp(player->name, DMNAME) ||
		   !strcmp(player->name, DMNAME2) ||
		   !strcmp(player->name, DMNAME3) ||
		   !strcmp(player->name, DMNAME4)) {
				return (PROMPT);
		}
		if(ply_ptr->class < CARETAKER && (player->class == CARETAKER ||
			player->class == DM)) {
				print(fd, "You may not finger that person.\n");
				return (0);
		}
		sprintf(tmp, "%s/%s", PLAYERPATH, cmnd->str[1]);
		if(stat(tmp, &f_stat)) {
			print(fd, "That person does not exist.\n");
			return (0);
		}
	    print(fd, "%s %+25s %+15s\n", 
			player->name, race_str[player->race], title_ply(player));
    	print(fd,"last login: %s", ctime(&f_stat.st_ctime));
    	free_crt(player);
	} 
	else {
		if(!strcmp(player->name, DMNAME) ||
		   !strcmp(player->name, DMNAME2) ||
		   !strcmp(player->name, DMNAME3) ||
		   !strcmp(player->name, DMNAME4)) {
				return (PROMPT);
		}
		if(F_ISSET(player, PDMINV) && (ply_ptr->class < CARETAKER || 
			(ply_ptr->class == CARETAKER && player->class == DM))) {
				print(fd, "You are currently unable to finger that person.\n");
				return (0);
		}
	    print(fd,"%s %+25s %+15s\n", player->name, 
   	    	 race_str[player->race], title_ply(player));
    	print(fd, "Currently logged on.\n");
	}

    sprintf(tmp, "%s/%s", POSTPATH, cmnd->str[1]);

    if(stat(tmp, &f_stat)) {
       print(fd, "No mail.\n");
       return (0);
    }

	if(f_stat.st_atime > f_stat.st_ctime)
		print(fd, "No unread mail since: %s", ctime(&f_stat.st_atime));
 	else
		print(fd, "New mail since: %s", ctime(&f_stat.st_ctime));
}
