#! /bin/ksh

# this script can be used to test the package
# anyway it is still very incomplete, feel free to improve
# (and submit your improvements)

SHELL=/bin/ksh
# fun enough, bash v2.05 @ OS 10.4 (at least)
# manipulates POSIXLY_CORRECT: in case it it set, it is reset to "y".
# this makes test 11 and co failing.

LOCKFILE1=/tmp/halockrun.1

(./halockrun -c ${LOCKFILE1} sleep 1000; rm -r ${LOCKFILE1}) 2>/dev/null&

sleep 1
if [ ! -f ${LOCKFILE1} ]; then
	echo "fail 1: lockfile not created"
	exit 1
fi
echo "test  1 ok"


./halockrun -t ${LOCKFILE1} >/dev/null
RS=$?

if [ $RS != 1 ]; then
	echo "fail 2: lockfile not locked (using -t), RS=$RS"
	exit 2
fi
echo "test  2 ok"

./halockrun -nN ${LOCKFILE1}
RS=$?

if [ $RS != 1 ]; then
	echo "fail 3: lockfile not locked (using -nN), RS=$RS"
	exit 3
fi
echo "test  3 ok"


RV=`./halockrun -t ${LOCKFILE1}`
E=$?

if [ -z "${RV}" ]; then
	echo "fail 4: returned nothing on locked file"
	exit 4
fi
echo "test  4 ok"

if [ ${RV} == 0 ]; then
	echo "fail 5: returned zero on locked file"
	exit 5
fi
echo "test  5 ok"

if [ $E -ne 1 ]; then
	echo "fail 6: returned $E on locked file (expected 1)"
	exit 6
fi
echo "test  6 ok"

./halockrun -t -E 2 ${LOCKFILE1} >/dev/null
E=$?

if [ $E -ne 2 ]; then
	echo "fail 7: returned $E on locked file (expected 2)"
	exit 7
fi
echo "test  7 ok"

./halockrun -nN -E 2 ${LOCKFILE1}
E=$?

if [ $E -ne 2 ]; then
	echo "fail 8: returned $E on locked file (expected 2)"
	exit 8 
fi
echo "test  8 ok"

kill `./halockrun -t ${LOCKFILE1}` >/dev/null

sleep 1

if [ -f ${LOCKFILE1} ]; then
	echo "fail 9: lockfile not removed"
	exit 9
fi
echo "test  9 ok"

touch ${LOCKFILE1}

unset POSIXLY_CORRECT
RV=`./halockrun -c ${LOCKFILE1} ${SHELL} -c 'env | grep POSIXLY_CORRECT'`
if [ -n "$RV" ]; then
	echo "fail 10: environment manipulation (${RV})"
	exit 10;
fi
echo "test 10 ok"

RV=`POSIXLY_CORRECT="VERY_CORRECT" ./halockrun -c ${LOCKFILE1} ${SHELL} -c 'echo ">${POSIXLY_CORRECT}<"'`
if [ "$RV" != ">VERY_CORRECT<" ]; then
	echo "fail 11: environment manipulation (POSIXLY_CORRECT=${RV})"
	exit 11;
fi
echo "test 11 ok"




./halockrun -t ${LOCKFILE1}
RS=$?

if [ $RS != 0 ]; then
	echo "fail 12: not locked but return code is ${RS}"
	exit 12
fi
echo "test 12 ok"

./halockrun -nN ${LOCKFILE1}
RS=$?

if [ $RS != 0 ]; then
	echo "fail 13: not locked but return code is ${RS}"
	exit 13
fi
echo "test 13 ok"


RV=`./halockrun -t ${LOCKFILE1}`

if [ ! -z "${RV}" ]; then
	echo "fail 14: not locked but stdout ist ${RV}"
	exit 14 
fi
echo "test 14 ok"

rm -f ${LOCKFILE1}


./halockrun -t -E 2 ${LOCKFILE1} 2>/dev/null
E=$?

if [ $E -ne 99 ]; then
	echo "fail 15: returned $E on error (expected 99)"
	exit 15
fi
echo "test 15 ok"

./halockrun -nN -E 2 ${LOCKFILE1} 2>/dev/null
E=$?

if [ $E -ne 99 ]; then
	echo "fail 16: returned $E on error (expected 99)"
	exit 16
fi
echo "test 16 ok"


./halockrun -t -e 3 -E 2 ${LOCKFILE1} 2>/dev/null
E=$?

if [ $E -ne 3 ]; then
	echo "fail 17: returned $E on error (expected 3)"
	exit 17
fi
echo "test 17 ok"


./halockrun -Nn -e 3 -E 2 ${LOCKFILE1} 2>/dev/null
E=$?

if [ $E -ne 3 ]; then
	echo "fail 18: returned $E on error (expected 3)"
	exit 18
fi
echo "test 18 ok"


#####
# test -a for bad commands

./halockrun -c ${LOCKFILE1} /command/does/not/exist 2>/dev/null
E=$?

if [ $E -ne 99 ]; then
	echo "fail 19: returned $E on error (expected 99)"
	exit 19
fi
echo "test 19 ok"


./halockrun -ca ${LOCKFILE1} /command/does/not/exist
E=$?

if [ $E -ne 99 ]; then
	echo "fail 20: returned $E on error (expected 99)"
	exit 20
fi
echo "test 20 ok"


#####
# testing hatimerun

./hatimerun -k INT -t 1 sleep 10
E=$?
if [ $E -ne 99 ]; then
	echo "fail 21: returned $E on error (expected 99)"
	exit 21
fi
echo "test 21 ok"

./hatimerun -vv -e 99 -e 100 -k INT -t 1 -t 2 ./printsignal 2 >/dev/null
E=$?
if [ $E -ne 99 ]; then
	echo "fail 22: returned $E on error (expected 99)"
	exit 22 
fi
echo "test 22 ok"

./hatimerun -e 99 -e 100 -k INT -t 1 -t 2 ./printsignal 10 >/dev/null
E=$?
if [ $E -ne 100 ]; then
	echo "fail 23: returned $E on error (expected 99)"
	exit 23
fi
echo "test 23 ok"

./halockrun -ancvv ${LOCKFILE1} ./printsignal 10 >/dev/null
./halockrun -t ${LOCKFILE1} >/dev/null
RS=$?

if [ $RS != 1 ]; then
	echo "fail 24: returned $RS on error (!= 1)"
	exit 24
fi
echo "test 24 ok"

echo "The next test takes a minute, please be patient"

./hatimerun -e 99 -e 100 -k INT -t 1:0 -k TERM -t 2 ./printsignal 90 > ${LOCKFILE1}
E=$?
if [ $E -ne 100 ]; then
	echo "fail 25: returned $E on error (expected 99)"
	exit 25
fi
echo "test 25 ok"

while read blah1 blah2 sig blah3 sec moreblah; do
	if [ "x$sig" == "xis" -o "x$sig" == "xseconds" ]; then
		/bin/true
	else
		if [ "$sec" -ge 59 -a "$sec" -le 63 ]; then
			/bin/true
		else
			echo "fail 26: time mismatch ($sec)"
			exit 26
		fi;
	fi;
done < ${LOCKFILE1}

echo "test 26 ok"


RV=`./hatimerun -t 10 ${SHELL} -c 'env | grep POSIXLY_CORRECT'`
if [ -n "$RV" ]; then
	echo "fail 27: environment manipulation (${RV})"
	exit 27;
fi
echo "test 27 ok"

RV=`POSIXLY_CORRECT="VERY_CORRECT" ./hatimerun -t 10 ${SHELL} -c 'echo ">${POSIXLY_CORRECT}<"'`
if [ "$RV" != ">VERY_CORRECT<" ]; then
	echo "fail 28: environment manipulation (POSIXLY_CORRECT=${RV})"
	exit 28;
fi
echo "test 28 ok"


./hatimerun -l 2>/dev/null
E=$?
if [ $E -ne 99 ]; then
	echo "fail 29: hatimerun -l returned $E, expected 99."
	exit 29;
fi




echo "ALL tests ok"
