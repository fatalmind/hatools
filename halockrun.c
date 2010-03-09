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

#include "halockrun.h"
#include "error.h"

extern char* optarg;
extern int optind;

int FAIL_EXITCODE = _FAIL_EXITCODE;
int FAIL_EXITCODE_NB = _FAIL_EXITCODE;
char *cmdname = NULL;

/* configuration */
int async = 0;

void notifyAsyncParent(int wpip, char val) {
	if (async) {
		/* let the (waiting) parent know */
		char buf = val;
		write(wpip, &buf, 1);
		close(wpip);
	}
}

/*
 *
 * fork and exec
 *
 */

int mk_child(int fork_mode, int wpip, char *argv[])
{
	int childstatus = 0;
	pid_t childpid = 0;
	struct sigaction act;
	int ret;

	if (fork_mode) {
		childpid = fork();
	}
	switch (childpid) {
	case -1: /* fail */
		error("fork() failed: %s\n", strerror(errno));
		break;
	case 0: /* fork() succeeded */
		/* kiddie */

		execvp(argv[0], argv);

		notifyAsyncParent(wpip, FAIL_EXITCODE);

		/* exec failed */
		error("exec(%s) failed: %s\n", argv[0], strerror(errno));
		/* never returns */
		break;
	default: /* parent */
		/*
		 *
		 * Set the signalhandlers for parent to ignore signals 
		 * in fork_mode
		 *
		 */
		act.sa_handler = SIG_IGN;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;

		sigaction(SIGTERM, &act, NULL);
		sigaction(SIGHUP,  &act, NULL);
		sigaction(SIGINT,  &act, NULL);
		sigaction(SIGPIPE, &act, NULL);
		sigaction(SIGQUIT, &act, NULL);
		sigaction(SIGUSR1, &act, NULL);
		sigaction(SIGUSR2, &act, NULL);

		do {
			ret = waitpid(childpid, &childstatus, 0);
		} while (ret == -1 && errno == EINTR);

		break;
	};
	if (WIFEXITED(childstatus)) {
		return WEXITSTATUS(childstatus);
	} else {
		return FAIL_EXITCODE;
	}
}


/*
 *
 * print usage and exit
 *
 */

void usage() {
	fprintf(stderr, "usage: %s [-ancf] [-e exitcode] [-E nexitcode] "
			"lockfile command [args]\n", cmdname);
	fprintf(stderr, "       %s -t|-N [-nc] [-e exitcode] [-E nexitcode] "
			"lockfile\n", cmdname);
	fprintf(stderr, "       %s -h|-?\n", cmdname);
	exit(FAIL_EXITCODE);
}

void longusage() {
	fprintf(stderr, 
"usage: %s [-ancf] [-e exitcode] [-E nexitcode] lockfile command [args]\n"
"       %s -t|-N [-nc] [-e exitcode] [-E nexitcode] lockfile\n"
"       %s -h|-?\n"
"Options:\n",
cmdname, cmdname, cmdname);
	fprintf(stderr,
"  -a            Async mode. Starts %s in the background\n"
"  -e exitcode   Changes the exitcode returned by %s if an error occured.\n"
"  -E nexitcode  Changes the exitcode returned by %s in case the lock could not\n"
"                obtained. Defaults to the values of -e if omitted.\n", cmdname, cmdname, cmdname);

	fprintf(stderr, 
"  -n            Nonblocking. Exits immediately if the lock cannot be obtained\n"
"  -c            Creates the lockfile if it doesn't exist\n"
"  -f            Fork mode. Runs %s and the command in differnt processes\n"
"  -t            Test mode. %s will check the lockfile and report the process\n"
"                who holds the lock if it's locked\n"
"  -N            Noop mode. Doesn't execute any command.\n\n",
cmdname, cmdname);
        fprintf(stderr,   
"Version:\n"
"  V%s\n"
"  Copyright (c) 2001,2003,2005-2010 by Markus Winand <mws@fatalmind.com>\n"
"  This program is distributed in the hope that it will be useful,\n"
"  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
"  GNU General Public License for more details.\n\n", VERSION);

	exit(FAIL_EXITCODE);
}

/*
 * global config parameters
 */

int fork_mode = 0;
int test_mode = 0;
int noop_mode = 0;
int openmode = O_WRONLY;
int lockmode = F_SETLKW;
char *lockfile = NULL;

/*
 * 
 * command line parsing
 *
 */

char **cmd_line(int argc, char *argv[]) {
	char **childargv = NULL;
	int childargc = argc-1;
	int hlp;
	int ch;
	char dummy;

	/* was argument -e or -E specified on the cmd line? */
	int argE = 0;
	int arge = 0;

#ifdef HAVE_SETENV
	char* env_POSIXLY_CORRECT;
	env_POSIXLY_CORRECT = getenv("POSIXLY_CORRECT");
	setenv("POSIXLY_CORRECT", "1", 0);
#endif

	while ((ch = getopt(argc,argv, "ance:E:h?ftN")) != -1) {
		switch (ch) {
		case 'a':
			async = 1;
			break;
		case 'n':
			lockmode = F_SETLK;
			break;
		case 't':
			test_mode = 1;	
			break;
		case 'N':
			noop_mode = 1;	
			break;
		case 'c':
			openmode |= O_CREAT;
			break;
		case 'e':
			if (sscanf(optarg, "%d%c", &hlp, &dummy) != 1) {
				error("illegal option \"-e %s\"\n",
						optarg);
			}
			FAIL_EXITCODE = hlp;
			arge = 1;
			break;
		case 'E':
			if (sscanf(optarg, "%d%c", &hlp, &dummy) != 1) {
				error("illegal option \"-E %s\"\n",
						optarg);
			}
			FAIL_EXITCODE_NB = hlp;
			argE = 1;
			break;
		case 'h':
		case '?':
			longusage();
			break;
		case 'f':
			fork_mode = 1;
			break;
		default:
			usage();
			exit(FAIL_EXITCODE);
		};
	}
#ifdef HAVE_SETENV
	if (env_POSIXLY_CORRECT == NULL) {
		unsetenv("POSIXLY_CORRECT");
	}
#endif
 		
	childargc = argc - optind;
	childargv = argv + optind;

	if ((test_mode || noop_mode)&& (childargc != 1)) {
		printf("illegal number of arguments (command not allowed if -t or -N is used)\n");
		usage();
	}
	if (! test_mode && ! noop_mode && childargc < 2) {
		printf("illegal number of arguments (command missing?)\n");
		usage();
	}

	/* if -e was specified but not -E */
	if (arge && ! argE) {
		/* we use the -e setting also for -E (compatibility) */
		FAIL_EXITCODE_NB = FAIL_EXITCODE;
	}
	if ((test_mode || noop_mode) && ! argE) {
		/* the default nexitcode in test mode */
		FAIL_EXITCODE_NB = 1;
	}
	lockfile = childargv[0];
	childargv++;
	childargc--;

	childargv[childargc] = NULL;

	if (test_mode) {
		/* override -afn flags if -t was specified */
		async = 0;
		fork_mode = 0;
		lockmode = F_GETLK;
	}
	if (noop_mode) {
		/* override -af flags if -N was specified */
		async = 0;
		fork_mode = 0;
	}

	if (test_mode && noop_mode) {
		printf("illegal arguments (-t and -N can't be combined)\n");
		usage();
	}

	return childargv;
}

void async_fork(int pip[2]) {
	/* use a pipe to communicate successfuly exec via close on exec flag */
	int flags;
	pid_t async_fork;
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
		/* fail */
		error("fork() failed: %s\n", strerror(errno));
	}

	if (async_fork > 0 ) {
		/* daddie waits for pipe */
		char buffer = '\0';
		close(pip[1]); /* all write-end's must be close in order to get EOF on read */
		if (read(pip[0], &buffer, 1) > 0) {
			/* if there was something to read */
			/* an error occured during execvp in the child */
			if (buffer == FAIL_EXITCODE || buffer == FAIL_EXITCODE_NB) {
				exit(buffer);
			}
			exit(FAIL_EXITCODE);
		}
	
		/* and dies */
		exit(0);
	}
	/* async child, closes readpipe */
	close(pip[0]);
}

/*
 *
 * main
 *
 */

int main(int argc, char *argv[]) {
	char **childargv = NULL;
	int child_return = 99;
	int lockfd;
	struct flock flock_struct;
	int lockfd_flags;
	int pip[2] = {0, 0}; /* used for error checking in async mode */

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
	 * Open the file
	 *
	 */

	lockfd = open(lockfile, openmode, 0600);

	if (lockfd == -1) {
		error("Can't open lockfile \"%s\": %s\n",
			lockfile, strerror(errno));
	}

	/*
	 *
	 * Set the Close on Exec flag
	 *
	 */
	

	if (fork_mode) {	
		lockfd_flags = fcntl(lockfd, F_GETFD);
		lockfd_flags |= FD_CLOEXEC;
		fcntl(lockfd, F_SETFD, lockfd_flags);
	}

	/*
	 *
	 * fork() for async mode
	 *
	 */

	if (async) {
		async_fork(pip);
	}

	/*
	 *
	 * get the exclusive lock
	 *
	 */

	flock_struct.l_type = F_WRLCK;
	flock_struct.l_whence = SEEK_SET;
	flock_struct.l_start = 0;
	flock_struct.l_len = 0;

	if (fcntl(lockfd, lockmode, &flock_struct) == -1) {
		/* POSIX allows eigther EAGAIN or EACCES to say it's locked 
		  if the file is really not accessible, the open call above
		  has already failed with EACCES */
		if ((lockmode == F_SETLK) && (errno == EAGAIN || errno == EACCES)) {
			/* nonblocking failed */
			notifyAsyncParent(pip[1], FAIL_EXITCODE_NB);
			exit(FAIL_EXITCODE_NB);
		}
		if (lockmode == F_GETLK) {
			/* test mode */
			error("Can't get lock information on \"%s\": %s\n",
				lockfile, strerror(errno));
		}
		error("Can't get exclusive lock on \"%s\": %s\n",
			lockfile, strerror(errno));
	}

	/*
	 *
	 * do the work (execute the command)
	 *
	 */

	if (! test_mode && ! noop_mode) {
		child_return = mk_child(fork_mode, pip[1], childargv);
	} else if (test_mode) {
		/* test mode */
		if (flock_struct.l_type == F_UNLCK) {
			child_return = 0;
		} else {
			printf("%ld\n", (long)flock_struct.l_pid);
			child_return = FAIL_EXITCODE_NB;
		}
	} else {
		/* noop_mode, just return ok */
		child_return = 0;
	}

	if (! test_mode) {
		/*
		 *
		 * unlock
		 *
		 */

		flock_struct.l_type = F_WRLCK;
		flock_struct.l_whence = SEEK_SET;
		flock_struct.l_start = 0;
		flock_struct.l_len = 0;
		fcntl(lockfd, F_SETLKW, &flock_struct); /* errors?? -> bad luck */
	}

	/*
	 *
	 * close the lockfile
	 *
	 */

	close(lockfd);	

	exit(child_return);
}

