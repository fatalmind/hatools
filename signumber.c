/*********************************************************************
 *** signumber.c
 *** Copyright (c) 2001,2003,2005-2009 by Markus Winand <mws@fatalmind.com>
 *** $Id$
 ********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "hatimerun.h"

#ifndef HAVE_SYS_SIGNAME

#ifdef HATOOLS_SYS_SIGNAME_H
#include "sys_signame.h"
#else
#include "sys_signame_posix.h"
#endif


int signumber(const char *signame) {
	int signum = 0;
	int i = 0;

	if (! strncasecmp(signame, "SIG", 3)) {
		signame += 3;
	}
	
	while (signum == 0 && sys_signame2[i] != NULL) {
		if (! strcasecmp(signame, sys_signame2[i])) {
			signum = sys_signumber2[i];
		}
		i++;
	}
	return signum;	
}

void siglist() {
	int i;

	for (i = 0; sys_signame2[i] != NULL; i++) {
		fprintf(stderr, "SIG%s\n", sys_signame2[i]);
	}
	exit (FAIL_EXITCODE[0]);
}
#else

#include <signal.h>
#include <ctype.h>
int signumber(const char *signame) {
	int i;
	int signum = 0;
	
	if (! strncasecmp(signame, "SIG", 3)) {
		signame += 3;
	}

	for (i = 0; (i < NSIG) && (signum == 0); i++) {
		if (! strcasecmp(signame, sys_signame[i])) {
			signum = i;
		}
	}
	return signum;
}

void sig_printUC(const char *signame) {
	int i = 0;

	fprintf(stderr, "SIG");
	while (signame[i] != 0) {
		fputc(toupper(signame[i]), stderr);
		i++;
	}
	fputc('\n', stderr);
	
}

void siglist() {
	int i;

	for (i = 0; i < NSIG; i++) {
		sig_printUC(sys_signame[i]);
	}
	exit (FAIL_EXITCODE[0]);
};

#endif
