extern int	psearch();
extern int	offensive_spell(), vigor(), curepoison(), light();
extern int	protection(), mend();
extern int	bless(), invisibility(), restore(), turn(), befuddle();
extern int	detectinvis(), detectmagic(), teleport(), enchant();
extern int	recall(), summon(), heal(), magictrack(), levitate(),
			resist_fire(), fly();
extern int	resist_magic(), know_alignment(), remove_curse();
extern int	resist_cold(), breathe_water(), earth_shield(),
			locate_player(), drain_exp (), rm_disease(), object_send();
extern int	room_vigor(), fear(), rm_blind(), silence(), blind(),
			spell_fail(), fortune(); 



struct {
	char 	*splstr;
	int	splno;
	int	(*splfn)();
}  
} spllist[] = {
	{ "vigor", SVIGOR },
	{ "hurt", SHURTS },
	{ "light", SLIGHT },
	{ "cure-poison", SCUREP },
	{ "bless", SBLESS },
	{ "protection", SPROTE },
	{ "fireball", SFIREB },
	{ "invisibility", SINVIS },
	{ "restore", SRESTO },
	{ "detect-invisible", SDINVI },
	{ "detect-magic", SDMAGI },
	{ "teleport", STELEP },
	{ "stun", SBEFUD },
	{ "lightning", SLGHTN },
	{ "iceblade", SICEBL },
	{ "enchant", SENCHA },
	{ "word-of-recall", SRECAL },
	{ "summon", SSUMMO },
	{ "mend-wounds", SMENDW },
	{ "heal", SFHEAL },
	{ "track", STRACK },
	{ "levitate", SLEVIT },
	{ "resist-fire", SRFIRE },
	{ "fly", SFLYSP },
	{ "resist-magic", SRMAGI },
	{ "shockbolt", SSHOCK },
	{ "rumble", SRUMBL },
	{ "burn", SBURNS },
	{ "blister", SBLIST },
	{ "dustgust", SDUSTG },
	{ "waterbolt", SWBOLT },
	{ "crush", SCRUSH },
	{ "shatterstone", SSHATT },
	{ "burstflame", SBURST },
	{ "steamblast", SSTEAM },
	{ "engulf", SENGUL },
	{ "immolate", SIMMOL },
	{ "bloodboil", SBLOOD },
	{ "thunderbolt", STHUND },
	{ "earthquake", SEQUAK },
	{ "flamefill", SFLFIL },
	{ "know-aura", SKNOWA },
	{ "remove-curse", SREMOV },
	{ "resist-cold", SRCOLD },
	{ "breathe-water", SBRWAT },
	{ "earth-shield", SSSHLD },
	{ "mind-meld", SLOCAT },
	{ "drain-exp", SDREXP },
	{ "cure-disease", SRMDIS },
	{ "cure-blindness", SRMBLD },
	{ "fear", SFEARS }, 
	{ "room-vigor", SRVIGO }, 
	{ "transport", STRANO },
	{ "blind", SBLIND },
	{ "silence", SSILNC },
	{ "remove-silence", SRMSIL },
	{ "fortune", SFORTU },
	{ "super-strength", SSTRNG },	
	{ "shrink", SSHRNK },	
	{ "reflect", SRFLCT },	
	{ "evil-eye", SEVEYE },	
	{ "freeze", SFREEZ },	
	{ "hex", SHEXSP },	
	{ "@", -1,0 }
}
