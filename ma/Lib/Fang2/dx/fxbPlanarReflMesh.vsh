;Basic Vertex Color Passthru: Used for static vertex lighting (ie WorldGeometry with no dynamic lighting).

#include "fdx8vshader_const.h"

vs.1.1

#define V_POS v0
#define V_NORMAL v1
#define V_COLOR v2
#define V_TEX0 v3

#define V_S r0
#define V_T r1
#define V_N r2
#define V_EYE r3
#define V_XYZ r5

#define VIEW_S r6
#define VIEW_T r7
#define VIEW_N r8

; Transform position to clip space and output it
dp4 oPos.x, V_POS, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POS, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POS, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POS, c[CV_WORLDVIEWPROJ_3]

dp4 V_XYZ.x, V_POS, c[CV_WORLD_0]
dp4 V_XYZ.y, V_POS, c[CV_WORLD_1]
dp4 V_XYZ.z, V_POS, c[CV_WORLD_2]
mov V_XYZ.w, c[CV_ONE]

add V_EYE, c[CV_EYE_POS_WORLD], -V_XYZ
;mov V_EYE, c[CV_EYE_POS_WORLD]
mov V_S, c[CV_ZERO]
mov V_S.x, c[CV_ONE]

mov V_T, c[CV_ZERO]
mov V_T.z, c[CV_ONE]

mov V_N, c[CV_ZERO]
mov V_N.y, c[CV_ONE]

;dp3 oT1.x, V_S, c[CV_WORLD_0];
;dp3 oT1.y, V_T, c[CV_WORLD_0];
;dp3 oT1.z, V_N, c[CV_WORLD_0];
mov oT1.xyz, V_S
mov oT1.w, V_EYE.x

;dp3 oT2.x, V_S, c[CV_WORLD_1];
;dp3 oT2.y, V_T, c[CV_WORLD_1];
;dp3 oT2.z, V_N, c[CV_WORLD_1];
mov oT2.xyz, V_T
mov oT2.w, V_EYE.y

;dp3 oT3.x, V_S, c[CV_WORLD_2];
;dp3 oT3.y, V_T, c[CV_WORLD_2];
;dp3 oT3.z, V_N, c[CV_WORLD_2];

mov oT3.xyz, V_N
mov oT3.w, V_EYE.z

mov oT0, V_TEX0

mov oD0, V_COLOR

