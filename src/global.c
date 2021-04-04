/*
 * GLOBAL.C:
 *
 *	Global variables.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mstruct.h"
#define MIGNORE
#include "mextern.h"



int			Tablesize;
int			Cmdnum;
long			Time;
long			StartTime;
struct 			lasttime	Shutdown;
struct 			lasttime Whisper[5];
int			Spy[PMAX];
int			Numlockedout;
lockout			*Lockout;
extern char     	report;

struct {
	creature	*ply;
	iobuf		*io;
	extra		*extr;
} Ply[PMAX];

struct {
	short		hpstart;
	short		mpstart;
	short		hp;
	short		mp;
	short		ndice;
	short		sdice;
	short		pdice;
} class_stats[15] = {
	{  0,  0,  0,  0,  0,  0,  0},
	{ 19,  2,  6,  2,  1,  6,  0},	/* assassin */
	{ 24,  1,  8,  1,  1,  3,  1},	/* barbarian */
	{ 16,  4,  5,  4,  1,  4,  0},	/* cleric */
	{ 22,  2,  7,  2,  1,  5,  0},	/* fighter */
	{ 14,  5,  4,  5,  1,  3,  0},	/* mage */
	{ 19,  3,  6,  3,  1,  4,  0},	/* paladin */
	{ 18,  3,  6,  3,  2,  2,  0},	/* ranger */
	{ 18,  3,  5,  2,  2,  2,  1},	/* thief */
	{ 15,  3,  5,  4,  2,  2,  1},  /* bard */
	{ 17,  3,  6,  2,  1,  3,  0},  /* monk */
	{ 18,  3,  6,  3,  2,  3,  0},  /* druid */
	{ 16,  4,  5,  4,  2,  3,  0},  /* alchemist */
	{ 30, 30, 10, 10,  5,  5,  5},	/* caretaker */
	{ 30, 30, 10, 10,  5,  5,  5}	/* DM */
};

int bonus[35] = { -4, -4, -4, -3, -3, -2, -2, -1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		  2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };

char class_str[][15] = { "None", "Assassin", "Barbarian", "Cleric",
	"Fighter", "Mage", "Paladin", "Ranger", "Thief", "Bard", 
	"Monk", "Druid", "Alchemist", "Caretaker", "Dungeonmaster" };

char race_str[][15] = { "Unknown", "Dwarf", "Elf", "Half-elf", "Hobbit",
	"Human", "Orc", "Half-giant", "Gnome", "Satyr", "Half-orc", "Ogre",
	"Dark-elf", "Centaur" };

char race_adj[][15] = { "Unknown", "Dwarven", "Elven", "Half-elven",
	"Halfling", "Human", "Orcish", "Half-giant", "Gnomish", "Satyrist", 
	"Half-orcish", "Ogroid", "Dark-elven", "Centaurion" };

short level_cycle[][10] = {
	{ 0,   0,   0,   0,   0,   0,   0,   0,   0,   0   },
	{ CON, STR, DEX, INT, STR, DEX, CON, INT, DEX, PTY },  /* assassin  */
	{ CON, DEX, STR, PTY, CON, STR, DEX, PTY, STR, INT },  /* barbarian */   
	{ INT, CON, PTY, STR, CON, PTY, INT, STR, PTY, DEX },  /* cleric    */
	{ DEX, CON, STR, INT, DEX, STR, CON, INT, STR, PTY },  /* fighter   */
	{ PTY, DEX, INT, CON, PTY, INT, CON, DEX, INT, STR },  /* mage      */
	{ STR, DEX, PTY, CON, DEX, PTY, CON, STR, PTY, INT },  /* paladin   */
	{ CON, STR, DEX, INT, PTY, STR, DEX, INT, CON, PTY },  /* ranger    */
	{ STR, CON, DEX, INT, CON, DEX, STR, INT, DEX, PTY },  /* thief     */
	{ STR, INT, PTY, DEX, STR, PTY, INT, DEX, PTY, CON },  /* bard      */
	{ STR, CON, DEX, PTY, STR, DEX, PTY, CON, DEX, INT },  /* monk      */
	{ PTY, DEX, CON, INT, PTY, CON, DEX, INT, CON, STR },  /* druid     */
	{ DEX, STR, INT, PTY, STR, INT, PTY, DEX, INT, CON }   /* alchemist */
};

short thaco_list[][20] = { 
	{ 20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20 }, 
/*a*/	{ 18,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,10,10, 9, 9 },
/*b*/	{ 20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 3, 2 },
/*c*/	{ 20,20,19,18,18,17,16,16,15,14,14,13,13,12,12,11,10,10, 9, 8 },
/*f*/	{ 20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 3, 3 },
/*m*/	{ 20,20,19,19,18,18,18,17,17,16,16,16,15,15,14,14,14,13,13,11 },
/*p*/	{ 20,19,18,18,17,16,16,15,15,14,14,13,13,12,11,11,10, 9, 8, 7 },
/*r*/	{ 20,19,18,17,16,16,15,15,14,14,13,12,12,11,11,10, 9, 9, 8, 7 },
/*t*/	{ 20,20,19,18,18,17,17,16,16,15,15,14,13,13,12,11,11,10, 9, 9 },
/*bd*/  { 20,18,18,17,17,16,16,15,15,14,14,13,12,11,11,10, 9, 9, 8, 7 },
/*mn*/  { 20,18,17,17,16,16,15,14,14,13,12,12,11,10, 9, 8, 7, 6, 6, 5 },	
/*d*/	{ 20,19,18,17,16,16,15,15,14,14,13,12,12,11,11,10, 9, 9, 8, 7 },
/*al*/  { 20,18,17,17,16,16,15,15,14,14,13,12,11,11,10,10, 9, 8, 7, 6 },	
	{  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

long quest_exp[] = {
	0125, 1000, 0125, 0125, 0125, 0125, 1000, 0125,
	0125, 0125, 0125, 0125, 0125, 1000, 1000, 2000,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
	1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
	1000, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125,
	0125, 0125, 0125, 0125, 0125, 0125, 0125, 0125
};
 
long needed_exp[] = {
    512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 100000,
    166410, 277360, 394171, 560992, 757125, 1087504, 1402815,
    1739616, 2203457, 2799000, 3505139, 4465120, 5792661, 7319072,
    8874375, 15000000};   



char lev_title[][8][20] = {
	{ "", "", "", "", "", "", "", "" },
	{ "Skulker", "Slasher", "Killer", "Cutthroat",
	  "Murderer", "Executioner", "Expert Assassin", "Master Assassin" },
	{ "Grunt", "Tribesman", "Savage", "Brutalizer", 
	  "Berserker", "Destroyer", "Barbarian Chief", "Dragon Slayer" },
	{ "Acolyte", "Priest", "Vicar", "Bishop",
	  "Cardinal", "Theologian", "Patriarch", "Mystical Prelate" },
	{ "Knifer", "Punisher", "Weapons Master", "Warrior",
	  "Hero", "Champion", "Lord Warrior", "Hammer of God" },
	{ "Apprentice", "Spellbinder", "Thaumaturgist", "Occultate",
	   "Illusionist", "Wizard", "Necromancer", "High Sorcerer" },
	{ "Gallant", "Knight", "Valiant", "Guardian",
	  "Ordained Paladin", "Chevalier", "Justiciar", "Knight Templar" },
	{ "Searcher", "Scout", "Tracker", "Guide", 
	  "Royal Guide", "Ranger", "Ranger Knight", "Ranger Lord" },
	{ "Filcher", "Pickpocket", "Burglar", "Dervish",
	  "Sharper", "Magsman", "High Thief", "Master Thief" },
	{ "Jongleur", "Lyrist", "Sonneteer", "Skald",
	  "Minstrel", "Muse", "Bard", "Master Bard" },
	{ "Novice", "Initiate", "Brother", "Disciple", "Templar",  
	  "Master", "Shadow Master", "Grand Master" },
	{ "Initiate", "Protector", "Oracle", "War Priest", 
	  "Appostate", "Mystic", "Great Druid", "Grand Druid" },
	{ "Novitiate", "Metalurgist", "Elementist", "Deviner", 
	  "Master Chemist", "Alchemist", "Master Alchemist", "Lord Alchemist" },
	{ "Caretaker", "Caretaker", "Caretaker", "Caretaker", "Caretaker",
	  "Caretaker", "Caretaker", "Caretaker" },
	{ "Dungeonmaster", "Dungeonmaster", "Dungeonmaster", "Dungeonmaster", 
	  "Dungeonmaster", "Dungeonmaster", "Dungeonmaster", "Dungeonmaster"  }
};

struct {
	char	*cmdstr;
	int		cmdno;
	int		(*cmdfn)();
} cmdlist[] = {
	{ "n", 1, move },
	{ "north", 1, move },
	{ "s", 1, move },
	{ "south", 1, move },
	{ "e", 1, move },
	{ "east", 1, move },
	{ "w", 1, move },
	{ "west", 1, move },
	{ "northeast", 1, move },
	{ "ne", 1, move },
	{ "northwest", 1, move },
	{ "nw", 1, move },
	{ "southeast", 1, move },
	{ "se", 1, move },
	{ "southwest", 1, move },
	{ "sw", 1, move },
	{ "u", 1, move },
	{ "up", 1, move },
	{ "d", 1, move },
	{ "down", 1, move },
	{ "out", 1, move },
	{ "dis", 1, move },
	{ "dismount", 1, move },
	{ "m", 1, move },
	{ "mount", 1, move },
	{ "left", 1, move },
	{ "r", 1, move },
	{ "right", 1, move },
	{ "forward", 1, move },
	{ "back", 1, move },
	{ "exit", 1, move },
	{ "aft", 1, move },
	{ "port", 1, move },
	{ "starboard", 1, move },
	{ "climb", 1, go },
	{ "l" , 2, look },
	{ "look", 2, look },
	{ "consider", 2, look },
	{ "examine", 2, look },
	{ "see", 2, look },
	{ "eyeball", 2, look },
	{ "quit", 3, quit },
	{ "bye", 3, quit },
	{ "goodbye", 3, quit },
	{ "say", 4, say },
	{ "\"", 4, say },
	{ "'", 4, say },
	{ "get", 5, get },
	{ "take", 5, get },
	{ "inventory", 6, inventory },
	{ "i", 6, inventory },
	{ "drop", 7, drop },
	{ "put", 7, drop },
	{ "who", 8, who },
	{ "wear", 9, wear },
	{ "remove", 10, remove_obj },
	{ "rm", 10, remove_obj },
	{ "equipment", 11, equipment },
	{ "hold", 12, hold },
	{ "wield", 13, ready },
	{ "ready", 13, ready },
	{ "help", 14, help },
	{ "?", 14, help },
	{ "health", 15, health },
	{ "score", 15, health },
	{ "sc", 15, health },
	{ "information", 16, info },
	{ "send", 17, psend },
	{ "sen", 17, psend },
	{ "tell", 17, psend },
	{ "flash", 17, psend },
	{ "follow", 18, follow },
	{ "lose", 19, lose },
	{ "nofollow", 19, lose },
	{ "group", 20, group },
	{ "party", 20, group },
	{ "track", 21, track },
	{ "peek", 22, peek },
	{ "attack", 23, attack },
	{ "kill", 23, attack },
	{ "stab", 23, attack },
	{ "impale", 23, attack },
	{ "smash", 23, attack },
	{ "club", 23, attack },
	{ "bludgeon", 23, attack },
	{ "k", 23, attack },
	{ "hit", 23, attack },
	{ "search", 24, search },
    { "emote", 25, emote},    
    { ":", 25, emote},    
    { ";", 25, emote},    
	{ "hide", 26, hide },
	{ "set", 27, set },
	{ "clear", 28, clear },
	{ "yell", 29, yell },
	{ "go", 30, go },
	{ "leave", 30, go },
	{ "enter", 30, go },
	{ "open", 31, openexit },
	{ "close", 32, closeexit },
	{ "shut", 32, closeexit },
	{ "unlock", 33, unlock },
	{ "lock", 34, lock },
	{ "pick", 35, picklock },
	{ "steal", 36, steal },
	{ "flee", 37, flee },
	{ "fl", 37, flee },
	{ "run", 37, flee },
	{ "escape", 37, flee },
	{ "c", 38, cast },
	{ "cast", 38, cast },
	{ "study", 39, study },
	{ "learn", 39, study },
	{ "read", 39, study },
	{ "recite", 40, readscroll },
	{ "list", 41, list },
	{ "buy", 42, buy },
	{ "sell", 43, sell },
	{ "value", 44, value },
	{ "cost", 44, value },
	{ "bs", 45, backstab },
	{ "backstab", 45, backstab },
	{ "train", 46, train },
	{ "advance", 46, train },
	{ "level", 46, train },
	{ "give", 47, give },
	{ "repair", 48, repair },
	{ "time", 49, prt_time },
	{ "circle", 50, circle },
	{ "bash", 51, bash },
	{ "save", 52, savegame },
	{ "mail", 53, postsend },
	{ "post", 53, postsend },
	{ "sendmail", 53, postsend },
	{ "sendletter", 53, postsend },
	{ "readmail", 54, postread },
	{ "readletter", 54, postread },
	{ "deletemail", 55, postdelete },
	{ "talk", 56, talk },
	{ "tale", 56, talk },
	{ "ask", 56, talk },
	{ "speak", 56, talk },
	{ "parley", 56, talk },
	{ "gtalk",57, gtalk },
	{ "drink", 58, drink },
	{ "quaff", 58, drink },
	{ "broadcast", 59, broadsend },
	{ "zap", 60, zap },
	{ "welcome", 61, welcome },
	{ "turn", 62, turn },
	{ "bribe", 63, bribe },
	{ "haste", 64, haste },
	{ "pray", 65, pray },
	{ "prepare", 66, prepare },
	{ "use", 67, use },
	{ "ignore", 68, ignore },
	{ "whois", 69, whois },
	{ "sneak", 70, sneak },
	{ "teach", 71, teach },
	{ "pledge", 72, pledge },
	{ "rescind", 73, rescind },
    { "purchase", 74, purchase },    
    { "selection", 75, selection }, 
    { "trade", 76, trade},    
	{ "suicide", 77, ply_suicide },
	{ "passwd", 78, passwd},
	{ "password", 78, passwd },
	{ "vote", 79, vote },
	{ "finger", 80, pfinger },
	{ "charm", 81, bard_song2 },
	{ "meditate", 82, meditate },
	{ "touch", 83, touch_of_death },
/*	{ "prep", 84, prep_herb },				*/
/*	{ "apply", 85, apply_herb },			*/
/*	{ "ingest", 86, ingest_herb },			*/
/*	{ "ingest", 87, eat },					*/
/*	{ "paste", 88, paste_herb },			*/
	{ "recharge", 89, recharge_wand },
	{ "transmute", 89, transmute },
	{ "glow", 89, silver_renew },
	{ "description", 90, describe_me },
	{ "renew", 91, renew },
	{ "bank", 92, bank_deposit },
	{ "deposit", 92, bank_deposit },
	{ "balance", 92, bank_balance },
	{ "withdraw", 92, bank_withdraw },
	{ "transfer", 92, bank_transfer }, 
	{ "borrow", 92, bank_loan }, 
	{ "newspaper", 93, news },
	{ "news", 93, news },
	{ "fasten", 94, fasten_cont },
	{ "unfasten", 94, unfasten_cont },
	{ "jimmy", 95, jimmylock },
	{ "pry", 95, jimmylock },
	{ "channelemote", 97, channelemote },
	{ "chemote", 97, channelemote },
	{ "broe", 97, channelemote },
	{ "classemote", 98, classemote },
	{ "clemote", 98, classemote },
	{ "classsend", 99, class_send },
	{ "clsend", 99, class_send },
	{ "rescue", 201, rescue },
	{ "home", 202, home },
	{ "clear_screen", 203, clear_screen },
	{ "cs", 203, clear_screen },
	{ "discard", 204, discard },
	{ "fling", 204, discard },
	{ "gaze", 206, gaze },
	{ "peer", 206, gaze },
	{ "scan", 206, gaze },
	{ "slitthroat", 207, throatslit },
	{ "slit", 207, throatslit },
	{ "slice", 207, throatslit },
	{ "stealgold", 208, stealgold },
	{ "distract", 209, distract },
	{ "plead", 210, plead },
	{ "mercy", 210, plead },
	{ "cripple", 211, cripple },
	{ "snatch", 212, snatch },
	{ "grab", 212, snatch },
	{ "show", 213, show },
	{ "fix", 214, fix },
	{ "agility", 215, compute_agility },
	{ "stumble", 215, compute_agility },
	{ "pay", 216, pay },
	{ "porter", 216, pay },
	{ "gatekeeper", 216, pay },
	{ "doorman", 216, pay },
	{ "whisper", 217, porter_return },
	{ "berserk", 218, berserk },
/*********************************/
	{ "ack", 100, action },
	{ "admire", 100, action },
	{ "beam", 100, action},
	{ "beckon", 100, action},
	{ "beg", 100, action},
	{ "belch", 100, action},
	{ "bird", 100, action },
	{ "blanch", 100, action },
	{ "bleed", 100, action },
	{ "blink", 100, action },
	{ "blush", 100, action },
	{ "bounce", 100, action },
	{ "bow", 100, action },
	{ "burp", 100, action },
	{ "cackle", 100, action },
	{ "cheer", 100, action },
	{ "chops", 100, action },
	{ "chuckle", 100, action },
	{ "clap", 100, action },
	{ "confused", 100, action },
	{ "comfort", 100, action },
	{ "copulate", 100, action },
	{ "cough", 100, action },
	{ "crap", 100, action },
	{ "cringe", 100, action },
	{ "crouch", 100, action },
	{ "cross", 100, action },
	{ "cry", 100, action },
	{ "curtsy", 100, action },
	{ "dance", 100, action },
	{ "declare", 100, action },
	{ "defecate", 100, action },
	{ "drool", 100, action },
	{ "expose", 100, action },
	{ "eye", 100, action },
	{ "faint", 100, action },
	{ "fart", 100, action },
	{ "fist", 100, action },
	{ "flex", 100, action },
	{ "flip", 100, action },
	{ "float", 100, action },
	{ "frustrate", 100, action },
	{ "frown", 100, action },
	{ "fume", 100, action },
	{ "gasp", 100, action },
	{ "giggle", 100, action },
	{ "glare", 100, action },
	{ "goose", 100, action },
	{ "groan", 100, action },
	{ "grin", 100, action },
	{ "grumble", 100, action },
	{ "grunt", 100, action },
	{ "growl", 100, action },
	{ "hand", 100, action },
	{ "high5", 100, action },
	{ "hiccup", 100, action },
	{ "hug", 100, action },
	{ "hum", 100, action },
	{ "idea", 100, action },
	{ "jump", 100, action },
	{ "kick", 100, action },
	{ "kiss", 100, action },
	{ "knee", 100, action },
	{ "land", 100, action },
	{ "laugh", 100, action },
	{ "hyslaugh", 100, action },
	{ "menlaugh", 100, action },
	{ "leer", 100, action },
	{ "lol", 100, action },
	{ "masturbate", 100, action},
	{ "masterbate", 100, action},
	{ "moan", 100, action },
	{ "moon", 100, action },
	{ "mutter", 100, action },
	{ "nervous", 100, action },
	{ "nod", 100, action },
	{ "ogle", 100, action },
	{ "pat", 100, action },
	{ "point", 100, action },
	{ "poke", 100, action },
	{ "ponder", 100, action },
	{ "pounce", 100, action },
	{ "power", 100, action },
	{ "puke", 100, action },
	{ "pout", 100, action },
	{ "purr", 100, action },
	{ "raise", 100, action },
	{ "rage", 100, action },
	{ "relax", 100, action },
	{ "roll", 100, action },
	{ "room#", 100, action },
	{ "salute", 100, action },
	{ "satisfied", 100, action },
	{ "scoff", 100, action },
	{ "scratch", 100, action },
	{ "scream", 100, action },
	{ "shake", 100, action },
	{ "shove", 100, action },
	{ "shudder", 100, action },
	{ "shrug", 100, action },
	{ "sigh", 100, action },
	{ "sing", 100, action },
	{ "sit", 100, action },
	{ "skip", 100, action },
	{ "slap", 100, action },
	{ "sleep", 100, action },
	{ "sulk", 100, action },
	{ "social", 100, action },
	{ "smack", 100, action },
	{ "smile", 100, action },
	{ "satsmile", 100, action },
	{ "warmsmile", 100, action },
	{ "smiles", 100, action },
	{ "smirk", 100, action },
	{ "smoke", 100, action },
	{ "snap", 100, action },
	{ "sneeze", 100, action },
	{ "snicker", 100, action },
	{ "sniff", 100, action },
	{ "splonk", 100, action },
	{ "splunk", 100, action },
	{ "stagger", 100, action },
	{ "stand", 100, action },
	{ "stomp", 100, action },
	{ "stretch", 100, action },
	{ "strut", 100, action },
	{ "stutter", 100, action },
	{ "spin", 100, action },
	{ "spit", 100, action },
	{ "suck", 100, action },
	{ "sweat", 100, action },
	{ "tackle", 100, action },
	{ "tango", 100, action },
	{ "tap", 100, action },
	{ "taunt", 100, action },
	{ "tickle", 100, action },
	{ "think", 100, action },	
	{ "tongue", 100, action },
	{ "twiddle", 100, action },
	{ "twitch", 100, action },
	{ "visible", 100, action},
	{ "wake", 100, action},
	{ "awake", 100, action},
	{ "whine", 100, action },
	{ "whirl", 100, action },
	{ "wince", 100, action },
	{ "worship", 100, action },
	{ "wave", 100, action },
	{ "whimper", 100, action },
	{ "whistle", 100, action },
	{ "wiggle", 100, action },
	{ "wink", 100, action },
	{ "yawn", 100, action },
/*************************************************/
	{ "*teleport", 101, dm_teleport },
	{ "*t", 101, dm_teleport },
/*	{ "", 102, },	*/
	{ "*reload", 103, dm_reload_rom },
	{ "*save", 104, dm_resave },
	{ "*create", 105, dm_create_obj },
	{ "*c", 105, dm_create_obj },
	{ "*perm", 106, dm_perm },
	{ "*invis", 107, dm_invis },
	{ "*i", 107, dm_invis },
	{ "*s", 108, dm_send },
	{ "*send", 108, dm_send },
	{ "*purge", 109, dm_purge },
	{ "*ac", 110, dm_ac },
	{ "*users", 111, dm_users },
	{ "*echo", 112, dm_echo },
	{ "*flushrooms", 113, dm_flushsave },
	{ "*shutdown", 114, dm_shutdown },
	{ "*f", 115, dm_force },
	{ "*force", 115, dm_force },
	{ "*flushcrtobj", 116, dm_flush_crtobj },
	{ "*m", 117, dm_create_crt },
	{ "*monster", 117, dm_create_crt },
	{ "*status", 118, dm_stat },
	{ "*add", 119, dm_add_rom },
	{ "*set", 120, dm_set },
	{ "*log", 121, dm_log },
	{ "*spy", 122, dm_spy },
	{ "*lock", 123, dm_loadlockout },
	{ "*finger", 124, dm_finger },
	{ "*list", 125, dm_list },
	{ "*info", 126, dm_info },
	{ "*parameter", 127, dm_param },
	{ "*silence", 128, dm_silence },
	{ "*broad", 129, dm_broadecho },
 	{ "*replace",130, dm_replace },    
    { "*name",131, dm_nameroom },    
   	{ "*append",132, dm_append },    
   	{ "*prepend",133, dm_prepend },    
   	{ "*gcast",134, dm_cast },    
    { "*group",135, dm_group },    
	{ "*notepad",136, notepad },
 	{ "*delete",137, dm_delete},    
	{ "*oname", 138, dm_obj_name },
	{ "*cname", 139, dm_crt_name },
	{ "*active", 140, list_act },
	{ "*dust", 141, dm_dust },
	{ "*dmhelp", 142, dm_help },
	{ "*attack", 143, dm_attack },
	{ "*enemy", 144, list_enm },
	{ "*charm", 145, list_charm },
	{ "*auth", 146, dm_auth },
	{ "*possess", 147, dm_alias }, 
	{ "*tell", 148, dm_flash }, 
	{ "*memory", 149, dm_memory },
	{ "*find", 150, dm_find_db },
	{ "*advance", 151, dm_advance },
	{ "*inventory", 152, dm_inventory },
	{ "*spells", 153, dm_spells },
	{ "*quests", 154, dm_quests },
	{ "*news",155, edit_news },
	{ "*creatureadd", 156, dm_add_crt },
	{ "*crtadd", 156, dm_add_crt },
	{ "*ca", 156, dm_add_crt },
	{ "*creaturesave", 156, dm_save_crt },
	{ "*crtsave", 156, dm_save_crt },
	{ "*cs", 156, dm_save_crt },
	{ "*objadd", 157, dm_add_obj },
	{ "*objectadd", 157, dm_add_obj },
	{ "*oa", 157, dm_add_obj },
	{ "*objectsave", 157, dm_save_obj },
	{ "*objsave", 157, dm_save_obj },
	{ "*os", 157, dm_save_obj },
	{ "*creaturetalk", 158, dm_add_talk },
	{ "*crttalk", 158, dm_add_talk },
	{ "*ct", 158, dm_add_talk },
	{ "*remove", 159, dm_remove_db },
	{ "*password", 165, dm_password },
	{ "*passwd", 165, dm_password },
	{ "*store", 166, store_item },
	{ "*exp", 167, dm_exp },
	{ "*bank", 168, dm_bal },
	{ "*balance", 168, dm_bal },

	{ "push", -2, 0 },
	{ "press", -2, 0 },
	{ "move", -2, 0 },
	{ "@", 0, 0 }
};

char article[][10] = { "the", "from", "to", "with", "an", "in", "for", "on", "@" };

char	ext[5][8] = {"out", "up", "down", "forward", "back"};

struct {
	char 	*splstr;
	int	splno;
	int	(*splfn)();
} spllist[] = {
	{ "vigor", SVIGOR, vigor },
	{ "hurt", SHURTS, offensive_spell },
	{ "light", SLIGHT, light },
	{ "cure-poison", SCUREP, curepoison },
	{ "bless", SBLESS, bless },
	{ "protection", SPROTE, protection },
	{ "fireball", SFIREB, offensive_spell },
	{ "invisibility", SINVIS, invisibility },
	{ "restore", SRESTO, restore },
	{ "detect-invisible", SDINVI, detectinvis },
	{ "detect-magic", SDMAGI, detectmagic },
	{ "teleport", STELEP, teleport },
	{ "stun", SBEFUD, befuddle },
	{ "lightning", SLGHTN, offensive_spell },
	{ "iceblade", SICEBL, offensive_spell },
	{ "enchant", SENCHA, enchant },
	{ "word-of-recall", SRECAL, recall },
	{ "summon", SSUMMO, summon },
	{ "mend-wounds", SMENDW, mend },
	{ "heal", SFHEAL, heal },
	{ "track", STRACK, magictrack },
	{ "levitate", SLEVIT, levitate },
	{ "resist-fire", SRFIRE, resist_fire },
	{ "fly", SFLYSP, fly },
	{ "resist-magic", SRMAGI, resist_magic },
	{ "shockbolt", SSHOCK, offensive_spell },
	{ "rumble", SRUMBL, offensive_spell },
	{ "burn", SBURNS, offensive_spell },
	{ "blister", SBLIST, offensive_spell },
	{ "dustgust", SDUSTG, offensive_spell },
	{ "waterbolt", SWBOLT, offensive_spell },
	{ "crush", SCRUSH, offensive_spell },
	{ "engulf", SENGUL, offensive_spell },
	{ "burstflame", SBURST, offensive_spell },
	{ "steamblast", SSTEAM, offensive_spell },
	{ "shatterstone", SSHATT, offensive_spell },
	{ "immolate", SIMMOL, offensive_spell },
	{ "bloodboil", SBLOOD, offensive_spell },
	{ "thunderbolt", STHUND, offensive_spell },
	{ "earthquake", SEQUAK, offensive_spell },
	{ "flamefill", SFLFIL, offensive_spell },
	{ "know-aura", SKNOWA, know_alignment },
	{ "remove-curse", SREMOV, remove_curse },
	{ "resist-cold", SRCOLD, resist_cold },
	{ "breathe-water", SBRWAT, breathe_water },
	{ "earth-shield", SSSHLD, earth_shield },
	{ "mind-meld", SLOCAT, locate_player },
	{ "drain-exp", SDREXP, drain_exp },
	{ "cure-disease", SRMDIS, rm_disease },
	{ "cure-blindness", SRMBLD, rm_blind },
	{ "fear", SFEARS, fear }, 
	{ "room-vigor", SRVIGO, room_vigor }, 
	{ "transport", STRANO, object_send },
	{ "blind", SBLIND, blind },
	{ "silence", SSILNC, silence },
	{ "remove-silence", SRMSIL, rm_silence },
	{ "fortune", SFORTU, fortune },
	{ "super-strength", SSTRNG, super_strength },	
	{ "shrink", SSHRNK, shrink },	
	{ "reflect", SRFLCT, reflect },	
	{ "evil-eye", SEVEYE, evil_eye },	
	{ "freeze", SFREEZ, freeze },	
	{ "hex", SHEXSP, hex },	
	{ "conjure", SCONJR, conjure },	
	{ "entangle", SENTNG, entangle },	
	{ "knock", SKNOCK, knock },	
	{ "absolve", SABSLV, absolve },	
	{ "dispel", SDISPL, dispel },	
	{ "resist-acid", SRACID, resist_acid },	
	{ "@", -1,0 }
};

/*******************************
	int		splno;
	char	realm;
	int		mp;
	int		ndice;
	int		sdice;
	int		pdice;
	char	bonus_type;
********************************/
int spllist_size = sizeof(spllist)/sizeof(*spllist);

struct osp_t ospell[] = {
	{ SHURTS,  WIND,  3, 3, 3,  1, 1 },	/* hurt			*/
	{ SRUMBL, EARTH,  3, 3, 3,  1, 1 },	/* rumble		*/
	{ SBURNS,  FIRE,  3, 3, 3,  1, 1 },	/* burn			*/
	{ SBLIST, WATER,  3, 3, 3,  1, 1 },	/* blister		*/

	{ SDUSTG,  WIND,  7, 4, 3,  8, 2 },	/* dustgust		*/
	{ SCRUSH, EARTH,  7, 4, 3,  8, 2 },	/* crush	    */
	{ SFIREB,  FIRE,  7, 4, 3,  8, 2 },	/* fireball		*/
	{ SWBOLT, WATER,  7, 4, 3,  8, 2 },	/* waterbolt	*/

	{ SSHOCK,  WIND, 10, 4, 4, 12, 2 },	/* shockbolt	*/
	{ SENGUL, EARTH, 10, 4, 4, 12, 2 },	/* engulf		*/
	{ SBURST,  FIRE, 10, 4, 4, 12, 2 },	/* burstflame	*/
	{ SSTEAM, WATER, 10, 4, 4, 12, 2 },	/* steamblast	*/

	{ SLGHTN,  WIND, 15, 4, 5, 15, 3 },	/* lightning	*/
	{ SSHATT, EARTH, 15, 4, 5, 15, 3 },	/* shatterstone	*/
	{ SIMMOL,  FIRE, 15, 4, 5, 15, 3 },	/* immolate		*/
	{ SBLOOD, WATER, 15, 4, 5, 15, 3 },	/* bloodboil	*/

	{ STHUND,  WIND, 22, 6, 4, 25, 3 },	/* thunderbolt	*/
	{ SEQUAK, EARTH, 22, 6, 4, 25, 3 }, /* earthquake	*/
	{ SFLFIL,  FIRE, 22, 6, 4, 25, 3 },	/* flamefill	*/
	{ SICEBL, WATER, 22, 6, 4, 25, 3 },	/* iceblade		*/
	{ -1, 0, 0, 0, 0, 0, 0 }
};

char number[][10] = {
	"zero",
	"one",
	"two",
	"three",
	"four",
	"five",
	"six",
	"seven",
	"eight",
	"nine",
	"ten",
	"eleven",
	"twelve",
	"thirteen",
	"fourteen",
	"fifteen",
	"sixteen",
	"seventeen",
	"eighteen",
	"nineteen",
	"twenty"
};
