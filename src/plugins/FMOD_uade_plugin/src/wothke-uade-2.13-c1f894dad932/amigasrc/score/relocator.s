* See https://sintonen.fi/src/loadseg/LS.asm for help when in trouble

relocator
	lea	relocator_exe_address(pc),a1
	move.l	a0,(a1)
	* a0 is the reloctable binary to process. memory is allocated for the
	* hunks, data is copied and address values are relocated.
	cmp.l	#$000003f3,(a0)+
	bne	hunkerror
	tst.l	(a0)+
	bne	hunkerror
	lea	relocator_nhunks(pc),a1
	move.l	(a0)+,(a1)		* take number of hunks
	* we could clear upper word of number of hunks, because original
	* implementation only uses 16 bits. it's an undocumented feature.
	* however, i'm sporty and want to see a player/custom that abuses
	* this feature. bring it on. the next cmp command will catch the
	* error.
	cmp.l	#100,(a1)
	bhi	hunkerror
	addq.l	#8,a0			* skip hunk load infos

	lea	relocator_hunks(pc),a1
	lea	chippoint(pc),a2
	move.l	relocator_nhunks(pc),d7
	subq	#1,d7
hunkcheckloop	move.l	(a0)+,d1
	move.l	d1,d2
	andi.l	#$3fffffff,d1
	lsl.l	#2,d1
	move.l	d1,(a1)+		* save hunk size (in bytes)
	* Harry Sintonen pointed out there is a possibility of extra long
	* memattr here (another long word) if MEMF_CHIP and MEMF_FAST flags
	* are both set, but i'm not testing for that work-around unless
	* it really happens with some player/custom we know of.
	andi.l	#$40000000,d2
	move.l	d2,(a1)+		* save hunk mem type
	move.l	(a2),d0			* take relocpoint
	andi.b	#-8,d0			* align by 8
	addq.l	#8,d0
	move.l	d0,(a1)+		* save reloc addr for hunk
	add.l	d1,d0
	move.l	d0,(a2)			* put new relocpoint
	dbf	d7,hunkcheckloop

	lea	relocator_hunks(pc),a1
	move.l	relocator_nhunks(pc),d7
	subq	#1,d7
	bmi.b	nomorehunks

HunkLoop	push	d7/a1
HunkLoopTakeNext
	move.l	(a0)+,d1
	andi.l	#$ffff,d1
	cmpi.l	#$000003f1,d1
	bne.b	NotDebugHunk
	pushr	a0
	lea	debughunkwarn(pc),a0
	bsr	put_string
	pullr	a0
	move.l	(a0)+,d0
	lsl.l	#2,d0
	add.l	d0,a0
	bra.b	HunkLoopTakeNext
NotDebugHunk
	cmpi.l	#$000003ea,d1
	beq	DataCodeHunk
	cmpi.l	#$000003e9,d1
	beq	DataCodeHunk
	cmpi.l	#$000003eb,d1
	beq	BSSHunk
hunklooperror	pull	d7/a1
	moveq	#-1,d0
	rts
conthunkloop	cmp.l	#$000003f2,(a0)+
	bne.b	hunklooperror
	pull	d7/a1
	add	#12,a1
	dbf	d7,HunkLoop
nomorehunks	move.l	relocator_hunks+8(pc),a0
	moveq	#0,d0
	rts
hunkerror	moveq	#-1,d0
	rts

DataCodeHunk	move.l	(a0)+,d0	* take hunk length (in long words)
	lsl.l	#2,d0

	* copy hunk data
	pushr	a1
	move.l	8(a1),a1
	bsr	memcopy
	add.l	d0,a0		* skip hunk data
	pullr	a1

	* if reported hunk size is bigger than copied hunk data,
	* fill the rest with zeros (Harry Sintonen said this is a must)
	move.l	(a1),d1
	sub.l	d0,d1
	beq.b	no_extra_in_hunk
	bpl.b	hunk_size_not_fucked_1
	push	all
	lea	hunksizeerr(pc),a0
	bsr	put_string
	pull	all
	bra.b	no_extra_in_hunk
hunk_size_not_fucked_1
	push	all
	lea	hunksizewarn(pc),a0
	bsr	put_string
	move.l	8(a1),a0		*   destination start address
	add.l	d0,a0			* + actual data size
	move.l	d1,d0			* extra size
	bsr	clearmem
	pull	all
no_extra_in_hunk

hunktailloop	cmp.l	#$000003ec,(a0)
	bne.b	noprogreloc
	addq.l	#4,a0
	pushr	a1
	move.l	8(a1),a1
	bsr	handlerelochunk
	pullr	a1
	bra.b	hunktailloop
noprogreloc	cmp.l	#$000003f7,(a0)
	bne.b	noprogreloc_3f7
	addq.l	#4,a0
	pushr	a1
	move.l	8(a1),a1
	bsr	handlerelochunk_3f7
	pullr	a1
	bra.b	hunktailloop
noprogreloc_3f7	cmp.l	#$000003f0,(a0)
	bne.b	nosymbolhunk
	push	all
	lea	symbolhunkwarn(pc),a0
	bsr	put_string
	pull	all
	addq.l	#4,a0
symbolhunkloop	move.l	(a0)+,d0
	beq.b	hunktailloop
	lsl.l	#2,d0
	add.l	d0,a0
	addq.l	#4,a0
	bra.b	symbolhunkloop
nosymbolhunk
	cmp.l	#$000003f1,(a0)
	bne.b	not_debug_hunk
	addq.l	#4,a0
	move.l	(a0)+,d0
	lsl.l	#2,d0
	add.l	d0,a0
	bra.b	hunktailloop
not_debug_hunk
	cmp.l	#$000003f2,(a0)
	beq	conthunkloop
	* Print the offset of the file that contains the invalid hunk
	move.l	a0,d0
	sub.l	relocator_exe_address(pc),d0
	bsr	put_value
	* Print hunk value
	move.l	(a0),d0
	bsr	put_value
	lea	illegalhunkwarn(pc),a0
	bsr	put_string
	illegal

* Clear BSS hunk memory with zeros
BSSHunk	move.l	(a0)+,d0	* take hunk length (in long words)
	lsl.l	#2,d0
	cmp.l	(a1),d0
	beq.b	r_size_match_2
	push	all
	lea	bsshunksizewarn(pc),a0
	bsr	put_string
	pull	all
r_size_match_2	pushr	a0
	move.l	8(a1),a0	* get hunk address
	bsr	clearmem
	pullr	a0
	bra	conthunkloop

handlerelochunk
	move.l	(a0)+,d0	* take number of reloc entries
	tst.l	d0
	bne.b	morereloentries
	rts
morereloentries	move.l	(a0)+,d1	* take index of associated hunk for
	lea	relocator_hunks(pc),a3	* following reloc entries
	mulu	#12,d1
	move.l	8(a3,d1),d2	* take reloced address for hunk
relochunkloop	move.l	(a0)+,d1	* take reloc entry (offset)
	add.l	d2,(a1,d1.l)	* add reloc base address
	subq.l	#1,d0
	bne.b	relochunkloop
	bra.b	handlerelochunk

* same as handle reloc hunk but takes 16 bit data word inputs
handlerelochunk_3f7
	moveq	#0,d0
	move	(a0)+,d0	* take number of reloc entries
	tst.l	d0
	bne.b	morereloentries_3f7
	* Round up address to the next value divisible by four
	move.l	a0,d0
	addq.l	#3,d0
	and.b	#$fc,d0
	move.l	d0,a0
	rts
morereloentries_3f7
	moveq	#0,d1
	move	(a0)+,d1	* take index of associated hunk for
	lea	relocator_hunks(pc),a3	* following reloc entries
	mulu	#12,d1
	move.l	8(a3,d1),d2	* take reloced address for hunk
	moveq	#0,d1
relochunkloop_3f7
	move	(a0)+,d1	* take reloc entry (offset)
	add.l	d2,(a1,d1.l)	* add reloc base address
	subq.l	#1,d0
	bne.b	relochunkloop_3f7
	bra.b	handlerelochunk_3f7

relocator_exe_address	dc.l	0
relocator_nhunks	dc.l	0
relocator_hunks	dcb.l	100*3,0

hunksizewarn	dc.b	'hunk size warn',0
bsshunksizewarn	dc.b	'bss hunk size warn',0
hunksizeerr	dc.b	'hunk size error',0
symbolhunkwarn	dc.b	'hunk relocator: symbol hunk warning!',0
illegalhunkwarn	dc.b	'illegal hunk',0
debughunkwarn	dc.b	'hunk relocator: debug hunk warning!', 0
	even
