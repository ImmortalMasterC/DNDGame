/*   Gstat.c 	Copywrite (C) 1991, 1997			*/
/*		by Brett Vickers and Brooke Paul			*/
/* patched for the MORDOR 4.0 series by William Briesemaster	*/


#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include "lib/mstruct.h"
#include "lib/mextern.h"

#define isdigit(a)		((a)>='0' && (a)<='9')
#define isupper(a)		((a)>='A' && (a)<='Z')

int 	Port;
char 	report = 1;

typedef struct n_node {
    char    name[21];
    char    type;
    int     level;
    long    exp;
    long    time;
    long    gold;
    struct  n_node *next;
} node;

    node	*d_lst, *d_last;
    node	*l_lst, *l_last;
    node	*g_lst, *g_last;
    node	*t_lst, *t_last;
    node	game_max[3];

    int		cls_lvl[15][26];
    int		ctotal[15];
    int		rtotal[14];
    int		gender[2];
    int		level, statl;

    long	age, gold;
    char	dm;

extern  int gstat();
extern  void info_print();

main(argc, argv)
int 	argc;
char 	*argv[];

{
    int     n = 0;
    int     total = 0;
    int     i, j;
    DIR     *dirfd;
    struct 	dirent  *dirp;

    for(i = 1; i < argc; i++) {
        if(!strncmp(argv[i], "-d", 2)) {
            dm = 1;
            continue;
        }

        if(argv[i][0] != '-' || (i + 1) >= argc) {
			printf("syntax: gstat [-glst #] [-d] [-P <players ...>]\n");
			printf("\toptions: g = gold\n");
			printf("\toptions: l = level\n");
			printf("\toptions: s = stats\n");
			printf("\toptions: t = time\n");
			printf("\toptions: d = dm\n");
            return;
        }

        if(argv[i][1] == 'P') {
            n = i;
            break;
        }

        if(!isdigit(argv[i + 1][0])) {
            printf("syntax error.\n");
            return;
        }

        switch(argv[i][1]) {
            case 'g':
                gold = atol(argv[i + 1]);
                gold = (gold < 0) ? 0: gold;
                break;
            case 'l':
                level = atoi(argv[i + 1]);
                if(level > 26 || level < 1)
                    level = 0;
                break;
            case 's':
                statl = atoi(argv[i + 1]);
                if(statl > 26 || statl < 1)
                    statl = 0;
                break;
            case 't':
                age = atol(argv[i + 1]);
                age = (age < 0) ? 0 : age;
                break;
           default:
                printf("invalid flag\n");
                return;

        }
        i++;
    }

    if(dm) {     
        d_lst = (node *)malloc(sizeof(node));
        d_lst->next = 0;
        d_last = d_lst;
    }

    if(age) {
		printf("Searching for players with %d age+\n", age);
        t_lst = (node *)malloc(sizeof(node));
        t_lst->next = 0;
        t_last = t_lst;
    }

    if(level) {
		printf("Searching for players with %d level+\n", level);
        l_lst = (node *)malloc(sizeof(node));
        l_lst->next = 0;
        l_last = l_lst;
    }

    if(gold) {
		printf("Searching for players with %d gold+\n", gold);
        g_lst = (node *)malloc(sizeof(node));
        g_lst->next = 0;
        g_last = g_lst;
    }

	if(statl)
		printf("Print stats for %d+ level players.\n", statl);

    if(n) 
        for(i = n; i < argc; i++) {
            if(!isupper(argv[i][0]))
                continue;   
            total += gstat(argv[i]);
    	}
    else {
        if((dirfd = opendir(PLAYERPATH)) == NULL) {
            printf("Directory could not be opened.\n");
            return;
        }
        while((dirp = readdir(dirfd)) != NULL) {
            if(dirp->d_name[0] == '.')
                continue; 
            if(!isupper(dirp->d_name[0]))
                continue;   
            total += gstat(dirp->d_name);
        }       
        closedir(dirfd);    
    }

    info_print();
    return;      
}

/*************************************************************************/

int gstat(name)
char *name;

{
    creature    *player;

	if(!strcmp(name, "BACKUP")) {
		return (0);
	}
	if(load_ply(name, &player) < 0) {
		printf("Player (%s) Load Error.\n", name);
		return (0);
	}

    if(player->level >=  statl) {
        if(player->level > 0 && player->level < 26)
            cls_lvl[player->class][player->level - 1] += 1;
        else
			cls_lvl[player->class][25] += 1;

    	if(player->race > 0  && player->race < 14)
    	    rtotal[player->race]++;
    	else
    	    rtotal[0]++;
    	
    	if(F_ISSET(player, PMALES))
    	    gender[0]++;
    	else
    	    gender[1]++;
    }

	if(player->experience > game_max[0].exp && player->class < CARETAKER) {
       	strcpy(game_max[0].name, player->name);
       	game_max[0].type = player->class;
       	game_max[0].level = player->level;
       	game_max[0].gold = player->gold;
       	game_max[0].exp = player->experience;
       	game_max[0].time = player->lasttime[LT_HOURS].interval;
	}

	if(player->gold > game_max[1].gold && player->class < CARETAKER) {
		strcpy(game_max[1].name, player->name);
		game_max[1].type = player->class;
		game_max[1].level = player->level;
		game_max[1].gold = player->gold;
		game_max[1].exp = player->experience;
		game_max[1].time = player->lasttime[LT_HOURS].interval;
	}

	if(player->lasttime[LT_HOURS].interval > game_max[2].time &&
		player->class < CARETAKER) {
			strcpy(game_max[2].name, player->name);
			game_max[2].type = player->class;
			game_max[2].level = player->level;
			game_max[2].gold = player->gold;
			game_max[2].exp = player->experience;
			game_max[2].time = player->lasttime[LT_HOURS].interval;
	}

	if(dm)
		if(player->class >= CARETAKER || player->class == 0) {
			strcpy(d_last->name, player->name);
			d_last->type = player->class;
			d_last->level = player->level;
			d_last->gold = player->gold;
			d_last->exp = player->experience;
			d_last->time = player->lasttime[LT_HOURS].interval; 
			d_last->next = (node *)malloc(sizeof(node));
			d_last->next->next = 0;
			d_last = d_last->next;
		}

	if((level > 0) && player->level >= level) {
		strcpy(l_last->name, player->name);
		l_last->type = player->class;
		l_last->level = player->level;
		l_last->gold = player->gold;
		l_last->exp = player->experience;
		l_last->time = player->lasttime[LT_HOURS].interval; 
		l_last->next = (node *)malloc(sizeof(node));
		l_last->next->next = 0;
		l_last = l_last->next;
	}

	if((gold > 0L) && player->gold >= gold) {
		strcpy(g_last->name, player->name);
		g_last->type = player->class;
		g_last->level = player->level;
		g_last->gold = player->gold;
		g_last->exp = player->experience;
		g_last->time = player->lasttime[LT_HOURS].interval; 
		g_last->next = (node *)malloc(sizeof(node));
		g_last->next->next = 0;
		g_last = g_last->next;
	}

	if((age > 0L) && player->lasttime[LT_HOURS].interval >= age) {
		strcpy(t_last->name, player->name);
		t_last->type = player->class;
		t_last->level = player->level;
		t_last->gold = player->gold;
		t_last->exp = player->experience;
		t_last->time = player->lasttime[LT_HOURS].interval; 
		t_last->next = (node *)malloc(sizeof(node));
		t_last->next->next = 0;
		t_last = t_last->next;
	}

	free_crt(player);
	return (1);
}

/********************************************************************/
/*   print_info function prints out all the game stats and other 	*/
/*   requested info about the game 									*/
/********************************************************************/

void info_print()

{
    int     i, j;
    int     all = 0, total = 0;
    node    *tmp;
    long    ptime;

	if(!dm && !level && !age && !gold) {
	    for(i = 0; i < 13; i++) {
	        for(j = 0; j < 26; j++)
	            ctotal[i] += cls_lvl[i][j];
	        if(ctotal[i])
	        	all += ctotal[i];
		}

    	all = (!all) ? 1 : all;
    	printf("\nClass/Level Breakdown:\n");
    	for(j = 0; j < CARETAKER; j++) {
    	    printf("%s: %d (%d)\n",class_str[j], ctotal[j],
    	    ctotal[j] - cls_lvl[j][0]);
    	    for(i = 0; i < 26; i++) {
    	        if(i == 13)
    	            printf("\n");
    	        printf("%-5d ", cls_lvl[j][i]);
			}
    	    printf("\n\n");
		}

    	printf("\n\t\t     ------Class Breakdown-------\n");

    	for(i = 1; i < CARETAKER; i++) {
    		printf("%12s: %4d (%.2f%%) \t\t%12s: %4d (%.2f%%)\n", 
				class_str[i], ctotal[i], 
					(float)((float)ctotal[i]/(float)all) * 100, 
						class_str[i + 1], ctotal[i + 1],
							(float)((float)ctotal[i + 1]/(float)all) * 100);
    		i++;
    	}

    	printf("\n\n\t\t     -------Race Breakdown-------\n");

		for(i = 0; i < 13; i++) {
			printf("%12s: %4d (%.2f%%) \t\t%12s: %4d (%.2f%%)\n",
				race_str[i], rtotal[i], 
					(float)((float)rtotal[i]/(float)all) * 100,
						race_str[i + 1], rtotal[i + 1], 
							(float)((float)rtotal[i + 1]/(float)all) * 100);
			i++;
		}

	
    	printf("\n\n-------Gender Breakdown-------\n");

    	printf("Male Characters: \t%d \t(%.1f%%)\n", gender[0],
    	    (float)((float)gender[0]/(float)all) * 100);
    	printf("Female Characters: \t%d \t(%.1f%%)\n", gender[1],
    	    (float)((float)gender[1]/(float)all) * 100);
		total = gender[0] + gender[1];
    	printf("Total of all Players:   %d\n", total);

    	printf("\n\nPlayer with most:\n");

    	for(i = 0; i < 3; i++) {
    		ptime = game_max[i].time;
        	if(i == 0) 
				printf("Experience:\n");
    		else if(i == 1) 
				printf("Gold:\n");
    		else 
				printf("Play Time:\n");
        	printf(" %-15s %.4s %4d lvl %9ld exp %9ld gld  %02dd %02dh %02ds\n",
        	    game_max[i].name, class_str[game_max[i].type], 
					game_max[i].level, game_max[i].exp, game_max[i].gold, 
						ptime/86400L, (ptime %86400L)/3600L, 
							(ptime % 3600L)/60L);
    	}	
	}

    if(dm) {
        printf("\nDungeon Masters and Caretakers:\n");
        while(d_lst->next) {
            printf("%-20s (%-9s) %-5d\n",
            d_lst->name, class_str[d_lst->type], d_lst->level);
            tmp = d_lst->next;
            free(d_lst);
            d_lst =  tmp;
        }
        printf("\n");
    }

    if(level) {
        printf("\nNames of Players %d+ level.\n", level);
        while(l_lst->next) {
        	printf("%-15s %.4s %4d lvl %9ld exp %9ld gld %11ld age\n",
        	    l_lst->name, class_str[l_lst->type], l_lst->level, l_lst->exp,
        	    	l_lst->gold, l_lst->time);
			tmp = l_lst->next;
			free(l_lst);
			l_lst =  tmp;
		}
        printf("\n");
    }


    if(age) {
        printf("\nNames of Players %d+ seconds old.\n", age);
        while(t_lst->next) {
        	printf("%-15s %.4s %4d lvl %9ld exp %9ld gld %11ld age\n",
        	    t_lst->name, class_str[t_lst->type], t_lst->level, t_lst->exp,
        	    	t_lst->gold, t_lst->time);
			tmp = t_lst->next;
			free(t_lst);
			t_lst =  tmp;
        }
        printf("\n");
    }

    if(gold) {
        printf("\nNames of Players %d+ gold.\n", gold);
        while(g_lst->next) {
        	printf("%-15s %.4s %4d lvl %9ld exp %9ld gld %11ld age\n",
        	    g_lst->name, class_str[g_lst->type], g_lst->level, g_lst->exp,
        		    g_lst->gold, g_lst->time);
            tmp = g_lst->next;
            free(g_lst);
            g_lst =  tmp;
        }
        printf("\n");
    }
}
