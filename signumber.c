/*********************************************************************
hatools (c) 2001,2003,2005-2010 by Markus Winand <mws@fatalmind.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*********************************************************************/


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
