CC = gcc
LEX = flex
YACC = bison

CFLAGS = -g
YFLAGS = -d -v -y

LFILE = casl2c.l
YFILE = casl2c.y

CFILES = $(wildcard *.c) y.tab.c lex.yy.c
OFILES = $(patsubst %.c,../obj/%.o,$(CFILES))

.PHONY : all clean

all:	../caslcomp

y.tab.c y.tab.h: $(YFILE)
	$(YACC) $(YFLAGS) $(YFILE)

lex.yy.c: ${LFILE} y.tab.h
	$(LEX) $(LFILE)

../obj/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

../caslcomp: ${OFILES}
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OFILES) ../caslcomp ../caslcomp.exe y.* lex.yy.c
