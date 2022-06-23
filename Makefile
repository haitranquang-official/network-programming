CFLAGS = -c -Wall
CC = gcc
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -ldl -lz -lssl -lcrypto -lresolv -lm -lrt -I/usr/include/mysql 

all: server

server: ftp-server.o file_transfer.o connection.o
	${CC} ftp-server.o file_transfer.o connection.o -o server ${LDFLAGS} 

ftp-server.o: ftp-server.c
	${CC} ${CFLAGS} ftp-server.c

file_transfer.o: file_transfer/file_transfer.c
	${CC} ${CFLAGS} file_transfer/file_transfer.c

connection.o: database/connection.c
	${CC} ${CFLAGS} database/connection.c

clean:
	rm -f *.o *~