/*
 * lex.c - lexical analysis
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

#include "zsh.h"

/* lexical state */

int len = 0, bsiz = 256;
char *bptr;
extern int hwgetword;

struct lexstack {
    struct lexstack *next;

    int incmdpos;
    int incond;
    int incasepat;
    int isfirstln;
    int isfirstch;
    int histactive;
    int histdone;
    int spaceflag;
    int stophist;
    int hlinesz;
    char *hline;
    char *hptr;
    int tok;
    int isnewlin;
    char *tokstr;
    char *yytext;
    char *bptr;
    int bsiz;
    short *chwords;
    int chwordlen;
    int chwordpos;
    int hwgetword;
    int lexstop;
    struct heredocs *hdocs;

    unsigned char *cstack;
    int csp;
};

static struct lexstack *lstack = NULL;

/* History word status variables from hist.c */
extern short *chwords;
extern int chwordlen, chwordpos;

/* save the lexical state */

/* is this a hack or what? */

/**/
void
lexsave(void)
{
    struct lexstack *ls;

    ls = (struct lexstack *)malloc(sizeof(struct lexstack));

    ls->incmdpos = incmdpos;
    ls->incond = incond;
    ls->incasepat = incasepat;
    ls->isfirstln = isfirstln;
    ls->isfirstch = isfirstch;
    ls->histactive = histactive;
    ls->histdone = histdone;
    ls->spaceflag = spaceflag;
    ls->stophist = stophist;
    ls->hline = chline;
    ls->hptr = hptr;
    ls->hlinesz = hlinesz;
    ls->cstack = cmdstack;
    ls->csp = cmdsp;
    cmdstack = (unsigned char *)zalloc(256);
    ls->tok = tok;
    ls->isnewlin = isnewlin;
    ls->tokstr = tokstr;
    ls->yytext = yytext;
    ls->bptr = bptr;
    ls->bsiz = bsiz;
    ls->chwords = chwords;
    ls->chwordlen = chwordlen;
    ls->chwordpos = chwordpos;
    ls->hwgetword = hwgetword;
    ls->lexstop = lexstop;
    ls->hdocs = hdocs;
    cmdsp = 0;
    inredir = 0;
    hdocs = NULL;

    ls->next = lstack;
    lstack = ls;
}

/* restore lexical state */

/**/
void
lexrestore(void)
{
    struct lexstack *ln;

    DPUTS(!lstack, "BUG: lexrestore() without lexsave()");
    incmdpos = lstack->incmdpos;
    incond = lstack->incond;
    incasepat = lstack->incasepat;
    isfirstln = lstack->isfirstln;
    isfirstch = lstack->isfirstch;
    histactive = lstack->histactive;
    histdone = lstack->histdone;
    spaceflag = lstack->spaceflag;
    stophist = lstack->stophist;
    chline = lstack->hline;
    hptr = lstack->hptr;
    if (cmdstack)
	free(cmdstack);
    cmdstack = lstack->cstack;
    cmdsp = lstack->csp;
    tok = lstack->tok;
    isnewlin = lstack->isnewlin;
    tokstr = lstack->tokstr;
    yytext = lstack->yytext;
    bptr = lstack->bptr;
    bsiz = lstack->bsiz;
    chwords = lstack->chwords;
    chwordlen = lstack->chwordlen;
    chwordpos = lstack->chwordpos;
    hwgetword = lstack->hwgetword;
    lexstop = lstack->lexstop;
    hdocs = lstack->hdocs;
    hlinesz = lstack->hlinesz;
    errflag = 0;

    ln = lstack->next;
    free(lstack);
    lstack = ln;
}

/**/
void
yylex(void)
{
    if (tok == LEXERR)
	return;
    do
	tok = gettok();
    while (tok != ENDINPUT && exalias());
    if (tok == NEWLIN || tok == ENDINPUT) {
	while (hdocs) {
	    struct heredocs *next = hdocs->next;

	    hwbegin(0);
	    cmdpush(hdocs->rd->type == HEREDOC ? CS_HEREDOC : CS_HEREDOCD);
	    STOPHIST
	    hdocs->rd->name = gethere(hdocs->rd->name, hdocs->rd->type);
	    ALLOWHIST
	    cmdpop();
	    hwend();
	    hdocs->rd->type = HERESTR;
	    zfree(hdocs, sizeof(struct heredocs));
	    hdocs = next;
	}
    }
    if (tok != NEWLIN)
	isnewlin = 0;
    else
	isnewlin = (inbufct) ? -1 : 1;
    if (tok == SEMI || tok == NEWLIN)
	tok = SEPER;
}

/**/
void
ctxtlex(void)
{
    static int oldpos;

    yylex();
    switch (tok) {
    case SEPER:
    case NEWLIN:
    case SEMI:
    case DSEMI:
    case AMPER:
    case AMPERBANG:
    case INPAR:
    case INBRACE:
    case DBAR:
    case DAMPER:
    case BAR:
    case BARAMP:
    case INOUTPAR:
    case DO:
    case THEN:
    case ELIF:
    case ELSE:
    case DOUTBRACK:
	incmdpos = 1;
	break;
    case STRING:
 /* case ENVSTRING: */
    case ENVARRAY:
    case OUTPAR:
    case CASE:
    case DINBRACK:
	incmdpos = 0;
	break;
    }
    if (IS_REDIROP(tok) || tok == FOR || tok == FOREACH || tok == SELECT) {
	inredir = 1;
	oldpos = incmdpos;
	incmdpos = 0;
    } else if (inredir) {
	incmdpos = oldpos;
	inredir = 0;
    }
}

#define LX1_BKSLASH 0
#define LX1_COMMENT 1
#define LX1_NEWLIN 2
#define LX1_SEMI 3
#define LX1_AMPER 5
#define LX1_BAR 6
#define LX1_INPAR 7
#define LX1_OUTPAR 8
#define LX1_INANG 13
#define LX1_OUTANG 14
#define LX1_OTHER 15

#define LX2_BREAK 0
#define LX2_OUTPAR 1
#define LX2_BAR 2
#define LX2_STRING 3
#define LX2_INBRACK 4
#define LX2_OUTBRACK 5
#define LX2_TILDE 6
#define LX2_INPAR 7
#define LX2_INBRACE 8
#define LX2_OUTBRACE 9
#define LX2_OUTANG 10
#define LX2_INANG 11
#define LX2_EQUALS 12
#define LX2_BKSLASH 13
#define LX2_QUOTE 14
#define LX2_DQUOTE 15
#define LX2_BQUOTE 16
#define LX2_COMMA 17
#define LX2_OTHER 18
#define LX2_META 19

unsigned char lexact1[256], lexact2[256], lextok2[256];

/**/
void
initlextabs(void)
{
    int t0;
    static char *lx1 = "\\q\n;!&|(){}[]<>";
    static char *lx2 = ";)|$[]~({}><=\\\'\"`,";

    for (t0 = 0; t0 != 256; t0++) {
	lexact1[t0] = LX1_OTHER;
	lexact2[t0] = LX2_OTHER;
	lextok2[t0] = t0;
    }
    for (t0 = 0; lx1[t0]; t0++)
	lexact1[(int)lx1[t0]] = t0;
    for (t0 = 0; lx2[t0]; t0++)
	lexact2[(int)lx2[t0]] = t0;
    lexact2['&'] = LX2_BREAK;
    lexact2[STOUC(Meta)] = LX2_META;
    lextok2['*'] = Star;
    lextok2['?'] = Quest;
    lextok2['{'] = Inbrace;
    lextok2['['] = Inbrack;
    lextok2['$'] = String;
    lextok2['~'] = Tilde;
    lextok2['#'] = Pound;
    lextok2['^'] = Hat;
}

/* initialize lexical state */

/**/
void
lexinit(void)
{
    incond = incasepat = nocorrect =
    lexstop = 0;
    incmdpos = 1;
    tok = ENDINPUT;
}

/* add a char to the string buffer */

/**/
void
add(int c)
{
    *bptr++ = c;
    if (bsiz == ++len) {
	int newbsiz;

	newbsiz = bsiz * 8;
	while (newbsiz < inbufct)
	    newbsiz *= 2;
	bptr = len + (tokstr = (char *)hrealloc(tokstr, bsiz, newbsiz));
	bsiz = newbsiz;
    }
}

#define SETPARBEGIN {if (zleparse && !(inbufflags & INP_ALIAS) && cs >= ll+1-inbufct) parbegin = inbufct;}
#define SETPAREND {\
	    if (zleparse && !(inbufflags & INP_ALIAS) && parbegin != -1 && parend == -1) {\
		if (cs >= ll + 1 - inbufct)\
		    parbegin = -1;\
		else\
		    parend = inbufct;\
	    } }

static int
cmd_or_math(int cs_type)
{
    int oldlen = len;
    int c;

    cmdpush(cs_type);
    c = dquote_parse(')', 0);
    cmdpop();
    *bptr = '\0';
    if (!c) {
	c = hgetc();
	if (c == ')')
	    return 1;
	hungetc(c);
	lexstop = 0;
	c = ')';
    }
    hungetc(c);
    lexstop = 0;
    while (len > oldlen) {
	len--;
	hungetc(itok(*--bptr) ? ztokens[*bptr - Pound] : *bptr);
    }
    hungetc('(');
    return 0;
}

static int
cmd_or_math_sub(void)
{
    int c = hgetc();

    if (c == '(') {
	add(Inpar);
	add('(');
	if (cmd_or_math(CS_MATHSUBST)) {
	    add(')');
	    return 0;
	}
	bptr -= 2;
	len -= 2;
    } else {
	hungetc(c);
	lexstop = 0;
    }
    return skipcomm();
}

/**/
int
gettok(void)
{
    int c, d;
    int peekfd = -1, peek;

    MUSTUSEHEAP("gettok");
  beginning:
    tokstr = NULL;
    while (iblank(c = hgetc()) && !lexstop);
    if (lexstop)
	return (errflag) ? LEXERR : ENDINPUT;
    isfirstln = 0;
    wordbeg = inbufct - (qbang && c == bangchar);
    hwbegin(-1-(qbang && c == bangchar));
    /* word includes the last character read and possibly \ before ! */
    if (idigit(c)) {	/* handle 1< foo */
	d = hgetc();
	if (d == '>' || d == '<') {
	    peekfd = c - '0';
	    c = d;
	} else {
	    hungetc(d);
	    lexstop = 0;
	}
    }

    /* chars in initial position in word */

    if (c == hashchar &&
	(isset(INTERACTIVECOMMENTS) ||
	 (!zleparse && !expanding &&
	  (!interact || unset(SHINSTDIN) || strin)))) {
	/* History is handled here to prevent extra  *
	 * newlines being inserted into the history. */

	while ((c = ingetc()) != '\n' && !lexstop) {
	    hwaddc(c);
	    addtoline(c);
	}

	if (errflag)
	    peek = LEXERR;
	else {
	    hwend();
	    hwbegin(0);
	    hwaddc('\n');
	    addtoline('\n');
	    peek = NEWLIN;
	}
	return peek;
    }
    switch (lexact1[STOUC(c)]) {
    case LX1_BKSLASH:
	d = hgetc();
	if (d == '\n')
	    goto beginning;
	hungetc(d);
	lexstop = 0;
	break;
    case LX1_NEWLIN:
	return NEWLIN;
    case LX1_SEMI:
	d = hgetc();
	if (d != ';') {
	    hungetc(d);
	    lexstop = 0;
	    return SEMI;
	}
	return DSEMI;
    case LX1_AMPER:
	d = hgetc();
	if (d == '&')
	    return DAMPER;
	else if (d == '!' || d == '|')
	    return AMPERBANG;
	else if (d == '>') {
	    d = hgetc();
	    if (d == '!' || d == '|')
		return OUTANGAMPBANG;
	    else if (d == '>') {
		d = hgetc();
		if (d == '!' || d == '|')
		    return DOUTANGAMPBANG;
		hungetc(d);
		lexstop = 0;
		return DOUTANGAMP;
	    }
	    hungetc(d);
	    lexstop = 0;
	    tokfd = -1;
	    return AMPOUTANG;
	}
	hungetc(d);
	lexstop = 0;
	return AMPER;
    case LX1_BAR:
	d = hgetc();
	if (d == '|')
	    return DBAR;
	else if (d == '&')
	    return BARAMP;
	hungetc(d);
	lexstop = 0;
	return BAR;
    case LX1_INPAR:
	d = hgetc();
	if (d == '(' && incmdpos) {
	    len = 0;
	    bptr = tokstr = (char *)ncalloc(bsiz = 256);
	    return cmd_or_math(CS_MATH) ? DINPAR : INPAR;
	} else if (d == ')')
	    return INOUTPAR;
	hungetc(d);
	lexstop = 0;
	if (!(incond == 1 || incmdpos))
	    break;
	return INPAR;
    case LX1_OUTPAR:
	return OUTPAR;
    case LX1_INANG:
	d = hgetc();
	if (!incmdpos && d == '(') {
	    hungetc(d);
	    lexstop = 0;
	    break;
	}
	if (d == '>')
	    peek = INOUTANG;
	else if (idigit(d) || d == '-') {
	    int tbs = 256, n = 0, nc;
	    char *tbuf, *tbp, *ntb;

	    tbuf = tbp = (char *)zalloc(tbs);
	    hungetc(d);

	    while ((nc = hgetc()) && !lexstop) {
		if (!idigit(nc) && nc != '-')
		    break;
		*tbp++ = (char)nc;
		if (++n == tbs) {
		    ntb = (char *)realloc(tbuf, tbs *= 2);
		    tbp += ntb - tbuf;
		    tbuf = ntb;
		}
	    }
	    if (nc == '>' && !lexstop) {
		hungetc(nc);
		while (n--)
		    hungetc(*--tbp);
		zfree(tbuf, tbs);
		break;
	    }
	    if (nc && !lexstop)
		hungetc(nc);
	    lexstop = 0;
	    while (n--)
		hungetc(*--tbp);
	    zfree(tbuf, tbs);
	    peek = INANG;
	} else if (d == '<') {
	    int e = hgetc();

	    if (e == '(') {
		hungetc(e);
		hungetc(d);
		peek = INANG;
	    } else if (e == '<')
		peek = TRINANG;
	    else if (e == '-')
		peek = DINANGDASH;
	    else {
		hungetc(e);
		lexstop = 0;
		peek = DINANG;
	    }
	} else if (d == '&')
	    peek = INANGAMP;
	else {
	    peek = INANG;
	    hungetc(d);
	    lexstop = 0;
	}
	tokfd = peekfd;
	return peek;
    case LX1_OUTANG:
	d = hgetc();
	if (d == '(') {
	    hungetc(d);
	    break;
	} else if (d == '&') {
	    d = hgetc();
	    if (d == '!' || d == '|')
		peek = OUTANGAMPBANG;
	    else {
		hungetc(d);
		lexstop = 0;
		peek = OUTANGAMP;
	    }
	} else if (d == '!' || d == '|')
	    peek = OUTANGBANG;
	else if (d == '>') {
	    d = hgetc();
	    if (d == '&') {
		d = hgetc();
		if (d == '!' || d == '|')
		    peek = DOUTANGAMPBANG;
		else {
		    hungetc(d);
		    lexstop = 0;
		    peek = DOUTANGAMP;
		}
	    } else if (d == '!' || d == '|')
		peek = DOUTANGBANG;
	    else if (d == '(') {
		hungetc(d);
		hungetc('>');
		peek = OUTANG;
	    } else {
		hungetc(d);
		lexstop = 0;
		peek = DOUTANG;
		if (isset(HISTALLOWCLOBBER))
		    hwaddc('|');
	    }
	} else {
	    hungetc(d);
	    lexstop = 0;
	    peek = OUTANG;
	    if (!incond && isset(HISTALLOWCLOBBER))
		hwaddc('|');
	}
	tokfd = peekfd;
	return peek;
    }

    /* we've started a string, now get the *
     * rest of it, performing tokenization */
    return gettokstr(c, 0);
}

/**/
int
gettokstr(int c, int sub)
{
    int bct = 0, pct = 0, brct = 0, fdpar = 0;
    int intpos = 1, in_brace_param = 0;
    int peek, inquote, unmatched = 0;
#ifdef DEBUG
    int ocmdsp = cmdsp;
#endif

    peek = STRING;
    if (!sub) {
	len = 0;
	bptr = tokstr = (char *)ncalloc(bsiz = 256);
    }
    for (;;) {
	int act;
	int e;
	int inbl = inblank(c);
	
	if (fdpar && !inbl && c != ')')
	    fdpar = 0;

	if (inbl && !in_brace_param && !pct)
	    act = LX2_BREAK;
	else {
	    act = lexact2[STOUC(c)];
	    c = lextok2[STOUC(c)];
	}
	switch (act) {
	case LX2_BREAK:
	    if (!in_brace_param && !sub)
		goto brk;
	    break;
	case LX2_META:
	    c = hgetc();
#ifdef DEBUG
	    if (lexstop) {
		fputs("BUG: input terminated by Meta\n", stderr);
		fflush(stderr);
		goto brk;
	    }
#endif
	    add(Meta);
	    break;
	case LX2_OUTPAR:
	    if (fdpar) {
		/* this is a single word `(   )', treat as INOUTPAR */
		add(c);
		*bptr = '\0';
		return INOUTPAR;
	    }
	    if ((sub || in_brace_param) && isset(SHGLOB))
		break;
	    if (!in_brace_param && !pct--) {
		if (sub) {
		    pct = 0;
		    break;
		} else
		    goto brk;
	    }
	    c = Outpar;
	    break;
	case LX2_BAR:
	    if (!pct && !in_brace_param) {
		if (sub)
		    break;
		else
		    goto brk;
	    }
	    if (unset(SHGLOB) || (!sub && !in_brace_param))
		c = Bar;
	    break;
	case LX2_STRING:
	    e = hgetc();
	    if (e == '[') {
		cmdpush(CS_MATHSUBST);
		add(String);
		add(Inbrack);
		c = dquote_parse(']', sub);
		cmdpop();
		if (c) {
		    peek = LEXERR;
		    goto brk;
		}
		c = Outbrack;
	    } else if (e == '(') {
		add(String);
		c = cmd_or_math_sub();
		if (c) {
		    peek = LEXERR;
		    goto brk;
		}
		c = Outpar;
	    } else {
		if (e == '{') {
		    add(c);
		    c = Inbrace;
		    ++bct;
		    cmdpush(CS_BRACEPAR);
		    if (!in_brace_param)
			in_brace_param = bct;
		} else {
		    hungetc(e);
		    lexstop = 0;
		}
	    }
	    break;
	case LX2_INBRACK:
	    add(c);
	    if (!in_brace_param)
		brct++;
	    c = hgetc();
	    if (c == '!' || c == '^') {
		add(c);
		c = hgetc();
	    }
	    if (c == ']')
		break;
	    if (lexstop)
		goto brk;
	    intpos = 0;
	    continue;
	case LX2_OUTBRACK:
	    if (!in_brace_param)
		brct--;
	    if (brct < 0) {
		brct = 0;
		break;
	    }
	    c = Outbrack;
	    break;
	case LX2_INPAR:
	    if ((sub || in_brace_param) && isset(SHGLOB))
		break;
	    if (!in_brace_param) {
		if (!sub) {
		    e = hgetc();
		    hungetc(e);
		    lexstop = 0;
		    /* For command words, parentheses are only
		     * special at the start.  But now we're tokenising
		     * the remaining string.  So I don't see what
		     * the old incmdpos test here is for.
		     *   pws 1999/6/8
		     *
		     * Oh, no.
		     *  func1(   )
		     * is a valid function definition in [k]sh.  The best
		     * thing we can do, without really nasty lookahead tricks,
		     * is break if we find a blank after a parenthesis.  At
		     * least this can't happen inside braces or brackets.  We
		     * only allow this with SHGLOB (set for both sh and ksh).
		     *
		     * Things like `print @( |foo)' should still
		     * work, because [k]sh don't allow multiple words
		     * in a function definition, so we only do this
		     * in command position.
		     *   pws 1999/6/14
		     */
		    if (e == ')' || (isset(SHGLOB) && inblank(e) && !bct &&
				     !brct && !intpos && incmdpos))
			goto brk;
		}
		/*
		 * This also handles the [k]sh `foo( )' function definition.
		 * Maintain a variable fdpar, set as long as a single set of
		 * parentheses contains only space.  Then if we get to the
		 * closing parenthesis and it is still set, we can assume we
		 * have a function definition.  Only do this at the start of
		 * the word, since the (...) must be a separate token.
		 */
		if (!pct++ && isset(SHGLOB) && intpos && !bct && !brct)
		    fdpar = 1;
	    }
	    c = Inpar;
	    break;
	case LX2_INBRACE:
	    if (isset(IGNOREBRACES) || sub)
		c = '{';
	    else {
		if (!len && incmdpos) {
		    add('{');
		    *bptr = '\0';
		    return STRING;
		}
		if (in_brace_param) {
		    cmdpush(CS_BRACE);
		}
		bct++;
	    }
	    break;
	case LX2_OUTBRACE:
	    if ((isset(IGNOREBRACES) || sub) && !in_brace_param)
		break;
	    if (!bct)
		break;
	    if (in_brace_param) {
		cmdpop();
	    }
	    if (bct-- == in_brace_param)
		in_brace_param = 0;
	    c = Outbrace;
	    break;
	case LX2_COMMA:
	    if (unset(IGNOREBRACES) && !sub && bct > in_brace_param)
		c = Comma;
	    break;
	case LX2_OUTANG:
	    if (!intpos) {
		if (in_brace_param || sub)
		    break;
		else
		    goto brk;
	    }
	    e = hgetc();
	    if (e != '(') {
		hungetc(e);
		lexstop = 0;
		goto brk;
	    }
	    add(Outang);
	    if (skipcomm()) {
		peek = LEXERR;
		goto brk;
	    }
	    c = Outpar;
	    break;
	case LX2_INANG:
	    if (isset(SHGLOB) && sub)
		break;
	    e = hgetc();
	    if (!(idigit(e) || e == '-' || (e == '(' && intpos))) {
		hungetc(e);
		lexstop = 0;
		if (in_brace_param || sub)
		    break;
		goto brk;
	    }
	    c = Inang;
	    if (e == '(') {
		add(c);
		if (skipcomm()) {
		    peek = LEXERR;
		    goto brk;
		}
		c = Outpar;
	    } else {
		add(c);
		c = e;
		while (c != '>' && !lexstop)
		    add(c), c = hgetc();
		c = Outang;
	    }
	    break;
	case LX2_EQUALS:
	    if (intpos) {
		e = hgetc();
		if (e != '(') {
		    hungetc(e);
		    lexstop = 0;
		    c = Equals;
		} else {
		    add(Equals);
		    if (skipcomm()) {
			peek = LEXERR;
			goto brk;
		    }
		    c = Outpar;
		}
	    } else if (!sub && peek != ENVSTRING &&
		       incmdpos && !bct && !brct) {
		char *t = tokstr;
		if (idigit(*t))
		    while (++t < bptr && idigit(*t));
		else {
		    while (iident(*t) && ++t < bptr);
		    if (t < bptr) {
			*bptr = '\0';
			skipparens(Inbrack, Outbrack, &t);
		    }
		}
		if (t == bptr) {
		    e = hgetc();
		    if (e == '(' && incmdpos) {
			*bptr = '\0';
			return ENVARRAY;
		    }
		    hungetc(e);
		    lexstop = 0;
		    peek = ENVSTRING;
		    intpos = 2;
		} else
		    c = Equals;
	    } else
		c = Equals;
	    break;
	case LX2_BKSLASH:
	    c = hgetc();
	    if (c == '\n') {
		c = hgetc();
		if (!lexstop)
		    continue;
	    } else
		add(Bnull);
	    if (lexstop)
		goto brk;
	    break;
	case LX2_QUOTE:
	    add(Snull);
	    cmdpush(CS_QUOTE);
	    for (;;) {
		STOPHIST
		while ((c = hgetc()) != '\'' && !lexstop) {
		    if (!sub && isset(CSHJUNKIEQUOTES) && c == '\n') {
			if (bptr[-1] == '\\')
			    bptr--, len--;
			else
			    break;
		    }
		    add(c);
		}
		ALLOWHIST
		if (c != '\'') {
		    unmatched = '\'';
		    peek = LEXERR;
		    cmdpop();
		    goto brk;
		}
		e = hgetc();
		if (e != '\'' || unset(RCQUOTES))
		    break;
		add(c);
	    }
	    cmdpop();
	    hungetc(e);
	    lexstop = 0;
	    c = Snull;
	    break;
	case LX2_DQUOTE:
	    add(Dnull);
	    cmdpush(CS_DQUOTE);
	    c = dquote_parse('"', sub);
	    cmdpop();
	    if (c) {
		unmatched = '"';
		peek = LEXERR;
		goto brk;
	    }
	    c = Dnull;
	    break;
	case LX2_BQUOTE:
	    add(Tick);
	    cmdpush(CS_BQUOTE);
	    SETPARBEGIN
	    inquote = 0;
	    while ((c = hgetc()) != '`' && !lexstop) {
		if (c == '\\') {
		    c = hgetc();
		    if (c != '\n') {
			add(c == '`' || c == '\\' || c == '$' ? Bnull : '\\');
			add(c);
		    }
		    else if (!sub && isset(CSHJUNKIEQUOTES))
			add(c);
		} else {
		    if (!sub && isset(CSHJUNKIEQUOTES) && c == '\n') {
			break;
		    }
		    add(c);
		    if (c == '\'') {
			if ((inquote = !inquote))
			    STOPHIST
			else
			    ALLOWHIST
		    }
		}
	    }
	    if (inquote)
		ALLOWHIST
	    cmdpop();
	    if (c != '`') {
		unmatched = '`';
		peek = LEXERR;
		goto brk;
	    }
	    c = Tick;
	    SETPAREND
	    break;
	}
	add(c);
	c = hgetc();
	if (intpos)
	    intpos--;
	if (lexstop)
	    break;
    }
  brk:
    hungetc(c);
    if (unmatched)
	zerr("unmatched %c", NULL, unmatched);
    if (in_brace_param) {
	while(bct-- >= in_brace_param)
	    cmdpop();
	zerr("closing brace expected", NULL, 0);
    } else if (unset(IGNOREBRACES) && !sub && len > 1 &&
	       peek == STRING && bptr[-1] == '}' && bptr[-2] != Bnull) {
	/* hack to get {foo} command syntax work */
	bptr--;
	len--;
	lexstop = 0;
	hungetc('}');
    }
    *bptr = '\0';
    DPUTS(cmdsp != ocmdsp, "BUG: gettok: cmdstack changed.");
    return peek;
}

extern int addedx;

/**/
int
dquote_parse(char endchar, int sub)
{
    int pct = 0, brct = 0, bct = 0, intick = 0, err = 0;
    int c;
    int math = endchar == ')' || endchar == ']';
    int zlemath = math && cs > ll + addedx - inbufct;

    while (((c = hgetc()) != endchar || bct ||
	    (math && ((pct > 0) || (brct > 0))) ||
	    intick) && !lexstop) {
      cont:
	switch (c) {
	case '\\':
	    c = hgetc();
	    if (c != '\n') {
		if (c == '$' || c == '\\' || (c == '}' && !intick && bct) ||
		    c == endchar || c == '`')
		    add(Bnull);
		else {
		    /* lexstop is implicitely handled here */
		    add('\\');
		    goto cont;
		}
	    } else if (sub || unset(CSHJUNKIEQUOTES) || endchar != '"')
		continue;
	    break;
	case '\n':
	    err = !sub && isset(CSHJUNKIEQUOTES) && endchar == '"';
	    break;
	case '$':
	    if (intick)
		break;
	    c = hgetc();
	    if (c == '(') {
		add(Qstring);
		err = cmd_or_math_sub();
		c = Outpar;
	    } else if (c == '[') {
		add(String);
		add(Inbrack);
		cmdpush(CS_MATHSUBST);
		err = dquote_parse(']', sub);
		cmdpop();
		c = Outbrack;
	    } else if (c == '{') {
		add(Qstring);
		c = Inbrace;
		cmdpush(CS_BRACEPAR);
		bct++;
	    } else if (c == '$')
		add(Qstring);
	    else {
		hungetc(c);
		lexstop = 0;
		c = Qstring;
	    }
	    break;
	case '}':
	    if (intick || !bct)
		break;
	    c = Outbrace;
	    bct--;
	    cmdpop();
	    break;
	case '`':
	    c = Qtick;
	    if (intick == 2)
		ALLOWHIST
	    if ((intick = !intick)) {
		SETPARBEGIN
		cmdpush(CS_BQUOTE);
	    } else {
		SETPAREND
	        cmdpop();
	    }
	    break;
	case '\'':
	    if (!intick)
		break;
	    if (intick == 1)
		intick = 2, STOPHIST
	    else
		intick = 1, ALLOWHIST
	    break;
	case '(':
	    pct++;
	    break;
	case ')':
	    err = (!pct-- && math);
	    break;
	case '[':
	    brct++;
	    break;
	case ']':
	    err = (!brct-- && math);
	    break;
	case '"':
	    if (intick || (!endchar && !bct))
		break;
	    if (bct) {
		add(Dnull);
		err = dquote_parse('"', sub);
		c = Dnull;
	    } else
		err = 1;
	    break;
	}
	if (err || lexstop)
	    break;
	add(c);
    }
    if (intick == 2)
	ALLOWHIST
    if (intick) {
	cmdpop();
    }
    while (bct--)
	cmdpop();
    if (lexstop)
	err = intick || endchar || err;
    else if (err == 1)
	err = c;
    if (zlemath && cs <= ll + 1 - inbufct)
	inwhat = IN_MATH;
    return err;
}

/* Tokenize a string given in s. Parsing is done as in double *
 * quotes.  This is usually called before singsub().          */

/**/
int
parsestr(char *s)
{
    int l = strlen(s), err;

    HEAPALLOC {
	lexsave();
	untokenize(s);
	inpush(dupstring(s), 0, NULL);
	strinbeg();
	stophist = 2;
	len = 0;
	bptr = tokstr = s;
	bsiz = l + 1;
	err = dquote_parse('\0', 1);
	*bptr = '\0';
	strinend();
	inpop();
	DPUTS(cmdsp, "BUG: parsestr: cmdstack not empty.");
	lexrestore();
	if (err) {
	    untokenize(s);
	    if (err > 32 && err < 127)
		zerr("parse error near `%c'", NULL, err);
	    else
		zerr("parse error", NULL, 0);
	}
    } LASTALLOC;
    return err;
}

/* Tokenize a string given in s. Parsing is done as if s were a normal *
 * command-line argument but it may contain separators.  This is used  *
 * to parse the right-hand side of ${...%...} substitutions.           */

/**/
int
parse_subst_string(char *s)
{
    int c, l = strlen(s), err;

    if (! *s)
	return 0;
    lexsave();
    untokenize(s);
    inpush(dupstring(s), 0, NULL);
    strinbeg();
    stophist = 2;
    len = 0;
    bptr = tokstr = s;
    bsiz = l + 1;
    c = hgetc();
    c = gettokstr(c, 1);
    err = errflag;
    strinend();
    inpop();
    DPUTS(cmdsp, "BUG: parse_subst_string: cmdstack not empty.");
    lexrestore();
    errflag = err;
    if (c == LEXERR) {
	untokenize(s);
	return 1;
    }
#ifdef DEBUG
    if (c != STRING || len != l || errflag) {
	fprintf(stderr, "Oops. Bug in parse_subst_string: %s\n",
		len < l ? "len < l" : errflag ? "errflag" : "c != STRING");
	fflush(stderr);
	untokenize(s);
	return 1;
    }
#endif
    return 0;
}

/* expand aliases and reserved words */

/**/
int
exalias(void)
{
    Alias an;
    Reswd rw;

    hwend();
    if (interact && isset(SHINSTDIN) && !strin && !incasepat &&
	tok == STRING && !nocorrect && !(inbufflags & INP_ALIAS) &&
	(isset(CORRECTALL) || (isset(CORRECT) && incmdpos)))
	spckword(&tokstr, 1, incmdpos, 1);

    if (!tokstr) {
	yytext = tokstrings[tok];
	if (yytext)
	    yytext = dupstring(yytext);
	return 0;
    }

    if (has_token(tokstr)) {
	char *p, *t;

	yytext = p = ncalloc(strlen(tokstr) + 1);
	for (t = tokstr; (*p++ = itok(*t) ? ztokens[*t++ - Pound] : *t++););
    } else
	yytext = tokstr;

    if (zleparse && !(inbufflags & INP_ALIAS)) {
	int zp = zleparse;

	gotword();
	if (zp == 1 && !zleparse) {
	    return 0;
	}
    }

    if (tok == STRING) {
	/* Check for an alias */
	an = noaliases ? NULL : (Alias) aliastab->getnode(aliastab, yytext);
	if (an && !an->inuse && ((an->flags & ALIAS_GLOBAL) || incmdpos ||
	     inalmore)) {
	    inpush(an->text, INP_ALIAS, an);
	    /* remove from history if it begins with space */
	    if (isset(HISTIGNORESPACE) && an->text[0] == ' ')
		remhist();
	    lexstop = 0;
	    return 1;
	}

	/* Then check for a reserved word */
	if ((incmdpos ||
	     (unset(IGNOREBRACES) && yytext[0] == '}' && !yytext[1])) &&
	    (rw = (Reswd) reswdtab->getnode(reswdtab, yytext))) {
	    tok = rw->token;
	    if (tok == DINBRACK)
		incond = 1;
	} else if (incond && !strcmp(yytext, "]]")) {
	    tok = DOUTBRACK;
	    incond = 0;
	} else if (incond && yytext[0] == '!' && !yytext[1])
	    tok = BANG;
    }
    inalmore = 0;
    return 0;
}

/* skip (...) */

/**/
int
skipcomm(void)
{
    int pct = 1, c;

    cmdpush(CS_CMDSUBST);
    SETPARBEGIN
    c = Inpar;
    do {
	add(c);
	c = hgetc();
	if (itok(c) || lexstop)
	    break;
	switch (c) {
	case '(':
	    pct++;
	    break;
	case ')':
	    pct--;
	    break;
	case '\\':
	    add(c);
	    c = hgetc();
	    break;
	case '\'':
	    add(c);
	    STOPHIST
	    while ((c = hgetc()) != '\'' && !lexstop)
		add(c);
	    ALLOWHIST
	    break;
	case '\"':
	    add(c);
	    while ((c = hgetc()) != '\"' && !lexstop)
		if (c == '\\') {
		    add(c);
		    add(hgetc());
		} else
		    add(c);
	    break;
	case '`':
	    add(c);
	    while ((c = hgetc()) != '`' && !lexstop)
		if (c == '\\')
		    add(c), add(hgetc());
		else
		    add(c);
	    break;
	}
    }
    while (pct);
    if (!lexstop)
	SETPAREND
    cmdpop();
    return lexstop;
}
