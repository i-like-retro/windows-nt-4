/*
 * builtin.c - builtin commands
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

static void printoptions _((int set));

static char *auxdata;
static int auxlen;

/* execute a builtin handler function after parsing the arguments */

#define MAX_OPS 128

/**/
int
execbuiltin(LinkList args, Builtin bn)
{
    LinkNode n;
    char ops[MAX_OPS], *arg, *pp, *name, **argv, **oargv, *optstr;
    char *oxarg, *xarg = NULL;
    int flags, sense, argc = 0, execop;

    /* initialise some static variables */
    auxdata = NULL;
    auxlen = 0;

    /* initialize some local variables */
    memset(ops, 0, MAX_OPS);
    name = (char *) ugetnode(args);

    arg = (char *) ugetnode(args);

    /* get some information about the command */
    flags = bn->flags;
    optstr = bn->optstr;

    /* Sort out the options. */
    if ((flags & BINF_ECHOPTS) && isset(BSDECHO))
	ops['E'] = 1;
    if (optstr)
	/* while arguments look like options ... */
	while (arg &&
	       ((sense = (*arg == '-')) ||
		 ((flags & BINF_PLUSOPTS) && *arg == '+')) &&
	       ((flags & BINF_PLUSOPTS) || !atoi(arg))) {
	    /* unrecognised options to echo etc. are not really options */
	    if (flags & BINF_ECHOPTS) {
		char *p = arg;
		while (*++p && strchr(optstr, (int) *p));
		if (*p)
		    break;
	    }
	    /* save the options in xarg, for execution tracing */
	    if (xarg) {
		oxarg = tricat(xarg, " ", arg);
		zsfree(xarg);
		xarg = oxarg;
	    } else
		xarg = ztrdup(arg);
	    /* handle -- or - (ops['-']), and + (ops['-'] and ops['+']) */
	    if (arg[1] == '-')
		arg++;
	    if (!arg[1]) {
		ops['-'] = 1;
		if (!sense)
		    ops['+'] = 1;
	    }
	    /* save options in ops, as long as they are in bn->optstr */
	    execop = -1;
	    while (*++arg)
		if (strchr(optstr, execop = (int)*arg))
		    ops[(int)*arg] = (sense) ? 1 : 2;
		else
		    break;
	    /* "typeset" may take a numeric argument *
	     * at the tail of the options            */
	    if (idigit(*arg) && (flags & BINF_TYPEOPT) &&
		(arg[-1] == 'L' || arg[-1] == 'R' ||
		 arg[-1] == 'Z' || arg[-1] == 'i'))
		auxlen = (int)zstrtol(arg, &arg, 10);
	    /* The above loop may have exited on an invalid option.  (We  *
	     * assume that any option requiring metafication is invalid.) */
	    if (*arg) {
		if(*arg == Meta)
		    *++arg ^= 32;
		zerr("bad option: -%c", NULL, *arg);
		zsfree(xarg);
		return 1;
	    }
	    arg = (char *) ugetnode(args);
	    /* for the "print" builtin, the options after -R are treated as
	    options to "echo" */
	    if ((flags & BINF_PRINTOPTS) && ops['R']) {
		optstr = "ne";
		flags |= BINF_ECHOPTS;
	    }
	    /* the option -- indicates the end of the options */
	    if (ops['-'])
		break;
	    /* for "fc", -e takes an extra argument */
	    if ((flags & BINF_FCOPTS) && execop == 'e') {
		auxdata = arg;
		arg = (char *) ugetnode(args);
	    }
	    /* for "typeset", -L, -R, -Z and -i take a numeric extra argument */
	    if ((flags & BINF_TYPEOPT) && (execop == 'L' || execop == 'R' ||
		execop == 'Z' || execop == 'i') && arg && idigit(*arg)) {
		auxlen = atoi(arg);
		arg = (char *) ugetnode(args);
	    }
	}
    if (flags & BINF_R)
	auxdata = "-";
    /* handle built-in options, for overloaded handler functions */
    if ((pp = bn->defopts))
	while (*pp)
	    ops[(int)*pp++] = 1;

    /* Set up the argument list. */
    if (arg) {
	/* count the arguments */
	argc = 1;
	n = firstnode(args);
	while (n)
	    argc++, incnode(n);
    }
    /* Get the actual arguments, into argv.  Oargv saves the *
     * beginning of the array for later reference.           */
    oargv = argv = (char **)ncalloc(sizeof(char **) * (argc + 1));
    if ((*argv++ = arg))
	while ((*argv++ = (char *)ugetnode(args)));
    argv = oargv;
    if (errflag) {
	zsfree(xarg);
	errflag = 0;
	return 1;
    }

    /* check that the argument count lies within the specified bounds */
    if (argc < bn->minargs || (argc > bn->maxargs && bn->maxargs != -1)) {
	zwarnnam(name, (argc < bn->minargs)
		? "not enough arguments" : "too many arguments", NULL, 0);
	zsfree(xarg);
	return 1;
    }

    /* display execution trace information, if required */
    if (isset(XTRACE)) {
	fprintf(xtrerr, "%s%s", (prompt4) ? prompt4 : "", name);
	if (xarg)
	    fprintf(xtrerr, " %s", xarg);
	while (*oargv)
	    fprintf(xtrerr, " %s", *oargv++);
	fputc('\n', xtrerr);
	fflush(xtrerr);
    }
    zsfree(xarg);
    /* call the handler function, and return its return value */
    return (*(bn->handlerfunc)) (name, argv, ops, bn->funcid);
}

/* Enable/disable an element in one of the internal hash tables.  *
 * With no arguments, it lists all the currently enabled/disabled *
 * elements in that particular hash table.                        */

/**/
int
bin_enable(char *name, char **argv, char *ops, int func)
{
    HashTable ht;
    HashNode hn;
    ScanFunc scanfunc;
    Comp com;
    int flags1 = 0, flags2 = 0;
    int match = 0, returnval = 0;

    /* Find out which hash table we are working with. */
    if (ops['f'])
	ht = shfunctab;
    else if (ops['r'])
	ht = reswdtab;
    else if (ops['a'])
	ht = aliastab;
    else
	ht = builtintab;

    /* Do we want to enable or disable? */
    if (func == BIN_ENABLE) {
	flags2 = DISABLED;
	scanfunc = ht->enablenode;
    } else {
	flags1 = DISABLED;
	scanfunc = ht->disablenode;
    }

    /* Given no arguments, print the names of the enabled/disabled elements  *
     * in this hash table.  If func == BIN_ENABLE, then scanhashtable will   *
     * print nodes NOT containing the DISABLED flag, else scanhashtable will *
     * print nodes containing the DISABLED flag.                             */
    if (!*argv) {
	scanhashtable(ht, 1, flags1, flags2, ht->printnode, 0);
	return 0;
    }

    /* With -m option, treat arguments as glob patterns. */
    if (ops['m']) {
	for (; *argv; argv++) {
	    /* parse pattern */
	    tokenize(*argv);
	    if ((com = parsereg(*argv)))
		match += scanmatchtable(ht, com, 0, 0, scanfunc, 0);
	    else {
		untokenize(*argv);
		zwarnnam(name, "bad pattern : %s", *argv, 0);
		returnval = 1;
	    }
	}
	/* If we didn't match anything, we return 1. */
	if (!match)
	    returnval = 1;
	return returnval;
    }

    /* Take arguments literally -- do not glob */
    for (; *argv; argv++) {
	    if ((hn = ht->getnode2(ht, *argv))) {
		scanfunc(hn, 0);
	    } else {
		zwarnnam(name, "no such hash table element: %s", *argv, 0);
		returnval = 1;
	    }
	}
    return returnval;
}

/* set: either set the shell options, or set the shell arguments, *
 * or declare an array, or show various things                    */

/**/
int
bin_set(char *nam, char **args, char *ops, int func)
{
    int action, optno, array = 0, hadopt = 0,
	hadplus = 0, hadend = 0, sort = 0;
    char **x;

    /* Obsolecent sh compatibility: set - is the same as set +xv *
     * and set - args is the same as set +xv -- args             */
    if (*args && **args == '-' && !args[0][1]) {
	dosetopt(VERBOSE, 0, 0);
	dosetopt(XTRACE, 0, 0);
	if (!args[1])
	    return 0;
    }

    /* loop through command line options (begins with "-" or "+") */
    while (*args && (**args == '-' || **args == '+')) {
	action = (**args == '-');
	hadplus |= !action;
	if(!args[0][1])
	    *args = "--";
	while (*++*args) {
	    if(**args == Meta)
		*++*args ^= 32;
	    if(**args != '-' || action)
		hadopt = 1;
	    /* The pseudo-option `--' signifies the end of options. */
	    if (**args == '-') {
		hadend = 1;
		args++;
		goto doneoptions;
	    } else if (**args == 'o') {
		if (!*++*args)
		    args++;
		if (!*args) {
		    zwarnnam(nam, "string expected after -o", NULL, 0);
		    inittyptab();
		    return 1;
		}
		if(!(optno = optlookup(*args)))
		    zwarnnam(nam, "no such option: %s", *args, 0);
		else if(dosetopt(optno, action, 0))
		    zwarnnam(nam, "can't change option: %s", *args, 0);
		break;
	    } else if(**args == 'A') {
		if(!*++*args)
		    args++;
		array = action ? 1 : -1;
		goto doneoptions;
	    } else if (**args == 's')
		sort = action ? 1 : -1;
	    else {
	    	if (!(optno = optlookupc(**args)))
		    zwarnnam(nam, "bad option: -%c", NULL, **args);
		else if(dosetopt(optno, action, 0))
		    zwarnnam(nam, "can't change option: -%c", NULL, **args);
	    }
	}
	args++;
    }
    doneoptions:
    inittyptab();

    /* Show the parameters, possibly with values */
    if (!hadopt && !*args)
	scanhashtable(paramtab, 1, 0, 0, paramtab->printnode,
	    hadplus ? PRINT_NAMEONLY : 0);

    if (array && !*args) {
	/* display arrays */
	scanhashtable(paramtab, 1, PM_ARRAY, 0, paramtab->printnode,
	    hadplus ? PRINT_NAMEONLY : 0);
    }
    if (!*args && !hadend)
	return 0;
    if (array)
	args++;
    if (sort)
	qsort(args, arrlen(args), sizeof(char *),
	      sort > 0 ? strpcmp : invstrpcmp);
    if (array) {
	/* create an array with the specified elements */
	char **a = NULL, **y, *name = args[-1];
	int len = arrlen(args);

	if (array < 0 && (a = getaparam(name))) {
	    int al = arrlen(a);

	    if (al > len)
		len = al;
	}
	for (x = y = zalloc((len + 1) * sizeof(char *)); len--; a++) {
	    if (!*args)
		args = a;
	    *y++ = ztrdup(*args++);
	}
	*y++ = NULL;
	setaparam(name, x);
    } else {
	/* set shell arguments */
	freearray(pparams);
	PERMALLOC {
	    pparams = arrdup(args);
	} LASTALLOC;
    }
    return 0;
}

/* setopt, unsetopt */

/**/
int
bin_setopt(char *nam, char **args, char *ops, int isun)
{
    int action, optno, match = 0;

    /* With no arguments or options, display options.  The output *
     * format is determined by the printoptions function (below). */
    if (!*args) {
	printoptions(!isun);
	return 0;
    }

    /* loop through command line options (begins with "-" or "+") */
    while (*args && (**args == '-' || **args == '+')) {
	action = (**args == '-') ^ isun;
	if(!args[0][1])
	    *args = "--";
	while (*++*args) {
	    if(**args == Meta)
		*++*args ^= 32;
	    /* The pseudo-option `--' signifies the end of options. */
	    if (**args == '-') {
		args++;
		goto doneoptions;
	    } else if (**args == 'o') {
		if (!*++*args)
		    args++;
		if (!*args) {
		    zwarnnam(nam, "string expected after -o", NULL, 0);
		    inittyptab();
		    return 1;
		}
		if(!(optno = optlookup(*args)))
		    zwarnnam(nam, "no such option: %s", *args, 0);
		else if(dosetopt(optno, action, 0))
		    zwarnnam(nam, "can't change option: %s", *args, 0);
		break;
	    } else if(**args == 'm') {
		match = 1;
	    } else {
	    	if (!(optno = optlookupc(**args)))
		    zwarnnam(nam, "bad option: -%c", NULL, **args);
		else if(dosetopt(optno, action, 0))
		    zwarnnam(nam, "can't change option: -%c", NULL, **args);
	    }
	}
	args++;
    }
    doneoptions:

    if (!match) {
	/* Not globbing the arguments -- arguments are simply option names. */
	while (*args) {
	    if(!(optno = optlookup(*args++)))
		zwarnnam(nam, "no such option: %s", args[-1], 0);
	    else if(dosetopt(optno, !isun, 0))
		zwarnnam(nam, "can't change option: %s", args[-1], 0);
	}
    } else {
	/* Globbing option (-m) set. */
	while (*args) {
	    Comp com;

	    /* Expand the current arg. */
	    tokenize(*args);
	    if (!(com = parsereg(*args))) {
		untokenize(*args);
		zwarnnam(nam, "bad pattern: %s", *args, 0);
		continue;
	    }
	    /* Loop over expansions. */
	    for(optno = OPT_SIZE; --optno; )
		if (optno != PRIVILEGED && domatch(optns[optno].name, com, 0))
	    	    dosetopt(optno, !isun, 0);
	    args++;
	}
    }
    inittyptab();
    return 0;
}

/* print options */

static void
printoptions(int set)
{
    int optno;

    if (isset(KSHOPTIONPRINT)) {
	/* ksh-style option display -- list all options, *
	 * with an indication of current status          */
	printf("Current option settings\n");
	for(optno = 1; optno < OPT_SIZE; optno++) {
	    if (defset(optno))
		printf("no%-20s%s\n", optns[optno].name, isset(optno) ? "off" : "on");
	    else
		printf("%-22s%s\n", optns[optno].name, isset(optno) ? "on" : "off");
	}
    } else {
	/* list all options that are on, or all that are off */
	for(optno = 1; optno < OPT_SIZE; optno++) {
	    if (set == (isset(optno) ^ defset(optno))) {
		if (set ^ isset(optno))
		    fputs("no", stdout);
		puts(optns[optno].name);
	    }
	}
    }
}

/**** job control builtins ****/

/* Make sure we have a suitable current and previous job set. */

static void
setcurjob(void)
{
    if (curjob == thisjob ||
	(curjob != -1 && !(jobtab[curjob].stat & STAT_INUSE))) {
	curjob = prevjob;
	setprevjob();
	if (curjob == thisjob ||
	    (curjob != -1 && !((jobtab[curjob].stat & STAT_INUSE) &&
			       curjob != thisjob))) {
	    curjob = prevjob;
	    setprevjob();
	}
    }
}

/* bg, disown, fg, jobs, wait: most of the job control commands are     *
 * here.  They all take the same type of argument.  Exception: wait can *
 * take a pid or a job specifier, whereas the others only work on jobs. */

/**/
int
bin_fg(char *name, char **argv, char *ops, int func)
{
    int job, lng, firstjob = -1, retval = 0;

    if (ops['Z']) {
	if (*argv)
	    strcpy(hackzero, *argv);
	return 0;
    }

    lng = (ops['l']) ? 1 : (ops['p']) ? 2 : 0;
    if ((func == BIN_FG || func == BIN_BG) && !jobbing) {
	/* oops... maybe bg and fg should have been disabled? */
	zwarnnam(name, "no job control in this shell.", NULL, 0);
	return 1;
    }

    /* If necessary, update job table. */
    if (unset(NOTIFY))
	scanjobs();

    setcurjob();

    if (func == BIN_JOBS)
        /* If you immediately type "exit" after "jobs", this      *
         * will prevent zexit from complaining about stopped jobs */
	stopmsg = 2;
    if (!*argv) {
	/* This block handles all of the default cases (no arguments).  bg,
	fg and disown act on the current job, and jobs and wait act on all the
	jobs. */
 	if (func == BIN_FG || func == BIN_BG || func == BIN_DISOWN) {
	    /* W.r.t. the above comment, we'd better have a current job at this
	    point or else. */
	    if (curjob == -1 || (jobtab[curjob].stat & STAT_NOPRINT)) {
		zwarnnam(name, "no current job", NULL, 0);
		return 1;
	    }
	    firstjob = curjob;
	} else if (func == BIN_JOBS) {
	    /* List jobs. */
	    for (job = 0; job != MAXJOB; job++)
		if (job != thisjob && jobtab[job].stat) {
		    if ((!ops['r'] && !ops['s']) ||
			(ops['r'] && ops['s']) ||
			(ops['r'] && !(jobtab[job].stat & STAT_STOPPED)) ||
			(ops['s'] && jobtab[job].stat & STAT_STOPPED))
			printjob(job + jobtab, lng, 2);
		}
	    return 0;
	} else {   /* Must be BIN_WAIT, so wait for all jobs */
	    for (job = 0; job != MAXJOB; job++)
		if (job != thisjob && jobtab[job].stat)
		    waitjob(job, SIGINT);
	    return 0;
	}
    }

    /* Defaults have been handled.  We now have an argument or two, or three...
    In the default case for bg, fg and disown, the argument will be provided by
    the above routine.  We now loop over the arguments. */
    for (; (firstjob != -1) || *argv; (void)(*argv && argv++)) {
	int stopped, ocj = thisjob;

	if (func == BIN_WAIT && isanum(*argv)) {
	    /* wait can take a pid; the others can't. */
	    waitforpid((long)atoi(*argv));
	    retval = lastval2;
	    thisjob = ocj;
	    continue;
	}
	/* The only type of argument allowed now is a job spec.  Check it. */
	job = (*argv) ? getjob(*argv, name) : firstjob;
	firstjob = -1;
	if (job == -1) {
	    retval = 1;
	    break;
	}
	if (!(jobtab[job].stat & STAT_INUSE) ||
	    (jobtab[job].stat & STAT_NOPRINT)) {
	    zwarnnam(name, "no such job: %d", 0, job);
	    return 1;
	}
	/* We have a job number.  Now decide what to do with it. */
	switch (func) {
	case BIN_FG:
	case BIN_BG:
	case BIN_WAIT:
	    if ((stopped = (jobtab[job].stat & STAT_STOPPED)))
		makerunning(jobtab + job);
	    else if (func == BIN_BG) {
		/* Silly to bg a job already running. */
		zwarnnam(name, "job already in background", NULL, 0);
		thisjob = ocj;
		return 1;
	    }
	    /* It's time to shuffle the jobs around!  Reset the current job,
	    and pick a sensible secondary job. */
	    if (curjob == job) {
		curjob = prevjob;
		prevjob = (func == BIN_BG) ? -1 : job;
	    }
	    if (prevjob == job || prevjob == -1)
		setprevjob();
	    if (curjob == -1) {
		curjob = prevjob;
		setprevjob();
	    }
	    if (func != BIN_WAIT)
		/* for bg and fg -- show the job we are operating on */
		printjob(jobtab + job, (stopped) ? -1 : 0, 1);
	    if (func == BIN_BG)
		jobtab[job].stat |= STAT_NOSTTY;
	    else {				/* fg or wait */
		if (strcmp(jobtab[job].pwd, pwd)) {
		    fprintf(shout, "(pwd : ");
		    fprintdir(jobtab[job].pwd, shout);
		    fprintf(shout, ")\n");
		}
		fflush(shout);
		if (func != BIN_WAIT) {		/* fg */
		    thisjob = job;
		    if ((jobtab[job].stat & STAT_SUPERJOB) &&
			((!jobtab[job].procs->next ||
			  (jobtab[job].stat & STAT_SUBLEADER) ||
			  killpg(jobtab[job].gleader, 0) == -1)) &&
			jobtab[jobtab[job].other].gleader)
			attachtty(jobtab[jobtab[job].other].gleader);
		    else
			attachtty(jobtab[job].gleader);
		}
	    }
	    if (stopped) {
		if (func != BIN_BG && jobtab[job].ty)
		    settyinfo(jobtab[job].ty);
		killjb(jobtab + job, SIGCONT);
	    }
	    if (func == BIN_WAIT)
	        waitjob(job, SIGINT);
	    if (func != BIN_BG) {
		waitjobs();
		retval = lastval2;
	    }
	    break;
	case BIN_JOBS:
	    printjob(job + jobtab, lng, 2);
	    break;
	case BIN_DISOWN:
	    {
		static struct job zero;

		jobtab[job] = zero;
		break;
	    }
	}
	thisjob = ocj;
    }
    return retval;
}

/* kill: send a signal to a process.  The process(es) may be specified *
 * by job specifier (see above) or pid.  A signal, defaulting to       *
 * SIGTERM, may be specified by name or number, preceded by a dash.    */

/**/
int
bin_kill(char *nam, char **argv, char *ops, int func)
{
    int sig = SIGTERM;
    int returnval = 0;

    /* check for, and interpret, a signal specifier */
    if (*argv && **argv == '-') {
	if (idigit((*argv)[1]))
	    /* signal specified by number */
	    sig = atoi(*argv + 1);
	else if ((*argv)[1] != '-' || (*argv)[2]) {
	    char *signame;

	    /* with argument "-l" display the list of signal names */
	    if ((*argv)[1] == 'l' && (*argv)[2] == '\0') {
		if (argv[1]) {
		    while (*++argv) {
			sig = zstrtol(*argv, &signame, 10);
			if (signame == *argv) {
			    for (sig = 1; sig <= SIGCOUNT; sig++)
				if (!cstrpcmp(sigs + sig, &signame))
				    break;
			    if (sig > SIGCOUNT) {
				zwarnnam(nam, "unknown signal: SIG%s",
					 signame, 0);
				returnval++;
			    } else
				printf("%d\n", sig);
			} else {
			    if (*signame) {
				zwarnnam(nam, "unknown signal: SIG%s",
					 signame, 0);
				returnval++;
			    } else {
				if (WIFSIGNALED(sig))
				    sig = WTERMSIG(sig);
				else if (WIFSTOPPED(sig))
				    sig = WSTOPSIG(sig);
				if (1 <= sig && sig <= SIGCOUNT)
				    printf("%s\n", sigs[sig]);
				else
				    printf("%d\n", sig);
			    }
			}
		    }
		    return returnval;
		}
		printf("%s", sigs[1]);
		for (sig = 2; sig <= SIGCOUNT; sig++)
		    printf(" %s", sigs[sig]);
		putchar('\n');
		return 0;
	    }
	    if ((*argv)[1] == 's' && (*argv)[2] == '\0')
		signame = *++argv;
	    else
		signame = *argv + 1;

	    /* check for signal matching specified name */
	    for (sig = 1; sig <= SIGCOUNT; sig++)
		if (!cstrpcmp(sigs + sig, &signame))
		    break;
	    if (*signame == '0' && !signame[1])
		sig = 0;
	    if (sig > SIGCOUNT) {
		zwarnnam(nam, "unknown signal: SIG%s", signame, 0);
		zwarnnam(nam, "type kill -l for a List of signals", NULL, 0);
		return 1;
	    }
	}
	argv++;
    }

    setcurjob();

    /* Remaining arguments specify processes.  Loop over them, and send the
    signal (number sig) to each process. */
    for (; *argv; argv++) {
	if (**argv == '%') {
	    /* job specifier introduced by '%' */
	    int p;

	    if ((p = getjob(*argv, nam)) == -1) {
		returnval++;
		continue;
	    }
	    if (killjb(jobtab + p, sig) == -1) {
		zwarnnam("kill", "kill %s failed: %e", *argv, errno);
		returnval++;
		continue;
	    }
	    /* automatically update the job table if sending a SIGCONT to a
	    job, and send the job a SIGCONT if sending it a non-stopping
	    signal. */
	    if (jobtab[p].stat & STAT_STOPPED) {
		if (sig == SIGCONT)
		    jobtab[p].stat &= ~STAT_STOPPED;
		if (sig != SIGKILL && sig != SIGCONT && sig != SIGTSTP
		    && sig != SIGTTOU && sig != SIGTTIN && sig != SIGSTOP)
		    killjb(jobtab + p, SIGCONT);
	    }
	} else if (!isanum(*argv)) {
	    zwarnnam("kill", "illegal pid: %s", *argv, 0);
	    returnval++;
	} else if (kill(atoi(*argv), sig) == -1) {
	    zwarnnam("kill", "kill %s failed: %e", *argv, errno);
	    returnval++;
	}
    }
    return returnval < 126 ? returnval : 1;
}

/* Suspend this shell */

/**/
int
bin_suspend(char *name, char **argv, char *ops, int func)
{
    /* won't suspend a login shell, unless forced */
    if (islogin && !ops['f']) {
	zwarnnam(name, "can't suspend login shell", NULL, 0);
	return 1;
    }
    if (jobbing) {
	/* stop ignoring signals */
	signal_default(SIGTTIN);
	signal_default(SIGTSTP);
	signal_default(SIGTTOU);
    }
    /* suspend ourselves with a SIGTSTP */
    kill(0, SIGTSTP);
    if (jobbing) {
	/* stay suspended */
	while (gettygrp() != mypgrp) {
	    sleep(1);
	    if (gettygrp() != mypgrp)
		kill(0, SIGTTIN);
	}
	/* restore signal handling */
	signal_ignore(SIGTTOU);
	signal_ignore(SIGTSTP);
	signal_ignore(SIGTTIN);
    }
    return 0;
}

/* find a job named s */

/**/
int
findjobnam(char *s)
{
    int jobnum;

    for (jobnum = MAXJOB - 1; jobnum >= 0; jobnum--)
	if (!(jobtab[jobnum].stat & (STAT_SUBJOB | STAT_NOPRINT)) &&
	    jobtab[jobnum].stat && jobtab[jobnum].procs && jobnum != thisjob &&
	    jobtab[jobnum].procs->text && strpfx(s, jobtab[jobnum].procs->text))
	    return jobnum;
    return -1;
}

/* Convert a job specifier ("%%", "%1", "%foo", "%?bar?", etc.) *
 * to a job number.                                             */

/**/
int
getjob(char *s, char *prog)
{
    int jobnum, returnval;

    /* if there is no %, treat as a name */
    if (*s != '%')
	goto jump;
    s++;
    /* "%%", "%+" and "%" all represent the current job */
    if (*s == '%' || *s == '+' || !*s) {
	if (curjob == -1) {
	    zwarnnam(prog, "no current job", NULL, 0);
	    returnval = -1;
	    goto done;
	}
	returnval = curjob;
	goto done;
    }
    /* "%-" represents the previous job */
    if (*s == '-') {
	if (prevjob == -1) {
	    zwarnnam(prog, "no previous job", NULL, 0);
	    returnval = -1;
	    goto done;
	}
	returnval = prevjob;
	goto done;
    }
    /* a digit here means we have a job number */
    if (idigit(*s)) {
	jobnum = atoi(s);
	if (jobnum && jobnum < MAXJOB && jobtab[jobnum].stat &&
	    !(jobtab[jobnum].stat & STAT_SUBJOB) && jobnum != thisjob) {
	    returnval = jobnum;
	    goto done;
	}
	zwarnnam(prog, "%%%s: no such job", s, 0);
	returnval = -1;
	goto done;
    }
    /* "%?" introduces a search string */
    if (*s == '?') {
	struct process *pn;

	for (jobnum = MAXJOB - 1; jobnum >= 0; jobnum--)
	    if (jobtab[jobnum].stat && !(jobtab[jobnum].stat & STAT_SUBJOB) &&
		jobnum != thisjob)
		for (pn = jobtab[jobnum].procs; pn; pn = pn->next)
		    if (strstr(pn->text, s + 1)) {
			returnval = jobnum;
			goto done;
		    }
	zwarnnam(prog, "job not found: %s", s, 0);
	returnval = -1;
	goto done;
    }
  jump:
    /* anything else is a job name, specified as a string that begins the
    job's command */
    if ((jobnum = findjobnam(s)) != -1) {
	returnval = jobnum;
	goto done;
    }
    /* if we get here, it is because none of the above succeeded and went
    to done */
    zwarnnam(prog, "job not found: %s", s, 0);
    returnval = -1;
  done:
    return returnval;
}

/* This simple function indicates whether or not s may represent      *
 * a number.  It returns true iff s consists purely of digits and     *
 * minuses.  Note that minus may appear more than once, and the empty *
 * string will produce a `true' response.                             */

/**/
int
isanum(char *s)
{
    while (*s == '-' || idigit(*s))
	s++;
    return *s == '\0';
}

/**** directory-handling builtins ****/

int doprintdir = 0;		/* set in exec.c (for autocd) */

/* pwd: display the name of the current directory */

/**/
int
bin_pwd(char *name, char **argv, char *ops, int func)
{
    if (ops['r'] || isset(CHASELINKS))
	printf("%s\n", zgetcwd());
    else {
	zputs(pwd, stdout);
	putchar('\n');
    }
    return 0;
}

/* dirs: list the directory stack, or replace it with a provided list */

/**/
int
bin_dirs(char *name, char **argv, char *ops, int func)
{
    LinkList l;

    /* with the -v option, provide a numbered list of directories, starting at
    zero */
    if (ops['v']) {
	LinkNode node;
	int pos = 1;

	printf("0\t");
	fprintdir(pwd, stdout);
	for (node = firstnode(dirstack); node; incnode(node)) {
	    printf("\n%d\t", pos++);
	    fprintdir(getdata(node), stdout);
	}
	putchar('\n');
	return 0;
    }
    /* given no arguments, list the stack normally */
    if (!*argv) {
	printdirstack();
	return 0;
    }
    /* replace the stack with the specified directories */
    PERMALLOC {
	l = newlinklist();
	if (*argv) {
	    while (*argv)
		addlinknode(l, ztrdup(*argv++));
	    freelinklist(dirstack, freestr);
	    dirstack = l;
	}
    } LASTALLOC;
    return 0;
}

/* cd, chdir, pushd, popd */

/* The main pwd changing function.  The real work is done by other     *
 * functions.  cd_get_dest() does the initial argument processing;     *
 * cd_do_chdir() actually changes directory, if possible; cd_new_pwd() *
 * does the ancilliary processing associated with actually changing    *
 * directory.                                                          */

/**/
int
bin_cd(char *nam, char **argv, char *ops, int func)
{
    LinkNode dir;
    struct stat st1, st2;

    doprintdir = (doprintdir == -1);

    PERMALLOC {
	pushnode(dirstack, ztrdup(pwd));
	if (!(dir = cd_get_dest(nam, argv, ops, func))) {
	    zsfree(getlinknode(dirstack));
	    LASTALLOC_RETURN 1;
	}
    } LASTALLOC;
    cd_new_pwd(func, dir);

    if (stat(unmeta(pwd), &st1) < 0) {
	zsfree(pwd);
	pwd = metafy(zgetcwd(), -1, META_REALLOC);
    } else if (stat(".", &st2) < 0)
	chdir(unmeta(pwd));
    else if (st1.st_ino != st2.st_ino || st1.st_dev != st2.st_dev) {
	if (isset(CHASELINKS)) {
	    zsfree(pwd);
	    pwd = metafy(zgetcwd(), -1, META_REALLOC);
	} else {
	    chdir(unmeta(pwd));
	}
    }
    return 0;
}

/* Get directory to chdir to */

/**/
LinkNode
cd_get_dest(char *nam, char **argv, char *ops, int func)
{
    LinkNode dir = NULL;
    LinkNode target;
    char *dest;

    if (!argv[0]) {
	if (func == BIN_POPD && !nextnode(firstnode(dirstack))) {
	    zwarnnam(nam, "directory stack empty", NULL, 0);
	    return NULL;
	}
	if (func == BIN_PUSHD && unset(PUSHDTOHOME))
	    dir = nextnode(firstnode(dirstack));
	if (dir)
	    insertlinknode(dirstack, dir, getlinknode(dirstack));
	else if (func != BIN_POPD)
	    pushnode(dirstack, ztrdup(home));
    } else if (!argv[1]) {
	int dd;
	char *end;

	doprintdir++;
	if (argv[0][1] && (argv[0][0] == '+' || argv[0][0] == '-')) {
	    dd = zstrtol(argv[0] + 1, &end, 10); 
	    if (*end == '\0') {
		if ((argv[0][0] == '+') ^ isset(PUSHDMINUS))
		    for (dir = firstnode(dirstack); dir && dd; dd--, incnode(dir));
		else
		    for (dir = lastnode(dirstack); dir != (LinkNode) dirstack && dd;
			 dd--, dir = prevnode(dir)); 
		if (!dir || dir == (LinkNode) dirstack) {
		    zwarnnam(nam, "no such entry in dir stack", NULL, 0);
		    return NULL;
		}
	    }
	}
	if (!dir)
	    pushnode(dirstack, ztrdup(strcmp(argv[0], "-")
				      ? (doprintdir--, argv[0]) : oldpwd));
    } else {
	char *u, *d;
	int len1, len2, len3;

	if (!(u = strstr(pwd, argv[0]))) {
	    zwarnnam(nam, "string not in pwd: %s", argv[0], 0);
	    return NULL;
	}
	len1 = strlen(argv[0]);
	len2 = strlen(argv[1]);
	len3 = u - pwd;
	d = (char *)zalloc(len3 + len2 + strlen(u + len1) + 1);
	strncpy(d, pwd, len3);
	strcpy(d + len3, argv[1]);
	strcat(d, u + len1);
	pushnode(dirstack, d);
	doprintdir++;
    }

    target = dir;
    if (func == BIN_POPD) {
	if (!dir) {
	    target = dir = firstnode(dirstack);
	} else if (dir != firstnode(dirstack)) {
	    return dir;
	}
	dir = nextnode(dir);
    }
    if (!dir) {
	dir = firstnode(dirstack);
    }
    if (!(dest = cd_do_chdir(nam, getdata(dir)))) {
	if (!target)
	    zsfree(getlinknode(dirstack));
	if (func == BIN_POPD)
	    zsfree(remnode(dirstack, dir));
	return NULL;
    }
    if (dest != getdata(dir)) {
	zsfree(getdata(dir));
	setdata(dir, dest);
    }
    return target ? target : dir;
}

/* Change to given directory, if possible.  This function works out  *
 * exactly how the directory should be interpreted, including cdpath *
 * and CDABLEVARS.  For each possible interpretation of the given    *
 * path, this calls cd_try_chdir(), which attempts to chdir to that  *
 * particular path.                                                  */

/**/
char *
cd_do_chdir(char *cnam, char *dest)
{
    char **pp, *ret;
    int hasdot = 0, eno = ENOENT;
    /* nocdpath indicates that cdpath should not be used.  This is the case iff
    dest is a relative path whose first segment is . or .., but if the path is
    absolute then cdpath won't be used anyway. */
    int nocdpath = dest[0] == '.' &&
    (dest[1] == '/' || !dest[1] || (dest[1] == '.' &&
				    (dest[2] == '/' || !dest[2])));

    /* if we have an absolute path, use it as-is only */
#if defined(_WIN32)
    if (is_win32abspath(dest)) {
#else
    if (*dest == '/') {
#endif
	if ((ret = cd_try_chdir(NULL, dest)))
	    return ret;
	zwarnnam(cnam, "%e: %s", dest, errno);
	return NULL;
    }

    /* if cdpath is being used, check it for . */
    if (!nocdpath)
	for (pp = cdpath; *pp; pp++)
	    if (!(*pp)[0] || ((*pp)[0] == '.' && (*pp)[1] == '\0'))
		hasdot = 1;
    /* if there is no . in cdpath (or it is not being used), try the directory
    as-is (i.e. from .) */
    if (!hasdot) {
	if ((ret = cd_try_chdir(NULL, dest)))
	    return ret;
	if (errno != ENOENT)
	    eno = errno;
    }
    /* if cdpath is being used, try given directory relative to each element in
    cdpath in turn */
    if (!nocdpath)
	for (pp = cdpath; *pp; pp++) {
	    if ((ret = cd_try_chdir(*pp, dest))) {
		if (strcmp(*pp, ".")) {
		    doprintdir++;
		}
		return ret;
	    }
	    if (errno != ENOENT)
		eno = errno;
	}

    /* handle the CDABLEVARS option */
    if ((ret = cd_able_vars(dest))) {
	if ((ret = cd_try_chdir(NULL, ret))) {
	    doprintdir++;
	    return ret;
	}
	if (errno != ENOENT)
	    eno = errno;
    }

    /* If we got here, it means that we couldn't chdir to any of the
    multitudinous possible paths allowed by zsh.  We've run out of options!
    Add more here! */
    zwarnnam(cnam, "%e: %s", dest, eno);
    return NULL;
}

/* If the CDABLEVARS option is set, return the new *
 * interpretation of the given path.               */

/**/
char *
cd_able_vars(char *s)
{
    char *rest, save;

    if (isset(CDABLEVARS)) {
	for (rest = s; *rest && *rest != '/'; rest++);
	save = *rest;
	*rest = 0;
	s = getnameddir(s);
	*rest = save;

	if (s && *rest)
	    s = dyncat(s, rest);

	return s;
    }
    return NULL;
}

/* Attempt to change to a single given directory.  The directory,    *
 * for the convenience of the calling function, may be provided in   *
 * two parts, which must be concatenated before attempting to chdir. *
 * Returns NULL if the chdir fails.  If the directory change is      *
 * possible, it is performed, and a pointer to the new full pathname *
 * is returned.                                                      */

/**/
char *
cd_try_chdir(char *pfix, char *dest)
{
    char buf[PATH_MAX], buf2[PATH_MAX];
    char *s;
    int dotsct;

    /* handle directory prefix */
    if (pfix && *pfix) {
	if (strlen(dest) + strlen(pfix) + 1 >= PATH_MAX)
	    return NULL;
	sprintf(buf, "%s/%s", (!strcmp("/", pfix)) ? "" : pfix, dest);
    } else {
	if (strlen(dest) >= PATH_MAX)
	    return NULL;
	strcpy(buf, dest);
    }
    /* Normalise path.  See the definition of fixdir() for what this means. */
    dotsct = fixdir(buf2, buf);

    /* if the path is absolute, the test and return value are (relatively)
    simple */
#if defined(_WIN32)
    /* see the "fix cd /. -amol 4/14/97" */
    if (is_win32abspath(buf2)) {
	if (chdir(unmeta(buf)) < 0)
	    return NULL;
	else {
	    getcwd(buf2, PATH_MAX);
	    return ztrdup(buf2);
	}
    }
#else
    if (buf2[0] == '/')
	return (chdir(unmeta(buf)) == -1) ? NULL : ztrdup(buf2);
#endif
    /* If the path is a simple `downward' relative path, the test is again
    fairly simple.  The relative path must be added to the end of the current
    directory. */
    if (!dotsct) {
	if (chdir(unmeta(buf)) == -1)
	    return NULL;
	if (*buf2) {
	    if (strlen(pwd) + strlen(buf2) + 1 >= PATH_MAX)
		return NULL;
#if defined(_WIN32)
    	    if ((pwd[3] == 0) && (pwd[1] == ':') && (pwd[2] == '/'))
		sprintf(buf, "%s%s", pwd, buf2);
	    else
		sprintf(buf, "%s/%s", pwd, buf2);
#else
	    sprintf(buf, "%s/%s", (!strcmp("/", pwd)) ? "" : pwd, buf2);
#endif
	} else
	    strcpy(buf, pwd);
	return ztrdup(buf);
    }
    /* There are one or more .. segments at the beginning of the relative path.
    A corresponding number of segments must be removed from the end of the
    current directory before the downward relative path is appended. */
    strcpy(buf, pwd);
    s = buf + strlen(buf) - 1;
    while (dotsct--)
	while (s != buf)
	    if (*--s == '/')
		break;
    if (s == buf || *buf2)
	s++;
    strcpy(s, buf2);
    /* For some reason, this chdir must be attempted with both the newly
    created path and the original non-normalised version. */
#if defined(_WIN32)
    if (buf[1]==':' && buf[2] == '\0') {
	buf[2] = '/';
	buf[3]='\0';
    }
#endif
    if (chdir(unmeta(buf)) != -1 || chdir(unmeta(dest)) != -1)
	return ztrdup(buf);
    return NULL;
}

/* do the extra processing associated with changing directory */

/**/
void
cd_new_pwd(int func, LinkNode dir)
{
    Param pm;
    List l;
    char *new_pwd, *s;
    int dirstacksize;

    if (func == BIN_PUSHD)
	rolllist(dirstack, dir);
    new_pwd = remnode(dirstack, dir);

    if (func == BIN_POPD && firstnode(dirstack)) {
	zsfree(new_pwd);
	new_pwd = getlinknode(dirstack);
    } else if (func == BIN_CD && unset(AUTOPUSHD))
	zsfree(getlinknode(dirstack));

    if (isset(CHASELINKS)) {
	s = new_pwd;
	new_pwd = findpwd(s);
	zsfree(s);
    }
    if (isset(PUSHDIGNOREDUPS)) {
	LinkNode n; 
	for (n = firstnode(dirstack); n; incnode(n)) {
	    if (!strcmp(new_pwd, getdata(n))) {
		zsfree(remnode(dirstack, n));
		break;
	    }
	}
    }

    /* shift around the pwd variables, to make oldpwd and pwd relate to the
    current (i.e. new) pwd */
    zsfree(oldpwd);
    oldpwd = pwd;
#if defined(_WIN32)
    caseify_pwd(new_pwd);
#endif
    pwd = new_pwd;
    /* update the PWD and OLDPWD shell parameters */
    if ((pm = (Param) paramtab->getnode(paramtab, "PWD")) &&
	(pm->flags & PM_EXPORTED) && pm->env)
	pm->env = replenv(pm->env, pwd);
    if ((pm = (Param) paramtab->getnode(paramtab, "OLDPWD")) &&
	(pm->flags & PM_EXPORTED) && pm->env)
	pm->env = replenv(pm->env, oldpwd);
    if (unset(PUSHDSILENT) && func != BIN_CD && isset(INTERACTIVE))
	printdirstack();
    else if (doprintdir) {
	fprintdir(pwd, stdout);
        putchar('\n');
    }

    /* execute the chpwd function */
    if ((l = getshfunc("chpwd"))) {
	fflush(stdout);
	fflush(stderr);
	doshfunc(l, NULL, 0, 1);
    }

    dirstacksize = getiparam("DIRSTACKSIZE");
    /* handle directory stack sizes out of range */
    if (dirstacksize > 0) {
	int remove = countlinknodes(dirstack) -
		     (dirstacksize < 2 ? 2 : dirstacksize);
	while (remove-- >= 0)
	    zsfree(remnode(dirstack, lastnode(dirstack)));
    }
}

/* Print the directory stack */

/**/
void
printdirstack(void)
{
    LinkNode node;

    fprintdir(pwd, stdout);
    for (node = firstnode(dirstack); node; incnode(node)) {
	putchar(' ');
	fprintdir(getdata(node), stdout);
    }
    putchar('\n');
}

/* Normalise a path.  Segments consisting of ., and foo/.. combinations, *
 * are removed.  The number of .. segments at the beginning of the       *
 * path is returned.  The normalised path, minus leading ..s, is copied  *
 * to dest.                                                              */

/**/
int
fixdir(char *dest, char *src)
{
    int ct = 0;
    char *d0 = dest;

/*** if have RFS superroot directory ***/
#ifdef HAVE_SUPERROOT
    /* allow /.. segments to remain */
    while (*src == '/' && src[1] == '.' && src[2] == '.' &&
      (!src[3] || src[3] == '/')) {
	*dest++ = '/';
	*dest++ = '.';
	*dest++ = '.';
	src += 3;
    }
#endif

    for (;;) {
	/* compress multiple /es into single */
	if (*src == '/') {
	    *dest++ = *src++;
	    while (*src == '/')
		src++;
	}
	/* if we are at the end of the input path, remove a trailing / (if it
	exists), and return ct */
	if (!*src) {
	    while (dest > d0 + 1 && dest[-1] == '/')
		dest--;
	    *dest = '\0';
	    return ct;
	}
	if (src[0] == '.' && src[1] == '.' &&
	  (src[2] == '\0' || src[2] == '/')) {
	    /* remove a foo/.. combination, or increment ct, as appropriate */
	    if (dest > d0 + 1) {
		for (dest--; dest > d0 + 1 && dest[-1] != '/'; dest--);
		if (dest[-1] != '/')
		    dest--;
	    } else
		ct++;
	    src++;
	    while (*++src == '/');
	} else if (src[0] == '.' && (src[1] == '/' || src[1] == '\0')) {
	    /* skip a . section */
	    while (*++src == '/');
	} else {
	    /* copy a normal segment into the output */
	    while (*src != '/' && *src != '\0')
		*dest++ = *src++;
	}
    }
}

/**** compctl builtin ****/

#define COMP_LIST	(1<<0)	/* -L */
#define COMP_COMMAND	(1<<1)	/* -C */
#define COMP_DEFAULT	(1<<2)	/* -D */
#define COMP_FIRST	(1<<3)	/* -T */
#define COMP_REMOVE	(1<<4)

#define COMP_SPECIAL (COMP_COMMAND|COMP_DEFAULT|COMP_FIRST)

/* Flag for listing, command, default, or first completion */
static int cclist;

/* Mask for determining what to print */
static unsigned long showmask = 0;

/* Main entry point for the `compctl' builtin */

/**/
int
bin_compctl(char *name, char **argv, char *ops, int func)
{
    Compctl cc = NULL;
    int ret = 0;

    /* clear static flags */
    cclist = 0;
    showmask = 0;

    /* Parse all the arguments */
    if (*argv) {
	cc = (Compctl) zcalloc(sizeof(*cc));
	if (get_compctl(name, &argv, cc, 1, 0)) {
	    freecompctl(cc);
	    return 1;
	}

	/* remember flags for printing */
	showmask = cc->mask;
	if ((showmask & CC_EXCMDS) && !(showmask & CC_DISCMDS))
	    showmask &= ~CC_EXCMDS;

	/* if no command arguments or just listing, we don't want cc */
	if (!*argv || (cclist & COMP_LIST))
	    freecompctl(cc);
    }

    /* If no commands and no -C, -T, or -D, print all the compctl's *
     * If some flags (other than -C, -T, or -D) were given, then    *
     * only print compctl containing those flags.                   */
    if (!*argv && !(cclist & COMP_SPECIAL)) {
	scanhashtable(compctltab, 1, 0, 0, compctltab->printnode, 0);
	printcompctl((cclist & COMP_LIST) ? "" : "COMMAND", &cc_compos, 0);
	printcompctl((cclist & COMP_LIST) ? "" : "DEFAULT", &cc_default, 0);
 	printcompctl((cclist & COMP_LIST) ? "" : "FIRST", &cc_first, 0);
	return ret;
    }

    /* If we're listing and we've made it to here, then there are arguments *
     * or a COMP_SPECIAL flag (-D, -C, -T), so print only those.            */
    if (cclist & COMP_LIST) {
	HashNode hn;
	char **ptr;

	showmask = 0;
	for (ptr = argv; *ptr; ptr++) {
	    if ((hn = compctltab->getnode(compctltab, *ptr))) {
		compctltab->printnode(hn, 0);
	    } else {
		zwarnnam(name, "no compctl defined for %s", *ptr, 0);
		ret = 1;
	    }
	}
	if (cclist & COMP_COMMAND)
	    printcompctl("", &cc_compos, 0);
	if (cclist & COMP_DEFAULT)
	    printcompctl("", &cc_default, 0);
	if (cclist & COMP_FIRST)
	    printcompctl("", &cc_first, 0);
	return ret;
    }

    /* Assign the compctl to the commands given */
    if (*argv) {
	if(cclist & COMP_SPECIAL)
	    /* Ideally we'd handle this properly, setting both the *
	     * special and normal completions.  For the moment,    *
	     * this is better than silently failing.               */
	    zwarnnam(name, "extraneous commands ignored", NULL, 0);
	else
	    compctl_process_cc(argv, cc);
    }

    return ret;
}

/* Parse the basic flags for `compctl' */

/**/
int
get_compctl(char *name, char ***av, Compctl cc, int first, int isdef)
{
    /* Parse the basic flags for completion:
     * first is a flag that we are not in extended completion,
     * while hx indicates or (+) completion (need to know for
     * default and command completion as the initial compctl is special). 
     * cct is a temporary just to hold flags; it never needs freeing.
     */
    struct compctl cct;
    char **argv = *av;
    int ready = 0, hx = 0;

    /* Handle `compctl + foo ...' specially:  turn it into
     * a default compctl by removing it from the hash table.
     */
    if (first && argv[0][0] == '+' && !argv[0][1] &&
	!(argv[1] && argv[1][0] == '-' && argv[1][1])) {
	argv++;
	if(argv[0] && argv[0][0] == '-')
	    argv++;
	*av = argv;
	freecompctl(cc);
 	cclist = COMP_REMOVE;
	return 0;
    }

    memset((void *)&cct, 0, sizeof(cct));

    /* Loop through the flags until we have no more:        *
     * those with arguments are not properly allocated yet, *
     * we just hang on to the argument that was passed.     */
    for (; !ready && argv[0] && argv[0][0] == '-' && (argv[0][1] || !first);) {
	if (!argv[0][1])
	    *argv = "-+";
	while (!ready && *++(*argv)) {
	    if(**argv == Meta)
		*++*argv ^= 32;
	    switch (**argv) {
	    case 'f':
		cct.mask |= CC_FILES;
		break;
	    case 'c':
		cct.mask |= CC_COMMPATH;
		break;
	    case 'm':
		cct.mask |= CC_EXTCMDS;
		break;
	    case 'w':
		cct.mask |= CC_RESWDS;
		break;
	    case 'o':
		cct.mask |= CC_OPTIONS;
		break;
	    case 'v':
		cct.mask |= CC_VARS;
		break;
	    case 'b':
		cct.mask |= CC_BINDINGS;
		break;
	    case 'A':
		cct.mask |= CC_ARRAYS;
		break;
	    case 'I':
		cct.mask |= CC_INTVARS;
		break;
	    case 'F':
		cct.mask |= CC_SHFUNCS;
		break;
	    case 'p':
		cct.mask |= CC_PARAMS;
		break;
	    case 'E':
		cct.mask |= CC_ENVVARS;
		break;
	    case 'j':
		cct.mask |= CC_JOBS;
		break;
	    case 'r':
		cct.mask |= CC_RUNNING;
		break;
	    case 'z':
		cct.mask |= CC_STOPPED;
		break;
	    case 'B':
		cct.mask |= CC_BUILTINS;
		break;
	    case 'a':
		cct.mask |= CC_ALREG | CC_ALGLOB;
		break;
	    case 'R':
		cct.mask |= CC_ALREG;
		break;
	    case 'G':
		cct.mask |= CC_ALGLOB;
		break;
	    case 'u':
		cct.mask |= CC_USERS;
		break;
	    case 'd':
		cct.mask |= CC_DISCMDS;
		break;
	    case 'e':
		cct.mask |= CC_EXCMDS;
		break;
	    case 'N':
		cct.mask |= CC_SCALARS;
		break;
	    case 'O':
		cct.mask |= CC_READONLYS;
		break;
	    case 'Z':
		cct.mask |= CC_SPECIALS;
		break;
	    case 'q':
		cct.mask |= CC_REMOVE;
		break;
	    case 'U':
		cct.mask |= CC_DELETE;
		break;
	    case 'n':
		cct.mask |= CC_NAMED;
		break;
	    case 'Q':
		cct.mask |= CC_QUOTEFLAG;
		break;
	    case 'k':
		if ((*argv)[1]) {
		    cct.keyvar = (*argv) + 1;
		    *argv = "" - 1;
		} else if (!argv[1]) {
		    zwarnnam(name, "variable name expected after -%c", NULL,
			    **argv);
		    return 1;
		} else {
		    cct.keyvar = *++argv;
		    *argv = "" - 1;
		}
		break;
	    case 'K':
		if ((*argv)[1]) {
		    cct.func = (*argv) + 1;
		    *argv = "" - 1;
		} else if (!argv[1]) {
		    zwarnnam(name, "function name expected after -%c", NULL,
			    **argv);
		    return 1;
		} else {
		    cct.func = *++argv;
		    *argv = "" - 1;
		}
		break;
	    case 'X':
		if ((*argv)[1]) {
		    cct.explain = (*argv) + 1;
		    *argv = "" - 1;
		} else if (!argv[1]) {
		    zwarnnam(name, "string expected after -%c", NULL, **argv);
		    return 1;
		} else {
		    cct.explain = *++argv;
		    *argv = "" - 1;
		}
		break;
	    case 'P':
		if ((*argv)[1]) {
		    cct.prefix = (*argv) + 1;
		    *argv = "" - 1;
		} else if (!argv[1]) {
		    zwarnnam(name, "string expected after -%c", NULL, **argv);
		    return 1;
		} else {
		    cct.prefix = *++argv;
		    *argv = "" - 1;
		}
		break;
	    case 'S':
		if ((*argv)[1]) {
		    cct.suffix = (*argv) + 1;
		    *argv = "" - 1;
		} else if (!argv[1]) {
		    zwarnnam(name, "string expected after -%c", NULL, **argv);
		    return 1;
		} else {
		    cct.suffix = *++argv;
		    *argv = "" - 1;
		}
		break;
	    case 'g':
		if ((*argv)[1]) {
		    cct.glob = (*argv) + 1;
		    *argv = "" - 1;
		} else if (!argv[1]) {
		    zwarnnam(name, "glob pattern expected after -%c", NULL,
			    **argv);
		    return 1;
		} else {
		    cct.glob = *++argv;
		    *argv = "" - 1;
		}
		break;
	    case 's':
		if ((*argv)[1]) {
		    cct.str = (*argv) + 1;
		    *argv = "" - 1;
		} else if (!argv[1]) {
		    zwarnnam(name, "command string expected after -%c", NULL,
			    **argv);
		    return 1;
		} else {
		    cct.str = *++argv;
		    *argv = "" - 1;
		}
		break;
	    case 'l':
		if ((*argv)[1]) {
		    cct.subcmd = (*argv) + 1;
		    *argv = "" - 1;
		} else if (!argv[1]) {
		    zwarnnam(name, "command name expected after -%c", NULL,
			    **argv);
		    return 1;
		} else {
		    cct.subcmd = *++argv;
		    *argv = "" - 1;
		}
		break;
	    case 'H':
		if ((*argv)[1])
		    cct.hnum = atoi((*argv) + 1);
		else if (argv[1])
		    cct.hnum = atoi(*++argv);
		else {
		    zwarnnam(name, "number expected after -%c", NULL,
			    **argv);
		    return 1;
		}
		if (!argv[1]) {
		    zwarnnam(name, "missing pattern after -%c", NULL,
			    **argv);
		    return 1;
		}
		cct.hpat = *++argv;
		if (cct.hnum < 1)
		    cct.hnum = 0;
		if (*cct.hpat == '*' && !cct.hpat[1])
		    cct.hpat = "";
		*argv = "" - 1;
		break;
	    case 'C':
		if (first && !hx) {
		    cclist |= COMP_COMMAND;
		} else {
		    zwarnnam(name, "misplaced command completion (-C) flag",
			    NULL, 0);
		    return 1;
		}
		break;
	    case 'D':
		if (first && !hx) {
		    isdef = 1;
		    cclist |= COMP_DEFAULT;
		} else {
		    zwarnnam(name, "misplaced default completion (-D) flag",
			    NULL, 0);
		    return 1;
		}
		break;
 	    case 'T':
              if (first && !hx) {
 		    cclist |= COMP_FIRST;
 		} else {
 		    zwarnnam(name, "misplaced first completion (-T) flag",
 			    NULL, 0);
 		    return 1;
 		}
 		break;
	    case 'L':
		if (!first || hx) {
		    zwarnnam(name, "illegal use of -L flag", NULL, 0);
		    return 1;
		}
		cclist |= COMP_LIST;
		break;
	    case 'x':
		if (!argv[1]) {
		    zwarnnam(name, "condition expected after -%c", NULL,
			    **argv);
		    return 1;
		}
		if (first) {
		    argv++;
		    if (get_xcompctl(name, &argv, &cct, isdef)) {
			if (cct.ext)
			    freecompctl(cct.ext);
			return 1;
		    }
		    ready = 2;
		} else {
		    zwarnnam(name, "recursive extended completion not allowed",
			    NULL, 0);
		    return 1;
		}
		break;
	    default:
		if (!first && (**argv == '-' || **argv == '+'))
		    (*argv)--, argv--, ready = 1;
		else {
		    zwarnnam(name, "bad option: -%c", NULL, **argv);
		    return 1;
		}
	    }
	}

	if (*++argv && (!ready || ready == 2) &&
	    **argv == '+' && !argv[0][1]) {
	    /* There's an alternative (+) completion:  assign
	     * what we have so far before moving on to that.
	     */
	    if (cc_assign(name, &cc, &cct, first && !hx))
		return 1;

	    hx = 1;
	    ready = 0;

	    if (!*++argv || **argv != '-' ||
		(**argv == '-' && (!argv[0][1] ||
				   (argv[0][1] == '-' && !argv[0][2])))) {
		/* No argument to +, which means do default completion */
		if (isdef)
		    zwarnnam(name,
			    "recursive xor'd default completions not allowed",
			    NULL, 0);
		else
		    cc->xor = &cc_default;
	    } else {
		/* more flags follow:  prepare to loop again */
		cc->xor = (Compctl) zcalloc(sizeof(*cc));
		cc = cc->xor;
		memset((void *)&cct, 0, sizeof(cct));
	    }
	}
    }
    if (!ready && *argv && **argv == '-')
	argv++;

    if (! (cct.mask & (CC_EXCMDS | CC_DISCMDS)))
	cct.mask |= CC_EXCMDS;

    /* assign the last set of flags we parsed */
    if (cc_assign(name, &cc, &cct, first && !hx))
	return 1;

    *av = argv;

    return 0;
}

/* Handle the -x ... -- part of compctl. */

/**/
int
get_xcompctl(char *name, char ***av, Compctl cc, int isdef)
{
    char **argv = *av, *t, *tt, sav;
    int n, l = 0, ready = 0;
    Compcond m, c, o;
    Compctl *next = &(cc->ext);

    while (!ready) {
	/* o keeps track of or's, m remembers the starting condition,
	 * c is the current condition being parsed
	 */
	o = m = c = (Compcond) zcalloc(sizeof(*c));
	/* Loop over each condition:  something like 's[...][...], p[...]' */
	for (t = *argv; *t;) {
	    while (*t == ' ')
		t++;
	    /* First get the condition code */
	    switch (*t) {
	    case 's':
		c->type = CCT_CURSUF;
		break;
	    case 'S':
		c->type = CCT_CURPRE;
		break;
	    case 'p':
		c->type = CCT_POS;
		break;
	    case 'c':
		c->type = CCT_CURSTR;
		break;
	    case 'C':
		c->type = CCT_CURPAT;
		break;
	    case 'w':
		c->type = CCT_WORDSTR;
		break;
	    case 'W':
		c->type = CCT_WORDPAT;
		break;
	    case 'n':
		c->type = CCT_CURSUB;
		break;
	    case 'N':
		c->type = CCT_CURSUBC;
		break;
	    case 'm':
		c->type = CCT_NUMWORDS;
		break;
	    case 'r':
		c->type = CCT_RANGESTR;
		break;
	    case 'R':
		c->type = CCT_RANGEPAT;
		break;
	    default:
		t[1] = '\0';
		zwarnnam(name, "unknown condition code: %s", t, 0);
		zfree(m, sizeof(struct compcond));

		return 1;
	    }
	    /* Now get the arguments in square brackets */
	    if (t[1] != '[') {
		t[1] = '\0';
		zwarnnam(name, "expected condition after condition code: %s", t, 0);
		zfree(m, sizeof(struct compcond));

		return 1;
	    }
	    t++;
	    /* First count how many or'd arguments there are,
	     * marking the active ]'s and ,'s with unprintable characters.
	     */
	    for (n = 0, tt = t; *tt == '['; n++) {
		for (l = 1, tt++; *tt && l; tt++)
		    if (*tt == '\\' && tt[1])
			tt++;
		    else if (*tt == '[')
			l++;
		    else if (*tt == ']')
			l--;
		    else if (l == 1 && *tt == ',')
			*tt = '\201';
		if (tt[-1] == ']')
		    tt[-1] = '\200';
	    }

	    if (l) {
		t[1] = '\0';
		zwarnnam(name, "error after condition code: %s", t, 0);
		zfree(m, sizeof(struct compcond));

		return 1;
	    }
	    c->n = n;

	    /* Allocate space for all the arguments of the conditions */
	    if (c->type == CCT_POS ||
		c->type == CCT_NUMWORDS) {
		c->u.r.a = (int *)zcalloc(n * sizeof(int));
		c->u.r.b = (int *)zcalloc(n * sizeof(int));
	    } else if (c->type == CCT_CURSUF ||
		       c->type == CCT_CURPRE)
		c->u.s.s = (char **)zcalloc(n * sizeof(char *));

	    else if (c->type == CCT_RANGESTR ||
		     c->type == CCT_RANGEPAT) {
		c->u.l.a = (char **)zcalloc(n * sizeof(char *));
		c->u.l.b = (char **)zcalloc(n * sizeof(char *));
	    } else {
		c->u.s.p = (int *)zcalloc(n * sizeof(int));
		c->u.s.s = (char **)zcalloc(n * sizeof(char *));
	    }
	    /* Now loop over the actual arguments */
	    for (l = 0; *t == '['; l++, t++) {
		for (t++; *t && *t == ' '; t++);
		tt = t;
		if (c->type == CCT_POS ||
		    c->type == CCT_NUMWORDS) {
		    /* p[...] or m[...]:  one or two numbers expected */
		    for (; *t && *t != '\201' && *t != '\200'; t++);
		    if (!(sav = *t)) {
			zwarnnam(name, "error in condition", NULL, 0);
			freecompcond(m);
			return 1;
		    }
		    *t = '\0';
		    c->u.r.a[l] = atoi(tt);
		    /* Second argument is optional:  see if it's there */
		    if (sav == '\200')
			/* no:  copy first argument */
			c->u.r.b[l] = c->u.r.a[l];
		    else {
			tt = ++t;
			for (; *t && *t != '\200'; t++);
			if (!*t) {
			    zwarnnam(name, "error in condition", NULL, 0);
			    freecompcond(m);
			    return 1;
			}
			*t = '\0';
			c->u.r.b[l] = atoi(tt);
		    }
		} else if (c->type == CCT_CURSUF ||
			   c->type == CCT_CURPRE) {
		    /* -s[..] or -S[..]:  single string expected */
		    for (; *t && *t != '\200'; t++)
			if (*t == '\201')
			    *t = ',';
		    if (!*t) {
			zwarnnam(name, "error in condition", NULL, 0);
			freecompcond(m);
			return 1;
		    }
		    *t = '\0';
		    c->u.s.s[l] = ztrdup(tt);
		} else if (c->type == CCT_RANGESTR ||
			   c->type == CCT_RANGEPAT) {
		    /* -r[..,..] or -R[..,..]:  two strings expected */
		    for (; *t && *t != '\201'; t++);
		    if (!*t) {
			zwarnnam(name, "error in condition", NULL, 0);
			freecompcond(m);
			return 1;
		    }
		    *t = '\0';
		    c->u.l.a[l] = ztrdup(tt);
		    tt = ++t;
		    /* any more commas are text, not active */
		    for (; *t && *t != '\200'; t++)
			if (*t == '\201')
			    *t = ',';
		    if (!*t) {
			zwarnnam(name, "error in condition", NULL, 0);
			freecompcond(m);
			return 1;
		    }
		    *t = '\0';
		    c->u.l.b[l] = ztrdup(tt);
		} else {
		    /* remaining patterns are number followed by string */
		    for (; *t && *t != '\200' && *t != '\201'; t++);
		    if (!*t || *t == '\200') {
			zwarnnam(name, "error in condition", NULL, 0);
			freecompcond(m);
			return 1;
		    }
		    *t = '\0';
		    c->u.s.p[l] = atoi(tt);
		    tt = ++t;
		    for (; *t && *t != '\200'; t++)
			if (*t == '\201')
			    *t = ',';
		    if (!*t) {
			zwarnnam(name, "error in condition", NULL, 0);
			freecompcond(m);
			return 1;
		    }
		    *t = '\0';
		    c->u.s.s[l] = ztrdup(tt);
		}
	    }
	    while (*t == ' ')
		t++;
	    if (*t == ',') {
		/* Another condition to `or' */
		o->or = c = (Compcond) zcalloc(sizeof(*c));
		o = c;
		t++;
	    } else if (*t) {
		/* Another condition to `and' */
		c->and = (Compcond) zcalloc(sizeof(*c));
		c = c->and;
	    }
	}
	/* Assign condition to current compctl */
	*next = (Compctl) zcalloc(sizeof(*cc));
	(*next)->cond = m;
	argv++;
	/* End of the condition; get the flags that go with it. */
	if (get_compctl(name, &argv, *next, 0, isdef))
	    return 1;
 	if ((!argv || !*argv) && (cclist & COMP_SPECIAL))
 	    /* default, first, or command completion finished */
	    ready = 1;
	else {
	    /* see if we are looking for more conditions or are
	     * ready to return (ready = 1)
	     */
	    if (!argv || !*argv || **argv != '-' ||
		((!argv[0][1] || argv[0][1] == '+') && !argv[1])) {
		zwarnnam(name, "missing command names", NULL, 0);
		return 1;
	    }
	    if (!strcmp(*argv, "--"))
		ready = 1;
	    else if (!strcmp(*argv, "-+") && argv[1] &&
		     !strcmp(argv[1], "--")) {
		ready = 1;
		argv++;
	    }
	    argv++;
	    /* prepare to put the next lot of conditions on the end */
	    next = &((*next)->next);
	}
    }
    /* save position at end of parsing */
    *av = argv - 1;
    return 0;
}

/**/
int
cc_assign(char *name, Compctl *ccptr, Compctl cct, int reass)
{
    /* Copy over the details from the values in cct to those in *ccptr */
    Compctl cc;

    if (cct->subcmd && (cct->keyvar || cct->glob || cct->str ||
			cct->func || cct->explain || cct->prefix)) {
	zwarnnam(name, "illegal combination of options", NULL, 0);
	return 1;
    }

    /* Handle assignment of new default or command completion */
    if (reass && !(cclist & COMP_LIST)) {
	/* if not listing */
	if (cclist == (COMP_COMMAND|COMP_DEFAULT)
	    || cclist == (COMP_COMMAND|COMP_FIRST)
	    || cclist == (COMP_DEFAULT|COMP_FIRST)
	    || cclist == COMP_SPECIAL) {
 	    zwarnnam(name, "can't set -D, -T, and -C simultaneously", NULL, 0);
	    /* ... because the following code wouldn't work. */
	    return 1;
	}
	if (cclist & COMP_COMMAND) {
	    /* command */
	    *ccptr = &cc_compos;
	    cc_reassign(*ccptr);
	} else if (cclist & COMP_DEFAULT) {
	    /* default */
	    *ccptr = &cc_default;
	    cc_reassign(*ccptr);
 	} else if (cclist & COMP_FIRST) {
 	    /* first */
 	    *ccptr = &cc_first;
 	    cc_reassign(*ccptr);
	}
    }

    /* Free the old compctl */
    cc = *ccptr;
    zsfree(cc->keyvar);
    zsfree(cc->glob);
    zsfree(cc->str);
    zsfree(cc->func);
    zsfree(cc->explain);
    zsfree(cc->prefix);
    zsfree(cc->suffix);
    zsfree(cc->subcmd);
    zsfree(cc->hpat);
    
    /* and copy over the new stuff, (permanently) allocating
     * space for strings.
     */
    cc->mask = cct->mask;
    cc->keyvar = ztrdup(cct->keyvar);
    cc->glob = ztrdup(cct->glob);
    cc->str = ztrdup(cct->str);
    cc->func = ztrdup(cct->func);
    cc->explain = ztrdup(cct->explain);
    cc->prefix = ztrdup(cct->prefix);
    cc->suffix = ztrdup(cct->suffix);
    cc->subcmd = ztrdup(cct->subcmd);
    cc->hpat = ztrdup(cct->hpat);
    cc->hnum = cct->hnum;

    /* careful with extended completion:  it's already allocated */
    cc->ext = cct->ext;

    return 0;
}

/**/
void
cc_reassign(Compctl cc)
{
    /* Free up a new default or command completion:
     * this is a hack to free up the parts which should be deleted,
     * without removing the basic variable which is statically allocated
     */
    Compctl c2;

    c2 = (Compctl) zcalloc(sizeof *cc);
    c2->xor = cc->xor;
    c2->ext = cc->ext;
    c2->refc = 1;

    freecompctl(c2);

    cc->ext = cc->xor = NULL;
}

/**/
void
compctl_process(char **s, int mask, char *uk, char *gl, char *st, char *fu, char *ex, char *pr, char *su, char *sc, char *hp, int hn)
{
    /* Command called internally to initialise some basic compctls */
    Compctl cc;

    cc = (Compctl) zcalloc(sizeof *cc);
    cc->mask = mask;
    cc->keyvar = ztrdup(uk);
    cc->glob = ztrdup(gl);
    cc->str = ztrdup(st);
    cc->func = ztrdup(fu);
    cc->explain = ztrdup(ex);
    cc->prefix = ztrdup(pr);
    cc->suffix = ztrdup(su);
    cc->subcmd = ztrdup(sc);
    cc->hpat = ztrdup(hp);
    cc->hnum = hn;

    cclist = 0;
    compctl_process_cc(s, cc);
}

/**/
void
compctl_process_cc(char **s, Compctl cc)
{
    Compctlp ccp;

    if (cclist & COMP_REMOVE) {
	/* Delete entries for the commands listed */
	for (; *s; s++) {
	    if ((ccp = (Compctlp) compctltab->removenode(compctltab, *s)))
		compctltab->freenode((HashNode) ccp);
	}
    } else {
	/* Add the compctl just read to the hash table */

	cc->refc = 0;
	for (; *s; s++) {
	    cc->refc++;
	    ccp = (Compctlp) zalloc(sizeof *ccp);
	    ccp->cc = cc;
	    compctltab->addnode(compctltab, ztrdup(*s), ccp);
	}
    }
}

/* Print a `compctl' */

/**/
void
printcompctl(char *s, Compctl cc, int printflags)
{
    Compctl cc2;
    char *css = "fcqovbAIFpEjrzBRGudeNOZUnQmw";
    char *mss = " pcCwWsSnNmrR";
    unsigned long t = 0x7fffffff;
    unsigned long flags = cc->mask;
    unsigned long oldshowmask;

    if ((flags & CC_EXCMDS) && !(flags & CC_DISCMDS))
	flags &= ~CC_EXCMDS;

    /* If showmask is non-zero, then print only those *
     * commands with that flag set.                   */
    if (showmask && !(flags & showmask))
	return;

    /* Temporarily clear showmask in case we make *
     * recursive calls to printcompctl.           */
    oldshowmask = showmask;
    showmask = 0;

    /* print either command name or start of compctl command itself */
    if (s) {
	if (cclist & COMP_LIST) {
	    printf("compctl");
	    if (cc == &cc_compos)
		printf(" -C");
	    if (cc == &cc_default)
		printf(" -D");
	    if (cc == &cc_first)
		printf(" -T");
	} else
	    quotedzputs(s, stdout);
    }

    /* loop through flags w/o args that are set, printing them if so */
    if (flags & t) {
	printf(" -");
	if ((flags & (CC_ALREG | CC_ALGLOB)) == (CC_ALREG | CC_ALGLOB))
	    putchar('a'), flags &= ~(CC_ALREG | CC_ALGLOB);
	while (*css) {
	    if (flags & t & 1)
		putchar(*css);
	    css++;
	    flags >>= 1;
	    t >>= 1;
	}
    }

    /* now flags with arguments */
    flags = cc->mask;
    printif(cc->keyvar, 'k');
    printif(cc->func, 'K');
    printif(cc->explain, 'X');
    printif(cc->prefix, 'P');
    printif(cc->suffix, 'S');
    printif(cc->glob, 'g');
    printif(cc->str, 's');
    printif(cc->subcmd, 'l');
    if (cc->hpat) {
	printf(" -H %d ", cc->hnum);
	quotedzputs(cc->hpat, stdout);
    }

    /* now the -x ... -- extended completion part */
    if (cc->ext) {
	Compcond c, o;
	int i;

	cc2 = cc->ext;
	printf(" -x");

	while (cc2) {
	    /* loop over conditions */
	    c = cc2->cond;

	    printf(" '");
	    for (c = cc2->cond; c;) {
		/* loop over or's */
		o = c->or;
		while (c) {
		    /* loop over and's */
		    putchar(mss[c->type]);

		    for (i = 0; i < c->n; i++) {
			/* for all [...]'s of a given condition */
			putchar('[');
			switch (c->type) {
			case CCT_POS:
			case CCT_NUMWORDS:
			    printf("%d,%d", c->u.r.a[i], c->u.r.b[i]);
			    break;
			case CCT_CURSUF:
			case CCT_CURPRE:
			    printqt(c->u.s.s[i]);
			    break;
			case CCT_RANGESTR:
			case CCT_RANGEPAT:
			    printqt(c->u.l.a[i]);
			    putchar(',');
			    printqt(c->u.l.b[i]);
			    break;
			default:
			    printf("%d,", c->u.s.p[i]);
			    printqt(c->u.s.s[i]);
			}
			putchar(']');
		    }
		    if ((c = c->and))
			putchar(' ');
		}
		if ((c = o))
		    printf(" , ");
	    }
	    putchar('\'');
	    c = cc2->cond;
	    cc2->cond = NULL;
	    /* now print the flags for the current condition */
	    printcompctl(NULL, cc2, 0);
	    cc2->cond = c;
	    if ((cc2 = (Compctl) (cc2->next)))
		printf(" -");
	}
	if (cclist & COMP_LIST)
	    printf(" --");
    }
    if (cc && cc->xor) {
	/* print xor'd (+) completions */
	printf(" +");
	if (cc->xor != &cc_default)
	    printcompctl(NULL, cc->xor, 0);
    }
    if (s) {
	if ((cclist & COMP_LIST) && (cc != &cc_compos)
	    && (cc != &cc_default) && (cc != &cc_first)) {
	    if(s[0] == '-' || s[0] == '+')
		printf(" -");
	    putchar(' ');
	    quotedzputs(s, stdout);
	}
	putchar('\n');
    }

    showmask = oldshowmask;
}

/**/
void
printcompctlp(HashNode hn, int printflags)
{
    Compctlp ccp = (Compctlp) hn;

    /* Function needed for use by scanhashtable() */
    printcompctl(ccp->nam, ccp->cc, printflags);
}

/**/
void
printqt(char *str)
{
    /* Print str, but turn any single quote into '\'' or ''. */
    for (; *str; str++)
	if (*str == '\'')
	    printf(isset(RCQUOTES) ? "''" : "'\\''");
	else
	    putchar(*str);
}

/**/
void
printif(char *str, int c)
{
    /* If flag c has an argument, print that */
    if (str) {
	printf(" -%c ", c);
	quotedzputs(str, stdout);
    }
}

/**** history list functions ****/

/* fc, history, r */

/**/
int
bin_fc(char *nam, char **argv, char *ops, int func)
{
    int first = -1, last = -1, retval, delayrem, minflag = 0;
    char *s;
    struct asgment *asgf = NULL, *asgl = NULL;
    Comp com = NULL;

    /* fc is only permitted in interactive shells */
    if (!interact) {
	zwarnnam(nam, "not interactive shell", NULL, 0);
	return 1;
    }
    /* with the -m option, the first argument is taken *
     * as a pattern that history lines have to match   */
    if (*argv && ops['m']) {
	tokenize(*argv);
	if (!(com = parsereg(*argv++))) {
	    zwarnnam(nam, "invalid match pattern", NULL, 0);
	    return 1;
	}
    }
    delayrem = 0;
    if (!(ops['l'] && unset(HISTNOSTORE)) &&
	!(ops['R'] || ops['W'] || ops['A']))
	delayrem = 1;
    if (ops['R']) {
	/* read history from a file */
	readhistfile(*argv ? *argv : getsparam("HISTFILE"), 1);
	return 0;
    }
    if (ops['W']) {
	/* write history to a file */
	savehistfile(*argv ? *argv : getsparam("HISTFILE"), 1,
		     (ops['I'] ? 2 : 0));
	return 0;
    }
    if (ops['A']) {
	/* append history to a file */
	savehistfile(*argv ? *argv : getsparam("HISTFILE"), 1,
		     (ops['I'] ? 3 : 1));
	return 0;
    }
    /* put foo=bar type arguments into the substitution list */
    while (*argv && equalsplit(*argv, &s)) {
	Asgment a = (Asgment) alloc(sizeof *a);

	if (!asgf)
	    asgf = asgl = a;
	else {
	    asgl->next = a;
	    asgl = a;
	}
	a->name = *argv;
	a->value = s;
	argv++;
    }
    /* interpret and check first history line specifier */
    if (*argv) {
	minflag = **argv == '-';
	first = fcgetcomm(*argv);
	if (first == -1) {
	    if (delayrem)
		remhist();
	    return 1;
	}
	argv++;
    }
    /* interpret and check second history line specifier */
    if (*argv) {
	last = fcgetcomm(*argv);
	if (last == -1) {
	    if (delayrem)
		remhist();
	    return 1;
	}
	argv++;
    }
    /* There is a maximum of two history specifiers.  At least, there *
     * will be as long as the history list is one-dimensional.        */
    if (*argv) {
	zwarnnam("fc", "too many arguments", NULL, 0);
	if (delayrem)
	    remhist();
	return 1;
    }
    /* default values of first and last, and range checking */
    if (first == -1)
	first = (ops['l']) ? curhist - 16 : curhist - 1;
    if (last == -1)
	last = (ops['l']) ? curhist - 1 : first;
    if (first < firsthist())
	first = firsthist();
    if (last == -1)
	last = (minflag) ? curhist : first;
    if (ops['l'])
	/* list the required part of the history */
	retval = fclist(stdout, !ops['n'], ops['r'], ops['D'],
			ops['d'] + ops['f'] * 2 + ops['E'] * 4 + ops['i'] * 8,
			first, last, asgf, com);
    else {
	/* edit history file, and (if successful) use the result as a new command */
	int tempfd;
	FILE *out;
	char *fil;

	retval = 1;
	fil = gettempname();
	if (((tempfd = open(fil, O_WRONLY | O_CREAT | O_EXCL, 0600)) == -1) ||
		((out = fdopen(tempfd, "w")) == NULL)) {
	    zwarnnam("fc", "can't open temp file: %e", NULL, errno);
	} else {
	    if (!fclist(out, 0, ops['r'], 0, 0, first, last, asgf, com)) {
		char *editor;

		editor = auxdata ? auxdata : getsparam("FCEDIT");
		if (!editor)
		    editor = DEFAULT_FCEDIT;

		if (fcedit(editor, fil)) {
		    if (stuff(fil))
			zwarnnam("fc", "%e: %s", s, errno);
		    else {
			if (delayrem)
			    remhist();
			delayrem = 0;
			loop(0,1);
			retval = lastval;
		    }
		}
	    }
	}
	unlink(fil);
    }
    if (delayrem)
	remhist();
    return retval;
}

/* History handling functions: these are called by ZLE, as well as  *
 * the actual builtins.  fcgetcomm() gets a history line, specified *
 * either by number or leading string.  fcsubs() performs a given   *
 * set of simple old=new substitutions on a given command line.     *
 * fclist() outputs a given range of history lines to a text file.  */

/* get the history event associated with s */

/**/
int
fcgetcomm(char *s)
{
    int cmd;

    /* First try to match a history number.  Negative *
     * numbers indicate reversed numbering.           */
    if ((cmd = atoi(s))) {
	if (cmd < 0)
	    cmd = curhist + cmd;
	if (cmd >= curhist) {
	    zwarnnam("fc", "bad history number: %d", 0, cmd);
	    return -1;
	}
	return cmd;
    }
    /* not a number, so search by string */
    cmd = hcomsearch(s);
    if (cmd == -1)
	zwarnnam("fc", "event not found: %s", s, 0);
    return cmd;
}

/* Perform old=new substituions.  Uses the asgment structure from zsh.h, *
 * which is essentially a linked list of string,replacement pairs.       */

/**/
int
fcsubs(char **sp, struct asgment *sub)
{
    char *oldstr, *newstr, *oldpos, *newpos, *newmem, *s = *sp;
    int subbed = 0;

    /* loop through the linked list */
    while (sub) {
	oldstr = sub->name;
	newstr = sub->value;
	sub = sub->next;
	oldpos = s;
	/* loop over occurences of oldstr in s, replacing them with newstr */
	while ((newpos = (char *)strstr(oldpos, oldstr))) {
	    newmem = (char *) alloc(1 + (newpos - s)
			+ strlen(newstr) + strlen(newpos + strlen(oldstr)));
	    ztrncpy(newmem, s, newpos - s);
	    strcat(newmem, newstr);
	    oldpos = newmem + strlen(newmem);
	    strcat(newmem, newpos + strlen(oldstr));
	    s = newmem;
	    subbed = 1;
	}
    }
    *sp = s;
    return subbed;
}

/* Print a series of history events to a file.  The file pointer is     *
 * given by f, and the required range of events by first and last.      *
 * subs is an optional list of foo=bar substitutions to perform on the  *
 * history lines before output.  com is an optional comp structure      *
 * that the history lines are required to match.  n, r, D and d are     *
 * options: n indicates that each line should be numbered.  r indicates *
 * that the lines should be output in reverse order (newest first).     *
 * D indicates that the real time taken by each command should be       *
 * output.  d indicates that the time of execution of each command      *
 * should be output; d>1 means that the date should be output too; d>3  *
 * means that mm/dd/yyyy form should be used for the dates, as opposed  *
 * to dd.mm.yyyy form; d>7 means that yyyy-mm-dd form should be used.   */

/**/
int
fclist(FILE *f, int n, int r, int D, int d, int first, int last, struct asgment *subs, Comp com)
{
    int fclistdone = 0;
    char *s, *hs;
    Histent ent;

    /* reverse range if required */
    if (r) {
	r = last;
	last = first;
	first = r;
    }
    /* suppress "no substitution" warning if no substitution is requested */
    if (!subs)
	fclistdone = 1;

    for (;;) {
	hs = quietgetevent(first);
	if (!hs) {
	    zwarnnam("fc", "no such event: %d", NULL, first);
	    return 1;
	}
	s = dupstring(hs);
	/* this if does the pattern matching, if required */
	if (!com || domatch(s, com, 0)) {
	    /* perform substitution */
	    fclistdone |= fcsubs(&s, subs);

	    /* do numbering */
	    if (n)
		fprintf(f, "%5d  ", first);
	    ent = NULL;
	    /* output actual time (and possibly date) of execution of the
	    command, if required */
	    if (d) {
		struct tm *ltm;
		if (!ent)
		    ent = gethistent(first);
		ltm = localtime(&ent->stim);
		if (d >= 2) {
		    if (d >= 8) {
			fprintf(f, "%d-%02d-%02d ",
				ltm->tm_year + 1900,
				ltm->tm_mon + 1, ltm->tm_mday);
		    } else if (d >= 4) {
			fprintf(f, "%d.%d.%d ",
				ltm->tm_mday, ltm->tm_mon + 1,
				ltm->tm_year + 1900);
		    } else {
			fprintf(f, "%d/%d/%d ",
				ltm->tm_mon + 1, ltm->tm_mday,
				ltm->tm_year + 1900);
		    }
		}
		fprintf(f, "%02d:%02d  ", ltm->tm_hour, ltm->tm_min);
	    }
	    /* display the time taken by the command, if required */
	    if (D) {
		long diff;
		if (!ent)
		    ent = gethistent(first);
		diff = (ent->ftim) ? ent->ftim - ent->stim : 0;
		fprintf(f, "%ld:%02ld  ", diff / 60, diff % 60);
	    }

	    /* output the command */
	    if (f == stdout) {
		nicezputs(s, f);
		putc('\n', f);
	    } else
		fprintf(f, "%s\n", s);
	}
	/* move on to the next history line, or quit the loop */
	if (first == last)
	    break;
	else if (first > last)
	    first--;
	else
	    first++;
    }

    /* final processing */
    if (f != stdout)
	fclose(f);
    if (!fclistdone) {
	zwarnnam("fc", "no substitutions performed", NULL, 0);
	return 1;
    }
    return 0;
}

/* edit a history file */

/**/
int
fcedit(char *ename, char *fn)
{
    char *s;

    if (!strcmp(ename, "-"))
	return 1;

    s = tricat(ename, " ", fn);
    execstring(s, 1, 0);
    zsfree(s);

    return !lastval;
}

/**** parameter builtins ****/

/* function to set a single parameter */

/**/
int
typeset_single(char *cname, char *pname, Param pm, int func, int on, int off, int roff, char *value)
{
    int usepm, tc;

    on &= ~PM_ARRAY;

    /* use the existing pm? */
    usepm = pm && !(pm->flags & PM_UNSET);

    /* Always use an existing pm if special at current locallevel */
    if (pm && (pm->flags & PM_SPECIAL) && pm->level == locallevel)
	usepm = 1;

    /*
     * Don't use a non-special existing param if
     *   - the local level has changed, and
     *   - the function is not `export'.
     */
    if (usepm && !(pm->flags & PM_SPECIAL) &&
	locallevel != pm->level && func != BIN_EXPORT)
	usepm = 0;

    /* attempting a type conversion? */
    if ((tc = usepm && (((off & pm->flags) | (on & ~pm->flags)) & PM_INTEGER)
	 && !(pm->flags & PM_SPECIAL)))
	usepm = 0;

    if (usepm) {
	if (pm->flags & PM_SPECIAL) {
	    func = 0;
	    on = (PM_TYPE(pm->flags) == PM_INTEGER) ?
		(on & ~(PM_LEFT | PM_RIGHT_B | PM_RIGHT_Z | PM_UPPER)) :
		(on & ~PM_INTEGER);
	    off &= ~PM_INTEGER;
	}
	if (pm->level) {
	    if ((on & PM_EXPORTED) && !(on &= ~PM_EXPORTED) && !off)
		return 1;
	}
	if (!on && !roff && !value) {
	    paramtab->printnode((HashNode)pm, 0);
	    return 0;
	}
	if ((on & PM_UNIQUE) && !(pm->flags & PM_READONLY & ~off)) {
	    Param apm;
	    char **x;
	    if (PM_TYPE(pm->flags) == PM_ARRAY) {
		x = (*pm->gets.afn)(pm);
		uniqarray(x);
		if (pm->ename && x)
		    arrfixenv(pm->ename, x);
	    } else if (PM_TYPE(pm->flags) == PM_SCALAR && pm->ename &&
		       (apm =
			(Param) paramtab->getnode(paramtab, pm->ename))) {
		x = (*apm->gets.afn)(apm);
		uniqarray(x);
		if (x)
		    arrfixenv(pm->nam, x);
	    }
	}
	pm->flags = (pm->flags | on) & ~(off | PM_UNSET);
	/* This auxlen/pm->ct stuff is a nasty hack. */
	if ((on & (PM_LEFT | PM_RIGHT_B | PM_RIGHT_Z | PM_INTEGER)) &&
	    auxlen)
	    pm->ct = auxlen;
	if (PM_TYPE(pm->flags) != PM_ARRAY) {
	    if (pm->flags & PM_EXPORTED) {
		if (!(pm->flags & PM_UNSET) && !pm->env)
		    pm->env = addenv(pname, value ? value : getsparam(pname));
	    } else if (pm->env) {
		delenv(pm->env);
		zsfree(pm->env);
		pm->env = NULL;
	    }
	    if (value)
		setsparam(pname, ztrdup(value));
	}
	return 0;
    }

    /*
     * We're here either because we're creating a new parameter,
     * or we're adding a parameter at a different local level,
     * or we're converting the type of a parameter.  In the
     * last case only, we need to delete the old parameter.
     */
    if (tc) {
	if (pm->flags & PM_READONLY) {
	    on |= ~off & PM_READONLY;
	    pm->flags &= ~PM_READONLY;
	}
	if (!value)
	    value = dupstring(getsparam(pname));
	/* pname may point to pm->nam which is about to disappear */
	pname = dupstring(pname);
	unsetparam_pm(pm, 0);
    } else if (locallist && func != BIN_EXPORT) {
	PERMALLOC {
	    addlinknode(locallist, ztrdup(pname));
	} LASTALLOC;
    }
    /*
     * Create a new node for a parameter with the flags in `on' minus the
     * readonly flag
     */
    pm = createparam(ztrdup(pname), on & ~PM_READONLY);
    DPUTS(!pm, "BUG: parameter not created");
    pm->ct = auxlen;
    if (func != BIN_EXPORT)
	pm->level = locallevel;
    if (value)
	setsparam(pname, ztrdup(value));
    pm->flags |= (on & PM_READONLY);

    return 0;
}

/* declare, export, integer, local, readonly, typeset */

/**/
int
bin_typeset(char *name, char **argv, char *ops, int func)
{
    Param pm;
    Asgment asg;
    Comp com;
    char *optstr = "iLRZlurtxU";
    int on = 0, off = 0, roff, bit = PM_INTEGER;
    int i;
    int returnval = 0, printflags = 0;

    /* hash -f is really the builtin `functions' */
    if (ops['f'])
	return bin_functions(name, argv, ops, func);

    /* Translate the options into PM_* flags.   *
     * Unfortunately, this depends on the order *
     * these flags are defined in zsh.h         */
    for (; *optstr; optstr++, bit <<= 1) {
	if (ops[*(unsigned char *)optstr] == 1)
	    on |= bit;
	else if (ops[*(unsigned char *)optstr] == 2)
	    off |= bit;
    }
    roff = off;

    /* Sanity checks on the options.  Remove conficting options. */
    if ((on | off) & PM_EXPORTED)
	func = BIN_EXPORT;
    if (on & PM_INTEGER)
	off |= PM_RIGHT_B | PM_LEFT | PM_RIGHT_Z | PM_UPPER | PM_ARRAY;
    if (on & PM_LEFT)
	off |= PM_RIGHT_B | PM_INTEGER;
    if (on & PM_RIGHT_B)
	off |= PM_LEFT | PM_INTEGER;
    if (on & PM_RIGHT_Z)
	off |= PM_INTEGER;
    if (on & PM_UPPER)
	off |= PM_LOWER;
    if (on & PM_LOWER)
	off |= PM_UPPER;
    on &= ~off;

    /* Given no arguments, list whatever the options specify. */
    if (!*argv) {
	if (!(on|roff))
	    printflags |= PRINT_TYPE;
	if (roff || ops['+'])
	    printflags |= PRINT_NAMEONLY;
	scanhashtable(paramtab, 1, on|roff, 0, paramtab->printnode, printflags);
	return 0;
    }

    /* With the -m option, treat arguments as glob patterns */
    if (ops['m']) {
	MUSTUSEHEAP("typeset -m");
	while ((asg = getasg(*argv++))) {
	    LinkList pmlist = newlinklist();
	    LinkNode pmnode;

	    tokenize(asg->name);   /* expand argument */
	    if (!(com = parsereg(asg->name))) {
		untokenize(asg->name);
		zwarnnam(name, "bad pattern : %s", argv[-1], 0);
		returnval = 1;
		continue;
	    }
	    /*
	     * Search through the parameter table and change all parameters
	     * matching the glob pattern to have these flags and/or value.
	     * Bad news:  if the parameter gets altered, e.g. by
	     * a type conversion, then paramtab can be shifted around,
	     * so we need to store the parameters to alter on a separate
	     * list for later use.	     
	     */
	    for (i = 0; i < paramtab->hsize; i++) {
		for (pm = (Param) paramtab->nodes[i]; pm;
		     pm = (Param) pm->next) {
		    if (pm->flags & PM_UNSET)
			continue;
		    if (domatch(pm->nam, com, 0))
			addlinknode(pmlist, pm);
		}
	    }
	    for (pmnode = firstnode(pmlist); pmnode; incnode(pmnode)) {
		pm = (Param) getdata(pmnode);
		if (typeset_single(name, pm->nam, pm, func, on, off, roff,
				   asg->value))
		    returnval = 1;
	    }
	}
	return returnval;
    }

    /* Take arguments literally.  Don't glob */
    while ((asg = getasg(*argv++))) {
	/* check if argument is a valid identifier */
	if (!isident(asg->name)) {
	    zerr("not an identifier: %s", asg->name, 0);
	    returnval = 1;
	    continue;
	}
	typeset_single(name, asg->name,
		       (Param) paramtab->getnode(paramtab, asg->name),
		       func, on, off, roff, asg->value);
    }
    return returnval;
}

/* Display or change the attributes of shell functions.   *
 * If called as autoload, it will define a new autoloaded *
 * (undefined) shell function.                            */

/**/
int
bin_functions(char *name, char **argv, char *ops, int func)
{
    Comp com;
    Shfunc shf;
    int i, returnval = 0;
    int on = 0, off = 0, pflags = 0;

    /* Do we have any flags defined? */
    if (ops['u'] || ops['t']) {
	if (ops['u'] == 1)
	    on |= PM_UNDEFINED;
	else if (ops['u'] == 2)
	    off |= PM_UNDEFINED;

	if (ops['t'] == 1)
	    on |= PM_TAGGED;
	else if (ops['t'] == 2)
	    off |= PM_TAGGED;
    }

    if (off & PM_UNDEFINED) {
	zwarnnam(name, "invalid option(s)", NULL, 0);
	return 1;
    }

    if (ops['f'] == 2 || ops['+'])
	pflags |= PRINT_NAMEONLY;

    /* If no arguments given, we will print functions.  If flags *
     * are given, we will print only functions containing these  *
     * flags, else we'll print them all.                         */
    if (!*argv) {
	scanhashtable(shfunctab, 1, on|off, DISABLED, shfunctab->printnode,
		      pflags);
	return 0;
    }

    /* With the -m option, treat arguments as glob patterns */
    if (ops['m']) {
	on &= ~PM_UNDEFINED;
	for (; *argv; argv++) {
	    /* expand argument */
	    tokenize(*argv);
	    if ((com = parsereg(*argv))) {
		/* with no options, just print all functions matching the glob pattern */
		if (!(on|off)) {
		    scanmatchtable(shfunctab, com, 0, DISABLED,
				   shfunctab->printnode, pflags);
		} else {
		/* apply the options to all functions matching the glob pattern */
		    for (i = 0; i < shfunctab->hsize; i++) {
			for (shf = (Shfunc) shfunctab->nodes[i]; shf; shf = (Shfunc) shf->next)
			    if (domatch(shf->nam, com, 0) && !(shf->flags & DISABLED))
				shf->flags = (shf->flags | on) & (~off);
		    }
		}
	    } else {
		untokenize(*argv);
		zwarnnam(name, "bad pattern : %s", *argv, 0);
		returnval = 1;
	    }
	}
	return returnval;
    }

    /* Take the arguments literally -- do not glob */
    for (; *argv; argv++) {
	if ((shf = (Shfunc) shfunctab->getnode(shfunctab, *argv))) {
	    /* if any flag was given */
	    if (on|off)
		/* turn on/off the given flags */
		shf->flags = (shf->flags | (on & ~PM_UNDEFINED)) & ~off;
	    else
		/* no flags, so just print */
		shfunctab->printnode((HashNode) shf, pflags);
	} else if (on & PM_UNDEFINED) {
	    /* Add a new undefined (autoloaded) function to the *
	     * hash table with the corresponding flags set.     */
	    shf = (Shfunc) zcalloc(sizeof *shf);
	    shf->flags = on;
	    shfunctab->addnode(shfunctab, ztrdup(*argv), shf);
	} else
	    returnval = 1;
    }
    return returnval;
}

/* unset: unset parameters */

/**/
int
bin_unset(char *name, char **argv, char *ops, int func)
{
    Param pm, next;
    Comp com;
    char *s;
    int match = 0, returnval = 0;
    int i;

    /* unset -f is the same as unfunction */
    if (ops['f'])
	return bin_unhash(name, argv, ops, func);

    /* with -m option, treat arguments as glob patterns */
    if (ops['m']) {
	while ((s = *argv++)) {
	    /* expand */
	    tokenize(s);
	    if ((com = parsereg(s))) {
		/* Go through the parameter table, and unset any matches */
		for (i = 0; i < paramtab->hsize; i++) {
		    for (pm = (Param) paramtab->nodes[i]; pm; pm = next) {
			/* record pointer to next, since we may free this one */
			next = (Param) pm->next;
			if (domatch(pm->nam, com, 0)) {
			    unsetparam(pm->nam);
			    match++;
			}
		    }
		}
	    } else {
		untokenize(s);
		zwarnnam(name, "bad pattern : %s", s, 0);
		returnval = 1;
	    }
	}
	/* If we didn't match anything, we return 1. */
	if (!match)
	    returnval = 1;
	return returnval;
    }

    /* do not glob -- unset the given parameter */
    while ((s = *argv++)) {
	if (paramtab->getnode(paramtab, s)) {
	    unsetparam(s);
	} else {
	    returnval = 1;
	}
    }
    return returnval;
}

/* vared: edit (literally) a parameter value */

/**/
int
bin_vared(char *name, char **args, char *ops, int func)
{
    char *s;
    char *t;
    Param pm;
    int create = 0, obreaks = breaks, haso = 0;
    char *p1 = NULL, *p2 = NULL;
    FILE *oshout = shout;

    /* all options are handled as arguments */
    while (*args && **args == '-') {
	while (*++(*args))
	    switch (**args) {
	    case 'c':
		/* -c option -- allow creation of the parameter if it doesn't
		yet exist */
		create = 1;
		break;
	    case 'p':
		/* -p option -- set main prompt string */
		if ((*args)[1])
		    p1 = *args + 1, *args = "" - 1;
		else if (args[1])
		    p1 = *(++args), *args = "" - 1;
		else {
		    zwarnnam(name, "prompt string expected after -%c", NULL,
			     **args);
		    return 1;
		}
		break;
	    case 'r':
		/* -r option -- set right prompt string */
		if ((*args)[1])
		    p2 = *args + 1, *args = "" - 1;
		else if (args[1])
		    p2 = *(++args), *args = "" - 1;
		else {
		    zwarnnam(name, "prompt string expected after -%c", NULL,
			     **args);
		    return 1;
		}
		break;
	    case 'h':
		/* -h option -- enable history */
		ops['h'] = 1;
		break;
	    default:
		/* unrecognised option character */
		zwarnnam(name, "unknown option: %s", *args, 0);
		return 1;
	    }
	args++;
    }

    /* check we have a parameter name */
    if (!*args) {
	zwarnnam(name, "missing variable", NULL, 0);
	return 1;
    }
    /* handle non-existent parameter */
    if (!(s = getsparam(args[0]))) {
	if (create)
	    createparam(args[0], PM_SCALAR);
	else {
	    zwarnnam(name, "no such variable: %s", args[0], 0);
	    return 1;
	}
    }

    if (SHTTY == -1) {
	/* need to open /dev/tty specially */
	if ((SHTTY = open("/dev/tty", O_RDWR|O_NOCTTY)) == -1) {
	    zerrnam(name, "can't access terminal", NULL, 0);
	    return 1;
	}
	init_shout();

	haso = 1;
    }

    /* edit the parameter value */
    PERMALLOC {
	pushnode(bufstack, ztrdup(s));
    } LASTALLOC;
    in_vared = !ops['h'];
    t = (char *) zleread(p1, p2);
    in_vared = 0;
    if (haso) {
	close(SHTTY);
	fclose(shout);
	shout = oshout;
	SHTTY = -1;
    }
    if (!t || errflag) {
	/* error in editing */
	errflag = 0;
	breaks = obreaks;
	return 1;
    }
    /* strip off trailing newline, if any */
    if (t[strlen(t) - 1] == '\n')
	t[strlen(t) - 1] = '\0';
    /* final assignment of parameter value */
    pm = (Param) paramtab->getnode(paramtab, args[0]);
    if (pm && PM_TYPE(pm->flags) == PM_ARRAY) {
	char **a;

	PERMALLOC {
	    a = spacesplit(t, 1);
	} LASTALLOC;
	setaparam(args[0], a);
    } else
	setsparam(args[0], t);
    return 0;
}

/* type, whence, which */

/**/
int
bin_whence(char *nam, char **argv, char *ops, int func)
{
    HashNode hn;
    Comp com;
    int returnval = 0;
    int printflags = 0;
    int csh, all, v;
    int informed;
    char *cnam;

    /* Check some option information */
    csh = ops['c'];
    v   = ops['v'];
    all = ops['a'];

    if (ops['c'])
	printflags |= PRINT_WHENCE_CSH;
    else if (ops['v'])
	printflags |= PRINT_WHENCE_VERBOSE;
    else
	printflags |= PRINT_WHENCE_SIMPLE;
    if(ops['f'])
	printflags |= PRINT_WHENCE_FUNCDEF;

    /* With -m option -- treat arguments as a glob patterns */
    if (ops['m']) {
	for (; *argv; argv++) {
	    /* parse the pattern */
	    tokenize(*argv);
	    if (!(com = parsereg(*argv))) {
		untokenize(*argv);
		zwarnnam(nam, "bad pattern : %s", *argv, 0);
		returnval = 1;
		continue;
	    }
	    if (!ops['p']) {
		/* -p option is for path search only.    *
		 * We're not using it, so search for ... */

		/* aliases ... */
		scanmatchtable(aliastab, com, 0, DISABLED, aliastab->printnode, printflags);

		/* and reserved words ... */
		scanmatchtable(reswdtab, com, 0, DISABLED, reswdtab->printnode, printflags);

		/* and shell functions... */
		scanmatchtable(shfunctab, com, 0, DISABLED, shfunctab->printnode, printflags);

		/* and builtins. */
		scanmatchtable(builtintab, com, 0, DISABLED, builtintab->printnode, printflags);
	    }
	    /* Done search for `internal' commands, if the -p option *
	     * was not used.  Now search the path.                   */
	    cmdnamtab->filltable(cmdnamtab);
	    scanmatchtable(cmdnamtab, com, 0, 0, cmdnamtab->printnode, printflags);

	}
    return returnval;
    }

    /* Take arguments literally -- do not glob */
    for (; *argv; argv++) {
	informed = 0;

	if (!ops['p']) {
	    /* Look for alias */
	    if ((hn = aliastab->getnode(aliastab, *argv))) {
		aliastab->printnode(hn, printflags);
		if (!all)
		    continue;
		informed = 1;
	    }
	    /* Look for reserved word */
	    if ((hn = reswdtab->getnode(reswdtab, *argv))) {
		reswdtab->printnode(hn, printflags);
		if (!all)
		    continue;
		informed = 1;
	    }
	    /* Look for shell function */
	    if ((hn = shfunctab->getnode(shfunctab, *argv))) {
		shfunctab->printnode(hn, printflags);
		if (!all)
		    continue;
		informed = 1;
	    }
	    /* Look for builtin command */
	    if ((hn = builtintab->getnode(builtintab, *argv))) {
		builtintab->printnode(hn, printflags);
		if (!all)
		    continue;
		informed = 1;
	    }
	    /* Look for commands that have been added to the *
	     * cmdnamtab with the builtin `hash foo=bar'.    */
	    if ((hn = cmdnamtab->getnode(cmdnamtab, *argv)) && (hn->flags & HASHED)) {
		cmdnamtab->printnode(hn, printflags);
		if (!all)
		    continue;
		informed = 1;
	    }
	}

	/* Option -a is to search the entire path, *
	 * rather than just looking for one match. */
	if (all) {
	    char **pp, buf[PATH_MAX], *z;

	    for (pp = path; *pp; pp++) {
		z = buf;
		if (**pp) {
		    strucpy(&z, *pp);
		    *z++ = '/';
		}
		if ((z - buf) + strlen(*argv) >= PATH_MAX)
		    continue;
		strcpy(z, *argv);
		if (iscom(buf)) {
		    if (v && !csh)
			printf("%s is %s\n", *argv, buf);
		    else
			puts(buf);
		    informed = 1;
		}
	    }
	    if (!informed && (v || csh)) {
		printf("%s not found\n", *argv);
		returnval = 1;
	    }
	} else if ((cnam = findcmd(*argv))) {
	    /* Found external command. */
	    if (v && !csh)
		printf("%s is %s\n", *argv, cnam);
	    else
		puts(cnam);
	    zsfree(cnam);
	} else {
	    /* Not found at all. */
	    if (v || csh)
		printf("%s not found\n", *argv);
	    returnval = 1;
	}
    }
    return returnval;
}

/**** command & named directory hash table builtins ****/

/*****************************************************************
 * hash -- explicitly hash a command.                            *
 * 1) Given no arguments, list the hash table.                   *
 * 2) The -m option prints out commands in the hash table that   *
 *    match a given glob pattern.                                *
 * 3) The -f option causes the entire path to be added to the    *
 *    hash table (cannot be combined with any arguments).        *
 * 4) The -r option causes the entire hash table to be discarded *
 *    (cannot be combined with any arguments).                   *
 * 5) Given argument of the form foo=bar, add element to command *
 *    hash table, so that when `foo' is entered, then `bar' is   *
 *    executed.                                                  *
 * 6) Given arguments not of the previous form, add it to the    *
 *    command hash table as if it were being executed.           *
 * 7) The -d option causes analogous things to be done using     *
 *    the named directory hash table.                            *
 *****************************************************************/

/**/
int
bin_hash(char *name, char **argv, char *ops, int func)
{
    HashTable ht;
    Comp com;
    Asgment asg;
    int returnval = 0;

    if (ops['d'])
	ht = nameddirtab;
    else
	ht = cmdnamtab;

    if (ops['r'] || ops['f']) {
	/* -f and -r can't be used with any arguments */
	if (*argv) {
	    zwarnnam("hash", "too many arguments", NULL, 0);
	    return 1;
	}

	/* empty the hash table */
	if (ops['r'])
	    ht->emptytable(ht);

	/* fill the hash table in a standard way */
	if (ops['f'])
	    ht->filltable(ht);

	return 0;
    }

    /* Given no arguments, display current hash table. */
    if (!*argv) {
	scanhashtable(ht, 1, 0, 0, ht->printnode, 0);
	return 0;
    }

    while ((asg = getasg(*argv))) {
	if (asg->value) {
	    /* The argument is of the form foo=bar, *
	     * so define an entry for the table.    */
	    if(ops['d']) {
		Nameddir nd = (Nameddir) zcalloc(sizeof *nd);
		nd->flags = 0;
		nd->dir = ztrdup(asg->value);
		ht->addnode(ht, ztrdup(asg->name), nd);
	    } else {
		Cmdnam cn = (Cmdnam) zcalloc(sizeof *cn);
		cn->flags = HASHED;
		cn->u.cmd = ztrdup(asg->value);
		ht->addnode(ht, ztrdup(asg->name), cn);
	    }
	} else if (ops['m']) {
	    /* with the -m option, treat the argument as a glob pattern */
	    tokenize(*argv);  /* expand */
	    if ((com = parsereg(*argv))) {
		/* display matching hash table elements */
		scanmatchtable(ht, com, 0, 0, ht->printnode, 0);
	    } else {
		untokenize(*argv);
		zwarnnam(name, "bad pattern : %s", *argv, 0);
		returnval = 1;
	    }
	} else if (!ht->getnode2(ht, asg->name)) {
	    /* With no `=value' part to the argument, *
	     * work out what it ought to be.          */
	    if(ops['d'] && !getnameddir(asg->name)) {
		zwarnnam(name, "no such directory name: %s", asg->name, 0);
		returnval = 1;
	    } else if (!hashcmd(asg->name, path)) {
		zwarnnam(name, "no such command: %s", asg->name, 0);
		returnval = 1;
	    }
	}
	argv++;
    }
    return returnval;
}

/* unhash: remove specified elements from a hash table */

/**/
int
bin_unhash(char *name, char **argv, char *ops, int func)
{
    HashTable ht;
    HashNode hn, nhn;
    Comp com;
    int match = 0, returnval = 0;
    int i;

    /* Check which hash table we are working with. */
    if (ops['d'])
	ht = nameddirtab;	/* named directories */
    else if (ops['f'])
	ht = shfunctab;		/* shell functions   */
    else if (ops['a'])
	ht = aliastab;		/* aliases           */
    else
	ht = cmdnamtab;		/* external commands */

    /* With -m option, treat arguments as glob patterns. *
     * "unhash -m '*'" is legal, but not recommended.    */
    if (ops['m']) {
	for (; *argv; argv++) {
	    /* expand argument */
	    tokenize(*argv);
	    if ((com = parsereg(*argv))) {
		/* remove all nodes matching glob pattern */
		for (i = 0; i < ht->hsize; i++) {
		    for (hn = ht->nodes[i]; hn; hn = nhn) {
			/* record pointer to next, since we may free this one */
			nhn = hn->next;
			if (domatch(hn->nam, com, 0)) {
			    ht->freenode(ht->removenode(ht, hn->nam));
			    match++;
			}
		    }
		}
	    } else {
		untokenize(*argv);
		zwarnnam(name, "bad pattern : %s", *argv, 0);
		returnval = 1;
	    }
	}
	/* If we didn't match anything, we return 1. */
	if (!match)
	    returnval = 1;
	return returnval;
    }

    /* Take arguments literally -- do not glob */
    for (; *argv; argv++) {
	if ((hn = ht->removenode(ht, *argv))) {
	    ht->freenode(hn);
	} else {
	    zwarnnam(name, "no such hash table element: %s", *argv, 0);
	    returnval = 1;
	}
    }
    return returnval;
}

/**** alias builtins ****/

/* alias: display or create aliases. */

/**/
int
bin_alias(char *name, char **argv, char *ops, int func)
{
    Alias a;
    Comp com;
    Asgment asg;
    int haveflags = 0, returnval = 0;
    int flags1 = 0, flags2 = DISABLED;
    int printflags = 0;

    /* Did we specify the type of alias? */
    if (ops['r'] || ops['g']) {
	if (ops['r'] && ops['g']) {
	    zwarnnam(name, "illegal combination of options", NULL, 0);
	    return 1;
	}
	haveflags = 1;
	if (ops['g'])
	    flags1 |= ALIAS_GLOBAL;
	else
	    flags2 |= ALIAS_GLOBAL;
    }

    if (ops['L'])
	printflags |= PRINT_LIST;

    /* In the absence of arguments, list all aliases.  If a command *
     * line flag is specified, list only those of that type.        */
    if (!*argv) {
	scanhashtable(aliastab, 1, flags1, flags2, aliastab->printnode, printflags);
	return 0;
    }

    /* With the -m option, treat the arguments as *
     * glob patterns of aliases to display.       */
    if (ops['m']) {
	for (; *argv; argv++) {
	    tokenize(*argv);  /* expand argument */
	    if ((com = parsereg(*argv))) {
		/* display the matching aliases */
		scanmatchtable(aliastab, com, flags1, flags2, aliastab->printnode, printflags);
	    } else {
		untokenize(*argv);
		zwarnnam(name, "bad pattern : %s", *argv, 0);
		returnval = 1;
	    }
	}
	return returnval;
    }

    /* Take arguments literally.  Don't glob */
    while ((asg = getasg(*argv++))) {
	if (asg->value && !ops['L']) {
	    /* The argument is of the form foo=bar and we are not *
	     * forcing a listing with -L, so define an alias      */
	    aliastab->addnode(aliastab, ztrdup(asg->name),
		createaliasnode(ztrdup(asg->value), flags1));
	} else if ((a = (Alias) aliastab->getnode(aliastab, asg->name))) {
	    /* display alias if appropriate */
	    if (!haveflags ||
		(ops['r'] && !(a->flags & ALIAS_GLOBAL)) ||
		(ops['g'] &&  (a->flags & ALIAS_GLOBAL)))
		aliastab->printnode((HashNode) a, printflags);
	} else
	    returnval = 1;
    }
    return returnval;
}


/**** resource limit builtins ****/

#ifdef HAVE_GETRLIMIT

#if defined(RLIM_T_IS_QUAD_T) || defined(RLIM_T_IS_UNSIGNED)
# define ZSTRTORLIMT(a, b, c)	zstrtorlimit((a), (b), (c))
#else
# define ZSTRTORLIMT(a, b, c)	zstrtol((a), (b), (c))
#endif

/* Generated rec array containing limits required for the limit builtin.
 * They must appear in this array in numerical order of the RLIMIT_* macros.
 */

#include "rlimits.h"

#endif /* HAVE_GETRLIMIT */

/* limit: set or show resource limits.  The variable hard indicates *
 * whether `hard' or `soft' resource limits are being set/shown.    */

/**/
int
bin_limit(char *nam, char **argv, char *ops, int func)
{
#ifndef HAVE_GETRLIMIT
    /* limit builtin not appropriate to this system */
    zwarnnam(nam, "not available on this system", NULL, 0);
    return 1;
#else
    char *s;
    int hard, limnum, lim;
    rlim_t val;
    int ret = 0;

    hard = ops['h'];
    if (ops['s'] && !*argv)
	return setlimits(NULL);
    /* without arguments, display limits */
    if (!*argv) {
	showlimits(hard, -1);
	return 0;
    }
    while ((s = *argv++)) {
	/* Search for the appropriate resource name.  When a name matches (i.e. *
	 * starts with) the argument, the lim variable changes from -1 to the   *
	 * number of the resource.  If another match is found, lim goes to -2.  */
	for (lim = -1, limnum = 0; limnum < ZSH_NLIMITS; limnum++)
	    if (!strncmp(recs[limnum], s, strlen(s))) {
		if (lim != -1)
		    lim = -2;
		else
		    lim = limnum;
	    }
	/* lim==-1 indicates that no matches were found.       *
	 * lim==-2 indicates that multiple matches were found. */
	if (lim < 0) {
	    zwarnnam("limit",
		     (lim == -2) ? "ambiguous resource specification: %s"
		     : "no such resource: %s", s, 0);
	    return 1;
	}
	/* without value for limit, display the current limit */
	if (!(s = *argv++)) {
	    showlimits(hard, lim);
	    return 0;
	}
	if (lim==RLIMIT_CPU) {
	    /* time-type resource -- may be specified as seconds, or minutes or *
	     * hours with the `m' and `h' modifiers, and `:' may be used to add *
	     * together more than one of these.  It's easier to understand from *
	     * the code:                                                        */
	    val = ZSTRTORLIMT(s, &s, 10);
	    if (*s) {
		if ((*s == 'h' || *s == 'H') && !s[1])
		    val *= 3600L;
		else if ((*s == 'm' || *s == 'M') && !s[1])
		    val *= 60L;
		else if (*s == ':')
		    val = val * 60 + ZSTRTORLIMT(s + 1, &s, 10);
		else {
		    zwarnnam("limit", "unknown scaling factor: %s", s, 0);
		    return 1;
		}
	    }
	}
# ifdef RLIMIT_NPROC
	else if (lim == RLIMIT_NPROC)
	    /* pure numeric resource -- only a straight decimal number is
	    permitted. */
	    val = ZSTRTORLIMT(s, &s, 10);
# endif /* RLIMIT_NPROC */
# ifdef RLIMIT_NOFILE
	else if (lim == RLIMIT_NOFILE)
	    /* pure numeric resource -- only a straight decimal number is
	    permitted. */
	    val = ZSTRTORLIMT(s, &s, 10);
# endif /* RLIMIT_NOFILE */
# ifdef RLIMIT_AIO_OPS
	else if (lim == RLIMIT_AIO_OPS)
	    /* pure numeric resource -- only a straight decimal number is
	    permitted. */
	    val = ZSTRTORLIMT(s, &s, 10);
# endif /* RLIMIT_AIO_OPS */
# ifdef RLIMIT_PTHREAD
	else if (lim == RLIMIT_PTHREAD)
	    /* pure numeric resource -- only a straight decimal number is
	    permitted. */
	    val = ZSTRTORLIMT(s, &s, 10);
# endif /* RLIMIT_PTHREAD */
	else {
	    /* memory-type resource -- `k' and `M' modifiers are permitted,
	    meaning (respectively) 2^10 and 2^20. */
	    val = ZSTRTORLIMT(s, &s, 10);
	    if (!*s || ((*s == 'k' || *s == 'K') && !s[1]))
		val *= 1024L;
	    else if ((*s == 'M' || *s == 'm') && !s[1])
		val *= 1024L * 1024;
	    else {
		zwarnnam("limit", "unknown scaling factor: %s", s, 0);
		return 1;
	    }
	}
	/* new limit is valid and has been interpreted; apply it to the
	specified resource */
	if (hard) {
	    /* can only raise hard limits if running as root */
	    if (val > current_limits[lim].rlim_max && geteuid()) {
		zwarnnam("limit", "can't raise hard limits", NULL, 0);
		return 1;
	    } else {
		limits[lim].rlim_max = val;
		if (val < limits[lim].rlim_cur)
		    limits[lim].rlim_cur = val;
	    }
	} else if (val > limits[lim].rlim_max) {
	    zwarnnam("limit", "limit exceeds hard limit", NULL, 0);
	    return 1;
	} else
	    limits[lim].rlim_cur = val;
	if (ops['s'] && zsetlimit(lim, "limit"))
	    ret++;
    }
    return ret;
#endif /* HAVE_GETRLIMIT */
}

/* unlimit: remove resource limits.  Much of this code is the same as *
 * that in bin_limit().                                               */

/**/
int
bin_unlimit(char *nam, char **argv, char *ops, int func)
{
#ifndef HAVE_GETRLIMIT
    /* unlimit builtin not appropriate to this system */
    zwarnnam(nam, "not available on this system", NULL, 0);
    return 1;
#else
    int hard, limnum, lim;
    int ret = 0;
    uid_t euid = geteuid();

    hard = ops['h'];
    /* Without arguments, remove all limits. */
    if (!*argv) {
	for (limnum = 0; limnum != RLIM_NLIMITS; limnum++) {
	    if (hard) {
		if (euid && current_limits[limnum].rlim_max != RLIM_INFINITY)
		    ret++;
		else
		    limits[limnum].rlim_max = RLIM_INFINITY;
	    }
	    else
		limits[limnum].rlim_cur = limits[limnum].rlim_max;
	}
	if (ops['s'])
	    ret += setlimits(nam);
	if (ret)
	    zwarnnam(nam, "can't remove hard limits", NULL, 0);
    } else {
	for (; *argv; argv++) {
	    /* Search for the appropriate resource name.  When a name     *
	     * matches (i.e. starts with) the argument, the lim variable  *
	     * changes from -1 to the number of the resource.  If another *
	     * match is found, lim goes to -2.                            */
	    for (lim = -1, limnum = 0; limnum < ZSH_NLIMITS; limnum++)
		if (!strncmp(recs[limnum], *argv, strlen(*argv))) {
		    if (lim != -1)
			lim = -2;
		    else
			lim = limnum;
		}
	    /* lim==-1 indicates that no matches were found.       *
	     * lim==-2 indicates that multiple matches were found. */
	    if (lim < 0) {
		zwarnnam(nam,
			 (lim == -2) ? "ambiguous resource specification: %s"
			 : "no such resource: %s", *argv, 0);
		return 1;
	    }
	    /* remove specified limit */
	    if (hard) {
		if (euid && current_limits[lim].rlim_max != RLIM_INFINITY) {
		    zwarnnam(nam, "can't remove hard limits", NULL, 0);
		    ret++;
		} else
		    limits[lim].rlim_max = RLIM_INFINITY;
	    } else
		limits[lim].rlim_cur = limits[lim].rlim_max;
	    if (ops['s'] && zsetlimit(lim, nam))
		ret++;
	}
    }
    return ret;
#endif /* HAVE_GETRLIMIT */
}

/* ulimit: set or display resource limits */

/**/
int
bin_ulimit(char *name, char **argv, char *ops, int func)
{
#ifndef HAVE_GETRLIMIT
    /* builtin not appropriate */
    zwarnnam(name, "not available on this system", NULL, 0);
    return 1;
#else
    int res, resmask = 0, hard = 0, soft = 0, nres = 0;
    char *options;

    do {
	options = *argv;
	if (options && *options == '-' && !options[1]) {
	    zwarnnam(name, "missing option letter", NULL, 0);
	    return 1;
	}
	res = -1;
	if (options && *options == '-') {
	    argv++;
	    while (*++options) {
		if(*options == Meta)
		    *++options ^= 32;
		res = -1;
		switch (*options) {
		case 'H':
		    hard = 1;
		    continue;
		case 'S':
		    soft = 1;
		    continue;
		case 'a':
		    if (*argv || options[1] || resmask) {
			zwarnnam(name, "no arguments required after -a",
				 NULL, 0);
			return 1;
		    }
		    resmask = (1 << RLIM_NLIMITS) - 1;
		    nres = RLIM_NLIMITS;
		    continue;
		case 't':
		    res = RLIMIT_CPU;
		    break;
		case 'f':
		    res = RLIMIT_FSIZE;
		    break;
		case 'd':
		    res = RLIMIT_DATA;
		    break;
		case 's':
		    res = RLIMIT_STACK;
		    break;
		case 'c':
		    res = RLIMIT_CORE;
		    break;
# ifdef RLIMIT_RSS
		case 'm':
		    res = RLIMIT_RSS;
		    break;
# endif /* RLIMIT_RSS */
# ifdef RLIMIT_MEMLOCK
		case 'l':
		    res = RLIMIT_MEMLOCK;
		    break;
# endif /* RLIMIT_MEMLOCK */
# ifdef RLIMIT_NOFILE
		case 'n':
		    res = RLIMIT_NOFILE;
		    break;
# endif /* RLIMIT_NOFILE */
# ifdef RLIMIT_NPROC
		case 'u':
		    res = RLIMIT_NPROC;
		    break;
# endif /* RLIMIT_NPROC */
# ifdef RLIMIT_VMEM
		case 'v':
		    res = RLIMIT_VMEM;
		    break;
# endif /* RLIMIT_VMEM */
		default:
		    /* unrecognised limit */
		    zwarnnam(name, "bad option: -%c", NULL, *options);
		    return 1;
		}
		if (options[1]) {
		    resmask |= 1 << res;
		    nres++;
		}
	    }
	}
	if (!*argv || **argv == '-') {
	    if (res < 0) {
		if (*argv || nres)
		    continue;
		else
		    res = RLIMIT_FSIZE;
	    }
	    resmask |= 1 << res;
	    nres++;
	    continue;
	}
	if (res < 0)
	    res = RLIMIT_FSIZE;
	if (strcmp(*argv, "unlimited")) {
	    /* set limit to specified value */
	    rlim_t limit;

	    limit = ZSTRTORLIMT(*argv, NULL, 10);
	    /* scale appropriately */
	    switch (res) {
	    case RLIMIT_FSIZE:
	    case RLIMIT_CORE:
		limit *= 512;
		break;
	    case RLIMIT_DATA:
	    case RLIMIT_STACK:
# ifdef RLIMIT_RSS
	    case RLIMIT_RSS:
# endif /* RLIMIT_RSS */
# ifdef RLIMIT_MEMLOCK
	    case RLIMIT_MEMLOCK:
# endif /* RLIMIT_MEMLOCK */
# ifdef RLIMIT_VMEM
	    case RLIMIT_VMEM:
# endif /* RLIMIT_VMEM */
# ifdef RLIMIT_AIO_MEM
	    case RLIMIT_AIO_MEM:
# endif /* RLIMIT_AIO_MEM */
		limit *= 1024;
		break;
	    }
	    if (hard) {
		/* can't raise hard limit unless running as root */
		if (limit > current_limits[res].rlim_max && geteuid()) {
		    zwarnnam(name, "can't raise hard limits", NULL, 0);
		    return 1;
		}
		limits[res].rlim_max = limit;
		if (limit < limits[res].rlim_cur)
		    limits[res].rlim_cur = limit;
	    }
	    if (!hard || soft) {
		/* can't raise soft limit above hard limit */
		if (limit > limits[res].rlim_max) {
		    if (limit > current_limits[res].rlim_max && geteuid()) {
			zwarnnam(name, "value exceeds hard limit", NULL, 0);
			return 1;
		    }
		    limits[res].rlim_max = limits[res].rlim_cur = limit;
		} else
		    limits[res].rlim_cur = limit;
	    }
	} else {
	    /* remove specified limit */
	    if (hard) {
		/* can't remove hard limit unless running as root */
		if (current_limits[res].rlim_max != RLIM_INFINITY && geteuid()) {
		    zwarnnam(name, "can't remove hard limits", NULL, 0);
		    return 1;
		}
		limits[res].rlim_max = RLIM_INFINITY;
	    }
	    if (!hard || soft)
		/* `removal' of soft limit means setting it equal to the
		   corresponding hard limit */
		limits[res].rlim_cur = limits[res].rlim_max;
	}
	if (zsetlimit(res, name))
	    return 1;
	argv++;
    } while (*argv);
    for (res = 0; res < RLIM_NLIMITS; res++, resmask >>= 1)
	if (resmask & 1)
	    printulimit(res, hard, nres > 1);
    return 0;
#endif /* HAVE_GETRLIMIT */
}

/* Display resource limits.  hard indicates whether `hard' or `soft'  *
 * limits should be displayed.  lim specifies the limit, or may be -1 *
 * to show all.                                                       */

#ifdef HAVE_GETRLIMIT
/**/
void
showlimits(int hard, int lim)
{
    int rt;
    rlim_t val;

    /* main loop over resource types */
    for (rt = 0; rt != ZSH_NLIMITS; rt++)
	if (rt == lim || lim == -1) {
	    /* display limit for resource number rt */
	    printf("%-16s", recs[rt]);
	    val = (hard) ? limits[rt].rlim_max : limits[rt].rlim_cur;
	    if (val == RLIM_INFINITY)
		printf("unlimited\n");
	    else if (rt==RLIMIT_CPU)
		/* time-type resource -- display as hours, minutes and
		seconds. */
		printf("%d:%02d:%02d\n", (int)(val / 3600),
		       (int)(val / 60) % 60, (int)(val % 60));
# ifdef RLIMIT_NPROC
	    else if (rt == RLIMIT_NPROC)
		/* pure numeric resource */
		printf("%d\n", (int)val);
# endif /* RLIMIT_NPROC */
# ifdef RLIMIT_NOFILE
	    else if (rt == RLIMIT_NOFILE)
		/* pure numeric resource */
		printf("%d\n", (int)val);
# endif /* RLIMIT_NOFILE */
# ifdef RLIMIT_AIO_OPS
	    else if (rt == RLIMIT_AIO_OPS)
		/* pure numeric resource */
		printf("%d\n", (int)val);
# endif /* RLIMIT_AIO_OPS */
# ifdef RLIMIT_PTHREAD
	    else if (rt == RLIMIT_PTHREAD)
		/* pure numeric resource */
		printf("%d\n", (int)val);
# endif /* RLIMIT_PTHREAD */
	    else if (val >= 1024L * 1024L)
		/* memory resource -- display with `K' or `M' modifier */
# ifdef RLIM_T_IS_LONG_LONG
#  ifdef RLIM_T_IS_UNSIGNED
		printf("%lluMB\n", val / (1024L * 1024L));
	    else
		printf("%llukB\n", val / 1024L);
#  else
		printf("%lldMB\n", val / (1024L * 1024L));
	    else
		printf("%lldkB\n", val / 1024L);
#  endif /* RLIM_T_IS_UNSIGNED */
# else
# ifdef RLIM_T_IS_QUAD_T
		printf("%qdMB\n", val / (1024L * 1024L));
	    else
		printf("%qdkB\n", val / 1024L);
# else
#  ifdef RLIM_T_IS_UNSIGNED
		printf("%luMB\n", val / (1024L * 1024L));
            else
		printf("%lukB\n", val / 1024L);
#  else
		printf("%ldMB\n", val / (1024L * 1024L));
            else
		printf("%ldkB\n", val / 1024L);
#  endif /* RLIM_T_IS_UNSIGNED */
# endif /* RLIM_T_IS_QUAD_T */
# endif /* RLIM_T_IS_LONG_LONG */
	}
}
#endif  /* HAVE_GETRLIMIT */

/* Display a resource limit, in ulimit style.  lim specifies which   *
 * limit should be displayed, and hard indicates whether the hard or *
 * soft limit should be displayed.                                   */

#ifdef HAVE_GETRLIMIT
/**/
void
printulimit(int lim, int hard, int head)
{
    rlim_t limit;

    /* get the limit in question */
    limit = (hard) ? limits[lim].rlim_max : limits[lim].rlim_cur;
    /* display the appropriate heading */
    switch (lim) {
    case RLIMIT_CPU:
	if (head)
	    printf("cpu time (seconds)         ");
	break;
    case RLIMIT_FSIZE:
	if (head)
	    printf("file size (blocks)         ");
	if (limit != RLIM_INFINITY)
	    limit /= 512;
	break;
    case RLIMIT_DATA:
	if (head)
	    printf("data seg size (kbytes)     ");
	if (limit != RLIM_INFINITY)
	    limit /= 1024;
	break;
    case RLIMIT_STACK:
	if (head)
	    printf("stack size (kbytes)        ");
	if (limit != RLIM_INFINITY)
	    limit /= 1024;
	break;
    case RLIMIT_CORE:
	if (head)
	    printf("core file size (blocks)    ");
	if (limit != RLIM_INFINITY)
	    limit /= 512;
	break;
# ifdef RLIMIT_RSS
    case RLIMIT_RSS:
	if (head)
	    printf("resident set size (kbytes) ");
	if (limit != RLIM_INFINITY)
	    limit /= 1024;
	break;
# endif /* RLIMIT_RSS */
# ifdef RLIMIT_MEMLOCK
    case RLIMIT_MEMLOCK:
	if (head)
	    printf("locked-in-memory size (kb) ");
	if (limit != RLIM_INFINITY)
	    limit /= 1024;
	break;
# endif /* RLIMIT_MEMLOCK */
# ifdef RLIMIT_NPROC
    case RLIMIT_NPROC:
	if (head)
	    printf("processes                  ");
	break;
# endif /* RLIMIT_NPROC */
# ifdef RLIMIT_NOFILE
    case RLIMIT_NOFILE:
	if (head)
	    printf("file descriptors           ");
	break;
# endif /* RLIMIT_NOFILE */
# ifdef RLIMIT_VMEM
    case RLIMIT_VMEM:
	if (head)
	    printf("virtual memory size (kb)   ");
	if (limit != RLIM_INFINITY)
	    limit /= 1024;
	break;
# endif /* RLIMIT_VMEM */
# if defined RLIMIT_AS && RLIMIT_AS != RLIMIT_VMEM
    case RLIMIT_AS:
	if (head)
	    printf("address space (kb)         ");
	if (limit != RLIM_INFINITY)
	    limit /= 1024;
	break;
# endif /* RLIMIT_AS */
# ifdef RLIMIT_TCACHE
    case RLIMIT_TCACHE:
	if (head)
	    printf("cached threads             ");
	break;
# endif /* RLIMIT_TCACHE */
# ifdef RLIMIT_AIO_OPS
    case RLIMIT_AIO_OPS:
	if (head)
	    printf("AIO operations             ");
	break;
# endif /* RLIMIT_AIO_OPS */
# ifdef RLIMIT_AIO_MEM
    case RLIMIT_AIO_MEM:
	if (head)
	    printf("AIO locked-in-memory (kb)  ");
	if (limit != RLIM_INFINITY)
	    limit /= 1024;
	break;
# endif /* RLIMIT_AIO_MEM */
# ifdef RLIMIT_SBSIZE
    case RLIMIT_SBSIZE:
	if (head)
	    printf("socket buffer size (kb)    ");
	if (limit != RLIM_INFINITY)
	    limit /= 1024;
	break;
# endif /* RLIMIT_SBSIZE */
# ifdef RLIMIT_PTHREAD
    case RLIMIT_PTHREAD:
	if (head)
	    printf("threads per process        ");
	break;
# endif /* RLIMIT_PTHREAD */
    }
    /* display the limit */
    if (limit == RLIM_INFINITY)
	printf("unlimited\n");
    else {
# ifdef RLIM_T_IS_QUAD_T
	printf("%qd\n", limit);
# else
#  ifdef RLIM_T_IS_LONG_LONG
#    ifdef RLIM_T_IS_UNSIGNED
	printf("%llu\n", limit);
#    else
	printf("%lld\n", limit);
#    endif /* RLIM_T_IS_UNSIGNED */
#  else
#   ifdef RLIM_T_IS_UNSIGNED
	printf("%lu\n", limit);
#   else
	printf("%ld\n", limit);
#   endif /* RLIM_T_IS_UNSIGNED */
#  endif /* RLIM_T_IS_LONG_LONG */
# endif /* RLIM_T_IS_QUAD_T */
    }
}
#endif /* HAVE_GETRLIMIT */

/**** miscellaneous builtins ****/

/* true, : (colon) */

/**/
int
bin_true(char *name, char **argv, char *ops, int func)
{
    return 0;
}

/* false builtin */

/**/
int
bin_false(char *name, char **argv, char *ops, int func)
{
    return 1;
}

/* echo, print, pushln */

/**/
int
bin_print(char *name, char **args, char *ops, int func)
{
    int nnl = 0, fd, argc, n;
    int *len;
    Histent ent;
    FILE *fout = stdout;

    /* -m option -- treat the first argument as a pattern and remove
     * arguments not matching */
    if (ops['m']) {
	Comp com;
	char **t, **p;

	tokenize(*args);
	if (!(com = parsereg(*args))) {
	    untokenize(*args);
	    zwarnnam(name, "bad pattern : %s", *args, 0);
	    return 1;
	}
	for (p = ++args; *p; p++)
	    if (!domatch(*p, com, 0))
		for (t = p--; (*t = t[1]); t++);
    }
    /* compute lengths, and interpret according to -P, -D, -e, etc. */
    argc = arrlen(args);
    len = (int *)ncalloc(argc * sizeof(int));
    for(n = 0; n < argc; n++) {
	/* first \ sequences */
	if (!ops['e'] && (ops['R'] || ops['r'] || ops['E']))
	    unmetafy(args[n], &len[n]);
	else
	    args[n] = getkeystring(args[n], &len[n],
				    func != BIN_ECHO && !ops['e'], &nnl);
	/* -P option -- interpret as a prompt sequence */
	if(ops['P']) {
	    char *arg = putprompt(metafy(args[n], len[n], META_NOALLOC),
				  &len[n], NULL, 0);
	    args[n] = (char *)alloc(len[n] + 1);
	    memcpy(args[n], arg, len[n]);
	    args[n][len[n]] = 0;
	    free(arg);
	}
	/* -D option -- interpret as a directory, and use ~ */
	if(ops['D']) {
	    Nameddir d = finddir(args[n]);
	    if(d) {
		char *arg = alloc(strlen(args[n]) + 1);
		sprintf(arg, "~%s%s", d->nam,
			args[n] + strlen(d->dir));
		args[n] = arg;
		len[n] = strlen(args[n]);
	    }
	}
    }

    /* -z option -- push the arguments onto the editing buffer stack */
    if (ops['z']) {
	PERMALLOC {
	    pushnode(bufstack, sepjoin(args, NULL));
	} LASTALLOC;
	return 0;
    }
    /* -s option -- add the arguments to the history list */
    if (ops['s']) {
	int nwords = 0, nlen, iwords;
	char **pargs = args;

	PERMALLOC {
	    ent = gethistent(++curhist);
	    zsfree(ent->text);
	    if (ent->nwords)
		zfree(ent->words, ent->nwords*2*sizeof(short));
	    while (*pargs++)
		nwords++;
	    if ((ent->nwords = nwords)) {
		ent->words = (short *)zalloc(nwords*2*sizeof(short));
		nlen = iwords = 0;
		for (pargs = args; *pargs; pargs++) {
		    ent->words[iwords++] = nlen;
		    nlen += strlen(*pargs);
		    ent->words[iwords++] = nlen;
		    nlen++;
		}
	    } else
		ent->words = (short *)NULL;
	    ent->text = zjoin(args, ' ');
	    ent->stim = ent->ftim = time(NULL);
	    ent->flags = 0;
	} LASTALLOC;
	return 0;
    }
    /* -u and -p -- output to other than standard output */
    if (ops['u'] || ops['p']) {
	if (ops['u']) {
	    for (fd = 0; fd < 10; fd++)
		if (ops[fd + '0'])
		    break;
	    if (fd == 10)
		fd = 0;
	} else
	    fd = coprocout;
	if ((fd = dup(fd)) < 0) {
	    zwarnnam(name, "bad file number", NULL, 0);
	    return 1;
	}
	if ((fout = fdopen(fd, "w")) == 0) {
	    zwarnnam(name, "bad mode on fd", NULL, 0);
	    return 1;
	}
    }

    /* -o and -O -- sort the arguments */
    if (ops['o']) {
	if (ops['i'])
	    qsort(args, arrlen(args), sizeof(char *), cstrpcmp);

	else
	    qsort(args, arrlen(args), sizeof(char *), strpcmp);
    } else if (ops['O']) {
	if (ops['i'])
	    qsort(args, arrlen(args), sizeof(char *), invcstrpcmp);

	else
	    qsort(args, arrlen(args), sizeof(char *), invstrpcmp);
    }
    /* after sorting arguments, recalculate lengths */
    if(ops['o'] || ops['O'])
	for(n = 0; n < argc; n++)
	    len[n] = strlen(args[n]);

    /* -c -- output in columns */
    if (ops['c']) {
	int l, nc, nr, sc, n, t, i;
	char **ap;

	for (n = l = 0, ap = args; *ap; ap++, n++)
	    if (l < (t = strlen(*ap)))
		l = t;

	sc = l + 2;
	nc = (columns + 1) / sc;
	if (!nc)
	    nc = 1;
	nr = (n + nc - 1) / nc;

	for (i = 0; i < nr; i++) {
	    ap = args + i;
	    do {
		l = strlen(*ap);
		fprintf(fout, "%s", *ap);
		for (t = nr; t && *ap; t--, ap++);
		if(*ap)
		    for (; l < sc; l++)
			fputc(' ', fout);
	    } while (*ap);
	    fputc(ops['N'] ? '\0' : '\n', fout);
	}
	if (fout != stdout)
	    fclose(fout);
	return 0;
    }
    /* normal output */
    for (; *args; args++, len++) {
	fwrite(*args, *len, 1, fout);
	if (args[1])
	    fputc(ops['l'] ? '\n' : ops['N'] ? '\0' : ' ', fout);
    }
    if (!(ops['n'] || nnl))
	fputc(ops['N'] ? '\0' : '\n', fout);
    if (fout != stdout)
	fclose(fout);
    return 0;
}

/* echotc: output a termcap */

/**/
int
bin_echotc(char *name, char **argv, char *ops, int func)
{
    char *s, buf[2048], *t, *u;
    int num, argct;

    s = *argv++;
    if (termflags & TERM_BAD)
	return 1;
    if ((termflags & TERM_UNKNOWN) && (isset(INTERACTIVE) || !init_term()))
	return 1;
    /* if the specified termcap has a numeric value, display it */
    if ((num = tgetnum(s)) != -1) {
	printf("%d\n", num);
	return 0;
    }
    /* if the specified termcap is boolean, and set, say so  *
     * ncurses can tell if an existing boolean capability is *
     * off so in this case we print "no".                    */
#ifndef NCURSES_VERSION
    if (tgetflag(s) > 0) {
	puts("yes");
	return (0);
    }
#else
    switch (tgetflag(s)) {
    case -1:
	break;
    case 0:
	puts("no");
	return 0;
    default:
	puts("yes");
	return 0;
    }
#endif
    /* get a string-type capability */
    u = buf;
    t = tgetstr(s, &u);
    if (!t || !*t) {
	/* capability doesn't exist, or (if boolean) is off */
	zwarnnam(name, "no such capability: %s", s, 0);
	return 1;
    }
    /* count the number of arguments required */
    for (argct = 0, u = t; *u; u++)
	if (*u == '%') {
	    if (u++, (*u == 'd' || *u == '2' || *u == '3' || *u == '.' ||
		      *u == '+'))
		argct++;
	}
    /* check that the number of arguments provided is correct */
    if (arrlen(argv) != argct) {
	zwarnnam(name, (arrlen(argv) < argct) ? "not enough arguments" :
		 "too many arguments", NULL, 0);
	return 1;
    }
    /* output string, through the proper termcap functions */
    if (!argct)
	tputs(t, 1, putraw);
    else {
	num = (argv[1]) ? atoi(argv[1]) : atoi(*argv);
	tputs(tgoto(t, atoi(*argv), num), num, putraw);
    }
    return 0;
}

/* shift builtin */

/**/
int
bin_shift(char *name, char **argv, char *ops, int func)
{
    int num = 1, l, ret = 0;
    char **s;
 
    /* optional argument can be either numeric or an array */
    if (*argv && !getaparam(*argv))
        num = matheval(*argv++);
 
    if (num < 0) {
        zwarnnam(name, "argument to shift must be non-negative", NULL, 0);
        return 1;
    }

    if (*argv) {
        for (; *argv; argv++)
            if ((s = getaparam(*argv))) {
                if (num > arrlen(s)) {
		    zwarnnam(name, "shift count must be <= $#", NULL, 0);
		    ret++;
		    continue;
		}
                PERMALLOC {
		    s = arrdup(s + num);
                } LASTALLOC;
                setaparam(*argv, s);
            }
    } else {
        if (num > (l = arrlen(pparams))) {
	    zwarnnam(name, "shift count must be <= $#", NULL, 0);
	    ret = 1;
	} else {
	    s = zalloc((l - num + 1) * sizeof(char *));
	    memcpy(s, pparams + num, (l - num + 1) * sizeof(char *));
	    while (num--)
		zsfree(pparams[num]);
	    zfree(pparams, (l + 1) * sizeof(char *));
	    pparams = s;
	}
    }
    return ret;
}

/* getopts: automagical option handling for shell scripts */

/**/
int
bin_getopts(char *name, char **argv, char *ops, int func)
{
    int lenstr, lenoptstr, i, plus;
    char *optstr = unmetafy(*argv++, &lenoptstr), *var = *argv++;
    char **args = (*argv) ? argv : pparams;
    static int optcind = 1, quiet;
    char *str, optbuf[2], *opch = optbuf + 1;

    /* zoptind keeps count of the current argument number */
    if (zoptind < 1)
	/* first call */
	zoptind = 1;
    if (zoptind == 1)
	quiet = 0;
    optbuf[0] = '+';
    zsfree(zoptarg);
    zoptarg = ztrdup("");
    setsparam(var, ztrdup(""));
    if (*optstr == ':') {
	quiet = 1;
	optstr++;
	lenoptstr--;
    }
    if (zoptind > arrlen(args))
	return 1;
    str = unmetafy(args[zoptind - 1], &lenstr);
    if ((*str != '+' && *str != '-') || optcind >= lenstr ||
	(lenstr == 2 && str[0] == '-' && str[1] == '-')) {
	/* current argument doesn't contain options, or optcind is impossibly
	large */
	if (*str == '+' || *str == '-')
	    zoptind++;
	optcind = 0;
	return 1;
    }
    /* Get the option character.  optcind records the current position within
    the argument. */
    if (!optcind)
	optcind = 1;
    *opch = str[optcind++];
    plus = (*str == '+');
    if (optcind == lenstr) {
	if(args[zoptind++])
	    str = unmetafy(args[zoptind - 1], &lenstr);
	optcind = 0;
    }
    /* look for option in the provided optstr */
    for (i = 0; i != lenoptstr; i++)
	if (*opch == optstr[i])
	    break;
    if (i == lenoptstr || *opch == ':') {
	/* not a valid option */
	setsparam(var, ztrdup("?"));
	if (quiet) {
	    zsfree(zoptarg);
	    zoptarg = metafy(opch, 1, META_DUP);
	    return 0;
	}
	zerr("bad option: -%c", NULL, *opch);
	errflag = 0;
	return 0;
    }
    /* copy option into specified parameter, with + if required */
    setsparam(var, metafy(opch - plus, 1 + plus, META_DUP));
    /* handle case of an expected extra argument */
    if (optstr[i + 1] == ':') {
	if (!args[zoptind - 1]) {
	    /* no extra argument was provided */
	    if (quiet) {
		zsfree(zoptarg);
		zoptarg = metafy(opch, 1, META_DUP);
		setsparam(var, ztrdup(":"));
		return 0;
	    }
	    setsparam(var, ztrdup("?"));
	    zerr("argument expected after -%c option", NULL, *opch);
	    errflag = 0;
	    return 0;
	}
	/* skip over the extra argument */
	zsfree(zoptarg);
	zoptarg = metafy(str + optcind, lenstr - optcind, META_DUP);
	zoptind++;
	optcind = 0;
    }
    return 0;
}

/* break, bye, continue, exit, logout, return -- most of these take   *
 * one numeric argument, and the other (logout) is related to return. *
 * (return is treated as a logout when in a login shell.)             */

/**/
int
bin_break(char *name, char **argv, char *ops, int func)
{
    int num = lastval, nump = 0;

    /* handle one optional numeric argument */
    if (*argv) {
	num = matheval(*argv++);
	nump = 1;
    }

    switch (func) {
    case BIN_CONTINUE:
	if (!loops) {   /* continue is only permitted in loops */
	    zerrnam(name, "not in while, until, select, or repeat loop", NULL, 0);
	    return 1;
	}
	contflag = 1;   /* ARE WE SUPPOSED TO FALL THROUGH HERE? */
    case BIN_BREAK:
	if (!loops) {   /* break is only permitted in loops */
	    zerrnam(name, "not in while, until, select, or repeat loop", NULL, 0);
	    return 1;
	}
	breaks = nump ? minimum(num,loops) : 1;
	break;
    case BIN_RETURN:
	if (isset(INTERACTIVE) || locallevel || sourcelevel) {
	    retflag = 1;
	    breaks = loops;
	    lastval = num;
	    if (trapreturn == -2)
		trapreturn = lastval;
	    return lastval;
	}
	zexit(num, 0);	/* else treat return as logout/exit */
	break;
    case BIN_LOGOUT:
	if (unset(LOGINSHELL)) {
	    zerrnam(name, "not login shell", NULL, 0);
	    return 1;
	}
	zexit(num, 0);
	break;
    case BIN_EXIT:
	zexit(num, 0);
	break;
    }
    return 0;
}

/* exit the shell.  val is the return value of the shell.  *
 * from_signal should be non-zero if zexit is being called *
 * because of a signal.                                    */

/**/
void
zexit(int val, int from_signal)
{
    static int in_exit;

    HEAPALLOC {
	if (isset(MONITOR) && !stopmsg && !from_signal) {
	    scanjobs();    /* check if jobs need printing           */
	    checkjobs();   /* check if any jobs are running/stopped */
	    if (stopmsg) {
		stopmsg = 2;
		LASTALLOC_RETURN;
	    }
	}
	if (in_exit++ && from_signal)
	    LASTALLOC_RETURN;
	if (isset(MONITOR))
	    /* send SIGHUP to any jobs left running  */
	    killrunjobs(from_signal);
	if (isset(RCS) && interact) {
	    if (!nohistsave)
		savehistfile(getsparam("HISTFILE"), 1, isset(APPENDHISTORY) ? 3 : 0);
	    if (islogin && !subsh) {
		sourcehome(".zlogout");
#ifdef GLOBAL_ZLOGOUT
		source(GLOBAL_ZLOGOUT);
#endif
	    }
	}
	if (sigtrapped[SIGEXIT])
	    dotrap(SIGEXIT);
	if (mypid != getpid())
	    _exit(val);
	else
	    exit(val);
    } LASTALLOC;
}

/* . (dot), source */

/**/
int
bin_dot(char *name, char **argv, char *ops, int func)
{
    char **old, *old0 = NULL;
    int ret, diddot = 0, dotdot = 0;
    char buf[PATH_MAX];
    char *s, **t, *enam, *arg0;
    struct stat st;

    if (!*argv || strlen(*argv) >= PATH_MAX)
	return 0;
    old = pparams;
    /* get arguments for the script */
    if (argv[1]) {
	PERMALLOC {
	    pparams = arrdup(argv + 1);
	} LASTALLOC;
    }
    enam = arg0 = ztrdup(*argv);
    if (isset(FUNCTIONARGZERO)) {
	old0 = argzero;
	argzero = arg0;
    }
    s = unmeta(enam);
    errno = ENOENT;
    ret = 1;
    /* for source only, check in current directory first */
    if (*name != '.' && access(s, F_OK) == 0
	&& stat(s, &st) >= 0 && !S_ISDIR(st.st_mode)) {
	diddot = 1;
	ret = source(enam);
    }
    if (ret) {
	/* use a path with / in it */
	for (s = arg0; *s; s++)
	    if (*s == '/') {
		if (*arg0 == '.') {
		    if (arg0 + 1 == s)
			++diddot;
		    else if (arg0[1] == '.' && arg0 + 2 == s)
			++dotdot;
		}
		ret = source(arg0);
		break;
	    }
	if (!*s || (ret && isset(PATHDIRS) && diddot < 2 && dotdot == 0)) {
	    /* search path for script */
	    for (t = path; *t; t++) {
		if (!(*t)[0] || ((*t)[0] == '.' && !(*t)[1])) {
		    if (diddot)
			continue;
		    diddot = 1;
		    strcpy(buf, arg0);
		} else {
		    if (strlen(*t) + strlen(arg0) + 1 >= PATH_MAX)
			continue;
		    sprintf(buf, "%s/%s", *t, arg0);
		}
		s = unmeta(buf);
		if (access(s, F_OK) == 0 && stat(s, &st) >= 0
		    && !S_ISDIR(st.st_mode)) {
		    ret = source(enam = buf);
		    break;
		}
	    }
	}
    }
    /* clean up and return */
    if (argv[1]) {
	freearray(pparams);
	pparams = old;
    }
    if (ret)
	zwarnnam(name, "%e: %s", enam, errno);
    zsfree(arg0);
    if (old0)
	argzero = old0;
    return ret ? ret : lastval;
}

/**/
int
bin_emulate(char *nam, char **argv, char *ops, int func)
{
    emulate(*argv, ops['R']);
    if (ops['L'])
	dosetopt(LOCALOPTIONS, 1, 0);
    return 0;
}

/* eval: simple evaluation */

/**/
int
bin_eval(char *nam, char **argv, char *ops, int func)
{
    List list;

    list = parse_string(zjoin(argv, ' '), 0);
    if (!list) {
	errflag = 0;
	return 1;
    }
    execlist(list, 1, 0);
    if (errflag) {
	lastval = errflag;
	errflag = 0;
    }
    return lastval;
}

static char *zbuf;
static int readfd;

/* Read a character from readfd, or from the buffer zbuf.  Return EOF on end of
file/buffer. */

extern int cs;

/* read: get a line of input, or (for compctl functions) return some *
 * useful data about the state of the editing line.  The -E and -e   *
 * options mean that the result should be sent to stdout.  -e means, *
 * in addition, that the result should not actually be assigned to   *
 * the specified parameters.                                         */

/**/
int
bin_read(char *name, char **args, char *ops, int func)
{
    char *reply, *readpmpt;
    int bsiz, c = 0, gotnl = 0, al = 0, first, nchars = 1, bslash, keys = 0;
    int haso = 0;	/* true if /dev/tty has been opened specially */
    int isem = !strcmp(term, "emacs");
    char *buf, *bptr, *firstarg, *zbuforig;
    LinkList readll = newlinklist();
    FILE *oshout = NULL;

    if ((ops['k'] || ops['b']) && *args && idigit(**args)) {
	if (!(nchars = atoi(*args)))
	    nchars = 1;
	args++;
    }

    firstarg = *args;
    if (*args && **args == '?')
	args++;
    /* default result parameter */
    reply = *args ? *args++ : ops['A'] ? "reply" : "REPLY";
    if (ops['A'] && *args) {
	zwarnnam(name, "only one array argument allowed", NULL, 0);
	return 1;
    }

    if ((ops['k'] && !ops['u'] && !ops['p']) || ops['q']) {
	if (!zleactive) {
	    if (SHTTY == -1) {
		/* need to open /dev/tty specially */
		if ((SHTTY = open("/dev/tty", O_RDWR|O_NOCTTY)) != -1) {
		    haso = 1;
		    oshout = shout;
		    init_shout();
		}
	    } else if (!shout) {
		/* We need an output FILE* on the tty */
		init_shout();
	    }
	    /* We should have a SHTTY opened by now. */
	    if (SHTTY == -1) {
		/* Unfortunately, we didn't. */
		fprintf(stderr, "not interactive and can't open terminal\n");
		fflush(stderr);
		return 1;
	    }
	    if (unset(INTERACTIVE))
		gettyinfo(&shttyinfo);
	    /* attach to the tty */
	    attachtty(mypgrp);
	    if (!isem && ops['k'])
		setcbreak();
	    readfd = SHTTY;
	}
	keys = 1;
    } else if (ops['u'] && !ops['p']) {
	/* -u means take input from the specified file descriptor. *
	 * -up means take input from the coprocess.                */
	for (readfd = 9; readfd && !ops[readfd + '0']; --readfd);
    } else if (ops['p'])
	readfd = coprocin;
    else
	readfd = 0;

    /* handle prompt */
    if (firstarg) {
	for (readpmpt = firstarg;
	     *readpmpt && *readpmpt != '?'; readpmpt++);
	if (*readpmpt++) {
	    if (keys || isatty(0)) {
		zputs(readpmpt, (shout ? shout : stderr));
		fflush(shout ? shout : stderr);
	    }
	    readpmpt[-1] = '\0';
	}
    }

    /* option -k means read only a given number of characters (default 1) */
    if (ops['k']) {
	int val;
	char d;

	/* allocate buffer space for result */
	bptr = buf = (char *)zalloc(nchars+1);

	do {
#if defined(_WIN32)
	    /* If read returns 0, is end of file */
	    if ((val = readfile(readfd, bptr, nchars)) <= 0)
		break;
#else
	    /* If read returns 0, is end of file */
	    if ((val = read(readfd, bptr, nchars)) <= 0)
		break;
#endif
	    
	    /* decrement number of characters read from number required */
	    nchars -= val;

	    /* increment pointer past read characters */
	    bptr += val;
	} while (nchars > 0);
	
	if (!ops['u'] && !ops['p']) {
	    /* dispose of result appropriately, etc. */
	    if (isem)
		while (val > 0 && read(SHTTY, &d, 1) == 1 && d != '\n');
	    else
		settyinfo(&shttyinfo);
	    if (haso) {
		fclose(shout);	/* close(SHTTY) */
		shout = oshout;
		SHTTY = -1;
	    }
	}

	if (ops['e'] || ops['E'])
	    fwrite(buf, bptr - buf, 1, stdout);
	if (!ops['e'])
	    setsparam(reply, metafy(buf, bptr - buf, META_REALLOC));
	else
	    zfree(buf, bptr - buf + 1);
	return val <= 0;
    }

    /* option -l is used in compctl functions */
    if (ops['l']) {
	/* only allowed to be called by ZLE */
	if (!inzlefunc) {
	    zwarnnam(name, "option valid only in functions called from zle",
		     NULL, 0);
	    return 1;
	}
	/* -ln gives the index of the word the cursor is currently on, which is
	available in cs (but remember that Zsh counts from one, not zero!) */
	if (ops['n']) {
	    char nbuf[14];

	    if (ops['e'] || ops['E'])
		printf("%d\n", cs + 1);
	    if (!ops['e']) {
		sprintf(nbuf, "%d", cs + 1);
		setsparam(reply, ztrdup(nbuf));
	    }
	    return 0;
	}
	/* without -n, the current line is assigned to the given parameter as a
	scalar */
	if (ops['e'] || ops['E']) {
	    zputs((char *) line, stdout);
	    putchar('\n');
	}
	if (!ops['e'])
	    setsparam(reply, ztrdup((char *) line));
	return 0;
    }

    /* option -c is used in compctl functions */
    if (ops['c']) {
	int i;

	/* only allowed to be called by ZLE */
	if (!inzlefunc) {
	    zwarnnam(name, "option valid only in functions called from zle",
		     NULL, 0);
	    return 1;
	}
	/* -cn gives the current cursor position within the current word, which
	is available in clwpos (but remember that Zsh counts from one, not
	zero!) */
	if (ops['n']) {
	    char nbuf[14];

	    if (ops['e'] || ops['E'])
		printf("%d\n", clwpos + 1);
	    if (!ops['e']) {
		sprintf(nbuf, "%d", clwpos + 1);
		setsparam(reply, ztrdup(nbuf));
	    }
	    return 0;
	}
	/* without -n, the words of the current line are assigned to the given
	parameters separately */
	if (ops['A'] && !ops['e']) {
	    /* the -A option means that one array is specified, instead of
	    many parameters */
	    char **p, **b = (char **)zcalloc((clwnum + 1) * sizeof(char *));

	    for (i = 0, p = b; i < clwnum; p++, i++)
		*p = ztrdup(clwords[i]);

	    setaparam(reply, b);
	    return 0;
	}
	if (ops['e'] || ops['E']) {
	    for (i = 0; i < clwnum; i++) {
		zputs(clwords[i], stdout);
		putchar('\n');
	    }

	    if (ops['e'])
		return 0;
	}

	for (i = 0; i < clwnum && *args; reply = *args++, i++)
	    setsparam(reply, ztrdup(clwords[i]));

	if (i < clwnum) {
	    int j, len;

	    for (j = i, len = 0; j < clwnum; len += strlen(clwords[j++]));
	    bptr = buf = zalloc(len + j - i);
	    while (i < clwnum) {
		strucpy(&bptr, clwords[i++]);
		*bptr++ = ' ';
	    }
	    bptr[-1] = '\0';
	} else
	    buf = ztrdup("");
	setsparam(reply, buf);

	return 0;
    }

    /* option -q means get one character, and interpret it as a Y or N */
    if (ops['q']) {
	char readbuf[2];

	/* set up the buffer */
	readbuf[1] = '\0';

	/* get, and store, reply */
	readbuf[0] = ((char)getquery(NULL)) == 'y' ? 'y' : 'n';

	/* dispose of result appropriately, etc. */
	if (haso) {
	    fclose(shout);	/* close(SHTTY) */
	    shout = oshout;
	    SHTTY = -1;
	}

	if (ops['e'] || ops['E'])
	    printf("%s\n", readbuf);
	if (!ops['e'])
	    setsparam(reply, ztrdup(readbuf));

	return readbuf[0] == 'n';
    }

    /* All possible special types of input have been exhausted.  Take one line,
    and assign words to the parameters until they run out.  Leftover words go
    onto the last parameter.  If an array is specified, all the words become
    separate elements of the array. */

    zbuforig = zbuf = (!ops['z']) ? NULL :
	(nonempty(bufstack)) ? (char *) getlinknode(bufstack) : ztrdup("");
    first = 1;
    bslash = 0;
    while (*args || (ops['A'] && !gotnl)) {
	sigset_t s = child_unblock();
	buf = bptr = (char *)zalloc(bsiz = 64);
	/* get input, a character at a time */
	while (!gotnl) {
	    c = zread();
	    /* \ at the end of a line indicates a continuation *
	     * line, except in raw mode (-r option)            */
	    if (bslash && c == '\n') {
		bslash = 0;
		continue;
	    }
	    if (c == EOF || c == '\n')
		break;
	    if (!bslash && isep(c)) {
		if (bptr != buf || (!iwsep(c) && first)) {
		    first |= !iwsep(c);
		    break;
		}
		first |= !iwsep(c);
		continue;
	    }
	    bslash = c == '\\' && !bslash && !ops['r'];
	    if (bslash)
		continue;
	    first = 0;
	    if (imeta(c)) {
		*bptr++ = Meta;
		*bptr++ = c ^ 32;
	    } else
		*bptr++ = c;
	    /* increase the buffer size, if necessary */
	    if (bptr >= buf + bsiz - 1) {
		int blen = bptr - buf;

		buf = realloc(buf, bsiz *= 2);
		bptr = buf + blen;
	    }
	}
	signal_setmask(s);
	if (c == '\n' || c == EOF)
	    gotnl = 1;
	*bptr = '\0';
	/* dispose of word appropriately */
	if (ops['e'] || ops['E']) {
	    zputs(buf, stdout);
	    putchar('\n');
	}
	if (!ops['e']) {
	    if (ops['A']) {
		addlinknode(readll, buf);
		al++;
	    } else
		setsparam(reply, buf);
	} else
	    free(buf);
	if (!ops['A'])
	    reply = *args++;
    }
    /* handle EOF */
    if (c == EOF) {
	if (readfd == coprocin) {
	    close(coprocin);
	    close(coprocout);
	    coprocin = coprocout = -1;
	}
    }
    /* final assignment (and display) of array parameter */
    if (ops['A']) {
	char **pp, **p = NULL;
	LinkNode n;

	p = (ops['e'] ? (char **)NULL
	     : (char **)zalloc((al + 1) * sizeof(char *)));

	for (pp = p, n = firstnode(readll); n; incnode(n)) {
	    if (ops['e'] || ops['E']) {
		zputs((char *) getdata(n), stdout);
		putchar('\n');
	    }
	    if (p)
		*pp++ = (char *)getdata(n);
	    else
		zsfree(getdata(n));
	}
	if (p) {
	    *pp++ = NULL;
	    setaparam(reply, p);
	}
	return c == EOF;
    }
    buf = bptr = (char *)zalloc(bsiz = 64);
    /* any remaining part of the line goes into one parameter */
    bslash = 0;
    if (!gotnl) {
	sigset_t s = child_unblock();
	for (;;) {
	    c = zread();
	    /* \ at the end of a line introduces a continuation line, except in
	    raw mode (-r option) */
	    if (bslash && c == '\n') {
		bslash = 0;
		continue;
	    }
	    if (c == EOF || (c == '\n' && !zbuf))
		break;
	    if (!bslash && isep(c) && bptr == buf) {
		if (iwsep(c))
		    continue;
		else if (!first) {
		    first = 1;
		    continue;
		}
	    }
	    bslash = c == '\\' && !bslash && !ops['r'];
	    if (bslash)
		continue;
	    if (imeta(c)) {
		*bptr++ = Meta;
		*bptr++ = c ^ 32;
	    } else
		*bptr++ = c;
	    /* increase the buffer size, if necessary */
	    if (bptr >= buf + bsiz - 1) {
		int blen = bptr - buf;

		buf = realloc(buf, bsiz *= 2);
		bptr = buf + blen;
	    }
	}
	signal_setmask(s);
    }
    while (bptr > buf && iwsep(bptr[-1]))
	bptr--;
    *bptr = '\0';
    /* final assignment of reply, etc. */
    if (ops['e'] || ops['E']) {
	zputs(buf, stdout);
	putchar('\n');
    }
    if (!ops['e'])
	setsparam(reply, buf);
    else
	zsfree(buf);
    if (zbuforig) {
	char first = *zbuforig;

	zsfree(zbuforig);
	if (!first)
	    return 1;
    } else if (c == EOF) {
	if (readfd == coprocin) {
	    close(coprocin);
	    close(coprocout);
	    coprocin = coprocout = -1;
	}
	return 1;
    }
    return 0;
}

/**/
int
zread(void)
{
    char cc, retry = 0;

    /* use zbuf if possible */
    if (zbuf) {
	/* If zbuf points to anything, it points to the next character in the
	buffer.  This may be a null byte to indicate EOF.  If reading from the
	buffer, move on the buffer pointer. */
	if (*zbuf == Meta)
	    return zbuf++, STOUC(*zbuf++ ^ 32);
	else
	    return (*zbuf) ? STOUC(*zbuf++) : EOF;
    }
    for (;;) {
	/* read a character from readfd */
#if defined(_WIN32)
	switch (readfile(readfd, &cc, 1)) {
#else
	switch (read(readfd, &cc, 1)) {
#endif
	case 1:
	    /* return the character read */
	    return STOUC(cc);
	case -1:
#if defined(EAGAIN) || defined(EWOULDBLOCK)
	    if (!retry && readfd == 0 && (
# ifdef EAGAIN
		    errno == EAGAIN
#  ifdef EWOULDBLOCK
		    ||
#  endif /* EWOULDBLOCK */
# endif /* EAGAIN */
# ifdef EWOULDBLOCK
		    errno == EWOULDBLOCK
# endif /* EWOULDBLOCK */
		) && setblock_stdin()) {
		retry = 1;
		continue;
	    } else
#endif /* EAGAIN || EWOULDBLOCK */
	    if (errno == EINTR && !(errflag || retflag || breaks || contflag))
		continue;
	    break;
	}
	return EOF;
    }
}

/* sched: execute commands at scheduled times */

/**/
int
bin_sched(char *nam, char **argv, char *ops, int func)
{
    char *s = *argv++;
    time_t t;
    long h, m;
    struct tm *tm;
    struct schedcmd *sch, *sch2, *schl;
    int sn;

    /* If the argument begins with a -, remove the specified item from the
    schedule. */
    if (s && *s == '-') {
	sn = atoi(s + 1);

	if (!sn) {
	    zwarnnam("sched", "usage for delete: sched -<item#>.", NULL, 0);
	    return 1;
	}
	for (schl = (struct schedcmd *)&schedcmds, sch = schedcmds, sn--;
	     sch && sn; sch = (schl = sch)->next, sn--);
	if (!sch) {
	    zwarnnam("sched", "not that many entries", NULL, 0);
	    return 1;
	}
	schl->next = sch->next;
	zsfree(sch->cmd);
	zfree(sch, sizeof(struct schedcmd));

	return 0;
    }

    /* given no arguments, display the schedule list */
    if (!s) {
	char tbuf[40];

	for (sn = 1, sch = schedcmds; sch; sch = sch->next, sn++) {
	    t = sch->time;
	    tm = localtime(&t);
	    ztrftime(tbuf, 20, "%a %b %e %k:%M:%S", tm);
	    printf("%3d %s %s\n", sn, tbuf, sch->cmd);
	}
	return 0;
    } else if (!*argv) {
	/* other than the two cases above, sched *
	 *requires at least two arguments        */
	zwarnnam("sched", "not enough arguments", NULL, 0);
	return 1;
    }

    /* The first argument specifies the time to schedule the command for.  The
    remaining arguments form the command. */
    if (*s == '+') {
	/* + introduces a relative time.  The rest of the argument is an
	hour:minute offset from the current time.  Once the hour and minute
	numbers have been extracted, and the format verified, the resulting
	offset is simply added to the current time. */
	h = zstrtol(s + 1, &s, 10);
	if (*s != ':') {
	    zwarnnam("sched", "bad time specifier", NULL, 0);
	    return 1;
	}
	m = zstrtol(s + 1, &s, 10);
	if (*s) {
	    zwarnnam("sched", "bad time specifier", NULL, 0);
	    return 1;
	}
	t = time(NULL) + h * 3600 + m * 60;
    } else {
	/* If there is no +, an absolute time of day must have been given.
	This is in hour:minute format, optionally followed by a string starting
	with `a' or `p' (for a.m. or p.m.).  Characters after the `a' or `p'
	are ignored. */
	h = zstrtol(s, &s, 10);
	if (*s != ':') {
	    zwarnnam("sched", "bad time specifier", NULL, 0);
	    return 1;
	}
	m = zstrtol(s + 1, &s, 10);
	if (*s && *s != 'a' && *s != 'A' && *s != 'p' && *s != 'P') {
	    zwarnnam("sched", "bad time specifier", NULL, 0);
	    return 1;
	}
	t = time(NULL);
	tm = localtime(&t);
	t -= tm->tm_sec + tm->tm_min * 60 + tm->tm_hour * 3600;
	if (*s == 'p' || *s == 'P')
	    h += 12;
	t += h * 3600 + m * 60;
	/* If the specified time is before the current time, it must refer to
	tomorrow. */
	if (t < time(NULL))
	    t += 3600 * 24;
    }
    /* The time has been calculated; now add the new entry to the linked list
    of scheduled commands. */
    sch = (struct schedcmd *) zcalloc(sizeof *sch);
    sch->time = t;
    PERMALLOC {
	sch->cmd = zjoin(argv, ' ');
    } LASTALLOC;
    sch->next = NULL;
    for (sch2 = (struct schedcmd *)&schedcmds; sch2->next; sch2 = sch2->next);
    sch2->next = sch;
    return 0;
}


/* holds lexer for par_cond():  normally yylex(), testlex() for bin_test() */
extern void (*condlex) _((void));

/* holds arguments for testlex() */
char **testargs;

/* test, [: the old-style general purpose logical expression builtin */

/**/
void
testlex(void)
{
    if (tok == LEXERR)
	return;

    tokstr = *testargs;
    if (!*testargs) {
	/* if tok is already zero, reading past the end:  error */
	tok = tok ? NULLTOK : LEXERR;
	return;
    } else if (!strcmp(*testargs, "-o"))
	tok = DBAR;
    else if (!strcmp(*testargs, "-a"))
	tok = DAMPER;
    else if (!strcmp(*testargs, "!"))
	tok = BANG;
    else if (!strcmp(*testargs, "("))
	tok = INPAR;
    else if (!strcmp(*testargs, ")"))
	tok = OUTPAR;
    else
	tok = STRING;
    testargs++;
}

/**/
int
bin_test(char *name, char **argv, char *ops, int func)
{
    char **s;
    Cond c;

    /* if "test" was invoked as "[", it needs a matching "]" *
     * which is subsequently ignored                         */
    if (func == BIN_BRACKET) {
	for (s = argv; *s; s++);
	if (s == argv || strcmp(s[-1], "]")) {
	    zwarnnam(name, "']' expected", NULL, 0);
	    return 1;
	}
	s[-1] = NULL;
    }
    /* an empty argument list evaluates to false (1) */
    if (!*argv)
	return 1;

    testargs = argv;
    tok = NULLTOK;
    condlex = testlex;
    testlex();
    c = par_cond();
    condlex = yylex;

    if (errflag) {
	errflag = 0;
	return 1;
    }

    if (!c || tok == LEXERR) {
	zwarnnam(name, tokstr ? "parse error" : "argument expected", NULL, 0);
	return 1;
    }

    /* syntax is OK, so evaluate */
    return !evalcond(c);
}

/* display a time, provided in units of 1/60s, as minutes and seconds */
#define pttime(X) printf("%ldm%ld.%02lds",((long) (X))/3600,\
			 ((long) (X))/60%60,((long) (X))*100/60%100)

/* times: display, in a two-line format, the times provided by times(3) */

/**/
int
bin_times(char *name, char **argv, char *ops, int func)
{
    struct tms buf;

    /* get time accounting information */
    if (times(&buf) == -1)
	return 1;
    pttime(buf.tms_utime);	/* user time */
    putchar(' ');
    pttime(buf.tms_stime);	/* system time */
    putchar('\n');
    pttime(buf.tms_cutime);	/* user time, children */
    putchar(' ');
    pttime(buf.tms_cstime);	/* system time, children */
    putchar('\n');
    return 0;
}

/* trap: set/unset signal traps */

/**/
int
bin_trap(char *name, char **argv, char *ops, int func)
{
    List l;
    char *arg, *s;
    int sig;

    if (*argv && !strcmp(*argv, "--"))
	argv++;

    /* If given no arguments, list all currently-set traps */
    if (!*argv) {
	for (sig = 0; sig < VSIGCOUNT; sig++) {
	    if (sigtrapped[sig] & ZSIG_FUNC) {
		char fname[20];
		HashNode hn;

		sprintf(fname, "TRAP%s", sigs[sig]);
		if ((hn = shfunctab->getnode(shfunctab, fname)))
		    shfunctab->printnode(hn, 0);
		DPUTS(!hn, "BUG: I did not find any trap functions!");
	    } else if (sigtrapped[sig]) {
		if (!sigfuncs[sig])
		    printf("trap -- '' %s\n", sigs[sig]);
		else {
		    s = getpermtext((void *) dupstruct((void *) sigfuncs[sig]));
		    printf("trap -- ");
		    quotedzputs(s, stdout);
		    printf(" %s\n", sigs[sig]);
		    zsfree(s);
		}
	    }
	}
	return 0;
    }

    /* If we have a signal number, unset the specified *
     * signals.  With only -, remove all traps.        */
    if ((getsignum(*argv) != -1) || (!strcmp(*argv, "-") && argv++)) {
	if (!*argv)
	    for (sig = 0; sig < VSIGCOUNT; sig++)
		unsettrap(sig);
	else
	    while (*argv)
		unsettrap(getsignum(*argv++));
	return 0;
    }

    /* Sort out the command to execute on trap */
    arg = *argv++;
    if (!*arg)
	l = NULL;
    else if (!(l = parse_string(arg, 0))) {
	zwarnnam(name, "couldn't parse trap command", NULL, 0);
	return 1;
    }

    /* set traps */
    for (; *argv; argv++) {
	List t;

	sig = getsignum(*argv);
	if (sig == -1) {
	    zwarnnam(name, "undefined signal: %s", *argv, 0);
	    break;
	}
	PERMALLOC {
	    t = (List) dupstruct(l);
	} LASTALLOC;
	if (settrap(sig, t))
	    freestruct(t);
    }
    return *argv != NULL;
}

/**/
int
bin_ttyctl(char *name, char **argv, char *ops, int func)
{
    if (ops['f'])
	ttyfrozen = 1;
    else if (ops['u'])
	ttyfrozen = 0;
    else
	printf("tty is %sfrozen\n", ttyfrozen ? "" : "not ");
    return 0;
}

/* let -- mathematical evaluation */

/**/
int
bin_let(char *name, char **argv, char *ops, int func)
{
    zlong val = 0;

    while (*argv)
	val = matheval(*argv++);
    /* Errors in math evaluation in let are non-fatal. */
    errflag = 0;
    return !val;
}

/* umask command.  umask may be specified as octal digits, or in the  *
 * symbolic form that chmod(1) uses.  Well, a subset of it.  Remember *
 * that only the bottom nine bits of umask are used, so there's no    *
 * point allowing the set{u,g}id and sticky bits to be specified.     */

/**/
int
bin_umask(char *nam, char **args, char *ops, int func)
{
    mode_t um;
    char *s = *args;

    /* Get the current umask. */
    um = umask(0);
    umask(um);
    /* No arguments means to display the current setting. */
    if (!s) {
	if (ops['S']) {
	    char *who = "ugo";

	    while (*who) {
		char *what = "rwx";
		printf("%c=", *who++);
		while (*what) {
		    if (!(um & 0400))
			putchar(*what);
		    um <<= 1;
		    what++;
		}
		putchar(*who ? ',' : '\n');
	    }
	} else {
	    if (um & 0700)
		putchar('0');
	    printf("%03o\n", (unsigned)um);
	}
	return 0;
    }

    if (idigit(*s)) {
	/* Simple digital umask. */
	um = zstrtol(s, &s, 8);
	if (*s) {
	    zwarnnam(nam, "bad umask", NULL, 0);
	    return 1;
	}
    } else {
	/* Symbolic notation -- slightly complicated. */
	int whomask, umaskop, mask;

	/* More than one symbolic argument may be used at once, each separated
	by commas. */
	for (;;) {
	    /* First part of the argument -- who does this apply to?
	    u=owner, g=group, o=other. */
	    whomask = 0;
	    while (*s == 'u' || *s == 'g' || *s == 'o' || *s == 'a')
		if (*s == 'u')
		    s++, whomask |= 0700;
		else if (*s == 'g')
		    s++, whomask |= 0070;
		else if (*s == 'o')
		    s++, whomask |= 0007;
		else if (*s == 'a')
		    s++, whomask |= 0777;
	    /* Default whomask is everyone. */
	    if (!whomask)
		whomask = 0777;
	    /* Operation may be +, - or =. */
	    umaskop = (int)*s;
	    if (!(umaskop == '+' || umaskop == '-' || umaskop == '=')) {
		if (umaskop)
		    zwarnnam(nam, "bad symbolic mode operator: %c", NULL, umaskop);
		else
		    zwarnnam(nam, "bad umask", NULL, 0);
		return 1;
	    }
	    /* Permissions mask -- r=read, w=write, x=execute. */
	    mask = 0;
	    while (*++s && *s != ',')
		if (*s == 'r')
		    mask |= 0444 & whomask;
		else if (*s == 'w')
		    mask |= 0222 & whomask;
		else if (*s == 'x')
		    mask |= 0111 & whomask;
		else {
		    zwarnnam(nam, "bad symbolic mode permission: %c",
			     NULL, *s);
		    return 1;
		}
	    /* Apply parsed argument to um. */
	    if (umaskop == '+')
		um &= ~mask;
	    else if (umaskop == '-')
		um |= mask;
	    else		/* umaskop == '=' */
		um = (um | (whomask)) & ~mask;
	    if (*s == ',')
		s++;
	    else
		break;
	}
	if (*s) {
	    zwarnnam(nam, "bad character in symbolic mode: %c", NULL, *s);
	    return 1;
	}
    }

    /* Finally, set the new umask. */
    umask(um);
    return 0;
}

/*** debugging functions ***/

#ifdef ZSH_HASH_DEBUG
/**/
int
bin_hashinfo(char *nam, char **args, char *ops, int func)
{
    printf("----------------------------------------------------\n");
    cmdnamtab->printinfo(cmdnamtab);
    printf("----------------------------------------------------\n");
    shfunctab->printinfo(shfunctab);
    printf("----------------------------------------------------\n");
    builtintab->printinfo(builtintab);
    printf("----------------------------------------------------\n");
    paramtab->printinfo(paramtab);
    printf("----------------------------------------------------\n");
    compctltab->printinfo(compctltab);
    printf("----------------------------------------------------\n");
    aliastab->printinfo(aliastab);
    printf("----------------------------------------------------\n");
    reswdtab->printinfo(reswdtab);
    printf("----------------------------------------------------\n");
    emkeybindtab->printinfo(emkeybindtab);
    printf("----------------------------------------------------\n");
    vikeybindtab->printinfo(vikeybindtab);
    printf("----------------------------------------------------\n");
    nameddirtab->printinfo(nameddirtab);
    printf("----------------------------------------------------\n");
    return 0;
}
#endif

/**** utility functions -- should go in utils.c ****/

/* Separate an argument into name=value parts, returning them in an     *
 * asgment structure.  Because the asgment structure used is global,    *
 * only one of these can be active at a time.  The string s gets placed *
 * in this global structure, so it needs to be in permanent memory.     */

/**/
Asgment
getasg(char *s)
{
    static struct asgment asg;

    /* sanity check for valid argument */
    if (!s)
	return NULL;

    /* check if name is empty */
    if (*s == '=') {
	zerr("bad assignment", NULL, 0);
	return NULL;
    }
    asg.name = s;

    /* search for `=' */
    for (; *s && *s != '='; s++);

    /* found `=', so return with a value */
    if (*s) {
	*s = '\0';
	asg.value = s + 1;
    } else {
    /* didn't find `=', so we only have a name */
	asg.value = NULL;
    }
    return &asg;
}

/* Get a signal number from a string */

/**/
int
getsignum(char *s)
{
    int x, i;

    /* check for a signal specified by number */
    x = atoi(s);
    if (idigit(*s) && x >= 0 && x < VSIGCOUNT)
	return x;

    /* search for signal by name */
    for (i = 0; i < VSIGCOUNT; i++)
	if (!strcmp(s, sigs[i]))
	    return i;

    /* no matching signal */
    return -1;
}
