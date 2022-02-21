#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage : $0 filename"
fi

input=$1
output=${input%.*}.pgm

# convert $input -compress none $output
convert $input $output
