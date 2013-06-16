#!/bin/sh
while [ 1 -eq 1 ]
 do
 ps x >check
 if [ `grep -c "./dms.tmp" check` = 0 ]
   then
   date >>CRASH.LOG
   tail -n 50 CRASH.LOG|mail -s "REAL SD Crash Log" adf
   ./dms.tmp&
   sleep 100
 else
   sleep 30
 fi
done 
