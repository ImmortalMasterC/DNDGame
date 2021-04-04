/*
 * DM2.C:
 *
 *	DM functions
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include <string.h>
#include <stdlib.h>


/************************************************************************/
/*				dm_stat					*/
/************************************************************************/
/*  This function will allow a DM to display information on an object	*/
/*  creature, player, or room.						*/

int dm_stat(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room		*rom_ptr, *home_rom;
	object		*obj_ptr;
	object		*obj_ptr2;
	creature	*crt_ptr;
	creature	*ply_ptr2;
	char		str[2048];
	int		fd, n, i, j;
	int		not_logged = 0;

	if(ply_ptr->class < CARETAKER)
		return(PROMPT);

	fd = ply_ptr->fd;


/* Give stats on room DM is currently in, or specified room # */

	if(cmnd->num < 2) {
		if(cmnd->val[0] >= RMAX) 
			return(0);
		if(cmnd->val[0] == 1)
			rom_ptr = ply_ptr->parent_rom;
		else {
			if(load_rom(cmnd->val[0], &rom_ptr) < 0) {
				print(fd, "Error (%d)\n", cmnd->val[0]);
				return(0);
			}
		}
		stat_rom(ply_ptr, rom_ptr);
		return(0);
	}


/*  Use player reference through 2nd parameter or default to DM */

	if(cmnd->num < 3)
		ply_ptr2 = ply_ptr;

	else {
		ply_ptr2 = find_crt(ply_ptr, ply_ptr->parent_rom->first_mon,
		    cmnd->str[2], cmnd->val[2]);
		cmnd->str[2][0] = up(cmnd->str[2][0]);
		if(!ply_ptr2)
			ply_ptr2 = find_crt(ply_ptr, ply_ptr->parent_rom->first_ply,
			    cmnd->str[2], cmnd->val[2]);
		if(!ply_ptr2)
			ply_ptr2 = find_who(cmnd->str[2]);
		if(!ply_ptr2 || (ply_ptr->class < DM && F_ISSET(ply_ptr2, PDMINV)))
			ply_ptr2 = ply_ptr;
	}

	rom_ptr = ply_ptr2->parent_rom;

	/* Give info on object, if found */
	obj_ptr = find_obj(ply_ptr2, ply_ptr2->first_obj, cmnd->str[1], 
		cmnd->val[1]);

	if(!obj_ptr) {
		for(i = 0, j = 0; i < MAXWEAR; i++) {
			if(EQUAL(ply_ptr2->ready[i], cmnd->str[1])) {
				j++;
				if(j == cmnd->val[1]) {
					obj_ptr = ply_ptr2->ready[i];
					break;
				}
			}
		}
	}

	if(!obj_ptr)
		obj_ptr = find_obj(ply_ptr2, rom_ptr->first_obj,
		   cmnd->str[1], cmnd->val[1]);

	if(obj_ptr) {
		stat_obj(ply_ptr, obj_ptr);
		return(0);
	}


/*  Search for creature or player to get info on */

	if(cmnd->str[1][0] =='.') 
		strcpy(cmnd->str[1], ply_ptr->name);
	crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon, cmnd->str[1], 
		cmnd->val[1]);
	cmnd->str[1][0] = up(cmnd->str[1][0]);

	if(!crt_ptr)
		crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply, cmnd->str[1], 
			cmnd->val[1]);

	if(!crt_ptr)
		crt_ptr = find_who(cmnd->str[1]);

	if(!crt_ptr) {
		if(load_ply(cmnd->str[1], &crt_ptr) < 0) {
			print(fd, "That person does not exist.\n"); 
			return(PROMPT);
		}
		if(!Ply[crt_ptr->fd].io) {
			print(fd, "Unable to load player.\n");
			return(PROMPT);
		}
		not_logged = 1;
	}

	if(crt_ptr) { 
		stat_crt(ply_ptr, crt_ptr);
		if(not_logged)
			free(crt_ptr);
		return(0);
	}
	else
		print(fd, "Unable to locate.\n");

	return(0);
}

/********************************************************************/
/*			stat_rom				    */
/********************************************************************/
/*  Display information on room given to player given.		    */

short		checklist[10];
int 		stat_rom(ply_ptr, rom_ptr)
creature	*ply_ptr;
room		*rom_ptr;

{
	short		checklist[10];
	int		i, fd;
	char		str[1024];
	xtag		*next_xtag;
	exit_		*ext;

	fd = ply_ptr->fd;
	ANSI(fd, YELLOW);	
	print(fd, "Room # %d\t  ", rom_ptr->rom_num);
	print(fd, "%s\t  ", rom_ptr->name);
	print(fd, "Traffic: %d%%\n", rom_ptr->traffic);


/*	Print permanent monsters and objects				*/

	print(fd, "\n Permanent objects:");				
	for(i = 0; i < 10; i++) {										
		if(rom_ptr->perm_obj[i].misc)
			print(fd, " %3hd", rom_ptr->perm_obj[i].misc);		
		else continue;
	}
        print(fd, "\n");

	print(fd, "Permanent monsters:");					
	for(i = 0; i < 10; i++) {						
		if(rom_ptr->perm_mon[i].misc)
			print(fd, " %3hd", rom_ptr->perm_mon[i].misc);		
		else continue;
	}
        print(fd, "\n");
	
	print(fd, "   Random monsters:");
	for(i = 0; i < 10; i++) {					
		if(rom_ptr->random[i])
			print(fd, " %3hd", rom_ptr->random[i]);
		else continue;
	}
	print(fd, "\n");
	print(fd, "------------------------------------------"
			  "-----------------------------------");  
	print(fd, "\n");


/*	Print trap types		 					*/

	if (rom_ptr->trap) {
		print(fd, " Trap type: ");	
		switch(rom_ptr->trap) {

		   case TRAP_PIT:
				print(fd, "Pit Trap (exit room %d)\n", rom_ptr->trapexit);
		    	break;

		   case TRAP_DART:
				print(fd, "Poison dart trap\n");
		    	break;

		   case TRAP_BLOCK:
				print(fd, "Falling block trap\n");
		    	break;

		   case TRAP_MPDAM:
				print(fd, "MP damage trap\n");
		    	break;

		   case TRAP_RMSPL:
				print(fd, "Negate spell trap\n");
		    	break;

		   case TRAP_NAKED:
				print(fd, "Naked trap\n");
		    	break;

		   case TRAP_ALARM:
				print(fd, "Alarm trap (guard room %d)\n", rom_ptr->trapexit);
		    	break;
		  
		   default:
				print(fd, "Invalid trap #\n");
		   		break;
		}
	}


/*	Print boundary levels 							*/

	if(rom_ptr->lolevel || rom_ptr->hilevel) {
		print(fd, "    Level boundary:  ");
		if(rom_ptr->lolevel && !(rom_ptr->hilevel))
			print(fd,"%d or higher.\n",rom_ptr->lolevel);
		if(rom_ptr->hilevel && !(rom_ptr->lolevel))
			print(fd,"%d or lower.\n",rom_ptr->hilevel);
		if(rom_ptr->hilevel == rom_ptr->lolevel)
			print(fd,"level %d only.\n",rom_ptr->hilevel);
		if(rom_ptr->lolevel && rom_ptr->lolevel < rom_ptr->hilevel)
			print(fd,"%d to %d inclusive\n",rom_ptr->lolevel, 
				rom_ptr->hilevel);
		print(fd, "------------------------------------------"
				  "-----------------------------------\n");  
	}


/*	Print room flags						*/

/*----------------------------------------------------------------------*/
/*		Determine class training				*/
/*		Flag 7 is LSB, and Flag 4 is MSB			*/
/*		4 bits = 16 possible training locations			*/
/*		binary number is equivalent to decimal class number	*/
/*----------------------------------------------------------------------*/

	if(F_ISSET(rom_ptr, RTRAIN) &&	 	 /* Room Flag 4		*/
		!F_ISSET(rom_ptr, RTRAIN + 1) &&
		!F_ISSET(rom_ptr, RTRAIN + 2) &&
		!F_ISSET(rom_ptr, RTRAIN + 3))
			strcpy(str, "Flag 4 - Thief");

	if(!F_ISSET(rom_ptr, RTRAIN) &&		
		F_ISSET(rom_ptr, RTRAIN + 1) &&	 /* Room Flag 5		*/
		!F_ISSET(rom_ptr, RTRAIN + 2) &&
		!F_ISSET(rom_ptr, RTRAIN + 3))
			strcpy(str, "Flag 5 - Fighter");

	if(F_ISSET(rom_ptr, RTRAIN) &&		 /* Room Flag 4		*/ 
		F_ISSET(rom_ptr, RTRAIN + 1) &&	 /* Room Flag 5		*/
		!F_ISSET(rom_ptr, RTRAIN + 2) &&
		!F_ISSET(rom_ptr, RTRAIN + 3))
			strcpy(str, "Flags 4/5 -  Alchemist");

	if(F_ISSET(rom_ptr, RTRAIN) &&		 /* Room Flag 4		*/ 
		!F_ISSET(rom_ptr, RTRAIN + 1) &&
		F_ISSET(rom_ptr, RTRAIN + 2) &&	 /* Room Flag 6		*/
		!F_ISSET(rom_ptr, RTRAIN + 3))
			strcpy(str, "Flags 4/6 - Monk");

	if(!F_ISSET(rom_ptr, RTRAIN) &&		 
		!F_ISSET(rom_ptr, RTRAIN + 1) &&
		F_ISSET(rom_ptr, RTRAIN + 2) &&	 /* Room Flag 6		*/
		!F_ISSET(rom_ptr, RTRAIN + 3))
			strcpy(str, "Flag 6 - Barbarian");

	if(!F_ISSET(rom_ptr, RTRAIN) &&		
		!F_ISSET(rom_ptr, RTRAIN + 1) &&
		F_ISSET(rom_ptr, RTRAIN + 2) &&	 /* Room Flag 6		*/
		F_ISSET(rom_ptr, RTRAIN + 3))	 /* Room Flag 7		*/
			strcpy(str, "Flags 6/7 - Cleric");

	if(!F_ISSET(rom_ptr, RTRAIN) &&		
		F_ISSET(rom_ptr, RTRAIN + 1) &&	 /* Room Flag 5		*/
		F_ISSET(rom_ptr, RTRAIN + 2) &&	 /* Room Flag 6		*/
		!F_ISSET(rom_ptr, RTRAIN + 3))
			strcpy(str, "Flags 5/6 - Paladin");

	if(!F_ISSET(rom_ptr, RTRAIN) &&		 
		F_ISSET(rom_ptr, RTRAIN + 1) &&	 /* Room Flag 5		*/
		F_ISSET(rom_ptr, RTRAIN + 2) &&	 /* Room Flag 6		*/
		F_ISSET(rom_ptr, RTRAIN + 3))	 /* Room Flag 7		*/
			strcpy(str, "Flags 5/6/7 - Ranger");

	if(!F_ISSET(rom_ptr, RTRAIN) &&		
		!F_ISSET(rom_ptr, RTRAIN + 1) &&
		!F_ISSET(rom_ptr, RTRAIN + 2) &&
		F_ISSET(rom_ptr, RTRAIN + 3))	 /* Room Flag 7		*/
			strcpy(str, "Flag 7 - Assassin");

	if(F_ISSET(rom_ptr, RTRAIN) &&		 /* Room Flag 4	*/
		!F_ISSET(rom_ptr, RTRAIN + 1) &&
		!F_ISSET(rom_ptr, RTRAIN + 2) &&
		F_ISSET(rom_ptr, RTRAIN + 3))	 /* Room Flag 7		*/
			strcpy(str, "Flags 4/7 - Bard");

	if(!F_ISSET(rom_ptr, RTRAIN) &&	
		F_ISSET(rom_ptr, RTRAIN + 1) &&	 /* Room Flag 5		*/
		!F_ISSET(rom_ptr, RTRAIN + 2) &&
		F_ISSET(rom_ptr, RTRAIN + 3))	 /* Room Flag 7		*/
			strcpy(str, "Flags 5/7 - Mage");

	if(F_ISSET(rom_ptr, RTRAIN) &&		 /* Room Flag 4		*/ 
		!F_ISSET(rom_ptr, RTRAIN + 1) &&
		F_ISSET(rom_ptr, RTRAIN + 2) &&	 /* Room Flag 6		*/
		F_ISSET(rom_ptr, RTRAIN + 3))	 /* Room Flag 7		*/
			strcpy(str, "Flags 4/6/7 - Druid");	

	if(F_ISSET(rom_ptr, RTRAIN) ||
		F_ISSET(rom_ptr, RTRAIN + 1) ||
		F_ISSET(rom_ptr, RTRAIN + 2) ||
		F_ISSET(rom_ptr, RTRAIN + 3)) { 
			print(fd, "%s Training.\n", str);		
			print(fd, "------------------------------------------"
					  "-----------------------------------");  
		    print(fd, "\n");
		}

	if(rom_ptr->special) 
		print(fd, "   Special: %d\n", rom_ptr->special);

	strcpy(str, "Room Flags: ");
	if(F_ISSET(rom_ptr, RSHOPP)) strcat(str, "Shoppe-1, ");
	if(F_ISSET(rom_ptr, RDUMPR)) strcat(str, "DumpRoom-2, ");
	if(F_ISSET(rom_ptr, RPAWNS)) strcat(str, "PawnShop-3, ");
	if(F_ISSET(rom_ptr, RREPAI)) strcat(str, "Repair-8, ");
	if(F_ISSET(rom_ptr, RDARKR)) strcat(str, "DarkAlways-9, ");
	if(F_ISSET(rom_ptr, RDARKN)) strcat(str, "DarkNight-10, ");
	if(F_ISSET(rom_ptr, RPOSTO)) strcat(str, "PostOffice-11, ");
	if(F_ISSET(rom_ptr, RNOKIL)) strcat(str, "NoPlyKill-12, ");
	if(F_ISSET(rom_ptr, RNOTEL)) strcat(str, "NoTeleport-13, ");
	if(F_ISSET(rom_ptr, RHEALR)) strcat(str, "HealFast-14, ");
	if(F_ISSET(rom_ptr, RONEPL)) strcat(str, "OnePlyr-15, ");
	if(F_ISSET(rom_ptr, RTWOPL)) strcat(str, "TwoPlyr-16, ");
	if(F_ISSET(rom_ptr, RTHREE)) strcat(str, "ThreePlyr-17, ");
	if(F_ISSET(rom_ptr, RNOMAG)) strcat(str, "NoMagic-18, ");
	if(F_ISSET(rom_ptr, RPTRAK)) strcat(str, "PermTrack-19, ");
	if(F_ISSET(rom_ptr, REARTH)) strcat(str, "Earth-20, ");
	if(F_ISSET(rom_ptr, RWINDR)) strcat(str, "Wind-21, ");
	if(F_ISSET(rom_ptr, RFIRER)) strcat(str, "Fire-22, ");
	if(F_ISSET(rom_ptr, RWATER)) strcat(str, "Water-23, ");
	if(F_ISSET(rom_ptr, RPLWAN)) strcat(str, "Grpwander-24, ");
	if(F_ISSET(rom_ptr, RPHARM)) strcat(str, "PHarm-25, ");
	if(F_ISSET(rom_ptr, RPPOIS)) strcat(str, "PlyrPoison-26, ");
	if(F_ISSET(rom_ptr, RPMPDR)) strcat(str, "PlyrMP-Drain-27, ");
	if(F_ISSET(rom_ptr, RPBEFU)) strcat(str, "Befuddle-28, ");
	if(F_ISSET(rom_ptr, RNOLEA)) strcat(str, "NoSummon-29, ");
	if(F_ISSET(rom_ptr, RPLDGK)) strcat(str, "Pledge-30, ");
	if(F_ISSET(rom_ptr, RRSCND)) strcat(str, "Rescind-31, ");
	if(F_ISSET(rom_ptr, RNOPOT)) strcat(str, "NoPotion-32, ");
	if(F_ISSET(rom_ptr, RPMEXT)) strcat(str, "PlyrMagic-33, ");
	if(F_ISSET(rom_ptr, RNOLOG)) strcat(str, "NoLog-34, ");
	if(F_ISSET(rom_ptr, RELECT)) strcat(str, "Elect-35, ");
	if(F_ISSET(rom_ptr, RNDOOR)) strcat(str, "InDoors-36, ");
	if(F_ISSET(rom_ptr, RSLIME)) strcat(str, "Slime-37, ");
	if(F_ISSET(rom_ptr, RJAILR)) strcat(str, "Jail-38, ");
	if(F_ISSET(rom_ptr, RNHOME)) strcat(str, "NoHome-39, ");
	if(F_ISSET(rom_ptr, RSILVR)) strcat(str, "SilvrRepair-40, ");
	if(F_ISSET(rom_ptr, RPBANK)) strcat(str, "Bank-41, ");
	if(F_ISSET(rom_ptr, RPSHOP)) strcat(str, "PlyrShop-42, ");
	if(strlen(str) > 13) {
		str[strlen(str) - 2] = '.';
		str[strlen(str) - 1] = 0;
	}
	else
		strcat(str, "None.");

	print(fd, "%s\n", str);

/*	Print room exits				*/
	print(fd, "Room Exits:\n");
	next_xtag = rom_ptr->first_ext;
	while(next_xtag) {
		ext = next_xtag->ext;
		print(fd, "%d - %s", ext->room, ext->name);
		if(ext->key) 
			print(fd, " (Key-%dd0)", ext->key);
		if(ext->ltime.misc)
			print(fd, "(misc=%d)", (ext->ltime.misc));
		*str = 0;
		if(F_ISSET(ext, XSECRT)) strcat(str, "Secrt-1, ");
		if(F_ISSET(ext, XINVIS)) strcat(str, "Invis-2, ");
		if(F_ISSET(ext, XLOCKD)) strcat(str, "Lckd-3, ");
		if(F_ISSET(ext, XCLOSD)) strcat(str, "Clsd-4, ");
		if(F_ISSET(ext, XLOCKS)) strcat(str, "Lckbl-5, ");
		if(F_ISSET(ext, XCLOSS)) strcat(str, "Clsbl-6, ");
		if(F_ISSET(ext, XUNPCK)) strcat(str, "UnPik-7, ");
		if(F_ISSET(ext, XNAKED)) strcat(str, "Naked-8, ");
		if(F_ISSET(ext, XCLIMB)) strcat(str, "ClmbUp-9, ");
		if(F_ISSET(ext, XREPEL)) strcat(str, "ClmbRepl-10, ");
		if(F_ISSET(ext, XDCLIM)) strcat(str, "HrdClmb-11, ");
		if(F_ISSET(ext, XFLYSP)) strcat(str, "Fly-12, ");
		if(F_ISSET(ext, XFEMAL)) strcat(str, "Fmale-13, ");
		if(F_ISSET(ext, XMALES)) strcat(str, "Male-14, ");
		if(F_ISSET(ext, XNGHTO)) strcat(str, "Nght-17, ");
		if(F_ISSET(ext, XDAYON)) strcat(str, "Day-18, ");
		if(F_ISSET(ext, XPGUAR)) strcat(str, "PassvGuard-19, ");
		if(F_ISSET(ext, XNOSEE)) strcat(str, "NoSee-20, ");
		if(F_ISSET(ext, XSMALL)) strcat(str, "Smll-38, ");
		if(F_ISSET(ext, XLARGE)) strcat(str, "Lrge-39, ");
		if(F_ISSET(ext, XNOBIG)) strcat(str, "NoBig-40, ");
		if(F_ISSET(ext, XMOUSE)) strcat(str, "MouseSiz-58, ");
		if(F_ISSET(ext, XNOEVL)) strcat(str, "NoEvil-59, ");
		if(F_ISSET(ext, XNGOOD)) strcat(str, "NoGood-60, ");
		if(F_ISSET(ext, XNPRNT)) strcat(str, "NoPrnt-61, ");
		if(F_ISSET(ext, XTPRNT)) strcat(str, "TmpPrnt-62, ");
		if(F_ISSET(ext, XMONEY)) strcat(str, "PaidExit-63, ");
		if(F_ISSET(ext, XNOINV)) strcat(str, "InvsCanntUse-64, ");
		if(F_ISSET(ext, XPLSEL)) {
			strcat(str ,"Class/");
			if(F_ISSET(ext, XPASSN)) strcat(str, "Assn-22, ");
			if(F_ISSET(ext, XPALCH)) strcat(str, "Alch-33, ");
			if(F_ISSET(ext, XPBARB)) strcat(str, "Barb-23, ");
			if(F_ISSET(ext, XPBARD)) strcat(str, "Bard-30, ");
			if(F_ISSET(ext, XPCLER)) strcat(str, "Clrc-24, ");
			if(F_ISSET(ext, XPDRUD)) strcat(str, "Dru-32, ");
			if(F_ISSET(ext, XPFGHT)) strcat(str, "Fght-25, ");
			if(F_ISSET(ext, XPMAGE)) strcat(str, "Mage-26, ");
			if(F_ISSET(ext, XPMONK)) strcat(str, "Monk-31, ");
			if(F_ISSET(ext, XPPALA)) strcat(str, "Pala-27, ");
			if(F_ISSET(ext, XPRNGR)) strcat(str, "Rngr-28, ");
			if(F_ISSET(ext, XPTHEF)) strcat(str, "Thf-29, ");
		}
		if(F_ISSET(ext, XRACES)) {
			strcat(str, "Race-41/");
			if(F_ISSET(ext, XRSDWF)) strcat(str, "Dwrf-42,");
			if(F_ISSET(ext, XRSELF)) strcat(str, "Elf-43, ");
			if(F_ISSET(ext, XRSHEF)) strcat(str, "H-Elf-44, ");
			if(F_ISSET(ext, XRSHOB)) strcat(str, "Hbbit-45, ");
			if(F_ISSET(ext, XRSHUM)) strcat(str, "Hum-46, ");
			if(F_ISSET(ext, XRSORC)) strcat(str, "Orc-47, ");
			if(F_ISSET(ext, XRSHGT)) strcat(str, "H-Gnt-48, ");
			if(F_ISSET(ext, XRSGNM)) strcat(str, "Gno-49, ");
			if(F_ISSET(ext, XRSSAT)) strcat(str, "Satyr-50, ");
			if(F_ISSET(ext, XRSHOC)) strcat(str, "H-Orc-51, ");
			if(F_ISSET(ext, XRSOGR)) strcat(str, "Ogre-52, ");
			if(F_ISSET(ext, XRSDEF)) strcat(str, "Dk-Elf-53, ");
			if(F_ISSET(ext, XRSCNT)) strcat(str, "Cntaur-54, ");
		}
		if(F_ISSET(ext, XPLDGK)) strcat(str, "Pldg-15, ");
		if(F_ISSET(ext, XPLDGK))
			if(F_ISSET(ext, XKNGDM)) strcat(str, "Rutg-16, ");
			else strcat(str, "Rag-!16, ");

		if(*str) {
			str[strlen(str) - 2] = '.';
			str[strlen(str) - 1] = 0;
			print(fd, ":  %s\n", str);
		}
		else
			print(fd, ".\n");
		
		next_xtag = next_xtag->next_tag;
	}
	ANSI(fd, BOLD);
	ANSI(fd, WHITE);
}

/************************************************************************/
/*			stat_crt					*/
/************************************************************************/
/*  Display information about player or creature to DM.			*/

int stat_crt(ply_ptr, crt_ptr)
creature	*ply_ptr;
creature	*crt_ptr;

{
	room		*home_rom;
	char		str[1024], temp[20];
	char		spl[128][20];
	int			i, j, n, l = 1, fd, total_stats = 0;
	long		t;

	fd = ply_ptr->fd;

	t=time(0);

	CLS(fd);

/*======================================================================*/
/*		Display stats for player				*/

	if(crt_ptr->type == PLAYER && Ply[crt_ptr->fd].io) {
		ANSI(fd, YELLOW);
		rc(fd, 1, 1);
		print(fd, "%s the %s,", crt_ptr->name, title_ply(crt_ptr));
		print(fd,"  %-3d years old.",
        	18 + crt_ptr->lasttime[LT_HOURS].interval/86400L);

		if(F_ISSET(crt_ptr, PCHAOS)) {
			ANSI(fd, RED);
			print(fd, "   Chaotic."); 
		}
		else {
			ANSI(fd, CYAN);
			print(fd, "   Lawful."); 
		}

		rc(fd, 1, 55);
		ANSI(fd, GREEN);
		print(fd, "Level: "); 
		ANSI(fd, YELLOW);
		print(fd, "%d", crt_ptr->level);

		ANSI(fd, GREEN);
		rc(fd, 1, 66);
		print(fd, "Idle:" );
		ANSI(fd, WHITE);
		print(fd, " %02ld:%02ld", (t-Ply[crt_ptr->fd].io->ltime)/60L,
			(t-Ply[crt_ptr->fd].io->ltime)%60L);

		ANSI(fd, GREEN);
		rc(fd, 2, 1);
		print(fd, "Desc: "); 
		ANSI(fd, WHITE);
		print(fd, "%s", crt_ptr->description);

		ANSI(fd, GREEN);
		rc(fd, 3, 1);
		print(fd, "IP Address: "); 
		ANSI(fd, WHITE);
		print(fd, "%s@%s", Ply[crt_ptr->fd].io->userid,
			Ply[crt_ptr->fd].io->address);

		ANSI(fd, GREEN);
		rc(fd, 4, 1);
		print(fd, "Room: ");
		ANSI(fd, WHITE);
		print(fd, "%-5hd", Ply[crt_ptr->fd].ply->rom_num);

		ANSI(fd, GREEN);
		rc(fd, 4, 20);
		print(fd, "Home: ");
		ANSI(fd, WHITE);
		print(fd, "%-5hd", Ply[crt_ptr->fd].extr->home_rom->rom_num);

		ANSI(fd, GREEN);
		rc(fd, 5, 1);
		print(fd, "Command: "); 
		ANSI(fd, WHITE);
		print(fd, "%s", Ply[crt_ptr->fd].extr->lastcommand);

		ANSI(fd, GREEN);
		rc(fd, 5, 61);
		print(fd, "Exper: ");
		ANSI(fd, WHITE);
		print(fd, "%d", crt_ptr->experience);

		rc(fd, 6, 1);
		ANSI(fd, BLUE);
		print(fd, "------------------------------------------"
				  "-----------------------------------");  

		rc(fd, 7, 1);
		ANSI(fd, GREEN);
		print(fd, "Class: ");
		ANSI(fd, WHITE);
		print(fd, "%s", class_str[crt_ptr->class]);

		rc(fd, 7, 28);
		ANSI(fd, GREEN);
		print(fd, "Race: "); 
		ANSI(fd, WHITE);
		print(fd, "%s", race_str[crt_ptr->race]); 

		rc(fd, 7, 45);
		ANSI(fd, GREEN);
		print(fd, "Agility: "); 
		ANSI(fd, WHITE);
		print(fd, "%d", Ply[crt_ptr->fd].extr->agility);

		rc(fd, 7, 62);
		ANSI(fd, GREEN);
		print(fd, "Luck: ");
		ANSI(fd, WHITE);
		print(fd, "%d", Ply[crt_ptr->fd].extr->luck);
	
		rc(fd, 7, 72);
		ANSI(fd, GREEN);
		print(fd, "Sex: "); 
		ANSI(fd, WHITE);
		if(F_ISSET(crt_ptr, MNOGEN))
			print(fd, "N");
		else
			print(fd, "%s", F_ISSET(crt_ptr, PMALES) ? "M" : "F"); 

		total_stats = crt_ptr->strength + crt_ptr->dexterity + 
			crt_ptr->constitution + crt_ptr->intelligence + crt_ptr->piety;

		ANSI(fd, GREEN);
		rc(fd, 8, 1);
		print(fd, "Strength     Dexterity     Constitution     Intelligence     Piety     Total");  

		ANSI(fd, WHITE);
		rc(fd, 9, 1);
		print(fd, "   %2d           %2d              %2d              %2d            %2d        %2d",  
			crt_ptr->strength, 
			    crt_ptr->dexterity, 
			        crt_ptr->constitution, 
			           crt_ptr->intelligence, 
			              crt_ptr->piety, 
							total_stats);

		ANSI(fd, BLUE);
		rc(fd, 10, 1);
		print(fd, "------------------------------------------"
				  "-----------------------------------");  
		ANSI(fd, GREEN);
		rc(fd, 11, 1);
		print(fd, "HP: "); 
		ANSI(fd, WHITE);
		print(fd, "%d/%d", crt_ptr->hpcur, crt_ptr->hpmax);

		ANSI(fd, GREEN);
		rc(fd, 11, 23);
		print(fd, "Hit: ");
		ANSI(fd, WHITE);
		print(fd, "%dd%d+%d", crt_ptr->ndice, crt_ptr->sdice, crt_ptr->pdice);

		ANSI(fd, GREEN);
		rc(fd, 11, 44);
		print(fd, "AC: ");
		ANSI(fd, WHITE);
		print(fd, "%d", crt_ptr->armor);

		ANSI(fd, GREEN);
		rc(fd, 11, 62);
		print(fd, "Bank: ");
		ANSI(fd, WHITE);
		print(fd, "%d", crt_ptr->bank_bal);

		ANSI(fd, GREEN);
		rc(fd, 12, 1);
		print(fd, "MP: "); 
		ANSI(fd, WHITE);
		print(fd, "%d/%d", crt_ptr->mpcur, crt_ptr->mpmax);

		ANSI(fd, GREEN);
		rc(fd, 12, 21);
		print(fd, "Align: ");
		ANSI(fd, WHITE);
		print(fd, "%d", crt_ptr->alignment);

		ANSI(fd, GREEN);
		rc(fd, 12, 41);
		print(fd, "THAC0: ");
		ANSI(fd, WHITE);
		print(fd, "%d", crt_ptr->thaco);

		ANSI(fd, GREEN);
		rc(fd, 12, 62);
		print(fd, "Gold: ");
		ANSI(fd, WHITE);
		print(fd, "%d", crt_ptr->gold);

		ANSI(fd, BLUE);
		rc(fd, 13, 1);
		print(fd, "------------------------------------------"
				  "-----------------------------------");  

		ANSI(fd, GREEN);
		rc(fd, 14, 1);
		print(fd, "Sharp: "); 
		ANSI(fd, WHITE);
    	print(fd, "%d%%", profic(crt_ptr, SHARP));

		ANSI(fd, GREEN);
		rc(fd, 14, 16);
		print(fd, "Thrust: "); 
		ANSI(fd, WHITE);
   		print(fd, "%d%%", profic(crt_ptr, THRUST));

		ANSI(fd, GREEN);
		rc(fd, 14, 31);
		print(fd, "Blunt: "); 
		ANSI(fd, WHITE);
   		print(fd, "%d%%", profic(crt_ptr, BLUNT));

		ANSI(fd, GREEN);
		rc(fd, 14, 46);
		print(fd, "Pole: "); 
		ANSI(fd, WHITE);
   		print(fd, "%d%%", profic(crt_ptr, POLE));

		ANSI(fd, GREEN);
		rc(fd, 14, 61);
		print(fd, "Missile: "); 
		ANSI(fd, WHITE);
   		print(fd, "%d%%", profic(crt_ptr, MISSILE));

		ANSI(fd, CYAN);
		rc(fd, 15, 1);	 print(fd, "%ld",  crt_ptr->proficiency[0]);
		rc(fd, 15, 16);	 print(fd, "%ld",  crt_ptr->proficiency[1]);
		rc(fd, 15, 31);	 print(fd, "%ld",  crt_ptr->proficiency[2]);
		rc(fd, 15, 46);	 print(fd, "%ld",  crt_ptr->proficiency[3]);
		rc(fd, 15, 61);	 print(fd, "%ld",  crt_ptr->proficiency[4]);

		ANSI(fd, GREEN);
		rc(fd, 17, 1);
		print(fd, "Earth: "); 
		ANSI(fd, WHITE);
		print(fd, "%d%%", mprofic(crt_ptr, EARTH));

		ANSI(fd, GREEN);
		rc(fd, 17, 18);
		print(fd, "Wind: "); 
		ANSI(fd, WHITE);
		print(fd, "%d%%", mprofic(crt_ptr, WIND));

		ANSI(fd, GREEN);
		rc(fd, 17, 32);
		print(fd, "Fire: "); 
		ANSI(fd, WHITE);
		print(fd, "%d%%", mprofic(crt_ptr, FIRE));

		ANSI(fd, GREEN);
		rc(fd, 17, 45);
		print(fd, "Water: "); 
		ANSI(fd, WHITE);
		print(fd, "%d%%", mprofic(crt_ptr, WATER));

		ANSI(fd, CYAN);
		rc(fd, 18, 1);    print(fd, "%ld",  crt_ptr->realm[0]);
		rc(fd, 18, 18);   print(fd, "%ld",  crt_ptr->realm[1]);
		rc(fd, 18, 32);   print(fd, "%ld",  crt_ptr->realm[2]);
		rc(fd, 18, 45);   print(fd, "%ld",  crt_ptr->realm[3]);

		ANSI(fd, BLUE);
		rc(fd, 19, 1);
		print(fd, "------------------------------------------"
				  "-----------------------------------");  
		ANSI(fd, GREEN);

		strcpy(str,"Flags set:  ");

		if(F_ISSET(crt_ptr, PWIMPY)) {
			sprintf(temp, "Wimpy = %d, ", crt_ptr->WIMPYVALUE);
			strcat(str, temp);
		}
		if(F_ISSET(crt_ptr, PBLESS)) strcat(str, "Bless-1, ");
		if(F_ISSET(crt_ptr, PHIDDN)) strcat(str, "Hidden-2, ");
		if(F_ISSET(crt_ptr, PINVIS)) strcat(str, "Invis-3, ");
		if(F_ISSET(crt_ptr, PNOBRD)) strcat(str, "NoBroad-4, ");
		if(F_ISSET(crt_ptr, PNOLDS)) strcat(str, "NoLong-5, ");
		if(F_ISSET(crt_ptr, PFROZE)) strcat(str, "Frozen-6, ");
		if(F_ISSET(crt_ptr, PHEXED)) strcat(str, "Hexed-7, ");
		if(F_ISSET(crt_ptr, PASSHL)) strcat(str, "Asshole-8, ");
		if(F_ISSET(crt_ptr, PPROTE)) strcat(str, "Protect-9, ");
		if(F_ISSET(crt_ptr, PAUTOA)) strcat(str, "AutoAttk-10, ");
		if(F_ISSET(crt_ptr, PDMINV)) strcat(str, "DMInvis-11, ");
		if(F_ISSET(crt_ptr, PNOFOL)) strcat(str, "NoFollow-12, ");
		if(F_ISSET(crt_ptr, PMALES)) strcat(str, "Male-13, ");
		if(F_ISSET(crt_ptr, PNOCOP)) strcat(str, "NoCop-14, ");
		if(F_ISSET(crt_ptr, PEAVES)) strcat(str, "EavesDrp-16, ");
		if(F_ISSET(crt_ptr, PPOISN)) strcat(str, "Poisoned-17, ");
		if(F_ISSET(crt_ptr, PLIGHT)) strcat(str, "Light-18, ");
		if(F_ISSET(crt_ptr, PPROMP)) strcat(str, "Prompt-19, ");
		if(F_ISSET(crt_ptr, PHASTE)) strcat(str, "Haste-20, ");
		if(F_ISSET(crt_ptr, PDMAGI)) strcat(str, "DetMagic-21, ");
		if(F_ISSET(crt_ptr, PDINVI)) strcat(str, "DetInvis-22, ");
		if(F_ISSET(crt_ptr, PPRAYD)) strcat(str, "Pray-23, ");
		if(F_ISSET(crt_ptr, PLOANB)) strcat(str, "Loan-24, ");
		if(F_ISSET(crt_ptr, PPREPA)) strcat(str, "Prepared-25, ");
		if(F_ISSET(crt_ptr, PLEVIT)) strcat(str, "Levitate-26, ");
		if(F_ISSET(crt_ptr, PANSIC)) strcat(str, "Ansi-27, ");
		if(F_ISSET(crt_ptr, PCHAOS)) strcat(str, "Chaotic-29, ");
		if(F_ISSET(crt_ptr, PRFIRE)) strcat(str, "RstFire-31, ");
		if(F_ISSET(crt_ptr, PFLYSP)) strcat(str, "Fly-32, ");
		if(F_ISSET(crt_ptr, PRMAGI)) strcat(str, "RstMagic-33, ");
		if(F_ISSET(crt_ptr, PKNOWA)) strcat(str, "KnowAlgn-34, ");
		if(F_ISSET(crt_ptr, PNOSUM)) strcat(str, "Nosummon-35, ");
		if(F_ISSET(crt_ptr, PIGNOR)) strcat(str, "Ignore-36, ");
		if(F_ISSET(crt_ptr, PRCOLD)) strcat(str, "RstCold-37, ");
		if(F_ISSET(crt_ptr, PBRWAT)) strcat(str, "BreathWtr-38, ");
		if(F_ISSET(crt_ptr, PSSHLD)) strcat(str, "EarthShld-39, ");
		if(F_ISSET(crt_ptr, PPLDGK)) strcat(str, "Pledged-40, ");
		if(F_ISSET(crt_ptr, PDISEA)) strcat(str, "WillDisease-42, ");
		if(F_ISSET(crt_ptr, PBLIND)) strcat(str, "Blind-43, ");
		if(F_ISSET(crt_ptr, PFEARS)) strcat(str, "Fearful-44, ");
		if(F_ISSET(crt_ptr, PSILNC)) strcat(str, "Mute-45, ");
		if(F_ISSET(crt_ptr, PCHARM)) strcat(str, "Charmed-46, ");
		if(F_ISSET(crt_ptr, PNLOGN)) strcat(str, "NoLoginMsg-47, ");
		if(F_ISSET(crt_ptr, PLECHO)) strcat(str, "Echo-48, ");
		if(F_ISSET(crt_ptr, PSECOK)) strcat(str, "Secure-49, ");
		if(F_ISSET(crt_ptr, PAUTHD)) strcat(str, "Authd-50, ");
		if(F_ISSET(crt_ptr, PALIAS)) strcat(str, "DM-Alias-51, ");
		if(F_ISSET(crt_ptr, PSTRNG)) strcat(str, "Super-Str-52, ");
		if(F_ISSET(crt_ptr, PSHRNK)) strcat(str, "Shrink-53, ");
		if(F_ISSET(crt_ptr, PRFLCT)) strcat(str, "Reflect-54, ");
		if(F_ISSET(crt_ptr, PDETLK)) strcat(str, "Det-Lck-55, ");
		if(F_ISSET(crt_ptr, PEVEYE)) strcat(str, "Evil-eye-56, ");
		if(F_ISSET(crt_ptr, PLOANU)) strcat(str, "LoanUsed-57, ");
		if(F_ISSET(crt_ptr, PSPLIT)) strcat(str, "SpltGold-58, ");
		if(F_ISSET(crt_ptr, PCLOAK)) strcat(str, "Cloak-60, ");
		if(F_ISSET(crt_ptr, PBERSK)) strcat(str, "Berserk-61, ");
		if(F_ISSET(crt_ptr, PRACID)) strcat(str, "RstAcid-62, ");
		if(F_ISSET(crt_ptr, PPLDGK)) {
			if(F_ISSET(crt_ptr, PKNGDM)) strcat(str, "Rutger-41, ");
			else strcat(str, "Ragnar-!41, ");
		}
		if(strlen(str) > 11) {
			str[strlen(str) - 2] = '.';
			str[strlen(str) - 1] = 0;
		}
		else
			strcat(str, "None.");
		ANSI(fd, GREEN);
			print(fd, "\n%s\n", str);

	    strcpy(str, "  ");
	    if(F_ISSET(crt_ptr, PBLESS)) strcat(str, "bless, ");
	    if(F_ISSET(crt_ptr, PBLIND)) strcat(str, "blind, ");
	    if(F_ISSET(crt_ptr, PBRWAT)) strcat(str, "breathe-water, ");
	    if(F_ISSET(crt_ptr, PDINVI)) strcat(str, "detect-invisible, ");
	    if(F_ISSET(crt_ptr, PDMAGI)) strcat(str, "detect-magic, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PSSHLD)) strcat(str, "earth-shield, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PEVEYE)) strcat(str, "evil-eye, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PFLYSP)) strcat(str, "fly, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PHEXED)) strcat(str, "hex, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PINVIS)) strcat(str, "invisibility, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PKNOWA)) strcat(str, "know-aura, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PLEVIT)) strcat(str, "levitation, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PLIGHT)) strcat(str, "light, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PPROTE)) strcat(str, "protection, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PRFLCT)) strcat(str, "reflect, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PRCOLD)) strcat(str, "resist-cold, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PRFIRE)) strcat(str, "resist-fire, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PRACID)) strcat(str, "resist-acid, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PRMAGI)) strcat(str, "resist-magic, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
	    if(F_ISSET(crt_ptr, PSHRNK)) strcat(str, "shrink, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
/*
	    if(F_ISSET(crt_ptr, PENTNG)) strcat(str, "entangle, ");
		if(strlen(str)/(l * 57) == 1) {
			strcat(str, "\n               ");
			l++;
		}
*/
	    if(F_ISSET(crt_ptr, PSTRNG))
			strcat(str, "super-strength, ");
	    if(strlen(str) == 15)
			strcat(str, "None.");
	    else {
			str[strlen(str) - 2] = '.';
			str[strlen(str) - 1] = 0;
	    }
	    ANSI(fd, GREEN);
	    print(fd, "\nBroadcasts: ");
    	    print(fd, "%d/%d", crt_ptr->daily[DL_BROAD].cur, crt_ptr->daily[DL_BROAD].max);

	    print(fd, "\nWeight: ");
    	    print(fd, "%d/%d", weight_ply(crt_ptr), max_weight(crt_ptr));

	    print(fd, "\nSpells under:");
	    ANSI(fd, YELLOW);
    	    print(fd, "%s\n", str);
	    ANSI(fd, WHITE);

	}

/*======================================================================*/
/*		Display stats for monster				*/


	if(crt_ptr->type == !PLAYER) {

		n = find_crt_num(crt_ptr);
		ANSI(fd, YELLOW);
		rc(fd, 1, 1);
		print(fd, "Name: "); 
		print(fd, "%s     ", crt_ptr->name);

		ANSI(fd, GREEN);
		rc(fd, 1, 52);
		print(fd, "Level: "); 
		ANSI(fd, YELLOW);
		print(fd, "%d", crt_ptr->level);

		ANSI(fd, GREEN);
		rc(fd, 1, 64);
		print(fd, "No. ");
		ANSI(fd, YELLOW);
		print(fd, "%d", n);

		ANSI(fd, GREEN);
		rc(fd, 2, 1);
	        print(fd, "Keys: ");
		ANSI(fd, WHITE);
	        print(fd, "%s %+20s %+20s",
			crt_ptr->key[0], crt_ptr->key[1], crt_ptr->key[2]);

		ANSI(fd, GREEN);
		rc(fd, 3, 1);
		print(fd, "Desc:  ");
		ANSI(fd, WHITE);
		rc(fd, 4, 1);
		print(fd, "%s", crt_ptr->description);

		ANSI(fd, GREEN);
		rc(fd, 5, 1);
		print(fd, "Talk: ");
		ANSI(fd, WHITE);
		rc(fd, 6, 1);
		print(fd, "%s", crt_ptr->talk);

		ANSI(fd, BLUE);
		rc(fd, 7, 1);
		print(fd, "------------------------------------------"
				  "-----------------------------------");  

		ANSI(fd, GREEN);
		rc(fd, 8, 1);
	        print(fd, "Carry: ");
		ANSI(fd, WHITE);
		for(i = 0; i < 10; i++) {
			print(fd, "%3d ", crt_ptr->carry[i]);
		}

	ANSI(fd, BLUE);
	rc(fd, 9, 1);
	print(fd, "------------------------------------------"
			  "-----------------------------------");  

	ANSI(fd, GREEN);
	rc(fd, 10, 1);
	print(fd, "Class: ");
	ANSI(fd, WHITE);
	print(fd, "%s", class_str[crt_ptr->class]);

	ANSI(fd, GREEN);
	rc(fd, 10, 23);
	print(fd, "Race: "); 
	ANSI(fd, WHITE);
	print(fd, "%s", race_str[crt_ptr->race]); 

	ANSI(fd, GREEN);
	rc(fd, 10, 44);
	print(fd, "Sex: "); 
	ANSI(fd, WHITE);
	if(F_ISSET(crt_ptr, MNOGEN))
		print(fd, "Neutral");
	else
	print(fd, "%s", F_ISSET(crt_ptr, PMALES) ? "Male" : "Female"); 

	ANSI(fd, GREEN);
	rc(fd, 10, 59);
	print(fd, "Special: ");
	ANSI(fd, WHITE);
	print(fd, "%d", crt_ptr->special);

	ANSI(fd, GREEN);
	rc(fd, 11, 1);
	print(fd, "HP: "); 
	ANSI(fd, WHITE);
	print(fd, "%d/%d", crt_ptr->hpcur, crt_ptr->hpmax);

	ANSI(fd, GREEN);
	rc(fd, 11, 25);
	print(fd, "AC: ");
	ANSI(fd, WHITE);
	print(fd, "%d", crt_ptr->armor);

	ANSI(fd, GREEN);
	rc(fd, 11, 44);
	print(fd, "Hit: ");
	ANSI(fd, WHITE);
	print(fd, "%dd%d+%d", crt_ptr->ndice, crt_ptr->sdice, crt_ptr->pdice);

	ANSI(fd, GREEN);
	rc(fd, 11, 61);
	print(fd, "THAC0: ");
	ANSI(fd, WHITE);
	print(fd, "%d", crt_ptr->thaco);

	ANSI(fd, GREEN);
	rc(fd, 12, 1);
	print(fd, "MP: "); 
	ANSI(fd, WHITE);
	print(fd, "%d/%d", crt_ptr->mpcur, crt_ptr->mpmax);

	ANSI(fd, GREEN);
	rc(fd, 12, 22);
	print(fd, "Align: ");
	ANSI(fd, WHITE);
	print(fd, "%d", crt_ptr->alignment);

	ANSI(fd, GREEN);
	rc(fd, 12, 42);
	print(fd, "Exper: ");
	ANSI(fd, WHITE);
	print(fd, "%d", crt_ptr->experience);

	ANSI(fd, GREEN);
	rc(fd, 12, 62);
	print(fd, "Gold: ");
	ANSI(fd, WHITE);
	print(fd, "%d", crt_ptr->gold);

	ANSI(fd, BLUE);
	rc(fd, 13, 1);
	print(fd, "------------------------------------------"
			  "-----------------------------------");  

	total_stats = crt_ptr->strength + crt_ptr->dexterity + 
		crt_ptr->constitution + crt_ptr->intelligence +  crt_ptr->piety;

	ANSI(fd, GREEN);
	rc(fd, 14, 1);
	print(fd, "Strength     Dexterity     Constitution     Intelligence     "
				"Piety     Total");  
	rc(fd, 15, 1);
	ANSI(fd, WHITE);
	print(fd, "   %2d           %2d              %2d              %2d            %2d        %2d\n",  
		crt_ptr->strength, 
		    crt_ptr->dexterity, 
		        crt_ptr->constitution, 
		           crt_ptr->intelligence, 
		              crt_ptr->piety, 
						total_stats);

	ANSI(fd, BLUE);
	rc(fd, 16, 1);
	print(fd, "------------------------------------------"
			  "-----------------------------------");  

	if(F_ISSET(crt_ptr, MTRADE)) {
		ANSI(fd, GREEN);
		print(fd, "\nAccept/Give in trade:");
		ANSI(fd, WHITE);
		strcpy(str, " ");
		for(i = 0; i < 5; i++) {
			sprintf(temp, "%3d ", crt_ptr->carry[i]);
			strcat(str, temp);
		}
		strcat(str, "\n                      ");
		for(i = 5; i < 10; i++) {
			sprintf(temp, "%3d ", crt_ptr->carry[i]);
			strcat(str, temp);
		}
		print(fd, "%s\n", str);
	}
	if(F_ISSET(crt_ptr, MPURIT)) {
		ANSI(fd, GREEN);
		print(fd, "\nSelection for purchase:  ");
		ANSI(fd, WHITE);
		for(i = 0; i < 10; i++) {
			print(fd, "%3d ", crt_ptr->carry[i]);
		}
	}

	strcpy(str,"Flags set:  ");

		if(F_ISSET(crt_ptr, MPERMT)) strcat(str, "Perm-1, ");
		if(F_ISSET(crt_ptr, MHIDDN)) strcat(str, "Hiddn-2, ");
		if(F_ISSET(crt_ptr, MINVIS)) strcat(str, "Invis-3, ");
		if(F_ISSET(crt_ptr, MTOMEN)) strcat(str, "MantoMen-4, ");
		if(F_ISSET(crt_ptr, MDROPS)) strcat(str, "NoPlural-5, ");
		if(F_ISSET(crt_ptr, MNOPRE)) strcat(str, "NoPrefix-6, ");
		if(F_ISSET(crt_ptr, MAGGRE)) strcat(str, "Aggr-7, ");
		if(F_ISSET(crt_ptr, MGUARD)) strcat(str, "Guard-8, ");
		if(F_ISSET(crt_ptr, MBLOCK)) strcat(str, "Block-9, ");
		if(F_ISSET(crt_ptr, MFOLLO)) strcat(str, "Follow-10, ");
		if(F_ISSET(crt_ptr, MFLEER)) strcat(str, "Flee-11, ");
		if(F_ISSET(crt_ptr, MSCAVE)) strcat(str, "Scav-12, ");
		if(F_ISSET(crt_ptr, MMALES)) strcat(str, "Male-13, ");
		if(F_ISSET(crt_ptr, MPOISS)) strcat(str, "Poisoner-14, ");
		if(F_ISSET(crt_ptr, MUNDED)) strcat(str, "Undead-15, ");
		if(F_ISSET(crt_ptr, MUNSTL)) strcat(str, "NoSteal-16, ");
		if(F_ISSET(crt_ptr, MPOISN)) strcat(str, "Poisoned-17, ");
		if(F_ISSET(crt_ptr, MMAGIC)) strcat(str, "CastMagic-18, ");
		if(F_ISSET(crt_ptr, MHASSC)) strcat(str, "Scavenged-19, ");
		if(F_ISSET(crt_ptr, MMGONL)) strcat(str, "MagicOnly-21, ");
		if(F_ISSET(crt_ptr, MDINVI)) strcat(str, "DetInvis-22, ");
		if(F_ISSET(crt_ptr, MENONL)) strcat(str, "Mag/Enchnt-23, ");
		if(F_ISSET(crt_ptr, MTALKS)) strcat(str, "Talks-24, ");
		if(F_ISSET(crt_ptr, MUNKIL)) strcat(str, "UnKill-25, ");
		if(F_ISSET(crt_ptr, MNRGLD)) strcat(str, "FixedGold-26, ");
		if(F_ISSET(crt_ptr, MTLKAG)) strcat(str, "TalkAggr-27, ");
		if(F_ISSET(crt_ptr, MRMAGI)) strcat(str, "ResMagic-28, ");
		if(F_ISSET(crt_ptr, MENEDR)) strcat(str, "ExpDrain-31, ");
		if(F_ISSET(crt_ptr, MPLDGK)) strcat(str, "CanPledg-33, ");
		if(F_ISSET(crt_ptr, MRSCND)) strcat(str, "CanRescind-34 ");
		if(F_ISSET(crt_ptr, MDISEA)) strcat(str, "Disease-35, ");
		if(F_ISSET(crt_ptr, MDISIT)) strcat(str, "Dissolve-36, ");
		if(F_ISSET(crt_ptr, MPURIT)) strcat(str, "Purchase-37, ");
		if(F_ISSET(crt_ptr, MTRADE)) strcat(str, "Trade-38, ");
		if(F_ISSET(crt_ptr, MPGUAR)) strcat(str, "PasvGrd-39, ");
		if(F_ISSET(crt_ptr, MGAGGR)) strcat(str, "GoodAggr-40, ");
		if(F_ISSET(crt_ptr, MEAGGR)) strcat(str, "EvilAggr-41, ");
		if(F_ISSET(crt_ptr, MDEATH)) strcat(str, "DeathScene-42, ");
		if(F_ISSET(crt_ptr, MMAGIO)) strcat(str, "CastPercnt-43, ");
		if(F_ISSET(crt_ptr, MRBEFD)) strcat(str, "ResistStun-44, ");
		if(F_ISSET(crt_ptr, MNOCIR)) strcat(str, "NoCircle-45, ");
		if(F_ISSET(crt_ptr, MBLNDR)) strcat(str, "Blinder-46, ");
		if(F_ISSET(crt_ptr, MDMFOL)) strcat(str, "Possessed-47, ");
		if(F_ISSET(crt_ptr, MFEARS)) strcat(str, "Fearful-48, ");
		if(F_ISSET(crt_ptr, MSILNC)) strcat(str, "Mute-49, ");
		if(F_ISSET(crt_ptr, MBLIND)) strcat(str, "Blind-50, ");
		if(F_ISSET(crt_ptr, MCHARM)) strcat(str, "Charmed-51, ");
		if(F_ISSET(crt_ptr, MMOBIL)) strcat(str, "Mobile-52, ");
		if(F_ISSET(crt_ptr, MROBOT)) strcat(str, "Logic-53, ");
		if(F_ISSET(crt_ptr, MIREGP)) strcat(str, "IrregPlrl-54, ");
		if(F_ISSET(crt_ptr, MSHEAL)) strcat(str, "SloHeal-55, ");
		if(F_ISSET(crt_ptr, MEVEYE)) strcat(str, "EvilEye-56, ");
		if(F_ISSET(crt_ptr, MAKGDM)) strcat(str, "PldgAggro-57, ");
		if(F_ISSET(crt_ptr, MFEARD)) strcat(str, "Feared-58, ");
		if(F_ISSET(crt_ptr, MPICKP)) strcat(str, "PickP-59, ");
		if(F_ISSET(crt_ptr, MFROZE)) strcat(str, "Frozen-60, ");
		if(F_ISSET(crt_ptr, MPLEDG)) strcat(str, "Pledged-61, ");
		if(F_ISSET(crt_ptr, MHEXED)) strcat(str, "Hexed-62, ");
		if(F_ISSET(crt_ptr, MCNOAG)) strcat(str, "SameClassNotAgg-63, ");
		if(F_ISSET(crt_ptr, MTALKR)) strcat(str, "RangerTalk-64, ");
		if(F_ISSET(crt_ptr, MTALKD)) strcat(str, "DruidTalk-65, ");
		if(F_ISSET(crt_ptr, MPBRIB)) strcat(str, "PermBribe-66, ");
		if(F_ISSET(crt_ptr, MNOBRB)) strcat(str, "NoBribe-67, ");
		if(F_ISSET(crt_ptr, MTIPOK)) strcat(str, "AcceptTip-68, ");
		if(F_ISSET(crt_ptr, MNOGEN)) strcat(str, "NoGender-69, ");
		if(F_ISSET(crt_ptr, MPASSW)) strcat(str, "Passwd-70, ");
		if(F_ISSET(crt_ptr, MCSAGG)) strcat(str, "SameClassAgg-71, ");
		if(F_ISSET(crt_ptr, MNPLEA)) strcat(str, "NoPlea-72, ");
		if(F_ISSET(crt_ptr, MGDOBJ)) strcat(str, "GuardObj-73, ");
		if(F_ISSET(crt_ptr, MNOEXT)) strcat(str, "No_Rnd_Exit-74, ");
		if(F_ISSET(crt_ptr, MPORTR)) strcat(str, "Porter-75, ");
		if(F_ISSET(crt_ptr, MGATEK)) strcat(str, "GateKeeper-76, ");
		if(F_ISSET(crt_ptr, MPLEDG) ) 
		    if(F_ISSET(crt_ptr, MKNGDM)) strcat(str, "Rutger-32, ");
			else strcat(str, "Ragnar-!32, ");
		if(F_ISSET(crt_ptr, MBRETH))
			if(!F_ISSET(crt_ptr, MBRWP1) && !F_ISSET(crt_ptr, MBRWP2))
				strcat(str, "BR-fire-!29&!30, ");
			else if(F_ISSET(crt_ptr, MBRWP1) && !F_ISSET(crt_ptr, MBRWP2))
				strcat(str, "BR-acid-29, ");
			else if(!F_ISSET(crt_ptr, MBRWP1) && F_ISSET(crt_ptr, MBRWP2))
				strcat(str, "BR-frost-30, ");
			else
				strcat(str, "BR-gas-29&30, ");

	if(strlen(str) > 11) {
		str[strlen(str) - 2] = '.';
		str[strlen(str) - 1] = 0;
	}
	else
		strcat(str, "None.");
	ANSI(fd, GREEN);
	print(fd, "\n%s\n", str);

	    strcpy(str, "\n  ");
	    for(i = 0, j = 0; i < 128; i++)
			if(S_ISSET(crt_ptr, i))
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
	    ANSI(fd, CYAN);
	    print(fd, "Spells known:");
	    ANSI(fd, YELLOW);
	    print(fd, "%s\n", str);
	    ANSI(fd, BLUEBG);
	    ANSI(fd, BOLD);
	    ANSI(fd, WHITE);

	}	
}	

/************************************************************************/
/*			stat_obj					*/
/************************************************************************/
/*  Display information on object given to player given.		*/

int stat_obj(ply_ptr, obj_ptr)
creature	*ply_ptr;
object		*obj_ptr;

{
	char	str[1024];
	int	n,	fd;

	fd = ply_ptr->fd;

	n = find_obj_num(obj_ptr);

	ANSI(fd, YELLOW);
	print(fd, " Name: %s	No. %d\n", obj_ptr->name, n);
	print(fd, " Desc: ");
	ANSI(fd, WHITE);
	print(fd, "%s\n", obj_ptr->description);
	ANSI(fd, YELLOW);
	print(fd, "  Use:  %s\n", obj_ptr->use_output);
	print(fd, " Keys: %s %+20s %+20s\n\n",obj_ptr->key[0],
		obj_ptr->key[1], obj_ptr->key[2]);
	print(fd, "  Hit: %dd%d + %d", obj_ptr->ndice, obj_ptr->sdice,
		obj_ptr->pdice);

	if(obj_ptr->adjustment)
		print(fd, " (+%d)\n", obj_ptr->adjustment);
	else
		print(fd, "\n");

	print(fd, "Shots: %d/%d\n", obj_ptr->shotscur, obj_ptr->shotsmax);

	if(obj_ptr->type == 6 || obj_ptr->type == 7 || obj_ptr->type == 8)
		print(fd, "Magic: %d = %s\n", obj_ptr->magicpower,
		    spllist[obj_ptr->magicpower-1].splstr); 

	if(obj_ptr->special)
		print(fd, " Spec: %d\n", obj_ptr->special);

	print(fd, " Type: ");
	if(obj_ptr->type <= MISSILE) {
		switch(obj_ptr->type) {
			case SHARP: print(fd, "0 (sharp"); break;
			case THRUST: print(fd, "1 (thrusting"); break;
			case BLUNT: print(fd, "2 (blunt"); break;
			case POLE: print(fd, "3 (pole"); break;
			case MISSILE: print(fd, "4 (missile"); break;
		}
		print(fd, " weapon.)");
	}
	else if(obj_ptr->type >= ARMOR) {
		switch(obj_ptr->type) {
			case 5: print(fd, "5 (armor)"); break;
			case 6: print(fd, "6 (potion)"); break;
			case 7: print(fd, "7 (scroll)"); break;
			case 8: print(fd, "8 (wand)"); break;
			case 9: print(fd, "9 (container)"); break;
			case 10: print(fd, "10 (money)"); break;
			case 11: print(fd, "11 (key)"); break;
			case 12: print(fd, "12 (light)"); break;
			case 13: print(fd, "13 (misc)"); break;
			case 14: print(fd, "14 (repair)"); break;
		}
	}

	print(fd, "   Value: %5.5d", obj_ptr->value);
	print(fd, "   Weight: %2.2d", obj_ptr->weight);

	if(obj_ptr->type != ARMOR) {
		print(fd, "   Wear: ");
		switch(obj_ptr->wearflag) {
			case 1:  print(fd, "body-1");		break;
			case 2:  print(fd, "arms-2");		break;
			case 3:  print(fd, "legs-3");		break;
			case 4:  print(fd, "neck-4");		break;
			case 6:  print(fd, "hands-6");	 	break;
			case 7:  print(fd, "head-7");		break;
			case 8:  print(fd, "feet-8");		break;
			case 9:  print(fd, "finger-9");		break;
			case 17: print(fd, "held-17");		break;
			case 18: print(fd, "shield-18");	break;
			case 19: print(fd, "face-19");		break;
			case 20: print(fd, "wield-20");		break;
			default: print(fd, "not set");		break;
		}
	}

	if((obj_ptr->type <= 4) && (obj_ptr->wearflag != 20)) 
		print(fd, "   Wear: ERROR");

	if(obj_ptr->type == ARMOR) {
		print(fd, "   AC: %2.2d", obj_ptr->armor);
		print(fd, "   Wear: ");
		switch(obj_ptr->wearflag) {
			case 1:  print(fd, "body-1");		break;
			case 2:  print(fd, "arms-2");		break;
			case 3:  print(fd, "legs-3");		break;
			case 4:  print(fd, "neck-4");		break;
			case 6:  print(fd, "hands-6");		break;
			case 7:  print(fd, "head-7");		break;
			case 8:  print(fd, "feet-8");		break;
			case 9:  print(fd, "finger-9");		break;
			case 17: print(fd, "held-17");		break;
			case 18: print(fd, "shield-18");	break;
			case 19: print(fd, "face-19");		break;
			case 20: print(fd, "wield-20");		break;
			default: print(fd, "not set");		break;
		}
	}
	else if(obj_ptr->armor) {
		print(fd, "   AC: %2.2d", obj_ptr->armor);
	}

	if(obj_ptr->questnum)
		print(fd, "   Quest: %d\n", obj_ptr->questnum);
	else
		print(fd, "\n");

	ANSI(fd, BLUE);
	print(fd, "------------------------------------------"
			  "-----------------------------------");  
	print(fd, "\n");
	ANSI(fd, YELLOW);
	
	strcpy(str, "Flags set: ");
	if(F_ISSET(obj_ptr, OPERMT)) strcat(str, "Perm-1, ");
	if(F_ISSET(obj_ptr, OHIDDN)) strcat(str, "Hidden-2, ");
	if(F_ISSET(obj_ptr, OINVIS)) strcat(str, "Invis-3, ");
	if(F_ISSET(obj_ptr, OSOMEA)) strcat(str, "SomePrefix-4, ");
	if(F_ISSET(obj_ptr, ODROPS)) strcat(str, "DropS-5, ");
	if(F_ISSET(obj_ptr, ONOPRE)) strcat(str, "NoPrefix-6, ");
	if(F_ISSET(obj_ptr, OCONTN)) strcat(str, "Container-7, ");
	if(F_ISSET(obj_ptr, OWTLES)) strcat(str, "Wtless-8, ");
	if(F_ISSET(obj_ptr, OTEMPP)) strcat(str, "TempPerm-9, ");
	if(F_ISSET(obj_ptr, OPERM2)) strcat(str, "PermInv-10, ");
	if(F_ISSET(obj_ptr, ONOMAG)) strcat(str, "Nomage-11, ");
	if(F_ISSET(obj_ptr, OLIGHT)) strcat(str, "Light-12, ");
	if(F_ISSET(obj_ptr, OGOODO)) strcat(str, "Good-13, ");
	if(F_ISSET(obj_ptr, OEVILO)) strcat(str, "Evil-14, ");
	if(F_ISSET(obj_ptr, OENCHA)) strcat(str, "Ench-15, ");
	if(F_ISSET(obj_ptr, ONOFIX)) strcat(str, "Nofix-16, ");
	if(F_ISSET(obj_ptr, OCLIMB)) strcat(str, "Clmbing-17, ");
	if(F_ISSET(obj_ptr, ONOTAK)) strcat(str, "Notake-18, ");
	if(F_ISSET(obj_ptr, OSCENE)) strcat(str, "Scenery-19, ");
	if(F_ISSET(obj_ptr, OSIZE1) || F_ISSET(obj_ptr, OSIZE2))
		strcat(str, "Sized: ");
	if(F_ISSET(obj_ptr, OSIZE1) && !F_ISSET(obj_ptr, OSIZE2))
		strcat(str, "Medium-20, ");
	if(!F_ISSET(obj_ptr, OSIZE1) && F_ISSET(obj_ptr, OSIZE2))
		strcat(str, "Small-21, ");
	if(F_ISSET(obj_ptr, OSIZE1) && F_ISSET(obj_ptr, OSIZE2))
		strcat(str, "Large-20+21, ");
	if(F_ISSET(obj_ptr, ORENCH)) strcat(str, "RandEnch-22, ");
	if(F_ISSET(obj_ptr, OCURSE)) strcat(str, "Cursed-23, ");
	if(F_ISSET(obj_ptr, OWEARS)) strcat(str, "Worn-24, ");
	if(F_ISSET(obj_ptr, OUSEFL)) strcat(str, "Use-floor-25, ");
	if(F_ISSET(obj_ptr, OCNDES)) strcat(str, "Devours-26, ");
	if(F_ISSET(obj_ptr, ONOMAL)) strcat(str, "Nomale-27, ");
	if(F_ISSET(obj_ptr, ONOFEM)) strcat(str, "Nofemale-28, ");
	if(F_ISSET(obj_ptr, ODDICE)) strcat(str, "NdS damage-29, ");
	if(F_ISSET(obj_ptr, OPLDGK)) strcat(str, "PldgOnly-30, ");
	if(F_ISSET(obj_ptr, OPLDGK))
		if(F_ISSET(obj_ptr, OKNGDM)) strcat(str, "Rut-31, ");
		else  strcat(str, "Rag-!31, ");
	if(F_ISSET(obj_ptr, ONSHAT)) strcat(str, "Shatterproof-45, ");
	if(F_ISSET(obj_ptr, OALCRT)) strcat(str, "AlwaysCrit-46, ");
	if(F_ISSET(obj_ptr, OLUCKY)) strcat(str, "Lucky-47, ");
	if(F_ISSET(obj_ptr, OIREGP)) strcat(str, "IregPlural-48, ");
	if(F_ISSET(obj_ptr, OCURSW)) strcat(str, "CurseWorn-49, ");
	if(F_ISSET(obj_ptr, OTMPEN)) strcat(str, "TempEnch-50, ");
	if(F_ISSET(obj_ptr, OSILVR)) strcat(str, "Silv-51, ");
	if(F_ISSET(obj_ptr, ONUSED)) strcat(str, "NeverBreak-52, ");
	if(F_ISSET(obj_ptr, ONOLAW)) strcat(str, "NoLawful-53, ");
	if(F_ISSET(obj_ptr, OFASTS)) strcat(str, "Fastenable-54, ");
	if(F_ISSET(obj_ptr, OFASTD)) strcat(str, "Fastened-55, ");
	if(F_ISSET(obj_ptr, ONJIMY)) strcat(str, "NoJimmy-56, ");
	if(F_ISSET(obj_ptr, OGUARD)) strcat(str, "Guarded-57, ");
	if(F_ISSET(obj_ptr, ONTFIX)) strcat(str, "NoToolFix-58, ");
	if(F_ISSET(obj_ptr, OCLOAK)) strcat(str, "Cloak-59, ");
	if(F_ISSET(obj_ptr, OCLSEL)) {
		strcat(str, "ClsSel-32: ");
		if (F_ISSET(obj_ptr, OASSNO)) strcat(str, "Assassin-33, ");
		if (F_ISSET(obj_ptr, OALCHO)) strcat(str, "Alchemist-44, ");
		if (F_ISSET(obj_ptr, OBARBO)) strcat(str, "Barbarian-34, ");
		if (F_ISSET(obj_ptr, OBARDO)) strcat(str, "Bard-41, ");
		if (F_ISSET(obj_ptr, OCLERO)) strcat(str, "Cleric-35, ");
		if (F_ISSET(obj_ptr, ODRUDO)) strcat(str, "Druid-43, ");
		if (F_ISSET(obj_ptr, OFIGHO)) strcat(str, "Fighter-36, ");
		if (F_ISSET(obj_ptr, OMAGEO)) strcat(str, "Mage-37, ");
		if (F_ISSET(obj_ptr, OMONKO)) strcat(str, "Monk-42, ");
		if (F_ISSET(obj_ptr, OPALAO)) strcat(str, "Paladin-38, ");
		if (F_ISSET(obj_ptr, ORNGRO)) strcat(str, "Ranger-39, ");
		if (F_ISSET(obj_ptr, OTHIEO)) strcat(str, "Thief-40, ");
		if(obj_ptr->magicpower) 
			strcat(str, "Magic, ");	
	}

	if(strlen(str) > 11) {
		str[strlen(str) - 2] = '.';
		str[strlen(str) - 1] = 0;
	}
	else
		strcat(str, "None.");
	print(fd, "%s\n", str);
	ANSI(fd, BLUEBG);
	ANSI(fd, BOLD);
	ANSI(fd, WHITE);
}

/**********************************************************************/
/*			dm_add_rom				      */
/**********************************************************************/
/* This function allows a DM to add a new, empty room to the current  */
/* database of rooms.						  */

int dm_add_rom(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	room	*new_rom;
	char	file[80];
	int	fd, ff;

	fd = ply_ptr->fd;

	if(ply_ptr->class < CARETAKER)
		return(PROMPT);

	if(cmnd->val[1] < 2) {
		print(fd, "Add room number?\n");
		return(0);
	}

	sprintf(file, "%s/r%05d", ROOMPATH, cmnd->val[1]);
	ff = open(file, O_RDONLY, 0);
	if(ff >= 0) {
		close(ff);
		print(fd, "Room already exists.\n");
		return(0);
	}

	new_rom = (room *)malloc(sizeof(room));
	if(!new_rom)
		merror("dm_add_room", FATAL);
	new_rom->rom_num = cmnd->val[1];
	sprintf(new_rom->name, "Room #%d", cmnd->val[1]);

	zero(new_rom, sizeof(room));

	ff = open(file, O_RDWR | O_CREAT | O_BINARY, ACC);
	if(ff < 0) {
		print(fd, "Error: Unable to open file.\n");
		return(0);
	}

	if(write_rom(ff, new_rom, 0) < 0) {
		print(fd, "Write failed.\n");
		return(0);
	}

	close(ff);
	free(new_rom);
	print(fd, "Room #%d created.\n", cmnd->val[1]);
	return(0);
}

/***********************************************************************/
/*			dm_spy					       */
/***********************************************************************/
/* 	This function allows a DM to observe a player's screen.	       */

int dm_spy(ply_ptr, cmnd)
creature 	*ply_ptr;
cmd		*cmnd;

{
	int 		fd, i;
	creature	*crt_ptr;

	if(ply_ptr->class < CARETAKER)
		return(PROMPT);

	fd = ply_ptr->fd;

	if(cmnd->num < 2 && !F_ISSET(ply_ptr, PSPYON)) {
		print(fd, "Spy on whom?\n");
		return(0);
	}
	
	if(F_ISSET(ply_ptr, PSPYON)) {
		for(i = 0; i < Tablesize; i++)
			if(Spy[i] == fd) {
				Spy[i] = -1;
				crt_ptr = Ply[i].ply;
				if(strcmp(ply_ptr->name, DMNAME)) {
					if(crt_ptr) {
						if(ply_ptr->class <= crt_ptr->class) {
							ANSI(crt_ptr->fd, YELLOW);
							print(crt_ptr->fd, 
								"%s has stopped observing you.\n", 
									ply_ptr->name);
							ANSI(crt_ptr->fd, WHITE);
						}
					}
				}
			}
		F_CLR(ply_ptr, PSPYON);
		print(fd, "Spy mode off.\n");
		return(0);
	}

	cmnd->str[1][0] = up(cmnd->str[1][0]);
	crt_ptr = find_who(cmnd->str[1]);
	if(!crt_ptr) {
		print(fd, "Spy on whom?  Use full names.\n");
		return(0);
	}
	if(!strcmp(crt_ptr->name, DMNAME)) {
		if(F_ISSET(crt_ptr, PDMINV)) {
			print(fd, "Spy on whom?  Use full names.\n");
			return(0);
		}
	}
	

	if(Spy[crt_ptr->fd] > -1) {
		print(fd, "That person is being spied on already.\n");
		return(0);
	}

	if(strcmp(ply_ptr->name, DMNAME)) {
   		if(crt_ptr->class >= ply_ptr->class) {
	    	print(fd, "%s notes your action.\n", crt_ptr->name);
	    	ANSI(crt_ptr->fd, YELLOW);
	    	print(crt_ptr->fd, "%s is observing you.\n", ply_ptr->name);
	    	ANSI(crt_ptr->fd, WHITE);
			output_buf();
		}
	}

	Spy[crt_ptr->fd] = ply_ptr->fd; 
	F_SET(ply_ptr, PSPYON);
	F_SET(ply_ptr, PDMINV);
	print(fd, "Spy on.  Type *spy to turn it off.\n");
	return(0);
}

/************************************************************************/
/*			dm_password					*/
/************************************************************************/
/*  This function will allow only the senior DM to display or change	*/
/*  the password of a player.						*/

int dm_password(ply_ptr, cmnd)
creature	*ply_ptr;
cmd		*cmnd;

{
	creature	*crt_ptr;
	creature	*ply_ptr2;
	int		fd;

	fd = ply_ptr->fd;

	if(ply_ptr->class < DM)
		return(PROMPT);

	if(cmnd->num != 3) {
		if(cmnd->num < 2)
			ply_ptr2 = ply_ptr;

		if(ply_ptr2 != ply_ptr && strcmp(ply_ptr->name, DMNAME)) {
			print(fd, "Only %s has access to passwords.\n", DMNAME);
			return(PROMPT);
		}

		if(ply_ptr2 != ply_ptr) {
			cmnd->str[1][0] = up(cmnd->str[1][0]);
			ply_ptr2 = find_crt(ply_ptr, ply_ptr->parent_rom->first_mon,
			    cmnd->str[1], cmnd->val[1]);
			if(!ply_ptr2)
				ply_ptr2 = find_crt(ply_ptr, ply_ptr->parent_rom->first_ply,
			    	cmnd->str[1], cmnd->val[1]);
			if(!ply_ptr2)
				ply_ptr2 = find_who(cmnd->str[1]);
			if(!ply_ptr2) {
				print(fd, "%s not found.\n", cmnd->str[1]);
				return(PROMPT);
			}
		}

		print(fd, "Password for %s:   %s\n", ply_ptr2->name, 
			ply_ptr2->password);
	}
	else if(cmnd->num == 3 && !strcmp(ply_ptr->name, DMNAME)) {
		cmnd->str[1][0] = up(cmnd->str[1][0]);
		ply_ptr2 = find_crt(ply_ptr, ply_ptr->parent_rom->first_mon,
		    cmnd->str[1], cmnd->val[1]);
		if(!ply_ptr2)
			ply_ptr2 = find_crt(ply_ptr, ply_ptr->parent_rom->first_ply,
		    	cmnd->str[1], cmnd->val[1]);
		if(!ply_ptr2)
			ply_ptr2 = find_who(cmnd->str[1]);
		if(!ply_ptr2) {
			print(fd, "%s not found.\n", cmnd->str[1]);
			return(PROMPT);
		}
		if(ply_ptr2) {
			strcpy(ply_ptr2->password, cmnd->str[2]);
			print(fd, "Password for %s changed to:   %s\n", 
				ply_ptr2->name, ply_ptr2->password);
		}
	}
	return(PROMPT);
}
