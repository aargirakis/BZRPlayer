MODULES = score

all:	$(MODULES)

score:	score.s
	vasm.vasmm68k-mot -no-opt -o $@ -Fbin $<

clean:
	rm -f $(MODULES)
