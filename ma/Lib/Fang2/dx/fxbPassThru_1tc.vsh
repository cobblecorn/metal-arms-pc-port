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

#define R_EYE_VERTEX r8
#define R_WORLD_VERTEX r9

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

mov oT0.xy, r1
mov oT1.xy, r1
mov oT2.xy, r1

;mad r2.z, R_EYE_VERTEX.z, c[CV_FOG_DEPTH].x, c[CV_FOG_DEPTH].y
;mad r2.w, R_WORLD_VERTEX.y, c[CV_FOG_DEPTH].z, c[CV_FOG_DEPTH].w
;max r2.zw, r2, c[CV_ZERO]
;mov oT2.xyz, c[CV_ONE]
;mad oT2.w, r2.z, r2.w, c[CV_EPS].w