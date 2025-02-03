/*
 * zle_hist.c - history editing
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1996 Paul Falstad
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Paul Falstad or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Paul Falstad and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Paul Falstad and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Paul Falstad and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#define ZLE
#include "zsh.h"

#define ZLETEXT(X) ((X)->zle_text ? (X)->zle_text : (X)->text)

/**/
void
remember_edits(void)
{
    if (histline == curhist) {
	zsfree(curhistline);
	curhistline = metafy((char *) line, ll, META_DUP);
    }
    else {
	Histent ent = gethistent(histline);

	if (metadiffer(ZLETEXT(ent), (char *) line, ll)) {
	    zsfree(ent->zle_text);
	    ent->zle_text = metafy((char *) line, ll, META_DUP);
	}
    }
}

/**/
void
forget_edits(void)
{
    int i;

    for (i = 0; i < histentct; i++) {
	zsfree(histentarr[i].zle_text);
	histentarr[i].zle_text = NULL;
    }
}

/**/
void
uphistory(void)
{
    if (!zle_goto_hist(histline - zmult) && isset(HISTBEEP))
	feep();
}

/**/
int
upline(void)
{
    int n = zmult;

    if (n < 0) {
	zmult = -zmult;
	n = downline();
	zmult = -zmult;
	return n;
    }
    if ((lastcmd & ZLE_LINEMOVE) != ZLE_LINEMOVE)
	lastcol = cs - findbol();
    cs = findbol();
    while (n) {
	if (!cs)
	    break;
	cs--;
	cs = findbol();
	n--;
    }
    if (!n) {
	int x = findeol();

	if ((cs += lastcol) >= x) {
	    cs = x;
	    if (cs > findbol() && bindtab == altbindtab)
		cs--;
	}
    }
    return n;
}

/**/
void
uplineorhistory(void)
{
    int ocs = cs;
    int n = upline();
    if (n) {
	int m = zmult;

	cs = ocs;
	if (virangeflag || in_vared) {
	    feep();
	    return;
	}
	zmult = n;
	uphistory();
	zmult = m;
    }
}

/**/
void
viuplineorhistory(void)
{
    int col = lastcol;
    uplineorhistory();
    lastcol = col;
    vifirstnonblank();
}

/**/
void
uplineorsearch(void)
{
    int ocs = cs;
    int n = upline();
    if (n) {
	int m = zmult;

	cs = ocs;
	if (virangeflag || in_vared) {
	    feep();
	    return;
	}
	zmult = n;
	historysearchbackward();
	zmult = m;
    }
}

/**/
int
downline(void)
{
    int n = zmult;

    if (n < 0) {
	zmult = -zmult;
	n = upline();
	zmult = -zmult;
	return n;
    }
    if ((lastcmd & ZLE_LINEMOVE) != ZLE_LINEMOVE)
	lastcol = cs - findbol();
    while (n) {
	int x = findeol();

	if (x == ll)
	    break;
	cs = x + 1;
	n--;
    }
    if (!n) {
	int x = findeol();

	if ((cs += lastcol) >= x) {
	    cs = x;
	    if (cs > findbol() && bindtab == altbindtab)
		cs--;
	}
    }
    return n;
}

/**/
void
downlineorhistory(void)
{
    int ocs = cs;
    int n = downline();
    if (n) {
	int m = zmult;

	cs = ocs;
	if (virangeflag || in_vared) {
	    feep();
	    return;
	}
	zmult = n;
	downhistory();
	zmult = m;
    }
}

/**/
void
vidownlineorhistory(void)
{
    int col = lastcol;
    downlineorhistory();
    lastcol = col;
    vifirstnonblank();
}

/**/
void
downlineorsearch(void)
{
    int ocs = cs;
    int n = downline();
    if (n) {
	int m = zmult;

	cs = ocs;
	if (virangeflag || in_vared) {
	    feep();
	    return;
	}
	zmult = n;
	historysearchforward();
	zmult = m;
    }
}

/**/
void
acceptlineanddownhistory(void)
{
    char *s;

    if (!(s = zle_get_event(histline + 1))) {
	feep();
	return;
    }
    pushnode(bufstack, ztrdup(s));
    done = 1;
    stackhist = histline + 1;
}

/**/
void
downhistory(void)
{
    if (!zle_goto_hist(histline + zmult) && isset(HISTBEEP))
	feep();
}

static int histpos;

/**/
void
historysearchbackward(void)
{
    int hl = histline;
    int n = zmult;
    int t0;
    char *s;

    if (zmult < 0) {
	zmult = -n;
	historysearchforward();
	zmult = n;
	return;
    }
    if (lastcmd & ZLE_HISTSEARCH)
	t0 = histpos;
    else {
	for (t0 = 0; t0 < ll && !iblank(line[t0]); t0++);
	if (t0 < ll)
	    t0++;
	histpos = t0;
    }
    for (;;) {
	hl--;
	if (!(s = zle_get_event(hl))) {
	    feep();
	    return;
	}
	if (metadiffer(s, (char *) line, t0) < 0 &&
	    metadiffer(s, (char *) line, ll) && --n <= 0)
	    break;
    }
    zle_goto_hist(hl);
}

/**/
void
historysearchforward(void)
{
    int hl = histline;
    int n = zmult;
    int t0;
    char *s;

    if (zmult < 0) {
	zmult = -n;
	historysearchbackward();
	zmult = n;
	return;
    }
    if (lastcmd & ZLE_HISTSEARCH)
	t0 = histpos;
    else {
	for (t0 = 0; t0 < ll && !iblank(line[t0]); t0++);
	if (t0 < ll)
	    t0++;
	histpos = t0;
    }
    for (;;) {
	hl++;
	if (!(s = zle_get_event(hl))) {
	    feep();
	    return;
	}
	if (metadiffer(s, (char *) line, t0) < (hl == curhist) &&
	    metadiffer(s, (char *) line, ll) && --n <= 0)
	    break;
    }
    zle_goto_hist(hl);
}

/**/
void
beginningofbufferorhistory(void)
{
    if (findbol())
	cs = 0;
    else
	beginningofhistory();
}

/**/
void
beginningofhistory(void)
{
    if (!zle_goto_hist(firsthist()) && isset(HISTBEEP))
	feep();
}

/**/
void
endofbufferorhistory(void)
{
    if (findeol() != ll)
	cs = ll;
    else
	endofhistory();
}

/**/
void
endofhistory(void)
{
    zle_goto_hist(curhist);
}

/**/
void
insertlastword(void)
{
    int n;
    char *s, *t;
    int len;
    Histent he;

/* multiple calls will now search back through the history, pem */
    static char *lastinsert;
    static int lasthist, lastpos;
    int evhist = curhist - 1, save;

    if (lastinsert) {
	int lastlen = ztrlen(lastinsert);
	int pos = cs;

	if (lastpos <= pos &&
	    lastlen == pos - lastpos &&
	    memcmp(lastinsert, (char *)&line[lastpos], lastlen) == 0) {
	    evhist = --lasthist;
	    cs = lastpos;
	    foredel(pos - cs);
	}
	zsfree(lastinsert);
	lastinsert = NULL;
    }
    if (!(he = quietgethist(evhist)) || !he->nwords) {
	feep();
	return;
    }
    if (zmult > 0) {
	n = he->nwords - (zmult - 1);
    } else {
	n = 1 - zmult;
    }
    if (n < 1 || n > he->nwords) {
	feep();
	return;
    }
    s = he->text + he->words[2*n-2];
    t = he->text + he->words[2*n-1];
    save = *t;
    *t = '\0';			/* ignore trailing whitespace */

    lasthist = evhist;
    lastpos = cs;
    lastinsert = ztrdup(s);
    spaceinline(len = ztrlen(s));
    while (len--) {
	line[cs++] = *s == Meta ? *++s ^ 32 : *s;
	s++;
    }

    *t = save;
}

/**/
char *
qgetevent(int ev)
{
    return ((ev == curhist) ? curhistline : quietgetevent(ev));
}

/**/
char *
zle_get_event(int ev)
{
    Histent ent;

    if (ev == curhist)
	return curhistline;
    if (! (ent = quietgethist(ev)))
	return NULL;
    if (ent->zle_text)
	return ent->zle_text;
    return ent->text;
}

/**/
int
zle_goto_hist(int ev)
{
    char *t;

    remember_edits();
    if(!(t = zle_get_event(ev)))
	return 0;
    histline = ev;
    setline(t);
    clearlist = 1;
    return 1;
}

/**/
void
pushline(void)
{
    int n = zmult;

    if (n < 0)
	return;
    pushnode(bufstack, metafy((char *) line, ll, META_DUP));
    while (--n)
	pushnode(bufstack, ztrdup(""));
    stackcs = cs;
    *line = '\0';
    ll = cs = 0;
    clearlist = 1;
}

/**/
void
pushlineoredit(void)
{
    int ics;
    unsigned char *s;
    char *hline = hgetline();

    if (zmult < 0)
	return;
    if (hline && *hline) {
	ics = ztrlen(hline);
	sizeline(ics + ll + 1);
	for (s = line + ll; --s >= line; *(s + ics) = *s);
	for (s = line; *hline; hline++)
	    *s++ = *hline == Meta ? *++hline ^ 32 : *hline;
	ll += ics;
	cs += ics;
    }
    pushline();
    if (!isfirstln) {
	errflag = done = 1;
    }
    clearlist = 1;
}

/**/
void
pushinput(void)
{
    int i;

    if (zmult < 0)
	return;
    zmult += i = !isfirstln;
    pushlineoredit();
    zmult -= i;
}

/**/
void
zgetline(void)
{
    char *s = (char *)getlinknode(bufstack);

    if (!s)
	feep();
    else {
	int cc;

	unmetafy(s, &cc);
	spaceinline(cc);
	memcpy((char *)line + cs, s, cc);
	cs += cc;
	free(s);
	clearlist = 1;
    }
}

/**/
void
historyincrementalsearchbackward(void)
{
    doisearch(-1);
}

/**/
void
historyincrementalsearchforward(void)
{
    doisearch(1);
}

static struct isrch_spot {
    int hl;			/* This spot's histline */
    unsigned short pos;		/* The search position in our metafied str */
    unsigned short cs;		/* The visible search position to the user */
    unsigned short len;		/* The search string's length */
    unsigned short flags;	/* This spot's flags */
#define ISS_FAILING	1
#define ISS_FORWARD	2
} *isrch_spots;

static int max_spot = 0;

static void
set_isrch_spot(int num, int hl, int pos, int cs, int len, int dir, int nomatch)
{
    if (num >= max_spot) {
	if (!isrch_spots) {
	    isrch_spots = (struct isrch_spot*)
			    zalloc((max_spot = 64) * sizeof *isrch_spots);
	} else {
	    isrch_spots = (struct isrch_spot*)realloc((char*)isrch_spots,
			    (max_spot += 64) * sizeof *isrch_spots);
	}
    }

    isrch_spots[num].hl = hl;
    isrch_spots[num].pos = (unsigned short)pos;
    isrch_spots[num].cs = (unsigned short)cs;
    isrch_spots[num].len = (unsigned short)len;
    isrch_spots[num].flags = (dir > 0? ISS_FORWARD : 0)
			   + (nomatch? ISS_FAILING : 0);
}

static void
get_isrch_spot(int num, int *hlp, int *posp, int *csp, int *lenp, int *dirp, int *nomatch)
{
    *hlp = isrch_spots[num].hl;
    *posp = (int)isrch_spots[num].pos;
    *csp = (int)isrch_spots[num].cs;
    *lenp = (int)isrch_spots[num].len;
    *dirp = (isrch_spots[num].flags & ISS_FORWARD)? 1 : -1;
    *nomatch = (isrch_spots[num].flags & ISS_FAILING);
}

#define ISEARCH_PROMPT		"failing XXX-i-search: "
#define NORM_PROMPT_POS		8
#define FIRST_SEARCH_CHAR	(NORM_PROMPT_POS + 14)

/**/
void
doisearch(int dir)
{
    char *s, *ibuf = halloc(80), *sbuf = ibuf + FIRST_SEARCH_CHAR;
    int sbptr = 0, top_spot = 0, pos, sibuf = 80;
    int nomatch = 0, skip_line = 0, skip_pos = 0;
    int odir = dir, sens = zmult == 1 ? 3 : 1;
    int hl = histline;
    int cmd;
    int *obindtab = bindtab;
    static char *previous_search = NULL;
    static int previous_search_len = 0;

    clearlist = 1;

    strcpy(ibuf, ISEARCH_PROMPT);
    memcpy(ibuf + NORM_PROMPT_POS, (dir == 1) ? "fwd" : "bck", 3);
    remember_edits();
    s = zle_get_event(hl);
    bindtab = mainbindtab;
    pos = metalen(s, cs);
    for (;;) {
	/* Remember the current values in case search fails (doesn't push). */
	set_isrch_spot(top_spot, hl, pos, cs, sbptr, dir, nomatch);
	if (sbptr == 1 && sbuf[0] == '^') {
	    cs = 0;
    	    nomatch = 0;
	    statusline = ibuf + NORM_PROMPT_POS;
	} else if (sbptr > 0) {
	    char *last_line = s;

	    for (;;) {
		char *t;

		if (skip_pos) {
		    if (dir < 0) {
			if (pos == 0)
			    skip_line = 1;
			else
			    pos -= 1 + (pos != 1 && s[pos-2] == Meta);
		    } else if (sbuf[0] != '^') {
			if (pos >= (int)strlen(s+1))
			    skip_line = 1;
			else
			    pos += 1 + (s[pos] == Meta);
		    } else
			skip_line = 1;
		    skip_pos = 0;
		}
		if (!skip_line && ((sbuf[0] == '^') ?
		    (t = metadiffer(s, sbuf + 1, sbptr - 1) < sens ? s : NULL) :
		    (t = hstrnstr(s, pos, sbuf, sbptr, dir, sens)))) {
		    zle_goto_hist(hl);
		    pos = t - s;
		    cs = ztrsub(t, s) + (dir == 1? sbptr - (sbuf[0]=='^') : 0);
	    	    nomatch = 0;
		    statusline = ibuf + NORM_PROMPT_POS;
		    break;
		}
		hl += dir;
		if (!(s = zle_get_event(hl))) {
		    if (sbptr == (int)isrch_spots[top_spot-1].len
		     && (isrch_spots[top_spot-1].flags & ISS_FAILING))
			top_spot--;
		    get_isrch_spot(top_spot, &hl, &pos, &cs, &sbptr,
				   &dir, &nomatch);
		    if (!nomatch) {
			feep();
			nomatch = 1;
		    }
		    s = last_line;
		    skip_line = 0;
		    statusline = ibuf;
		    break;
		}
		pos = dir == 1? 0 : strlen(s);
		skip_line = !strcmp(last_line, s);
	    }
	} else {
	    top_spot = 0;
    	    nomatch = 0;
	    statusline = ibuf + NORM_PROMPT_POS;
	}
	sbuf[sbptr] = '_';
	statusll = sbuf - statusline + sbptr + 1;
    ref:
	refresh();
	if ((cmd = getkeycmd()) < 0 || cmd == z_sendbreak) {
	    int i;
	    get_isrch_spot(0, &hl, &pos, &i, &sbptr, &dir, &nomatch);
	    s = zle_get_event(hl);
	    zle_goto_hist(hl);
	    cs = i;
	    break;
	}
	switch (cmd) {
	case z_clearscreen:
	    clearscreen();
	    goto ref;
	case z_redisplay:
	    redisplay();
	    goto ref;
	case z_vicmdmode:
	    bindtab = (bindtab == mainbindtab) ? altbindtab : mainbindtab;
	    goto ref;
	case z_vibackwarddeletechar:
	case z_backwarddeletechar:
	    if (top_spot)
		get_isrch_spot(--top_spot, &hl, &pos, &cs, &sbptr,
			       &dir, &nomatch);
	    else
		feep();
	    if (nomatch) {
		statusline = ibuf;
		skip_pos = 1;
	    }
	    s = zle_get_event(hl);
	    if (nomatch || !sbptr || (sbptr == 1 && sbuf[0] == '^')) {
		int i = cs;
		zle_goto_hist(hl);
		cs = i;
	    }
	    memcpy(ibuf + NORM_PROMPT_POS, (dir == 1) ? "fwd" : "bck", 3);
	    continue;
	case z_acceptandhold:
	    acceptandhold();
	    goto brk;
	case z_acceptandinfernexthistory:
	    acceptandinfernexthistory();
	    goto brk;
	case z_acceptlineanddownhistory:
	    acceptlineanddownhistory();
	    goto brk;
	case z_acceptline:
	    acceptline();
	    goto brk;
	case z_historyincrementalsearchbackward:
	    set_isrch_spot(top_spot++, hl, pos, cs, sbptr, dir, nomatch);
	    if (dir != -1)
		dir = -1;
	    else
		skip_pos = 1;
	    goto rpt;
	case z_historyincrementalsearchforward:
	    set_isrch_spot(top_spot++, hl, pos, cs, sbptr, dir, nomatch);
	    if (dir != 1)
		dir = 1;
	    else
		skip_pos = 1;
	    goto rpt;
	case z_virevrepeatsearch:
	    set_isrch_spot(top_spot++, hl, pos, cs, sbptr, dir, nomatch);
	    dir = -odir;
	    skip_pos = 1;
	    goto rpt;
	case z_virepeatsearch:
	    set_isrch_spot(top_spot++, hl, pos, cs, sbptr, dir, nomatch);
	    dir = odir;
	    skip_pos = 1;
	rpt:
	    if (!sbptr && previous_search_len) {
		if (previous_search_len > sibuf - FIRST_SEARCH_CHAR - 2) {
		    ibuf = hrealloc(ibuf, sibuf, sibuf + previous_search_len);
		    sbuf = ibuf + FIRST_SEARCH_CHAR;
		    sibuf += previous_search_len;
		}
		memcpy(sbuf, previous_search, sbptr = previous_search_len);
	    }
	    memcpy(ibuf + NORM_PROMPT_POS, (dir == 1) ? "fwd" : "bck", 3);
	    continue;
	case z_sendstring:
	    sendstring();
	    goto ref;
	case z_viquotedinsert:
	    sbuf[sbptr] = '^';
	    refresh();
	case z_quotedinsert:
	    if ((c = getkey(0)) == EOF) {
		feep();
		continue;
	    }
	    goto ins;
	default:
	    if(cmd == z_selfinsertunmeta) {
		c &= 0x7f;
		if(c == '\r')
		    c = '\n';
	    } else if (cmd == z_magicspace)
		c = ' ';
	    else if (cmd != z_selfinsert) {
		ungetkeycmd();
		if (cmd == z_sendbreak)
		    sbptr = 0;
		goto brk;
	    }
	ins:
	    if (sbptr == PATH_MAX) {
		feep();
		break;
	    }
	    set_isrch_spot(top_spot++, hl, pos, cs, sbptr, dir, nomatch);
	    if (sbptr == sibuf - FIRST_SEARCH_CHAR - 2) {
		ibuf = hrealloc(ibuf, sibuf, sibuf * 2);
		sbuf = ibuf + FIRST_SEARCH_CHAR;
		sibuf *= 2;
	    }
	    sbuf[sbptr++] = c;
	}
    }
  brk:
    if (sbptr) {
	zfree(previous_search, previous_search_len);
	previous_search = zalloc(sbptr);
	memcpy(previous_search, sbuf, previous_search_len = sbptr);
    }
    statusline = NULL;
    bindtab = obindtab;
}

/**/
void
acceptandinfernexthistory(void)
{
    int t0;
    char *s;

    done = 1;
    for (t0 = histline - 2;; t0--) {
	if (!(s = qgetevent(t0)))
	    return;
	if (!metadiffer(s, (char *) line, ll))
	    break;
    }
    if (!(s = qgetevent(t0 + 1)))
	return;
    pushnode(bufstack, ztrdup(s));
    stackhist = t0 + 1;
}

/**/
void
infernexthistory(void)
{
    int t0;
    char *s;

    for (t0 = histline - 2;; t0--) {
	if (!(s = qgetevent(t0))) {
	    feep();
	    return;
	}
	if (! metadiffer(s, (char *) line, ll))
	    break;
    }
    if (!(s = qgetevent(t0 + 1))) {
	feep();
	return;
    }
    zle_goto_hist(t0 + 1);
}

/**/
void
vifetchhistory(void)
{
    if (zmult < 0)
	return;
    if (histline == curhist) {
	if (!gotmult) {
	    cs = ll;
	    cs = findbol();
	    return;
	}
    }
    if (!zle_goto_hist(gotmult ? zmult : curhist) && isset(HISTBEEP))
	feep();
}

/**/
int
getvisrchstr(void)
{
    char *sbuf = halloc(80);
    int sptr = 1, ret = 0, ssbuf = 80;
    int cmd;
    int *obindtab = bindtab;

    if (visrchstr) {
	zsfree(visrchstr);
	visrchstr = NULL;
    }
    clearlist = 1;
    statusline = sbuf;
    sbuf[0] = (visrchsense == -1) ? '?' : '/';
    bindtab = mainbindtab;
    while (sptr) {
	sbuf[sptr] = '_';
	statusll = sptr + 1;
	refresh();
	if ((cmd = getkeycmd()) < 0 || cmd == z_sendbreak) {
	    ret = 0;
	    break;
	}
	if(cmd == z_magicspace) {
	    c = ' ';
	    cmd = z_selfinsert;
	}
	switch(cmd) {
	  case z_redisplay:
	    redisplay();
	    break;
	  case z_clearscreen:
	    clearscreen();
	    break;
	  case z_acceptline:
	  case z_vicmdmode:
	    sbuf[sptr] = 0;
	    visrchstr = metafy(sbuf + 1, sptr - 1, META_DUP);
	    ret = 1;
	    sptr = 0;
	    break;
	  case z_backwarddeletechar:
	  case z_vibackwarddeletechar:
	    sptr--;
	    break;
	  case z_backwardkillword:
	  case z_vibackwardkillword:
	    while(sptr != 1 && iblank(sbuf[sptr - 1]))
		sptr--;
	    if(iident(sbuf[sptr - 1]))
		while(sptr != 1 && iident(sbuf[sptr - 1]))
		    sptr--;
	    else
		while(sptr != 1 && !iident(sbuf[sptr - 1]) && !iblank(sbuf[sptr - 1]))
		    sptr--;
	    break;
	  case z_sendstring:
	    sendstring();
	    break;
	  case z_viquotedinsert:
	    sbuf[sptr] = '^';
	    refresh();
	    /* fall through */
	  case z_quotedinsert:
	    if ((c = getkey(0)) == EOF) {
		feep();
		break;
	    }
	    goto ins;
	  case z_selfinsertunmeta:
	    c &= 0x7f;
	    if(c == '\r')
		c = '\n';
	  case z_selfinsert:
	  ins:
	    if(sptr == ssbuf - 1) {
		char *newbuf = halloc(ssbuf *= 2);
		strcpy(newbuf, sbuf);
		statusline = sbuf = newbuf;
	    }
	    sbuf[sptr++] = c;
	    break;
	  default:
	    feep();
	}
    }
    statusline = NULL;
    bindtab = obindtab;
    return ret;
}

/**/
void
vihistorysearchforward(void)
{
    visrchsense = 1;
    if (getvisrchstr())
	virepeatsearch();
}

/**/
void
vihistorysearchbackward(void)
{
    visrchsense = -1;
    if (getvisrchstr())
	virepeatsearch();
}

/**/
void
virepeatsearch(void)
{
    int hl = histline;
    int t0;
    int n = zmult;
    char *s;

    if (!visrchstr) {
	feep();
	return;
    }
    if (zmult < 0) {
	n = -n;
	visrchsense = -visrchsense;
    }
    t0 = strlen(visrchstr);
    for (;;) {
	hl += visrchsense;
	if (!(s = zle_get_event(hl))) {
	    feep();
	    return;
	}
	if (!metadiffer(s, (char *) line, ll))
	    continue;
	if (*visrchstr == '^') {
	    if (strncmp(s, visrchstr + 1, t0 - 1) != 0)
		continue;
	} else if (!hstrnstr(s, 0, visrchstr, t0, 1, 1))
	    continue;
	if (--n <= 0)
	    break;
    }
    zle_goto_hist(hl);
}

/**/
void
virevrepeatsearch(void)
{
    visrchsense = -visrchsense;
    virepeatsearch();
    visrchsense = -visrchsense;
}

/* Extra function added by A.R. Iano-Fletcher.	*/
/*The extern variable "cs" is the position of the cursor. */
/* history-beginning-search-backward */

/**/
void
historybeginningsearchbackward(void)
{
    int cpos = cs;		/* save cursor position */
    int hl = histline;
    int n = zmult;
    char *s;

    if (zmult < 0) {
	zmult = -n;
	historybeginningsearchforward();
	zmult = n;
	return;
    }
    for (;;) {
	hl--;
	if (!(s = zle_get_event(hl))) {
	    feep();
	    return;
	}
	if (metadiffer(s, (char *)line, cs) < 0 &&
	    metadiffer(s, (char *)line, ll) && --n <= 0)
	    break;
    }

    zle_goto_hist(hl);
    cs = cpos;
}

/* Extra function added by A.R. Iano-Fletcher.	*/

/* history-beginning-search-forward */
/**/
void
historybeginningsearchforward(void)
{
    int cpos = cs;		/* save cursor position */
    int hl = histline;
    int n = zmult;
    char *s;

    if (zmult < 0) {
	zmult = -n;
	historybeginningsearchbackward();
	zmult = n;
	return;
    }
    for (;;) {
	hl++;
	if (!(s = zle_get_event(hl))) {
	    feep();
	    return;
	}
	if (metadiffer(s, (char *)line, cs) < (hl == curhist) &&
	    metadiffer(s, (char *)line, ll) && --n <= 0)
	    break;
    }

    zle_goto_hist(hl);
    cs = cpos;
}
