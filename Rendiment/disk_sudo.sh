#!/bin/bash
i=0
echo -n "sudo $1," >> out_disk.csv
until [ $i -gt 10 ]
do
	sudo ./write-to-disk "$1" >>  out_disk.csv
	echo -n "," >> out_disk.csv
	((i=i+1))
done
echo "" >> out_disk.csv
