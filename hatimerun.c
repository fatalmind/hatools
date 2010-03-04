/*********************************************************************
 *** hatimerun.c
 *** $Id$
 ********************************************************************/
/*
HA-Tools (c) 2001,2003,2005-2009 by Markus Winand <mws@fatalmind.com>

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
*/


#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "hatimerun.h"
#include "signumber.h"
#include "error.h"

/* default signal to be sent if -k was not specified */
#define _FAIL_SIGNAL  SIGKILL
/* defines the maximum pair number of -k -t pairs */
#define MAXARGSIZE  16

extern char* optarg;
extern int optind;

int debug = 1;
char *cmdname = NULL;
int async = 0;

/* _idx variables are the indexes used during parsing, */
/* as well as during alarming. */
int term_signal[MAXARGSIZE];
int timeout[MAXARGSIZE];
int FAIL_EXITCODE[MAXARGSIZE];

volatile int timeouthappend_idx = 0;

void hatimerun_sighand(int a) {
	timeouthappend_idx++;
}

void initialize_arrays() {
	int i;
	for (i=0; i < MAXARGSIZE; ++i) {
		term_signal[i]   = _FAIL_SIGNAL;;
		timeout[i]       = 0;
		FAIL_EXITCODE[i] = _FAIL_EXITCODE;
	}
}

int mk_child(char *argv[], int wpip)
{
	pid_t childpid;
	int childcode = 0;

	childpid = fork();
	switch (childpid) {
	case -1: /* fail*/
		error("fork() failed: %s\n", strerror(errno));
		break;
	case 0: /* fork() succeeded*/
		/* kiddie*/

		/* set into own new process group*/
		setpgid(0,0);

		execvp(argv[0], argv);
		
		if (async) {
			/* let the (waiting) parent know */
			char buf = 'X';
			write(wpip, &buf, 1);
			close(wpip);
		}

		/* exec failed*/
		error("exec(%s) failed: %s\n", argv[0], strerror(errno));
		/* never returns*/
		break;
	default: /*parent*/
		/*   
		 *
		 * Set the signalhandlers for parent to ignore signals
		 * 
		 */
		{ /* new block allows local variables ;) */
		int ret;
		int childstatus;
		struct sigaction ign,alrt;
		int timeoutsprocessed_idx = timeouthappend_idx;

		ign.sa_handler = SIG_IGN;
		sigemptyset(&ign.sa_mask);
		ign.sa_flags = 0;

		sigaction(SIGTERM, &ign, NULL);
		sigaction(SIGHUP,  &ign, NULL);
		sigaction(SIGINT,  &ign, NULL);
		sigaction(SIGPIPE, &ign, NULL);
		sigaction(SIGQUIT, &ign, NULL);
		sigaction(SIGUSR1, &ign, NULL);
		sigaction(SIGUSR2, &ign, NULL);

		alrt.sa_handler = hatimerun_sighand;
		sigemptyset(&alrt.sa_mask);
		alrt.sa_flags = 0;

		sigaction(SIGALRM, &alrt,NULL);
		alarm(timeout[timeouthappend_idx]);

		do {
			ret = waitpid(childpid, &childstatus, 0);
			if (ret == -1 && errno == EINTR && timeouthappend_idx > timeoutsprocessed_idx) {
				/* kill it*/
				kill(-childpid, term_signal[timeoutsprocessed_idx]); /* happend -1 since it is not yet processed */
				childcode = FAIL_EXITCODE[timeoutsprocessed_idx];
				if (timeout[timeouthappend_idx] > 0) {
					alarm(timeout[timeouthappend_idx]);
				}
				timeoutsprocessed_idx = timeouthappend_idx;
			}
		} while (ret == -1 && errno == EINTR );

		if ((timeouthappend_idx == 0) && WIFEXITED(childstatus)) {
			childcode = WEXITSTATUS(childstatus);
		} /* else? already set above */

		} /* block for local vairables; */
		break;
	};
	return (childcode);
}

/*
 *
 * Signal Handler
 *
 */

void nop(int i) {
	return;
}

void usage() {
	fprintf(stderr, "usage: %s [-a] [-e exitcode] [-k signame] " 
		"-t secs command [args]\n", cmdname);
	exit (FAIL_EXITCODE[0]);
}

void longusage() {
	fprintf(stderr, 
"usage: %s [-a] [-e exitcode] [-k signame] -t [[hh:]mm:]secs command [args]\n"
"       %s [-l|-h|-?]\n"
"Options:\n"
"  -a           Async mode. Starts %s in the background\n"
"  -e exitcode  Changes the exitcode returned by %s on fail\n", cmdname, cmdname, cmdname, cmdname);
	fprintf(stderr, 
"  -k signame   Specifies the signal witch will be sent to the process group\n"
"               if a timeout occures\n"
"  -t time      Specifies the timeout.\n"
"               You can specify the number of seconds or hours/minutes/seconds\n"
"               notation seperated by colon (e.g. 1:30:0 means 1 hour, 30 minutes)\n"
"  -l           Print list of available signals on this platform and exit.\n");
	fprintf(stderr, 
"Version:\n"
"  V%s\n" 
"  Copyright (c) 2001,2003,2005-2009 by Markus Winand <mws@fatalmind.com>\n"
"  This program is distributed in the hope that it will be useful,\n"
"  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
"  GNU General Public License for more details.\n\n", VERSION);

	exit (FAIL_EXITCODE[0]) ;
}


int parse_timespec(char* arg) {
	int h=0, m=0, s=0, rv = 0;
	char dummy;
	if (sscanf(optarg, "%d%c", &s, &dummy) != 1) {
		/* not a pure numeric input, maybe colon seperated? */
		if (sscanf(optarg, "%d:%d%c", &m, &s, &dummy) != 2) {
			if (sscanf(optarg, "%d:%d:%d%c", &h, &m, &s, &dummy) != 3) {
				error("illegal option \"-t %s\"\n", optarg);
			}
		}
	}
	if (s < 0 || m < 0 || h < 0) {
		error("illegal option \"-t %s\"\n", optarg);
		exit (FAIL_EXITCODE[0]);
	}
	
	rv = (h*60 + m) * 60 + s;
	if (rv <= 0) {
		error("illegal option \"-t %s\"\n", optarg);
		exit (FAIL_EXITCODE[0]);
	}
	return rv;
}

/*
 *
 * Command line parsing
 *
 */

char **cmd_line(int argc, char *argv[]) {
	char **childargv = NULL;
	int childargc = argc - 1;
	int hlp;
	char dummy;
	int ch;

	int timeout_idx = 0;
	int term_signal_idx = 0;
	int FAIL_EXITCODE_idx = 0;

#ifdef HAVE_SETENV
	char* env_POSIXLY_CORRECT;
	env_POSIXLY_CORRECT = getenv("POSIXLY_CORRECT");
	setenv("POSIXLY_CORRECT", "1", 0);
#endif

	while ((ch = getopt(argc, argv, "ae:k:t:lh?")) != -1) {
		switch(ch) {
		case 'a':
			async = 1;
			break;
		case 'e': /* exitcode */
			if (FAIL_EXITCODE_idx >= MAXARGSIZE) {
				error("limit of -e occurances reached, set MAXARGSIZE higher and recompile (current limit: %d)", MAXARGSIZE);
			}
			if (sscanf(optarg, "%d%c", &hlp, &dummy) != 1) {
				error("illegal option \"-e %s\"\n", optarg);
			}		
			FAIL_EXITCODE[FAIL_EXITCODE_idx++] = hlp;
			break;
			
		case 'k': /* signame */
			if (term_signal_idx >= MAXARGSIZE) {
				error("limit of -k occurances reached, set MAXARGSIZE higher and recompile (current limit: %d)", MAXARGSIZE);
			}
			if (sscanf(optarg,"%d%c", &hlp, &dummy) == 1) {
				term_signal[term_signal_idx++] = hlp;
			} else {
				/* check for signal name*/
				hlp = signumber(optarg);
				if (hlp == 0) {
					error("unknown signal \"%s\"\n",optarg);
				}
				term_signal[term_signal_idx++] = hlp;
			}
			break;
		case 't':
			if (timeout_idx >= MAXARGSIZE) {
				error("limit of -t occurances reached, set MAXARGSIZE higher and recompile (current limit: %d)", MAXARGSIZE);
			}
			hlp = parse_timespec(optarg);
			timeout[timeout_idx++] = hlp;
			break;
		case 'l':
			siglist();
			break;
		case '?':
		case 'h':
			longusage();
			break;
		}
	}
#ifdef HAVE_SETENV
	if (env_POSIXLY_CORRECT == NULL) {
		unsetenv("POSIXLY_CORRECT");
	}
#endif

	childargc = argc - optind;
	childargv = argv + optind;


	if (timeout_idx == 0 || childargc < 1) {
		usage();
	}

	/* null terminated argv for execvp */
	childargv[childargc] = NULL;

	return childargv;
}	
/*
 *
 * Main
 *
 */

int main(int argc, char *argv[]) {
	char **childargv = NULL;
	int child_return = 99;
	struct sigaction act;
	pid_t async_fork = 0;
	int pip[2] = {0, 0}; /* used for error checking in async mode */

	initialize_arrays();

	cmdname = argv[0];

	if (argc < 2) {
		usage();
	}
	
	/*
	 *
	 * Command line parsing
	 *
	 */

	childargv = cmd_line(argc, argv);

	/*
	 *
	 * fork() for async mode
	 *
	 */

	if (async) {
		/* use a pipe to communicate successfuly exec via close on exec flag */
		int flags;
		if (pipe(pip) != 0) {
			error("pipe() failed: %s\n", strerror(errno));
		}

		/* close on exec for the write end */
		flags = fcntl(pip[0], F_GETFD);
		flags |= FD_CLOEXEC;
		fcntl(pip[0], F_SETFD, flags);

		flags = fcntl(pip[1], F_GETFD);
		flags |= FD_CLOEXEC;
		fcntl(pip[1], F_SETFD, flags);

		async_fork = fork();
		if (async_fork == -1) {
			/* fail*/
			error("fork() failed: %s\n", strerror(errno));
		}
		if (async_fork > 0) {
			/* daddie waits for pipe */
			char buffer = ' ';
			close(pip[1]); /* all write-end's must be close in order to get EOF on read */
			if (read(pip[0], &buffer, 1) > 0) {
				/* if there was something to read */
				/* an error occured during execvp in the child */
				exit(FAIL_EXITCODE[0]);
			}

			/* and dies */
			exit(0);
		}
		/* async child, closes readpipe */
		close(pip[0]);
	}

	/*
	 *
	 * Set a nop signalhandler for SIGCHLD, needed to interrupt the sleep
	 *
	 */
	
	act.sa_handler = nop;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &act, NULL) != 0) {
		error("sigaction() failed: %s\n", strerror(errno));
	}

	/*
	 *
	 * do the work
	 *
	 */

	child_return = mk_child(childargv, pip[1]);

	exit (child_return);
}


