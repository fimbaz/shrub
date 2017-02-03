CC=gcc
CFLAGS=-Wall -Werror -Wno-missing-braces -std=gnu99 -m32 -I.
SHRUB_DEPS= shrub.c  genome.c perlin.c roulette.c serialize.c worldview.c game.c items.c
SHRUB_LIBS= -lncurses -lpanel -lm -lmenu -lform
fast: $(SHRUB_DEPS) TAGS
	$(CC) $(CFLAGS) -o shrub $(SHRUB_DEPS) $(SHRUB_LIBS) -O3
TAGS: $(SHRUB_DEPS)
	etags --language=c -R *.c *.h
debug: $(SHRUB_DEPS) TAGS
	$(CC) $(CFLAGS) -o shrub $(SHRUB_DEPS) $(SHRUB_LIBS) -g -pg
menu: TAGS menu1.c
	$(CC) $(CFLAGS) menu1.c  -lform -lncurses -lpanel -lmenu -o menu -g
bg:	bg.c TAGS
	gcc -Wall -std=gnu99 -lncurses bg.c -o bg
utest: roulette.c
	$(CC) $(CFLAGS) -o utest utest.c roulette.c
