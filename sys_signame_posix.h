/*********************************************************************
 *** sys_signame_posix.h
 *** Copyright (c)  2001,2003,2005-2009 by Markus Winand <mws@fatalmind.com>
 *** $Id$
 ********************************************************************/

#include <signal.h>
#include <stdlib.h>


const char *sys_signame2[] =
{
	"ABRT" ,
	"ALRM" ,
	"FPE" ,
	"HUP" ,
	"ILL" ,
	"INT" ,
	"KILL" ,
	"PIPE" ,
	"QUIT" ,
	"SEGV" ,
	"TERM" ,
	"USR1" ,
	"USR2" ,
	
	"CHLD" ,
	"CONT" ,
	"STOP" ,
	"TSTP" ,
	"TTIN" ,
	"TTOU" ,
	NULL
};


const int sys_signumber2[] =
{
	SIGABRT ,
	SIGALRM ,
	SIGFPE ,
	SIGHUP ,
	SIGILL ,
	SIGINT ,
	SIGKILL ,
	SIGPIPE ,
	SIGQUIT ,
	SIGSEGV ,
	SIGTERM ,
	SIGUSR1 ,
	SIGUSR2 ,
	
	SIGCHLD ,
	SIGCONT ,
	SIGSTOP ,
	SIGTSTP ,
	SIGTTIN ,
	SIGTTOU ,
	0
};


