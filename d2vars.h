#ifndef D2_VARS_H
#define D2_VARS_H

#define CELLNO_MYSHRINES 1176
#define CELLNO_WAYPOINT 307
#define CELLNO_SHRINE 310
#define NUMOF_SHRINES 23


enum ItemQuality{
	ITEM_QUALITY_INVALID	= 0,
	ITEM_QUALITY_LOW		= 1 , 
	ITEM_QUALITY_NORMAL		= 2 , 
	ITEM_QUALITY_SUPERIOR	= 3 , 
	ITEM_QUALITY_MAGIC		= 4 , 
	ITEM_QUALITY_SET		= 5 , 
	ITEM_QUALITY_RARE		= 6 , 
	ITEM_QUALITY_UNIQUE		= 7 , 
	ITEM_QUALITY_CRAFTED	= 8 , 
	ITEM_QUALITY_TAMPERED	= 9 
};

enum ItemFlag{
	ITEMFLAG_IDENTIFIED  =	0x00000010,
	ITEMFLAG_SWITCHIN  =	0x00000040,
	ITEMFLAG_SWITCHOUT =	0x00000080,
	ITEMFLAG_BROKEN =		0x00000100,
	ITEMFLAG_SOCKETED =		0x00000800,
	ITEMFLAG_INSTORE =		0x00002000,
	ITEMFLAG_NAMED =		0x00008000,//for ears, personalized items etc
	ITEMFLAG_ISEAR =		0x00010000,
	ITEMFLAG_INEXPENSIVE =	0x00020000,//always costs 1 for repair /	sell
	ITEMFLAG_COMPACTSAVE =	0x00200000,
	ITEMFLAG_ETHEREAL =		0x00400000,
	ITEM_PERSONALIZED =		0x01000000,
	ITEMFLAG_RUNEWORD =		0x04000000
};

enum UnitNo {
	UNITNO_PLAYER	= 0,
	UNITNO_MONSTER	= 1,
	UNITNO_OBJECT	= 2,
	UNITNO_MISSILE	= 3,
	UNITNO_ITEM		= 4,
	UNITNO_ROOMTILE	= 5
};


enum UIVar {
	UIVAR_UNK0 =		0,		// always  1
	UIVAR_INVENTORY =	1,		// hotkey 'B'
	UIVAR_STATS =		2,		// hotkey 'C'
	UIVAR_CURRSKILL =	3,		// left or right hand skill selection
	UIVAR_SKILLS =		4,		// hotkey 'T'
	UIVAR_CHATINPUT =	5,		// chat with other players, hotkey ENTER
	UIVAR_NEWSTATS =	6,		// new stats button
	UIVAR_NEWSKILL =	7,		// new skills button
	UIVAR_INTERACT =	8,		// interact with NPC
	UIVAR_GAMEMENU =	9,		// hotkey ESC
	UIVAR_ATUOMAP =		10,		// hotkey TAB
	UIVAR_CFGCTRLS =	11,		// config control key combination
	UIVAR_NPCTRADE =	12,		// trade, game, repair with NPC
	UIVAR_SHOWITEMS =	13,		// hotkey ALT
	UIVAR_MODITEM =		14,		// add socket, imbue item
	UIVAR_QUEST =		15,		// hotkey 'Q'
	UIVAR_UNK16 =		16,
	UIVAR_NEWQUEST =	17,		// quest log button on the bottom left screen
	UIVAR_STATUSAREA =	18,		// lower panel can not redraw when set
	UIVAR_UNK19 =		19,		// init 1
	UIVAR_WAYPOINT =	20,
	UIVAR_MINIPANEL =	21,
	UIVAR_PARTY =		22,		// hotkey 'P'
	UIVAR_PPLTRADE =	23,		// trade, exchange items with other player
	UIVAR_MSGLOG =		24,
	UIVAR_STASH =		25,
	UIVAR_CUBE =		26,
	UIVAR_UNK27 =		27,
	UIVAR_INVENTORY2 =	28,
	UIVAR_INVENTORY3 =	29,
	UIVAR_INVENTORY4 =	30,
	UIVAR_BELT =		31,
	UIVAR_UNK32 =		32,
	UIVAR_HELP =		33,		// help scrren, hotkey 'H'
	UIVAR_UNK34 =		34,
	UIVAR_PARTYHEAD =	35,		// party head 
	UIVAR_PET =			36,		// hotkey 'O'
	UIVAR_QUESTSCROLL = 37,		// for showing quest information when click quest item.
};


enum UnitStat {
	STAT_STRENGTH =				0	,		// str
	STAT_ENERGY =				1	,		// energy
	STAT_DEXTERITY =			2	,		// dexterity
	STAT_VITALITY =				3	,		// vitality
	STAT_STATPOINTSLEFT	=		4	,		// stats point left
	STAT_NEWSKILLS =			5	,		// skill point left
	STAT_HP	=					6	,		// life
	STAT_MAXHP =				7	,		// max life
	STAT_MANA =					8	,		// mana
	STAT_MAXMANA =				9	,		// max mana
	STAT_STAMINA =				10  ,		// stamina
	STAT_MAXSTAMINA	=			11	,		// max stamina
	STAT_LEVEL =				12	,		// level
	STAT_EXP =					13	,		// experience
	STAT_GOLD =					14	,		// gold
	STAT_GOLDBANK =				15	,		// stash gold
	STAT_TOBLOCK =				20	,		// to block
	STATS_MANARECOVERY =		26	,		//
	STAT_LASTEXP =				29	,		//
	STAT_NEXTEXP =				30	,		//
	STAT_MAGIC_DAMAGE_REDUCED = 35	,		// damage reduction
	STAT_DAMAGE_REDUCED =		36	,		// magic damage reduction
	STAT_MAGIC_RESIST =			37	,		// magic resist
	STAT_MAX_MAGIC_RESIST =		38	,		// max magic resist
	STAT_FIRE_RESIST =			39	,		// fire resist
	STAT_MAX_FIRE_RESIST =		40	,		// max fire resist
	STAT_LIGHTING_RESIST =		41	,		// lightning resist
	STAT_MAX_LIGHTING_RESIST =	42	,		// max lightning resist
	STAT_COLD_RESIST =			43	,		// cold resist
	STAT_MAX_COLD_RESIST =		44	,		// max cold resist
	STAT_POSION_RESIST =		45	,		// poison resist
	STAT_MAX_POSION_RESIST =	46	,		// max poison resist
	STAT_LIFE_LEECH =			60	,		// Life Leech
	STAT_MANA_LEECH	=			62	,		// Mana Leech
	STAT_VELOCITYPERCENT =		67	,		// effective run/walk
	STAT_AMMOQUANTITY =			70	,		// ammo quantity(arrow/bolt/throwing)
	STAT_DURABILITY	=			72	,		// item durability
	STAT_MAXDURABILITY =		73	,		// max item durability
	STAT_HPREGEN =				74 ,		// hp regen
	STAT_EXTRA_GOLD	=			79	,		// Gold find (GF)
	STAT_MAGIC_FIND =			80	,		// magic find (MF)
	STAT_IAS =					93	,		// ias
	STAT_FRW =					96	,		// faster run/walk
	STAT_FHR =					99	,		// faster hit recovery
	STAT_FBR =					102	,		// faster block rate
	STAT_FCR =					105	,		// faster cast rate
	STAT_PLR =					110	, 
	STAT_DTM =					114	,		// damage taken goes to mana
	STAT_OW =					135	, 
	STAT_CB =					136	,		// crushing blow
	STAT_EK =					138	,		// mana after each kill
	STAT_DS =					141	,		//deadly strike 
	STAT_FIRE_ABSORB =			143	,
	STAT_LIGHTING_ABSORB =		145	,
	STAT_COLD_ABSORB =			148	,
	STAT_NUMSOCKETS	=			194	,
	STATS_ITEM_HP_PERLEVEL =	216	,
	STAT_ITEMDIFFICULTY	=		356,
};


enum BodyLocation {  
	EQUIP_NONE =			0 ,		// Not equipped
	EQUIP_HEAD =			1 ,		// Head
	EQUIP_AMULET =			2 ,		// Amulet
	EQUIP_BODY =			3 ,		// Body armor
	EQUIP_RIGHT_PRIMARY =	4 ,		// Right primary slot
	EQUIP_LEFT_PRIMARY =	5 ,		// Left primary slot
	EQUIP_RIGHT_RING =		6 ,		// Right ring
	EQUIP_LEFT_RING =		7 ,		// Left ring
	EQUIP_BELT =			8 ,		// Belt
	EQUIP_FEET =			9 ,		// Feet
	EQUIP_GLOVES =			10,		// Gloves
	EQUIP_RIGHT_SECONDARY =	11,		// Right secondary slot
	EQUIP_LEFT_SECONDARY =	12,		// Left secondary slot
};

/*
UnitAny.dwFlags1
0x00000001 - tells the game to update the unit ( set after operateFn for objects, when reinitializing a unit etc ) 
0x00000002 - whenever the unit can be selected as a target 
0x00000004 - whenever the unit can be attacked 
0x00000008 - used to check if the unit is a valid target, curses use this 
0x00000010 - whenever the unit seed has been initialized 
0x00000020 - whenever to draw a shadow or not ( client only ) 
0x00000040 - whenever the SkillDo func has executed for the active skill 
0x00000080 - saw this used only with objects so far, when set the pre_operate is disabled 
0x00000100 - whenever the unit has a text message attached to it ( do not set this randomly ) 
0x00000200 - if this is set the unit will be treated as a hireling for certain things like lifebar display (but also for gameplay relevant aspects) 
0x00000400 - whenever the unit has a event sound specified ( server-side, do not set this randomly ) 
0x00000800 - this is only used for the summoner as far as I can tell, don't know what exactly for. 
0x00001000 - used by items to send a refresh message when they drop to the ground (etc) 
0x00002000 - whenever the unit is linked into a update message chain ( don't set this randomly ) 
0x00004000 - whenever to load new graphics when using a skill sequence ( ie the current sequence frame uses a different animation mode then the previous one ). 
0x00008000 - updates the client with the most recent life percentage and hitclass (used mostly by softhit attacks) 
0x00010000 - the unit is dead 
0x00020000 - disables treasureclass drops 
0x00080000 - this is set when the MonMode changes, didn't see exact use yet 
0x00100000 - whenever to predraw this unit ( ie treat it as a floor tile for display purposes, client specific ) 
0x00200000 - whenever this unit is an async unit ( exists only clientside like critters ) 
0x00400000 - whenever this unit is a client unit 
0x01000000 - set once the unit has been initialized, didn't check specifics 
0x02000000 - set for resurrected units and items on the floor 
0x04000000 - never gives experience when slain 
0x10000000 - automap related, not documented here 
0x20000000 - ditto 
0x40000000 - units that pet ais should ignore 
0x80000000 - this is a revived monster 

UnitAny.dwFlags2
0x00000001 - this item has an inventory attached ( aka sockets ) 
0x00000002 - whenever to update the inventory contents 
0x00000004 - set for items spawned inside a vendor shop 
0x00000008 - whenever the unit is currently shapeshifted 
0x00000010 - set for items, seams to be related to initialization 
0x00000080 - whenever the unit is within the clients line of sight 
0x00000100 - the unit has been deleted but not freed yet ( do not set this, ever ) 
0x00000400 - pUnit stores details about the owner ( missiles use this, but also set for linked pets ) 
0x00001000 - the unit is a corpse ( check +0xC4 & 0x10000 rather then this ) 
0x00010000 - the unit has teleported and needs to resync 
0x00020000 - whenever pUnit stores info about the last attacker 
0x00040000 - don't draw this unit 
0x02000000 - whenever this is an expansion unit 
0x04000000 - whenever this is a server-side unit


*/




#endif