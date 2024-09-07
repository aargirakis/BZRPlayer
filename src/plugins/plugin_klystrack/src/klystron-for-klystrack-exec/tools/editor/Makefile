TARGET=editor.exe
VPATH=src:src
ECHO = echo
CFG = debug
MACHINE = -march=pentium2 

# The directories containing the source files, separated by ':'


# To make "debug" the default (if invoked with just "make"):
#
# ifeq ($(CFG),)
# CFG=debug
# endif

# The source files: regardless of where they reside in 
# the source tree, VPATH will locate them...
Group0_SRC = $(notdir ${wildcard src/*.c})

# Build a Dependency list and an Object list, by replacing 
# the .cpp extension to .d for dependency files, and .o for 
# object files.
Group0_DEP = $(patsubst %.c, deps/Group0_%.d, ${Group0_SRC})
Group0_OBJ = $(patsubst %.c, objs.$(CFG)/Group0_%.o, \
	${Group0_SRC}) 


	
# What compiler to use for generating dependencies: 
# it will be invoked with -MM
CXX = gcc -std=c99 --no-strict-aliasing 
CXXDEP = gcc -E -std=c99

# What include flags to pass to the compiler

CXXFLAGS = $(MACHINE) -ftree-vectorize

ifdef COMSPEC
	CXXFLAGS += -mthreads
	INCLUDEFLAGS := -I ../Common -I src -I /MinGW/include/SDL2 -I ../../src 
else
	INCLUDEFLAGS := `sdl-config --cflags` -U_FORTIFY_SOURCE -I ../../src 
endif

LDFLAGS = -L ../../bin.$(CFG) -lengine_gfx -lengine_util -lengine_gui -lmingw32 -lSDL2_image -lSDLmain -lSDL2 -lconfig 

# Separate compile options per configuration
ifeq ($(CFG),debug)
CXXFLAGS += -O3 -g -Wall ${INCLUDEFLAGS} -DDEBUG
else
ifeq ($(CFG),profile)
CXXFLAGS += -O3 -g -pg -Wall ${INCLUDEFLAGS}
else
ifeq ($(CFG),release)
CXXFLAGS += -O3 -g -Wall ${INCLUDEFLAGS}
else
@$(ECHO) "Invalid configuration "$(CFG)" specified."
@$(ECHO) "You must specify a configuration when "
@$(ECHO) "running make, e.g. make CFG=debug"
@$(ECHO) "Possible choices for configuration are "
@$(ECHO) "'release', 'profile' and 'debug'"
@exit 1
exit
endif
endif
endif

all:	inform bin.$(CFG)/${TARGET} 

inform:
	@echo "Configuration "$(CFG)
	@echo "------------------------"

bin.$(CFG)/${TARGET}: ${Group0_OBJ} | inform
	@mkdir -p bin.$(CFG)
	$(CXX) $(CXXFLAGS) -o $@ $^ ${LDFLAGS}

objs.$(CFG)/Group0_%.o: %.c
	@mkdir -p objs.$(CFG)
	$(CXX) $(CXXFLAGS) -c $(CXXFLAGS) -o $@ $<

deps/Group0_%.d: %.c
	@mkdir -p deps
	@$(ECHO) "Generating dependencies for $<"
	@set -e ; $(CXXDEP) -MM $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,objs.$(CFG)\/Group0_\1.o $@ : ,g' \
		< $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	@rm -rf deps objs.$(CFG) bin.$(CFG)

# Unless "make clean" is called, include the dependency files
# which are auto-generated. Don't fail if they are missing
# (-include), since they will be missing in the first 
# invocation!
ifneq ($(MAKECMDGOALS),clean)
-include ${Group0_DEP}
endif
