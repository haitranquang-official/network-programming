CFLAGS = -c -Wall
CC = gcc
LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -ldl -lz -lssl -lcrypto -lresolv -lm -lrt -I/usr/include/mysql 

all: server

server: ftp-server.o file_transfer.o connection.o login.o user.o scan_dir.o
	${CC} ftp-server.o file_transfer.o connection.o login.o user.o scan_dir.o -o server ${LDFLAGS} 

ftp-server.o: ftp-server.c
	${CC} ${CFLAGS} ftp-server.c

file_transfer.o: file_transfer/file_transfer.c
	${CC} ${CFLAGS} file_transfer/file_transfer.c

connection.o: database/connection.c
	${CC} ${CFLAGS} database/connection.c

login.o: login/login.c
	${CC} ${CFLAGS} login/login.c

user.o: database/user.c
	${CC} ${CFLAGS} database/user.c

scan_dir.o: scan_dir/scan_dir.c
	${CC} ${CFLAGS} scan_dir/scan_dir.c

clean:
	rm -f *.o *~