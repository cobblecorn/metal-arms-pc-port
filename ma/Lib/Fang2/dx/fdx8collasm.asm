;//////////////////////////////////////////////////////////////////////////////////////
;// fdx8collasm.asm - 
;//
;// Author: Steve Ranck     
;//////////////////////////////////////////////////////////////////////////////////////
;// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
;// Copyright (c) 2001
;//
;// The contents of this file may not be disclosed to third
;// parties, copied or duplicated in any form, in whole or in part,
;// without the prior written permission of Swingin' Ape Studios, Inc.
;//////////////////////////////////////////////////////////////////////////////////////
;// Modification History:
;//
;// Date     Who         Description
;// -------- ----------  --------------------------------------------------------------
;// 06/22/01 Ranck       Created.
;//////////////////////////////////////////////////////////////////////////////////////


				.686		; Pentium 2
				.xmm		; SIMD


_USE_PREFETCH				equ		0


;--------------------------------------------------------------------------------------------------------------
; Definitions:

; FDX8MeshHW_SIMDCollPacket_t
_CollPacket_s				STRUCT
	qfBoundSphereRadius		dword		4 dup(?)	; Triangle bounding sphere (x4)...
	qfBoundSpherePosX		dword		4 dup(?)
	qfBoundSpherePosY		dword		4 dup(?)
	qfBoundSpherePosZ		dword		4 dup(?)

	qfTriVtxX				dword		4 dup(?)	; A point on the triangle (x4)...
	qfTriVtxY				dword		4 dup(?)
	qfTriVtxZ				dword		4 dup(?)

	qfUnitFaceNormX			dword		4 dup(?)	; Triangle's unit face normal (x4)...
	qfUnitFaceNormY			dword		4 dup(?)
	qfUnitFaceNormZ			dword		4 dup(?)

	apCollData				dword		4 dup(?)	; Pointer to triangle's FDX8MeshHW_CollData_t (x4)...
	anReserved				dword		4 dup(?)	; Not used
_CollPacket_s				ENDS


; FDX8MeshHW_CollIntersect_t
_CollIntersect_s			STRUCT
	pCollData				dword		?			; Pointer to FDX8MeshHW_CollData_t
	nSphereIndex			dword		?			; Index into the sphere list of which sphere this intersect belongs to
	fUnitFaceNormX			dword		?			; Unit face normal of the triangle involved in the intersection...
	fUnitFaceNormY			dword		?
	fUnitFaceNormZ			dword		?
_CollIntersect_s			ENDS


; CFSphere
_Sphere_s					STRUCT
	fRadius					dword		?			; Standard sphere definition...
	fPosX					dword		?
	fPosY					dword		?
	fPosZ					dword		?
_Sphere_s					ENDS


; Expanded version of _Sphere_s
_XSphere_s					STRUCT
	qfRadius				dword		4 dup(?)	; Expanded sphere definition...
	qfPosX					dword		4 dup(?)
	qfPosY					dword		4 dup(?)
	qfPosZ					dword		4 dup(?)
_XSphere_s					ENDS


; CFVec3
_Vec3_s						STRUCT
	fPosX					dword		?
	fPosY					dword		?
	fPosZ					dword		?
_Vec3_s						ENDS


; Expanded version of _Vec3_s
_XVec3_s					STRUCT
	fPosX					dword		4 dup(?)
	fPosY					dword		4 dup(?)
	fPosZ					dword		4 dup(?)
_XVec3_s					ENDS




_P3_CACHE_LINE_BITS			equ		5
_P3_L1_CACHE_BYTES			equ		4096
_P3_CACHE_LINE_BYTES		equ		(1 SHL _P3_CACHE_LINE_BITS)
_P3_CACHE_LINES_IN_L1		equ		(_P3_L1_CACHE_BYTES / _P3_CACHE_LINE_BYTES)

_COLLPACKET_BYTES			equ		SIZE(_CollPacket_s)
_COLLPACKET_CACHE_LINES		equ		(_COLLPACKET_BYTES / _P3_CACHE_LINE_BYTES)
_COLLPACKETS_IN_L1			equ		(_P3_L1_CACHE_BYTES / _COLLPACKET_BYTES)

_XSPHERE_BYTES				equ		SIZE(_XSphere_s)
_XSPHERE_CACHE_LINES		equ		(_XSPHERE_BYTES / _P3_CACHE_LINE_BYTES)
_XSPHERE_ELEMENTS_IN_L1		equ		(_P3_L1_CACHE_BYTES / _XSPHERE_BYTES)

_XVEC3_BYTES				equ		SIZE(_XVec3_s)

_MAX_SPHERE_L1_BANKS		equ		2
_MAX_SPHERES				equ		(_XSPHERE_ELEMENTS_IN_L1 * _MAX_SPHERE_L1_BANKS)

_COLLINTERSECT_BYTES		equ		SIZE(_CollIntersect_s)



.ERRNZ	(_COLLPACKET_BYTES MOD _P3_CACHE_LINE_BYTES)	; sizeof(_CollPacket_s) must be an even multiple of _P3_CACHE_LINE_BYTES
.ERRNZ	(_XSPHERE_BYTES MOD _P3_CACHE_LINE_BYTES)		; sizeof(_XSphere_s) must be an even multiple of _P3_CACHE_LINE_BYTES



;--------------------------------------------------------------------------------------------------------------
; Data Segment:

_DATA						segment para public use32 'DATA'

							align 16
_qfHalf						dd		4 dup(0.5)
_qfNegBits					dd		4 dup(80000000h)

_aXSphereBuf				dd		( SIZE(_XSphere_s) * _MAX_SPHERES / SIZE(dword) ) dup(?)
_XMasterSphere				dd		( SIZE(_XSphere_s) / SIZE(dword) ) dup(?)
_XStartPoint				dd		( SIZE(_XVec3_s) / SIZE(dword) ) dup(?)
_XEndPoint					dd		( SIZE(_XVec3_s) / SIZE(dword) ) dup(?)
_XUnitDir					dd		( SIZE(_XVec3_s) / SIZE(dword) ) dup(?)


							align 4
_nIntersectCount			dd		0		; Number of intersections found
_pIntersectBuf				dd		0		; Points to our intersect buffer

_bFindAllSphereIntersects	dd		0		; FALSE=find the first sphere intersection, TRUE=find all spheres in sphere list that intersect
_nIntersectBufCount			dd		0		; Number of intersections that can fit into _pIntersectBuf
_pIntersectBufEnd			dd		0		; Points to one entry past the end of our intersect buffer

_nSphereCount				dd		0		; Number of spheres in our sphere list
_nPacketCount				dd		0		; Number of packets we're currently processing

_Sphere						dd		( SIZE(_Sphere_s) / SIZE(dword) ) dup(?)

_DATA						ends




;--------------------------------------------------------------------------------------------------------------
; Code Segment:

				assume ds:flat, cs:flat, ss:flat
_TEXT			segment dword public use32 'CODE'

;--------------------------------------------------------------------------------------------------------------
; BOOL fdx8collasm_SetBuffers( void *pXSphereBuf, u32 nXSphereBufCount, FDX8MeshHW_CollIntersect_t *pIntersectBuf, u32 nIntersectBufCount );
;
; Returns TRUE if successful.
; Returns FALSE if failure.

fdx8collasm_SetBuffers PROC C pXSphereBuf:dword, nXSphereBufCount:dword, pIntersectBuf:dword, nIntersectBufCount:dword

				;---------------------------------------------------
				; Store buffer information:
				;---------------------------------------------------

				mov		eax, pIntersectBuf								; Store intersect buffer location...
				mov		_pIntersectBuf, eax

				mov		eax, nIntersectBufCount							; Store intersect buffer count...
				mov		_nIntersectBufCount, eax

				imul	eax, _COLLINTERSECT_BYTES						; Store intersect buffer end location...
				add		eax, _pIntersectBuf
				mov		_pIntersectBufEnd, eax

				;---------------------------------------------------
				; Return success:
				;---------------------------------------------------

				mov		eax, 1											; Return success code...
				jmp		_ExitSetBuffers

				;---------------------------------------------------
				; Return error:
				;---------------------------------------------------

_ExitSetBuffersErr:

				xor		eax, eax										; Return error code...

				;---------------------------------------------------
				; Exit function:
				;---------------------------------------------------

_ExitSetBuffers:

				ret

fdx8collasm_SetBuffers ENDP



;--------------------------------------------------------------------------------------------------------------
; Returns TRUE successful, or FALSE if the specified sphere list count is too big for the expanded sphere list
; buffer.
;
; BOOL fdx8collasm_SetSphereList( const CFSphere *pSrcMasterSphere, const CFSphere *pSrcSphereList, u32 nSrcSphereListCount );

fdx8collasm_SetSphereList PROC C pSrcMasterSphere:dword, pSrcSphereList:dword, nSrcSphereListCount:dword

				push	ecx
				push	esi
				push	edi

				;---------------------------------------------------
				; Expand sphere list:
				;---------------------------------------------------

				mov	ecx, nSrcSphereListCount							; Make sure the number of spheres passed in will
				cmp	ecx, _MAX_SPHERES									;  fit into our expanded sphere buffer...
				ja		_ExitExpandSphereListErr						; Jump if sphere list is too big
				mov		_nSphereCount, ecx								; Store the number of spheres in the list
				jecxz	_ExpandMasterSphere								; Jump if there are no spheres in our sphere list

				mov		ecx, (SIZE(_Sphere_s) / SIZE(dword))			; ecx = number of dwords in source sphere list to expand...
				imul	ecx, nSrcSphereListCount
				mov		esi, pSrcSphereList								; esi points to source sphere list
				mov		edi, OFFSET _aXSphereBuf						; edi points to destination sphere list

_ExpandSphereLoop:

				movlps	xmm0, [esi]										; Expand one dword into 4 copies...
				shufps	xmm0, xmm0, 00h
				movaps	[edi], xmm0

				add		esi, SIZE(dword)								; Point esi to next source dword
				add		edi, (SIZE(dword) * 4)							; Point edi to next destination quad-dword
				loop	_ExpandSphereLoop								; Do it until done

				;---------------------------------------------------
				; Expand master sphere:
				;---------------------------------------------------

_ExpandMasterSphere:

				mov		ecx, (SIZE(_Sphere_s) / SIZE(dword))			; ecx = number of dwords in master sphere to expand
				mov		edi, OFFSET _XMasterSphere						; edi points to destination master sphere

				mov		esi, pSrcMasterSphere							; esi points to source master sphere (unless...)
				cmp		_nSphereCount, 1								; If there is exactly 1 sphere in our sphere list,
				cmove	esi, pSrcSphereList								;   use it as our master sphere

_ExpandMasterSphereLoop:

				movlps	xmm0, [esi]										; Expand one dword into 4 copies...
				shufps	xmm0, xmm0, 00h
				movaps	[edi], xmm0

				add		esi, SIZE(dword)								; Point esi to next source dword
				add		edi, (SIZE(dword) * 4)							; Point edi to next destination quad-dword
				loop	_ExpandMasterSphereLoop							; Do it until done

				;---------------------------------------------------
				; If the sphere list is empty, we'll just set it
				; up with a single sphere that is the master
				; sphere:
				;---------------------------------------------------

				cmp		_nSphereCount, 0								; Is sphere list empty?
				jne		_ExitExpandSphereListOk							; Jump if not

				inc		_nSphereCount									; Set sphere list count to 1

				mov		ecx, (SIZE(_XSphere_s) / SIZE(dword))			; Copy our master sphere to the sphere list...
				mov		esi, OFFSET _XMasterSphere
				mov		edi, OFFSET _aXSphereBuf
				cld
				rep movsd

				;---------------------------------------------------
				; Return success:
				;---------------------------------------------------

_ExitExpandSphereListOk:

				mov		eax, 1											; Return success code...
				jmp		_ExitExpandSphereList

				;---------------------------------------------------
				; Return error:
				;---------------------------------------------------

_ExitExpandSphereListErr:

				xor		eax, eax										; Return error code...

				;---------------------------------------------------
				; Exit function:
				;---------------------------------------------------

_ExitExpandSphereList:
				pop		edi
				pop		esi
				pop		ecx
				ret

fdx8collasm_SetSphereList ENDP



;--------------------------------------------------------------------------------------------------------------
; u32 fdx8collasm_CollideWithSphereList( FDX8MeshHW_SIMDCollPacket_t *pCollPacketArray, u32 nCollPacketCount, BOOL bFindAllSphereIntersections );

;----------------------------------------------------------------------------------------------
; The P3 has a 16K, 4-way set associative L1 cache. It uses a pseudo LRU algorithm to determine
; when to boot out cache lines (cache lines are 32 bytes). Due to the 4-way nature of the cache,
; the L1 is actually broken into 4 banks of 4K each. When data is to be cached, its
; address-MOD-4096 is computed and used to simultaneously index into the 4 banks to retrieve
; information as to which bank holds the entry that has been referenced longest ago. That entry
; is replaced by the freshly loaded data and the slot is moved to the most recently referenced
; list.
;
; We make use of the 4-way nature of the L1 cache in this code. We will fill up to two of our
; 4 L1 banks with expanded sphere list data. Since a sphere is 16 bytes (4 floats for x, y, z,
; radius), then an expanded sphere is 16*4=64 bytes. So we can fit 4096/64=64 expanded spheres
; per bank, and 128 expanded spheres total. Next, we will fill up to 4096 bytes of collision
; packet data so that it fills a single L1 bank. We'll leave the last bank free just in case
; we need access to other memory.
;
; With our L1 loaded, we can perform our collision algorithm on the data with single-cycle
; accesses since it's all in L1 cache. As we generate intersections, we'll use the SIMD streaming
; store instructions to write our output to only the L2 cache so we won't corrupt the data
; we're so carefully placed in the L1. The number of output intersections should be small anyway.
;
; When we're done processing that chunk of collision packets, we prefetch the next bunch into
; the L1 and begin again.
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; nCollPacketCount is the number of elements pointed to by esi
;----------------------------------------------------------------------------------------------

fdx8collasm_CollideWithSphereList PROC C pCollPacketArray:dword, nCollPacketCount:dword, bFindAllSphereIntersections:dword

				push	ebx
				push	ecx
				push	edx
				push	esi
				push	edi

				mov		esi, pCollPacketArray							; esi points to the packet array...
				ASSUME	esi: ptr _CollPacket_s
				mov		_nIntersectCount, 0

				mov		eax, bFindAllSphereIntersections				; Save flag...
				mov		_bFindAllSphereIntersects, eax

;==============================================================================================================
; Fill L1 cache with our sphere list.
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; nCollPacketCount is the number of packets pointed to by esi
;==============================================================================================================

				IF NOT _USE_PREFETCH
				 jmp		_FillL1WithPackets1
				ENDIF

				;---------------------------------------------------
				; Determinine whether there are any spheres
				; in the sphere list:
				;---------------------------------------------------

				cmp		_nSphereCount, 0								; Are there any spheres in the sphere list?
				je		_FillL1WithPackets								; Jump if not

				mov		edi, OFFSET _aXSphereBuf						; edi points to our expanded sphere buffer
				ASSUME	edi: ptr _XSphere_s

				;---------------------------------------------------
				; Preload TLB entries:
				;---------------------------------------------------

				nByteOffset = 0
				REPT	_MAX_SPHERE_L1_BANKS
				 mov	eax, [edi + nByteOffset].qfRadius
				 nByteOffset = nByteOffset + _P3_L1_CACHE_BYTES
				ENDM

				;---------------------------------------------------
				; Prefetch spheres in sphere list:
				;---------------------------------------------------

				mov		ebx, _nSphereCount								; ebx = # cache lines to prefetch
				imul	ebx, _XSPHERE_CACHE_LINES

@@:				prefetchnta [edi]

				add		edi, _P3_CACHE_LINE_BYTES						; Point to next cache line
				dec		ebx												; Loop until we fill the L1 cache...
				jnz		@B

				ASSUME	edi: nothing

;==============================================================================================================
; Fill L1 cache with as many collision packets as possible
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; nCollPacketCount is the number of packets pointed to by esi
;==============================================================================================================

_FillL1WithPackets1:

				mov		ebx, _pIntersectBuf								; ebx points into our destination intersect buffer
				ASSUME	ebx: ptr _CollIntersect_s

_FillL1WithPackets:

				;---------------------------------------------------
				; See if there are any remaining
				; packets to process:
				;---------------------------------------------------

				mov		edx, nCollPacketCount							; edx is the number of packets remaining
				or		edx, edx										; Are there any remaining packets to process?
				jnz		_BeginFillingL1WithPackets						; Jump if so
				jmp		_Exit											; If not, we're done. Exit this function!

				;---------------------------------------------------
				; There are remaining packets to process.
				; Compute the number of packets to put in the
				; L1 cache:
				;---------------------------------------------------

_BeginFillingL1WithPackets:

				IF NOT _USE_PREFETCH
				 mov		_nPacketCount, edx
				 mov		nCollPacketCount, 0
				 jmp		_PacketLoop1
				ENDIF

				mov		eax, [esi].qfBoundSpherePosX					; Make sure TLB for first bank will be loaded and waiting for us

				mov		ecx, _COLLPACKETS_IN_L1							; Assume we have enough packets to fill the entire L1
				cmp		edx, _COLLPACKETS_IN_L1							; Check if we have fewer packets
				cmovb	ecx, edx										; If we have fewer packets, just use the elements we have

				mov		_nPacketCount, ecx								; Count these packets as processed...
				sub		nCollPacketCount, ecx
				jz		_PrepForFillintL1WithPackets					; Jump if this is the last bunch of packets
				mov		eax, [esi + _P3_L1_CACHE_BYTES].qfBoundSpherePosX	; Make sure TLB for next bank will be loaded and waiting for us next loop

_PrepForFillintL1WithPackets:

				mov		edi, esi										; edi points to packets
				ASSUME	edi: ptr _CollPacket_s
				mov		edx, ecx										; edx = # cache lines to prefetch...
				imul	edx, _COLLPACKET_CACHE_LINES

@@:				prefetchnta [edi]										; Prefetch one cache line

				add		edi, _P3_CACHE_LINE_BYTES						; Point to next cache line
				dec		edx												; Loop until we fill the L1 cache...
				jnz		@B

				ASSUME	edi: nothing

;==============================================================================================================
; Test our master sphere against the packet triangle spheres.
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; _nPacketCount is the # of collision packets we're going to process
;==============================================================================================================

				;---------------------------------------------------
				; Load xmm0-3 with expanded master sphere:
				;---------------------------------------------------

_PacketLoop1:	movaps	xmm0, _XMasterSphere._XSphere_s.qfRadius
				movaps	xmm1, _XMasterSphere._XSphere_s.qfPosX
				movaps	xmm2, _XMasterSphere._XSphere_s.qfPosY
				movaps	xmm3, _XMasterSphere._XSphere_s.qfPosZ

				;---------------------------------------------------
				; Load xmm4-7 with our collision packet spheres:
				;---------------------------------------------------

_PacketLoop2:	movaps	xmm4, [esi].qfBoundSphereRadius
				movaps	xmm5, [esi].qfBoundSpherePosX
				movaps	xmm6, [esi].qfBoundSpherePosY
				movaps	xmm7, [esi].qfBoundSpherePosZ

				;---------------------------------------------------
				; Compute the distance-squared between
				; the two sphere centers:
				;---------------------------------------------------

				subps	xmm5, xmm1										; xmm5 = fDx
				subps	xmm6, xmm2										; xmm6 = fDy
				subps	xmm7, xmm3										; xmm7 = fDz
				mulps	xmm5, xmm5										; xmm5 = fDx*fDx
				mulps	xmm6, xmm6										; xmm6 = fDy*fDy
				mulps	xmm7, xmm7										; xmm7 = fDz*fDz
				addps	xmm5, xmm6										; xmm5 = fDx*fDx + fDy*fDy
				addps	xmm5, xmm7										; xmm5 = fDx*fDx + fDy*fDy + fDz*fDz

				;---------------------------------------------------
				; Compute the square of the sum of the
				; two sphere radii:
				;---------------------------------------------------

				addps	xmm4, xmm0										; xmm4 = fRm + fRt
				mulps	xmm4, xmm4										; xmm4 = (fRm + fRt)*(fRm + fRt)

				;---------------------------------------------------
				; Find out if any of the 4 triangle spheres
				; intersect the master sphere:
				;---------------------------------------------------

				cmpltps	xmm5, xmm4										; Set mask if spheres intersect
				movmskps eax, xmm5										; Move mask to eax
				test	eax, 1111b										; Are any bits set?
				jnz		_TestSphereList									; Jump if so

				;---------------------------------------------------
				; None of the 4 triangle spheres intersect the
				; master sphere. Point to next packet:
				;---------------------------------------------------

				add		esi, _COLLPACKET_BYTES							; Point to next packet
				dec		_nPacketCount									; Loop through all packets...
				jnz		_PacketLoop2
				jmp		_FillL1WithPackets								; No more packets. Go grab some more.

;==============================================================================================================
; At least one of the triangle spheres in the packet intersects the master sphere.
; We must now check the entire sphere list against the triangle spheres in the packet.
;
; al mask holding result of master sphere intersection test
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; _nPacketCount is the # of collision packets we're going to process
;==============================================================================================================

_TestSphereList:

				mov		ecx, _nSphereCount								; ecx = # of spheres
				mov		edi, OFFSET _aXSphereBuf						; edi points into expanded sphere buffer
				ASSUME	edi: ptr _XSphere_s

				cmp		ecx, 1											; If there is only 1 sphere in the sphere list,
				mov		dl, al											;  the master sphere is the only sphere to check.
				je		_TestSphereVsTriPlane							; Jump if so

				;---------------------------------------------------
				; Load xmm4-7 with our collision packet spheres:
				;---------------------------------------------------

				movaps	xmm4, [esi].qfBoundSphereRadius
				movaps	xmm5, [esi].qfBoundSpherePosX
				movaps	xmm6, [esi].qfBoundSpherePosY
				movaps	xmm7, [esi].qfBoundSpherePosZ

_TestSphereListLoop:

				;---------------------------------------------------
				; Load xmm0-3 with the current sphere in our
				; sphere list:
				;---------------------------------------------------

				movaps	xmm0, [edi].qfRadius
				movaps	xmm1, [edi].qfPosX
				movaps	xmm2, [edi].qfPosY
				movaps	xmm3, [edi].qfPosZ

				;---------------------------------------------------
				; Compute the distance-squared between
				; the two sphere centers:
				;---------------------------------------------------

				subps	xmm1, xmm5										; xmm1 = fDx
				subps	xmm2, xmm6										; xmm2 = fDy
				subps	xmm3, xmm7										; xmm3 = fDz
				mulps	xmm1, xmm1										; xmm1 = fDx*fDx
				mulps	xmm2, xmm2										; xmm2 = fDy*fDy
				mulps	xmm3, xmm3										; xmm3 = fDz*fDz
				addps	xmm1, xmm2										; xmm1 = fDx*fDx + fDy*fDy
				addps	xmm1, xmm3										; xmm1 = fDx*fDx + fDy*fDy + fDz*fDz

				;---------------------------------------------------
				; Compute the square of the sum of the
				; two sphere radii:
				;---------------------------------------------------

				addps	xmm0, xmm4										; xmm0 = fRm + fRt
				mulps	xmm0, xmm0										; xmm0 = (fRm + fRt)*(fRm + fRt)

				;---------------------------------------------------
				; Find out if any of the 4 triangle spheres
				; intersect the sphere:
				;---------------------------------------------------

				cmpltps	xmm1, xmm0										; Set mask if spheres intersect
				movmskps eax, xmm1										; Move mask to eax
				test	eax, 1111b										; Are any bits set?
				mov		dl, al											; Save mask in dl
				jnz		_TestSphereVsTriPlane							; Jump if so

				;---------------------------------------------------
				; None of the 4 triangle spheres intersect the
				; sphere. Try the next sphere in the list:
				;---------------------------------------------------

_NextSphereInSphereList:

				add		edi, _XSPHERE_BYTES								; Point to next sphere in sphere list
				dec		ecx												; Loop through all spheres...
				jnz		_TestSphereListLoop

				;---------------------------------------------------
				; Done scanning through spheres. Point to next
				; packet:
				;---------------------------------------------------

_NextPacket:	add		esi, _COLLPACKET_BYTES							; Point to next packet
				dec		_nPacketCount									; Loop through all packets...
				jnz		_PacketLoop1
				jmp		_FillL1WithPackets								; No more packets. We're done!

;==============================================================================================================
; At least one of the triangle spheres in the packet intersects the current sphere
; in our sphere list. We must now check the sphere in the sphere list with the triangle
; plane to see if they intersect.
;
; xmm0-3 hold expanded sphere (r, x, y, z)
; dl mask holding result of master sphere intersection test
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; _nPacketCount is the # of collision packets we're going to process
;==============================================================================================================

_TestSphereVsTriPlane:

				;---------------------------------------------------
				; Load xmm0-3 with the current sphere in our
				; sphere list:
				;---------------------------------------------------

				movaps	xmm0, [edi].qfRadius
				movaps	xmm1, [edi].qfPosX
				movaps	xmm2, [edi].qfPosY
				movaps	xmm3, [edi].qfPosZ

				;---------------------------------------------------
				; Compute v = SpherePos - TriVtx
				; (xmm1=v.x,  xmm2=v.y,  xmm3=v.z)
				;---------------------------------------------------

				subps	xmm1, [esi].qfTriVtxX
				subps	xmm2, [esi].qfTriVtxY
				subps	xmm3, [esi].qfTriVtxZ

				;---------------------------------------------------
				; Compute xmm1 = v DOT TriFaceUnitNorm
				;---------------------------------------------------

				mulps	xmm1, [esi].qfUnitFaceNormX
				mulps	xmm2, [esi].qfUnitFaceNormY
				mulps	xmm3, [esi].qfUnitFaceNormZ
				addps	xmm1, xmm2
				addps	xmm1, xmm3

				xorps	xmm3, xmm3										; xmm3 = 0

				addps	xmm1, xmm0										; Switch from -R -> +R to 0 -> R...
				mulps	xmm1, _qfHalf
				movaps	xmm2, xmm1										; 2 comparisons are coming, so xmm2 = copy of xmm1

				;---------------------------------------------------
				; Test if sphere is in front of the tri plane:
				;---------------------------------------------------

				cmpnleps xmm1, xmm0										; Set mask if sphere is in front of plane
				movmskps eax, xmm1										; Move mask to eax
				not		eax												; Clear mask if sphere is in front of plane
				and		dl, al											; Update mask
				jz		_NextSphereInSphereList							; If no mask bit is set, continue checking spheres

				;---------------------------------------------------
				; Test if sphere is in back of the tri plane:
				;---------------------------------------------------

				cmpltps	xmm2, xmm3										; Set mask if sphere is behind plane
				movmskps eax, xmm2										; Move mask to eax
				not		eax												; Clear mask if sphere is behind plane
				and		dl, al											; Update mask
				jz		_NextSphereInSphereList							; If no mask bit is set, continue checking spheres

;==============================================================================================================
; Create an intersection entry for any of the set bits in our mask register, dl:
;
; dl mask holding result of master sphere intersection test
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; _nPacketCount is the # of collision packets we're going to process
;==============================================================================================================

				mov		dh, 4											; Only look at lower 4 bits
_ScanMaskLoop:	shr		dl, 1											; Move bit into carry flag
				jnc		_ScanNextMask									; Jump if mask bit is 0

				;---------------------------------------------------
				; Mask bit is 1.
				; Check if there's room to create one intersect:
				;---------------------------------------------------

				cmp		ebx, _pIntersectBufEnd							; Do we have room in our intersect buffer?
				jae		_Exit											; Jump if not

				;---------------------------------------------------
				; There's room in our intersect buffer.
				; Create one intersect:
				;---------------------------------------------------

				mov		eax, [esi].apCollData							; Create one intersect entry...
				or		eax, eax
				jz		_ScanNextMask
				mov		[ebx].pCollData, eax
				mov		eax, _nSphereCount
				sub		eax, ecx
				mov		[ebx].nSphereIndex, eax
				mov		eax, [esi].qfUnitFaceNormX
				mov		[ebx].fUnitFaceNormX, eax
				mov		eax, [esi].qfUnitFaceNormY
				mov		[ebx].fUnitFaceNormY, eax
				mov		eax, [esi].qfUnitFaceNormZ
				mov		[ebx].fUnitFaceNormZ, eax

				inc		_nIntersectCount								; Count the intersect
				add		ebx, _COLLINTERSECT_BYTES						; Point to next intersect entry

_ScanNextMask:	add		esi, SIZE(dword)								; Adjust packet pointer for next bit
				dec		dh												; Loop until we've checked all 4 bits...
				jnz		_ScanMaskLoop

				sub		esi, (4 * SIZE(dword))							; Restore esi to original value

				cmp		_bFindAllSphereIntersects, 0					; If we're not finding all sphere intersections,
				je		_NextPacket										;  go check the next packet
				jmp		_NextSphereInSphereList							; Otherwise, go check next sphere in sphere list

;==============================================================================================================
; Exit
;==============================================================================================================

_Exit:			pop		edi
				pop		esi
				pop		edx
				pop		ecx
				pop		ebx

				mov		eax, _nIntersectCount
				ret

fdx8collasm_CollideWithSphereList  ENDP





;--------------------------------------------------------------------------------------------------------------
; u32 fdx8collasm_CollideWithLineSeg( FDX8MeshHW_SIMDCollPacket_t *pCollPacketArray, u32 nCollPacketCount, const CFVec3 *pStartPoint_WS, const CFVec3 *pEnd_WS, const CFVec3 *pUnitDir );

fdx8collasm_CollideWithLineSeg PROC C pCollPacketArray:dword, nCollPacketCount:dword, pStartPoint:dword, pEndPoint:dword, pUnitDir:dword

				push	ebx
				push	ecx
				push	edx
				push	esi
				push	edi

;==============================================================================================================
; Expand pStartPoint, pEndPoint, and pUnitDir...
;==============================================================================================================

				;---------------------------------------------------
				; Expand start point:
				;---------------------------------------------------

				mov		ecx, (SIZE(_Vec3_s) / SIZE(dword))				; ecx = number of dwords in source pStartPoint to expand...
				mov		esi, pStartPoint								; esi points to source vector
				mov		edi, OFFSET _XStartPoint						; edi points to destination vector
@@:				movlps	xmm0, [esi]										; Expand one dword into 4 copies...
				shufps	xmm0, xmm0, 00h
				movaps	[edi], xmm0
				add		esi, SIZE(dword)								; Point esi to next source dword
				add		edi, (SIZE(dword) * 4)							; Point edi to next destination quad-dword
				loop	@B												; Do it until done

				;---------------------------------------------------
				; Expand end point:
				;---------------------------------------------------

				mov		ecx, (SIZE(_Vec3_s) / SIZE(dword))				; ecx = number of dwords in source pStartPoint to expand...
				mov		esi, pEndPoint									; esi points to source vector
				mov		edi, OFFSET _XEndPoint							; edi points to destination vector
@@:				movlps	xmm0, [esi]										; Expand one dword into 4 copies...
				shufps	xmm0, xmm0, 00h
				movaps	[edi], xmm0
				add		esi, SIZE(dword)								; Point esi to next source dword
				add		edi, (SIZE(dword) * 4)							; Point edi to next destination quad-dword
				loop	@B												; Do it until done

				;---------------------------------------------------
				; Expand unit dir:
				;---------------------------------------------------

				mov		ecx, (SIZE(_Vec3_s) / SIZE(dword))				; ecx = number of dwords in source pStartPoint to expand...
				mov		esi, pUnitDir									; esi points to source vector
				mov		edi, OFFSET _XUnitDir							; edi points to destination vector
@@:				movlps	xmm0, [esi]										; Expand one dword into 4 copies...
				shufps	xmm0, xmm0, 00h
				movaps	[edi], xmm0
				add		esi, SIZE(dword)								; Point esi to next source dword
				add		edi, (SIZE(dword) * 4)							; Point edi to next destination quad-dword
				loop	@B												; Do it until done

;==============================================================================================================
; Fill L1 cache with as many collision packets as possible
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; nCollPacketCount is the number of packets pointed to by esi
;==============================================================================================================

				mov		_nIntersectCount, 0

				mov		esi, pCollPacketArray							; esi points to the packet array...
				ASSUME	esi: ptr _CollPacket_s

				mov		ebx, _pIntersectBuf								; ebx points into our destination intersect buffer
				ASSUME	ebx: ptr _CollIntersect_s

_FillL1WithPackets:

				;---------------------------------------------------
				; See if there are any remaining
				; packets to process:
				;---------------------------------------------------

				mov		edx, nCollPacketCount							; edx is the number of packets remaining
				or		edx, edx										; Are there any remaining packets to process?
				jnz		_BeginFillingL1WithPackets						; Jump if so
				jmp		_Exit											; If not, we're done. Exit this function!

				;---------------------------------------------------
				; There are remaining packets to process.
				; Compute the number of packets to put in the
				; L1 cache:
				;---------------------------------------------------

_BeginFillingL1WithPackets:

				IF NOT _USE_PREFETCH
				 mov		_nPacketCount, edx
				 mov		nCollPacketCount, 0
				 jmp		_PacketLoop
				ENDIF

				mov		eax, [esi].qfBoundSpherePosX					; Make sure TLB for first bank will be loaded and waiting for us

				mov		ecx, _COLLPACKETS_IN_L1							; Assume we have enough packets to fill the entire L1
				cmp		edx, _COLLPACKETS_IN_L1							; Check if we have fewer packets
				cmovb	ecx, edx										; If we have fewer packets, just use the elements we have

				mov		_nPacketCount, ecx								; Count these packets as processed...
				sub		nCollPacketCount, ecx
				jz		_PrepForFillintL1WithPackets					; Jump if this is the last bunch of packets
				mov		eax, [esi + _P3_L1_CACHE_BYTES].qfBoundSpherePosX	; Make sure TLB for next bank will be loaded and waiting for us next loop

_PrepForFillintL1WithPackets:

				mov		edi, esi										; edi points to packets
				ASSUME	edi: ptr _CollPacket_s
				mov		edx, ecx										; edx = # cache lines to prefetch...
				imul	edx, _COLLPACKET_CACHE_LINES

_FillL1Loop:	prefetchnta [edi]										; Prefetch one cache line

				add		edi, _P3_CACHE_LINE_BYTES						; Point to next cache line
				dec		edx												; Loop until we fill the L1 cache...
				jnz		_FillL1Loop

				ASSUME	edi: nothing

;==============================================================================================================
; Test the infinite line against the packet triangle spheres.
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; _nPacketCount is the # of collision packets we're going to process
;==============================================================================================================

_PacketLoop:	

				;---------------------------------------------------
				; xmm4 = sphere radius squared
				;---------------------------------------------------
				movaps	xmm4, [esi].qfBoundSphereRadius
				mulps	xmm4, xmm4

				;---------------------------------------------------
				; xmm0-2 = xmm5-7 = LineStartToSphereCenter
				;---------------------------------------------------

				movaps	xmm0, [esi].qfBoundSpherePosX
				movaps	xmm1, [esi].qfBoundSpherePosY
				movaps	xmm2, [esi].qfBoundSpherePosZ
				subps	xmm0, _XStartPoint._XVec3_s.fPosX
				subps	xmm1, _XStartPoint._XVec3_s.fPosY
				subps	xmm2, _XStartPoint._XVec3_s.fPosZ

				movaps	xmm5, xmm0
				movaps	xmm6, xmm1
				movaps	xmm7, xmm2

				;---------------------------------------------------
				; xmm0 = LineStartToSphereCenter DOT LineUnitDir
				;---------------------------------------------------

				mulps	xmm0, _XUnitDir._XVec3_s.fPosX
				mulps	xmm1, _XUnitDir._XVec3_s.fPosY
				mulps	xmm2, _XUnitDir._XVec3_s.fPosZ
				addps	xmm0, xmm1
				addps	xmm0, xmm2

				;------------------------------------------------------------------
				; xmm1-3 = LineUnitDir*(LineStartToSphereCenter DOT LineUnitDir)
				;------------------------------------------------------------------

				movaps	xmm1, _XUnitDir._XVec3_s.fPosX
				movaps	xmm2, _XUnitDir._XVec3_s.fPosY
				movaps	xmm3, _XUnitDir._XVec3_s.fPosZ
				mulps	xmm1, xmm0
				mulps	xmm2, xmm0
				mulps	xmm3, xmm0

				;------------------------------------------------------------------------------------------
				; xmm0-2 = ShortestVecFromSphereCenterToLine
				;        = LineUnitDir*(LineStartToSphereCenter DOT LineUnitDir) - LineStartToSphereCenter
				;------------------------------------------------------------------------------------------

				subps	xmm1, xmm5
				subps	xmm2, xmm6
				subps	xmm3, xmm7

				;---------------------------------------------------------------
				; xmm1 = magnitude squared of ShortestVecFromSphereCenterToLine
				;---------------------------------------------------------------

				mulps	xmm1, xmm1
				mulps	xmm2, xmm2
				mulps	xmm3, xmm3
				addps	xmm1, xmm2
				addps	xmm1, xmm3

				;---------------------------------------------------
				; Find out if the distance from the sphere center to
				; the line falls within the sphere's radius:
				;---------------------------------------------------

				cmpleps	xmm1, xmm4										; Set mask if line intersects sphere
				movmskps eax, xmm1										; Move mask to eax
				test	eax, 1111b										; Are any bits set?
				jnz		_TestEndpoints									; Jump if so

				;---------------------------------------------------
				; Next packet:
				;---------------------------------------------------

_NextPacket:	add		esi, _COLLPACKET_BYTES							; Point to next packet
				dec		_nPacketCount									; Loop through all packets...
				jnz		_PacketLoop
				jmp		_FillL1WithPackets								; No more packets. We're done!

;==============================================================================================================
; Test to see if the endpoints are on opposite sides of the bounding sphere:
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; dh is the mask result from the infinite line test
; _nPacketCount is the # of collision packets we're going to process
;==============================================================================================================

_TestEndpoints:

				mov		dh, al											; dh = mask result from infinite line test
				xorps	xmm3, xmm3										; xmm3 = 0.0f

				;---------------------------------------------------
				; xmm0-2 = LineStartToSphereCenter DOT LineUnitDir
				;---------------------------------------------------

				movaps	xmm0, xmm5
				movaps	xmm1, xmm6
				movaps	xmm2, xmm7

				mulps	xmm0, _XUnitDir._XVec3_s.fPosX
				mulps	xmm1, _XUnitDir._XVec3_s.fPosY
				mulps	xmm2, _XUnitDir._XVec3_s.fPosZ
				addps	xmm0, xmm1
				addps	xmm0, xmm2

				;---------------------------------------------------------------
				; Set mask if (LineStartToSphereCenter DOT LineUnitDir) >= 0.0f
				;---------------------------------------------------------------

				cmpnltps xmm0, xmm3
				movmskps eax, xmm0
				mov		dl, al

				;---------------------------------------------------
				; xmm0-2 = LineEndToSphereCenter DOT LineUnitDir
				;---------------------------------------------------

				movaps	xmm0, [esi].qfBoundSpherePosX
				movaps	xmm1, [esi].qfBoundSpherePosY
				movaps	xmm2, [esi].qfBoundSpherePosZ
				subps	xmm0, _XEndPoint._XVec3_s.fPosX
				subps	xmm1, _XEndPoint._XVec3_s.fPosY
				subps	xmm2, _XEndPoint._XVec3_s.fPosZ

				mulps	xmm0, _XUnitDir._XVec3_s.fPosX
				mulps	xmm1, _XUnitDir._XVec3_s.fPosY
				mulps	xmm2, _XUnitDir._XVec3_s.fPosZ
				addps	xmm0, xmm1
				addps	xmm0, xmm2

				;---------------------------------------------------------------
				; Set mask if (LineEndToSphereCenter DOT LineUnitDir) <= 0.0f
				;---------------------------------------------------------------

				cmpleps xmm0, xmm3
				movmskps eax, xmm0

				;---------------------------------------------------
				; All 3 of the above masks must be set to result in
				; a potential impact:
				;---------------------------------------------------

				and		dl, al
				and		dl, dh
				cmp		dl, 1111b
				je		_CreateImpacts

;==============================================================================================================
; Test to see if the start point is inside the bounding sphere:
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; dl is the mask result from the endpoint test AND dh
; dh is the mask result from the infinite line test
; _nPacketCount is the # of collision packets we're going to process
;==============================================================================================================

				;---------------------------------------------------
				; xmm5-7 = mag-squared of LineStartToSphereCenter:
				;---------------------------------------------------

				mulps	xmm5, xmm5
				mulps	xmm6, xmm6
				mulps	xmm7, xmm7
				addps	xmm5, xmm6
				addps	xmm5, xmm7

				;---------------------------------------------------
				; Set mask if start point is inside sphere:
				;---------------------------------------------------

				cmpleps	xmm5, xmm4
				movmskps eax, xmm5

				;---------------------------------------------------
				; Combine masks:
				;---------------------------------------------------

				or		dl, al
				cmp		dl, 1111b
				je		_CreateImpacts

;==============================================================================================================
; Test to see if the end point is inside the bounding sphere:
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; dl is the mask result from the endpoint and start-in-sphere tests AND dh
; dh is the mask result from the infinite line test
; _nPacketCount is the # of collision packets we're going to process
;==============================================================================================================

				;---------------------------------------------------
				; xmm5-7 = mag-squared of LineEndToSphereCenter:
				;---------------------------------------------------

				movaps	xmm5, [esi].qfBoundSpherePosX
				movaps	xmm6, [esi].qfBoundSpherePosY
				movaps	xmm7, [esi].qfBoundSpherePosZ
				subps	xmm5, _XEndPoint._XVec3_s.fPosX
				subps	xmm6, _XEndPoint._XVec3_s.fPosY
				subps	xmm7, _XEndPoint._XVec3_s.fPosZ

				mulps	xmm5, xmm5
				mulps	xmm6, xmm6
				mulps	xmm7, xmm7
				addps	xmm5, xmm6
				addps	xmm5, xmm7

				;---------------------------------------------------
				; Set mask if end point is inside sphere:
				;---------------------------------------------------

				cmpleps	xmm5, xmm4
				movmskps eax, xmm5

				;---------------------------------------------------
				; Combine masks:
				;---------------------------------------------------

				or		dl, al
				jz		_NextPacket

;==============================================================================================================
; Create an intersection entry for any of the set bits in our mask register, dl:
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; dl is our impact mask
; _nPacketCount is the # of collision packets we're going to process
;==============================================================================================================

_CreateImpacts:

				mov		dh, 4											; Only look at lower 4 bits
_ScanMaskLoop:	shr		dl, 1											; Move bit into carry flag
				jnc		_ScanNextMask									; Jump if mask bit is 0

				;---------------------------------------------------
				; Mask bit is 1.
				; Check if there's room to create one intersect:
				;---------------------------------------------------

				cmp		ebx, _pIntersectBufEnd							; Do we have room in our intersect buffer?
				jae		_Exit											; Jump if not

				;---------------------------------------------------
				; There's room in our intersect buffer.
				; Create one intersect:
				;---------------------------------------------------

				mov		eax, [esi].apCollData							; Create one intersect entry...
				or		eax, eax
				jz		_ScanNextMask
				mov		[ebx].pCollData, eax
				mov		[ebx].nSphereIndex, 0
				mov		eax, [esi].qfUnitFaceNormX
				mov		[ebx].fUnitFaceNormX, eax
				mov		eax, [esi].qfUnitFaceNormY
				mov		[ebx].fUnitFaceNormY, eax
				mov		eax, [esi].qfUnitFaceNormZ
				mov		[ebx].fUnitFaceNormZ, eax

				inc		_nIntersectCount								; Count the intersect
				add		ebx, _COLLINTERSECT_BYTES						; Point to next intersect entry

_ScanNextMask:	add		esi, SIZE(dword)								; Adjust packet pointer for next bit
				dec		dh												; Loop until we've checked all 4 bits...
				jnz		_ScanMaskLoop

				sub		esi, (4 * SIZE(dword))							; Restore esi to original value

				jmp		_NextPacket										; Go check next packet

;==============================================================================================================
; Exit
;==============================================================================================================

_Exit:			pop		edi
				pop		esi
				pop		edx
				pop		ecx
				pop		ebx

				mov		eax, _nIntersectCount
				ret

fdx8collasm_CollideWithLineSeg  ENDP






;--------------------------------------------------------------------------------------------------------------
; u32 fdx8collasm_BuildShadowReceiverList( FDX8MeshHW_SIMDCollPacket_t *pCollPacketArray, u32 nCollPacketCount, const CFVec3 *pStartPoint, const CFVec3 *pEndPoint, f32 fCylRadius, const CFVec3 *pUnitDir );

fdx8collasm_BuildShadowReceiverList PROC C pCollPacketArray:dword, nCollPacketCount:dword, pStartPoint:dword, pEndPoint:dword, fCylRadius:dword, pUnitDir:dword

				push	ebx
				push	ecx
				push	edx
				push	esi
				push	edi

				mov		_nIntersectCount, 0
				cmp		nCollPacketCount, 0								; Exit if there are no packets...
				je		_Exit

;==============================================================================================================
; Compute the cylinder's bounding sphere:
;==============================================================================================================

				movss	xmm0, pEndPoint._Vec3_s.fPosX					; xmm0-2 = half vector from start to end points...
				movss	xmm1, pEndPoint._Vec3_s.fPosY
				movss	xmm2, pEndPoint._Vec3_s.fPosZ
				subss	xmm0, pStartPoint._Vec3_s.fPosX
				subss	xmm1, pStartPoint._Vec3_s.fPosY
				subss	xmm2, pStartPoint._Vec3_s.fPosZ
				mulss	xmm0, _qfHalf
				mulss	xmm1, _qfHalf
				mulss	xmm2, _qfHalf

				movss	xmm3, xmm0										; xmm3-5 = copy of half vector...
				movss	xmm4, xmm1
				movss	xmm5, xmm2

				addss	xmm0, pStartPoint._Vec3_s.fPosX					; Compute bound sphere center...
				addss	xmm1, pStartPoint._Vec3_s.fPosY
				addss	xmm2, pStartPoint._Vec3_s.fPosZ
				movss	_Sphere._Sphere_s.fPosX, xmm0
				movss	_Sphere._Sphere_s.fPosY, xmm1
				movss	_Sphere._Sphere_s.fPosZ, xmm2

				mulss	xmm3, xmm3										; xmm3 = length squared of half vector...
				mulss	xmm4, xmm4
				mulss	xmm5, xmm5
				addss	xmm3, xmm4
				addss	xmm3, xmm5

				movss	xmm4, fCylRadius								; xmm4 = cylinder radius squared...
				mulss	xmm4, xmm4

				addss	xmm3, xmm4										; Compute sphere radius...
				sqrtss	xmm0, xmm3
				movss	_Sphere._Sphere_s.fRadius, xmm0

;==============================================================================================================
; Expand pStartPoint, pEndPoint, pUnitDir, and _Sphere:
;==============================================================================================================

				;---------------------------------------------------
				; Expand start point:
				;---------------------------------------------------

				mov		ecx, (SIZE(_Vec3_s) / SIZE(dword))				; ecx = number of dwords in source pStartPoint to expand...
				mov		esi, pStartPoint								; esi points to source vector
				mov		edi, OFFSET _XStartPoint						; edi points to destination vector
@@:				movlps	xmm0, [esi]										; Expand one dword into 4 copies...
				shufps	xmm0, xmm0, 00h
				movaps	[edi], xmm0
				add		esi, SIZE(dword)								; Point esi to next source dword
				add		edi, (SIZE(dword) * 4)							; Point edi to next destination quad-dword
				loop	@B												; Do it until done

				;---------------------------------------------------
				; Expand end point:
				;---------------------------------------------------

				mov		ecx, (SIZE(_Vec3_s) / SIZE(dword))				; ecx = number of dwords in source pStartPoint to expand...
				mov		esi, pEndPoint									; esi points to source vector
				mov		edi, OFFSET _XEndPoint							; edi points to destination vector
@@:				movlps	xmm0, [esi]										; Expand one dword into 4 copies...
				shufps	xmm0, xmm0, 00h
				movaps	[edi], xmm0
				add		esi, SIZE(dword)								; Point esi to next source dword
				add		edi, (SIZE(dword) * 4)							; Point edi to next destination quad-dword
				loop	@B												; Do it until done

				;---------------------------------------------------
				; Expand unit dir:
				;---------------------------------------------------

				mov		ecx, (SIZE(_Vec3_s) / SIZE(dword))				; ecx = number of dwords in source pStartPoint to expand...
				mov		esi, pUnitDir									; esi points to source vector
				mov		edi, OFFSET _XUnitDir							; edi points to destination vector
@@:				movlps	xmm0, [esi]										; Expand one dword into 4 copies...
				shufps	xmm0, xmm0, 00h
				movaps	[edi], xmm0
				add		esi, SIZE(dword)								; Point esi to next source dword
				add		edi, (SIZE(dword) * 4)							; Point edi to next destination quad-dword
				loop	@B												; Do it until done

				;---------------------------------------------------
				; Expand sphere:
				;---------------------------------------------------

				mov		ecx, (SIZE(_Sphere_s) / SIZE(dword))			; ecx = number of dwords in sphere to expand
				mov		esi, OFFSET _Sphere								; esi points to source master sphere (unless...)
				mov		edi, OFFSET _XMasterSphere						; edi points to destination master sphere
@@:				movlps	xmm0, [esi]										; Expand one dword into 4 copies...
				shufps	xmm0, xmm0, 00h
				movaps	[edi], xmm0

				add		esi, SIZE(dword)								; Point esi to next source dword
				add		edi, (SIZE(dword) * 4)							; Point edi to next destination quad-dword
				loop	@B												; Do it until done

;==============================================================================================================
; Prepare for main loop:
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; ecx is the number of remaining packets
;==============================================================================================================

				mov		esi, pCollPacketArray							; esi points to the packet array...
				ASSUME	esi: ptr _CollPacket_s

				mov		ebx, _pIntersectBuf								; ebx points into our destination intersect buffer
				ASSUME	ebx: ptr _CollIntersect_s

				mov		ecx, nCollPacketCount

;==============================================================================================================
; Test the cylinder bounding sphere against the packet triangle spheres.
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; ecx is the number of remaining packets
;==============================================================================================================

				;---------------------------------------------------
				; Load xmm0-3 with expanded cylinder bound sphere:
				;---------------------------------------------------

_PacketLoop1:	movaps	xmm0, _XMasterSphere._XSphere_s.qfRadius
				movaps	xmm1, _XMasterSphere._XSphere_s.qfPosX
				movaps	xmm2, _XMasterSphere._XSphere_s.qfPosY
				movaps	xmm3, _XMasterSphere._XSphere_s.qfPosZ

				;---------------------------------------------------
				; Load xmm4-7 with our collision packet spheres:
				;---------------------------------------------------

_PacketLoop2:	movaps	xmm4, [esi].qfBoundSphereRadius
				movaps	xmm5, [esi].qfBoundSpherePosX
				movaps	xmm6, [esi].qfBoundSpherePosY
				movaps	xmm7, [esi].qfBoundSpherePosZ

				;---------------------------------------------------
				; Compute the distance-squared between
				; the two sphere centers:
				;---------------------------------------------------

				subps	xmm5, xmm1										; xmm5 = fDx
				subps	xmm6, xmm2										; xmm6 = fDy
				subps	xmm7, xmm3										; xmm7 = fDz
				mulps	xmm5, xmm5										; xmm5 = fDx*fDx
				mulps	xmm6, xmm6										; xmm6 = fDy*fDy
				mulps	xmm7, xmm7										; xmm7 = fDz*fDz
				addps	xmm5, xmm6										; xmm5 = fDx*fDx + fDy*fDy
				addps	xmm5, xmm7										; xmm5 = fDx*fDx + fDy*fDy + fDz*fDz

				;---------------------------------------------------
				; Compute the square of the sum of the
				; two sphere radii:
				;---------------------------------------------------

				addps	xmm4, xmm0										; xmm4 = fRm + fRt
				mulps	xmm4, xmm4										; xmm4 = (fRm + fRt)*(fRm + fRt)

				;---------------------------------------------------
				; Find out if any of the 4 triangle spheres
				; intersect the master sphere:
				;---------------------------------------------------

				cmpltps	xmm5, xmm4										; Set mask if spheres intersect
				movmskps eax, xmm5										; Move mask to eax
				test	eax, 1111b										; Are any bits set?
				jnz		_TestCylRadius									; Jump if so

				;---------------------------------------------------
				; None of the 4 triangle spheres intersect the
				; cylinder bound sphere. Point to next packet:
				;---------------------------------------------------

_NextPacket:

				add		esi, _COLLPACKET_BYTES							; Point to next packet
				dec		ecx												; Loop if more packets...
				jnz		_PacketLoop2
				jmp		_Exit											; No more packets. Exit function!

;==============================================================================================================
; At least one of the triangle spheres intersects the cylinder bound sphere.
; Test the cylinder radius against the packet triangle spheres.
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; ecx is the number of remaining packets
; dl = current intersection mask
;==============================================================================================================

_TestCylRadius:
				mov		dl, al											; Save mask in dl

				;---------------------------------------------------
				; xmm4 = (cyl sphere radius + tri sphere radius)^2
				;---------------------------------------------------
				movaps	xmm4, [esi].qfBoundSphereRadius
				addps	xmm4, _XMasterSphere._XSphere_s.qfRadius
				mulps	xmm4, xmm4

				;---------------------------------------------------
				; xmm0-2 = xmm5-7 = LineStartToSphereCenter
				;---------------------------------------------------

				movaps	xmm0, [esi].qfBoundSpherePosX
				movaps	xmm1, [esi].qfBoundSpherePosY
				movaps	xmm2, [esi].qfBoundSpherePosZ
				subps	xmm0, _XStartPoint._XVec3_s.fPosX
				subps	xmm1, _XStartPoint._XVec3_s.fPosY
				subps	xmm2, _XStartPoint._XVec3_s.fPosZ

				movaps	xmm5, xmm0
				movaps	xmm6, xmm1
				movaps	xmm7, xmm2

				;---------------------------------------------------
				; xmm0 = LineStartToSphereCenter DOT LineUnitDir
				;---------------------------------------------------

				mulps	xmm0, _XUnitDir._XVec3_s.fPosX
				mulps	xmm1, _XUnitDir._XVec3_s.fPosY
				mulps	xmm2, _XUnitDir._XVec3_s.fPosZ
				addps	xmm0, xmm1
				addps	xmm0, xmm2

				;------------------------------------------------------------------
				; xmm1-3 = LineUnitDir*(LineStartToSphereCenter DOT LineUnitDir)
				;------------------------------------------------------------------

				movaps	xmm1, _XUnitDir._XVec3_s.fPosX
				movaps	xmm2, _XUnitDir._XVec3_s.fPosY
				movaps	xmm3, _XUnitDir._XVec3_s.fPosZ
				mulps	xmm1, xmm0
				mulps	xmm2, xmm0
				mulps	xmm3, xmm0

				;------------------------------------------------------------------------------------------
				; xmm1-3 = ShortestVecFromSphereCenterToLine
				;        = LineUnitDir*(LineStartToSphereCenter DOT LineUnitDir) - LineStartToSphereCenter
				;------------------------------------------------------------------------------------------

				subps	xmm1, xmm5
				subps	xmm2, xmm6
				subps	xmm3, xmm7

				;---------------------------------------------------------------
				; xmm1 = magnitude squared of ShortestVecFromSphereCenterToLine
				;---------------------------------------------------------------

				mulps	xmm1, xmm1
				mulps	xmm2, xmm2
				mulps	xmm3, xmm3
				addps	xmm1, xmm2
				addps	xmm1, xmm3

				;---------------------------------------------------
				; Find out if the distance from the sphere center to
				; the line falls within the radius sum:
				;---------------------------------------------------

				cmpleps	xmm1, xmm4										; Set mask if line intersects sphere
				movmskps eax, xmm1										; Move mask to eax
				and		dl, al											; Any intersections remaining?
				jz		_NextPacket										; Jump if not

;==============================================================================================================
; Test to see if the sphere is beyond either end of the cylinder:
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; ecx is the number of remaining packets
; dl = current intersection mask
; xmm0 = LineStartToSphereCenter DOT LineUnitDir
;==============================================================================================================

				;---------------------------------------------------
				; Check if tri sphere is beyond cylinder start cap:
				;---------------------------------------------------

				xorps	xmm0, _qfNegBits								; xmm0 = dist from start cap to tri sphere center, along cyl axis
				cmpleps	xmm0, [esi].qfBoundSphereRadius					; Set mask if tri sphere isn't beyond start cap
				movmskps eax, xmm0										; Move mask to eax
				and		dl, al											; Any intersections remaining?
				jz		_NextPacket										; Jump if not

				;---------------------------------------------------
				; Check if tri sphere is beyond cylinder end cap:
				;---------------------------------------------------

				movaps	xmm0, [esi].qfBoundSpherePosX					; xmm0-2 = vector from cyl end point to tri sphere center...
				movaps	xmm1, [esi].qfBoundSpherePosY
				movaps	xmm2, [esi].qfBoundSpherePosZ
				subps	xmm0, _XEndPoint._XVec3_s.fPosX
				subps	xmm1, _XEndPoint._XVec3_s.fPosY
				subps	xmm2, _XEndPoint._XVec3_s.fPosZ

				mulps	xmm0, _XUnitDir._XVec3_s.fPosX					; xmm0 = vector DOT cyl unit dir...
				mulps	xmm1, _XUnitDir._XVec3_s.fPosY
				mulps	xmm2, _XUnitDir._XVec3_s.fPosZ
				addps	xmm0, xmm1
				addps	xmm0, xmm2

				cmpleps	xmm0, [esi].qfBoundSphereRadius					; Set mask if tri sphere isn't beyond end cap
				movmskps eax, xmm0										; Move mask to eax
				and		dl, al											; Any intersections remaining?
				jz		_NextPacket										; Jump if not

;==============================================================================================================
; Test to see if the triangle is facing the light direction:
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; ecx is the number of remaining packets
; dl = current intersection mask
; xmm0 = LineStartToSphereCenter DOT LineUnitDir
;==============================================================================================================

				xorps	xmm3, xmm3										; xmm3 = 0

				movaps	xmm0, [esi].qfUnitFaceNormX						; xmm0-2 = tri's unit face normal...
				movaps	xmm1, [esi].qfUnitFaceNormY
				movaps	xmm2, [esi].qfUnitFaceNormZ

				mulps	xmm0, _XUnitDir._XVec3_s.fPosX					; xmm0 = tri normal DOT light dir...
				mulps	xmm1, _XUnitDir._XVec3_s.fPosY
				mulps	xmm2, _XUnitDir._XVec3_s.fPosZ
				addps	xmm0, xmm1
				addps	xmm0, xmm2

				cmpltps	xmm0, xmm3										; Set mask if tri is facing light (DOT is negative)
				movmskps eax, xmm0										; Move mask to eax
				and		dl, al											; Any intersections remaining?
				jz		_NextPacket										; Jump if not

;==============================================================================================================
; Create an intersection entry for any of the set bits in our mask register, dl:
;
; esi points to array of FDX8MeshHW_SIMDCollPacket_t
; ebx points to the next slot in our output intersect buffer
; ecx is the number of remaining packets
; dl is our impact mask
;==============================================================================================================

				mov		dh, 4											; Only look at lower 4 bits
_ScanMaskLoop:	shr		dl, 1											; Move bit into carry flag
				jnc		_ScanNextMask									; Jump if mask bit is 0

				;---------------------------------------------------
				; Mask bit is 1.
				; Check if there's room to create one intersect:
				;---------------------------------------------------

				cmp		ebx, _pIntersectBufEnd							; Do we have room in our intersect buffer?
				jae		_Exit											; Jump if not

				;---------------------------------------------------
				; There's room in our intersect buffer.
				; Create one intersect:
				;---------------------------------------------------

				mov		eax, [esi].apCollData							; Create one intersect entry...
				or		eax, eax
				jz		_ScanNextMask
				mov		[ebx].pCollData, eax
				mov		[ebx].nSphereIndex, 0
				mov		eax, [esi].qfUnitFaceNormX
				mov		[ebx].fUnitFaceNormX, eax
				mov		eax, [esi].qfUnitFaceNormY
				mov		[ebx].fUnitFaceNormY, eax
				mov		eax, [esi].qfUnitFaceNormZ
				mov		[ebx].fUnitFaceNormZ, eax

				inc		_nIntersectCount								; Count the intersect
				add		ebx, _COLLINTERSECT_BYTES						; Point to next intersect entry

_ScanNextMask:	add		esi, SIZE(dword)								; Adjust packet pointer for next bit
				dec		dh												; Loop until we've checked all 4 bits...
				jnz		_ScanMaskLoop

				sub		esi, (4 * SIZE(dword))							; Restore esi to original value

				jmp		_NextPacket										; Go check next packet

;==============================================================================================================
; Exit
;==============================================================================================================

_Exit:			pop		edi
				pop		esi
				pop		edx
				pop		ecx
				pop		ebx

				mov		eax, _nIntersectCount
				ret

fdx8collasm_BuildShadowReceiverList  ENDP


_TEXT	ends
		end

