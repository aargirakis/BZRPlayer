# no user-configurable paths below this line

ifndef DO
$(error Use "Makefile" instead of "python.mk")
endif

python: python/asap.py
.PHONY: python

python/asap.py: $(call src,asap.ci asap6502.ci asapinfo.ci cpu6502.ci pokey.ci) $(ASM6502_PLAYERS_OBX)
	$(CITO)
CLEAN += python/asap.py
