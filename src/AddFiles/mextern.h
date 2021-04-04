/*
 * MEXTERN.H:
 *
 *	This file contains the external function and variable 
 *	declarations required by the rest of the program.
 *
 *	Copyright (C) 1991, 1992, 1993 Brett J. Vickers
 *
 */

#include <stdio.h>
#include <fcntl.h>

#ifdef DMALLOC
  #include "/usr/local/include/dmalloc.h"
#endif

#ifndef MIGNORE

extern int			Tablesize;
extern int			Cmdnum;
extern long			Time;
extern long			StartTime;
extern struct 		lasttime	Shutdown;
extern struct 		lasttime  Whisper[4];
extern int			Spy[PMAX];
extern int			Numlockedout;
extern lockout		*Lockout;

extern struct {
	creature	*ply;
	iobuf		*io;
	extra		*extr;
} Ply[PMAX];

extern struct {
	short		hpstart;
	short		mpstart;
	short		hp;
	short		mp;
	short		ndice;
	short		sdice;
	short		pdice;
} class_stats[11];

extern struct cmdstruct {
	char	*cmdstr;
	int		cmdno;
	int		(*cmdfn)();
} cmdlist[];

extern struct {
	char	*splstr;
	int		splno;
	int		(*splfn)();
} spllist[];

extern struct {
	int		splno;
	char	realm;
	int		mp;
	int		ndice;
	int		sdice;
	int		pdice;
	char	bonus_type;
} ospell[];

extern short	level_cycle[][10];
extern short	thaco_list[][20];
extern long		quest_exp[];
extern int		bonus[35];
extern char		class_str[][15];
extern char		race_str[][15];
extern char		race_adj[][15];
extern char		lev_title[][8][20];
extern char 	article[][10];
extern char 	number[][10];
extern long		needed_exp[25];

#endif

/* FILES1.C */
extern int	count_obj(), write_obj(), count_inv(), player_inv(), write_crt(),
			count_mon(), count_ite(), count_ext(), count_ply(), write_rom(), 
			read_obj(), read_crt(), read_rom();
extern void	free_obj(), free_crt(), free_rom();

/* FILES2.C */
extern int	load_rom(), load_mon(), load_obj(), load_ply(), 
			save_ply(), is_rom_loaded(), reload_rom(), resave_rom(), 
			read_forbidden_name();
extern void	put_queue(), pull_queue(), front_queue(), flush_rom(),
			flush_crt(), flush_obj(), resave_all_rom(), save_all_ply();

/* FILES3.C */
extern int	write_obj_to_mem(), write_crt_to_mem(), read_obj_from_mem(),
			read_crt_from_mem(), room_obj_count();

/* IO.C */
extern int	io_check(), accept_input(), locked_out(), addr_equal(),
			remove_wait();
extern void	sock_init(), sock_loop(), accept_connect(), output_buf(), 
			print(), handle_commands(), disconnect(), broadcast(), 
			broadcast_login(), broadcast_wiz(), broadcast_rom(), 
			broadcast_rom2(), broadcast_robot_rom(), add_wait(), 
			init_connect(), waiting(), child_died(), reap_children(),
			whisper();

/* BANK.C */
extern int	bank_balance(), bank_withdraw(), bank_deposit(), bank_transfer(),
			bank_loan();

/* COMMAND1.C */
extern void	login(), create_ply(), command(), parse(), suicide();
extern int	process_cmd(), passwd(), ply_suicide();

/* COMMAND2.C */
extern int	get(), inventory(), drop(), discard();
extern void	get_all_rom(), get_all_obj(), drop_all_rom(), drop_all_obj();

/* COMMAND3.C */
extern int	wear(), remove_obj(), equipment(), ready(), hold(), equip_weight(), obj_message();
extern void	wear_all(), remove_all(), equip_list();

/* COMMAND4.C */
extern int	health(), info(), peek(), help(), welcome(), who(), whois(), 
			pfinger(), vote();
extern void	info_2();

/* COMMAND5.C */
extern int	search(), hide(), quit(), steal(), stealgold(), distract(), 
            snatch();

/* COMMAND6.C */
extern int	go(), move(), openexit(), closeexit(), unlock(), lock(), picklock(),
			flee(), track(), sneak(), home(), follow(), lose(), group();

/* COMMAND7.C */
extern int	list(), buy(), sell(), value(), purchase(), selection(), trade(), 
			pay(), porter_return(); 

/* COMMAND8.C */
extern int	give(), repair(), renew(), prt_time(), savegame(), fix();
extern void	give_money();

/* COMMAND9.C */
extern int	bribe(), tip(), haste(), pray(), prepare(), use(), plead(), berserk();

/* COMMAND10.C */
extern int	pledge(), rescind(), train();
extern void lower_prof(), add_prof(), lose_all(), dissolve_item();

/* COMMAND11.C */ 
extern int 	look(), gaze(), describe_me(), set(), clear(), show();

/* COMMAND12.C */
extern int	bard_song(), bard_song2(), meditate();

/* COMMAND13.C */
extern int	fasten_cont(), unfasten_cont(), jimmylock(), store_item();

/* COMMAND14.C */
extern int	emote(), channelemote(), classemote(), class_send();

/* COMMAND15.C */
extern int	pay(), porter_return(); 

/* COMBAT.C */
extern int	update_combat(), attack(), attack_crt(), backstab(), throatslit(),
			rescue(), touch_of_death(), circle(), bash(), cripple();

/* MAGIC1.C */
extern int	cast(), teach(), study(), readscroll(), drink(), zap(),
			zap_obj(), offensive_spell();

/* MAGIC2.C */
extern int	vigor(), curepoison(), light(), protection(), mend();

/* MAGIC3.C */
extern int	bless(), invisibility(), restore(), turn(), befuddle();

/* MAGIC4.C */
extern int	detectinvis(), detectmagic(), teleport(), enchant();

/* MAGIC5.C */
extern int	recall(), summon(), heal(), magictrack(), levitate(),
			resist_fire(), fly();

/* MAGIC6.C */
extern int	resist_magic(), know_alignment(), remove_curse(), absolve();

/* MAGIC7.C */
extern int	resist_cold(), breathe_water(), earth_shield(), locate_player(), 
			drain_exp (), rm_disease(), object_send();

/* MAGIC8.C */
extern int	room_vigor(), fear(), rm_blind(), silence(), blind(), rm_silence(), 
			spell_fail(), fortune(); 

/* MAGIC9.C */	
extern int	recharge_wand(), transmute(), silver_renew(), super_strength(), 
			shrink(), reflect(), evil_eye(), freeze(), hex(), conjure(),
			entangle(), knock(), dispel(), resist_acid();

/* DM1.C */
extern int	dm_teleport(), dm_send(), dm_purge(), dm_users(), dm_echo(), 
			dm_flushsave(), dm_reload_rom(), dm_resave(), dm_create_obj(), 
			dm_perm(), dm_invis(), dm_ac(), dm_shutdown(), dm_force(), 
			dm_flush_crtobj(), dm_create_crt();

/* DM2.C */
extern int	stat_rom(), stat_crt(), stat_obj(), dm_stat(), dm_add(), 
			dm_add_rom(), dm_add_ext(), dm_add_crt(), dm_add_obj(), dm_spy(), 
			dm_password();

/* DM3.C */
extern void	link_rom(), expand_exit_name();
extern int	dm_set(), dm_set_rom(), dm_setobj(), dm_set_crt(), dm_set_ext(), 
			del_exit(), dm_log(), dm_loadlockout(), dm_finger(), dm_list(), 
			dm_info(), dm_set_misc(), dm_set_xflg(), dm_set_multi_word_ext(); 

/*  dm_set_xkey();  */

/* DM4.C */
extern int	dm_param(), dm_silence(), dm_broadecho(), dm_cast(), dm_group(), 
			dm_view(), dm_obj_name(), dm_crt_name();

/* DM5.C */
extern int	dm_inventory(), dm_spells(), dm_quests(); 

/* DM6.C */
extern int	dm_dust(), dm_attack(), list_enm(), list_charm(), dm_alias(), 
			dm_auth(), dm_flash(), dm_memory();

/* DM7.C */
extern int	dm_save_crt(), dm_add_crt(), dm_add_obj(), dm_save_obj(), 
			dm_add_talk(), dm_add_talk_key(), dm_add_talk_resp(),
			dm_add_talk_show(), dm_add_talk_save(), dm_find_db(), 
			dm_remove_db(); 

/* DM8.C */
extern int	dm_advance(), dm_exp(), dm_bal();

/* EDIT.C */
extern int  dm_replace(), desc_search(), dm_nameroom(), dm_append(), 
            dm_prepend(), dm_delete(), txt_parse(), dm_help();

/* PLAYER.C */
extern void	init_ply(), uninit_ply(), update_ply(), up_level(), down_level(), 
			add_obj_crt(), del_obj_crt(), compute_ac(), compute_thaco();
extern char	*title_ply(), *ply_prompt();
extern int	weight_ply(), max_weight(), profic(), mprofic(), mod_profic(), 
			has_light(), fall_ply(), luck(), compute_agility();
extern creature	*find_who(), *lowest_piety(), *low_piety_alg(), 
			*low_piety_pledge(), *low_piety_class();

/* CREATURE.C */
extern creature	*find_crt();
extern int 	add_enm_crt(), del_enm_crt(), is_enm_crt(), add_charm_crt(), 
			del_charm_crt(), is_charm_crt(), attack_mon(), mobile_crt();
extern void	end_enm_crt(), die(), temp_perm(), die_perm_crt(), 
			check_for_flee(), consider(), add_enm_dmg(), monster_combat();

/* ROOM.C */
extern void	add_ply_rom(), del_ply_rom(), add_obj_rom(), del_obj_rom(), 
			add_crt_rom(), del_crt_rom(), add_permcrt_rom(), add_permobj_rom(), 
			check_exits(), display_rom(), check_traps(), get_perm_obj();
extern int	count_vis_ply();
extern exit_	*find_ext();

/* OBJECT.C */
extern void	add_obj_obj(), del_obj_obj(), rand_enchant();
extern int	list_obj(), weight_obj();
extern object	*find_obj();

/* UPDATE.C */
extern void	update_game(), update_users(), update_random(), update_active(), 
			update_time(), update_exit(), update_shutdown(), add_active(), 
			del_active(), update_security();
extern int	crt_spell(), choose_item(), is_crt_active();

/* WHISPERS.C */
extern void	update_whisper();

 /* POST.C */
extern int	postsend(), postread(), postdelete(), notepad(), edit_news(),
			news();
extern void	postedit(), noteedit(), newsedit();

/* ACTION.C */
extern int	action();

/* MISC.C */
extern void	merror(), lowercize(), zero(), delimit(), view_file(), loge(),
			sort_cmds(), load_lockouts(), please_wait(), logn();
extern int	low(), up(), dice(), exp_to_lev(), dec_daily(), sort_cmp(),
			file_exists(), isnumber();
extern char	*crt_str(), *obj_str();

/* SPECIAL1.C */
extern int	special_read(), special_cmd();

/* TALK.C */
extern int	talk(), say(), yell(), gtalk(), psend(), broadsend(), ignore(),
			load_crt_tlk(), talk_crt_act();
extern void	talk_action();

/* SCREEN.C */
extern int	clear_screen();
extern void	setcolor(), rc(), scroll(), delline(), clrscr(), save_cursor(),
			load_cursor();

#ifdef COMPRESS
/* COMPRESS.C */
extern int	compress(), uncompress();
#endif

extern int nirvana(), list_act();
extern void log_act();
