/*
 * exec.c - command execution
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1997 Paul Falstad
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

#define execerr() if (!forked) { lastval = 1; goto done; } else _exit(1)

static LinkList args;
static int doneps4;
static char *STTYval;

/* parse string into a list */

/**/
List
parse_string(char *s, int ln)
{
    List l;
    int oldlineno = lineno;

    lexsave();
    inpush(s, (ln? INP_LINENO : 0), NULL);
    strinbeg();
    stophist = 2;
    lineno = ln ? 1 : -1;
    l = parse_list();
    lineno = oldlineno;
    strinend();
    inpop();
    lexrestore();
    return l;
}

#ifdef HAVE_GETRLIMIT
/**/
int
zsetlimit(int limnum, char *nam)
{
    if (limits[limnum].rlim_max != current_limits[limnum].rlim_max ||
	limits[limnum].rlim_cur != current_limits[limnum].rlim_cur) {
	if (setrlimit(limnum, limits + limnum)) {
	    if (nam)
		zwarnnam(nam, "setrlimit failed: %e", NULL, errno);
	    return -1;
	}
	current_limits[limnum] = limits[limnum];
    }
    return 0;
}

/**/
int
setlimits(char *nam)
{
    int limnum;
    int ret = 0;

    for (limnum = 0; limnum < RLIM_NLIMITS; limnum++)
	if (zsetlimit(limnum, nam))
	    ret++;
    return ret;
}
#endif

/* fork and set limits */

/**/
pid_t
zfork(void)
{
    pid_t pid;

    if (thisjob >= MAXJOB - 1) {
	zerr("job table full", NULL, 0);
	return -1;
    }
    pid = fork();
    if (pid == -1) {
	zerr("fork failed: %e", NULL, errno);
	return -1;
    }
#ifdef HAVE_GETRLIMIT
    if (!pid)
	/* set resource limits for the child process */
	setlimits(NULL);
#endif
    return pid;
}

/*
 *   Allen Edeln gebiet ich Andacht,
 *   Hohen und Niedern von Heimdalls Geschlecht;
 *   Ich will list_pipe's Wirken kuenden
 *   Die aeltesten Sagen, der ich mich entsinne...
 *
 * In most shells, if you do something like:
 *
 *   cat foo | while read a; do grep $a bar; done
 *
 * the shell forks and executes the loop in the sub-shell thus created.
 * In zsh this traditionally executes the loop in the current shell, which
 * is nice to have if the loop does something to change the shell, like
 * setting parameters or calling builtins.
 * Putting the loop in a sub-shell makes live easy, because the shell only
 * has to put it into the job-structure and then treats it as a normal
 * process. Suspending and interrupting is no problem then.
 * Some years ago, zsh either couldn't suspend such things at all, or
 * it got really messed up when users tried to do it. As a solution, we
 * implemented the list_pipe-stuff, which has since then become a reason
 * for many nightmares.
 * Pipelines like the one above are executed by the functions in this file
 * which call each other (and sometimes recursively). The one above, for
 * example would lead to a function call stack roughly like:
 *
 *  execlist->execpline->execcmd->execwhile->execlist->execpline
 *
 * (when waiting for the grep, ignoring execpline2 for now). At this time,
 * zsh has build two job-table entries for it: one for the cat and one for
 * the grep. If the user hits ^Z at this point (and jobbing is used), the 
 * shell is notified that the grep was suspended. The list_pipe flag is
 * used to tell the execpline where it was waiting that it was in a pipeline
 * with a shell construct at the end (which may also be a shell function or
 * several other things). When zsh sees the suspended grep, it forks to let
 * the sub-shell execute the rest of the while loop. The parent shell walks
 * up in the function call stack to the first execpline. There it has to find
 * out that it has just forked and then has to add information about the sub-
 * shell (its pid and the text for it) in the job entry of the cat. The pid
 * is passed down in the list_pipe_pid variable.
 * But there is a problem: the suspended grep is a child of the parent shell
 * and can't be adopted by the sub-shell. So the parent shell also has to 
 * keep the information about this process (more precisely: this pipeline)
 * by keeping the job table entry it created for it. The fact that there
 * are two jobs which have to be treated together is remembered by setting
 * the STAT_SUPERJOB flag in the entry for the cat-job (which now also
 * contains a process-entry for the whole loop -- the sub-shell) and by
 * setting STAT_SUBJOB in the job of the grep-job. With that we can keep
 * sub-jobs from being displayed and we can handle an fg/bg on the super-
 * job correctly. When the super-job is continued, the shell also wakes up
 * the sub-job. But then, the grep will exit sometime. Now the parent shell
 * has to remember not to try to wake it up again (in case of another ^Z).
 * It also has to wake up the sub-shell (which suspended itself immediately
 * after creation), so that the rest of the loop is executed by it.
 * But there is more: when the sub-shell is created, the cat may already
 * have exited, so we can't put the sub-shell in the process group of it.
 * In this case, we put the sub-shell in the process group of the parent
 * shell and in any case, the sub-shell has to put all commands executed
 * by it into its own process group, because only this way the parent
 * shell can control them since it only knows the process group of the sub-
 * shell. Of course, this information is also important when putting a job
 * in the foreground, where we have to attach its process group to the
 * controlling tty.
 * All this is made more difficult because we have to handle return values
 * correctly. If the grep is signaled, its exit status has to be propagated
 * back to the parent shell which needs it to set the exit status of the
 * super-job. And of course, when the grep is signaled (including ^C), the
 * loop has to be stopped, etc.
 * The code for all this is distributed over three files (exec.c, jobs.c,
 * and signals.c) and none of them is a simple one. So, all in all, there
 * may still be bugs, but considering the complexity (with race conditions,
 * signal handling, and all that), this should probably be expected.
 */

int list_pipe = 0, simple_pline = 0;

static pid_t list_pipe_pid;
static int nowait, pline_level = 0;
static int list_pipe_child = 0, list_pipe_job;
static char list_pipe_text[JOBTEXTSIZE];

/* execute a current shell command */

/**/
int
execcursh(Cmd cmd)
{
    if (!list_pipe && thisjob != list_pipe_job)
	deletejob(jobtab + thisjob);
    execlist(cmd->u.list, 1, cmd->flags & CFLAG_EXEC);
    cmd->u.list = NULL;
    return lastval;
}

/* execve after handling $_ and #! */

#define POUNDBANGLIMIT 64

/**/
int
zexecve(char *pth, char **argv)
{
    int eno;
    static char buf[PATH_MAX * 2];
    char **eep;

    unmetafy(pth, NULL);
    for (eep = argv; *eep; eep++)
	if (*eep != pth)
	    unmetafy(*eep, NULL);
    for (eep = environ; *eep; eep++)
	if (**eep == '_' && (*eep)[1] == '=')
	    break;
    buf[0] = '_';
    buf[1] = '=';
    if (IS_DIRSEP(*pth))
	strcpy(buf + 2, pth);
    else
	sprintf(buf + 2, "%s/%s", pwd, pth);
    if (!*eep)
	eep[1] = NULL;
    *eep = buf;
    execve(pth, argv, environ);

    /* If the execve returns (which in general shouldn't happen),   *
     * then check for an errno equal to ENOEXEC.  This errno is set *
     * if the process file has the appropriate access permission,   *
     * but has an invalid magic number in its header.               */
    if ((eno = errno) == ENOEXEC) {
	char execvebuf[POUNDBANGLIMIT + 1], *ptr, *ptr2, *argv0;
	int fd, ct, t0;

	if ((fd = open(pth, O_RDONLY)) >= 0) {
	    argv0 = *argv;
	    *argv = pth;
	    ct = read(fd, execvebuf, POUNDBANGLIMIT);
	    close(fd);
	    if (ct > 0) {
		if (execvebuf[0] == '#') {
		    if (execvebuf[1] == '!') {
			for (t0 = 0; t0 != ct; t0++)
			    if (execvebuf[t0] == '\n')
				break;
			while (inblank(execvebuf[t0]))
			    execvebuf[t0--] = '\0';
			execvebuf[POUNDBANGLIMIT] = '\0';
			for (ptr = execvebuf + 2; *ptr && *ptr == ' '; ptr++);
			for (ptr2 = ptr; *ptr && *ptr != ' '; ptr++);
			if (*ptr) {
			    *ptr = '\0';
			    argv[-2] = ptr2;
			    argv[-1] = ptr + 1;
			    execve(ptr2, argv - 2, environ);
			} else {
			    argv[-1] = ptr2;
			    execve(ptr2, argv - 1, environ);
			}
		    } else {
			argv[-1] = "sh";
			execve("/bin/sh", argv - 1, environ);
		    }
		} else {
		    for (t0 = 0; t0 != ct; t0++)
			if (!execvebuf[t0])
			    break;
		    if (t0 == ct) {
			argv[-1] = "sh";
			execve("/bin/sh", argv - 1, environ);
		    }
		}
	    } else
		eno = errno;
	    *argv = argv0;
	} else
	    eno = errno;
    }
    /* restore the original arguments and path but do not bother with *
     * null characters as these cannot be passed to external commands *
     * anyway.  So the result is truncated at the first null char.    */
    pth = metafy(pth, -1, META_NOALLOC);
    for (eep = argv; *eep; eep++)
	if (*eep != pth)
	    (void) metafy(*eep, -1, META_NOALLOC);
    return eno;
}

#define MAXCMDLEN (PATH_MAX*4)

/* test whether we really want to believe the error number */

/**/
int
isgooderr(int e, char *dir)
{
    /*
     * Maybe the directory was unreadable, or maybe it wasn't
     * even a directory. 
     */
    return ((e != EACCES || !access(dir, X_OK)) &&
	    e != ENOENT && e != ENOTDIR); 
}

/* execute an external command */

/**/
void
execute(Cmdnam not_used_yet, int dash)
{
    Cmdnam cn;
    char buf[MAXCMDLEN], buf2[MAXCMDLEN];
    char *s, *z, *arg0;
    char **argv, **pp;
    int eno = 0, ee;

    /* If the parameter STTY is set in the command's environment, *
     * we first run the stty command with the value of this       *
     * parameter as it arguments.                                 */
    if ((s = STTYval) && isatty(0) && (GETPGRP() == getpid())) {
	LinkList exargs = args;
	char *t = tricat("stty", " ", s);

	STTYval = 0;	/* this prevents infinite recursion */
	zsfree(s);
	args = NULL;
	execstring(t, 1, 0);
	zsfree(t);
	args = exargs;
    } else if (s) {
	STTYval = 0;
	zsfree(s);
    }

    arg0 = (char *) peekfirst(args);
    cn = (Cmdnam) cmdnamtab->getnode(cmdnamtab, arg0);

    /* If ARGV0 is in the commands environment, we use *
     * that as argv[0] for this external command       */
    if ((z = zgetenv("ARGV0"))) {
	setdata(firstnode(args), (void *) ztrdup(z));
	delenv(z - 6);
    } else if (dash) {
    /* Else if the pre-command `-' was given, we add `-' *
     * to the front of argv[0] for this command.         */
	sprintf(buf2, "-%s", arg0);
	setdata(firstnode(args), (void *) ztrdup(buf2));
    }

    argv = makecline(args);
    closem(3);
    child_unblock();
    if ((int) strlen(arg0) >= PATH_MAX) {
	zerr("command too long: %s", arg0, 0);
	_exit(1);
    }
    for (s = arg0; *s; s++)
	if (IS_DIRSEP(*s)) {
	    errno = zexecve(arg0, argv);
	    if (arg0 == s || unset(PATHDIRS) ||
		(arg0[0] == '.' && (arg0 + 1 == s ||
				    (arg0[1] == '.' && arg0 + 2 == s)))) {
		zerr("%e: %s", arg0, errno);
		_exit(1);
	    }
	    break;
	}

    if (cn) {
	char nn[PATH_MAX], *dptr;

	if (cn->flags & HASHED)
	    strcpy(nn, cn->u.cmd);
	else {
	    for (pp = path; pp < cn->u.name; pp++)
		if (!**pp || (**pp == '.' && (*pp)[1] == '\0')) {
		    ee = zexecve(arg0, argv);
		    if (isgooderr(ee, *pp))
			eno = ee;
		} else if (**pp != '/') {
		    z = buf;
		    strucpy(&z, *pp);
		    *z++ = '/';
		    strcpy(z, arg0);
		    ee = zexecve(buf, argv);
		    if (isgooderr(ee, *pp))
			eno = ee;
		}
	    strcpy(nn, cn->u.name ? *(cn->u.name) : "");
	    strcat(nn, "/");
	    strcat(nn, cn->nam);
	}
	ee = zexecve(nn, argv);

	if ((dptr = strrchr(nn, '/')))
	    *dptr = '\0';
	if (isgooderr(ee, *nn ? nn : "/"))
	    eno = ee;
    }
    for (pp = path; *pp; pp++)
	if (!(*pp)[0] || ((*pp)[0] == '.' && !(*pp)[1])) {
	    ee = zexecve(arg0, argv);
	    if (isgooderr(ee, *pp))
		eno = ee;
	} else {
	    z = buf;
	    strucpy(&z, *pp);
	    *z++ = '/';
	    strcpy(z, arg0);
	    ee = zexecve(buf, argv);
	    if (isgooderr(ee, *pp))
		eno = ee;
	}
    if (eno)
	zerr("%e: %s", arg0, eno);
    else
	zerr("command not found: %s", arg0, 0);
    _exit(1);
}

#if defined(_WIN32)
#define try(X) { if (iscom(X)) return ztrdupp(X); }
#else
#define try(X) { if (iscom(X)) return ztrdup(X); }
#endif

/* get the full pathname of an external command */

/**/
char *
findcmd(char *arg0)
{
    char **pp;
    char *z, *s, buf[MAXCMDLEN];
    Cmdnam cn;

    cn = (Cmdnam) cmdnamtab->getnode(cmdnamtab, arg0);
    if (!cn && isset(HASHCMDS))
	cn = hashcmd(arg0, path);
    if ((int) strlen(arg0) > PATH_MAX)
	return NULL;
    for (s = arg0; *s; s++)
	if (IS_DIRSEP(*s)) {
	    try(arg0);
	    if (arg0 == s || unset(PATHDIRS)) {
		return NULL;
	    }
	    break;
	}
    if (cn) {
	char nn[PATH_MAX];

	if (cn->flags & HASHED)
	    strcpy(nn, cn->u.cmd);
	else {
	    for (pp = path; pp < cn->u.name; pp++)
		if (**pp != '/') {
		    z = buf;
		    if (**pp) {
			strucpy(&z, *pp);
			*z++ = '/';
		    }
		    strcpy(z, arg0);
		    try(buf);
		}
	    strcpy(nn, cn->u.name ? *(cn->u.name) : "");
	    strcat(nn, "/");
	    strcat(nn, cn->nam);
	}
	try(nn);
    }
    for (pp = path; *pp; pp++) {
	z = buf;
	if (**pp) {
	    strucpy(&z, *pp);
	    *z++ = '/';
	}
	strcpy(z, arg0);
	try(buf);
    }
    return NULL;
}

/**/
int
iscom(char *s)
{
    char *us = unmeta(s);
#if defined(_WIN32)
    /* our access() is always false for directories */
    return (access(us, X_OK) == 0);
#else
    struct stat statbuf;

    return (access(us, X_OK) == 0 && stat(us, &statbuf) >= 0 && S_ISREG(statbuf.st_mode));
#endif
}

/**/
int
isrelative(char *s)
{
    /* @@@@ use IS_DIRSEP() ? */
#if defined(_WIN32)
    if ((*s != '/') && (s[1] && s[1] != ':'))
#else
    if (*s != '/')
#endif 
	return 1;
    for (; *s; s++)
	if (*s == '.' && s[-1] == '/' &&
	    (s[1] == '/' || s[1] == '\0' ||
	     (s[1] == '.' && (s[2] == '/' || s[2] == '\0'))))
	    return 1;
    return 0;
}

/**/
Cmdnam
hashcmd(char *arg0, char **pp)
{
    Cmdnam cn;
    char *s, buf[PATH_MAX];
    char **pq;

    for (; *pp; pp++)
	if (IS_DIRSEP(**pp)) {
	    s = buf;
	    strucpy(&s, *pp);
	    *s++ = '/';
	    if ((s - buf) + strlen(arg0) >= PATH_MAX)
		continue;
	    strcpy(s, arg0);
	    if (iscom(buf))
		break;
	}

    if (!*pp)
	return NULL;

    cn = (Cmdnam) zcalloc(sizeof *cn);
    cn->flags = 0;
    cn->u.name = pp;
    cmdnamtab->addnode(cmdnamtab, ztrdup(arg0), cn);

    if (isset(HASHDIRS)) {
	for (pq = pathchecked; pq <= pp; pq++)
	    hashdir(pq);
	pathchecked = pp + 1;
    }

    return cn;
}

/* execute a string */

/**/
void
execstring(char *s, int dont_change_job, int exiting)
{
    List list;

    pushheap();
    if ((list = parse_string(s, 0)))
	execlist(list, dont_change_job, exiting);
    popheap();
}

/* Main routine for executing a list.                                *
 * exiting means that the (sub)shell we are in is a definite goner   *
 * after the current list is finished, so we may be able to exec the *
 * last command directly instead of forking.  If dont_change_job is  *
 * nonzero, then restore the current job number after executing the  *
 * list.                                                             */

/**/
void
execlist(List list, int dont_change_job, int exiting)
{
    Sublist slist;
    static int donetrap;
    int ret, cj;
    int old_pline_level, old_list_pipe, oldlineno;
    /*
     * ERREXIT only forces the shell to exit if the last command in a &&
     * or || fails.  This is the case even if an earlier command is a
     * shell function or other current shell structure, so we have to set
     * noerrexit here if the sublist is not of type END.
     */
    int oldnoerrexit = noerrexit;

    cj = thisjob;
    old_pline_level = pline_level;
    old_list_pipe = list_pipe;
    oldlineno = lineno;

    if (sourcelevel && unset(SHINSTDIN))
	pline_level = list_pipe = 0;

    /* Loop over all sets of comands separated by newline, *
     * semi-colon or ampersand (`sublists').               */
    while (list && list != &dummy_list && !breaks && !retflag) {
	/* Reset donetrap:  this ensures that a trap is only *
	 * called once for each sublist that fails.          */
	donetrap = 0;
	simplifyright(list);
	slist = list->left;

	/* Loop through code followed by &&, ||, or end of sublist. */
	while (slist) {
	    if (!oldnoerrexit)
		noerrexit = (slist->type != END);
	    switch (slist->type) {
	    case END:
		/* End of sublist; just execute, ignoring status. */
		execpline(slist, list->type, !list->right && exiting);
		goto sublist_done;
		break;
	    case ANDNEXT:
		/* If the return code is non-zero, we skip pipelines until *
		 * we find a sublist followed by ORNEXT.                   */
		if ((ret = execpline(slist, Z_SYNC, 0))) {
		    while ((slist = slist->right))
			if (slist->type == ORNEXT)
			    break;
		    if (!slist) {
			/* We've skipped to the end of the list, not executing *
			 * the final pipeline, so don't perform error handling *
			 * for this sublist.                                   */
			donetrap = 1;
			goto sublist_done;
		    }
		}
		break;
	    case ORNEXT:
		/* If the return code is zero, we skip pipelines until *
		 * we find a sublist followed by ANDNEXT.              */
		if (!(ret = execpline(slist, Z_SYNC, 0))) {
		    while ((slist = slist->right))
			if (slist->type == ANDNEXT)
			    break;
		    if (!slist) {
			/* We've skipped to the end of the list, not executing *
			 * the final pipeline, so don't perform error handling *
			 * for this sublist.                                   */
			donetrap = 1;
			goto sublist_done;
		     }
		}
		break;
	    }
	    slist = slist->right;
	}
sublist_done:

	noerrexit = oldnoerrexit;

	if (sigtrapped[SIGDEBUG])
	    dotrap(SIGDEBUG);

	/* Check whether we are suppressing traps/errexit *
	 * (typically in init scripts) and if we haven't  *
	 * already performed them for this sublist.       */
	if (!noerrexit && !donetrap) {
	    if (sigtrapped[SIGZERR] && lastval) {
		dotrap(SIGZERR);
		donetrap = 1;
	    }
	    if (lastval && isset(ERREXIT)) {
		if (sigtrapped[SIGEXIT])
		    dotrap(SIGEXIT);
		if (mypid != getpid())
		    _exit(lastval);
		else
		    exit(lastval);
	    }
	}

	list = list->right;
    }

    pline_level = old_pline_level;
    list_pipe = old_list_pipe;
    lineno = oldlineno;
    if (dont_change_job)
	thisjob = cj;
}

/* Execute a pipeline.                                                *
 * last1 is a flag that this command is the last command in a shell   *
 * that is about to exit, so we can exec instead of forking.  It gets *
 * passed all the way down to execcmd() which actually makes the      *
 * decision.  A 0 is always passed if the command is not the last in  *
 * the pipeline.  This function assumes that the sublist is not NULL. *
 * If last1 is zero but the command is at the end of a pipeline, we   *
 * pass 2 down to execcmd().                                          *
 */

/**/
int
execpline(Sublist l, int how, int last1)
{
    int ipipe[2], opipe[2];
    int pj, newjob;
    int old_simple_pline = simple_pline;
    static int lastwj, lpforked;

    if (!l->left)
	return lastval = (l->flags & PFLAG_NOT) != 0;

    pj = thisjob;
    ipipe[0] = ipipe[1] = opipe[0] = opipe[1] = 0;
    child_block();

    /* get free entry in job table and initialize it */
    if ((thisjob = newjob = initjob()) == -1)
	return 1;
    if (how & Z_TIMED)
	jobtab[thisjob].stat |= STAT_TIMED;

    if (l->flags & PFLAG_COPROC) {
	how = Z_ASYNC;
	if (coprocin >= 0) {
	    zclose(coprocin);
	    zclose(coprocout);
	}
	mpipe(ipipe);
	mpipe(opipe);
	coprocin = ipipe[0];
	coprocout = opipe[1];
	fdtable[coprocin] = fdtable[coprocout] = 0;
    }
    /* This used to set list_pipe_pid=0 unconditionally, but in things
     * like `ls|if true; then sleep 20; cat; fi' where the sleep was
     * stopped, the top-level execpline() didn't get the pid for the
     * sub-shell because it was overwritten. */
    if (!pline_level++) {
        list_pipe_pid = 0;
	nowait = 0;
	simple_pline = (l->left->type == END);
	list_pipe_job = (simple_pline ? 0 : newjob);
    }
    lastwj = lpforked = 0;
    execpline2(l->left, how, opipe[0], ipipe[1], last1);
    pline_level--;
    if (how & Z_ASYNC) {
	lastwj = newjob;
	jobtab[thisjob].stat |= STAT_NOSTTY;
	if (l->flags & PFLAG_COPROC) {
	    zclose(ipipe[1]);
	    zclose(opipe[0]);
	}
	if (how & Z_DISOWN) {
	    deletejob(jobtab + thisjob);
	    thisjob = -1;
	}
	else
	    spawnjob();
	child_unblock();
	return 0;
    } else {
	if (newjob != lastwj) {
	    Job jn = jobtab + newjob;
	    int updated;

	    if (newjob == list_pipe_job && list_pipe_child)
		_exit(0);

	    lastwj = thisjob = newjob;

	    if (list_pipe || (pline_level && !(how & Z_TIMED)))
		jn->stat |= STAT_NOPRINT;

	    if (nowait) {
		if(!pline_level) {
		    struct process *pn, *qn;

		    curjob = newjob;
		    DPUTS(!list_pipe_pid, "invalid list_pipe_pid");
		    addproc(list_pipe_pid, list_pipe_text);

		    /* If the super-job contains only the sub-shell, the
		       sub-shell is the group leader. */
		    if (!jn->procs->next || lpforked == 2) {
			jn->gleader = list_pipe_pid;
			jn->stat |= STAT_SUBLEADER;
		    }
		    for (pn = jobtab[jn->other].procs; pn; pn = pn->next)
			if (WIFSTOPPED(pn->status))
			    break;

		    if (pn) {
			for (qn = jn->procs; qn->next; qn = qn->next);
			qn->status = pn->status;
		    }

		    jn->stat &= ~(STAT_DONE | STAT_NOPRINT);
		    jn->stat |= STAT_STOPPED | STAT_CHANGED | STAT_LOCKED;
		    printjob(jn, !!isset(LONGLISTJOBS), 1);
		}
		else if (newjob != list_pipe_job)
		    deletejob(jn);
		else
		    lastwj = -1;
	    }

	    errbrk_saved = 0;
	    for (; !nowait;) {
		if (list_pipe_child) {
		    jn->stat |= STAT_NOPRINT;
		    makerunning(jn);
		}
		if (!(jn->stat & STAT_LOCKED)) {
		    updated = !!jobtab[thisjob].procs;
		    waitjobs();
		    child_block();
		} else
		    updated = 0;
		if (!updated &&
		    list_pipe_job && jobtab[list_pipe_job].procs &&
		    !(jobtab[list_pipe_job].stat & STAT_STOPPED)) {
		    child_unblock();
		    child_block();
		}
		if (list_pipe_child &&
		    jn->stat & STAT_DONE &&
		    lastval2 & 0200)
		    killpg(mypgrp, lastval2 & ~0200);
		if (!list_pipe_child && !lpforked && !subsh && jobbing &&
		    (list_pipe || last1 || pline_level) &&
		    ((jn->stat & STAT_STOPPED) ||
  		     (list_pipe_job && pline_level &&
  		      (jobtab[list_pipe_job].stat & STAT_STOPPED)))) {
		    pid_t pid;
		    int synch[2];

		    pipe(synch);

		    if ((pid = fork()) == -1) {
			trashzle();
			close(synch[0]);
			close(synch[1]);
			putc('\n', stderr);
			fprintf(stderr, "zsh: job can't be suspended\n");
			fflush(stderr);
			makerunning(jn);
			killjb(jn, SIGCONT);
			thisjob = newjob;
		    }
		    else if (pid) {
			char dummy;

			lpforked = 
			    (killpg(jobtab[list_pipe_job].gleader, 0) == -1 ? 2 : 1);
			list_pipe_pid = pid;
			nowait = errflag = 1;
			breaks = loops;
			close(synch[1]);
			read(synch[0], &dummy, 1);
			close(synch[0]);
			/* If this job has finished, we leave it as a
			 * normal (non-super-) job. */
			if (!(jn->stat & STAT_DONE)) {
			    jobtab[list_pipe_job].other = newjob;
			    jobtab[list_pipe_job].stat |= STAT_SUPERJOB;
			    jn->stat |= STAT_SUBJOB | STAT_NOPRINT;
			    jn->other = pid;
			}
			if ((list_pipe || last1) && jobtab[list_pipe_job].procs)
			    killpg(jobtab[list_pipe_job].gleader, SIGSTOP);
			break;
		    }
		    else {
			close(synch[0]);
			entersubsh(Z_ASYNC, 0, 0);
			if (jobtab[list_pipe_job].procs) {
			    if (setpgrp(0L, mypgrp = jobtab[list_pipe_job].gleader)
				== -1) {
				setpgrp(0L, mypgrp = getpid());
			    }
			} else
			    setpgrp(0L, mypgrp = getpid());
			close(synch[1]);
			kill(getpid(), SIGSTOP);
			list_pipe = 0;
			list_pipe_child = 1;
			opts[INTERACTIVE] = 0;
			if (errbrk_saved) {
			    errflag = prev_errflag;
			    breaks = prev_breaks;
			}
			break;
		    }
		}
		else if (subsh && jn->stat & STAT_STOPPED)
		    thisjob = newjob;
		else
		    break;
	    }
	    child_unblock();

	    if (list_pipe && (lastval & 0200) && pj >= 0 &&
		(!(jn->stat & STAT_INUSE) || (jn->stat & STAT_DONE))) {
		deletejob(jn);
		jn = jobtab + pj;
		killjb(jn, lastval & ~0200);
	    }
	    if (list_pipe_child ||
		((jn->stat & STAT_DONE) &&
		 (list_pipe || (pline_level && !(jn->stat & STAT_SUBJOB)))))
		deletejob(jn);
	    thisjob = pj;

	}
	if (l->flags & PFLAG_NOT)
	    lastval = !lastval;
    }
    if (!pline_level)
	simple_pline = old_simple_pline;
    return lastval;
}

int subsh_close = -1;

/* execute pipeline.  This function assumes the `pline' is not NULL. */

/**/
void
execpline2(Pline pline, int how, int input, int output, int last1)
{
    pid_t pid;
    int pipes[2];

    if (breaks || retflag)
	return;

    if (pline->left->lineno >= 0)
	lineno = pline->left->lineno;

    if (pline_level == 1)
	strcpy(list_pipe_text, getjobtext((void *) pline->left));
    if (pline->type == END) {
	execcmd(pline->left, input, output, how, last1 ? 1 : 2);
	pline->left = NULL;
    } else {
	int old_list_pipe = list_pipe;

	mpipe(pipes);

	/* if we are doing "foo | bar" where foo is a current *
	 * shell command, do foo in a subshell and do the     *
	 * rest of the pipeline in the current shell.         */
	if (pline->left->type >= CURSH && (how & Z_SYNC)) {
	    int synch[2];

	    pipe(synch);
	    if ((pid = fork()) == -1) {
		close(synch[0]);
		close(synch[1]);
		zerr("fork failed: %e", NULL, errno);
	    } else if (pid) {
		char dummy, *text;

		text = getjobtext((void *) pline->left);
		addproc(pid, text);
		close(synch[1]);
		read(synch[0], &dummy, 1);
		close(synch[0]);
	    } else {
		zclose(pipes[0]);
		close(synch[0]);
		entersubsh(how, 2, 0);
		close(synch[1]);
		execcmd(pline->left, input, pipes[1], how, 0);
		_exit(lastval);
	    }
	} else {
	/* otherwise just do the pipeline normally. */
	    subsh_close = pipes[0];
	    execcmd(pline->left, input, pipes[1], how, 0);
	}
	pline->left = NULL;
	zclose(pipes[1]);
	if (pline->right) {
	    /* if another execpline() is invoked because the command is *
	     * a list it must know that we're already in a pipeline     */
	    list_pipe = 1;
	    execpline2(pline->right, how, pipes[0], output, last1);
	    list_pipe = old_list_pipe;
	    zclose(pipes[0]);
	    subsh_close = -1;
	}
    }
}

/* make the argv array */

/**/
char **
makecline(LinkList list)
{
    LinkNode node;
    char **argv, **ptr;

    /* A bigger argv is necessary for executing scripts */
    ptr  =
    argv = 2 + (char **) ncalloc((countlinknodes(list) + 4) * sizeof(char *));
    if (isset(XTRACE)) {
	if (!doneps4)
	    fprintf(xtrerr, "%s", (prompt4) ? prompt4 : "");

	for (node = firstnode(list); node; incnode(node)) {
	    *ptr++ = (char *)getdata(node);
	    zputs(getdata(node), xtrerr);
	    if (nextnode(node))
		fputc(' ', xtrerr);
	}
	fputc('\n', xtrerr);
	fflush(xtrerr);
    } else {
	for (node = firstnode(list); node; incnode(node))
	    *ptr++ = (char *)getdata(node);
    }
    *ptr = NULL;
    return (argv);
}

/**/
void
untokenize(char *s)
{
    for (; *s; s++) {
	if (itok(*s)) {
	    if (*s == Nularg)
		chuck(s--);
	    else
		*s = ztokens[*s - Pound];
	}
    }
}

/* Open a file for writing redirection */

/**/
int
clobber_open(struct redir *f)
{
    struct stat buf;
    int fd;

    if (isset(CLOBBER) || IS_CLOBBER_REDIR(f->type))
	return open(unmeta(f->name), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (stat(unmeta(f->name), &buf) || S_ISREG(buf.st_mode))
	return open(unmeta(f->name), O_WRONLY | O_CREAT | O_EXCL, 0666);
    if ((fd = open(unmeta(f->name), O_WRONLY)) < 0)
	return fd;
    if (!fstat(fd, &buf) && !S_ISREG(buf.st_mode))
	return fd;
    /* someone replaced the file between stat and open */
    close(fd);
    errno = EEXIST;
    return -1;
}

/* size of buffer for tee and cat processes */
#define TCBUFSIZE 4092

/* close an multio (success) */

/* WIN32: Microsoft compilers at -O2 use registers agressively. If a local variable is in a
 * register during fork(), it gets lost in the child. There is no cure
 * for this, except to turn off the optimization. -amol 5/19/99
 */
#if defined(_WIN32)
#include <optoff.h>
#endif
/**/
void
closemn(struct multio **mfds, int fd)
{
    struct multio *mn = mfds[fd];
    char buf[TCBUFSIZE];
    int len, i;

    if (fd < 0 || !mfds[fd] || mfds[fd]->ct < 2)
	return;
    if (zfork()) {
	for (i = 0; i < mn->ct; i++)
	    zclose(mn->fds[i]);
	zclose(mn->pipe);
	mn->ct = 1;
	mn->fds[0] = fd;
	return;
    }
    /* pid == 0 */
    closeallelse(mn);
    if (mn->rflag) {
	/* tee process */
	while ((len = read(mn->pipe, buf, TCBUFSIZE)) > 0)
	    for (i = 0; i < mn->ct; i++)
		write(mn->fds[i], buf, len);
    } else {
	/* cat process */
	for (i = 0; i < mn->ct; i++)
	    while ((len = read(mn->fds[i], buf, TCBUFSIZE)) > 0)
		write(mn->pipe, buf, len);
    }
    _exit(0);
}
#if defined(_WIN32)
#include <opton.h>
#endif

/* close all the mnodes (failure) */

/**/
void
closemnodes(struct multio **mfds)
{
    int i, j;

    for (i = 0; i < 10; i++)
	if (mfds[i]) {
	    for (j = 0; j < mfds[i]->ct; j++)
		zclose(mfds[i]->fds[j]);
	    mfds[i] = NULL;
	}
}

/**/
void
closeallelse(struct multio *mn)
{
    int i, j;

    for (i = 0; i < OPEN_MAX; i++)
	if (mn->pipe != i) {
	    for (j = 0; j < mn->ct; j++)
		if (mn->fds[j] == i)
		    break;
	    if (j == mn->ct)
		zclose(i);
	}
}

/* A multio is a list of fds associated with a certain fd.       *
 * Thus if you do "foo >bar >ble", the multio for fd 1 will have *
 * two fds, the result of open("bar",...), and the result of     *
 * open("ble",....).                                             */

/* Add a fd to an multio.  fd1 must be < 10, and may be in any state. *
 * fd2 must be open, and is `consumed' by this function.  Note that   *
 * fd1 == fd2 is possible, and indicates that fd1 was really closed.  *
 * We effectively do `fd2 = movefd(fd2)' at the beginning of this     *
 * function, but in most cases we can avoid an extra dup by delaying  *
 * the movefd: we only >need< to move it if we're actually doing a    *
 * multiple redirection.                                              */

/**/
void
addfd(int forked, int *save, struct multio **mfds, int fd1, int fd2, int rflag)
{
    int pipes[2];

    if (!mfds[fd1] || unset(MULTIOS)) {
	if(!mfds[fd1]) {		/* starting a new multio */
	    mfds[fd1] = (struct multio *) alloc(sizeof(struct multio));
	    if (!forked && save[fd1] == -2)
		save[fd1] = (fd1 == fd2) ? -1 : movefd(fd1);
	}
	redup(fd2, fd1);
	mfds[fd1]->ct = 1;
	mfds[fd1]->fds[0] = fd1;
	mfds[fd1]->rflag = rflag;
    } else {
	if (mfds[fd1]->rflag != rflag) {
	    zerr("file mode mismatch on fd %d", NULL, fd1);
	    return;
	}
	if (mfds[fd1]->ct == 1) {	/* split the stream */
	    mfds[fd1]->fds[0] = movefd(fd1);
	    mfds[fd1]->fds[1] = movefd(fd2);
	    mpipe(pipes);
	    mfds[fd1]->pipe = pipes[1 - rflag];
	    redup(pipes[rflag], fd1);
	    mfds[fd1]->ct = 2;
	} else {		/* add another fd to an already split stream */
	    if(!(mfds[fd1]->ct % MULTIOUNIT)) {
		int new = sizeof(struct multio) + sizeof(int) * mfds[fd1]->ct;
		int old = new - sizeof(int) * MULTIOUNIT;
		mfds[fd1] = hrealloc((char *)mfds[fd1], old, new);
	    }
	    mfds[fd1]->fds[mfds[fd1]->ct++] = movefd(fd2);
	}
    }
    if (subsh_close >= 0 && !fdtable[subsh_close])
	subsh_close = -1;
}

/**/
void
addvars(LinkList l, int export)
{
    Varasg v;
    LinkList vl;
    int xtr;
    char **arr, **ptr;

    xtr = isset(XTRACE);
    if (xtr && nonempty(l)) {
	fprintf(xtrerr, "%s", prompt4 ? prompt4 : "");
	doneps4 = 1;
    }

    while (nonempty(l)) {
	v = (Varasg) ugetnode(l);
	singsub(&v->name);
	if (errflag)
	    return;
	untokenize(v->name);
	if (xtr)
	    fprintf(xtrerr, "%s=", v->name);
	if (v->type == PM_SCALAR) {
	    vl = newlinklist();
	    addlinknode(vl, v->str);
	} else
	    vl = v->arr;
	prefork(vl, v->type == PM_SCALAR ? 7 : 3);
	if (errflag)
	    return;
	if (isset(GLOBASSIGN) || v->type != PM_SCALAR)
	    globlist(vl);
	if (errflag)
	    return;
	if (v->type == PM_SCALAR && (empty(vl) || !nextnode(firstnode(vl)))) {
	    Param pm;
	    char *val;
	    int allexp;

	    if (empty(vl))
		val = ztrdup("");
	    else {
		untokenize(peekfirst(vl));
		val = ztrdup(ugetnode(vl));
	    }
	    if (xtr)
		fprintf(xtrerr, "%s ", val);
	    if (export) {
		if (export < 0)
		    /* We are going to fork so do not bother freeing this */
		    paramtab->removenode(paramtab, v->name);
		if (strcmp(v->name, "STTY") == 0) {
		    zsfree(STTYval);
		    STTYval = ztrdup(val);
		}
		allexp = opts[ALLEXPORT];
		opts[ALLEXPORT] = 1;
		pm = setsparam(v->name, ztrdup(val));
		opts[ALLEXPORT] = allexp;
	    } else
		pm = setsparam(v->name, ztrdup(val));
	    if (errflag)
		return;
	    zsfree(val);
	    continue;
	}
	ptr = arr = (char **) zalloc(sizeof(char **) * (countlinknodes(vl) + 1));

	while (nonempty(vl))
	    *ptr++ = ztrdup((char *) ugetnode(vl));

	*ptr = NULL;
	if (xtr) {
	    fprintf(xtrerr, "( ");
	    for (ptr = arr; *ptr; ptr++)
		fprintf(xtrerr, "%s ", *ptr);
	    fprintf(xtrerr, ") ");
	}
	setaparam(v->name, arr);
	if (errflag)
	    return;
    }
}

#if defined(_WIN32)
#include <optoff.h>
#endif
/**/
void
execcmd(Cmd cmd, int input, int output, int how, int last1)
{
    HashNode hn = NULL;
    LinkNode node;
    Redir fn;
    struct multio *mfds[10];
    char *text;
    int save[10];
    int fil, dfil, is_cursh, type, i;
    int nullexec = 0, assign = 0, forked = 0;
    int is_shfunc = 0, is_builtin = 0, is_exec = 0;
    /* Various flags to the command. */
    int cflags = 0, checked = 0;
    FILE *oxtrerr = xtrerr;

    doneps4 = 0;
    args = cmd->args;
    type = cmd->type;

    for (i = 0; i < 10; i++) {
	save[i] = -2;
	mfds[i] = NULL;
    }

    /* If the command begins with `%', then assume it is a *
     * reference to a job in the job table.                */
    if (type == SIMPLE && nonempty(args) &&
	*(char *)peekfirst(args) == '%') {
	pushnode(args, dupstring((how & Z_DISOWN)
				 ? "disown" : (how & Z_ASYNC) ? "bg" : "fg"));
	how = Z_SYNC;
    }

    /* If AUTORESUME is set, the command is SIMPLE, and doesn't have *
     * any redirections, then check if it matches as a prefix of a   *
     * job currently in the job table.  If it does, then we treat it *
     * as a command to resume this job.                              */
    if (isset(AUTORESUME) && type == SIMPLE && (how & Z_SYNC) &&
	nonempty(args) && empty(cmd->redir) && !input &&
	!nextnode(firstnode(args))) {
	if (unset(NOTIFY))
	    scanjobs();
	if (findjobnam(peekfirst(args)) != -1)
	    pushnode(args, dupstring("fg"));
    }

    /* Check if it's a builtin needing automatic MAGIC_EQUALS_SUBST      *
     * handling.  Things like typeset need this.  We can't detect the    *
     * command if it contains some tokens (e.g. x=ex; ${x}port), so this *
     * only works in simple cases.  has_token() is called to make sure   *
     * this really is a simple case.                                     */
    if (type == SIMPLE) {
	while (nonempty(args)) {
	    char *cmdarg = (char *) peekfirst(args);
	    checked = !has_token(cmdarg);
	    if (!checked)
		break;
	    if (!(cflags & (BINF_BUILTIN | BINF_COMMAND)) &&
		(hn = shfunctab->getnode(shfunctab, cmdarg))) {
		is_shfunc = 1;
		break;
	    }
	    if (!(hn = builtintab->getnode(builtintab, cmdarg))) {
		checked = !(cflags & BINF_BUILTIN);
		break;
	    }
	    if (!(hn->flags & BINF_PREFIX)) {
		is_builtin = 1;
		assign = (hn->flags & BINF_MAGICEQUALS);
		break;
	    }
	    cflags &= ~BINF_BUILTIN & ~BINF_COMMAND;
	    cflags |= hn->flags;
	    uremnode(args, firstnode(args));
	    hn = NULL;
	    checked = 0;
	    if ((cflags & BINF_COMMAND) && unset(POSIXBUILTINS))
		break;
	}
    }

    /* Do prefork substitutions */
    prefork(args, assign ? 2 : isset(MAGICEQUALSUBST));

    if (type == SIMPLE) {
	int unglobbed = 0;

	for (;;) {
	    char *cmdarg;

	    if (!(cflags & BINF_NOGLOB))
		while (!checked && !errflag && nonempty(args) &&
		       has_token((char *) peekfirst(args)))
		    glob(args, firstnode(args));
	    else if (!unglobbed) {
		for (node = firstnode(args); node; incnode(node))
		    untokenize((char *) getdata(node));
		unglobbed = 1;
	    }

	    /* Current shell should not fork unless the *
	     * exec occurs at the end of a pipeline.    */
	    if ((cflags & BINF_EXEC) && last1 == 2)
		cmd->flags |= CFLAG_EXEC;

	    /* Empty command */
	    if (empty(args)) {
		if (nonempty(cmd->redir)) {
		    if (cmd->flags & CFLAG_EXEC) {
			/* Was this "exec < foobar"? */
			nullexec = 1;
			break;
		    } else if (cmd->vars && nonempty(cmd->vars)) {
			nullexec = 2;
			break;
		    } else if (!nullcmd || !*nullcmd ||
			       (cflags & BINF_PREFIX)) {
			zerr("redirection with no command", NULL, 0);
			errflag = lastval = 1;
			return;
		    } else if (readnullcmd && *readnullcmd &&
			       ((Redir) peekfirst(cmd->redir))->type == READ &&
			       !nextnode(firstnode(cmd->redir))) {
			addlinknode(args, dupstring(readnullcmd));
		    } else
			addlinknode(args, dupstring(nullcmd));
		} else if ((cflags & BINF_PREFIX) && (cflags & BINF_COMMAND)) {
		    lastval = 0;
		    return;
		} else {
		    cmdoutval = 0;
		    addvars(cmd->vars, 0);
		    if (errflag)
			lastval = errflag;
		    else
			lastval = cmdoutval;
		    if (isset(XTRACE)) {
			fputc('\n', xtrerr);
			fflush(xtrerr);
		    }
		    return;
		}
	    }

	    if (errflag || checked ||
		(unset(POSIXBUILTINS) && (cflags & BINF_COMMAND)))
		break;

	    cmdarg = (char *) peekfirst(args);
	    if (!(cflags & (BINF_BUILTIN | BINF_COMMAND)) &&
		(hn = shfunctab->getnode(shfunctab, cmdarg))) {
		is_shfunc = 1;
		break;
	    }
	    if (!(hn = builtintab->getnode(builtintab, cmdarg))) {
		if (cflags & BINF_BUILTIN) {
		    zerr("no such builtin: %s", cmdarg, 0);
		    errflag = lastval = 1;
		    return;
		}
		break;
	    }
	    if (!(hn->flags & BINF_PREFIX)) {
		is_builtin = 1;
		break;
	    }
	    cflags &= ~BINF_BUILTIN & ~BINF_COMMAND;
	    cflags |= hn->flags;
	    uremnode(args, firstnode(args));
	    hn = NULL;
	}
    }

    if (errflag) {
	lastval = 1;
	return;
    }

    /* Get the text associated with this command. */
    if (jobbing || (how & Z_TIMED))
	text = getjobtext((void *) cmd);
    else
	text = NULL;

    /* Set up special parameter $_ */
    zsfree(underscore);
    if (nonempty(args)
	&& (underscore = ztrdup((char *) getdata(lastnode(args)))))
	untokenize(underscore); 
    else
  	underscore = ztrdup("");

    /* Warn about "rm *" */
    if (type == SIMPLE && interact && unset(RMSTARSILENT)
	&& isset(SHINSTDIN) && nonempty(args) && nextnode(firstnode(args))
	&& !strcmp(peekfirst(args), "rm")) {
	LinkNode node, next;

	for (node = nextnode(firstnode(args)); node && !errflag; node = next) {
	    char *s = (char *) getdata(node);
	    int l = strlen(s);

	    next = nextnode(node);
	    if (s[0] == Star && !s[1]) {
		if (!checkrmall(pwd))
		    uremnode(args, node);
	    } else if (l > 2 && s[l - 2] == '/' && s[l - 1] == Star) {
		char t = s[l - 2];

		s[l - 2] = 0;
		if (!checkrmall(s))
		    uremnode(args, node);
		s[l - 2] = t;
	    }
	}
	if (!nextnode(firstnode(args)))
	    errflag = 1;
    }

    if (errflag) {
	lastval = 1;
	return;
    }

    if (type == SIMPLE && !nullexec) {
	char *s;

	DPUTS(empty(args), "BUG: empty(args) in exec.c");
	if (!hn) {
	    /* Resolve external commands */
	    char *cmdarg = (char *) peekfirst(args);

	    hn = cmdnamtab->getnode(cmdnamtab, cmdarg);
	    if (!hn && isset(HASHCMDS) && strcmp(cmdarg, "..")) {
		for (s = cmdarg; *s && *s != '/'; s++);
		if (!*s)
		    hn = (HashNode) hashcmd(cmdarg, pathchecked);
	    }
	}

	/* If no command found yet, see if it  *
	 * is a directory we should AUTOCD to. */
	if (!hn && isset(AUTOCD) && isset(SHINSTDIN) && empty(cmd->redir)
	    && !nextnode(firstnode(args)) && *(char *)peekfirst(args)
	    && (s = cancd(peekfirst(args)))) {
	    peekfirst(args) = (void *) s;
	    pushnode(args, dupstring("cd"));
	    if ((hn = builtintab->getnode(builtintab, "cd")))
		is_builtin = 1;
	}
    }

    /* This is nonzero if the command is a current shell procedure? */
    is_cursh = (is_builtin || is_shfunc || (type >= CURSH) || nullexec);

    /**************************************************************************
     * Do we need to fork?  We need to fork if:                               *
     * 1) The command is supposed to run in the background. (or)              *
     * 2) There is no `exec' flag, and either:                                *
     *    a) This is a builtin or shell function with output piped somewhere. *
     *    b) This is an external command and we can't do a `fake exec'.       *
     *                                                                        *
     * A `fake exec' is possible if we have all the following conditions:     *
     * 1) last1 flag is 1.  This indicates that the current shell will not    *
     *    be needed after the current command.  This is typically the case    *
     *    when when the command is the last stage in a subshell, or is the    *
     *    last command after the option `-c'.                                 *
     * 2) We are not trapping EXIT or ZERR.                                   *
     * 3) We don't have any files to delete.                                  *
     *                                                                        *
     * The condition above for a `fake exec' will also work for a current     *
     * shell command such as a builtin, but doesn't really buy us anything    *
     * (doesn't save us a process), since it is already running in the        *
     * current shell.                                                         *
     **************************************************************************/

    if ((how & Z_ASYNC) || (!(cmd->flags & CFLAG_EXEC) &&
       (((is_builtin || is_shfunc) && output) ||
       (!is_cursh && (last1 != 1 || sigtrapped[SIGZERR] ||
        sigtrapped[SIGEXIT] || havefiles()))))) {

	pid_t pid;
	int synch[2];
	char dummy;

	child_block();
	pipe(synch);

	if ((pid = zfork()) == -1) {
	    close(synch[0]);
	    close(synch[1]);
	    return;
	} if (pid) {
	    close(synch[1]);
	    read(synch[0], &dummy, 1);
	    close(synch[0]);
#ifdef PATH_DEV_FD
	    closem(2);
#endif
	    if (how & Z_ASYNC) {
		lastpid = (zlong) pid;
	    } else if (!jobtab[thisjob].stty_in_env && nonempty(cmd->vars)) {
		/* search for STTY=... */
		while (nonempty(cmd->vars))
		    if (!strcmp(((Varasg) ugetnode(cmd->vars))->name, "STTY")) {
			jobtab[thisjob].stty_in_env = 1;
			break;
		    }
	    }
	    addproc(pid, text);
	    return;
	}
	/* pid == 0 */
	close(synch[0]);
	entersubsh(how, type != SUBSH && !(how & Z_ASYNC) ? 2 : 1, 0);
	close(synch[1]);
	forked = 1;
	if (sigtrapped[SIGINT] & ZSIG_IGNORED)
	    holdintr();
	/* Check if we should run background jobs at a lower priority. */
	if ((how & Z_ASYNC) && isset(BGNICE))
	    nice(5);

    } else if (is_cursh) {
	/* This is a current shell procedure that didn't need to fork.    *
	 * This includes current shell procedures that are being exec'ed, *
	 * as well as null execs.                                         */
	jobtab[thisjob].stat |= STAT_CURSH;
    } else {
	/* This is an exec (real or fake) for an external command.    *
	 * Note that any form of exec means that the subshell is fake *
	 * (but we may be in a subshell already).                     */
	is_exec = 1;
    }

    if (!(cflags & BINF_NOGLOB))
	globlist(args);
    if (errflag) {
	lastval = 1;
	goto err;
    }

    /* Make a copy of stderr for xtrace output before redirecting */
    fflush(xtrerr);
    if (isset (XTRACE) && xtrerr == stderr &&
	(type == SIMPLE || type == ZCTIME)) {
	if (!(xtrerr = fdopen(movefd(dup(fileno(stderr))), "w")))
	    xtrerr = stderr;
	else
	    fdtable[fileno(xtrerr)] = 3;
    }

    /* Add pipeline input/output to mnodes */
    if (input)
	addfd(forked, save, mfds, 0, input, 0);
    if (output)
	addfd(forked, save, mfds, 1, output, 1);

    /* Do process substitutions */
    spawnpipes(cmd->redir);

    /* Do io redirections */
    while (nonempty(cmd->redir)) {
	fn = (Redir) ugetnode(cmd->redir);
	DPUTS(fn->type == HEREDOC || fn->type == HEREDOCDASH,
	      "BUG: unexpanded here document");
	if (fn->type == INPIPE) {
	    if (fn->fd2 == -1) {
		closemnodes(mfds);
		fixfds(save);
		execerr();
	    }
	    addfd(forked, save, mfds, fn->fd1, fn->fd2, 0);
	} else if (fn->type == OUTPIPE) {
	    if (fn->fd2 == -1) {
		closemnodes(mfds);
		fixfds(save);
		execerr();
	    }
	    addfd(forked, save, mfds, fn->fd1, fn->fd2, 1);
	} else {
	    if (fn->type != HERESTR && xpandredir(fn, cmd->redir))
		continue;
	    if (errflag) {
		closemnodes(mfds);
		fixfds(save);
		execerr();
	    }
	    if (unset(EXECOPT))
		continue;
	    switch(fn->type) {
	    case HERESTR:
		fil = getherestr(fn);
		if (fil == -1) {
		    closemnodes(mfds);
		    fixfds(save);
		    if (errno != EINTR)
			zerr("%e", NULL, errno);
		    execerr();
		}
		addfd(forked, save, mfds, fn->fd1, fil, 0);
		break;
	    case READ:
	    case READWRITE:
		if (fn->type == READ)
		    fil = open(unmeta(fn->name), O_RDONLY);
		else
		    fil = open(unmeta(fn->name), O_RDWR | O_CREAT, 0666);
		if (fil == -1) {
		    closemnodes(mfds);
		    fixfds(save);
		    if (errno != EINTR)
			zerr("%e: %s", fn->name, errno);
		    execerr();
		}
		addfd(forked, save, mfds, fn->fd1, fil, 0);
		/* If this is 'exec < file', read from stdin, *
		 * not terminal, unless `file' is a terminal. */
		if (nullexec == 1 && fn->fd1 == 0 &&
		    isset(SHINSTDIN) && interact)
		    init_io();
		break;
	    case CLOSE:
		if (!forked && fn->fd1 < 10 && save[fn->fd1] == -2)
		    save[fn->fd1] = movefd(fn->fd1);
		closemn(mfds, fn->fd1);
		zclose(fn->fd1);
		break;
	    case MERGEIN:
	    case MERGEOUT:
		if (fn->fd2 < 10)
		    closemn(mfds, fn->fd2);
		if (fn->fd2 > 9 &&
		    (fdtable[fn->fd2] ||
		     fn->fd2 == coprocin ||
		     fn->fd2 == coprocout)) {
		    fil = -1;
		    errno = EBADF;
		} else {
		    int fd = fn->fd2;
		    if (fd == -2)
			fd = (fn->type == MERGEOUT) ? coprocout : coprocin;
		    fil = dup(fd);
		}
		if (fil == -1) {
		    char fdstr[4];

		    closemnodes(mfds);
		    fixfds(save);
		    if (fn->fd2 != -2)
		    	sprintf(fdstr, "%d", fn->fd2);
		    zerr("%s: %e", fn->fd2 == -2 ? "coprocess" : fdstr, errno);
		    execerr();
		}
		addfd(forked, save, mfds, fn->fd1, fil, fn->type == MERGEOUT);
		break;
	    default:
		if (IS_APPEND_REDIR(fn->type))
		    fil = open(unmeta(fn->name),
			       (unset(CLOBBER) && !IS_CLOBBER_REDIR(fn->type)) ?
			       O_WRONLY | O_APPEND : O_WRONLY | O_APPEND | O_CREAT, 0666);
		else
		    fil = clobber_open(fn);
		if(fil != -1 && IS_ERROR_REDIR(fn->type))
		    dfil = dup(fil);
		else
		    dfil = 0;
		if (fil == -1 || dfil == -1) {
		    if(fil != -1)
			close(fil);
		    closemnodes(mfds);
		    fixfds(save);
		    if (errno != EINTR)
			zerr("%e: %s", fn->name, errno);
		    execerr();
		}
		addfd(forked, save, mfds, fn->fd1, fil, 1);
		if(IS_ERROR_REDIR(fn->type))
		    addfd(forked, save, mfds, 2, dfil, 1);
		break;
	    }
	}
    }

    /* We are done with redirection.  close the mnodes, *
     * spawning tee/cat processes as necessary.         */
    for (i = 0; i < 10; i++)
	closemn(mfds, i);

    if (nullexec) {
	if (nullexec == 1) {
	    /*
	     * If nullexec is 1 we specifically *don't* restore the original
	     * fd's before returning.
	     */
	    for (i = 0; i < 10; i++)
		if (save[i] != -2)
		    zclose(save[i]);
	    goto done;
	}
	/*
	 * If nullexec is 2, we have variables to add with the redirections
	 * in place.
	 */
	if (cmd->vars)
	    addvars(cmd->vars, 0);
	lastval = errflag ? errflag : cmdoutval;
	if (isset(XTRACE)) {
	    fputc('\n', xtrerr);
	    fflush(xtrerr);
	}
    } else if (isset(EXECOPT) && !errflag) {
	/*
	 * We delay the entersubsh() to here when we are exec'ing
	 * the current shell (including a fake exec to run a builtin then
	 * exit) in case there is an error return.
	 */
	if (is_exec)
	    entersubsh(how, type != SUBSH ? 2 : 1, 1);
	if (type >= CURSH) {
	    static int (*func[]) _((Cmd)) = {
		execcursh, exectime, execfuncdef, execfor, execwhile,
		execrepeat, execif, execcase, execselect, execcond,
		execarith
	    };

	    if (last1 == 1)
		cmd->flags |= CFLAG_EXEC;
	    lastval = (func[type - CURSH]) (cmd);
	} else if (is_builtin || is_shfunc) {
	    LinkList restorelist = 0, removelist = 0;
	    /* builtin or shell function */

	    if (!forked && ((cflags & BINF_COMMAND) ||
			    (unset(POSIXBUILTINS) && !assign) ||
			    (isset(POSIXBUILTINS) && !is_shfunc &&
			     !(hn->flags & BINF_PSPECIAL))))
		save_params(cmd, &restorelist, &removelist);

	    if (cmd->vars) {
		/* Export this if the command is a shell function,
		 * but not if it's a builtin.
		 */
		addvars(cmd->vars, is_shfunc);
		if (errflag) {
		    restore_params(restorelist, removelist);
		    lastval = 1;
		    fixfds(save);
		    goto done;
		}
	    }

	    if (is_shfunc) {
		/* It's a shell function */
#ifdef PATH_DEV_FD
		int i;

		for (i = 10; i <= max_zsh_fd; i++)
		    if (fdtable[i] > 1)
			fdtable[i]++;
#endif
		if (subsh_close >= 0)
		    zclose(subsh_close);
		subsh_close = -1;
		execshfunc(cmd, (Shfunc) hn);
#ifdef PATH_DEV_FD
		for (i = 10; i <= max_zsh_fd; i++)
		    if (fdtable[i] > 1)
			if (--(fdtable[i]) <= 2)
			    zclose(i);
#endif
	    } else {
		/* It's a builtin */
		if (forked)
		    closem(1);
		lastval = execbuiltin(args, (Builtin) hn);
#ifdef PATH_DEV_FD
		closem(2);
#endif
		if (isset(PRINTEXITVALUE) && isset(SHINSTDIN) && lastval && !subsh) {
		    fprintf(stderr, "zsh: exit %ld\n", (long)lastval);
		}
		fflush(stdout);
		if (save[1] == -2) {
		    if (ferror(stdout)) {
			zerr("write error: %e", NULL, errno);
			clearerr(stdout);
			errflag = 0;
		    }
		} else
		    clearerr(stdout);
	    }

	    if (cmd->flags & CFLAG_EXEC) {
		if (subsh)
		    _exit(lastval);

		/* If we are exec'ing a command, and we are not in a subshell, *
		 * then check if we should save the history file.              */
		if (isset(RCS) && interact && !nohistsave)
		    savehistfile(getsparam("HISTFILE"), 1, isset(APPENDHISTORY) ? 3 : 0);
		exit(lastval);
	    }

	    restore_params(restorelist, removelist);

	} else {
	    if (cmd->flags & CFLAG_EXEC) {
		setiparam("SHLVL", --shlvl);
		/* If we are exec'ing a command, and we are not *
		 * in a subshell, then save the history file.   */
		if (!subsh && isset(RCS) && interact && !nohistsave)
		    savehistfile(getsparam("HISTFILE"), 1, isset(APPENDHISTORY) ? 3 : 0);
	    }
	    if (type == SIMPLE) {
		if (cmd->vars) {
		    addvars(cmd->vars, -1);
		    if (errflag)
			_exit(1);
		}
		closem(1);
		if (coprocin > 0)
		    zclose(coprocin);
		if (coprocout > 0)
		    zclose(coprocout);
#ifdef HAVE_GETRLIMIT
		if (!forked)
		    setlimits(NULL);
#endif
		if (how & Z_ASYNC) {
		    zsfree(STTYval);
		    STTYval = 0;
		}
		execute((Cmdnam) hn, cflags & BINF_DASH);
	    } else {		/* ( ... ) */
		DPUTS(cmd->vars && nonempty(cmd->vars),
		      "BUG: assigment before complex command.");
		list_pipe = 0;
		if (subsh_close >= 0)
		    zclose(subsh_close);
                subsh_close = -1;
		/* If we're forked (and we should be), no need to return */
		DPUTS(last1 != 1 && !forked, "BUG? not exiting?");
		execlist(cmd->u.list, 0, 1);
	    }
	}
    }

  err:
    if (forked)
	_exit(lastval);
    fixfds(save);

 done:
    if (xtrerr != oxtrerr) {
	fil = fileno(xtrerr);
	fclose(xtrerr);
	xtrerr = oxtrerr;
	zclose(fil);
    }

    zsfree(STTYval);
    STTYval = 0;
}
#if defined(_WIN32)
#include <opton.h>
#endif

/* Arrange to have variables restored. */

/**/
void
save_params(Cmd cmd, LinkList *restore_p, LinkList *remove_p)
{
    Param pm;
    LinkNode node;
    char *s;

    MUSTUSEHEAP("save_params()");

    *restore_p = newlinklist();
    *remove_p = newlinklist();

    for (node = firstnode(cmd->vars); node; incnode(node)) {
	s = ((Varasg) getdata(node))->name;
	if ((pm = (Param) paramtab->getnode(paramtab, s))) {
	    if (!(pm->flags & PM_SPECIAL)) {
		paramtab->removenode(paramtab, s);
	    } else if (!(pm->flags & PM_READONLY)) {
		Param tpm = (Param) alloc(sizeof *tpm);

		tpm->nam = s;
		tpm->flags = pm->flags;
		switch (PM_TYPE(pm->flags)) {
		case PM_SCALAR:
		    tpm->u.str = ztrdup(pm->gets.cfn(pm));
		    break;
		case PM_INTEGER:
		    tpm->u.val = pm->gets.ifn(pm);
		    break;
		case PM_ARRAY:
		    PERMALLOC {
			tpm->u.arr = arrdup(pm->gets.afn(pm));
		    } LASTALLOC;
		    break;
		}
		pm = tpm;
	    }
	    addlinknode(*remove_p, s);
	    addlinknode(*restore_p, pm);
	} else {
	    addlinknode(*remove_p, s);
	}
    }
}

/* Restore saved parameters after executing a shfunc or builtin */

/**/
void
restore_params(LinkList restorelist, LinkList removelist)
{
    Param pm;
    char *s;

    if (removelist) {
	/* remove temporary parameters */
	while ((s = (char *) ugetnode(removelist))) {
	    if ((pm = (Param) paramtab->getnode(paramtab, s)) &&
		!(pm->flags & PM_SPECIAL)) {
		pm->flags &= ~PM_READONLY;
		unsetparam_pm(pm, 0);
	    }
	}
    }

    if (restorelist) {
	/* restore saved parameters */
	while ((pm = (Param) ugetnode(restorelist))) {
	    if (pm->flags & PM_SPECIAL) {
		Param tpm = (Param) paramtab->getnode(paramtab, pm->nam);

		DPUTS(!tpm || PM_TYPE(pm->flags) != PM_TYPE(tpm->flags) ||
		      !(pm->flags & PM_SPECIAL),
		      "BUG in restoring special parameters.");
		tpm->flags = pm->flags;
		switch (PM_TYPE(pm->flags)) {
		case PM_SCALAR:
		    tpm->sets.cfn(tpm, pm->u.str);
		    break;
		case PM_INTEGER:
		    tpm->sets.ifn(tpm, pm->u.val);
		    break;
		case PM_ARRAY:
		    tpm->sets.afn(tpm, pm->u.arr);
		    break;
		}
	    } else
		paramtab->addnode(paramtab, pm->nam, pm);
	    if (pm->flags & PM_EXPORTED)
		pm->env = addenv(pm->nam, getsparam(pm->nam));
	}
    }
}

/* restore fds after redirecting a builtin */

/**/
void
fixfds(int *save)
{
    int old_errno = errno;
    int i;

    for (i = 0; i != 10; i++)
	if (save[i] != -2)
	    redup(save[i], i);
    errno = old_errno;
}

/**/
void
entersubsh(int how, int cl, int fake)
{
    int sig;

    if (cl != 2)
	for (sig = 0; sig < VSIGCOUNT; sig++)
	    if (!(sigtrapped[sig] & ZSIG_FUNC))
		unsettrap(sig);
    if (unset(MONITOR)) {
	if (how & Z_ASYNC) {
	    settrap(SIGINT, NULL);
	    settrap(SIGQUIT, NULL);
	    if (isatty(0)) {
		close(0);
		if (open("/dev/null", O_RDWR)) {
		    zerr("can't open /dev/null: %e", NULL, errno);
		    _exit(1);
		}
	    }
	}
    } else if (thisjob != -1 && cl) {
	if (jobtab[list_pipe_job].gleader && (list_pipe || list_pipe_child)) {
	    if (setpgrp(0L, jobtab[list_pipe_job].gleader) == -1 ||
		killpg(jobtab[list_pipe_job].gleader, 0) == -1) {
		jobtab[list_pipe_job].gleader =
		    jobtab[thisjob].gleader = (list_pipe_child ? mypgrp : getpid());
		setpgrp(0L, jobtab[list_pipe_job].gleader);
		if (how & Z_SYNC)
		    attachtty(jobtab[thisjob].gleader);
	    }
	}
	else if (!jobtab[thisjob].gleader ||
		 setpgrp(0L, jobtab[thisjob].gleader) == -1) {
	    jobtab[thisjob].gleader = getpid();
	    if (list_pipe_job != thisjob &&
		!jobtab[list_pipe_job].gleader)
		jobtab[list_pipe_job].gleader = jobtab[thisjob].gleader;
	    setpgrp(0L, jobtab[thisjob].gleader);
	    if (how & Z_SYNC)
		attachtty(jobtab[thisjob].gleader);
	}
    }
    if (!fake)
	subsh = 1;
    if (SHTTY != -1) {
	shout = NULL;
	zclose(SHTTY);
	SHTTY = -1;
    }
    if (isset(MONITOR)) {
	signal_default(SIGTTOU);
	signal_default(SIGTTIN);
	signal_default(SIGTSTP);
    }
    if (interact) {
	signal_default(SIGTERM);
	if (!(sigtrapped[SIGINT] & ZSIG_IGNORED))
	    signal_default(SIGINT);
    }
    if (!(sigtrapped[SIGQUIT] & ZSIG_IGNORED))
	signal_default(SIGQUIT);
    opts[MONITOR] = opts[USEZLE] = 0;
    zleactive = 0;
    if (cl)
	clearjobtab();
    times(&shtms);
}

/* close internal shell fds */

/**/
void
closem(int how)
{
    int i;

    for (i = 10; i <= max_zsh_fd; i++)
	if (fdtable[i] && (!how || fdtable[i] == how))
	    zclose(i);
}

/* convert here document into a here string */

/**/
char *
gethere(char *str, int typ)
{
    char *buf;
    int bsiz, qt = 0, strip = 0;
    char *s, *t, *bptr, c;

    for (s = str; *s; s++)
	if (INULL(*s)) {
	    *s = Nularg;
	    qt = 1;
	}
    untokenize(str);
    if (typ == HEREDOCDASH) {
	strip = 1;
	while (*str == '\t')
	    str++;
    }
    bptr = buf = zalloc(bsiz = 256);
    for (;;) {
	t = bptr;

	while ((c = hgetc()) == '\t' && strip)
	    ;
	for (;;) {
	    if (bptr == buf + bsiz) {
		buf = realloc(buf, 2 * bsiz);
		t = buf + bsiz - (bptr - t);
		bptr = buf + bsiz;
		bsiz *= 2;
	    }
	    if (lexstop || c == '\n')
		break;
	    *bptr++ = c;
	    c = hgetc();
	}
	*bptr = '\0';
	if (!strcmp(t, str))
	    break;
	if (lexstop) {
	    t = bptr;
	    break;
	}
	*bptr++ = '\n';
    }
    if (t > buf && t[-1] == '\n')
	t--;
    *t = '\0';
    if (!qt) {
	int ef = errflag;

	parsestr(buf);

	if (!errflag)
	    errflag = ef;
    }
    s = dupstring(buf);
    zfree(buf, bsiz);
    return s;
}

/* open here string fd */

/**/
int
getherestr(struct redir *fn)
{
    char *s, *t;
    int fd, len;

    t = fn->name;
    singsub(&t);
    untokenize(t);
    unmetafy(t, &len);
    t[len++] = '\n';
    s = gettempname();
    if (!s || (fd = open(s, O_CREAT | O_WRONLY | O_EXCL, 0600)) == -1)
	return -1;
    write(fd, t, len);
    close(fd);
#if defined(_WIN32)
    fd = open(s, O_RDONLY | O_TEMPORARY);
#else
    fd = open(s, O_RDONLY);
#endif
    unlink(s);
    return fd;
}

/* $(...) */

#if defined(_WIN32)
#include <optoff.h>
#endif
/**/
LinkList
getoutput(char *cmd, int qt)
{
    List list;
    int pipes[2];
    pid_t pid;
    Cmd c;
    Redir r;

    if (!(list = parse_string(cmd, 0)))
	return NULL;
    if (list != &dummy_list && !list->right && !list->left->flags &&
	list->left->type == END && list->left->left->type == END &&
	(c = list->left->left->left)->type == SIMPLE && empty(c->args) &&
	empty(c->vars) && nonempty(c->redir) &&
	!nextnode(firstnode(c->redir)) &&
	(r = (Redir) getdata(firstnode(c->redir)))->fd1 == 0 &&
	r->type == READ) {
	/* $(< word) */
	int stream;
	char *s = r->name;

	singsub(&s);
	if (errflag)
	    return NULL;
	untokenize(s);
	if ((stream = open(unmeta(s), O_RDONLY)) == -1) {
	    zerr("%e: %s", s, errno);
	    return NULL;
	}
	return readoutput(stream, qt);
    }

    mpipe(pipes);
    child_block();
    cmdoutval = 0;
    if ((cmdoutpid = pid = zfork()) == -1) {
	/* fork error */
	zclose(pipes[0]);
	zclose(pipes[1]);
	errflag = 1;
	cmdoutpid = 0;
	child_unblock();
	return NULL;
    } else if (pid) {
	LinkList retval;

	zclose(pipes[1]);
	retval = readoutput(pipes[0], qt);
	fdtable[pipes[0]] = 0;
	waitforpid(pid);		/* unblocks */
	lastval = cmdoutval;
	return retval;
    }

    /* pid == 0 */
    child_unblock();
    zclose(pipes[0]);
    redup(pipes[1], 1);
    opts[MONITOR] = 0;
    entersubsh(Z_SYNC, 1, 0);
    execlist(list, 0, 1);
    close(1);
    _exit(lastval);
    zerr("exit returned in child!!", NULL, 0);
    kill(getpid(), SIGKILL);
    return NULL;
}
#if defined(_WIN32)
#include <opton.h>
#endif

/* read output of command substitution */

/**/
LinkList
readoutput(int in, int qt)
{
    LinkList ret;
    char *buf, *ptr;
    int bsiz, c, cnt = 0;
    FILE *fin;

    fin = fdopen(in, "r");
    ret = newlinklist();
    ptr = buf = (char *) ncalloc(bsiz = 64);
    while ((c = fgetc(fin)) != EOF || errno == EINTR) {
	if (c == EOF) {
	    errno = 0;
	    clearerr(fin);
	    continue;
	}
	if (imeta(c)) {
	    *ptr++ = Meta;
	    c ^= 32;
	    cnt++;
	}
	if (++cnt >= bsiz) {
	    char *pp = (char *) ncalloc(bsiz *= 2);

	    memcpy(pp, buf, cnt - 1);
	    ptr = (buf = pp) + cnt - 1;
	}
	*ptr++ = c;
    }
    fclose(fin);
    while (cnt && ptr[-1] == '\n')
	ptr--, cnt--;
    *ptr = '\0';
    if (qt) {
	if (!cnt) {
	    *ptr++ = Nularg;
	    *ptr = '\0';
	}
	addlinknode(ret, buf);
    } else {
	char **words = spacesplit(buf, 0);

	while (*words) {
	    if (isset(GLOBSUBST))
		tokenize(*words);
	    addlinknode(ret, *words++);
	}
    }
    return ret;
}

/**/
List
parsecmd(char *cmd)
{
    char *str;
    List list;

    for (str = cmd + 2; *str && *str != Outpar; str++);
    if (!*str || cmd[1] != Inpar) {
	zerr("oops.", NULL, 0);
	return NULL;
    }
    *str = '\0';
    if (str[1] || !(list = parse_string(cmd + 2, 0))) {
	zerr("parse error in process substitution", NULL, 0);
	return NULL;
    }
    return list;
}

/* =(...) */

#if defined(_WIN32)
#include <optoff.h>
#endif
/**/
char *
getoutputfile(char *cmd)
{
    pid_t pid;
    char *nam;
    List list;
    int fd;

    if (thisjob == -1)
	return NULL;
    if (!(list = parsecmd(cmd)))
	return NULL;
    if (!(nam = gettempname()))
	return NULL;

    nam = ztrdup(nam);
    PERMALLOC {
	if (!jobtab[thisjob].filelist)
	    jobtab[thisjob].filelist = newlinklist();
	addlinknode(jobtab[thisjob].filelist, nam);
    } LASTALLOC;
    child_block();
    fd = open(nam, O_WRONLY | O_CREAT | O_EXCL, 0600); /* create the file */

    if (fd < 0 || (cmdoutpid = pid = zfork()) == -1) {
	/* fork or open error */
	child_unblock();
	return nam;
    } else if (pid) {
	int os;

	close(fd);
	os = jobtab[thisjob].stat;
	waitforpid(pid);
	cmdoutval = 0;
	jobtab[thisjob].stat = os;
	return nam;
    }

    /* pid == 0 */
    redup(fd, 1);
    opts[MONITOR] = 0;
    entersubsh(Z_SYNC, 1, 0);
    execlist(list, 0, 1);
    close(1);
    _exit(lastval);
    zerr("exit returned in child!!", NULL, 0);
    kill(getpid(), SIGKILL);
    return NULL;
}
/* _WIN32: continuing with optimizations off */

#if !defined(PATH_DEV_FD) && defined(HAVE_FIFOS)
/* get a temporary named pipe */

/**/
char *
namedpipe(void)
{
    char *tnam = gettempname();

# ifdef HAVE_MKFIFO
    if (mkfifo(tnam, 0600) < 0)
# else
    if (mknod(tnam, 0010600, 0) < 0)
# endif
	return NULL;
    return tnam;
}
#endif /* ! PATH_DEV_FD && HAVE_FIFOS */

/* <(...) or >(...) */

/**/
char *
getproc(char *cmd)
{
#if !defined(HAVE_FIFOS) && !defined(PATH_DEV_FD)
    zerr("doesn't look like your system supports FIFOs.", NULL, 0);
    return NULL;
#else
    List list;
    int out = *cmd == Inang;
    char *pnam;
#ifndef PATH_DEV_FD
    int fd;
#else
    int pipes[2];
#endif

    if (thisjob == -1)
	return NULL;
#ifndef PATH_DEV_FD
    if (!(pnam = namedpipe()))
	return NULL;
#else
    pnam = ncalloc(strlen(PATH_DEV_FD) + 6);
#endif
    if (!(list = parsecmd(cmd)))
	return NULL;
#ifndef PATH_DEV_FD
    PERMALLOC {
	if (!jobtab[thisjob].filelist)
	    jobtab[thisjob].filelist = newlinklist();
	addlinknode(jobtab[thisjob].filelist, ztrdup(pnam));
    } LASTALLOC;
    if (zfork()) {
#else
    mpipe(pipes);
    if (zfork()) {
	sprintf(pnam, "%s/%d", PATH_DEV_FD, pipes[!out]);
	zclose(pipes[out]);
	fdtable[pipes[!out]] = 2;
#endif
	return pnam;
    }
#ifndef PATH_DEV_FD
    closem(0);
    fd = open(pnam, out ? O_WRONLY : O_RDONLY);
    if (fd == -1) {
	zerr("can't open %s: %e", pnam, errno);
	_exit(1);
    }
    entersubsh(Z_ASYNC, 1, 0);
    redup(fd, out);
#else
    entersubsh(Z_ASYNC, 1, 0);
    redup(pipes[out], out);
    closem(0);   /* this closes pipes[!out] as well */
#endif
    execlist(list, 0, 1);
    zclose(out);
    _exit(lastval);
    return NULL;
#endif   /* HAVE_FIFOS and PATH_DEV_FD not defined */
}

/* > >(...) or < <(...) (does not use named pipes) */

/**/
int
getpipe(char *cmd)
{
    List list;
    int pipes[2], out = *cmd == Inang;

    if (!(list = parsecmd(cmd)))
	return -1;
    mpipe(pipes);
    if (zfork()) {
	zclose(pipes[out]);
	return pipes[!out];
    }
    entersubsh(Z_ASYNC, 1, 0);
    redup(pipes[out], out);
    closem(0);	/* this closes pipes[!out] as well */
    execlist(list, 0, 1);
    _exit(lastval);
    return 0;
}
#if defined(_WIN32)
#include <opton.h>
#endif

/* open pipes with fds >= 10 */

/**/
void
mpipe(int *pp)
{
    pipe(pp);
    pp[0] = movefd(pp[0]);
    pp[1] = movefd(pp[1]);
}

/* Do process substitution with redirection */

/**/
void
spawnpipes(LinkList l)
{
    LinkNode n;
    Redir f;
    char *str;

    n = firstnode(l);
    for (; n; incnode(n)) {
	f = (Redir) getdata(n);
	if (f->type == OUTPIPE || f->type == INPIPE) {
	    str = f->name;
	    f->fd2 = getpipe(str);
	}
    }
}

/* evaluate a [[ ... ]] */

/**/
int
execcond(Cmd cmd)
{
    return !evalcond(cmd->u.cond);
}

/* evaluate a ((...)) arithmetic command */

/**/
int
execarith(Cmd cmd)
{
    char *e;
    zlong val = 0;

    while ((e = (char *) ugetnode(cmd->args)))
	val = matheval(e);
    errflag = 0;
    return !val;
}

/* perform time ... command */

/**/
int
exectime(Cmd cmd)
{
    int jb;

    jb = thisjob;
    if (!cmd->u.pline) {
	shelltime();
	return 0;
    }
    execpline(cmd->u.pline, Z_TIMED|Z_SYNC, 0);
    thisjob = jb;
    return lastval;
}

/* Define a shell function */

/**/
int
execfuncdef(Cmd cmd)
{
    Shfunc shf;
    char *s;
    int signum;

    PERMALLOC {
	while ((s = (char *) ugetnode(cmd->args))) {
	    shf = (Shfunc) zalloc(sizeof *shf);
	    shf->funcdef = (List) dupstruct(cmd->u.list);
	    shf->flags = 0;

	    /* is this shell function a signal trap? */
	    if (!strncmp(s, "TRAP", 4) && (signum = getsignum(s + 4)) != -1) {
		if (settrap(signum, shf->funcdef)) {
		    freestruct(shf->funcdef);
		    zfree(shf, sizeof *shf);
		    LASTALLOC_RETURN 1;
		}
		sigtrapped[signum] |= ZSIG_FUNC;
	    }
	    shfunctab->addnode(shfunctab, ztrdup(s), shf);
	}
    } LASTALLOC;
    return 0;
}

/* Main entry point to execute a shell function.  It will retrieve *
 * an autoloaded shell function if it is currently undefined.      */

/**/
void
execshfunc(Cmd cmd, Shfunc shf)
{
    List funcdef;
    char *nam = (char *) peekfirst(cmd->args);
    char *oldscriptname = scriptname;
    LinkList last_file_list = NULL;

    if (errflag)
	return;

    if (!list_pipe && thisjob != list_pipe_job) {
	/* Without this deletejob the process table *
	 * would be filled by a recursive function. */
	last_file_list = jobtab[thisjob].filelist;
	jobtab[thisjob].filelist = NULL;
	deletejob(jobtab + thisjob);
    }

    scriptname = nam;

    /* Are we dealing with an autoloaded shell function? */
    if (shf->flags & PM_UNDEFINED) {
	++locallevel;
	funcdef = getfpfunc(nam);
	--locallevel;
	if (!funcdef) {
	    scriptname = oldscriptname;
	    zerr("function not found: %s", nam, 0);
	    lastval = 1;
	} else {
	    PERMALLOC {
		shf->flags &= ~PM_UNDEFINED;
		funcdef = shf->funcdef = (List) dupstruct(funcdef);
	    } LASTALLOC;

	    /* Execute the function definition, we just retrived */
	    doshfunc(shf->funcdef, cmd->args, shf->flags, 0);

	    /* See if this file defined the autoloaded function *
	     * by name.  If so, we execute it again.            */
	    if ((shf = (Shfunc) shfunctab->getnode(shfunctab, nam))
		&& shf->funcdef && shf->funcdef != funcdef)
		doshfunc(shf->funcdef, cmd->args, shf->flags, 0);
	}
    } else
	/* Normal shell function execution */
	doshfunc(shf->funcdef, cmd->args, shf->flags, 0);
    if (!list_pipe)
	deletefilelist(last_file_list);

    scriptname = oldscriptname;
}

/* execute a shell function */

/**/
void
doshfunc(List list, LinkList doshargs, int flags, int noreturnval)
/* If noreturnval is nonzero, then reset the current return *
 * value (lastval) to its value before the shell function   *
 * was executed.                                            */
{
    Param pm;
    char **tab, **x, *oargv0 = NULL;
    int xexittr, newexittr, oldzoptind, oldlastval;
    LinkList olist;
    char *s, *ou;
    void *xexitfn, *newexitfn;
    char saveopts[OPT_SIZE];
    int obreaks = breaks;

    HEAPALLOC {
	pushheap();
	if (trapreturn < 0)
	    trapreturn--;
	oldlastval = lastval;
	xexittr = sigtrapped[SIGEXIT];
	if (xexittr & ZSIG_FUNC)
	    xexitfn = shfunctab->removenode(shfunctab, "TRAPEXIT");
	else
	    xexitfn = sigfuncs[SIGEXIT];
	sigtrapped[SIGEXIT] = 0;
	sigfuncs[SIGEXIT] = NULL;
	tab = pparams;
	oldzoptind = zoptind;
	zoptind = 1;

	/* We need to save the current options even if LOCALOPTIONS is *
	 * not currently set.  That's because if it gets set in the    *
	 * function we need to restore the original options on exit.   */
	memcpy(saveopts, opts, sizeof(opts));

	if (flags & PM_TAGGED)
	    opts[XTRACE] = 1;
	opts[PRINTEXITVALUE] = 0;
	if (doshargs) {
	    LinkNode node;

	    node = doshargs->first;
	    pparams = x = (char **) zcalloc(((sizeof *x) * (1 + countlinknodes(doshargs))));
	    if (isset(FUNCTIONARGZERO)) {
		oargv0 = argzero;
		argzero = ztrdup((char *) node->dat);
	    }
	    node = node->next;
	    for (; node; node = node->next, x++)
		*x = ztrdup((char *) node->dat);
	} else {
	    pparams = (char **) zcalloc(sizeof *pparams);
	    if (isset(FUNCTIONARGZERO)) {
		oargv0 = argzero;
		argzero = ztrdup(argzero);
	    }
	}
	PERMALLOC {
	    olist = locallist;		/* save the old locallist since shell functions may be nested */
	    locallist = newlinklist();	/* make a new list of local variables that we have to destroy */
	} LASTALLOC;
	locallevel++;
	ou = underscore;
	underscore = ztrdup(underscore);
	execlist(dupstruct(list), 1, 0);
	zsfree(underscore);
	underscore = ou;
	locallevel--;

	/* destroy the local variables we have created in the shell function */
	while ((s = (char *) getlinknode(locallist))) {
	    if((pm = (Param) paramtab->getnode(paramtab, s)) && (pm->level > locallevel))
		unsetparam_pm(pm, 0);
	    zsfree(s);
	}
	zfree(locallist, sizeof(struct linklist));

	locallist = olist;	/* restore the old list of local variables */
	if (retflag) {
	    retflag = 0;
	    breaks = obreaks;
	}
	freearray(pparams);
	if (oargv0) {
	    zsfree(argzero);
	    argzero = oargv0;
	}
	zoptind = oldzoptind;
	pparams = tab;

	if (isset(LOCALOPTIONS)) {
	    /* restore all shell options except PRIVILEGED */
	    saveopts[PRIVILEGED] = opts[PRIVILEGED];
	    memcpy(opts, saveopts, sizeof(opts));
	} else {
	    /* just restore a couple. */
	    opts[XTRACE] = saveopts[XTRACE];
	    opts[PRINTEXITVALUE] = saveopts[PRINTEXITVALUE];
	    opts[LOCALOPTIONS] = saveopts[LOCALOPTIONS];
	}

	/*
	 * The trap '...' EXIT runs in the environment of the caller,
	 * so remember it here but run it after resetting the
	 * traps for the parent.
	 */
	newexittr = sigtrapped[SIGEXIT];
	newexitfn = sigfuncs[SIGEXIT];
	if (newexittr & ZSIG_FUNC)
	    shfunctab->removenode(shfunctab, "TRAPEXIT");

	sigtrapped[SIGEXIT] = xexittr;
	if (xexittr & ZSIG_FUNC) {
	    shfunctab->addnode(shfunctab, ztrdup("TRAPEXIT"), xexitfn);
	    sigfuncs[SIGEXIT] = ((Shfunc) xexitfn)->funcdef;
	} else
	    sigfuncs[SIGEXIT] = (List) xexitfn;

	if (newexitfn) {
	    dotrapargs(SIGEXIT, &newexittr, newexitfn);
	    freestruct(newexitfn);
	}

	if (trapreturn < -1)
	    trapreturn++;
	if (noreturnval)
	    lastval = oldlastval;
	popheap();
    } LASTALLOC;
}

/* Search fpath for an undefined function. */

/**/
List
getfpfunc(char *s)
{
    char **pp, buf[PATH_MAX];
    off_t len;
    char *d;
    List r;
    int fd;

    pp = fpath;
    for (; *pp; pp++) {
	if (strlen(*pp) + strlen(s) + 1 >= PATH_MAX)
	    continue;
	if (**pp)
	    sprintf(buf, "%s/%s", *pp, s);
	else
	    strcpy(buf, s);
	unmetafy(buf, NULL);
	if (!access(buf, R_OK) && (fd = open(buf, O_RDONLY)) != -1) {
	    if ((len = lseek(fd, 0, 2)) != -1) {
		lseek(fd, 0, 0);
		d = (char *) zcalloc(len + 1);
		if (read(fd, d, len) == len) {
		    close(fd);
		    d = metafy(d, len, META_REALLOC);
		    HEAPALLOC {
			r = parse_string(d, 1);
		    } LASTALLOC;
		    zfree(d, len + 1);
		    return r;
		} else {
		    zfree(d, len + 1);
		    close(fd);
		}
	    } else {
		close(fd);
	    }
	}
    }
    return NULL;
}

/* check to see if AUTOCD applies here */

extern int doprintdir;

/**/
char *
cancd(char *s)
{
    int nocdpath = s[0] == '.' &&
    (s[1] == '/' || !s[1] || (s[1] == '.' && (s[2] == '/' || !s[1])));
    char *t;

    if (*s != '/') {
	char sbuf[PATH_MAX], **cp;

	if (cancd2(s))
	    return s;
	if (access(unmeta(s), X_OK) == 0)
	    return NULL;
	if (!nocdpath)
	    for (cp = cdpath; *cp; cp++) {
		if (strlen(*cp) + strlen(s) + 1 >= PATH_MAX)
		    continue;
		if (**cp)
		    sprintf(sbuf, "%s/%s", *cp, s);
		else
		    strcpy(sbuf, s);
		if (cancd2(sbuf)) {
		    doprintdir = -1;
		    return dupstring(sbuf);
		}
	    }
	if ((t = cd_able_vars(s))) {
	    if (cancd2(t)) {
		doprintdir = -1;
		return t;
	    }
	}
	return NULL;
    }
    return cancd2(s) ? s : NULL;
}

/**/
int
cancd2(char *s)
{
    struct stat buf;
    char *us = unmeta(s);

    return !(access(us, X_OK) || stat(us, &buf) || !S_ISDIR(buf.st_mode));
}

/**/
void
execsave(void)
{
    struct execstack *es;

    es = (struct execstack *) malloc(sizeof(struct execstack));
    es->args = args;
    es->list_pipe_pid = list_pipe_pid;
    es->nowait = nowait;
    es->pline_level = pline_level;
    es->list_pipe_child = list_pipe_child;
    es->list_pipe_job = list_pipe_job;
    strcpy(es->list_pipe_text, list_pipe_text);
    es->lastval = lastval;
    es->noeval = noeval;
    es->badcshglob = badcshglob;
    es->cmdoutpid = cmdoutpid;
    es->cmdoutval = cmdoutval;
    es->trapreturn = trapreturn;
    es->noerrs = noerrs;
    es->subsh_close = subsh_close;
    es->underscore = underscore;
    underscore = ztrdup(underscore);
    es->next = exstack;
    exstack = es;
    noerrs = cmdoutpid = 0;
}

/**/
void
execrestore(void)
{
    struct execstack *en;

    DPUTS(!exstack, "BUG: execrestore() without execsave()");
    args = exstack->args;
    list_pipe_pid = exstack->list_pipe_pid;
    nowait = exstack->nowait;
    pline_level = exstack->pline_level;
    list_pipe_child = exstack->list_pipe_child;
    list_pipe_job = exstack->list_pipe_job;
    strcpy(list_pipe_text, exstack->list_pipe_text);
    lastval = exstack->lastval;
    noeval = exstack->noeval;
    badcshglob = exstack->badcshglob;
    cmdoutpid = exstack->cmdoutpid;
    cmdoutval = exstack->cmdoutval;
    trapreturn = exstack->trapreturn;
    noerrs = exstack->noerrs;
    subsh_close = exstack->subsh_close;
    zsfree(underscore);
    underscore = exstack->underscore;
    en = exstack->next;
    free(exstack);
    exstack = en;
}
