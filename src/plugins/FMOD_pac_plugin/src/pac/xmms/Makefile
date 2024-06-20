CFLAGS = -W -Wall -Wno-unused -O2 -DNDEBUG -fPIC -I.. `xmms-config --cflags`
LIBS = `xmms-config --libs` -pthread

libxmmspac.so:	plugin.o libpac.a
		${CC} -s -shared -o libxmmspac.so plugin.o ../libpac.a ${LIBS}

libpac.a:
		cd .. && ${MAKE} libpac.a

clean:
		rm -f *~ plugin.o libxmmspac.so
