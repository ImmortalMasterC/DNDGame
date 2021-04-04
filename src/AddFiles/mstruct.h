/*
 * MSTRUCT.H:
 *
 *	Main data structures and type definitions.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include "mtype.h"

typedef struct obj_tag {		/* Object list tags */
	struct obj_tag 	*next_tag;
	struct object	*obj;
} otag;

typedef struct crt_tag {		/* Creature list tags */
	struct crt_tag	*next_tag;
	struct creature	*crt;
} ctag;

typedef struct rom_tag {		/* Room list tags */
	struct rom_tag	*next_tag;
	struct room	*rom;
} rtag;

typedef struct ext_tag {		/* Exit list tags */
	struct ext_tag	*next_tag;
	struct exit_	*ext;
} xtag;

typedef struct enm_tag {		/* Enemy list tags */
	struct enm_tag	*next_tag;
	char		enemy[80];
	int			damage;
} etag;

typedef struct tlk_tag {		/* Talk list tags */
	struct tlk_tag	*next_tag;
	char		*key;
	char		*response;
	char		type;
	char		*action;
	char		*target;
    char        on_cmd;          /* action functions */
    char        if_cmd;          /* if # command succesful */         
    char        test_for;        /* test for condition in room */
    char        do_act;          /* do action */
    char        success;         /* command was succesful */
    char        if_goto_cmd;     /* used to jump to new cmd point */
    char        not_goto_cmd;    /* jump to cmd if not success */
    char        goto_cmd;        /* unconditional goto cmd */
    int         arg1;
    int         arg2;
} ttag;

typedef struct daily {			/* Daily-use operation struct */
	char		max;
	char		cur;
	long		ltime;
} daily;

typedef struct lasttime {		/* Timed operation struct */
	long		interval;
	long		ltime;
	short		misc;
} lasttime;

typedef struct iobuf {			/* I/O buffers for players */
	char		input[IBUFSIZE];
	char		output[OBUFSIZE];
	short		ihead, itail;
	short		ohead, otail;
	void		(*fn)();
	char		fnparam;
	long		ltime;
	char		intrpt;
	char		commands;
	int		    lookup_pid;
	char		address[40];
	char		userid[9];
} iobuf;

typedef struct extra {			/* Extra (non-saved) player fields */
	char		tempstr[4][80];
	char		lastcommand[80];
	ctag		*first_charm;
	etag		*first_ignore;
	int			luck;
	struct      creature	*alias_crt;	
	int			last_rom;
	struct		room		*home_rom;
	int			agility;
} extra;

typedef struct lockout {
	char		address[80];
	char		password[20];
	char		userid[9];
} lockout;

typedef struct cmd {
	int		    num;
	char		fullstr[256];
	char		str[COMMANDMAX][25];
	long		val[COMMANDMAX];
} cmd;

typedef struct osp_t {
	int		splno;
	char	realm;
	int		mp;
	int		ndice;
	int		sdice;
	int		pdice;
	char	bonus_type;
} osp_t;

typedef struct exit_ {
	char		name[20];
	short		room;
	char		flags[4];
	struct      lasttime	ltime;			/* Timed open/close */
	unsigned char		key;	 	        /* Key required */
} exit_;

typedef struct object {
	char 		name[80];
	char		description[80];
	char		key[3][20]; 	    		/* key 3 hold object index */
	char		use_output[80];			/* String output by successful use */
	long 		value;
	short 		weight;
	char 		type;
	char		adjustment;
	short		shotsmax;			/* Shots before breakage */
	short		shotscur;
	short		ndice;				/* Number/Sides/Plus dice */
	short		sdice;
	short		pdice;
	char		armor;				/* AC adjustment */
	char		wearflag;			/* Where/If it can be worm */
	char		magicpower;
	char		magicrealm;
	short		special;
	char		flags[8];
	char		questnum;			/* Quest fulfillment number */
	otag		*first_obj;			/* Objects contained inside */
	struct object	*parent_obj;			/* Object this is in */
	struct room		*parent_rom;		/* Room this is in */
	struct creature	*parent_crt;			/* Creature this is in */
} object;

typedef struct room {
	short		rom_num;
	char		name[80];
	char		*short_desc;			/* Descriptions */
	char		*long_desc;
	char		*obj_desc;
	char		lolevel;			/* Lowest level allowed in */
	char		hilevel;			/* Highest level allowed in */
	short		special;
	char		trap;
	short		trapexit;
	char		track[80];			/* Track exit */
	char		flags[8];
	short		random[10];			/* Random monsters */
	char		traffic;			/* R. monster traffic */
	struct lasttime	perm_mon[10];			/* Permanent/reappearing monsters */
	struct lasttime	perm_obj[10];			/* Permanent/reappearing items */
	long		beenhere;			/* # times room visited */
	long		established;			/* Time room was established */
	xtag		*first_ext;			/* Exits */
	otag		*first_obj;			/* Items */
	ctag		*first_mon;			/* Monsters */
	ctag		*first_ply;			/* Players */
} room;

typedef struct creature {
	char		name[80];
	char		description[80];
	char		talk[80];
	char		password[15];
	char		key[3][20];
	short		fd;				/* Socket number */
	char		level;
	char		type;				/* Creature type */
	char		class;
	char		race;
	char		numwander;
	short		alignment;
	char		strength;
	char		dexterity;
	char		constitution;
	char		intelligence;
	char		piety;
	short		hpmax;				/* Max/Current hp and mp */
	short		hpcur;
	short		mpmax;
	short		mpcur;
	char		armor;				/* Armor Class */
	char		thaco;				/* To hit armor class 0 */
	long		experience;
	long		gold;
	short		ndice;				/* Bare-handed damage */
	short		sdice;
	short		pdice;
	short		special;
	long		proficiency[5]; 		/* Weapon proficiencies */
	long		realm[4];			/* Magic Spell realms */
	char		spells[16];			/* Known spells */
	char		flags[8];
	char		quests[16];			/* Quests fulfilled (P) */
#define 		NUMHITS 	quests[0]	
	char		questnum;			/* Quest fulfillment number (M) */
	short		carry[10];			/* Random items monster carries */
#define			WIMPYVALUE 	carry[0]
	short		rom_num;
	struct 	object		*ready[MAXWEAR];	/* Worn/readied items */
	struct 	daily		daily[10];		/* Daily use variables */
	struct 	lasttime	lasttime[45];		/* Last-time-used variables */
	struct 	creature	*following;		/* Creature being followed */
	ctag		*first_fol;			/* List of followers */
	otag		*first_obj;			/* List of inventory */
	etag		*first_enm;			/* List of enemies */
	ttag		*first_tlk;			/* List of talk responses */
	struct 	room		*parent_rom;		/* Room creature is in */
	long		bank_bal;			/* Bank Balance	*/
} creature;


typedef struct queue_tag {				/* General queue tag data struct */
	int			index;
	struct queue_tag	*next;
	struct queue_tag	*prev;
} qtag;

typedef struct rsparse {				/* Sparse pointer array for rooms */
	room 			*rom;
	qtag			*q_rom;
} rsparse;

typedef struct csparse {				/* Sparse pointer array for creatures */
	creature		*crt;
	qtag			*q_crt;
} csparse;

typedef struct osparse {				/* Sparse pointer array for objects */
	object			*obj;
	qtag			*q_obj;
} osparse;
