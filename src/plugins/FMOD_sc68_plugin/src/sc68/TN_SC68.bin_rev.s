VERSION = 1
REVISION = 3

.macro DATE
.ascii "27.11.2007"
.endm

.macro VERS
.ascii "TN_SC68.bin 1.3"
.endm

.macro VSTRING
.ascii "TN_SC68.bin 1.3 (27.11.2007)"
.byte 13,10,0
.endm

.macro VERSTAG
.byte 0
.ascii "$VER: TN_SC68.bin 1.3 (27.11.2007)"
.byte 0
.endm
