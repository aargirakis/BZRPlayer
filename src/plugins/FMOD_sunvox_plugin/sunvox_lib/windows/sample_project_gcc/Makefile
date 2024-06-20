all:
	cp ../lib_x86/sunvox.dll ./
	cp ../../resources/song01.sunvox ./
	i686-w64-mingw32-gcc -g -fno-exceptions -c test.cpp
	i686-w64-mingw32-gcc -o test.exe test.o

clean:
	rm -f *.o *.dll *.exe *.sunvox