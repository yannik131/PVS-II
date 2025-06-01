#!/bin/bash

enscript \
    --header='$n - Page $% of $= || Moritz Lunk, bt712482 | Yannik Schroeder, bt722248 | Justin Stoll, bt709742' \
    --header-font='Helvetica-Bold10' \
    --line-numbers \
    --color \
    --highlight=c \
    -f Courier8 \
    -T 4 \
    3_1.c 3_2.c 3_3.c \
    -o output.ps

ps2pdf output.ps output.pdf