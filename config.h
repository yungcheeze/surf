/* modifier 0 means no modifier */
static int surfuseragent    = 0;  /* Append Surf version to default WebKit user agent */
static char *fulluseragent  = ""; /* Or override the whole user agent string */
static char *scriptfiles[]  = {
"~/Software/surf/.surf/scripts/script.js",
"~/Software/surf/.surf/scripts/linkhint.js",
};
static char *dldir          = "~/Downloads/";
static char *dlstatus       = "~/.surf/dlstatus/";
static char *styledir       = "~/Software/surf/.surf/styles/";
static char *certdir        = "~/Software/surf/.surf/certificates/";
static char *cachedir       = "~/Software/surf/.surf/cache/";
static char *cookiefile     = "~/Software/surf/.surf/cookies.txt";
static char *historyfile    = "~/Software/surf/.surf/history";

/* Webkit default features */
/* Highest priority value will be used.
 * Default parameters are priority 0
 * Per-uri parameters are priority 1
 * Command parameters are priority 2
 */
static Parameter defconfig[ParameterLast] = {
	/* parameter                    Arg value       priority */
	[AcceleratedCanvas]   =       { { .i = 1 },     },
	[AccessMicrophone]    =       { { .i = 0 },     },
	[AccessWebcam]        =       { { .i = 0 },     },
	[Certificate]         =       { { .i = 0 },     },
	[CaretBrowsing]       =       { { .i = 0 },     },
	[CookiePolicies]      =       { { .v = "@Aa" }, },
	[DefaultCharset]      =       { { .v = "UTF-8" }, },
	[DiskCache]           =       { { .i = 1 },     },
	[DNSPrefetch]         =       { { .i = 0 },     },
	[FileURLsCrossAccess] =       { { .i = 0 },     },
	[FontSize]            =       { { .i = 12 },    },
	[FrameFlattening]     =       { { .i = 0 },     },
	[Geolocation]         =       { { .i = 0 },     },
	[HideBackground]      =       { { .i = 0 },     },
	[Inspector]           =       { { .i = 0 },     },
	[Java]                =       { { .i = 1 },     },
	[JavaScript]          =       { { .i = 1 },     },
	[KioskMode]           =       { { .i = 0 },     },
	[LoadImages]          =       { { .i = 1 },     },
	[MediaManualPlay]     =       { { .i = 1 },     },
	[Plugins]             =       { { .i = 0 },     },
	[PreferredLanguages]  =       { { .v = (char *[]){ NULL } }, },
	[RunInFullscreen]     =       { { .i = 0 },     },
	[ScrollBars]          =       { { .i = 1 },     },
	[ShowIndicators]      =       { { .i = 1 },     },
	[SiteQuirks]          =       { { .i = 1 },     },
	[SmoothScrolling]     =       { { .i = 0 },     },
	[SpellChecking]       =       { { .i = 0 },     },
	[SpellLanguages]      =       { { .v = ((char *[]){ "en_GB", NULL }) }, },
	[StrictTLS]           =       { { .i = 1 },     },
	[Style]               =       { { .i = 1 },     },
	[WebGL]               =       { { .i = 1 },     },
	[ZoomLevel]           =       { { .f = 1.0 },   },
	[ClipboardNotPrimary] =		  { { .i = 1 },	    },
};

static UriParameters uriparams[] = {};

/* default window size: width, height */
static int winsize[] = { 800, 600 };

static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
                                    WEBKIT_FIND_OPTIONS_WRAP_AROUND;

#define PROMPT_GO   "Go:"
#define PROMPT_FIND "Find:"

/* SETPROP(readprop, setprop, prompt)*/
#define SETPROP(r, s, p) { \
        .v = (const char *[]){ "/bin/sh", "-c", \
             "prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
             "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\")\" " \
             "| dmenu -p \"$4\" -w $1)\" && xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
             "surf-setprop", winid, r, s, p, NULL \
        } \
}

#define DLSTATUS { \
        .v = (const char *[]){ "st", "-e", "/bin/sh", "-c",\
            "while true; do cat $1/* 2>/dev/null || echo \"no hay descargas\";"\
            "A=; read A; "\
            "if [ $A = \"clean\" ]; then rm $1/*; fi; clear; done",\
            "surf-dlstatus", dlstatus, NULL } \
}

/* PLUMB(URI) */
/* This called when some URI which does not begin with "about:",
 * "http://" or "https://" should be opened.
 */
#define PLUMB(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "xdg-open \"$0\"", u, NULL \
        } \
}

/* VIDEOPLAY(URI) */
#define VIDEOPLAY(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "mpv --really-quiet \"$0\"", u, NULL \
        } \
}

/* BM_ADD(readprop) */
#define BM_ADD(r) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "(echo $(xprop -id $0 $1) | cut -d '\"' -f2 " \
             "| sed 's/.*https*:\\/\\/\\(www\\.\\)\\?//' && cat ~/.surf/bookmarks) " \
             "| awk '!seen[$0]++' > ~/.surf/bookmarks.tmp && " \
             "mv ~/.surf/bookmarks.tmp ~/.surf/bookmarks", \
             winid, r, NULL \
        } \
}

#define BM_GO { \
        .v = (const char *[]){ "/bin/sh", "-c", \
             "prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
             "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\" && cat ~/.surf/bookmarks)\" " \
             "| dmenu -l 10 -p \"$4\" -w $1)\" && " \
             "xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
             "surf-setprop", winid, "_SURF_URI", "_SURF_GO", PROMPT_GO, NULL \
        } \
}

#define SETURI(p)       { .v = (char *[]){ "/bin/sh", "-c", \
"prop=\"`dmenu.uri.sh \"$1\"`\" &&" \
"xprop -id $1 -f $0 8s -set $0 \"$prop\"", \
p, winid, NULL } }

#define ONLOAD(u) { \
	.v = (char *[]){"/bin/sh", "-c", \
		"~/.surf/shell/omnibar addhist \"$0\"", u, NULL \
	} \
}

#define GOTO { \
	.v = (char *[]){"/bin/sh", "-c", \
		"~/.surf/shell/omnibar goto \"$0\" \"$1\"", winid, "_SURF_GO", NULL \
	} \
}

/* styles */
/*
 * The iteration will stop at the first match, beginning at the beginning of
 * the list.
 */
static SiteSpecific styles[] = {
	/* regexp               file in $styledir */
	{ ".*",                 "default.css" },
};

/* certificates */
/*
 * Provide custom certificate for urls
 */
static SiteSpecific certs[] = {
	/* regexp               file in $certdir */
	{ "://suckless\\.org/", "suckless.org.crt" },
};

#define MODKEY GDK_CONTROL_MASK
#define ALTKEY GDK_MOD1_MASK

/* hotkeys */
/*
 * If you use anything else but MODKEY and GDK_SHIFT_MASK, don't forget to
 * edit the CLEANMASK() macro.
 */
static Key keys[] = {
    /* modifier              keyval          function    arg */
	{ MODKEY,                GDK_KEY_g,      spawn,      GOTO },
	{ ALTKEY, GDK_KEY_g,      spawn,      SETPROP("_SURF_URI", "_SURF_GO", PROMPT_GO) },
	{ ALTKEY, GDK_KEY_s,      spawn,      SETPROP("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_s,      spawn,      SETPROP("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },
    { MODKEY,                GDK_KEY_m,      spawn,      BM_ADD("_SURF_URI") },
	{ ALTKEY, GDK_KEY_m,      spawn,      BM_GO },
    { MODKEY,                GDK_KEY_h,      spawn,      SETURI("_SURF_GO") },
    { MODKEY,                GDK_KEY_t,      newwindow,       { 0 } },
    { MODKEY,                GDK_KEY_e,      watch_youtube,  { 0 } },
	/* download-console */
	{ MODKEY,                GDK_KEY_d,      spawndls,   { 0 } },


	{ 0,                     GDK_KEY_Escape, stop,       { 0 } },
	{ MODKEY,                GDK_KEY_c,      stop,       { 0 } },

	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_r,      reload,     { .i = 1 } },

	{ MODKEY,                GDK_KEY_period,      navigate,   { .i = +1 } },
	{ MODKEY,                GDK_KEY_comma,      navigate,   { .i = -1 } },

	/* vertical and horizontal scrolling, in viewport percentage */
	{ MODKEY,                GDK_KEY_n,      scrollv,    { .i = +10 } },
	{ MODKEY,                GDK_KEY_p,      scrollv,    { .i = -10 } },
	{ MODKEY,                GDK_KEY_v,      scrollv,    { .i = +50 } },
	{ ALTKEY,                GDK_KEY_v,      scrollv,    { .i = -50 } },
	{ ALTKEY|GDK_SHIFT_MASK, GDK_KEY_less,      scrollend,    { .i = -100 } },
	{ ALTKEY|GDK_SHIFT_MASK, GDK_KEY_greater,      scrollend,    { .i = +100 } },
	{ MODKEY,                GDK_KEY_f,      scrollh,    { .i = +10 } },
	{ MODKEY,                GDK_KEY_b,      scrollh,    { .i = -10 } },


	{ MODKEY,                GDK_KEY_minus,  zoom,       { .i = -1 } },
	{ MODKEY,                GDK_KEY_equal,  zoom,       { .i = +1 } },
	{ MODKEY,                GDK_KEY_0,      zoom,       { .i = 0  } },

	{ MODKEY,                GDK_KEY_y,      clipboard,  { .i = 1 } },
	{ MODKEY,                GDK_KEY_w,      clipboard,  { .i = 0 } },

	{ MODKEY,                GDK_KEY_s,      find,       { .i = +1 } },
	{ MODKEY,                GDK_KEY_r,      find,       { .i = -1 } },

	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_p,      print,      { 0 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_t,      showcert,   { 0 } },

	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_a,      togglecookiepolicy, { 0 } },
	{ 0,                     GDK_KEY_F11,    togglefullscreen, { 0 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_o,      toggleinspector, { 0 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_i,      toggle,     { .i = StrictTLS } },
};

/* button definitions */
/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons[] = {
	/* target       event mask      button  function        argument        stop event */
	{ OnLink,       0,              2,      clicknewwindow, { .i = 0 },     1 },
	{ OnLink,       MODKEY,         2,      clicknewwindow, { .i = 1 },     1 },
	{ OnLink,       MODKEY,         1,      clicknewwindow, { .i = 1 },     1 },
	{ OnAny,        0,              8,      clicknavigate,  { .i = -1 },    1 },
	{ OnAny,        0,              9,      clicknavigate,  { .i = +1 },    1 },
	{ OnMedia,      MODKEY,         1,      clickexternplayer, { 0 },       1 },
};
