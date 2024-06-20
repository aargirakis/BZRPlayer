**********************************************************************************************
      CAUTION: I once had a strange phantom bug that at the time seemed to depend on wheather 
      source files contained Windows-CR/LF instead of Unix-LF: Previously functioning code no 
      longer worked. Weirdly I could no longer reproduce the effect the next day and it
      is totally unclear WTF was going on! Just keep that in mind next time there is some
      phantom bug..
**********************************************************************************************


for setup of cross-compiler (e.g. to recompile "score" see: 
https://blitterstudio.com/setting-up-an-amiga-cross-compiler/

	export VBCC=/usr/local/vbcc
	export PATH=$VBCC/bin:$PATH
	vasmm68k_mot.exe -no-opt  -o score -Fbin  score.s
	
	
	D:\Programs\vasm\vasmm68k_mot.exe -no-opt  -o score -Fbin  score.s
	

Note: I've added the amiga clib include files so that the respective data structures 
can be more easily manipulated from the C++ side. (UADE likes to do these things 
on the m68k side but I prefer using a higher level language rather than ASM!
Make sure to convert the endianness and properly translate memory addresses
when accessing "the amiga" from the UADE C side.)
