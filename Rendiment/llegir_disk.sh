#!/bin/bash
i=0
echo -n "$1," >> out_llegir_disk.csv
until [ $i -gt 10 ]
do
        ./read-from-disk "$1" >>  out_llegir_disk.csv
        echo -n "," >> out_llegir_disk.csv
        ((i=i+1))
done
echo "" >> out_llegir_disk.csv
