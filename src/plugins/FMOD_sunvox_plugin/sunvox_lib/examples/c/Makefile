all:
	cp ../../resources/* ./
	gcc $(CFLAGS) -g -c test1.c
	gcc $(CFLAGS) -g -c test2.c
	gcc $(CFLAGS) -g -c test3.c
	gcc $(CFLAGS) -g -c test4.c
	gcc $(CFLAGS) -g -c test5.c
	gcc $(CFLAGS) -g -c test6.c
	gcc $(CFLAGS) -g -c test7.c
	gcc $(CFLAGS) -g -c test8.c
	gcc $(LDFLAGS) -o test1 test1.o $(LIBS)
	gcc $(LDFLAGS) -o test2 test2.o $(LIBS)
	gcc $(LDFLAGS) -o test3 test3.o $(LIBS)
	gcc $(LDFLAGS) -o test4 test4.o $(LIBS)
	gcc $(LDFLAGS) -o test5 test5.o $(LIBS)
	gcc $(LDFLAGS) -o test6 test6.o $(LIBS)
	gcc $(LDFLAGS) -o test7 test7.o $(LIBS)
	gcc $(LDFLAGS) -o test8 test8.o $(LIBS)

clean:
	rm -f *.o *.so *.dylib test1 test2 test3 test4 test5 test6 test7 test8 *.sunvox *.sunsynth *.xi *.wav *.ogg
