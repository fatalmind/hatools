bin_PROGRAMS		= halockrun hatimerun
noinst_PROGRAMS         = printsignal
halockrun_SOURCES	= halockrun.c halockrun.h error.h error.c config.h
hatimerun_SOURCES	= hatimerun.c hatimerun.h signumber.c signumber.h sys_signame_posix.h error.h error.c config.h
printsignal_SOURCES     = printsignal.c
printsignal_LDADD       = ${MAYBE_LIBPOSIX4}

man_MANS	= halockrun.1 hatimerun.1
EXTRA_DIST	= $(man_MANS) mk_siglist.sh test.sh

TESTS = test.sh
