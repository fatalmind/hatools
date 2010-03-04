#!/bin/sh

/bin/kill -l |tr '\n' ' '|awk 'BEGIN{RS=" "; print "#include <signal.h>\n#include <stdlib.h>\n\n\nconst char *sys_signame2[] =\n{"} !/EXIT/&&!/NULL/&&!/^RT/&&/^[A-Za-z0-9]+$/{print "\t\"" toupper($1) "\" ,";} END {print "\tNULL\n};\n\n"}' > sys_signame.h
/bin/kill -l |tr '\n' ' '|awk 'BEGIN{RS=" "; print "const int sys_signumber2[] =\n{"} !/EXIT/&&!/NULL/&&!/^RT/&&/^[A-Za-z0-9]+$/{print "\tSIG" toupper($1) " ,";} END {print "\t0\n};\n\n"}' >> sys_signame.h
