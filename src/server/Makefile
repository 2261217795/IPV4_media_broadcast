CFLAGS+=-I../include/ -pthread -g 
LDFLAGS+=-pthread

all:server

server: medialib.o thr_list.o thr_channel.o server.o mytbf.o
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)


clean:
	rm *.o server.o -rf

