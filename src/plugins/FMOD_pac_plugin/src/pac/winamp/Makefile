CFLAGS = -W -Wall -Wno-unused -std=c99 -pedantic -DNDEBUG -O2 -I.. -Iinclude
OBJS = $(patsubst %.c, %.o, $(wildcard ../*.c))
TARGET = in_pac.dll

$(TARGET):	$(OBJS) plugin.o
		$(CC) $(CFLAGS) -shared -o $(TARGET) $(OBJS) plugin.o

$(OBJS):	$(wildcard ../*.h)
		$(MAKE) -C ..

clean:
		$(RM) -f *~ $(TARGET) plugin.o
