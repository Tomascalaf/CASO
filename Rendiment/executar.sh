#!/bin/bash
i=0
echo -n "$1," >> out.csv
until [ $i -gt $2 ]
do
	./"$1" >>  out.csv
	echo -n "," >> out.csv
	((i=i+1))
done
echo "" >> out.csv
