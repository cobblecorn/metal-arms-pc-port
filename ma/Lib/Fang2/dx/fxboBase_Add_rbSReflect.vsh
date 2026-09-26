#include "fdx8vshader_const.h"

vs.1.0

#define V_POS v0
#define V_NORMAL v1
#define V_COLOR v2
#define V_TEX0 v3
#define V_TEX1 v4
#define V_TEX2 v5
#define V_TEX3 v6

#define R_EYE_VERTEX r8
#define R_EYE_NORMAL r9
#define R_EYE_VECTOR r3
#define R_DOT2 r4

; Transform position
dp4 oPos.x, V_POS, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POS, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POS, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POS, c[CV_WORLDVIEWPROJ_3]

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

;Now (s,t)*0.5f + 0.5f
mul R_EYE_VECTOR, R_EYE_NORMAL, -c[CV_HALF]
add oT1, R_EYE_VECTOR, c[CV_HALF]

mov oT1.w, c[CV_ONE].x

mov r0.xy, V_TEX0
mov r0.z, c[CV_ONE]

dp3 oT0.x, r0, c[CV_TEXMTX0_0]
dp3 oT0.y, r0, c[CV_TEXMTX0_1]

mov oD0, c[CV_FACTOR]
mov oD1, c[CV_FACTOR]
