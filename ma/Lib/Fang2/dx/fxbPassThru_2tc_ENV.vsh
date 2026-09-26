;Basic Passthru: vertex color & texture coord.

#include "fdx8vshader_const.h"

vs.1.0

#define V_POS v0
#define V_NORMAL v1
#define V_COLOR v2
#define V_TEX0 v3
#define V_TEX1 v4
#define V_TEX2 v5
#define V_TEX3 v6

#define R_DOT2		 r6
#define R_EYE_NORMAL r7
#define R_EYE_VERTEX r8
#define R_EYE_VECTOR r9

; Transform position to clip space and output it
dp4 oPos.x, V_POS, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POS, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POS, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POS, c[CV_WORLDVIEWPROJ_3]

;dp4 R_EYE_VERTEX.z, V_POS, c[CV_WORLDVIEW_2]
;dp4 R_WORLD_VERTEX.y, V_POS, c[CV_WORLD_1]

mov oD0.xyz, c[CV_FACTOR]
mov oD0.w, V_COLOR.w

mov r0.xy, V_TEX0
mov r0.z, c[CV_ONE]

dp3 r1.x, r0, c[CV_TEXMTX0_0]
dp3 r1.y, r0, c[CV_TEXMTX0_1]
mov r1.zw, c[CV_ZERO]

mov r0.xy, V_TEX1

dp3 r2.x, r0, c[CV_TEXMTX1_0]
dp3 r2.y, r0, c[CV_TEXMTX1_1]

mov oT0.xy, r1
mov oT1.xy, r2

mul oT3.xy, r1, c[CV_DETAIL_SCALE]

;*******************do reflection vector here
dp4 R_EYE_VERTEX.x, V_POS, c[CV_WORLDVIEW_0]
dp4 R_EYE_VERTEX.y, V_POS, c[CV_WORLDVIEW_1]
dp4 R_EYE_VERTEX.z, V_POS, c[CV_WORLDVIEW_2]
dp4 R_EYE_VERTEX.w, V_POS, c[CV_WORLDVIEW_3]

; Create R_EYE_VECTOR, the normalized vector from the eye to the vertex
dp3 R_EYE_VECTOR.w, R_EYE_VERTEX, R_EYE_VERTEX
rsq R_EYE_VECTOR.w, R_EYE_VECTOR.w		
mul R_EYE_VECTOR, R_EYE_VERTEX, R_EYE_VECTOR.w

; Transform normal to eye-space
; We use the inverse transpose of the worldview
; matrix to do this
dp3 R_EYE_NORMAL.x, V_NORMAL, c[CV_WORLDVIEW_0]
dp3 R_EYE_NORMAL.y, V_NORMAL, c[CV_WORLDVIEW_1]
dp3 R_EYE_NORMAL.z, V_NORMAL, c[CV_WORLDVIEW_2]

; Need to re-normalize normal
dp3 R_EYE_NORMAL.w, R_EYE_NORMAL, R_EYE_NORMAL
rsq R_EYE_NORMAL.w, R_EYE_NORMAL.w
mul R_EYE_NORMAL, R_EYE_NORMAL, R_EYE_NORMAL.w

; Calculate E - 2*(E dot N)*N
dp3 R_DOT2, R_EYE_VECTOR, R_EYE_NORMAL
add R_DOT2, R_DOT2, R_DOT2
mul R_EYE_NORMAL, R_EYE_NORMAL, R_DOT2

;Now (s,t)*0.5f + 0.5f
add R_EYE_VECTOR, R_EYE_VECTOR, -R_EYE_NORMAL
mul R_EYE_VECTOR, R_EYE_VECTOR, c[CV_HALF]
add oT2, R_EYE_VECTOR, c[CV_HALF]

mov oT2.w, c[CV_ONE].x
;****************************
