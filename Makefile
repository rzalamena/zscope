CC	 =	gcc
YACC	 =	yacc -d
LEX	 =	flex

LDFLAGS	+=	-lsqlite3
CFLAGS	+=	-I. -Isys/ -Wall -Werror
CFLAGS  +=      -Wstrict-prototypes -Wmissing-prototypes
CFLAGS  +=      -Wmissing-declarations -Wshadow
CFLAGS  +=      -Wpointer-arith -Wcast-qual
CFLAGS  +=      -Wsign-compare

OBJS	 =	zscope.o zs_sqlite.o y.tab.o lex.yy.o
PROG	 =	zscope

.PHONY: clean

.c.o:
	${CC} ${CFLAGS} -c $<;

all: ${PROG}

y.tab.o:
	${YACC} cparser.y;
	${CC} -I. -c y.tab.c -o $@;

lex.yy.o:
	${LEX} cparser.l;
	${CC} -I. -c lex.yy.c -o $@;

${PROG}: ${OBJS}
	${CC} ${OBJS} ${LDFLAGS} -o $@;

clean:
	${RM} ${PROG} ${OBJS} lex.yy.c y.tab.c y.tab.h;
