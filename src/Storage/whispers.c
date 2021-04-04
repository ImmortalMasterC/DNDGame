/*
 * WHISPERS.C:
 *
 *	Routines to broadcast whispers
 *  Written by Roy Wilson
 *
 *  susurrus \su-SUHR-uhs\, noun:
 *  A whispering or rustling sound; a murmur.
 */

#include "mstruct.h"
#include "mextern.h"
#include "update.h"
#include <stdlib.h>
#include <sys/signal.h>

static long last_whisper_update;
short       Rnd_update_interval = 13;

/************************************************************************/
/*			update_whisper					*/
/************************************************************************/
/*	Broadcasts whispers on the wind.				*/	

void update_whisper(t)
long	t;

{
int	j, wtime, n = 0, k;

	last_whisper_update = t;
	wtime = (int)(Time % 24L);	
	
	for(j = 0; j < 5; j++) {	
		if(Whisper[j].ltime + Whisper[j].interval < t) {
	    	switch (j) {
	    		case 0:
				if(mrand(1, 100) > 80) {
	    				n = mrand(0, 2);
                    			n -= 1;
                    			n = Whisper[j].misc + n;
                    			if(n < 0 || n > 4)
                       				n = 0;
						switch (n) {
							case 0:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** The shades of ancient druids still watch over the Island of Anglesey.");

else if(k == 2)
whisper("** A flash of lightning is followed by a roll of thunder.");

else if(k == 3)
whisper("** A quiet voice whispers something about the Island of Anglesey.");

else if(k == 4)
whisper("** Was that a scream of terror or merely the wind?");

else if(k == 5)
whisper("** The freezing wind moans... or is it something else?");

else if(k == 6)
whisper("** Beware the dragons of Anglesey who lay in wait near the island.");

else if(k == 7)
whisper("** Several screams occur, then nothing but the sound of the wind.");

else if(k == 8)
whisper("** A frigid chill passes through you for just a moment, and then is gone.");
else if(k == 9)
whisper("** It is the witch Maggia who controls the undead.");

else
whisper("** Some say the Island of Anglesey is a myth and better forgotten.");

								break;


    						case 1:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A ghostly figure hovers over you for a moment, then disappears.");

else if(k == 2)
whisper("** The spirit of a dead wizard points his finger and silently screams at you.");

else if(k == 3)
whisper("** A plane of the spirit world seems to impinge on this part of the world.");

else if(k == 4)
whisper("** An etheral figure of a murdered dwarf floats silently by.");

else if(k == 5)
whisper("** Is that the wind howling with the sound of a tortured creature?");

else if(k == 6)
whisper("** If you look hard, you can make out spirit figures in another world.");

else if(k == 7)
whisper("** There is the sound of banshees screaming in the distance.");

else if(k == 8)
whisper("** A large and grotesque mouth is forming in the air before you.");

else if(k == 9)
whisper("** Maggia can make even the undead scream.");

else {
whisper("** The luminescent face of a disembodied blind man stares at you");
whisper("** from sightless eyes, then drifts away into the shadows.\n");
}

								break;
						}


    					Whisper[j].misc = (short)n;
    					Whisper[j].interval = 3200;
    				}
    				break;
				
    			case 1:   
					if(mrand(1, 100) > 50 && wtime > 6 && wtime < 20) {
    					n = mrand(0, 2);
    					n -= 1;
    					n = Whisper[j].misc + n;
    					if(n < 0 || n > 4) 
							n = 0;
    					switch (n) {
    						case 0:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A cold dampness is in the air.");

else if(k == 2)
whisper("** Is it the coldness that makes you shiver?");

else if(k == 3)
whisper("** The rain makes tiny sizzling sounds as it strikes metal.");

else if(k == 4)
whisper("** Leather and metal are poor garb in cold, wet weather.");

else if(k == 5)
whisper("** There are secrets to be learned in the burned library at Kayleigh.");

else if(k == 6)
whisper("** Chzye wenste artirg zeje mureed ente Darbonne?");

else if(k == 7)
whisper("** A gathering of witches will happen at the blood moon.");

else if(k == 8)
whisper("** The power of Alith holds the thirteen witches at bay.");

else if(k == 9)
whisper("** The undead walk when the witch Maggia commands it.");

else
whisper("** The air is as cold as the Witch of Glamis.");

								break;


						   	case 1: 
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** The Witch of Elmoren works in stone.  Have you seen her work?");

else if(k == 2)
whisper("** For just a moment, there is a wet, slithery sound close to you.");

else if(k == 3)
whisper("** The air takes on a strange, silvery color around you.");

else if(k == 4)
whisper("** Thirteen witches have vowed to destroy Darbonne.");

else if(k == 5)
whisper("** Fog does the bidding of the Witch of Strathmire.");

else if(k == 6)
whisper("** Find the secret room at Holyrood Abbey, before doth Gowrie, the ice witch.");

else if(k == 7)
whisper("** Thirteen stones in a circle.  The witch's symbol.");

else if(k == 8)
whisper("** A champion is needed to best Gowrie, the ice witch.");

else if(k == 9)
whisper("** Targon bested three witches, but there were four.");

else
whisper("** Living stone statues protest the Witch of Elmoren.");

								break;


						    case 2:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** The moan of long-dead warriors is heard from afar.");

else if(k == 2)
whisper("** A shriek, as that of harpies, comes from overhead.");

else if(k == 3)
whisper("** Strange words drift from the darkness.");

else if(k == 4)
whisper("** The cold air muffles strange words behind you.");

else if(k == 5)
whisper("** The sound of your name is whispered behind you.");

else if(k == 6)
whisper("** Some say Trokkawilff still resides in the cellars of Ragnar's castle.");

else if(k == 7)
whisper("** The copper dragons can take you to Kayleigh.");

else if(k == 8)
whisper("** Are there untold treasures beneath the castles of the two princes?");

else if(k == 9)
whisper("** Four runes hold the key to Darbonne's past.");

else
whisper("** A low voice whispers nearby in a strange tongue.");

								break;


						    case3:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A fetid smell wafts from somewhere nearby.");

else if(k == 2)
whisper("** The putrid smell of a rotting corpse comes on the air.");

else if(k == 3)
whisper("** Rotting flesh and the smell of death is somewhere near.");

else if(k == 4)
whisper("** The stink of dead and bloated bodies wafts from somewere.");

else if(k == 5)
whisper("** The deadly smell of almonds is close by.  Be careful.");

else if(k == 6)
whisper("** The odor of recent death is close at hand.");

else if(k == 7)
whisper("** Did Trokkawilff truely die in the catacombs beneath Ragnar's castle?");

else if(k == 8)
whisper("** Alith reassemblies the colored parchments... But only Zeth knew of the dark ones.");

else if(k == 9)
whisper("** If you discover the name of the Witch of Fear she will lose some of her power.");

else
whisper("** The first plane of hell is Avernus, and Fenris guards the gate.");

								break;


						    case4:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** There is a smell of disease and death around you.");

else if(k == 2)
whisper("** Disease rides on the air, striking randomly.");

else if(k == 3)
whisper("** Disease is the hidden vanguard force of an evil invasion.");

else if(k == 4)
whisper("** Perhaps the very air you breath is poisoned with the Red Death.");

else if(k == 5)
whisper("** A Red Death haze floats toward you, enveloping everything.");

else if(k == 6)
whisper("** The Red Death has the peculiar smell of almonds.");

else if(k == 7)
whisper("** The stench of burning flesh comes through the air.");

else if(k == 8)
whisper("** The silent scream of the Warlock of Kayleigh pierces your mind.");

else if(k == 9)
whisper("** During the great wars, Loki served both of good and evil.");

else
whisper("** Tiamat, the chromatic dragon of hell, has five heads.");

								break;
						}

						Whisper[j].misc = (short)n;
						Whisper[j].interval = 1600;
					}
					break;


    			case 2:  
					if(mrand(1, 100) > 50) { 
						n = mrand(0, 2);
						n -= 1;
						n = Whisper[j].misc + n; 
						if(n < 0 || n > 4) 
							n = 0;
						switch (n) {
							case 0:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** The air snaps with sparkles of color.");

else if(k == 2)
whisper("** Magicke swirls about you, causing sparkles in the air.");

else if(k == 3)
whisper("** Magicke has been used only recently in this place.");

else if(k == 4)
whisper("** The power of an unseen sorcerer freezes the air around you.");

else if(k == 5)
whisper("** A mage's spell was cast here recently.  The air still sparkles.");

else if(k == 6)
whisper("** The residue of pixie dust still lingers in the air.");

else if(k == 7)
whisper("** A foul smelling gas indicates a spell failed here.");

else if(k == 8)
whisper("** Did you feel that shiver?  The Witch of Fear might be close!");

else if(k == 9)
whisper("** The Norsemen were gallent allies in the Great Battle and were rewarded with land in Darbonne.");

else
whisper("** Flashes of color indicate that the air is charged with magicks.");

								break;


					    	case 1:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** The air stirs, coming from the east.");

else if(k == 2)
whisper("** For just a moment, the air is thick with old battle cries.");

else if(k == 3)
whisper("** Ghostly battles swirl around you, and suddenly disappear.");

else if(k == 4)
whisper("** The spirit of a red dragon roars with rage from the next plane.");

else if(k == 5)
whisper("** Ancient messages are carried by the north wind.");

else if(k == 6)
whisper("** Vaguely seen demons of hell appear as etheral figures, then disappear.");

else if(k == 7)
whisper("** Beware the abishai of Avernus, for they serve Tiamat.");

else if(k == 8)
whisper("** Beware the tunnels beneath Castle Ragnar.  Beware Trokkawilff!");

else if(k == 9)
whisper("** The Witch of Fear and the Witch of Shadows share a familiar.");

else
whisper("** Stories of ancient battles are borne on the eastern breezes.");

								break;


						    case 2:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A strong wind, full of old images, swirls across the land.");

else if(k == 2)
whisper("** Old battle cries stir in the swirling wind.");

else if(k == 3)
whisper("** Garth and Caledon look down in wonder at Darbonne.");

else if(k == 4)
whisper("** Perhaps it was the face of a troll that just peeked at you?");

else if(k == 5)
whisper("** Did you just feel a quick hand at your side?");

else if(k == 6)
whisper("** You almost hear a name in the wind.  Caledon...?");

else if(k == 7)
whisper("** There was a city long ago on the coast called Oceancrest.  Legend?");

else if(k == 8)
whisper("** King Gylfe of the Vikings brought his family and his warriors to the Great Battle.");

else if(k == 9)
whisper("** Across the River Leffey and south.  That is where you will find the Vikings.");

else {
whisper("** Ghosts of long dead adventurers suddenly swirl about you,");
whisper("** and just as quickly they are gone.  Did you hear your name?");
								}

								break;


					    	case 3:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** The gusts of wind bring cold sheets of rain to Mordor.");

else if(k == 2)
whisper("** The hammering rain drives across Mordor.");

else if(k == 3)
whisper("** The drizzle of an acid rain falls quietly.");

else if(k == 4)
whisper("** Messages to the court speak of Spindrill the warlock attacking Isengard.");

else if(k == 5)
whisper("** The metal parts of your armor sizzle as the acid rain spatters you.");

else if(k == 6)
whisper("** Garth was the dark one.  He knew all the secrets of Mordor.");

else if(k == 7)
whisper("** Caledon was followed by Styx, and they built Oceancrest.");

else if(k == 8)
whisper("** The Vikings came in long boats, but they never left the floating city of Darbonne.");

else if(k == 9)
whisper("** Find the Stone of Odin and discover its power.");

else
whisper("** Cold rain is hammered across all of Mordor.");

								break;


						    case 4:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** Gale winds bring black clouds scudding across the sky.");

else if(k == 2)
whisper("** The Witch of Argath has been known to ride the black clouds.");

else if(k == 3)
whisper("** South of Sprite Lough and west of Quickhand the Norsemen dwell.");

else if(k == 4)
whisper("** Agatha is the Witch of the Sea, and summers on Angelsey Island.");

else if(k == 5)
whisper("** The Witch of the Sea sends the fog.  Her name is Agatha.");

else
whisper("** Roiling black clouds scud before the wind over Mordor.");

							    break;
						}


						Whisper[j].misc = (short)n;
						Whisper[j].interval = 900;
					}
					break;

    			case 3:  
					if(mrand(1,100) > 50) {
						n = mrand(0, 2);                        	
						n -= 1;
						n = Whisper[j].misc + n; 
						if(n < 0 || n > 6) 
							n = 0;
						switch (n) {
							case 0:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** The eternal fog silently curls around your body.");

else if(k == 2)
whisper("** Something hidden in the fog brushes against you.");

else
whisper("** The fog seems almost alive, the way it wraps itself around you.");

								break;


							case 1:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** Death is hidden in the fog.  Be careful you do not find it.");

else if(k == 2)
whisper("** Tendrils of fog creep about you, bringing the scent of death.");

else if(k == 3)
whisper("** It was Ereborea and her sister Arigreil that led the rear guard.");

else if(k == 4)
whisper("** Arigreil was the younger, but she fought as bravely as Targon, her king.");

else if(k == 5)
whisper("** Did you know that Targon ruled the elves during the Age of Stars?");

else if(k == 6)
whisper("** The Hall of Insanity stretches to infinity, but only in one direction.");

else if(k == 7)
whisper("** Zeth created the Hall of Insanity during his sickness.");

else if(k == 8)
whisper("** The Battle of Unending Tears did not end in victory for the elves.");

else
whisper("** An unknown danger awaits in the mist.  Be careful.");

								break;


							case 2:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** Bards whisper that the Red Death was brewed by the Witch of Argath.");

else if(k == 2)
whisper("** The Red Death has visited many.  Beware the Witch of Argath.");
	
else if(k == 3)
whisper("** The Valley of Rainflowers may be the home of the Witch of Argath.");

else if(k == 4)
whisper("** Boran guards the way to the Valley of Rainflowers and to Mebelith.");

else if(k == 5)
whisper("** The Valley of Rainflowers may be the source of the Red Death.");

else if(k == 6)
whisper("** Some say the Red Death is brought by the eternal acid rain.");

else if(k == 7)
whisper("** The mountain giants guard the sealed cave of Halaster, the mad wizard.");

else if(k == 8)
whisper("** Her name is Mebelith, and she lives in the Valley of Rainflowers.");

else if(k == 9)
whisper("** Halaster, the wizard sealed in a mountain cave, was driven mad by his wife.");

else
whisper("** At times, the rain takes on a strange reddish color, almost like blood.");

								break;

							case 3:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A deadly acid rain falls quietly across Mordor.");
	
else if(k == 2)
whisper("** The Witch of Argath can often be seen above the acid rain.");

else if(k == 3)
whisper("** Beware the towers of Argath.  Death awaits at every turn.");

else if(k == 4)
whisper("** There is knowledge in the elven city of Chandre.  And death.");

else if(k == 5)
whisper("** The Blue Horse Hills have many mysteries.");

else if(k == 6)
whisper("** Look for the rainbow.  It will take you many places.");

else if(k == 7)
whisper("** The Enchanted Forest lies near the tower of Argath.");

else if(k == 8)
whisper("** Cloud giants guard the apex of the rainbow, but welcome fighters of evil.");

else
whisper("** The Witch of Argath claims she alone controls the acid rain.");

								break;


						    case 4:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A heavy and acidic rain begins to fall on the Land of Mordor.");

else if(k == 2)
whisper("** A blood-colored acid rain falls over all of Mordor.");

else if(k == 3)
whisper("** Drops of acid-rich rain spatter your armor and weapons.");

else if(k == 4)
whisper("** The blood-red rain falls from dark, reddish-black clouds.");

else if(k == 5)
whisper("** Zeth can convert the lawful to chaotic.  Take hold the horns of Zeth!");

else if(k == 6)
whisper("** The horns of Zeth lie in the sanctum of his temple.");

else if(k == 7) {
whisper("** You might travel forever in the Hall of Insanity, if you do not");
whisper("** look behind you.  Some say the hall is long only one way.");
}

else if(k == 8)
whisper("** There are 13 witches.  Do you know their names?  They surely know yours.");

else
whisper("** The Witch of Argath rides somewhere above, delivering the acid rain.");

								break;


							case 5:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** The ghostly blat of an ancient war bugle echos far away.");

else if(k == 2)
whisper("** A faint sound of ancient battles echos from the past.");

else if(k == 3)
whisper("** The echo of an old battle stills rings in this place.");

else if(k == 4)
whisper("** For a brief moment, you see ghost warriors and hear the clash of swords.");

else if(k == 5)
whisper("** A battle cry sounds, but it is a faint one from the dead past.");

else if(k == 6)
whisper("** It was the dragons at the battle of Eldenmar, but worse now are witches.");

else if(k == 7)
whisper("** Learn a dragon's name, and you diminish its power over you.");

else if(k == 8)
whisper("** The sound of the hard rain muffles a long dead call to arms.");

else {
whisper("** The elves were brilliant in the battle of Eldenmar, and the dwarves");
whisper("** were courageous, but it was the men who gave their lives.");
}

								break;

							case 6:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A shadow moves in the shadows around you.");

else if(k == 2)
whisper("** There may be something hiding in the shadows.");

else if(k == 3)
whisper("** A shadow falls across the shadows.");

else if(k == 4)
whisper("** A darkness seems to swell all around you.");

else if(k == 5)
whisper("** Deep shadows twist and turn as you step into them.");

else if(k == 6)
whisper("** Blackness and shadow dulls your perceptions.  Be wary.");

else if(k == 7)
whisper("** Not all shadows are empty.  Some have substance.");

else if(k == 8)
whisper("** One of Sadena's shadows falls across your path, but not silently.");

else
whisper("** The shadows seem to move when you do.  Could it be Sadena?");

								break;
						}

						Whisper[j].misc = (short)n;
						Whisper[j].interval = 1100;
					}

					break;

				case 4:  
					/* Night time whispers */
					if(mrand(1, 100) > 50 && (wtime > 20 || wtime < 6)) {
						n = mrand(0, 2);
						n -= 1;
						n += Whisper[j].misc;
						if(n < 0 || n > 4) 
							n = 0;
						switch (n) {
							case 0:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** Black clouds cover the sky, obscuring the moon.");

else if(k == 2)
whisper("** Far away, a dire wolf howls as the moon vanishes behind black clouds.");

else if(k == 3)
whisper("** Vampyrs prefer the coldness of the moonless nights.");

else if(k == 4)
whisper("** A night without a moon is a night for vampyrs.");

else if(k == 5)
whisper("** Vampyrs, unlike the werewolves, prefer the moonless night.");

else if(k == 6)
whisper("** The Witch of Elmoren and the Witch of Strathmore are crib sisters.");

else if(k == 7)
whisper("** Sadena is the Witch of Shadows, and she has eyes everywhere in the dark.");

else
whisper("** Away in the distance a pack of dire wolves honor the dark.");

								break;

							case 1:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A crescent of silver emerges in the dark sky.");

else if(k == 2)
whisper("** The crescent moon casts an eerie light across the Land of Mordor.");

else if(k == 3)
whisper("** Blood on a crescent moon portends a night of death.");

else if(k == 4)
whisper("** A blood-red crescent moon appears as a newly used dagger.");

else if(k == 5)
whisper("** The crescent moon marks the coming of the werewolves.");

else if(k == 6)
whisper("** Werewolves are restless with the coming of the crescent moon.");

else if(k == 7)
whisper("** The tower at Argath is blood red beneath a blood moon.");

else if(k == 8)
whisper("** Zeth's temple can only be entered beneath a dark moon.");

else
whisper("** The crescent moon casts little light tonight.");

								break;

							case 2:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A waxing moon scuds in the black sky.");

else if(k == 2)
whisper("** The devil's waxing moon climbs across the clouded sky.");

else if(k == 3)
whisper("** Evilness comes with the waxing moon's rise.");

else if(k == 4)
whisper("** There's a waxing moon in the night sky.");

else if(k == 5)
whisper("** The waxing moon casts a cold light on the world.");

else if(k == 6) {
whisper("** Briefly, before clouds cover a waxing moon, you see the");
whisper("** outline of a winged monster.");
}

else if(k == 7)
whisper("** A halo appears around the waxing moon, a sure sign of terror for many.");

else if(k == 8)
whisper("** The flutter of harpie wings stir the moonlight overhead.");

else
whisper("** The waxing moon brings another night of horror.");

								break;

							case 3:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A gibbous moon rises over the Land of Mordor.");

else if(k == 2)
whisper("** The moon is almost full, providing pale light to a dark world.");

else
whisper("** The gibbous moon brings an evil omen as its face slides behind clouds.");

								break;

							case 4:
								Whisper[j].ltime = t;
								k = mrand(1, 10);

if(k == 1)
whisper("** A werewolf's full moon fills the cloudy sky.");
	
else if(k == 2)
whisper("** The full moon is the delight of werewolves and dire wolves.");

else if(k == 3)
whisper("** Denke vui remanze en werewolves nyre silvorre en lunare cor?");

else if(k == 4)
whisper("** The hatred between werewolves and vampyrs is strongest on nights like this.");

else if(k == 5)
whisper("** The bond between werewolves and witches is old as the moon.");

else if(k == 6)
whisper("** Beware the cemetery when the moon comes to its zenith.");

else if(k == 7)
whisper("** Restless are the undead when the moon comes to brightness.");

else if(k == 8)
whisper("** An open grave under the moon is a sign of an impending death.  Yours?");

else {
whisper("** The silver light of the full moon reminds you that demons and");
whisper("** werewolves cannot abide the touch of silver.");
}

								break;
							}

							Whisper[j].misc = (short)n;
							Whisper[j].interval = 1200;
					}

					break;
			} 
		} 
	} 

	n = 11 - Whisper[1].misc - Whisper[2].misc - (Whisper[3].misc - 2)
		+ Whisper[4].misc;
	if(n > 25 || n < 2) 
		n = 11;
	Rnd_update_interval = (short)n;

	return;
} 

