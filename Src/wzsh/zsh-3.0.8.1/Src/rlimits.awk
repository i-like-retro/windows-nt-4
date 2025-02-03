# 
# rlimits.awk: {g,n}awk script to generate rlimits.h
# rewritten by Peter Stephenson <pws@ifh.de> from Geoff Wing
# <mason@werple.apana.org.au>'s signames.awk
# NB: On SunOS 4.1.3 - user-functions don't work properly, also \" problems
# Without 0 + hacks some nawks compare numbers as strings
#
BEGIN {limidx = 0}

/^[\t ]*(#[\t ]*define[\t _]*RLIMIT_[A-Z_]*[\t ]*[0-9][0-9]*|RLIMIT_[A-Z_]*,[\t ]*|RLIMIT_[A-Z_]*[\t ]*=[\t ]*[0-9][0-9]*,[\t ]*)/ {

    limindex = index($0, "RLIMIT_")
    limtail = substr($0, limindex, 80)
    split(limtail, tmp)
    limnam = substr(tmp[1], 8, 20)
    limnum = tmp[2]
    # in this case I assume GNU libc resourcebits.h
    if (limnum == "") {
	limnum = limidx++
	sub (",", "", limnam)
    }
    if (limnum == "=") {
	limnum = limidx++
	sub (",", "", limnam)
    }

    limrev[limnam] = limnum
    if (lim[limnum] == "") {
	lim[limnum] = limnam
	if (limnum ~ /^[0-9]*$/) {
	    if (limnam == "MEMLOCK") { msg[limnum] = "memorylocked" }
	    if (limnam == "RSS")     { msg[limnum] = "resident" }
	    if (limnam == "VMEM")    { msg[limnum] = "vmemorysize" }
	    if (limnam == "NOFILE")  { msg[limnum] = "descriptors" }
	    if (limnam == "OFILE")   { msg[limnum] = "descriptors" }
	    if (limnam == "CORE")    { msg[limnum] = "coredumpsize" }
	    if (limnam == "STACK")   { msg[limnum] = "stacksize" }
	    if (limnam == "DATA")    { msg[limnum] = "datasize" }
	    if (limnam == "FSIZE")   { msg[limnum] = "filesize" }
	    if (limnam == "CPU")     { msg[limnum] = "cputime" }
	    if (limnam == "NPROC")   { msg[limnum] = "maxproc" }
	    if (limnam == "AS")      { msg[limnum] = "addressspace" }
	    if (limnam == "TCACHE")  { msg[limnum] = "cachedthreads" }
	    if (limnam == "AIO_OPS") { msg[limnum] = "aiooperations" }
	    if (limnam == "AIO_MEM") { msg[limnum] = "aiomemorylocked" }
	    if (limnam == "SBSIZE")  { msg[limnum] = "sockbufsize" }
	    if (limnam == "PTHREAD") { msg[limnum] = "maxpthreads" }
	    if (limnam == "LOCKS") { msg[limnum] = "maxfilelocks" }
        }
    }
}
/^[\t ]*#[\t ]*define[\t _]*RLIM_NLIMITS[\t ]*[0-9][0-9]*/ {
    limindex = index($0, "RLIM_")
    limtail = substr($0, limindex, 80)
    split(limtail, tmp)
    nlimits = tmp[2]
}
# in case of GNU libc
/^[\t ]*RLIM_NLIMITS[\t ]*=[\t ]*RLIMIT_NLIMITS/ {
    if(!nlimits) { nlimits = limidx }
}

/^[\t ]*RLIMIT_NLIMITS[\t ]*=[\t ]*[0-9][0-9]*/ {
    limindex = index($0, "=")
    limtail = substr($0, limindex, 80)
    split(limtail, tmp)
    nlimits = tmp[2]
}

END {
    if (limrev["MEMLOCK"] != "") {
        irss = limrev["RSS"]
        msg[irss] = "memoryuse"
    }
    ps = "%s"

    printf("%s\n%s\n\n", "/** rlimits.h                                 **/", "/** architecture-customized limits for zsh **/")
    printf("#define ZSH_NLIMITS %d\n\nstatic char *recs[ZSH_NLIMITS+1] = {\n", 0 + nlimits)

    for (i = 0; i < 0 + nlimits; i++)
	if (msg[i] == "") {
            badlimit++
            printf("\t%c%s%c,\n", 34, lim[i], 34)
	} else
	    printf("\t%c%s%c,\n", 34, msg[i], 34)
    print "\tNULL"
    print "};"
    print ""
    exit(badlimit)
}
