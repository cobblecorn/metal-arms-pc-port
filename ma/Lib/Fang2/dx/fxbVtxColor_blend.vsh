;Basic Vertex Color Passthru: Used for static vertex lighting (ie WorldGeometry with no dynamic lighting).

#include "fdx8vshader_const.h"

vs.1.0

#define V_POS v0
#define V_BLEND v1
#define V_NORMAL v2
#define V_COLOR v3
#define V_TEX0 v4
#define V_TEX1 v5
#define V_TEX2 v6
#define V_TEX3 v7

//Blend my vertex;
dp4 r1.x, V_POS, c[CV_BONEMTX0_0]
dp4 r1.y, V_POS, c[CV_BONEMTX0_1]
dp4 r1.z, V_POS, c[CV_BONEMTX0_2]

//second bone results -> r2
dp4 r2.x, V_POS, c[CV_BONEMTX1_0]
dp4 r2.y, V_POS, c[CV_BONEMTX1_1]
dp4 r2.z, V_POS, c[CV_BONEMTX1_2]

//third bone results -> r3
dp4 r3.x, V_POS, c[CV_BONEMTX2_0]
dp4 r3.y, V_POS, c[CV_BONEMTX2_1]
dp4 r3.z, V_POS, c[CV_BONEMTX2_2]

//fourth bone results -> r4
dp4 r4.x, V_POS, c[CV_BONEMTX3_0]
dp4 r4.y, V_POS, c[CV_BONEMTX3_1]
dp4 r4.z, V_POS, c[CV_BONEMTX3_2]

//find fourth bone weight: w4 = 1 - (w1+w2+w3)
add r5.x, V_BLEND.x, V_BLEND.y
add r5.x, r5.x, V_BLEND.z
add r5.x, c[CV_ONE], -r5.x

//Blend verts by weight: v1*w1 + v2*w2 + v3*w3 + v4*w4 
mul r1.xyz, r1.xyz, V_BLEND.x
mad r2, r2.xyz, V_BLEND.y, r1.xyz
mad r3, r3.xyz, V_BLEND.z, r2.xyz
mad r4, r4.xyz, r5.x, r3.xyz

mov r4.w, c[CV_ONE].z //set w to one
//r4 now contains final position

; Transform position to clip space and output it
dp4 oPos.x, r4, c[CV_VIEWPROJ_0]
dp4 oPos.y, r4, c[CV_VIEWPROJ_1]
dp4 oPos.z, r4, c[CV_VIEWPROJ_2]
dp4 oPos.w, r4, c[CV_VIEWPROJ_3]

add oD0, V_COLOR, c[CV_MAT_AMBIENT]
;mov oT0, V_TEX0

mov r0.xy, V_TEX0
mov r0.z, c[CV_ONE]

dp3 r1.x, r0, c[CV_TEXMTX0_0]
dp3 r1.y, r0, c[CV_TEXMTX0_1]

;for testing specular
;mov oD0, c[CV_ZERO]
mov oT0.xy, r1
mov oT1.xy, r1
mov oT2, c[CV_MAT_EMISSIVE]
