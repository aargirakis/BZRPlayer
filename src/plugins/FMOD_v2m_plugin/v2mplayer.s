	.file	"v2mplayer.cpp"
	.intel_syntax noprefix
	.section .rdata,"dr"
LC0:
	.ascii " \0"
	.text
	.align 2
	.p2align 2,,3
.globl __ZN9V2MPlayer8InitBaseEPKv
	.def	__ZN9V2MPlayer8InitBaseEPKv;	.scl	2;	.type	32;	.endef
__ZN9V2MPlayer8InitBaseEPKv:
LFB6:
	push	ebp
LCFI0:
	mov	ebp, esp
LCFI1:
	push	edi
LCFI2:
	push	esi
LCFI3:
	push	ebx
LCFI4:
	sub	esp, 4
LCFI5:
	mov	edi, DWORD PTR [ebp+8]
	mov	eax, DWORD PTR [ebp+12]
	mov	edx, DWORD PTR [eax]
	mov	DWORD PTR [edi+3145744], edx
	lea	edx, [edx+edx*4]
	lea	edx, [edx+edx*4]
	lea	edx, [edx+edx*4]
	lea	edx, [edx+edx*4]
	sal	edx, 4
	mov	DWORD PTR [edi+3145748], edx
	mov	edx, DWORD PTR [eax+4]
	mov	DWORD PTR [edi+3145752], edx
	mov	edx, DWORD PTR [eax+8]
	mov	DWORD PTR [edi+3145760], edx
	add	eax, 12
	mov	DWORD PTR [edi+3145756], eax
	lea	edx, [edx+edx*4]
	lea	eax, [eax+edx*2]
	lea	esi, [edi+3145764]
	xor	edx, edx
	mov	DWORD PTR [ebp-16], edi
	mov	edi, edx
	.p2align 2,,3
L4:
	mov	edx, DWORD PTR [eax]
	mov	DWORD PTR [esi], edx
	add	eax, 4
	test	edx, edx
	je	L2
	mov	DWORD PTR [esi+4], eax
	lea	edx, [edx+edx*4]
	add	eax, edx
	mov	edx, DWORD PTR [eax]
	mov	DWORD PTR [esi+8], edx
	add	eax, 4
	mov	DWORD PTR [esi+12], eax
	lea	eax, [eax+edx*4]
	mov	edx, DWORD PTR [eax]
	mov	DWORD PTR [esi+16], edx
	add	eax, 4
	mov	DWORD PTR [esi+20], eax
	lea	edx, [edx+edx*4]
	add	eax, edx
	lea	edx, [edi+edi*4]
	sal	edx, 4
	mov	ecx, DWORD PTR [ebp-16]
	lea	edx, [ecx+3145788+edx]
	xor	ecx, ecx
	.p2align 2,,3
L3:
	mov	ebx, DWORD PTR [eax]
	mov	DWORD PTR [edx], ebx
	add	eax, 4
	mov	DWORD PTR [edx+4], eax
	lea	eax, [eax+ebx*4]
	inc	ecx
	add	edx, 8
	cmp	ecx, 7
	jne	L3
L2:
	inc	edi
	add	esi, 80
	cmp	edi, 16
	jne	L4
	mov	edi, DWORD PTR [ebp-16]
	mov	edx, DWORD PTR [eax]
	cmp	edx, 16384
	ja	L10
	add	eax, 4
	mov	DWORD PTR [edi+3145740], eax
	add	eax, edx
	mov	ecx, DWORD PTR [eax]
	cmp	ecx, 1048576
	ja	L10
	add	eax, 4
	mov	DWORD PTR [edi+3145736], eax
	lea	ecx, [eax+ecx]
	mov	eax, DWORD PTR [ecx]
	dec	eax
	cmp	eax, 8190
	ja	L17
	lea	ebx, [ecx+4]
	mov	DWORD PTR [edi+3147044], ebx
	mov	esi, DWORD PTR [ecx+4]
	test	esi, esi
	je	L11
	xor	eax, eax
	.p2align 2,,3
L8:
	mov	edx, DWORD PTR [ecx+8+eax*4]
	add	edx, ebx
	mov	DWORD PTR [edi+3147048+eax*4], edx
	inc	eax
	cmp	eax, esi
	jne	L8
L11:
	mov	eax, 1
	pop	edx
	pop	ebx
LCFI6:
	pop	esi
LCFI7:
	pop	edi
LCFI8:
	leave
LCFI9:
	ret
	.p2align 2,,3
L10:
LCFI10:
	xor	eax, eax
	pop	edx
	pop	ebx
LCFI11:
	pop	esi
LCFI12:
	pop	edi
LCFI13:
	leave
LCFI14:
	ret
L17:
LCFI15:
	xor	eax, eax
	.p2align 2,,3
L7:
	mov	DWORD PTR [edi+3147048+eax*4], OFFSET FLAT:LC0
	inc	eax
	cmp	eax, 256
	jne	L7
	mov	ax, 1
	pop	edx
	pop	ebx
LCFI16:
	pop	esi
LCFI17:
	pop	edi
LCFI18:
	leave
LCFI19:
	ret
LFE6:
	.align 2
	.p2align 2,,3
.globl __ZN9V2MPlayer5ResetEv
	.def	__ZN9V2MPlayer5ResetEv;	.scl	2;	.type	32;	.endef
__ZN9V2MPlayer5ResetEv:
LFB7:
	push	ebp
LCFI20:
	mov	ebp, esp
LCFI21:
	push	edi
LCFI22:
	push	esi
LCFI23:
	push	ebx
LCFI24:
	sub	esp, 60
LCFI25:
	mov	esi, DWORD PTR [ebp+8]
	mov	DWORD PTR [esi+3148076], 0
	mov	DWORD PTR [esi+3148080], -1
	mov	eax, DWORD PTR [esi+3145756]
	mov	DWORD PTR [esi+3148084], eax
	mov	DWORD PTR [esi+3148092], 0
	mov	edx, DWORD PTR [esi+3145760]
	test	edx, edx
	je	L19
	movzx	ecx, BYTE PTR [eax+edx]
	sal	ecx, 8
	movzx	ebx, BYTE PTR [eax]
	add	ecx, ebx
	movzx	eax, BYTE PTR [eax+edx*2]
	sal	eax, 16
	lea	eax, [ecx+eax]
	mov	DWORD PTR [esi+3148088], eax
	mov	DWORD PTR [esi+3148080], eax
L19:
	lea	eax, [esi+3145764]
	mov	DWORD PTR [ebp-40], eax
	lea	edx, [esi+3148156]
	mov	DWORD PTR [ebp-32], edx
	mov	DWORD PTR [ebp-44], 0
	.p2align 2,,3
L26:
	mov	ecx, DWORD PTR [ebp-40]
	mov	eax, DWORD PTR [ecx]
	test	eax, eax
	je	L20
	mov	edx, DWORD PTR [ecx+4]
	mov	edi, DWORD PTR [ebp-32]
	mov	DWORD PTR [edi-32], edx
	mov	BYTE PTR [edi-19], 0
	mov	BYTE PTR [edi-20], 0
	mov	DWORD PTR [edi-28], 0
	mov	edi, DWORD PTR [esi+3148076]
	movzx	ebx, BYTE PTR [edx+eax]
	sal	ebx, 8
	movzx	ecx, BYTE PTR [edx]
	add	ebx, ecx
	movzx	ecx, BYTE PTR [edx+eax*2]
	sal	ecx, 16
	lea	ecx, [ebx+ecx]
	add	ecx, edi
	mov	eax, DWORD PTR [ebp-32]
	mov	DWORD PTR [eax-24], ecx
	cmp	ecx, DWORD PTR [esi+3148080]
	jae	L21
	mov	DWORD PTR [esi+3148080], ecx
L21:
	mov	edx, DWORD PTR [ebp-40]
	mov	eax, DWORD PTR [edx+12]
	mov	ecx, DWORD PTR [ebp-32]
	mov	DWORD PTR [ecx-16], eax
	mov	BYTE PTR [ecx-4], 0
	mov	DWORD PTR [ecx-12], 0
	mov	ecx, DWORD PTR [ebp-40]
	mov	edx, DWORD PTR [ecx+8]
	test	edx, edx
	je	L22
	movzx	ebx, BYTE PTR [eax+edx]
	sal	ebx, 8
	movzx	ecx, BYTE PTR [eax]
	lea	ecx, [ebx+ecx]
	movzx	eax, BYTE PTR [eax+edx*2]
	sal	eax, 16
	add	ecx, eax
	lea	edi, [ecx+edi]
	mov	eax, DWORD PTR [ebp-32]
	mov	DWORD PTR [eax-8], edi
	cmp	edi, DWORD PTR [esi+3148080]
	jae	L22
	mov	DWORD PTR [esi+3148080], edi
L22:
	mov	edx, DWORD PTR [ebp-40]
	mov	eax, DWORD PTR [edx+20]
	mov	ecx, DWORD PTR [ebp-32]
	mov	DWORD PTR [ecx], eax
	mov	BYTE PTR [ecx+13], 0
	mov	BYTE PTR [ecx+12], 0
	mov	DWORD PTR [ecx+4], 0
	mov	edi, DWORD PTR [ebp-40]
	mov	edx, DWORD PTR [edi+8]
	test	edx, edx
	je	L23
	movzx	ebx, BYTE PTR [eax+edx]
	sal	ebx, 8
	movzx	ecx, BYTE PTR [eax]
	lea	ecx, [ebx+ecx]
	movzx	eax, BYTE PTR [eax+edx*2]
	sal	eax, 16
	add	ecx, eax
	add	ecx, DWORD PTR [esi+3148076]
	mov	eax, DWORD PTR [ebp-32]
	mov	DWORD PTR [eax+8], ecx
	cmp	ecx, DWORD PTR [esi+3148080]
	jae	L23
	mov	DWORD PTR [esi+3148080], ecx
L23:
	mov	edx, DWORD PTR [ebp-44]
	lea	eax, [edx+edx*4]
	mov	edx, eax
	sal	edx, 4
	lea	edx, [esi+3145792+edx]
	sal	eax, 5
	lea	eax, [esi+3148172+eax]
	mov	DWORD PTR [ebp-28], 0
	.p2align 2,,3
L25:
	mov	ecx, DWORD PTR [edx]
	mov	DWORD PTR [eax], ecx
	mov	BYTE PTR [eax+12], 0
	mov	DWORD PTR [eax+8], 0
	mov	ebx, DWORD PTR [edx-4]
	test	ebx, ebx
	je	L24
	movzx	edi, BYTE PTR [ecx+ebx]
	sal	edi, 8
	mov	DWORD PTR [ebp-36], edi
	movzx	edi, BYTE PTR [ecx]
	add	edi, DWORD PTR [ebp-36]
	movzx	ecx, BYTE PTR [ecx+ebx*2]
	sal	ecx, 16
	add	edi, ecx
	add	edi, DWORD PTR [esi+3148076]
	mov	DWORD PTR [eax+4], edi
	cmp	edi, DWORD PTR [esi+3148080]
	jae	L24
	mov	DWORD PTR [esi+3148080], edi
L24:
	inc	DWORD PTR [ebp-28]
	add	edx, 8
	add	eax, 16
	cmp	DWORD PTR [ebp-28], 7
	jne	L25
L20:
	inc	DWORD PTR [ebp-44]
	add	DWORD PTR [ebp-40], 80
	add	DWORD PTR [ebp-32], 160
	cmp	DWORD PTR [ebp-44], 16
	jne	L26
	mov	eax, DWORD PTR [esi+3150700]
	lea	edx, [eax+eax*4]
	lea	edx, [edx+edx*4]
	lea	edx, [edx+edx*4]
	lea	edx, [edx+edx*4]
	sal	edx, 3
	mov	DWORD PTR [esi+3148096], edx
	mov	DWORD PTR [esi+3148100], 4
	mov	DWORD PTR [esi+3148104], 4
	mov	DWORD PTR [esi+3148108], 8
	mov	DWORD PTR [esi+3148112], 0
	mov	DWORD PTR [esi+3148116], 0
	mov	DWORD PTR [esi+3148120], 0
	mov	DWORD PTR [esi+3150692], 0
	test	eax, eax
	je	L18
	mov	DWORD PTR [esp+8], eax
	mov	eax, DWORD PTR [esi+3145736]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], esi
	call	_synthInit@12
	sub	esp, 12
	mov	eax, DWORD PTR [esi+3145740]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], esi
	call	_synthSetGlobals@8
	sub	esp, 8
	lea	eax, [esi+3147048]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], esi
	call	_synthSetLyrics@8
	sub	esp, 8
L18:
	lea	esp, [ebp-12]
	pop	ebx
LCFI26:
	pop	esi
LCFI27:
	pop	edi
LCFI28:
	leave
LCFI29:
	ret
LFE7:
	.align 2
	.p2align 2,,3
.globl __ZN9V2MPlayer4TickEv
	.def	__ZN9V2MPlayer4TickEv;	.scl	2;	.type	32;	.endef
__ZN9V2MPlayer4TickEv:
LFB8:
	push	ebp
LCFI30:
	mov	ebp, esp
LCFI31:
	push	edi
LCFI32:
	push	esi
LCFI33:
	push	ebx
LCFI34:
	sub	esp, 92
LCFI35:
	mov	eax, DWORD PTR [ebp+8]
	cmp	DWORD PTR [eax+3148072], 2
	je	L70
L30:
	lea	esp, [ebp-12]
	pop	ebx
LCFI36:
	pop	esi
LCFI37:
	pop	edi
LCFI38:
	leave
LCFI39:
	ret
	.p2align 2,,3
L70:
LCFI40:
	mov	edx, DWORD PTR [eax+3148080]
	mov	DWORD PTR [ebp-32], edx
	add	edx, DWORD PTR [eax+3148120]
	sub	edx, DWORD PTR [eax+3148076]
	mov	DWORD PTR [eax+3148120], edx
	mov	edi, DWORD PTR [eax+3145744]
	cmp	edx, edi
	jb	L71
	mov	esi, edi
	neg	esi
	sub	edx, edi
	mov	ebx, DWORD PTR [ebp+8]
	mov	ecx, DWORD PTR [ebx+3148116]
	inc	ecx
	mov	DWORD PTR [ebp-28], edi
	jmp	L34
	.p2align 2,,3
L61:
	mov	ecx, ebx
	mov	edx, eax
L34:
	lea	eax, [edx+esi]
	lea	ebx, [ecx+1]
	mov	edi, DWORD PTR [ebp-28]
	add	edi, eax
	jae	L61
	mov	eax, DWORD PTR [ebp+8]
	mov	DWORD PTR [eax+3148120], edx
	mov	DWORD PTR [eax+3148116], ecx
L33:
	mov	edx, DWORD PTR [ebp+8]
	mov	eax, DWORD PTR [edx+3148100]
	sal	eax, 2
	mov	ebx, edx
	xor	edx, edx
	div	DWORD PTR [ebx+3148104]
	cmp	ecx, eax
	jb	L35
	mov	ebx, DWORD PTR [ebx+3148112]
	mov	esi, eax
	neg	esi
	sub	ecx, eax
	jmp	L36
	.p2align 2,,3
L62:
	mov	ecx, edx
L36:
	inc	ebx
	lea	edx, [ecx+esi]
	mov	edi, edx
	add	edi, eax
	jae	L62
	mov	eax, DWORD PTR [ebp+8]
	mov	DWORD PTR [eax+3148116], ecx
	mov	DWORD PTR [eax+3148112], ebx
L35:
	mov	ecx, DWORD PTR [ebp-32]
	mov	edx, DWORD PTR [ebp+8]
	mov	DWORD PTR [edx+3148076], ecx
	mov	DWORD PTR [edx+3148080], -1
	add	edx, 3150708
	mov	DWORD PTR [ebp-68], edx
	mov	ecx, edx
	mov	ebx, DWORD PTR [ebp+8]
	mov	esi, DWORD PTR [ebx+3148092]
	mov	edi, ebx
	mov	ebx, DWORD PTR [ebx+3145760]
	cmp	esi, ebx
	jb	L72
L37:
	mov	esi, DWORD PTR [ebp+8]
	add	esi, 3145764
	mov	eax, DWORD PTR [ebp+8]
	add	eax, 3148160
	mov	DWORD PTR [ebp-48], 0
	mov	DWORD PTR [ebp-44], -1
	.p2align 2,,3
L59:
	mov	ebx, DWORD PTR [esi]
	test	ebx, ebx
	je	L40
	mov	edx, DWORD PTR [esi+8]
	cmp	DWORD PTR [eax-16], edx
	jae	L41
	mov	ebx, DWORD PTR [eax-12]
	mov	edi, DWORD PTR [ebp+8]
	cmp	DWORD PTR [edi+3148076], ebx
	je	L73
L42:
	mov	edx, DWORD PTR [ebp+8]
	cmp	ebx, DWORD PTR [edx+3148080]
	jae	L41
	mov	DWORD PTR [edx+3148080], ebx
L41:
	mov	edi, DWORD PTR [ebp-48]
	lea	ebx, [edi+edi*4]
	mov	edx, ebx
	sal	edx, 5
	mov	edi, DWORD PTR [ebp+8]
	lea	edx, [edi+3148180+edx]
	mov	DWORD PTR [ebp-28], edx
	sal	ebx, 4
	lea	ebx, [edi+3145788+ebx]
	mov	edi, 1
	mov	dl, BYTE PTR [ebp-48]
	or	edx, -80
	mov	BYTE PTR [ebp-36], dl
	movzx	edx, dl
	mov	DWORD PTR [ebp-40], edx
	mov	DWORD PTR [ebp-76], ecx
	mov	edx, DWORD PTR [ebp-28]
	mov	DWORD PTR [ebp-56], eax
	mov	eax, edi
	mov	DWORD PTR [ebp-60], esi
	mov	edi, DWORD PTR [ebp+8]
	.p2align 2,,3
L49:
	mov	ecx, DWORD PTR [ebx]
	cmp	DWORD PTR [edx], ecx
	jae	L45
	mov	esi, DWORD PTR [edx-4]
	cmp	DWORD PTR [edi+3148076], esi
	je	L74
L46:
	cmp	esi, DWORD PTR [edi+3148080]
	jae	L45
	mov	DWORD PTR [edi+3148080], esi
L45:
	inc	eax
	add	edx, 16
	add	ebx, 8
	cmp	eax, 8
	jne	L49
	mov	ecx, DWORD PTR [ebp-76]
	mov	eax, DWORD PTR [ebp-56]
	mov	esi, DWORD PTR [ebp-60]
	mov	edi, DWORD PTR [esi+16]
	cmp	DWORD PTR [eax], edi
	jae	L50
	mov	edx, DWORD PTR [eax+4]
	mov	ebx, DWORD PTR [ebp+8]
	cmp	DWORD PTR [ebx+3148076], edx
	je	L75
L51:
	mov	ebx, DWORD PTR [ebp+8]
	cmp	DWORD PTR [ebx+3148080], edx
	jbe	L50
	mov	DWORD PTR [ebx+3148080], edx
L50:
	mov	edi, DWORD PTR [eax-32]
	mov	edx, DWORD PTR [esi]
	mov	bl, BYTE PTR [ebp-48]
	or	ebx, -112
	mov	BYTE PTR [ebp-40], bl
	movzx	ebx, bl
	mov	DWORD PTR [ebp-36], ebx
	mov	DWORD PTR [ebp-52], edi
	.p2align 2,,3
L68:
	cmp	DWORD PTR [ebp-52], edx
	jb	L76
L40:
	inc	DWORD PTR [ebp-48]
	add	esi, 80
	add	eax, 160
	cmp	DWORD PTR [ebp-48], 16
	jne	L59
	mov	BYTE PTR [ecx], -3
	mov	eax, DWORD PTR [ebp-68]
	mov	DWORD PTR [esp+4], eax
	mov	edx, DWORD PTR [ebp+8]
	mov	DWORD PTR [esp], edx
	call	_synthProcessMIDI@8
	sub	esp, 8
	mov	ecx, DWORD PTR [ebp+8]
	cmp	DWORD PTR [ecx+3148080], -1
	jne	L30
	mov	DWORD PTR [ecx+3148072], 1
	lea	esp, [ebp-12]
	pop	ebx
LCFI41:
	pop	esi
LCFI42:
	pop	edi
LCFI43:
	leave
LCFI44:
	ret
	.p2align 2,,3
L76:
LCFI45:
	mov	ebx, DWORD PTR [eax-28]
	mov	edi, DWORD PTR [ebp+8]
	cmp	DWORD PTR [edi+3148076], ebx
	je	L77
	mov	edi, DWORD PTR [ebp+8]
	cmp	ebx, DWORD PTR [edi+3148080]
	jae	L40
	mov	edi, DWORD PTR [ebp+8]
	mov	DWORD PTR [edi+3148080], ebx
	jmp	L40
	.p2align 2,,3
L77:
	mov	ebx, DWORD PTR [ebp-44]
	cmp	DWORD PTR [ebp-36], ebx
	je	L58
	mov	dl, BYTE PTR [ebp-40]
	mov	BYTE PTR [ecx], dl
	inc	ecx
	mov	edx, DWORD PTR [esi]
	mov	ebx, DWORD PTR [ebp-36]
	mov	DWORD PTR [ebp-44], ebx
L58:
	lea	edx, [edx+edx*2]
	add	edx, DWORD PTR [eax-36]
	mov	dl, BYTE PTR [edx]
	add	edx, DWORD PTR [eax-24]
	mov	BYTE PTR [eax-24], dl
	mov	BYTE PTR [ecx], dl
	mov	ebx, DWORD PTR [esi]
	mov	edx, DWORD PTR [eax-36]
	mov	dl, BYTE PTR [edx+ebx*4]
	add	dl, BYTE PTR [eax-23]
	mov	BYTE PTR [eax-23], dl
	mov	BYTE PTR [ecx+1], dl
	add	ecx, 2
	mov	ebx, DWORD PTR [eax-32]
	inc	ebx
	mov	DWORD PTR [ebp-52], ebx
	mov	DWORD PTR [eax-32], ebx
	mov	edi, DWORD PTR [eax-36]
	mov	DWORD PTR [ebp-32], edi
	inc	edi
	mov	DWORD PTR [eax-36], edi
	mov	edx, DWORD PTR [esi]
	cmp	ebx, edx
	jae	L68
	movzx	ebx, BYTE PTR [edi+edx]
	sal	ebx, 8
	movzx	edi, BYTE PTR [edi+edx*2]
	sal	edi, 16
	add	edi, ebx
	mov	DWORD PTR [ebp-28], edi
	mov	ebx, DWORD PTR [ebp-32]
	movzx	edi, BYTE PTR [ebx+1]
	add	edi, DWORD PTR [ebp-28]
	je	L68
	mov	edx, DWORD PTR [ebp+8]
	add	edi, DWORD PTR [edx+3148076]
	mov	DWORD PTR [eax-28], edi
	mov	edx, DWORD PTR [esi]
	jmp	L68
	.p2align 2,,3
L74:
	mov	esi, DWORD PTR [ebp-44]
	cmp	DWORD PTR [ebp-40], esi
	je	L47
	mov	cl, BYTE PTR [ebp-36]
	mov	esi, DWORD PTR [ebp-76]
	mov	BYTE PTR [esi], cl
	inc	esi
	mov	DWORD PTR [ebp-76], esi
	mov	ecx, DWORD PTR [ebp-40]
	mov	DWORD PTR [ebp-44], ecx
L47:
	mov	esi, DWORD PTR [ebp-76]
	mov	BYTE PTR [esi], al
	mov	esi, DWORD PTR [ebx]
	lea	esi, [esi+esi*2]
	add	esi, DWORD PTR [edx-8]
	mov	cl, BYTE PTR [esi]
	mov	BYTE PTR [ebp-28], cl
	mov	cl, BYTE PTR [edx+4]
	add	BYTE PTR [ebp-28], cl
	mov	cl, BYTE PTR [ebp-28]
	mov	BYTE PTR [edx+4], cl
	mov	esi, DWORD PTR [ebp-76]
	mov	BYTE PTR [esi+1], cl
	add	esi, 2
	mov	DWORD PTR [ebp-76], esi
	mov	esi, DWORD PTR [edx]
	inc	esi
	mov	DWORD PTR [ebp-32], esi
	mov	DWORD PTR [edx], esi
	mov	ecx, DWORD PTR [edx-8]
	mov	DWORD PTR [ebp-52], ecx
	mov	esi, ecx
	inc	esi
	mov	DWORD PTR [edx-8], esi
	mov	ecx, DWORD PTR [ebx]
	mov	DWORD PTR [ebp-28], ecx
	cmp	DWORD PTR [ebp-32], ecx
	jae	L48
	movzx	ecx, BYTE PTR [esi+ecx]
	sal	ecx, 8
	mov	DWORD PTR [ebp-64], ecx
	mov	ecx, DWORD PTR [ebp-28]
	movzx	esi, BYTE PTR [esi+ecx*2]
	sal	esi, 16
	add	esi, DWORD PTR [ebp-64]
	mov	DWORD PTR [ebp-28], esi
	mov	ecx, DWORD PTR [ebp-52]
	movzx	esi, BYTE PTR [ecx+1]
	add	esi, DWORD PTR [ebp-28]
	je	L48
	add	esi, DWORD PTR [edi+3148076]
	mov	DWORD PTR [edx-4], esi
L48:
	mov	esi, DWORD PTR [ebp-32]
	cmp	DWORD PTR [ebx], esi
	jbe	L45
	mov	esi, DWORD PTR [edx-4]
	jmp	L46
	.p2align 2,,3
L73:
	mov	bl, BYTE PTR [ebp-48]
	or	ebx, -64
	movzx	edi, bl
	cmp	edi, DWORD PTR [ebp-44]
	je	L43
	mov	BYTE PTR [ecx], bl
	inc	ecx
	mov	edx, DWORD PTR [esi+8]
	mov	DWORD PTR [ebp-44], edi
L43:
	lea	edx, [edx+edx*2]
	add	edx, DWORD PTR [eax-20]
	mov	dl, BYTE PTR [edx]
	add	edx, DWORD PTR [eax-8]
	mov	BYTE PTR [eax-8], dl
	mov	BYTE PTR [ecx], dl
	inc	ecx
	mov	ebx, DWORD PTR [eax-16]
	inc	ebx
	mov	DWORD PTR [eax-16], ebx
	mov	edx, DWORD PTR [eax-20]
	mov	DWORD PTR [ebp-32], edx
	inc	edx
	mov	DWORD PTR [eax-20], edx
	mov	edi, DWORD PTR [esi+8]
	mov	DWORD PTR [ebp-36], edi
	cmp	ebx, edi
	jae	L44
	movzx	edi, BYTE PTR [edx+edi]
	sal	edi, 8
	mov	DWORD PTR [ebp-28], edi
	mov	edi, DWORD PTR [ebp-36]
	movzx	edx, BYTE PTR [edx+edi*2]
	sal	edx, 16
	add	edx, DWORD PTR [ebp-28]
	mov	DWORD PTR [ebp-28], edx
	mov	edi, DWORD PTR [ebp-32]
	movzx	edx, BYTE PTR [edi+1]
	add	edx, DWORD PTR [ebp-28]
	jne	L78
L44:
	cmp	DWORD PTR [esi+8], ebx
	jbe	L41
	mov	ebx, DWORD PTR [eax-12]
	jmp	L42
	.p2align 2,,3
L75:
	mov	bl, BYTE PTR [ebp-48]
	or	ebx, -32
	movzx	edx, bl
	cmp	edx, DWORD PTR [ebp-44]
	je	L52
	mov	BYTE PTR [ecx], bl
	inc	ecx
	mov	DWORD PTR [ebp-44], edx
L52:
	mov	edx, DWORD PTR [esi+8]
	lea	edx, [edx+edx*2]
	add	edx, DWORD PTR [eax-4]
	mov	dl, BYTE PTR [edx]
	add	edx, DWORD PTR [eax+8]
	mov	BYTE PTR [eax+8], dl
	mov	BYTE PTR [ecx], dl
	mov	ebx, DWORD PTR [esi+8]
	mov	edx, DWORD PTR [eax-4]
	mov	dl, BYTE PTR [edx+ebx*4]
	add	dl, BYTE PTR [eax+9]
	mov	BYTE PTR [eax+9], dl
	mov	BYTE PTR [ecx+1], dl
	add	ecx, 2
	mov	ebx, DWORD PTR [eax]
	inc	ebx
	mov	DWORD PTR [eax], ebx
	mov	edi, DWORD PTR [eax-4]
	mov	DWORD PTR [ebp-32], edi
	mov	edx, edi
	inc	edx
	mov	DWORD PTR [eax-4], edx
	mov	edi, DWORD PTR [esi+16]
	mov	DWORD PTR [ebp-36], edi
	cmp	ebx, edi
	jae	L53
	movzx	edi, BYTE PTR [edx+edi]
	sal	edi, 8
	mov	DWORD PTR [ebp-28], edi
	mov	edi, DWORD PTR [ebp-36]
	movzx	edx, BYTE PTR [edx+edi*2]
	sal	edx, 16
	add	edx, DWORD PTR [ebp-28]
	mov	DWORD PTR [ebp-28], edx
	mov	edi, DWORD PTR [ebp-32]
	movzx	edx, BYTE PTR [edi+1]
	add	edx, DWORD PTR [ebp-28]
	jne	L79
L53:
	cmp	ebx, DWORD PTR [esi+16]
	jae	L50
	mov	edx, DWORD PTR [eax+4]
	jmp	L51
L72:
	mov	eax, DWORD PTR [edi+3148088]
	mov	DWORD PTR [ebp-40], eax
	cmp	DWORD PTR [ebp-32], eax
	je	L80
	mov	eax, -1
L38:
	cmp	DWORD PTR [ebp-40], eax
	jae	L37
	mov	edx, DWORD PTR [ebp-40]
	mov	eax, DWORD PTR [ebp+8]
	mov	DWORD PTR [eax+3148080], edx
	jmp	L37
L79:
	mov	edi, DWORD PTR [ebp+8]
	add	edx, DWORD PTR [edi+3148076]
	mov	DWORD PTR [eax+4], edx
	jmp	L53
L78:
	mov	edi, DWORD PTR [ebp+8]
	add	edx, DWORD PTR [edi+3148076]
	mov	DWORD PTR [eax-12], edx
	jmp	L44
L80:
	mov	edx, DWORD PTR [edi+3148084]
	mov	DWORD PTR [ebp-32], edx
	lea	edi, [ebx+ebx]
	mov	DWORD PTR [ebp-28], edi
	mov	eax, edi
	add	eax, ebx
	lea	eax, [eax+esi*4]
	mov	DWORD PTR [ebp-44], eax
	mov	edx, DWORD PTR [ebp+8]
	mov	eax, DWORD PTR [edx+3150700]
	mov	edi, 1374389535
	mul	edi
	shr	edx, 5
	mov	eax, DWORD PTR [ebp-32]
	mov	edi, DWORD PTR [ebp-44]
	imul	edx, DWORD PTR [eax+edi]
	mov	edi, DWORD PTR [ebp+8]
	mov	DWORD PTR [edi+3148096], edx
	lea	edx, [0+ebx*8]
	mov	eax, edx
	sub	eax, ebx
	add	eax, esi
	mov	edi, DWORD PTR [ebp-32]
	movzx	edi, BYTE PTR [edi+eax]
	mov	eax, DWORD PTR [ebp+8]
	mov	DWORD PTR [eax+3148100], edi
	lea	edi, [edx+esi]
	mov	eax, DWORD PTR [ebp-32]
	movzx	eax, BYTE PTR [eax+edi]
	mov	edi, DWORD PTR [ebp+8]
	mov	DWORD PTR [edi+3148104], eax
	lea	eax, [edx+ebx]
	add	eax, esi
	mov	edx, DWORD PTR [ebp-32]
	movzx	eax, BYTE PTR [edx+eax]
	mov	DWORD PTR [edi+3148108], eax
	lea	eax, [esi+1]
	mov	DWORD PTR [edi+3148092], eax
	cmp	ebx, eax
	jbe	L37
	add	ebx, edx
	movzx	edx, BYTE PTR [ebx+eax]
	sal	edx, 8
	add	eax, DWORD PTR [ebp-28]
	mov	ebx, DWORD PTR [ebp-32]
	movzx	eax, BYTE PTR [ebx+eax]
	sal	eax, 16
	lea	eax, [edx+eax]
	movzx	edx, BYTE PTR [ebx+1+esi]
	add	eax, edx
	je	L39
	mov	edi, DWORD PTR [ebp-40]
	lea	edi, [eax+edi]
	mov	esi, DWORD PTR [ebp+8]
	mov	DWORD PTR [esi+3148088], edi
	mov	DWORD PTR [ebp-40], edi
L39:
	mov	edi, DWORD PTR [ebp+8]
	mov	eax, DWORD PTR [edi+3148080]
	jmp	L38
L71:
	mov	ecx, DWORD PTR [eax+3148116]
	jmp	L33
LFE8:
	.align 2
	.p2align 2,,3
.globl __ZN9V2MPlayer4OpenEPKvm
	.def	__ZN9V2MPlayer4OpenEPKvm;	.scl	2;	.type	32;	.endef
__ZN9V2MPlayer4OpenEPKvm:
LFB9:
	push	ebp
LCFI46:
	mov	ebp, esp
LCFI47:
	push	ebx
LCFI48:
	sub	esp, 20
LCFI49:
	mov	ebx, DWORD PTR [ebp+8]
	mov	edx, DWORD PTR [ebx+3145732]
	test	edx, edx
	je	L82
	mov	eax, DWORD PTR [ebx+3148072]
	test	eax, eax
	je	L83
	mov	DWORD PTR [ebx+3148072], 0
L83:
	mov	DWORD PTR [ebx+3145732], 0
L82:
	mov	eax, DWORD PTR [ebp+16]
	mov	DWORD PTR [ebx+3150700], eax
	mov	eax, DWORD PTR [ebp+12]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], ebx
	call	__ZN9V2MPlayer8InitBaseEPKv
	test	eax, eax
	jne	L86
	xor	eax, eax
	add	esp, 20
	pop	ebx
LCFI50:
	leave
LCFI51:
	ret
	.p2align 2,,3
L86:
LCFI52:
	mov	DWORD PTR [esp], ebx
	call	__ZN9V2MPlayer5ResetEv
	mov	DWORD PTR [ebx+3145732], 1
	mov	eax, 1
	add	esp, 20
	pop	ebx
LCFI53:
	leave
LCFI54:
	ret
LFE9:
	.align 2
	.p2align 2,,3
.globl __ZN9V2MPlayer5CloseEv
	.def	__ZN9V2MPlayer5CloseEv;	.scl	2;	.type	32;	.endef
__ZN9V2MPlayer5CloseEv:
LFB10:
	push	ebp
LCFI55:
	mov	ebp, esp
LCFI56:
	mov	eax, DWORD PTR [ebp+8]
	mov	edx, DWORD PTR [eax+3145732]
	test	edx, edx
	je	L87
	mov	ecx, DWORD PTR [eax+3148072]
	test	ecx, ecx
	je	L89
	mov	DWORD PTR [eax+3148072], 0
L89:
	mov	DWORD PTR [eax+3145732], 0
L87:
	leave
LCFI57:
	ret
LFE10:
	.align 2
	.p2align 2,,3
.globl __ZN9V2MPlayer4PlayEm
	.def	__ZN9V2MPlayer4PlayEm;	.scl	2;	.type	32;	.endef
__ZN9V2MPlayer4PlayEm:
LFB11:
	push	ebp
LCFI58:
	mov	ebp, esp
LCFI59:
	push	ebx
LCFI60:
	sub	esp, 20
LCFI61:
	mov	ebx, DWORD PTR [ebp+8]
	mov	eax, DWORD PTR [ebx+3145732]
	test	eax, eax
	je	L90
	mov	ecx, DWORD PTR [ebx+3150700]
	test	ecx, ecx
	je	L90
	mov	DWORD PTR [ebx+3148072], 0
	mov	DWORD PTR [esp], ebx
	call	__ZN9V2MPlayer5ResetEv
	mov	DWORD PTR [ebx+3148072], 2
	mov	DWORD PTR [ebx+3150688], 0
	mov	DWORD PTR [ebx+3150692], 0
	xor	eax, eax
	sub	eax, DWORD PTR [ebx+3150684]
	mov	DWORD PTR [ebx+3150704], eax
	mov	DWORD PTR [ebx+3154804], 0x3f800000
	mov	DWORD PTR [ebx+3154808], 0x00000000
	mov	DWORD PTR [ebx+3145732], 1
L90:
	add	esp, 20
	pop	ebx
LCFI62:
	leave
LCFI63:
	ret
LFE11:
	.align 2
	.p2align 2,,3
.globl __ZN9V2MPlayer4StopEm
	.def	__ZN9V2MPlayer4StopEm;	.scl	2;	.type	32;	.endef
__ZN9V2MPlayer4StopEm:
LFB12:
	push	ebp
LCFI64:
	mov	ebp, esp
LCFI65:
	sub	esp, 8
LCFI66:
	mov	eax, DWORD PTR [ebp+8]
	mov	ecx, DWORD PTR [eax+3145732]
	test	ecx, ecx
	je	L96
	mov	edx, DWORD PTR [ebp+12]
	test	edx, edx
	jne	L99
	mov	DWORD PTR [eax+3148072], 0
L96:
	leave
LCFI67:
	ret
	.p2align 2,,3
L99:
LCFI68:
	xor	edx, edx
	xor	ecx, ecx
	mov	DWORD PTR [ebp-8], edx
	mov	DWORD PTR [ebp-4], ecx
	fild	QWORD PTR [ebp-8]
	fdivr	DWORD PTR [eax+3154804]
	fstp	DWORD PTR [eax+3154808]
	leave
LCFI69:
	ret
LFE12:
	.align 2
	.p2align 2,,3
.globl __ZN9V2MPlayer6RenderEPfmi
	.def	__ZN9V2MPlayer6RenderEPfmi;	.scl	2;	.type	32;	.endef
__ZN9V2MPlayer6RenderEPfmi:
LFB13:
	push	ebp
LCFI70:
	mov	ebp, esp
LCFI71:
	push	edi
LCFI72:
	push	esi
LCFI73:
	push	ebx
LCFI74:
	sub	esp, 76
LCFI75:
	mov	ebx, DWORD PTR [ebp+8]
	mov	esi, DWORD PTR [ebp+12]
	mov	edi, DWORD PTR [ebp+16]
	test	esi, esi
	je	L100
	mov	eax, DWORD PTR [ebx+3145732]
	test	eax, eax
	jne	L120
	mov	edx, DWORD PTR [ebx+3148072]
L103:
	test	edx, edx
	je	L110
	test	eax, eax
	jne	L111
L110:
	mov	eax, DWORD PTR [ebp+20]
	test	eax, eax
	jne	L104
L104:
	fld	DWORD PTR [ebx+3154808]
	fldz
	fxch	st(1)
	fucomp	st(1)
	fnstsw	ax
	and	ah, 69
	cmp	ah, 64
	je	L126
	test	edi, edi
	je	L121
	fld	DWORD PTR [ebx+3154804]
	fxch	st(1)
	xor	edx, edx
	fst	DWORD PTR [ebp-44]
	fxch	st(1)
	mov	ecx, DWORD PTR [ebp-44]
	jmp	L116
	.p2align 2,,3
L127:
	fxch	st(1)
L116:
	fmul	DWORD PTR [esi+edx*8]
	fstp	DWORD PTR [esi+edx*8]
	fld	DWORD PTR [esi+4+edx*8]
	fmul	DWORD PTR [ebx+3154804]
	fstp	DWORD PTR [esi+4+edx*8]
	fld	DWORD PTR [ebx+3154804]
	fsub	DWORD PTR [ebx+3154808]
	fst	DWORD PTR [ebx+3154804]
	fxch	st(1)
	fucom	st(1)
	fnstsw	ax
	test	ah, 69
	jne	L114
	fstp	st(1)
	mov	DWORD PTR [ebx+3154804], 0x00000000
	mov	DWORD PTR [ebp-44], ecx
	fld	DWORD PTR [ebp-44]
	fxch	st(1)
L114:
	inc	edx
	cmp	edi, edx
	ja	L127
	fstp	st(0)
L113:
	fldz
	fxch	st(1)
	fucompp
	fnstsw	ax
	and	ah, 69
	xor	ah, 64
	jne	L100
	mov	esi, DWORD PTR [ebx+3145732]
	test	esi, esi
	je	L100
	mov	DWORD PTR [ebx+3148072], 0
	jmp	L100
	.p2align 2,,3
L126:
	fstp	st(0)
L100:
	lea	esp, [ebp-12]
	pop	ebx
LCFI76:
	pop	esi
LCFI77:
	pop	edi
LCFI78:
	leave
LCFI79:
	ret
	.p2align 2,,3
L120:
LCFI80:
	mov	edx, DWORD PTR [ebx+3148072]
	cmp	edx, 2
	jne	L103
	test	edi, edi
	je	L104
	mov	DWORD PTR [ebp-28], esi
	mov	esi, ebx
	mov	DWORD PTR [ebp-32], edi
	jmp	L109
	.p2align 2,,3
L125:
	mov	ebx, eax
	test	ebx, ebx
	jne	L122
L107:
	test	eax, eax
	je	L123
L108:
	test	edi, edi
	je	L124
L109:
	mov	eax, DWORD PTR [esi+3150688]
	cmp	eax, edi
	jb	L125
	mov	ebx, edi
	test	ebx, ebx
	je	L107
L122:
	mov	eax, DWORD PTR [ebp+20]
	mov	DWORD PTR [esp+16], eax
	mov	DWORD PTR [esp+12], 0
	mov	DWORD PTR [esp+8], ebx
	mov	eax, DWORD PTR [ebp-28]
	mov	DWORD PTR [esp+4], eax
	mov	DWORD PTR [esp], esi
	call	_synthRender@20
	sub	esp, 20
	mov	eax, DWORD PTR [ebp-28]
	lea	eax, [eax+ebx*8]
	mov	DWORD PTR [ebp-28], eax
	sub	edi, ebx
	mov	eax, DWORD PTR [esi+3150688]
	sub	eax, ebx
	mov	DWORD PTR [esi+3150688], eax
	add	DWORD PTR [esi+3150684], ebx
	test	eax, eax
	jne	L108
L123:
	mov	DWORD PTR [esp], esi
	call	__ZN9V2MPlayer4TickEv
	cmp	DWORD PTR [esi+3148072], 2
	je	L108
	mov	DWORD PTR [esi+3150688], -1
	test	edi, edi
	jne	L109
L124:
	mov	ebx, esi
	mov	esi, DWORD PTR [ebp-28]
	mov	edi, DWORD PTR [ebp-32]
	jmp	L104
	.p2align 2,,3
L111:
	mov	eax, DWORD PTR [ebp+20]
	mov	DWORD PTR [esp+16], eax
	mov	DWORD PTR [esp+12], 0
	mov	DWORD PTR [esp+8], edi
	mov	DWORD PTR [esp+4], esi
	mov	DWORD PTR [esp], ebx
	call	_synthRender@20
	sub	esp, 20
	add	DWORD PTR [ebx+3150684], edi
	jmp	L104
L121:
	fstp	st(0)
	fld	DWORD PTR [ebx+3154804]
	jmp	L113
LFE13:
	.align 2
	.p2align 2,,3
.globl __ZN9V2MPlayer9IsPlayingEv
	.def	__ZN9V2MPlayer9IsPlayingEv;	.scl	2;	.type	32;	.endef
__ZN9V2MPlayer9IsPlayingEv:
LFB14:
	push	ebp
LCFI81:
	mov	ebp, esp
LCFI82:
	mov	eax, DWORD PTR [ebp+8]
	mov	edx, DWORD PTR [eax+3145732]
	test	edx, edx
	je	L130
	cmp	DWORD PTR [eax+3148072], 2
	sete	al
	movzx	eax, al
	leave
LCFI83:
	ret
	.p2align 2,,3
L130:
LCFI84:
	xor	eax, eax
	leave
LCFI85:
	ret
LFE14:
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
	.long	LFB7
	.long	LFE7-LFB7
	.byte	0x4
	.long	LCFI20-LFB7
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI21-LCFI20
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI25-LCFI21
	.byte	0x83
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x4
	.byte	0x87
	.uleb128 0x3
	.byte	0x4
	.long	LCFI26-LCFI25
	.byte	0xc3
	.byte	0x4
	.long	LCFI27-LCFI26
	.byte	0xc6
	.byte	0x4
	.long	LCFI28-LCFI27
	.byte	0xc7
	.byte	0x4
	.long	LCFI29-LCFI28
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
	.long	LFB8
	.long	LFE8-LFB8
	.byte	0x4
	.long	LCFI30-LFB8
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI31-LCFI30
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI35-LCFI31
	.byte	0x83
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x4
	.byte	0x87
	.uleb128 0x3
	.byte	0x4
	.long	LCFI36-LCFI35
	.byte	0xa
	.byte	0xc3
	.byte	0x4
	.long	LCFI37-LCFI36
	.byte	0xc6
	.byte	0x4
	.long	LCFI38-LCFI37
	.byte	0xc7
	.byte	0x4
	.long	LCFI39-LCFI38
	.byte	0xc5
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x4
	.long	LCFI40-LCFI39
	.byte	0xb
	.byte	0x4
	.long	LCFI41-LCFI40
	.byte	0xa
	.byte	0xc3
	.byte	0x4
	.long	LCFI42-LCFI41
	.byte	0xc6
	.byte	0x4
	.long	LCFI43-LCFI42
	.byte	0xc7
	.byte	0x4
	.long	LCFI44-LCFI43
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.byte	0xc5
	.byte	0x4
	.long	LCFI45-LCFI44
	.byte	0xb
	.align 4
LEFDE3:
LSFDE5:
	.long	LEFDE5-LASFDE5
LASFDE5:
	.long	LASFDE5-Lframe1
	.long	LFB9
	.long	LFE9-LFB9
	.byte	0x4
	.long	LCFI46-LFB9
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI47-LCFI46
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI49-LCFI47
	.byte	0x83
	.uleb128 0x3
	.byte	0x4
	.long	LCFI50-LCFI49
	.byte	0xa
	.byte	0xc3
	.byte	0x4
	.long	LCFI51-LCFI50
	.byte	0xc5
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x4
	.long	LCFI52-LCFI51
	.byte	0xb
	.byte	0x4
	.long	LCFI53-LCFI52
	.byte	0xc3
	.byte	0x4
	.long	LCFI54-LCFI53
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.byte	0xc5
	.align 4
LEFDE5:
LSFDE7:
	.long	LEFDE7-LASFDE7
LASFDE7:
	.long	LASFDE7-Lframe1
	.long	LFB11
	.long	LFE11-LFB11
	.byte	0x4
	.long	LCFI58-LFB11
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI59-LCFI58
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI61-LCFI59
	.byte	0x83
	.uleb128 0x3
	.byte	0x4
	.long	LCFI62-LCFI61
	.byte	0xc3
	.byte	0x4
	.long	LCFI63-LCFI62
	.byte	0xc5
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.align 4
LEFDE7:
LSFDE9:
	.long	LEFDE9-LASFDE9
LASFDE9:
	.long	LASFDE9-Lframe1
	.long	LFB13
	.long	LFE13-LFB13
	.byte	0x4
	.long	LCFI70-LFB13
	.byte	0xe
	.uleb128 0x8
	.byte	0x85
	.uleb128 0x2
	.byte	0x4
	.long	LCFI71-LCFI70
	.byte	0xd
	.uleb128 0x5
	.byte	0x4
	.long	LCFI75-LCFI71
	.byte	0x83
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x4
	.byte	0x87
	.uleb128 0x3
	.byte	0x4
	.long	LCFI76-LCFI75
	.byte	0xa
	.byte	0xc3
	.byte	0x4
	.long	LCFI77-LCFI76
	.byte	0xc6
	.byte	0x4
	.long	LCFI78-LCFI77
	.byte	0xc7
	.byte	0x4
	.long	LCFI79-LCFI78
	.byte	0xc5
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x4
	.long	LCFI80-LCFI79
	.byte	0xb
	.align 4
LEFDE9:
	.def	_synthInit@12;	.scl	2;	.type	32;	.endef
	.def	_synthSetGlobals@8;	.scl	2;	.type	32;	.endef
	.def	_synthSetLyrics@8;	.scl	2;	.type	32;	.endef
	.def	_synthProcessMIDI@8;	.scl	2;	.type	32;	.endef
	.def	_synthRender@20;	.scl	2;	.type	32;	.endef
