#!/bin/bash

rm /boinc/milkyway/download/plots/*.jpeg
searchname=""
for arg in `find /boinc/milkyway/log_milkyway -type d -exec ls {} \;`
do
	searchname=${arg##/*/}
	if [ "$searchname" = "" ]
	then
		echo "skipping ($searchname)"
		continue
	fi
	if [ "$searchname" = "." ]
	then
		echo "skipping ($searchname)"
		continue
	fi

	if [ "${searchname:(-3)}" != "log" ]
	then
		echo "skipping, not a log ($searchname)"
		continue
	fi
	#searchname=`echo ${arg##/*/} | sed 's/^..//'`
	if [ "${searchname:0:2}" = "ps" ]
	then
		/boinc/milkyway/bin/pso_plot.sh $searchname 300
	elif [ "${searchname:0:6}" = "nbody-" ]
	then
		/boinc/milkyway/bin/pso_plot.sh $searchname 300
#	elif [ "${searchname:0:2}" = "gs" ]
#	then
#		/boinc/milkyway/results/milkyway/gs_plot.sh $searchname 300
#	elif [ "${searchname:0:2}" = "de" ]
#	then
#		/boinc/milkyway/results/milkyway/de_plot.sh $searchname 300
	fi
	echo "parsing search: ($searchname)"
done
