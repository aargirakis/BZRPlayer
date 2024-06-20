	.file	"main.cpp"
	.intel_syntax noprefix
	.text
	.p2align 2,,3
.globl __Z9getlengthP16FMOD_CODEC_STATEPjj@12
	.def	__Z9getlengthP16FMOD_CODEC_STATEPjj@12;	.scl	2;	.type	32;	.endef
__Z9getlengthP16FMOD_CODEC_STATEPjj@12:
LFB852:
	push	ebp
LCFI0:
	mov	ebp, esp
LCFI1:
	xor	eax, eax
	leave
LCFI2:
	ret	12
LFE852:
	.p2align 2,,3
.globl __Z11setpositionP16FMOD_CODEC_STATEijj@16
	.def	__Z11setpositionP16FMOD_CODEC_STATEijj@16;	.scl	2;	.type	32;	.endef
__Z11setpositionP16FMOD_CODEC_STATEijj@16:
LFB853:
	push	ebp
LCFI3:
	mov	ebp, esp
LCFI4:
	xor	eax, eax
	leave
LCFI5:
	ret	16
LFE853:
	.p2align 2,,3
.globl __Z4readP16FMOD_CODEC_STATEPvjPj@16
	.def	__Z4readP16FMOD_CODEC_STATEPvjPj@16;	.scl	2;	.type	32;	.endef
__Z4readP16FMOD_CODEC_STATEPvjPj@16:
LFB851:
	push	ebp
LCFI6:
	mov	ebp, esp
LCFI7:
	sub	esp, 24
LCFI8:
	mov	DWORD PTR [esp+12], 0
	mov	DWORD PTR [esp+8], 128
	mov	eax, DWORD PTR [ebp+12]
	mov	DWORD PTR [esp+4], eax
	mov	eax, DWORD PTR [ebp+8]
	mov	eax, DWORD PTR [eax+8]
	add	eax, 4
	mov	DWORD PTR [esp], eax
	call	__ZN9V2MPlayer6RenderEPfmi
	mov	eax, DWORD PTR [ebp+20]
	mov	edx, DWORD PTR [ebp+16]
	mov	DWORD PTR [eax], edx
	xor	eax, eax
	leave
LCFI9:
	ret	16
LFE851:
	.p2align 2,,3
.globl __Z5closeP16FMOD_CODEC_STATE@4
	.def	__Z5closeP16FMOD_CODEC_STATE@4;	.scl	2;	.type	32;	.endef
__Z5closeP16FMOD_CODEC_STATE@4:
LFB850:
	push	ebp
LCFI10:
	mov	ebp, esp
LCFI11:
	sub	esp, 24
LCFI12:
	mov	eax, DWORD PTR [ebp+8]
	mov	eax, DWORD PTR [eax+8]
	test	eax, eax
	je	L5
	mov	DWORD PTR [esp+4], 0
	add	eax, 4
	mov	DWORD PTR [esp], eax
	call	__ZN9V2MPlayer4StopEm
L5:
	xor	eax, eax
	leave
LCFI13:
	ret	4
LFE850:
	.section .rdata,"dr"
LC0:
	.ascii "v2m\0"
	.text
	.p2align 2,,3
.globl __Z4openP16FMOD_CODEC_STATEjP22FMOD_CREATESOUNDEXINFO@12
	.def	__Z4openP16FMOD_CODEC_STATEjP22FMOD_CREATESOUNDEXINFO@12;	.scl	2;	.type	32;	.endef
__Z4openP16FMOD_CODEC_STATEjP22FMOD_CREATESOUNDEXINFO@12:
LFB849:
	push	ebp
LCFI14:
	mov	ebp, esp
LCFI15:
	push	edi
LCFI16:
	push	esi
LCFI17:
	push	ebx
LCFI18:
	sub	esp, 60
LCFI19:
	mov	ebx, DWORD PTR [ebp+8]
	mov	DWORD PTR [esp], 4
	call	__Znaj
	mov	esi, eax
	mov	DWORD PTR [esp+8], eax
	mov	DWORD PTR [esp+4], 0
	mov	eax, DWORD PTR [ebx+12]
	mov	DWORD PTR [esp], eax
	call	[DWORD PTR [ebx+24]]
	sub	esp, 12
	mov	DWORD PTR [esp+16], 0
	mov	DWORD PTR [esp+12], 0
	mov	DWORD PTR [esp+8], 4
	mov	DWORD PTR [esp+4], esi
	mov	eax, DWORD PTR [ebx+12]
	mov	DWORD PTR [esp], eax
	call	[DWORD PTR [ebx+20]]
	sub	esp, 20
	cmp	BYTE PTR [esi], -128
	je	L12
L8:
	mov	DWORD PTR [esp], 0
	call	__ZdlPv
	mov	DWORD PTR [esp], esi
	call	__ZdlPv
	mov	eax, 25
L10:
	lea	esp, [ebp-12]
	pop	ebx
LCFI20:
	pop	esi
LCFI21:
	pop	edi
LCFI22:
	leave
LCFI23:
	ret	12
	.p2align 2,,3
L12:
LCFI24:
	cmp	BYTE PTR [esi+1], 0
	jne	L8
	cmp	BYTE PTR [esi+2], 0
	jne	L8
	cmp	BYTE PTR [esi+3], 0
	jne	L8
	mov	eax, DWORD PTR [ebx+16]
	sal	eax
	mov	DWORD PTR [esp], eax
	call	__Znaj
	mov	edx, eax
	mov	DWORD PTR [esp+8], eax
	mov	DWORD PTR [esp+4], 0
	mov	eax, DWORD PTR [ebx+12]
	mov	DWORD PTR [esp], eax
	mov	DWORD PTR [ebp-32], edx
	call	[DWORD PTR [ebx+24]]
	sub	esp, 12
	mov	DWORD PTR [esp+16], 0
	mov	DWORD PTR [esp+12], 0
	mov	eax, DWORD PTR [ebx+16]
	mov	DWORD PTR [esp+8], eax
	mov	edx, DWORD PTR [ebp-32]
	mov	DWORD PTR [esp+4], edx
	mov	eax, DWORD PTR [ebx+12]
	mov	DWORD PTR [esp], eax
	call	[DWORD PTR [ebx+20]]
	sub	esp, 20
	mov	DWORD PTR [esp], 3155112
	call	__Znwj
	mov	esi, eax
	mov	DWORD PTR [eax], ebx
	lea	eax, [eax+3154816]
	mov	DWORD PTR [ebp-28], eax
	mov	ecx, 296
	xor	eax, eax
	mov	edi, DWORD PTR [ebp-28]
	rep stosb
	mov	DWORD PTR [esi+3145732], 1000
	mov	DWORD PTR [esi+3145736], 0
	lea	edi, [esi+4]
	mov	DWORD PTR [esp+8], 44100
	mov	edx, DWORD PTR [ebp-32]
	mov	DWORD PTR [esp+4], edx
	mov	DWORD PTR [esp], edi
	call	__ZN9V2MPlayer4OpenEPKvm
	test	eax, eax
	jne	L13
	mov	eax, 25
	jmp	L10
L13:
	mov	eax, DWORD PTR [ebp+16]
	mov	edx, DWORD PTR [eax+68]
	mov	DWORD PTR [esi+3155072], 2
	mov	DWORD PTR [esi+3155076], 2
	mov	DWORD PTR [esi+3155080], 44100
	mov	DWORD PTR [esi+3155092], 4
	mov	DWORD PTR [esi+3155088], -1
	mov	eax, DWORD PTR [ebp-28]
	mov	DWORD PTR [ebx+4], eax
	mov	DWORD PTR [ebx], 0
	mov	DWORD PTR [ebx+8], esi
	mov	DWORD PTR [esp+8], 3
	mov	DWORD PTR [esp+4], OFFSET FLAT:LC0
	lea	eax, [edx+200]
	mov	DWORD PTR [esp], eax
	mov	DWORD PTR [ebp-32], edx
	call	__ZNSs6assignEPKcj
	mov	DWORD PTR [esp+8], 3
	mov	DWORD PTR [esp+4], OFFSET FLAT:LC0
	mov	edx, DWORD PTR [ebp-32]
	add	edx, 160
	mov	DWORD PTR [esp], edx
	call	__ZNSs6assignEPKcj
	mov	DWORD PTR [esp+4], 0
	mov	DWORD PTR [esp], edi
	call	__ZN9V2MPlayer4PlayEm
	xor	eax, eax
	jmp	L10
LFE849:
	.p2align 2,,3
.globl __FMODGetCodecDescription@0
	.def	__FMODGetCodecDescription@0;	.scl	2;	.type	32;	.endef
__FMODGetCodecDescription@0:
LFB848:
	push	ebp
LCFI25:
	mov	ebp, esp
LCFI26:
	mov	eax, OFFSET FLAT:_tfmxcodec
	leave
LCFI27:
	ret
LFE848:
.globl _tfmxcodec
	.section .rdata,"dr"
LC1:
	.ascii "V2M player plugin\0"
	.data
	.align 32
_tfmxcodec:
	.long	LC1
	.long	65536
	.long	1
	.long	1
	.long	__Z4openP16FMOD_CODEC_STATEjP22FMOD_CREATESOUNDEXINFO@12
	.long	__Z5closeP16FMOD_CODEC_STATE@4
	.long	__Z4readP16FMOD_CODEC_STATEPvjPj@16
	.long	__Z9getlengthP16FMOD_CODEC_STATEPjj@12
	.long	__Z11setpositionP16FMOD_CODEC_STATEijj@16
	.long	0
	.long	0
	.space 4
	.section	.eh_frame,"w"
Lframe1:
	.long	LECIE1-LSCIE1
LSCIE1:
	.long	0x0
	.byte	0x1
	.ascii "\0"
	.uleb128 0x1
	.sleb128 -4
	.byte	0x8
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x88
	.uleb128 0x1
	.align 4
LECIE1:
LSFDE1:
	.long	LEFDE1-LASFDE1
LASFDE1:
	.long	LASFDE1-Lframe1
	.long	LFB851
	.long	LFE851-LFB851
	.byte	0x4
	.long	LCFI6-LFB851
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI7-LCFI6
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI9-LCFI7
	.byte	0xc5
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.align 4
LEFDE1:
LSFDE3:
	.long	LEFDE3-LASFDE3
LASFDE3:
	.long	LASFDE3-Lframe1
	.long	LFB850
	.long	LFE850-LFB850
	.byte	0x4
	.long	LCFI10-LFB850
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI11-LCFI10
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI13-LCFI11
	.byte	0xc5
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.align 4
LEFDE3:
LSFDE5:
	.long	LEFDE5-LASFDE5
LASFDE5:
	.long	LASFDE5-Lframe1
	.long	LFB849
	.long	LFE849-LFB849
	.byte	0x4
	.long	LCFI14-LFB849
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI15-LCFI14
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI19-LCFI15
	.byte	0x83
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x4
	.byte	0x87
	.uleb128 0x3
	.byte	0x4
	.long	LCFI20-LCFI19
	.byte	0xa
	.byte	0xc3
	.byte	0x4
	.long	LCFI21-LCFI20
	.byte	0xc6
	.byte	0x4
	.long	LCFI22-LCFI21
	.byte	0xc7
	.byte	0x4
	.long	LCFI23-LCFI22
	.byte	0xc5
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x4
	.long	LCFI24-LCFI23
	.byte	0xb
	.align 4
LEFDE5:
	.def	__ZN9V2MPlayer6RenderEPfmi;	.scl	2;	.type	32;	.endef
	.def	__ZN9V2MPlayer4StopEm;	.scl	2;	.type	32;	.endef
	.def	__Znaj;	.scl	2;	.type	32;	.endef
	.def	__ZdlPv;	.scl	2;	.type	32;	.endef
	.def	__Znwj;	.scl	2;	.type	32;	.endef
	.def	__ZN9V2MPlayer4OpenEPKvm;	.scl	2;	.type	32;	.endef
	.def	__ZNSs6assignEPKcj;	.scl	2;	.type	32;	.endef
	.def	__ZN9V2MPlayer4PlayEm;	.scl	2;	.type	32;	.endef
	.section .drectve
	.ascii " -export:\"_FMODGetCodecDescription@0\""
