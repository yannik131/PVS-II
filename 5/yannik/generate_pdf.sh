#!/bin/bash

enscript \
    --header='$n - Page $% of $= || Moritz Lunk, bt712482 | Yannik Schroeder, bt722248 | Justin Stoll, bt709742' \
    --header-font='Helvetica-Bold10' \
    --line-numbers \
    --color \
    --highlight=java \
    -f Courier8 \
    -T 4 \
    PrefixSumCalculator.java App.java 5_3.c \
    -o output.ps

ps2pdf output.ps output.pdf