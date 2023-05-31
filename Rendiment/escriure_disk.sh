#!/bin/bash
i=0
echo -n "$1," >> out_escriure_disk.csv
until [ $i -gt 10 ]
do
	write-to-disk "$1" >>  out_escriure_disk.csv
	echo -n "," >> out_escriure_disk.csv
	((i=i+1))
done
echo "" >> out_escriure_disk.csv
