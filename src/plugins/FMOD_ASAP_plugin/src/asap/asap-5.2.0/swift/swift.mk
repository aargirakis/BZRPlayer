SWIFTC = $(DO)swiftc -no-color-diagnostics -sdk '$(SDKROOT)'

# no user-configurable paths below this line

ifndef DO
$(error Use "Makefile" instead of "swift.mk")
endif

swift: swift/asap2wav.exe
.PHONY: swift

swift/asap2wav.exe: swift/main.swift swift/asap.swift
	$(SWIFTC) -O -o $@ $^
CLEAN += swift/asap2wav.exe swift/asap2wav.exp swift/asap2wav.lib

swift/asap.swift: $(call src,asap.ci asap6502.ci asapinfo.ci cpu6502.ci pokey.ci) $(ASM6502_PLAYERS_OBX)
	$(CITO)
CLEAN += swift/asap.swift
