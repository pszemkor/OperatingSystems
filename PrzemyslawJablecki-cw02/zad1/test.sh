#! /bin/bash

record_sizes=(1 4 512 1024 4096 8192)
records_no=(1000 2000 3000 4000)

for size in ${record_sizes[*]}
do
    echo "record size: $size"
    for amount in ${records_no[*]}
    do
        echo "records no: $amount"
        ./lab2 generate data $amount $size > /dev/null 2>&1
        echo "lib version: "
        ./lab2 copy data data1 $amount $size lib
        ./lab2 sort data $amount $size lib
        echo "sys version: "
        ./lab2 copy data data1 $amount $size sys
        ./lab2 sort data $amount $size sys
        echo " "
    done
done