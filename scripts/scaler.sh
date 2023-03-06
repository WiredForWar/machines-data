#!/bin/bash

FILES=$(find . -maxdepth 1 -iname "*.bmp")
for FILE in $FILES
do
    BASE=$(basename $FILE .bmp)
    convert -filter point -resize 200% $FILE ${BASE}_2x.png
done
