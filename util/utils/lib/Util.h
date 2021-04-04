/*
 * 	util.h
 *
 *	Extern's and stuff needed for the mordor utils.
 *	Written by:  Steve Smith
*/

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <dirent.h>
#include "mstruct.h"
#include "mtype.h"
#include "mextern.h"




/* extern char	class_str[][15];
   extern char	race_str[][15];
*/

extern void	merror(), logf(), del_active(), free_obj(), free_crt(),
		front_queue(), put_queue(), pull_queue();

extern int	vigor(), read_crt(), load_obj(), load_ply(), read_obj_from_mem(), 
		read_crt_from_mem(), compress(), uncompress(), read_obj();



