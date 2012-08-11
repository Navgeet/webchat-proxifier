CC=gcc
CFLAGS=-ggdb -Wall

CFLAGS+=-I/home/croux/c/chat-0.3/curl/include
LIB+=-L/home/croux/c/chat-0.3/curl/lib -lcurl -lssl -lrt

CFLAGS += $(shell pkg-config gtk+-2.0 --cflags)
LIB += $(shell pkg-config gtk+-2.0 --libs)

OBJS=gui.o main.o buffer.o net.o irc.o cJSON.o event.o

freenode: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIB)

curl: curl.c
	$(CC) $(CFLAGS) -o $@ $< $(LIB)

gui.c: gui.k
	gtkcooker gui.k -l -a -hout gui.h.temp
	if `cmp -s gui.h.temp gui.h`; then rm gui.h.temp; else mv gui.h.temp gui.h; fi

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f curl freenode *.o core *~ *.bak
	rm -rf freenode-release

dep:
	sed -e "/^#EOM/q" < Makefile > Makefile.new
	gcc -MM -MG *.[ch] >> Makefile.new
	mv Makefile.new Makefile

release:
	rm -rf freenode-release
	mkdir freenode-release
	cp -a *.[chk] Makefile TODO README freenode-release
	touch freenode-release/gui.c

#EOM
buffer.o: buffer.c buffer.h err.h
buffer.o: buffer.h
cJSON.o: cJSON.c cJSON.h
cJSON.o: cJSON.h
curl.o: curl.c
err.o: err.h
event.o: event.c event.h net.h buffer.h irc.h err.h
event.o: event.h net.h buffer.h irc.h
gui.o: gui.c gui.h err.h
gui.o: gui.h
irc.o: irc.c irc.h buffer.h err.h cJSON.h
irc.o: irc.h buffer.h
main.o: main.c net.h buffer.h irc.h err.h gui.h cJSON.h event.h
net.o: net.c net.h buffer.h err.h
net.o: net.h buffer.h
