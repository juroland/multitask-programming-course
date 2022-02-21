#!/bin/bash
cat big.txt | tr -s '[:digit:]' ' ' | tr '[A-Z]' '[a-z]' | tr -s '[:punct:]' ' ' | tr -s '\n\f\t\r ' '\n' | sort | uniq -c
