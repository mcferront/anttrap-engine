; Listing generated by Microsoft (R) Optimizing Compiler Version 19.15.26730.0 

	TITLE	E:\git\mcferront\anttrap-engine\3rdParty\zlib\zlib-1.2.5\adler32.c
	.686P
	.XMM
	include listing.inc
	.model	flat

INCLUDELIB LIBCMTD
INCLUDELIB OLDNAMES

PUBLIC	_adler32@12
PUBLIC	_adler32_combine@12
PUBLIC	_adler32_combine64@12
; Function compile flags: /Odtp /ZI
; File e:\git\mcferront\anttrap-engine\3rdparty\zlib\zlib-1.2.5\adler32.c
;	COMDAT _adler32_combine_
_TEXT	SEGMENT
_rem$ = -12						; size = 4
_sum2$ = -8						; size = 4
_sum1$ = -4						; size = 4
_adler1$ = 8						; size = 4
_adler2$ = 12						; size = 4
_len2$ = 16						; size = 4
_adler32_combine_ PROC					; COMDAT

; 135  : {

	push	ebp
	mov	ebp, esp
	sub	esp, 76					; 0000004cH
	push	ebx
	push	esi
	push	edi

; 136  :     unsigned long sum1;
; 137  :     unsigned long sum2;
; 138  :     unsigned rem;
; 139  : 
; 140  :     /* the derivation of this formula is left as an exercise for the reader */
; 141  :     rem = (unsigned)(len2 % BASE);

	mov	eax, DWORD PTR _len2$[ebp]
	xor	edx, edx
	mov	ecx, 65521				; 0000fff1H
	div	ecx
	mov	DWORD PTR _rem$[ebp], edx

; 142  :     sum1 = adler1 & 0xffff;

	mov	eax, DWORD PTR _adler1$[ebp]
	and	eax, 65535				; 0000ffffH
	mov	DWORD PTR _sum1$[ebp], eax

; 143  :     sum2 = rem * sum1;

	mov	eax, DWORD PTR _rem$[ebp]
	imul	eax, DWORD PTR _sum1$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax

; 144  :     MOD(sum2);

	mov	eax, DWORD PTR _sum2$[ebp]
	xor	edx, edx
	mov	ecx, 65521				; 0000fff1H
	div	ecx
	mov	DWORD PTR _sum2$[ebp], edx

; 145  :     sum1 += (adler2 & 0xffff) + BASE - 1;

	mov	eax, DWORD PTR _adler2$[ebp]
	and	eax, 65535				; 0000ffffH
	mov	ecx, DWORD PTR _sum1$[ebp]
	lea	edx, DWORD PTR [ecx+eax+65520]
	mov	DWORD PTR _sum1$[ebp], edx

; 146  :     sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + BASE - rem;

	mov	eax, DWORD PTR _adler1$[ebp]
	shr	eax, 16					; 00000010H
	and	eax, 65535				; 0000ffffH
	mov	ecx, DWORD PTR _adler2$[ebp]
	shr	ecx, 16					; 00000010H
	and	ecx, 65535				; 0000ffffH
	lea	edx, DWORD PTR [eax+ecx+65521]
	sub	edx, DWORD PTR _rem$[ebp]
	add	edx, DWORD PTR _sum2$[ebp]
	mov	DWORD PTR _sum2$[ebp], edx

; 147  :     if (sum1 >= BASE) sum1 -= BASE;

	cmp	DWORD PTR _sum1$[ebp], 65521		; 0000fff1H
	jb	SHORT $LN2@adler32_co
	mov	eax, DWORD PTR _sum1$[ebp]
	sub	eax, 65521				; 0000fff1H
	mov	DWORD PTR _sum1$[ebp], eax
$LN2@adler32_co:

; 148  :     if (sum1 >= BASE) sum1 -= BASE;

	cmp	DWORD PTR _sum1$[ebp], 65521		; 0000fff1H
	jb	SHORT $LN3@adler32_co
	mov	eax, DWORD PTR _sum1$[ebp]
	sub	eax, 65521				; 0000fff1H
	mov	DWORD PTR _sum1$[ebp], eax
$LN3@adler32_co:

; 149  :     if (sum2 >= (BASE << 1)) sum2 -= (BASE << 1);

	cmp	DWORD PTR _sum2$[ebp], 131042		; 0001ffe2H
	jb	SHORT $LN4@adler32_co
	mov	eax, DWORD PTR _sum2$[ebp]
	sub	eax, 131042				; 0001ffe2H
	mov	DWORD PTR _sum2$[ebp], eax
$LN4@adler32_co:

; 150  :     if (sum2 >= BASE) sum2 -= BASE;

	cmp	DWORD PTR _sum2$[ebp], 65521		; 0000fff1H
	jb	SHORT $LN5@adler32_co
	mov	eax, DWORD PTR _sum2$[ebp]
	sub	eax, 65521				; 0000fff1H
	mov	DWORD PTR _sum2$[ebp], eax
$LN5@adler32_co:

; 151  :     return sum1 | (sum2 << 16);

	mov	eax, DWORD PTR _sum2$[ebp]
	shl	eax, 16					; 00000010H
	or	eax, DWORD PTR _sum1$[ebp]

; 152  : }

	pop	edi
	pop	esi
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret	0
_adler32_combine_ ENDP
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File e:\git\mcferront\anttrap-engine\3rdparty\zlib\zlib-1.2.5\adler32.c
;	COMDAT _adler32_combine64@12
_TEXT	SEGMENT
_adler1$ = 8						; size = 4
_adler2$ = 12						; size = 4
_len2$ = 16						; size = 4
_adler32_combine64@12 PROC				; COMDAT

; 167  : {

	push	ebp
	mov	ebp, esp
	sub	esp, 64					; 00000040H
	push	ebx
	push	esi
	push	edi

; 168  :     return adler32_combine_(adler1, adler2, len2);

	mov	eax, DWORD PTR _len2$[ebp]
	push	eax
	mov	ecx, DWORD PTR _adler2$[ebp]
	push	ecx
	mov	edx, DWORD PTR _adler1$[ebp]
	push	edx
	call	_adler32_combine_
	add	esp, 12					; 0000000cH

; 169  : }

	pop	edi
	pop	esi
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret	12					; 0000000cH
_adler32_combine64@12 ENDP
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File e:\git\mcferront\anttrap-engine\3rdparty\zlib\zlib-1.2.5\adler32.c
;	COMDAT _adler32_combine@12
_TEXT	SEGMENT
_adler1$ = 8						; size = 4
_adler2$ = 12						; size = 4
_len2$ = 16						; size = 4
_adler32_combine@12 PROC				; COMDAT

; 159  : {

	push	ebp
	mov	ebp, esp
	sub	esp, 64					; 00000040H
	push	ebx
	push	esi
	push	edi

; 160  :     return adler32_combine_(adler1, adler2, len2);

	mov	eax, DWORD PTR _len2$[ebp]
	push	eax
	mov	ecx, DWORD PTR _adler2$[ebp]
	push	ecx
	mov	edx, DWORD PTR _adler1$[ebp]
	push	edx
	call	_adler32_combine_
	add	esp, 12					; 0000000cH

; 161  : }

	pop	edi
	pop	esi
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret	12					; 0000000cH
_adler32_combine@12 ENDP
_TEXT	ENDS
; Function compile flags: /Odtp /ZI
; File e:\git\mcferront\anttrap-engine\3rdparty\zlib\zlib-1.2.5\adler32.c
;	COMDAT _adler32@12
_TEXT	SEGMENT
tv298 = -76						; size = 4
tv83 = -76						; size = 4
_n$ = -8						; size = 4
_sum2$ = -4						; size = 4
_adler$ = 8						; size = 4
_buf$ = 12						; size = 4
_len$ = 16						; size = 4
_adler32@12 PROC					; COMDAT

; 64   : {

	push	ebp
	mov	ebp, esp
	sub	esp, 76					; 0000004cH
	push	ebx
	push	esi
	push	edi

; 65   :     unsigned long sum2;
; 66   :     unsigned n;
; 67   : 
; 68   :     /* split Adler-32 into component sums */
; 69   :     sum2 = (adler >> 16) & 0xffff;

	mov	eax, DWORD PTR _adler$[ebp]
	shr	eax, 16					; 00000010H
	and	eax, 65535				; 0000ffffH
	mov	DWORD PTR _sum2$[ebp], eax

; 70   :     adler &= 0xffff;

	mov	eax, DWORD PTR _adler$[ebp]
	and	eax, 65535				; 0000ffffH
	mov	DWORD PTR _adler$[ebp], eax

; 71   : 
; 72   :     /* in case user likes doing a byte at a time, keep it fast */
; 73   :     if (len == 1) {

	cmp	DWORD PTR _len$[ebp], 1
	jne	SHORT $LN13@adler32

; 74   :         adler += buf[0];

	mov	eax, 1
	imul	ecx, eax, 0
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax

; 75   :         if (adler >= BASE)

	cmp	DWORD PTR _adler$[ebp], 65521		; 0000fff1H
	jb	SHORT $LN14@adler32

; 76   :             adler -= BASE;

	mov	eax, DWORD PTR _adler$[ebp]
	sub	eax, 65521				; 0000fff1H
	mov	DWORD PTR _adler$[ebp], eax
$LN14@adler32:

; 77   :         sum2 += adler;

	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax

; 78   :         if (sum2 >= BASE)

	cmp	DWORD PTR _sum2$[ebp], 65521		; 0000fff1H
	jb	SHORT $LN15@adler32

; 79   :             sum2 -= BASE;

	mov	eax, DWORD PTR _sum2$[ebp]
	sub	eax, 65521				; 0000fff1H
	mov	DWORD PTR _sum2$[ebp], eax
$LN15@adler32:

; 80   :         return adler | (sum2 << 16);

	mov	eax, DWORD PTR _sum2$[ebp]
	shl	eax, 16					; 00000010H
	or	eax, DWORD PTR _adler$[ebp]
	jmp	$LN1@adler32
$LN13@adler32:

; 81   :     }
; 82   : 
; 83   :     /* initial Adler-32 value (deferred check for len == 1 speed) */
; 84   :     if (buf == Z_NULL)

	cmp	DWORD PTR _buf$[ebp], 0
	jne	SHORT $LN16@adler32

; 85   :         return 1L;

	mov	eax, 1
	jmp	$LN1@adler32
$LN16@adler32:

; 86   : 
; 87   :     /* in case short lengths are provided, keep it somewhat fast */
; 88   :     if (len < 16) {

	cmp	DWORD PTR _len$[ebp], 16		; 00000010H
	jae	SHORT $LN4@adler32
$LN2@adler32:

; 89   :         while (len--) {

	mov	eax, DWORD PTR _len$[ebp]
	mov	DWORD PTR tv83[ebp], eax
	mov	ecx, DWORD PTR _len$[ebp]
	sub	ecx, 1
	mov	DWORD PTR _len$[ebp], ecx
	cmp	DWORD PTR tv83[ebp], 0
	je	SHORT $LN3@adler32

; 90   :             adler += *buf++;

	mov	eax, DWORD PTR _buf$[ebp]
	movzx	ecx, BYTE PTR [eax]
	add	ecx, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], ecx
	mov	edx, DWORD PTR _buf$[ebp]
	add	edx, 1
	mov	DWORD PTR _buf$[ebp], edx

; 91   :             sum2 += adler;

	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax

; 92   :         }

	jmp	SHORT $LN2@adler32
$LN3@adler32:

; 93   :         if (adler >= BASE)

	cmp	DWORD PTR _adler$[ebp], 65521		; 0000fff1H
	jb	SHORT $LN18@adler32

; 94   :             adler -= BASE;

	mov	eax, DWORD PTR _adler$[ebp]
	sub	eax, 65521				; 0000fff1H
	mov	DWORD PTR _adler$[ebp], eax
$LN18@adler32:

; 95   :         MOD4(sum2);             /* only added so many BASE's */

	mov	eax, DWORD PTR _sum2$[ebp]
	xor	edx, edx
	mov	ecx, 65521				; 0000fff1H
	div	ecx
	mov	DWORD PTR _sum2$[ebp], edx

; 96   :         return adler | (sum2 << 16);

	mov	eax, DWORD PTR _sum2$[ebp]
	shl	eax, 16					; 00000010H
	or	eax, DWORD PTR _adler$[ebp]
	jmp	$LN1@adler32
$LN4@adler32:

; 97   :     }
; 98   : 
; 99   :     /* do length NMAX blocks -- requires just one modulo operation */
; 100  :     while (len >= NMAX) {

	cmp	DWORD PTR _len$[ebp], 5552		; 000015b0H
	jb	$LN5@adler32

; 101  :         len -= NMAX;

	mov	eax, DWORD PTR _len$[ebp]
	sub	eax, 5552				; 000015b0H
	mov	DWORD PTR _len$[ebp], eax

; 102  :         n = NMAX / 16;          /* NMAX is divisible by 16 */

	mov	DWORD PTR _n$[ebp], 347			; 0000015bH
$LN8@adler32:

; 103  :         do {
; 104  :             DO16(buf);          /* 16 sums unrolled */

	mov	eax, 1
	imul	ecx, eax, 0
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	shl	eax, 0
	mov	ecx, DWORD PTR _buf$[ebp]
	movzx	edx, BYTE PTR [ecx+eax]
	add	edx, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], edx
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	shl	eax, 1
	mov	ecx, DWORD PTR _buf$[ebp]
	movzx	edx, BYTE PTR [ecx+eax]
	add	edx, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], edx
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 3
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	shl	eax, 2
	mov	ecx, DWORD PTR _buf$[ebp]
	movzx	edx, BYTE PTR [ecx+eax]
	add	edx, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], edx
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 5
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 6
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 7
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	shl	eax, 3
	mov	ecx, DWORD PTR _buf$[ebp]
	movzx	edx, BYTE PTR [ecx+eax]
	add	edx, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], edx
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 9
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 10
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 11
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 12
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 13
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 14
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 15
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax

; 105  :             buf += 16;

	mov	eax, DWORD PTR _buf$[ebp]
	add	eax, 16					; 00000010H
	mov	DWORD PTR _buf$[ebp], eax

; 106  :         } while (--n);

	mov	eax, DWORD PTR _n$[ebp]
	sub	eax, 1
	mov	DWORD PTR _n$[ebp], eax
	jne	$LN8@adler32

; 107  :         MOD(adler);

	mov	eax, DWORD PTR _adler$[ebp]
	xor	edx, edx
	mov	ecx, 65521				; 0000fff1H
	div	ecx
	mov	DWORD PTR _adler$[ebp], edx

; 108  :         MOD(sum2);

	mov	eax, DWORD PTR _sum2$[ebp]
	xor	edx, edx
	mov	ecx, 65521				; 0000fff1H
	div	ecx
	mov	DWORD PTR _sum2$[ebp], edx

; 109  :     }

	jmp	$LN4@adler32
$LN5@adler32:

; 110  : 
; 111  :     /* do remaining bytes (less than NMAX, still just one modulo) */
; 112  :     if (len) {                  /* avoid modulos if none remaining */

	cmp	DWORD PTR _len$[ebp], 0
	je	$LN19@adler32
$LN9@adler32:

; 113  :         while (len >= 16) {

	cmp	DWORD PTR _len$[ebp], 16		; 00000010H
	jb	$LN11@adler32

; 114  :             len -= 16;

	mov	eax, DWORD PTR _len$[ebp]
	sub	eax, 16					; 00000010H
	mov	DWORD PTR _len$[ebp], eax

; 115  :             DO16(buf);

	mov	eax, 1
	imul	ecx, eax, 0
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	shl	eax, 0
	mov	ecx, DWORD PTR _buf$[ebp]
	movzx	edx, BYTE PTR [ecx+eax]
	add	edx, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], edx
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	shl	eax, 1
	mov	ecx, DWORD PTR _buf$[ebp]
	movzx	edx, BYTE PTR [ecx+eax]
	add	edx, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], edx
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 3
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	shl	eax, 2
	mov	ecx, DWORD PTR _buf$[ebp]
	movzx	edx, BYTE PTR [ecx+eax]
	add	edx, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], edx
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 5
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 6
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 7
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	shl	eax, 3
	mov	ecx, DWORD PTR _buf$[ebp]
	movzx	edx, BYTE PTR [ecx+eax]
	add	edx, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], edx
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 9
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 10
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 11
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 12
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 13
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 14
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax
	mov	eax, 1
	imul	ecx, eax, 15
	mov	edx, DWORD PTR _buf$[ebp]
	movzx	eax, BYTE PTR [edx+ecx]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], eax
	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax

; 116  :             buf += 16;

	mov	eax, DWORD PTR _buf$[ebp]
	add	eax, 16					; 00000010H
	mov	DWORD PTR _buf$[ebp], eax

; 117  :         }

	jmp	$LN9@adler32
$LN11@adler32:

; 118  :         while (len--) {

	mov	eax, DWORD PTR _len$[ebp]
	mov	DWORD PTR tv298[ebp], eax
	mov	ecx, DWORD PTR _len$[ebp]
	sub	ecx, 1
	mov	DWORD PTR _len$[ebp], ecx
	cmp	DWORD PTR tv298[ebp], 0
	je	SHORT $LN12@adler32

; 119  :             adler += *buf++;

	mov	eax, DWORD PTR _buf$[ebp]
	movzx	ecx, BYTE PTR [eax]
	add	ecx, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _adler$[ebp], ecx
	mov	edx, DWORD PTR _buf$[ebp]
	add	edx, 1
	mov	DWORD PTR _buf$[ebp], edx

; 120  :             sum2 += adler;

	mov	eax, DWORD PTR _sum2$[ebp]
	add	eax, DWORD PTR _adler$[ebp]
	mov	DWORD PTR _sum2$[ebp], eax

; 121  :         }

	jmp	SHORT $LN11@adler32
$LN12@adler32:

; 122  :         MOD(adler);

	mov	eax, DWORD PTR _adler$[ebp]
	xor	edx, edx
	mov	ecx, 65521				; 0000fff1H
	div	ecx
	mov	DWORD PTR _adler$[ebp], edx

; 123  :         MOD(sum2);

	mov	eax, DWORD PTR _sum2$[ebp]
	xor	edx, edx
	mov	ecx, 65521				; 0000fff1H
	div	ecx
	mov	DWORD PTR _sum2$[ebp], edx
$LN19@adler32:

; 124  :     }
; 125  : 
; 126  :     /* return recombined sums */
; 127  :     return adler | (sum2 << 16);

	mov	eax, DWORD PTR _sum2$[ebp]
	shl	eax, 16					; 00000010H
	or	eax, DWORD PTR _adler$[ebp]
$LN1@adler32:

; 128  : }

	pop	edi
	pop	esi
	pop	ebx
	mov	esp, ebp
	pop	ebp
	ret	12					; 0000000cH
_adler32@12 ENDP
_TEXT	ENDS
END
