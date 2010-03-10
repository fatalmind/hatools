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
#include <ctype.h>
#include "hatimerun.h"

const char NONE[] = "NONE";

#ifndef HAVE_SYS_SIGNAME

#ifdef HATOOLS_SYS_SIGNAME_H
#include "sys_signame.h"
#else
#include "sys_signame_posix.h"
#endif



int signumber(const char *signame) {
	int signum = -1;
	int i = 0;

	if (! strncasecmp(signame, "SIG", 3)) {
		signame += 3;
	}

	if (! strncasecmp(signame, NONE, 4)) {
		return 0;
	}
	
	while (signum < 0 && sys_signame2[i] != NULL) {
		if (! strcasecmp(signame, sys_signame2[i])) {
			signum = sys_signumber2[i];
		}
		i++;
	}
	return signum;	
}

#else   /* ifdef HAVE_SYS_SIGNAME */

#include <signal.h>
int signumber(const char *signame) {
	int i;
	int signum = -1;
	
	if (! strncasecmp(signame, "SIG", 3)) {
		signame += 3;
	}

	if (! strncasecmp(signame, NONE, 4)) {
		return 0;
	}

	for (i = 0; (i < NSIG) && (signum < 0); i++) {
		if (! strcasecmp(signame, sys_signame[i])) {
			signum = i;
		}
	}
	return signum;
}

#endif


#ifndef HAVE_SYS_SIGNAME
#define NUMBER_OF_SIGNALS sys_nsigname2
#else
#define NUMBER_OF_SIGNALS NSIG
#endif

void get_sig_nameUC(const int signal, char* dest, const int mxlength) {
	if (signal == 0) {
		strncpy(dest, NONE, mxlength);	
	} else if  (signal >= NUMBER_OF_SIGNALS || signal < 0) {
		int rv;
		
		rv = snprintf(dest, mxlength, "%d", signal);
		if ((rv < 0 || rv >= mxlength) && (mxlength > 0)) {
			dest[0] = 0;
		}
		return;
	} else {

#ifndef HAVE_SYS_SIGNAME
		/* Offset: -1, because sys_signame2 starts with signal "1" (HUP) at index 0 */
		const char* const signame = sys_signame2[signal - 1];
#else
		const char* const signame = sys_signame[signal];
#endif
		if (strlen(signame) + 3 < mxlength) {
			int i = 0;
			strcpy(dest, "SIG");
			while (signame[i] != 0) {
				dest[i+3] = toupper(signame[i]);
				++i;	
			}
			dest[i+3] = 0;
		} else {
			/* leave it unchanged */	
		}
	}
}

#define MX_LENGTH 64
void siglist() {
	int i;
	char signameUC[MX_LENGTH+1];
	int end = NUMBER_OF_SIGNALS;

	for (i = 1; i < end; i++) {
		get_sig_nameUC(i, signameUC, MX_LENGTH);		
		fputs(signameUC, stderr);
		fputc('\n', stderr);
	}
	exit (FAIL_EXITCODE[0]);
}

