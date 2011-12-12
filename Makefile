PROGS = tftpd	

all:	${PROGS}

tftpd:	tftpd.o tftpdfunc.o
		${CC} ${CFLAGS} -o $@ tftpd.o tftpdfunc.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
