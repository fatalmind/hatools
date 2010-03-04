/*********************************************************************
 *** printsignal.c
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

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

void mygettimeofday(struct timeval *tv) {
	if (gettimeofday(tv, NULL)) {
		printf("gettimeofday: errno: %d\n", errno);
		abort();
	}
}

struct timeval starttime;

void printsignal_sighand(int no) {
	/* directly calling an "unsafe" fuction, however, i know */
	/* this process isn't in any library function except sleep */
	/* right now... */
	
	struct timeval nowtime;
        double diff;
	
	mygettimeofday(&nowtime);	
	diff  = nowtime.tv_sec + (((double)nowtime.tv_usec)/1000000);
	diff -= starttime.tv_sec + (((double)starttime.tv_usec)/1000000);

	printf("Caught signal %d after %8.0f (%8.3f) seconds\n", no, diff, diff);
}


int main(int argc, char *argv[]) {
	struct sigaction alrt;
	int hlp;
	char dummy;
	struct timespec slp;
	
	if (argc != 2) {
		printf("you need to specify the number of seconds to sleep\n");
		exit(1);
	}

	if (sscanf(argv[1], "%d%c", &hlp, &dummy) != 1) {
		printf("Specified argument is not numeric\n");
		exit(1);
	}
	if (hlp <= 0) {
		printf("Specified argument has to be a positive integer\n");
		exit(1);
	}
	
	mygettimeofday(&starttime);

	printf("This program is intended for testing hatimerun.\n");
	printf("Waiting %d seconds and printing all signals received\n", hlp);

	alrt.sa_handler = printsignal_sighand;
	sigemptyset(&alrt.sa_mask);
	alrt.sa_flags = 0;

	sigaction(SIGTERM, &alrt, NULL);
	sigaction(SIGHUP,  &alrt, NULL);
	sigaction(SIGINT,  &alrt, NULL);
	sigaction(SIGPIPE, &alrt, NULL);
	sigaction(SIGQUIT, &alrt, NULL);
	sigaction(SIGUSR1, &alrt, NULL);
	sigaction(SIGUSR2, &alrt, NULL);
	sigaction(SIGALRM, &alrt, NULL);

	/* This is a very strange program, it is not intended to exit... */
	slp.tv_sec  = hlp;
	slp.tv_nsec = 0;
	while (nanosleep(&slp, &slp) == -1 && errno == EINTR) {
	}
	return 0;
}
