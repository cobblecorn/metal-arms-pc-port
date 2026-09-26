;LightMap Passthru

#include "fdx8vshader_const.h"

vs.1.0

#define V_POS v0
#define V_NORMAL v1
#define V_COLOR v2
;Base UV
#define V_TEX0 v3
;LMap UVs
#define V_TEX1 v4
#define V_TEX2 v5
#define V_TEX3 v6
#define V_TEX4 v7

#define R_EYE_VERTEX r8
#define R_WORLD_VERTEX r9

; Transform position to clip space and output it
dp4 oPos.x, V_POS, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POS, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POS, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POS, c[CV_WORLDVIEWPROJ_3]

dp4 R_EYE_VERTEX.z, V_POS, c[CV_WORLDVIEW_2]
dp4 R_WORLD_VERTEX.y, V_POS, c[CV_WORLD_1]

mov oT0, V_TEX0
mov oT1, V_TEX0
mov oT2, V_TEX1

mov r0, c[CV_HALF]
mad oD0, c[CV_MAT_AMBIENT], r0, V_COLOR

mad r2.z, R_EYE_VERTEX.z, c[CV_FOG_DEPTH].x, c[CV_FOG_DEPTH].y
mad r2.w, R_WORLD_VERTEX.y, c[CV_FOG_DEPTH].z, c[CV_FOG_DEPTH].w
min r2.zw, r2, c[CV_ONE]
max r2.zw, r2, c[CV_FOG_MIN]
mov oT3.xyz, c[CV_ZERO]
mad oT3.w, r2.z, r2.w, c[CV_EPS].w
