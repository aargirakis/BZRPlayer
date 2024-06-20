CFLAGS+=-W -Wall -Wno-unused -O2 -DNDEBUG -I.. -I/usr/local/include \
   -L/usr/local/lib
LIBS=-lm -lao

pacplay:	pacplay.o libpac.a
		${CC} ${CFLAGS} -s pacplay.o ../libpac.a -o pacplay ${LIBS} 

libpac.a:	
		cd .. && make libpac.a

clean:
		rm -rf *~ pacplay.o pacplay
