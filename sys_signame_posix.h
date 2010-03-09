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


