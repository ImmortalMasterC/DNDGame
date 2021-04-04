/*
 * MTYPE.H:
 *
 *      #defines required by the rest of the program
 *
 *      Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 *		Mordor code - Heavily modified by Roy Wilson 
 */

/* socket port number */
#define PORTNUM         3000
 
/* DM's name */
#define DMNAME          "Styx"
#define DMNAME2         "Charon"
#define DMNAME3         "Miphon"
#define DMNAME4         ""
#define DMNAME5         ""
#define DMNAME6         ""
#define DMNAME7         ""
 
/* I/O buffer sizes */
#define IBUFSIZE        2048
#define OBUFSIZE        8192
 
/* File permissions */
#define s_IWRITE        00660 
#define s_IREAD         00006 
#define O_BINARY        0
#define ACC             00660
 
/* merror() error types */
#define FATAL           1
#define NONFATAL        0
 
/* pathnames */
#define ROOMPATH        "/app/mud/darbonne/rooms"
#define MONPATH         "/app/mud/darbonne/objmon"
#define OBJPATH         "/app/mud/darbonne/objmon"
#define PLAYERPATH      "/app/mud/darbonne/player"
#define DOCPATH         "/app/mud/darbonne/help"
#define POSTPATH        "/app/mud/darbonne/post"
#define BINPATH         "/app/mud/darbonne/bin"
#define LOGPATH         "/app/mud/darbonne/log" 
#define LIBPATH         "/app/mud/darbonne/lib" 
        
#define COMMANDMAX      5

/* Monster and object files sizes (in terms of monsters or objects) 	*/
#define MFILESIZE       100
#define OFILESIZE       100

/* memory limits */
#define RMAX            20000    /* Max number of rooms (was 9000) 	 */
#define CMAX            5000
#define OMAX            5000
#define PMAX            1024

#define RQMAX           800     /* Max number of these allowed in memory */
#define CQMAX           400     /* at any one time                       */
#define OQMAX           400

/* how often (in seconds) players get saved */
#define SAVEINTERVAL    1200

/* Save flags */
#define PERMONLY        1
#define ALLITEMS        0

/* Command status returns */
#define DISCONNECT      1
#define PROMPT          2
#define DOPROMPT        3

/* Warring Kingdoms */
#define AT_WAR 1		/* princes at war=1 / 0=peace    	*/
#define REWARD 1000		/* base value for pledge and rescind 	*/
#define MAXALVL 25		/*max size of exp level array	     	*/

/* Creature stats */
#define STR             1
#define DEX             2
#define CON             3
#define INT             4
#define PTY             5

/* Character classes */
#define ASSASSIN        1
#define BARBARIAN       2
#define CLERIC          3
#define FIGHTER         4
#define MAGE            5
#define PALADIN         6
#define RANGER          7
#define THIEF           8
#define BARD		9
#define MONK		10
#define DRUID		11
#define ALCHEMIST	12
#define CARETAKER       13
#define DM              14

/* Character races */
#define DWARF           1	/* Small race - can see in the dark 	*/
#define ELF             2	/* Medium race - can see in the dark 	*/
#define HALFELF         3	/* Medium race		*/	
#define HOBBIT          4	/* Small race 		*/
#define HUMAN           5	/* Medium race		*/
#define ORC             6	/* Medium race 		*/
#define HALFGIANT       7	/* Large race 		*/
#define GNOME           8	/* Small race 		*/
#define SATYR		9	/* Medium race		*/
#define HALFORC		10	/* Medium race		*/
#define OGRE		11	/* Large race 		*/
#define DARKELF		12	/* Medium race 		*/
#define CENTAUR		13	/* Large race 		*/

/* creature types */
#define PLAYER          0
#define MONSTER         1
#define NPC             2

/* Proficiencies */
#define SHARP           0
#define THRUST          1
#define BLUNT           2
#define POLE            3
#define MISSILE         4

/* object types */
#define ARMOR           5
#define POTION          6
#define SCROLL          7
#define WAND            8
#define CONTAINER       9
#define MONEY           10
#define KEY             11
#define LIGHTSOURCE     12
#define MISC            13
#define REPAIR          14
#define SHIT            505	/* this must be the object number in the database */


/* Spell Realms */
#define EARTH           1
#define WIND            2
#define FIRE            3
#define WATER           4

/* Daily use variables */
#define DL_BROAD        0       /* Daily broadcasts		*/
#define DL_ENCHA        1       /* Daily enchants		*/
#define DL_FHEAL        2       /* Daily heals			*/
#define DL_TRACK        3       /* Daily track spells 		*/
#define DL_DEFEC        4       /* Daily defecations 		*/
#define DL_CHARM	5	/* Daily charms			*/
#define DL_STRNG	6	/* Daily super-strengths	*/
#define DL_RCHRG	7	/* Daily wand recharges	 	*/
#define DL_SHRNK	8	/* Daily mouse-size	 	*/
#define DL_BROAE	9	/* Daily channel broadcasts	*/
#define DL_CONJR	10	/* Daily conjure		*/
#define DL_DISPL	11	/* Daily dispel			*/

/* Last-time specifications */
#define LT_INVIS        0
#define LT_PROTE        1
#define LT_BLESS        2
#define LT_ATTCK        3
#define LT_TRACK        4
#define LT_MSCAV        4
#define LT_MPICK        5
#define LT_STEAL        5
#define LT_DISTR        5
#define LT_PICKL        6
#define LT_JIMMY        6
#define LT_MWAND        6
#define LT_SERCH        7
#define LT_HEALS        8
#define LT_FROZE        8
#define LT_SPELL        9
#define LT_PEEKS        10
#define LT_PLYKL        11
#define LT_READS        12
#define LT_LIGHT        13
#define LT_HIDES        14
#define LT_TURNS        15
#define LT_HASTE        16
#define LT_DINVI        17
#define LT_DMAGI        18
#define LT_PRAYD        19
#define LT_PREPA        20
#define LT_LEVIT        21
#define LT_PSAVE        22
#define LT_RBRTH        23
#define LT_FLYSP        24
#define LT_RMAGI        25
#define LT_MOVED        26
#define LT_KNOWA        27
#define LT_HOURS        28 	/* Use this to change player's age	*/
#define LT_XXXXX        29	/* This flag is available		*/
#define LT_BRWAT        30
#define LT_SSHLD        31
#define LT_FEARS	33
#define LT_SILNC	34
#define LT_SINGS	35
#define LT_CHRMD	36
#define LT_MEDIT	37
#define LT_TOUCH	38
#define LT_SECCK	39
#define LT_STRNG	40
#define LT_SHRNK	41
#define LT_RFLCT	42
#define LT_EVEYE	43
#define LT_BLIND	44
/****************** no more allowed in the code *************************/

/* Wear locations */
#define MAXWEAR         20
#define BODY            1
#define ARMS            2
#define LEGS            3
#define NECK            4
#define NECK1           4
#define NECK2           5
#define HANDS           6
#define HEAD            7
#define FEET            8
#define FINGER          9
#define FINGER1         9
#define FINGER2         10
#define FINGER3         11
#define FINGER4         12
#define FINGER5         13
#define FINGER6         14
#define FINGER7         15
#define FINGER8         16
#define HELD            17
#define SHIELD          18
#define FACE            19
#define WIELD           20
/****************** no more allowed in the code *************************/

/* Spell flags */
#define SVIGOR          0       /* Vigor			*/
#define SHURTS          1       /* Hurt			 	*/
#define SLIGHT          2       /* Light		 	*/
#define SCUREP          3       /* Curepoison			*/
#define SBLESS          4       /* Bless			*/
#define SPROTE          5       /* Protection			*/
#define SFIREB          6       /* Fireball		 	*/
#define SINVIS          7       /* Invisibility			*/
#define SRESTO          8       /* Restore			*/
#define SDINVI          9       /* Detect-invisibility		*/
#define SDMAGI          10      /* Detect-magic			*/
#define STELEP          11      /* Teleport			*/
#define SBEFUD          12      /* Befuddle			*/
#define SLGHTN          13      /* Lightning			*/
#define SICEBL          14      /* Iceblade			*/
#define SENCHA          15      /* Enchant			*/
#define SRECAL          16      /* Word-of-recall		*/
#define SSUMMO          17      /* Summon		 	*/
#define SMENDW          18      /* Mend-wounds			*/
#define SFHEAL          19      /* Heal				*/
#define STRACK          20      /* Track			*/
#define SLEVIT          21      /* Levitation			*/
#define SRFIRE          22      /* Resist-fire			*/
#define SFLYSP          23      /* Fly				*/
#define SRMAGI          24      /* Resist-magic			*/
#define SSHOCK          25      /* Shockbolt			*/
#define SRUMBL          26      /* Rumble			*/
#define SBURNS          27      /* Burn				*/
#define SBLIST          28      /* Blister			*/
#define SDUSTG          29      /* Dustgust			*/
#define SWBOLT          30      /* Waterbolt			*/
#define SCRUSH          31      /* Stonecrush			*/
#define SENGUL          32      /* Engulf			*/
#define SBURST          33      /* Burstflame			*/
#define SSTEAM          34      /* Steamblast			*/
#define SSHATT          35      /* Shatterstone			*/
#define SIMMOL          36      /* Immolate			*/
#define SBLOOD          37      /* Bloodboil			*/
#define STHUND          38      /* Thunderbolt			*/
#define SEQUAK          39      /* Earthquake			*/
#define SFLFIL          40      /* Flamefill			*/
#define SKNOWA          41      /* Know-aura			*/
#define SREMOV          42      /* Remove-curse			*/
#define SRCOLD          43      /* Resist-cold			*/
#define SBRWAT          44      /* Breathe-water	 	*/
#define SSSHLD          45      /* Stone-shield		 	*/
#define SLOCAT          46      /* Mind-meld		 	*/
#define SDREXP          47      /* Drain experience	 	*/
#define SRMDIS          48      /* Cure-disease		 	*/
#define SRMBLD          49      /* Cure-blindess	 	*/
#define SFEARS          50      /* Fear			 	*/
#define SRVIGO		51	/* Room-vigor		 	*/
#define STRANO		52	/* Item-transport	 	*/
#define SBLIND		53	/* Cause-blindness	 	*/
#define SSILNC		54	/* Cause-silence	 	*/
#define SRMSIL		55	/* Remove-silence	 	*/
#define SFORTU		56	/* Fortune		 	*/
#define SSTRNG		57	/* Super-strength		*/
#define SSHRNK		58	/* Shrink to mouse-size		*/
#define SRFLCT		59	/* Reflect offensive spells	*/
#define SEVEYE		60	/* Evil eye			*/
#define SFREEZ		61	/* Freeze			*/
#define SHEXSP		62	/* Hex				*/
#define SCONJR		63	/* Conjure			*/
#define SENTNG		64	/* Entangle			*/
#define SKNOCK		65	/* Knock			*/
#define SABSLV		66	/* Absolution			*/
#define SDISPL		67	/* Dispel			*/
#define SRACID		68	/* Resist-Acid			*/

/*Trap types */
#define TRAP_PIT        1       /* Pit trap			*/
#define TRAP_DART       2       /* Poison dart trap		*/
#define TRAP_BLOCK      3       /* Falling block		*/
#define TRAP_MPDAM      4       /* MP damaging trap		*/
#define TRAP_RMSPL      5       /* Spell loss trap		*/
#define TRAP_NAKED      6       /* Player loses all items	*/
#define TRAP_ALARM      7       /* Monster alarm trap	 	*/

/* Spell casting types */
#define CAST            0

/* Room flags */
#define RSHOPP          0       /* Shoppe				*/
#define RDUMPR          1       /* Dump					 					*/
#define RPAWNS          2       /* Pawn Shoppe				*/
#define RTRAIN          3       /* Training class bits (3-6):  6=LSB	*/
#define RREPAI          7       /* Repair Shoppe			*/
#define RDARKR          8       /* Room is dark always			*/
#define RDARKN          9       /* Room is dark at night		*/
#define RPOSTO          10      /* Post office				*/
#define RNOKIL          11      /* Safe room, no playerkilling		*/
#define RNOTEL          12      /* Cannot teleport to this room		*/
#define RHEALR          13      /* Heal faster in this room		*/
#define RONEPL          14      /* 1-player only inside			*/
#define RTWOPL          15      /* 2-players only inside		*/
#define RTHREE          16      /* 3-players only inside		*/
#define RNOMAG          17      /* No magic allowed in room		*/
#define RPTRAK          18      /* Permanent tracks in room		*/
#define REARTH          19      /* Earth realm				*/
#define RWINDR          20      /* Wind realm				*/
#define RFIRER          21      /* Fire realm				*/
#define RWATER          22      /* Water realm				*/
#define RPLWAN          23      /* Player-dependent monster wanders	*/
#define RPHARM          24      /* Player harming room			*/
#define RPPOIS          25      /* Player poison room			*/
#define RPMPDR          26      /* Player mp drain room			*/
#define RPBEFU          27      /* Player befuddle room			*/
#define RNOLEA          28      /* Player cannot be summon out		*/
#define RPLDGK          29      /* Player can pledge in room		*/
#define RRSCND          30      /* Player can rescind in room		*/
#define RNOPOT          31      /* No potion room			*/
#define RPMEXT          32      /* Player magic spell extended		*/
#define RNOLOG          33      /* No player login			*/
#define RELECT          34      /* Election Booth			*/ 
#define RNDOOR		35	/* Indoors				*/
#define RSLIME		36	/* Floor slime - devours dropped items	*/
#define RJAILR		37	/* Jail					*/
#define RNHOME		38	/* Cannot use as home			*/
#define RSILVR		39	/* Repair shop for enchanted silver	*/
#define RPBANK		40	/* Bank			 		*/
#define RPSHOP		41	/* Player owned shop	 		*/

/* Player flags */
#define PBLESS          0       /* Player blessed			*/
#define PHIDDN          1       /* Hidden				*/
#define PINVIS          2       /* Invisibility				*/
#define PNOBRD          3       /* Don't show broadcasts		*/
#define PNOLDS          4       /* Don't show long description		*/
#define PFROZE          5       /* Player frozen - cannot move		*/
#define PHEXED          6       /* Hex spell				*/
#define PASSHL          7       /* Player is an asshole                 */
#define PPROTE          8       /* Protection spell			*/
#define PAUTOA          9       /* Auto attack for players		*/
#define PDMINV          10      /* DM Invisibility			*/
#define PNOFOL          11      /* Player does not allow following	*/
#define PMALES          12      /* Sex == male				*/
#define PNOCOP          13      /* No copulation			*/
#define PWIMPY          14      /* Wimpy mode				*/
#define PEAVES          15      /* Eavesdropping mode			*/
#define PPOISN          16      /* Poisoned				*/
#define PLIGHT          17      /* Light spell cast			*/
#define PPROMP          18      /* Display status prompt		*/
#define PHASTE          19      /* Haste flag (for rangers)		*/
#define PDMAGI          20      /* Detect magic				*/
#define PDINVI          21      /* Detect invisible			*/
#define PPRAYD          22      /* Prayer activated			*/
#define PLOANB          23      /* Has a bank loan			*/
#define PPREPA          24      /* Prepared for trap			*/
#define PLEVIT          25      /* Levitation				*/
#define PANSIC          26      /* Ansi Color				*/
#define PSPYON          27      /* Spying on someone			*/
#define PCHAOS          28      /* Chaotic/!Lawful			*/
#define PREADI          29      /* Reading a file			*/
#define PRFIRE          30      /* Resisting fire			*/
#define PFLYSP          31      /* Flying				*/
#define PRMAGI          32      /* Resist magic				*/
#define PKNOWA          33      /* Know alignment			*/
#define PNOSUM          34      /* Nosummon flag			*/
#define PIGNOR          35      /* Ignore all send			*/ 
#define PRCOLD          36      /* Resist-cold				*/
#define PBRWAT          37      /* Breathe water			*/
#define PSSHLD          38      /* Earth shield				*/
#define PPLDGK          39      /* Player pledged to a prince			*/
#define PKNGDM          40      /* Pledged to prince  0=RAGNAR, 1=RUTGER	*/
#define PDISEA          41      /* Diseased				*/
#define PBLIND          42      /* Blind				*/
#define PFEARS		43	/* Fearful				*/
#define	PSILNC		44	/* Silenced				*/
#define PCHARM		45	/* Charmed				*/
#define PNLOGN		46	/* Disable login messages		*/
#define PLECHO		47	/* Echo messages to sender		*/
#define PSECOK		48	/* Player has passed security check 	*/
#define PAUTHD		49	/* No-port that has been authorized 	*/
#define	PALIAS		50	/* DM is aliasing			*/
#define	PSTRNG		51	/* Super-strength 			*/
#define	PSHRNK		52	/* Shrunk to mouse-size	 		*/
#define	PRFLCT		53	/* Protected by reflect spell		*/
#define	PDETLK		54	/* Detect luck		 		*/
#define	PEVEYE		55	/* Suffering from evil eye		*/
#define PLOANU          56      /* Bank loan has been used	 	*/
#define PSPLIT          57      /* Split gold with the group		*/
#define PNUMBS          58      /* Numb 				*/
#define PCLOAK          59      /* Player is cloaked 			*/
#define PBERSK          60      /* Barbarian is berserk			*/
#define PRACID          61      /* Player resist acid 			*/

/* Monster flags */
#define MPERMT          0       /* Permanent monster			*/
#define MHIDDN          1       /* Hidden				*/
#define MINVIS          2       /* Invisible				*/
#define MTOMEN          3       /* Man to men on plural			*/
#define MDROPS          4       /* Don't add s on plural		*/
#define MNOPRE          5       /* No prefix				*/
#define MAGGRE          6       /* Aggressive				*/
#define MGUARD          7       /* Guards treasure			*/
#define MBLOCK          8       /* Blocks exits				*/
#define MFOLLO          9       /* Monster follows attacker		*/
#define MFLEER          10      /* Monster flees			*/
#define MSCAVE          11      /* Monster is a scavenger		*/
#define MMALES          12      /* Sex == male				*/
#define MPOISS          13      /* Poisoner				*/
#define MUNDED          14      /* Undead				*/
#define MUNSTL          15      /* Cannot be stolen from		*/
#define MPOISN          16      /* Poisoned				*/
#define MMAGIC          17      /* Can cast spells			*/
#define MHASSC          18      /* Has already scavenged something	*/
#define MBRETH          19      /* Breath weapon			*/
#define MMGONL          20      /* Harmed only by magic			*/
#define MDINVI          21      /* Detect invisibility			*/
#define MENONL          22      /* Harmed only by magic/ench.weapon	*/
#define MTALKS          23      /* Monster can talk interactively	*/
#define MUNKIL          24      /* Monster cannot be harmed		*/
#define MNRGLD          25      /* Monster has fixed amt of gold	*/
#define MTLKAG          26      /* Becomes aggressive after talking	*/
#define MRMAGI          27      /* Resist magic				*/
#define MBRWP1          28      /* MBRWP1 & MBRWP2 type of breath	*/
#define MBRWP2          29      /* 00 =fire, 01= ice, 10 =gas, 11= acid	*/
#define MENEDR          30      /* Energy (exp) drain	 		*/
#define MKNGDM          31      /* Monster is pledged			*/
#define MPLDGK          32      /* Players can pledge to monster	*/
#define MRSCND          33      /* Players can rescind to monster	*/
#define MDISEA          34      /* Monster causes disease		*/
#define MDISIT          35      /* Monster can dissolve items		*/
#define MPURIT          36      /* Player can purchase from monster	*/
#define MTRADE          37      /* Monster will give items		*/
#define MPGUAR          38      /* Passive exit guard			*/
#define MGAGGR          39      /* Monster aggro to good players	*/
#define MEAGGR          40      /* Monster aggro to evil players	*/
#define MDEATH          41      /* Monster has additional death desc	*/
#define MMAGIO		42	/* Monster cast magic % flag (prof 1)	*/
#define MRBEFD		43	/* Monster resists stun only		*/
#define MNOCIR		44	/* Monster cannot be circled		*/
#define MBLNDR		45	/* Monster blinds			*/
#define MDMFOL		46	/* Monster will follow DM		*/
#define MFEARS		47	/* Monster is fearful			*/
#define MSILNC		48	/* Monster is silenced			*/
#define MBLIND		49	/* Monster is blind			*/
#define MCHARM		50	/* Monster is charmed			*/
#define MMOBIL		51	/* Mobile monster			*/
#define MROBOT		52	/* Logic Monster			*/
#define MIREGP		53	/* Irregular Plural			*/
#define MSHEAL		54	/* Monster has slow heal - 4x		*/
#define	MEVEYE		55	/* Monster suffering from evil eye	*/
#define	MAKGDM		56	/* Monster aggro to pledged players-0/1	*/
#define MFEARD		57	/* Monster causes fear			*/
#define MPICKP		58	/* Monster is a pickpocket		*/
#define MFROZE		59	/* Monster is frozen in place		*/
#define MPLEDG          60      /* Pledged:  0=RAGNAR 1=RUTGER		*/
#define MHEXED		61		/* Under hex spell		*/
#define MCNOAG      	62  	/* Class aggressive except same class	*/
#define MTALKR      	63  	/* Talks to Rangers 			*/
#define MTALKD      	64  	/* Talks to Druids 			*/
#define MPBRIB      	65  	/* Perm monster can be bribed		*/
#define MNOBRB      	66  	/* Monster cannot be bribed		*/
#define MTIPOK      	67  	/* Monster accepts a tip		*/
#define MNOGEN      	68  	/* No gender				*/
#define MPASSW      	69  	/* Accepts a password                   */
#define MCSAGG      	70  	/* Aggressive to same class 		*/
#define MNPLEA      	71  	/* Monster will not grant mercy        	*/
#define MGDOBJ      	72  	/* Monster guards a single object      	*/
#define MNOEXT      	73  	/* Stays in the game			*/
#define MPORTR      	74  	/* Porter		 		*/
#define MGATEK      	75  	/* Gatekeeper   			*/

/* Object flags */
#define OPERMT      0   /* Permanent item (not yet taken)	*/
#define OHIDDN      1   /* Hidden				*/
#define OINVIS      2   /* Invisible				*/
#define OSOMEA      3   /* "some" prefix			*/
#define ODROPS      4   /* Don't add s on plural		*/
#define ONOPRE      5   /* No prefix				*/
#define OCONTN      6   /* Container object			*/
#define OWTLES      7   /* Container of weightless holding	*/
#define OTEMPP      8   /* Temporarily permanent		*/
#define OPERM2      9   /* Permanent INVENTORY item		*/
#define ONOMAG      10  /* Mages cannot wear/use it		*/
#define OLIGHT      11  /* Object serves as a light		*/
#define OGOODO      12  /* Usable only by good players		*/
#define OEVILO      13  /* Usable only by evil players		*/
#define OENCHA      14  /* Object enchanted			*/
#define ONOFIX      15  /* Cannot be repaired			*/
#define OCLIMB      16  /* Climbing gear			*/
#define ONOTAK      17  /* Cannot be taken			*/
#define OSCENE      18  /* Part of room description/scenery	*/
#define OSIZE1      19  /* OSIZE: 00=all wear, 01=small wear	*/
#define OSIZE2      20  /* 10=medium wear, 11=large wear	*/
#define ORENCH      21  /* Random enchantment flag		*/
#define OCURSE      22  /* The item is cursed			*/
#define OWEARS      23  /* The item is being worn		*/
#define OUSEFL      24  /* Can be used from the floor		*/
#define OCNDES      25  /* Container devours items		*/
#define ONOMAL      26  /* Usable by only females		*/
#define ONOFEM      27  /* Usable by only males			*/
#define ODDICE      28  /* Damage based on object nds		*/
#define OPLDGK      29  /* Pledge players may only use		*/
#define OKNGDM      30  /* Object belongs to kingdom 0/1	*/
#define OCLSEL      31  /* Class selective weapon		*/
#define OASSNO      32 	/* Class selective: assassin		*/
#define OBARBO      33 	/* Class selective: barbarian		*/
#define OCLERO      34 	/* Class selective: cleric		*/
#define OFIGHO      35 	/* Class selective: fighter		*/
#define OMAGEO      36 	/* Class selective: mage		*/
#define OPALAO      37 	/* Class selective: paladin		*/
#define ORNGRO      38 	/* Class selective: ranger		*/
#define OTHIEO      39 	/* Class selective: thief		*/
#define OBARDO	    40	/* Class selective: bard		*/
#define OMONKO	    41	/* Class selective: monk		*/
#define ODRUDO	    42	/* Class selective: druid		*/
#define OALCHO	    43	/* Class selective: alchemist		*/
#define ONSHAT	    44 	/* Weapon will never shatter		*/
#define OALCRT	    45  /* Weapon will always critical		*/
#define OLUCKY	    46  /* Item is a luck charm			*/
#define OIREGP	    47  /* Irregular plural	    		*/
#define OCURSW      48  /* The item is cursed and worn		*/
#define OTMPEN      49  /* Object is temporaily enchanted	*/
#define OSILVR      50  /* Object is made of enchanted silver	*/
#define ONUSED      51  /* Object is never used up/broken	*/
#define ONOLAW      52  /* Cannot be used by lawful players     */
#define OFASTS      53  /* Container object is fastenable	*/
#define OFASTD      54  /* Container object is fastened		*/
#define ONJIMY      55  /* Container fastener cannnot be jimmied	*/
#define OGUARD      56  /* Object is guarded			*/
#define ONTFIX      57  /* Object cannot be fixed with a tool   */
#define OCLOAK      58  /* Object cloaks player                 */
#define OPSALE      59  /* For sale by player                   */

/* Exit flags */
#define XSECRT      0   /* Secret		 		*/
#define XINVIS      1   /* Invisible				*/
#define XLOCKD      2   /* Locked		 		*/
#define XCLOSD      3   /* Closed		 		*/
#define XLOCKS      4   /* Lockable		 		*/
#define XCLOSS      5   /* Closable		 		*/
#define XUNPCK      6   /* Un-pickable lock	 		*/
#define XNAKED      7   /* Naked exit		 		*/
#define XCLIMB      8   /* Climbing gear required to go up	*/
#define XREPEL      9   /* Climbing gear require to repel	*/
#define XDCLIM      10  /* Very difficult climb			*/
#define XFLYSP      11  /* Must fly to go that way		*/
#define XFEMAL      12  /* Female only exit			*/
#define XMALES      13  /* Male only exit	 		*/
#define XPLDGK      14  /* Pledge player exit only		*/
#define XKNGDM      15  /* Exit for kingdom 0/1			*/
#define XNGHTO      16  /* Only open at night			*/
#define XDAYON      17  /* Only open during day			*/
#define XPGUAR      18  /* Passive guarded exit			*/
#define XNOSEE      19  /* Cannot use / see exit		*/
#define XPLSEL	    20	/* Class selective exit			*/
#define XPASSN	    21	/* Class:  assassin		 	*/
#define XPBARB	    22	/* Class:  barbarian			*/
#define XPCLER	    23	/* Class:  cleric			*/
#define XPFGHT	    24	/* Class:  fighter			*/
#define XPMAGE	    25	/* Class:  mage				*/
#define XPPALA	    26	/* Class:  paladin			*/
#define XPRNGR	    27	/* Class:  ranger			*/
#define XPTHEF	    28	/* Class:  thief			*/
#define XPBARD	    29	/* Class:  bard 			*/
#define XPMONK	    30	/* Class:  monk 			*/
#define XPDRUD	    31	/* Class:  druid			*/
#define XPALCH	    32	/* Class:  alchemist			*/
#define XPXXX1	    33	/* Class:  reserved			*/
#define XPXXX2	    34	/* Class:  reserved			*/
#define XPXXX3	    35	/* Class:  reserved			*/
#define XPXXX4	    36	/* Class:  reserved			*/
#define XSMALL	    37	/* Small race - dwarf/hobbit/gnome	*/
#define XLARGE	    38	/* Large race - half-giant/ogre/centaur	*/
#define XNOBIG	    39	/* No large race exit			*/
#define XRACES	    40	/* Race selective exit			*/
#define XRSDWF	    41	/* Race:  dwarf		 		*/
#define XRSELF	    42	/* Race:  elf		 		*/
#define XRSHEF	    43	/* Race:  half elf	 		*/
#define XRSHOB	    44	/* Race:  hobbit	 		*/
#define XRSHUM	    45	/* Race:  human		 		*/
#define XRSORC	    46	/* Race:  orc		 		*/
#define XRSHGT	    47	/* Race:  half giant			*/
#define XRSGNM	    48	/* Race:  gnome		 		*/
#define XRSSAT	    49	/* Race:  satyr		 		*/
#define XRSHOC	    50	/* Race:  half orc	 		*/
#define XRSOGR	    51	/* Race:  ogre		 		*/
#define XRSDEF	    52	/* Race:  dark elf	 		*/
#define XRSCNT	    53	/* Race:  centaur	 		*/
#define XRXXX1	    54	/* Race:  reserved	 		*/
#define XRXXX2	    55	/* Race:  reserved	 		*/
#define XRXXX3	    56	/* Race:  reserved	 		*/
#define XMOUSE	    57	/* Mouse-size only exit:  Shrink spell	*/
#define XNOEVL	    58	/* Evil players cannot use		*/
#define XNGOOD	    59	/* Good players cannot use		*/
#define XNPRNT	    60	/* Useable exit, but does not print	*/
#define XTPRNT	    61	/* No-print exit temporarily open	*/
#define XMONEY	    62	/* Can be opened by paid or bribed monster	*/
#define XNOINV	    63	/* Invisible player cannot use			*/
/******************** no more allowed in the code ***************************/


/* Object specials */
#define SP_MAPSC        1       /* Map or scroll	 	*/
#define SP_COMBO        2       /* Combination lock	 	*/
#define SP_NOSEL        3       /* No sell			*/
#define SP_HORNS        4       /* Horns of Zeth		*/
#define SP_ENCHT        5       /* Enchanting object		*/

/* Creature specials */
#define CRT_EXITS       1       /* Creature exits 		*/
#define CRT_PORTR       2       /* Porter			*/

/* obj_str and crt_str flags */
#define CAP             1
#define INV             2
#define SHD             3
#define MAG             4

#define RETURN(a,b,c)   Ply[a].io->fn = b; Ply[a].io->fnparam = c; return;

#define F_ISSET(p,f)    ((p)->flags[(f)/8] & 1<<((f)%8))
#define F_SET(p,f)      ((p)->flags[(f)/8] |= 1<<((f)%8))
#define F_CLR(p,f)      ((p)->flags[(f)/8] &= ~(1<<((f)%8)))

#define S_ISSET(p,f)    ((p)->spells[(f)/8] & 1<<((f)%8))
#define S_SET(p,f)      ((p)->spells[(f)/8] |= 1<<((f)%8))
#define S_CLR(p,f)      ((p)->spells[(f)/8] &= ~(1<<((f)%8)))

#define Q_ISSET(p,f)    ((p)->quests[(f)/8] & 1<<((f)%8))
#define Q_SET(p,f)      ((p)->quests[(f)/8] |= 1<<((f)%8))
#define Q_CLR(p,f)      ((p)->quests[(f)/8] &= ~(1<<((f)%8)))

#define BOOL(a)         ((a) ? 1 : 0)
#define EQUAL(a,b)      ((a) && (b) && \
                         (!strncmp((a)->name, (b), strlen(b)) || \
                          !strncmp((a)->key[0], (b), strlen(b)) || \
                          !strncmp((a)->key[1], (b), strlen(b)) || \
                          !strncmp((a)->key[2], (b), strlen(b))))       

#define LT(a,b)         ((a)->lasttime[(b)].ltime + (a)->lasttime[(b)].interval)

#define mrand(a,b)      ((a)+(rand()%((b)*10-(a)*10+10))/10)
#define mdice(a)        (dice((a)->ndice, (a)->sdice, (a)->pdice))
#define MIN(a,b)        (((a)<(b)) ? (a):(b))
#define MAX(a,b)        (((a)>(b)) ? (a):(b))

#define BEEP(a)         if( (a) > -1 ) { print((a), "%c",007);}
 
#define BLACK           30
#define RED             31
#define GREEN           32
#define YELLOW          33
#define BLUE            34
#define MAGENTA         35
#define CYAN            36
#define WHITE           37
#define BLACKBG         40
#define REDBG	        41
#define GREENBG         42
#define YELLOWBG        43
#define BLUEBG          44  
#define NORMAL          0  
#define BOLD            1
#define BLINK           5
#define REVERSE         7  

#define COLR(a,b)       if( (a) > -1 ) { print((a), "%c[%dm", 27, (b));}
#define ANSI(a,b)       if( (a) > -1 ) { if( F_ISSET(Ply[(a)].ply, PANSIC)) \
                            print((a), "%c[%dm", 27, (b));}
#define CLS(a)       if( (a) > -1 ) print((a), "\033[2J");
