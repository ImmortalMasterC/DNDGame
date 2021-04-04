/*
 * ACTION.C:
 *
 *      This file contains the routines necessary to achieve action
 *      commands.
 *
 *      Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#include "mextern.h"
#include "string.h"

/**********************************************************************/
/*                              action                                */
/**********************************************************************/
/* This function allows a player to commit an action command.         */

#define OUT(a,b) \
	print(fd, a); \
	broadcast_rom(fd, ply_ptr->rom_num, b, ply_ptr);

#define OUT2(a,b) \
	print(fd, a); \
	broadcast_rom(fd, ply_ptr->rom_num, b, ply_ptr, \
		F_ISSET(ply_ptr, PMALES) ? "his":"her");

#define OUT3(a,b) \
	print(fd, a); \
	broadcast_rom(fd, ply_ptr->rom_num, b, ply_ptr, \
		F_ISSET(ply_ptr, PMALES) ? "he":"she");

#define OUT4(a,b,c) \
	print(fd, a, crt_ptr); \
	print(crt_ptr->fd, b, ply_ptr); \
	broadcast_rom2(fd, crt_ptr->fd, rom_ptr->rom_num, c, ply_ptr, crt_ptr);

#define OUT5(a,b) \
	print(fd, a); \
	broadcast_rom(fd, ply_ptr->rom_num, b, ply_ptr, \
		F_ISSET(ply_ptr, PMALES) ? "him":"her");

#define OUT6(a,b) \
	print(fd, a); \
	broadcast_rom(fd, ply_ptr->rom_num, b, ply_ptr, \
		F_ISSET(ply_ptr, PMALES) ? "himself":"herself");

#define OUT7(a,b) \
	print(fd, a, obj_ptr); \
	broadcast_rom(fd, ply_ptr->rom_num, b, ply_ptr, obj_ptr);

#define OUT8(a,b) \
	print(fd, a, ext_ptr); \
	broadcast_rom(fd, ply_ptr->rom_num, b, ply_ptr, ext_ptr);

#define OUT9(a,b,c) \
	print(fd, a, crt_ptr); \
	print(crt_ptr->fd, b, ply_ptr, F_ISSET(ply_ptr, PMALES) ? "his":"her"); \
	broadcast_rom2(fd, crt_ptr->fd, rom_ptr->rom_num, c, ply_ptr, \
		F_ISSET(ply_ptr, PMALES) ? "his":"her", crt_ptr);

int action(ply_ptr, cmnd)
creature        *ply_ptr;
cmd             *cmnd;

{
	room            *rom_ptr;
	exit_		*ext_ptr;
	creature        *crt_ptr = 0;
	object          *obj_ptr;
	int             match = 0, cmdno = 0, c = 0, n, fd, num;

	fd = ply_ptr->fd;
	rom_ptr = ply_ptr->parent_rom;

	F_CLR(ply_ptr, PHIDDN);

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

	if(cmnd->num == 2) {
		obj_ptr = find_obj(ply_ptr, rom_ptr->first_obj,
			cmnd->str[1], cmnd->val[1]);

		if(!obj_ptr) {
			ext_ptr = find_ext(ply_ptr, rom_ptr->first_ext,
				cmnd->str[1], cmnd->val[1]);
		}

		if(!obj_ptr && !ext_ptr) {
			crt_ptr = find_crt(ply_ptr, rom_ptr->first_mon,
				cmnd->str[1], cmnd->val[1]);
			if(!crt_ptr) {
				cmnd->str[1][0] = up(cmnd->str[1][0]);
				crt_ptr = find_crt(ply_ptr, rom_ptr->first_ply,
			   		cmnd->str[1], cmnd->val[1]);
			}
		}

		if(!crt_ptr && !obj_ptr && !ext_ptr || crt_ptr == ply_ptr) {
			print(fd, "That's not here.\n");
			return(0);
		}
	}

/*
	if(!strcmp(cmdlist[cmdno].cmdstr, "room#")) {
		print(fd, "This is room number %d.\n", rom_ptr->rom_num);
	}
*/
	if(!strcmp(cmdlist[cmdno].cmdstr, "eat")) {
		OUT("Your stomach rebels.\n", "%M gags.");
	}
	if(!strcmp(cmdlist[cmdno].cmdstr, "splunk")) {
		OUT("You are advanced one level.\n", "%M splunks.");
	}
	if(!strcmp(cmdlist[cmdno].cmdstr, "splonk")) {
		OUT("You are advanced two levels.\n", "%M splonks.");
	}
	if(!strcmp(cmdlist[cmdno].cmdstr, "nod")) {
		OUT("You nod.\n", "%M nods.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "blanch")) {
		OUT("Terror turns your face pale with fright.\n", 
			"%M's face turns pale with fright!");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "idea")) {
		OUT("Your eyes twinkle with a brilliant idea!\n", 
			"%M's eyes twinkle with an idea!");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "menlaugh")) {
		OUT("You laugh menacingly.\n", "%M laughs menacingly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "laugh")) {
		OUT("You shake with laughter.\n", "%M shakes with laughter.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "hyslaugh")) {
		OUT("You laugh hysterically.\n", "%M laughs hysterically.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "lol")) {
		OUT("You roll on the floor with laughter.\n", 
			"%M rolls on the floor with laughter.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "stutter")) {
		OUT("You stutter in disbelief.\n", 
			"%M stutters in complete disbelief.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "scream")) {
		OUT("You shriek in fear.\n", 
			"%M screams in fright.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "chops")) {
		OUT2("You lick your chops.\n", "%M licks %s chops with anticipation.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "hand")) {
		OUT2("You raise your hand.\n", "%M raises %s hand for attention.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "wiggle")) {
		OUT2("You wiggle your nose.\n", "%M wiggles %s nose.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "fist")) {
		OUT2("You put your fist over your head in agreement.\n", 
			"%M puts %s fist in the air in agreement.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "stretch")) {
		OUT("You stretch your body.\n", "%M stretches the kinks out.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "smile")) {
		OUT("You smile happily.\n", "%M smiles happily.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "satsmile")) {
		OUT("You smile with satisfaction.\n", "%M smiles with satisfaction.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "warmsmile")) {
		OUT("You smile warmly.\n", "%M smiles warmly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "float")) {
		if(F_ISSET(ply_ptr, PFLYSP)) {
			OUT("You float into the air and fly in a circle.\n", 
				"%M floats into the air and flys in a circle.");
		}
		else
			print(fd, "You can't fly.\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "whirl")) {
		OUT("You whirl about.\n", "%M whirls about quickly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "crouch")) {
		OUT("You crouch down.\n", "%M crouchs in a defensive posture.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "blink")) {
		OUT("You blink in disbelief.\n", "%M blinks in disbelief.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "snicker")) {
        OUT("You snicker.\n", "%M snickers.");
    }
	else if(!strcmp(cmdlist[cmdno].cmdstr, "beam")) {
		OUT("You beam happily.\n", "%M beams happily.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "relax")) {
		OUT("You breath deeply.\n", 
			"%M takes a deep breath and sighs comfortably.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "smoke")) {
		OUT("You take a puff.\n", "%M puffs on a pipe.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "puke")) {
        OUT("You blow chunks.\n", "%M vomits on the ground.");
    }
	else if(!strcmp(cmdlist[cmdno].cmdstr, "burp")) {
        OUT("You belch loudly.\n", "%M belches loudly and rudely.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "ponder")) {
        OUT("You ponder the situation.\n", "%M ponders the situation.");
    }
    else if(!strcmp(cmdlist[cmdno].cmdstr, "think")) {
        OUT("You think carefully.\n", "%M thinks carefully.");
    }
    else if(!strcmp(cmdlist[cmdno].cmdstr, "ack")) {
        OUT("You ack.\n", "%M acks.");
    }
	else if(!strcmp(cmdlist[cmdno].cmdstr, "nervous")) {
		OUT("You titter nervously.\n", "%M titters nervously.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "sleep")) {
		OUT("You take a nap.\n", "%M dozes off.");
	}
	else if((!strcmp(cmdlist[cmdno].cmdstr, "masturbate")) ||
		(!strcmp(cmdlist[cmdno].cmdstr, "masterbate"))) {
			OUT("You masturbate till it hurts.\n", 
				"%M masturbates clumsily.\n");
		if(mrand(1, 100) < 50  && !F_ISSET(ply_ptr, PBLIND)) {
			F_SET(ply_ptr, PBLIND);
		 	CLS(fd);	
		 	ANSI(fd, RED);
		 	print(fd, "You've gone blind!\n");
		 	ANSI(fd, WHITE);
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "tongue")) {
		if(crt_ptr) {
			OUT9("You stick your tongue out at %m.\n", 
				"%M sticks %s tongue out at you.\n", 
					"%M sticks %s tongue out at %m.");
		}
		else {
			OUT2("You stick out your tongue.\n", 
				"%M makes a face and sticks out %s tongue.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "curtsy")) {
		if(!F_ISSET(ply_ptr, PMALES)) {
			if(crt_ptr) {
				OUT4("You curtsy to %m.\n", 
					"%M makes a graceful curtsy to you.\n", 
						"%M makes a graceful curtsy towards %m.");
			}
			else {
				OUT("You make a graceful curtsy.\n", 
					"%M makes a graceful curtsy.");
			}
		}
		else
			print(fd, "Don't be a such a sissy.  Only females curtsy.\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "sneeze")) {
		if(crt_ptr) {
			OUT4("You sneeze in %m's face.\n", 
				"%M sneezes in your face.\n", 
					"%M sneezes in %m's face.");
		}
		else {
			OUT2("You sneeze noisily.\n", "%M closes %s eyes and sneezes.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "shake")) {
		if(crt_ptr) {
			OUT4("You shake %m's hand.\n", 
				"%M shakes your hand.\n", 
					"%M shakes %m's hand.");
		}
		else {
			OUT2("You shake your head.\n", "%M shakes %s head.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "knee")) {
		if(crt_ptr) {
			OUT4("You knee %m in the crotch.\n",
				"%M painfully knees you in the crotch.\n",
					 "%M painfully knees %m.");
			if(crt_ptr->hpcur > 3)
				crt_ptr->hpcur -= 1;
		}
		else
			print(fd, "Knee whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "beckon")) {
		if(crt_ptr) {
			OUT4("You beckon for %m to follow you.\n", 
				"%M beckons for you to follow.\n", 
					"%M beckons %m to follow.");
		}
		else {
			OUT2("You beckon for everyone to follow.\n", 
				"%M beckons for eveyone to follow %s lead.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "admire")) {
		if(crt_ptr) {
			OUT4("You admire %m.\n", 
				"%M looks at you admiringly.\n", 
					"%M gives an admiring look to %m.");
		}
		else
			print(fd, "Admire whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "pounce")) {
		if(crt_ptr) {
			OUT4("You pounce on %m.\n", 
				"%M pounces on you.\n", 
					"%M pounces on %m.");
		}
		else
			print(fd, "Pounce on whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "tickle")) {
		if(crt_ptr) {
			OUT4("You tickle %m mercilessly.\n",
				"%M tickles you mercilessly.\n", 
					"%M tickles %m mercilessly.");
		}
		else
			print(fd, "Tickle whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "kick")) {
		if(crt_ptr) {
			OUT4("You kick %m.\n", 
				"%M kicks you.\n", 
					"%M kicks %m.");
		}
		else
			print(fd, "Kick whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "tackle")) {
		if(crt_ptr) {
			OUT4("You tackle %m to the ground.\n",
				"%M tackles you to the ground.\n",
					"%M tackles %m to the ground.");
		}
		else
			print(fd, "Tackle whom?\n");
    }
	else if(!strcmp(cmdlist[cmdno].cmdstr, "frustrate")) {
            OUT2("You pull your hair out.\n",
                "%M pulls %s hair out in frustration.");
    }
	else if(!strcmp(cmdlist[cmdno].cmdstr, "tap")) {
            OUT2("You tap your foot impatiently.\n",
                "%M taps %s foot impatiently.");
    }
	else if(!strcmp(cmdlist[cmdno].cmdstr, "cheer")) {
            if(crt_ptr) {
                 OUT4("You cheer for %m.\n", 
					"%M cheers for you.\n",
                    	"%M cheers for %m.");
            }
            else {
                 OUT("You cheer.\n", "%M yells like a cheerleader.");
            }
    }
	else if(!strcmp(cmdlist[cmdno].cmdstr, "poke")) {
		if(crt_ptr) {
			OUT4("You poke %m.\n", 
				"%M pokes you.\n",
			     	"%M pokes %m.");
		}
		else {
			OUT6("You poke yourself in the eye.\n", "%M pokes %s in the eye.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "ogle")) {
		if(crt_ptr) {
			OUT4("You ogle %m with carnal intent.\n", 
				"%M ogles you salaciously.\n",
			     	"%M ogles %m salaciously.");
		}
		else
			print(fd, "Ogle whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "fart")) {
		if(crt_ptr) {
			OUT4("You fart on %m.\n", 
				"%M farts on you.\n",
			     	"%M farts on %m.");
		}
		else {
			OUT("You fart loudly.\n", "%M breaks wind.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "spit")) {
		if(crt_ptr) {
			OUT4("You spit on %m.\n", 
				"%M spits on you.\n",
			     	"%M spits on %m.");
		}
		else {
			OUT("You spit.\n", "%M spits.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "expose")) {
		if(crt_ptr) {
			OUT4("You expose yourself in front of %m.\n", 
			     "%M gets naked in front of you.\n",
			     	"%M gets naked in front of %m.");
		}
		else {
			OUT5("You expose yourself.\n", "%M exposes %sself.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "leer")) {
		if(crt_ptr) {
			OUT4("You leer at %m.\n", 
				"%M leers at you.\n",
			     	"%M leers at %m.");
		}
		else {
			OUT("You leer.\n", "%M leers at everyone.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "wink")) {
		if(crt_ptr) {
			OUT4("You wink at %m.\n", 
				"%M winks at you.\n",
			     	"%M winks at %m.");
		}
		else {
			OUT("You wink.\n", "%M winks.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "wave")) {
		if(crt_ptr) {
			OUT4("You wave to %m.\n", 
				"%M waves to you.\n",
			     	"%M waves to %m.");
		}
		else {
			OUT("You wave happily.\n", "%M waves happily.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "chuckle")) {
		OUT("You chuckle.\n", "%M chuckles.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "cackle")) {
		OUT("You cackle gleefully.\n",
		    "%M cackles out loud with glee.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "sigh")) {
		OUT("You sigh sadly.\n", "%M lets out a long, sad sigh.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "bounce")) {
		OUT3("You bounce around wildly!\n", 
		     "%M is so excited, %s can hardly keep still!");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "shrug")) {
		OUT("You shrug your shoulders.\n", "%M shrugs helplessly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "twiddle")) {
		OUT2("You twiddle your thumbs.\n",
		     "%M twiddles %s thumbs.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "yawn")) {
		OUT("You yawn loudly.\n", "%M yawns out loud.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "grin")) {
		OUT("You grin evilly.\n", "%M grins evilly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "frown")) {
		OUT("You frown.\n", "%M frowns.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "giggle")) {
		OUT("You giggle inanely.\n", "%M giggles inanely.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "sing")) {
		if(ply_ptr->class == BARD) {
			bard_song(ply_ptr, cmnd);
			return(0);
		}
		else 
		OUT("You sing a song.\n", "%M sings a song.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "hum")) {
		OUT("You hum a little tune.\n", "%M hums a little tune.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "snap")) {
		OUT2("You snap your fingers.\n", "%M snaps %s fingers.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "jump")) {
		OUT("You jump for joy.\n", "%M jumps for joy.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "skip")) {
		OUT("You skip like a girl.\n", "%M skips like a girl.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "dance")) {
		OUT("You dance about the room.\n", "%M dances about the room.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "cry")) {
		OUT("You burst into tears.\n", "%M bursts into tears.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "bleed")) {
		OUT("You bleed profusely.\n", "%M bleeds profusely.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "sniff")) {
		OUT("You sniff the air.\n", "%M sniffs the air.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "whimper")) {
		OUT("You whimper like a beat dog.\n", 
		    "%M whimpers like a beat dog.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "cringe")) {
		OUT("You cringe fearfully.\n", "%M cringes fearfully.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "whistle")) {
		OUT("You whistle a tune.\n", "%M whistles a tune.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "smirk")) {
		OUT("You smirk wryly.\n", "%M smirks wryly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "gasp")) {
		OUT("Your jaw drops.\n", "%M gasps in amazement.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "grunt")) {
		OUT("You grunt.\n", "%M grunts agonizingly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "flex")) {
		OUT2("You flex your muscles.\n", "%M flexes %s muscles.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "blush")) {
		OUT("You blush.\n", "%M turns beet-red.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "stomp")) {
		OUT("You stomp around.\n", "%M stomps around ostentatiously.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "fume")) {
		OUT("You fume.\n", "%M fumes. You can almost see the steam.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "clap")) {
		OUT2("You clap your hands.\n", "%M claps %s hands.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "rage")) {
		OUT("You rage like a madman.\n", "%M rages likes a madman.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "defecate") ||
		   (!strcmp(cmdlist[cmdno].cmdstr, "shit")) || 
		   (!strcmp(cmdlist[cmdno].cmdstr, "crap"))) {
		if(!dec_daily(&ply_ptr->daily[DL_DEFEC]) || ply_ptr->level < 2) { 
			print(fd,"You don't have to go.\n");
			return(0);
		}
		n = load_obj(SHIT, &obj_ptr);
		if(n > -1) {
		    sprintf(obj_ptr->name, "pile of shit left by %s",
			(F_ISSET(ply_ptr, PALIAS) ? 
				Ply[ply_ptr->fd].extr->alias_crt->name : 
					ply_ptr->name));
			add_obj_rom(obj_ptr, ply_ptr->parent_rom);
		}
		OUT("You squat down and shit.\n", "%M squats down and craps.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "pout")) {
		OUT("You pout.\n", "%M pouts like a child.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "drool")) {
		OUT5("You drool.\n", "%M drools all over %sself.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "faint")) {
		OUT("You faint.\n", "%M faints.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "goose")) {
	    if(crt_ptr) {
	   	 OUT4("You goose %m.\n", "%M gooses you.\n", "%M gooses %m.");
	    }
	    else
		print(fd, "Goose whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "copulate")) {
		if(crt_ptr) {
			if(!F_ISSET(crt_ptr, PNOCOP)) {
				OUT4("You copulate with %m.\n", 
			    	 "%M copulates with you.\n",
			     		"%M copulates with %m.");
			}
			else {
			    OUT4("%m turned aside and glared at you.\n", 
			       "%M tried to copulate with you, but failed.\n",
			     	"%M clumsily attempted to copulate with %m.");
			    ply_ptr->mpcur = 0;
			}
		}
		else
			print(fd, "Copulate with whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "hug")) {
		if(crt_ptr) {
			OUT4("You hug %m.\n", "%M hugs you close.\n", "%M hugs %m close.");
		}
		else
			print(fd, "Hug whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "comfort")) {
		if(crt_ptr) {
			OUT4("You comfort %m.\n", "%M comforts you.\n", "%M comforts %m.");
		}
		else
			print(fd, "Comfort whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "kiss")) {
		if(crt_ptr) {
			OUT4("You kiss %m gently.\n", 
			     "%M kisses you gently.\n",
			     	"%M kisses %m.");
		}
		else
			print(fd, "Kiss whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "slap")) {
		if(crt_ptr) {
			OUT4("You slap %m.\n", 
			     "%M slaps you across the face.\n",
			     	"%M slaps %m across the face.");
		}
		else
			print(fd, "Slap whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "glare")) {
		if(crt_ptr) {
			OUT4("You glare menacingly at %m.\n", 
			     "%M glares menacingly at you.\n",
			     	"%M glares menacingly at %m.");
		}
		else
			print(fd, "Glare at whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "pat")) {
		if(crt_ptr) {
			OUT4("You give %m a friendly pat.\n", 
			     "%M gives you a friendly pat on the back.\n",
			     	"%M gives %m a friendly pat on the back.");
		}
		else
			print(fd, "Pat whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "bow")) {
		if(crt_ptr) {
			OUT4("You bow before %m.\n", 
			     "%M bows before you.\n",
			     	"%M bows before %m.");
		}
		else {
			OUT("You make a full-sweeping bow.\n",
			    "%M makes a full-sweeping bow.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "cough")) {
		OUT("You cough politely.\n", "%M coughs politely.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "confused")) {
		OUT("You look bewildered.\n", "%M looks bewildered.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "grumble")) {
		OUT("You grumble darkly.\n", "%M grumbles darkly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "hiccup")) {
		OUT("You hiccup.\n", "%M hiccups noisily.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "mutter")) {
		OUT2("You mutter.\n", "%M mutters obscenities under %s breath.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "scratch")) {
		OUT2("You scratch your head cluelessly.\n", 
		    "%M scratches %s head cluelessly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "strut")) {
		OUT("You strut around vainly.\n", "%M struts around vainly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "sulk")) {
		OUT("You sulk.\n", "%M sulks in dejection.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "satisfied")) {
		OUT("You smile with satisfaction.\n", "%M smiles with satisfaction.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "sit")) {
		OUT("You sit down.\n", "%M takes a seat.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "stand")) {
		OUT("You stand up.\n", "%M stands up.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "wince")) {
		OUT("You wince painfully.\n", "%M winces painfully.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "power")) {
		OUT("You power up.\n", "%M powers up.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "roll")) {
		OUT2("You roll your eyes in exasperation.\n", 
		     "%M rolls %s eyes in exasperation.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "raise")) {
		OUT("You raise an eyebrow questioningly.\n", 
		    "%M raises an eyebrow questioningly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "whine")) {
		OUT("You whine annoyingly.\n", "%M whines annoyingly.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "growl")) {
		if(crt_ptr) {
			OUT4("You growl at %m.\n", 
			     "%M threatens you with a growl.\n",
			     "%M growls at %m threateningly.");
		}
		else {
			OUT("You growl.\n", "%M growls threateningly.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "bird")) {
		if(crt_ptr) {
			OUT4("You flip off %m.\n", 
			     "%M flips you the bird.\n",
			     	"%M gestures indignantly at %m.");
		}
		else {
			OUT("You gesture indignantly.\n", "%M gestures indignantly.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "shove")) {
		if(crt_ptr) {
			OUT4("You shove %m out of the way.\n", 
			     "%M shoves you out of the way.\n",
			     	"%M shoves %m out of the way.");
		}
		else
			print(fd, "Shove whom?\n");
		
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "high5")) {
		if(crt_ptr) {
			OUT4("You slap %m a triumphant highfive.\n", 
			     "%M slaps you a triumphant highfive.\n",
			     	"%M slaps %m a triumphant highfive.");
		}
		else
			print(fd, "Who gets the highfive?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "moon")) {
		if(crt_ptr) {
			OUT4("You moon %m.\n", 
			     "%M moons you.  It's a full moon tonight!\n",
			     	"%M moons %m.  It's a full moon tonight!");
		}
		else {
			OUT2("You moon the world.\n", 
			    "%M drops %s pants and moons the world.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "purr")) {
		if(crt_ptr) {
			OUT4("You purr at %m.\n", 
			     "%M purrs provocatively at you.\n",
			     	"%M purrs provocatively at %m.");
		}
		else {
			OUT("You purr provocatively.\n", "%M purrs provocatively.");
		}
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "taunt")) {
		if(crt_ptr) {
			OUT4("You taunt and jeer at %m.\n", 
			     "%M taunts and jeers at you.\n",
			     	"%M taunts and jeers at %m.");
		}
		else 
			print(fd, "Taunt whom?\n"); 
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "eye")) {
		if(crt_ptr) {
			OUT4("You eye %m suspiciously.\n", 
			     "%M eyes you suspiciously.\n",
			     	"%M eyes %m suspiciously.");
		}
		else
			print(fd, "Eye whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "worship")) {
		if(crt_ptr) {
			OUT4("You worship %m.\n", 
			     "%M kneels and gives praise to you.\n",
			     	"%M kneels and gives praise to %m.");
		}
		else
			print(fd, "Worship whom?\n");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "groan")) {
		OUT("You groan miserabily.\n", "%M groans miserabily.\n");
	
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "point")) {
		if(cmnd->num == 1) {
			OUT6("You point to yourself.\n", "%M points at %s.");
		}
		else {
			if(crt_ptr) {
				OUT4("You point at %m.\n", 
			    	"%M points a finger at you.\n",
				     	"%M points a finger at %m.");
			}
			else if(obj_ptr) {
				OUT7("You point to the %s\n", "%M points to the %s.");
			}
			else if(ext_ptr) {
				OUT8("You point toward the %s.\n", "%M points towards the %s.");
			}
		}
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "salute")) {
		if(crt_ptr) {
			OUT4("You salute %m.\n", 
			     "%M stands and salutes you.\n",
			     	"%M stands and salutes %m.");
		}
		else
			print(fd, "salute whom?\n");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "flip")) {
		num = mrand(1, 100);
		if(crt_ptr) {
			print(fd, "You flip a coin:  %s.\n",
				(num > 50) ? "heads":"tails");
			print(fd, "Only you and %m can see what it is.\n", crt_ptr);
			print(crt_ptr->fd, "%M flips a coin: it comes up %s.\n",
				ply_ptr, (num > 50) ? "heads":"tails");
			broadcast_rom2(fd, crt_ptr->fd, rom_ptr->rom_num, 
				  "%M flips a coin and shows it to %m.", ply_ptr, crt_ptr);
		}
		else {
			print(fd, "You flip a coin in full view:  %s.\n",
				(num > 50) ? "heads":"tails");
			broadcast_rom(fd, ply_ptr->rom_num,
				"%M flips a coin.  It comes up %s.", ply_ptr, 
				  (num > 50) ? "heads":"tails");
		}
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "smack")) {
		if(cmnd->num == 1) {
			OUT6("You smack your forehead.\n", "%M smacks %s on the forehead.");
		}
		else if(crt_ptr) {
			OUT4("You smack %m.\n", "%M smacks you.\n", "%M smacks %m.");
		}
		else
			print(fd, "smack whom?\n");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "shudder")) {
		OUT("You shudder with fear.\n", "%M shudders with fear.");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "belch")) {
		OUT("You belch with pride.\n", "%M belches loudly.");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "sweat")) {
		OUT("You break into a sweat.\n", "%M sweats profusely.");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "spin")) {
		OUT("You spin in circles.\n", "%M spins in circles.");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "twitch")) {
		OUT("You twitch uncontrollably.\n", "%M twitchs in fear.");
	}
	else if(!strcmp(cmdlist[cmdno].cmdstr, "cross")) {
		OUT2("You cross your fingers hopefully.\n", 
		     "%M crosses %s fingers hopefully.");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "stagger")) {
		OUT("You stagger like a fool.\n", "%M staggers about like a fool.");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "suck")) {
		OUT2("You suck your thumb.\n", "%M sucks %s thumb.");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "declare")) {
		OUT2("You declare your health.\n", "%M declares that %s health is ");
		broadcast_rom(fd, ply_ptr->rom_num,
		     "%d/%d H and %d/%d M.", ply_ptr->hpcur, ply_ptr->hpmax, 
			ply_ptr->mpcur, ply_ptr->mpmax);
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "moan")) {
		OUT("You moan with pain.\n", "%M moans painfully.");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "tango")) {
		if(cmnd->num == 1) {
			OUT("You do the deadly tango.\n", "%M does the deadly tango.");
		}
		else if(crt_ptr) {
			OUT4("You do the deadly tango with %m.\n", 
		        "%M does the deadly tango with you.\n",
		     	"%M does the deadly tango with %m.");
		}
		else
			print(fd, "smack whom?\n");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "awake") ||
		(!strcmp(cmdlist[cmdno].cmdstr, "wake"))) {
		if(crt_ptr) {
			OUT9("You try to awaken %m.\n", 
			"%M tries to rouse you from sleep.\n", 
			"%M does %s best to awaken %m.");
		}
		else {
			OUT2("You rouse yourself from sleep.\n", 
				"%M opens %s eyes and awakens.");
		}
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "land")) {
		if(F_ISSET(ply_ptr, PFLYSP) || F_ISSET(ply_ptr, PLEVIT)) {
			OUT3("You are no longer in the air.\n", 
		    	 "%M lands and %s is no longer in the air.");
			F_CLR(ply_ptr, PFLYSP);
			F_CLR(ply_ptr, PLEVIT);
		}
		else
			print(fd, "You are neither flying nor levitating.\n");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "visible")) {
		if(F_ISSET(ply_ptr, PINVIS)) {
			OUT2("Your body emerges into view.\n", 
		    	 "%M appears as %s body emerges into view.");
			F_CLR(ply_ptr, PINVIS);
		}
		else
			print(fd, "You are not invisible.\n");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "scoff")) {
		if(crt_ptr) {
			OUT4("You scoff at %m's remarks.\n", 
			     "%M scoffs at your remarks.\n",
			     	"%M scoffs at %m's remarks.");
		}
		else
			print(fd, " Scoff at whom?\n");
	}

	else if(!strcmp(cmdlist[cmdno].cmdstr, "beg")) {
		if(cmnd->num == 1) {
			OUT2("You grovel and beg from your knees.\n", 
				"%M grovels and begs from %s knees.");
		}
		else if(crt_ptr) {
			OUT4("You grovel and beg at the feet of %m.\n", 
				"%M grovels and begs at your feet.\n", 
				"%M grovels and begs at the feet of %m.");
		}
	}


	return(0);
}
 
