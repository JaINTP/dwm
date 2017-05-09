#include "gaplessgrid.c"
#include "selfrestart.c"
#include "zoomswap.c"
#include "moveresize.c"

/* mbp-mappings */
#define XF86AudioMute			0x1008ff12
#define XF86AudioLowerVolume	0x1008ff11
#define XF86AudioRaiseVolume	0x1008ff13
#define XF86TouchpadToggle		0x1008ffa9

static const char *fonts[] = {
	/*"termsynu:size=12",*/
	"Misc Ohsnap.Icons:size=17",
	"Siji"
};

static const unsigned int borderpx 			= 2;	/* border pixel of windows */
static const unsigned int snap 				= 10;	/* snap pixel */
static const unsigned int tagpadding 		= 8;	/* inner padding of tags */
static const unsigned int tagspacing 		= 1;	/* space between tags */
static const unsigned int gappx				= 12;	/* gap pixel between windows */
static const unsigned int systraypinning 	= 1;	/* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing 	= 2;	/* systray spacing */
static const int systraypinningfailfirst 	= 1;	/* 1: if pinning fails, display systray on the first monitor, 0: display systray on the last monitor*/
static const int showsystray 				= 1;	/* 0 means no systray */
static const int showbar 					= 1;	/* 0 means no bar */
static const int topbar 					= 1;	/* 0 means bottom bar */

#define NUMCOLORS 9
static const char colors[NUMCOLORS][MAXCOLORS][9] = {
	// border	 foreground	background
	{ "#c14f0d", "#c14f0d", "#03070b" }, // 0 = normal
	{ "#c14f0d", "#FFFFFF", "#c14f0d" }, // 1 = selected
	{ "#c14f0d", "#f5f5f5", "#b23450" }, // 2 = red / urgent
	{ "#565352", "#f5f5f5", "#14161A" }, // 3 = green / occupied
	{ "#c14f0d", "#ab7438", "#0b0606" }, // 4 = yellow
	{ "#c14f0d", "#475971", "#0b0606" }, // 5 = blue
	{ "#c14f0d", "#694255", "#0b0606" }, // 6 = cyan
	{ "#c14f0d", "#3e6868", "#0b0606" }, // 7 = magenta
	{ "#c14f0d", "#565352", "#0b0606" }, // 8 = grey
};

/* tagging */
static const char *tags[] = {
		"web",
        "chat",
		"term",
		"code",
		"media",
		"game",
		"misc"
};

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class			instance	title		tags mask	isfloating		monitor */
	{ "Chromium",		NULL,		NULL,		1 << 0,			0,			-1 },
	{ "Steam",			NULL,		NULL,		1 << 4,			1,			-1 },
	{ "Nitrogen",		NULL,		NULL,		0,			  	1,			-1 },
	{ "Lxappearance",	NULL,		NULL,		0,				1,			-1 },
	{ "XCalc",			NULL,		NULL,		0,				1,			-1 },
	{ "Xgrabcolor",		NULL,		NULL,		0,				1,			-1 },
};

/* layout(s) */
static const float mfact 		= 0.55;	/* factor of master area size [0.05..0.95] */
static const int nmaster 		= 1;	/* number of clients in master area */
static const int resizehints 	= 0;	/* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol   arrange function */
	{ "þ",      tile },    /* first entry is default */
	{ "ý",      NULL },    /* no layout function means floating behavior */
	{ "ÿ",      monocle },
	{ "\ue003", htile },
	{ "\ue00a", gaplessgrid },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] 		     = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenuCmd[] 	     = { "spawn_rofi", NULL };
static const char *termCmd[] 	     = { "urxvt", NULL };
// Editors
static const char *editorCmd[]     = { "urxvt", "-e", "vim", NULL };
// Internet
static const char *browserCmd[]      = { "chromium", NULL };
// Audio
static const char *volUpCmd[]        = { "amixer", "-q", "set", "Master", "1+", "unmute", NULL };
static const char *volDoCmd[]        = { "amixer", "-q", "set", "Master", "1-", "unmute", NULL };
static const char *muteCmd[]         = { "amixer", "-q", "set", "Master", "toggle", NULL };
// Music
static const char *musicCmd[]        = { "urxvt", "-e", "ncmpcpp", NULL };
static const char *musicStopCmd[]    = { "ncmpcpp", "stop", NULL };
static const char *musicNextCmd[]    = { "ncmpcpp", "next", NULL };
static const char *musicPrevCmd[]    = { "ncmpcpp", "prev", NULL };
// System
static const char *togtouchpad[] 	 = { "touchpad", NULL };
static const char *screenshotCmd[] 	 = { "screenshot", NULL};


static Key keys[] = {
	/* modifier						key							function			argument */
	{ MODKEY,						XK_r,						spawn,				{.v = dmenuCmd } },
	{ MODKEY,						XK_Return,					spawn,				{.v = termCmd } },
	{ MODKEY,                       XK_c,                       spawn,              {.v = browserCmd } },
	{ MODKEY,                       XK_e,                       spawn,              {.v = editorCmd } },
	{ MODKEY|ShiftMask,				XK_b,						togglebar,			{0} },
	{ MODKEY,						XK_Right,					focusstack,			{.i = +1 } },
	{ MODKEY,						XK_Left,					focusstack,			{.i = -1 } },
	{ MODKEY,						XK_i,						incnmaster,			{.i = +1 } },
	{ MODKEY,						XK_d,						incnmaster,			{.i = -1 } },
	{ MODKEY|ShiftMask,				XK_Left,					setmfact,			{.f = -0.05} },
	{ MODKEY|ShiftMask,				XK_Right,					setmfact,			{.f = +0.05} },
	{ MODKEY|ShiftMask,				XK_Up,						setcfact,			{.f = +0.25} },
	{ MODKEY|ShiftMask,				XK_Down,					setcfact,			{.f = -0.25} },
	{ MODKEY|ShiftMask,				XK_o,						setcfact,			{.f =  0.00} },
	{ MODKEY,						XK_Down,					pushdown,			{0} },
    { MODKEY,						XK_Up,						pushup,				{0} },
	{ MODKEY|ShiftMask,				XK_Return,					zoom,				{0} },
	{ MODKEY,						XK_Tab,						view,				{0} },
	{ MODKEY|ShiftMask,				XK_w,						killclient,			{0} },
	{ MODKEY,						XK_t,						setlayout,			{.v = &layouts[0]} },
	{ MODKEY,						XK_f,						setlayout,			{.v = &layouts[1]} },
	{ MODKEY,						XK_m,						setlayout,			{.v = &layouts[2]} },
	{ MODKEY,						XK_b,						setlayout,			{.v = &layouts[3]} },
	{ MODKEY,						XK_g,						setlayout,			{.v = &layouts[4]} },
	{ MODKEY,						XK_space,					setlayout,			{0} },
    { MODKEY,						XK_u,						togglefullscreen,	{0} },
	{ MODKEY|ShiftMask,				XK_space,					togglefloating,		{0} },
	{ MODKEY,						XK_0,						view,				{.ui = ~0 } },
	{ MODKEY|ShiftMask,				XK_0,						tag,				{.ui = ~0 } },
	{ MODKEY,						XK_comma,					focusmon,			{.i = -1 } },
	{ MODKEY,						XK_period,					focusmon,			{.i = +1 } },
	{ MODKEY|ShiftMask,				XK_comma,					tagmon,				{.i = -1 } },
	{ MODKEY|ShiftMask,				XK_period,					tagmon,				{.i = +1 } },
	{ Mod4Mask,						XK_Up,						moveresize,			{.v = "0x -25y 0w 0h"} },
	{ Mod4Mask,						XK_Down,					moveresize,			{.v = "0x 25y 0w 0h"} },
	{ Mod4Mask,						XK_Left,					moveresize,			{.v = "-25x 0y 0w 0h"} },
	{ Mod4Mask,						XK_Right,					moveresize,			{.v = "25x 0y 0w 0h"} },
	{ Mod4Mask|ShiftMask,			XK_Up,						moveresize,			{.v = "0x 0y 0w -25h"} },
	{ Mod4Mask|ShiftMask,			XK_Down,					moveresize,			{.v = "0x 0y 0w 25h"} },
	{ Mod4Mask|ShiftMask,			XK_Left,					moveresize,			{.v = "0x 0y -25w 0h"} },
	{ Mod4Mask|ShiftMask,			XK_Right,					moveresize,			{.v = "0x 0y 25w 0h"} },
	TAGKEYS(						XK_1,						0)
	TAGKEYS(						XK_2,						1)
	TAGKEYS(						XK_3,						2)
	TAGKEYS(						XK_4,						3)
	TAGKEYS(						XK_5,						4)
	TAGKEYS(						XK_6,						5)
	TAGKEYS(						XK_7,						6)
	TAGKEYS(						XK_8,						7)
	TAGKEYS(						XK_9,						8)
	{ MODKEY|ShiftMask,				XK_q,						quit,				{0} },
	{ MODKEY|ShiftMask,				XK_r,						self_restart,		{0} },
    { 0,							XK_Print,					spawn,				{.v = screenshotCmd } },
	{ 0,							XF86AudioRaiseVolume,		spawn,				{.v = volUpCmd } },
    { 0,							XF86AudioLowerVolume,		spawn,				{.v = volDoCmd } },
    { 0,							XF86AudioMute,				spawn,				{.v = muteCmd } },
    { 0,                            0x1008ff15,                 spawn,              {.v = musicStopCmd } },
	{ 0,                            0x1008ff16,                 spawn,              {.v = musicPrevCmd } },
	{ 0,                            0x1008ff17,                 spawn,              {.v = musicNextCmd } },
    { 0,							XF86TouchpadToggle,			spawn,				{.v = togtouchpad } },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click					event mask		button			function			argument */
	{ ClkLtSymbol,					0,			Button1,		setlayout,			{0} },
	{ ClkLtSymbol,					0,			Button3,		setlayout,			{.v = &layouts[2]} },
	{ ClkStatusText,				0,			Button2,		spawn,				{.v = termCmd } },
	{ ClkClientWin,					MODKEY,		Button1,		movemouse,			{0} },
	{ ClkClientWin,					MODKEY,		Button2,		togglefloating,		{0} },
	{ ClkClientWin,					MODKEY,		Button3,		resizemouse,		{0} },
	{ ClkTagBar,					0,			Button1,		view,				{0} },
	{ ClkTagBar,					0,			Button3,		toggleview,			{0} },
	{ ClkTagBar,					MODKEY,		Button1,		tag,				{0} },
	{ ClkTagBar,					MODKEY,		Button3,		toggletag,			{0} },
};
