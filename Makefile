CFLAGS=-Wall -I/usr/local/include/ -I/usr/local/include/json-c -I/usr/include/curl
LIBS=-L/usr/local/lib -loauth -lJson -lCurl

all: libTwitter

test: testtwitter_home_timeline testtwitter_sample_stream testtwitter_track_stream
	valgrind --leak-check=full ./testtwitter_home_timeline
	valgrind --leak-check=full ./testtwitter_sample_stream
	valgrind --leak-check=full ./testtwitter_track_stream

testtwitter_home_timeline: Makefile addr.h testtwitter_home_timeline.c
	gcc ${CFLAGS} -o testtwitter_home_timeline testtwitter_home_timeline.c ${LIBS} -lTwitter

testtwitter_sample_stream: Makefile addr.h testtwitter_sample_stream.c
	gcc ${CFLAGS} -o testtwitter_sample_stream testtwitter_sample_stream.c ${LIBS} -lTwitter

testtwitter_track_stream: Makefile addr.h testtwitter_track_stream.c
	gcc ${CFLAGS} -o testtwitter_track_stream testtwitter_track_stream.c ${LIBS} -lTwitter

libTwitter: Makefile twitter.o twitter.h addr.h
	gcc -shared -W1,-soname,libTwitter.so.1 -o libTwitter.so.1.0 twitter.o ${LIBS}

twitter.o: Makefile twitter.h twitter.c addr.h
	gcc ${CFLAGS} -fPIC -c twitter.c -o twitter.o

install:
	cp libTwitter.so.1.0 /usr/local/lib
	ln -sf /usr/local/lib/libTwitter.so.1.0 /usr/local/lib/libTwitter.so.1
	ln -sf /usr/local/lib/libTwitter.so.1.0 /usr/local/lib/libTwitter.so
	ldconfig /usr/local/lib
	cp twitter.h /usr/local/include/Twitter.h

clean:
	rm *.o; rm *.so*; rm testtwitter
