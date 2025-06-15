#!/bin/bash

enscript \
    --header='$n - Page $% of $= || Moritz Lunk, bt712482 | Yannik Schroeder, bt722248 | Justin Stoll, bt709742' \
    --header-font='Helvetica-Bold10' \
    --line-numbers \
    --color \
    --highlight=c \
    -f Courier8 \
    -T 4 \
    4_1.c barrier_semaphore.c consumer_producer.c Exercise4_4.java makefile util.h util.c vector.h vector.c \
    -o output.ps

ps2pdf output.ps output.pdf