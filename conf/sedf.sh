#!/bin/bash

jflist=`find  -type f -print`
for jf in $jflist
do
     echo "sed $jf"
	sed -i  's/matchfr_t/matchfr_t/g' $jf
done
