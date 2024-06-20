CXX = g++
CXXFLAGS = -O2 -Wall

# no user-configurable paths below this line

ifndef DO
$(error Use "Makefile" instead of "opencl.mk")
endif

opencl: opencl/asapcl
.PHONY: opencl

opencl/asapcl: opencl/asapcl.cpp opencl/asap-cl.h asap.o asap.h
	$(DO)$(CXX) $(CXXFLAGS) -o $@ $(INCLUDEOPTS) $< asap.o -lOpenCL

opencl/asap-cl.h: opencl/asap.cl opencl/asap2wav-kernel.cl
	$(DO)(echo 'R"CLC(' && cat $^ && echo ')CLC"') >$@
CLEAN += opencl/asap-cl.h

opencl/asap.cl: $(call src,asap.ci asap6502.ci asapinfo.ci cpu6502.ci pokey.ci) $(ASM6502_PLAYERS_OBX)
	$(CITO) -D OPENCL
CLEAN += opencl/asap.cl
