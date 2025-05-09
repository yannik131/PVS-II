enscript \
    --header='$n - Page $% of $= || Moritz Lunk, bt712482 | Yannik Schroeder, bt722248 | Justin Stoll, bt709742' \
    --header-font='Helvetica-Bold10' \
    --line-numbers \
    --color \
    --highlight=c \
    -f Courier8 \
    -T 4 \
    1_1.c 1_2.c 1_3.c util.h util.c vector.h vector.c \
    -o output.ps

ps2pdf output.ps output.pdf