#define MAX_TEXIT 5

typedef struct {
	int		room;
	char    *name1, *name2;
	int		exit1, exit2;
	char	*mess1, *mess2;
} tx_tag;

static t_toggle = 0; 
static tx_tag   time_x[MAX_TEXIT] = { 
	{1940, "rainbow", "rainbow", 0, 1942,
		" One end of a rainbow touches Darbonne.",
			"A rainbow touches both Darbonne and the earth below."},
	{1941, "rainbow", NULL, 1943, 0, NULL, NULL},
	{1942, "pool", "pool", 0, 1940, NULL, NULL},
	{1943, "cave", NULL, 1941, 0, NULL, NULL}
} ;
