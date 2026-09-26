;Per-pixel spot light mask

#include "fdx8vshader_const.h"

;Need to pass in the following:
; Point Light Color*MaterialColor	[CV_LIGHT1_DIFFUSE]
;			  Ambient				[CV_LIGHT1_AMBIENT]
;			  Position (xyz)		[CV_LIGHT1_POSITION]
;			  Attenuation (a0,a1,a2)[CV_LIGHT1_ATTENUATION]
; Directional Light Clr*Material	[CV_LIGHT2_DIFFUSE]
;			  Direction				[CV_LIGHT2_DIRECTION]

vs.1.0

#define V_POS v0
#define V_NORMAL v1
#define V_COLOR v2
#define V_TEX0 v3
#define V_TEX1 v4
#define V_TEX2 v5
#define V_TEX3 v6

#define R_WORLD_NORMAL r1
#define R_WORLD_VERTEX r0
#define R_VERTEX_TO_LIGHT r9
#define R_DIFFUSE1 r6
#define R_DIFFUSE2 r7

#define R_ATTEN r9

#define R_TEMP r2
#define R_TEXKILL r8

; Transform position to clip space and output it
dp4 oPos.x, V_POS, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POS, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POS, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POS, c[CV_WORLDVIEWPROJ_3]

; Transform position to world space
dp4 R_WORLD_VERTEX.x, V_POS, c[CV_WORLD_0]
dp4 R_WORLD_VERTEX.y, V_POS, c[CV_WORLD_1]
dp4 R_WORLD_VERTEX.z, V_POS, c[CV_WORLD_2]
dp4 R_WORLD_VERTEX.w, V_POS, c[CV_WORLD_3]

; Transform normal to world space
dp3 R_WORLD_NORMAL.x, V_NORMAL, c[CV_WORLD_0]
dp3 R_WORLD_NORMAL.y, V_NORMAL, c[CV_WORLD_1]
dp3 R_WORLD_NORMAL.z, V_NORMAL, c[CV_WORLD_2]

; Normalize transformed normal
dp3 R_WORLD_NORMAL.w, R_WORLD_NORMAL, R_WORLD_NORMAL
rsq R_WORLD_NORMAL.w, R_WORLD_NORMAL.w	
mul R_WORLD_NORMAL, R_WORLD_NORMAL, R_WORLD_NORMAL.w

; ************** Per-Pixel Point 1 ***************
dp4 oT0.x, R_WORLD_VERTEX, c[CV_LIGHTMTX0_0]
dp4 oT0.y, R_WORLD_VERTEX, c[CV_LIGHTMTX0_1]
dp4 oT0.w, R_WORLD_VERTEX, c[CV_LIGHTMTX0_2]
dp4 oT0.z, R_WORLD_VERTEX, c[CV_LIGHTMTX0_3]

dp3 R_TEXKILL.x, R_WORLD_VERTEX.xyz, -c[CV_LIGHT1_ATTENUATION_PP].xyz
add oT1.x, R_TEXKILL.x, -c[CV_LIGHT1_ATTENUATION_PP].w
mov oT1.y, c[CV_ZERO]

dp3 R_DIFFUSE1.x, c[CV_LIGHT1_DIRECTION_PP], R_WORLD_NORMAL
max R_DIFFUSE1.x, c[CV_ZERO].x, R_DIFFUSE1.x

add R_ATTEN.xyz, R_WORLD_VERTEX, -c[CV_LIGHT1_POSITION_PP]
dp3 R_ATTEN.w, R_ATTEN, R_ATTEN
mul R_ATTEN.w, R_ATTEN.w, c[CV_LIGHT1_POSITION_PP].w

min R_ATTEN.w, c[CV_ONE], R_ATTEN.w
max R_ATTEN.w, c[CV_ZERO], R_ATTEN.w

add R_ATTEN.w, c[CV_ONE], -R_ATTEN.w

mul R_ATTEN.w, R_ATTEN.w, R_DIFFUSE1.xxxx
mul oD0.xyz, c[CV_LIGHT1_DIFFUSE_PP], R_ATTEN.w;

; ************** Per-Pixel Point 2 ***************
dp4 oT2.x, R_WORLD_VERTEX, c[CV_LIGHTMTX1_0]
dp4 oT2.y, R_WORLD_VERTEX, c[CV_LIGHTMTX1_1]
dp4 oT2.w, R_WORLD_VERTEX, c[CV_LIGHTMTX1_2]
dp4 oT2.z, R_WORLD_VERTEX, c[CV_LIGHTMTX1_3]

dp3 R_TEXKILL.x, R_WORLD_VERTEX.xyz, -c[CV_LIGHT2_ATTENUATION_PP].xyz
add oT3.x, R_TEXKILL.x, -c[CV_LIGHT2_ATTENUATION_PP].w
mov oT3.y, c[CV_ZERO]

dp3 R_DIFFUSE2, c[CV_LIGHT2_DIRECTION_PP], R_WORLD_NORMAL
min R_DIFFUSE2, c[CV_ONE], R_DIFFUSE2
max R_DIFFUSE2, c[CV_ZERO], R_DIFFUSE2

add R_ATTEN.xyz, R_WORLD_VERTEX, -c[CV_LIGHT2_POSITION_PP]
dp3 R_ATTEN.w, R_ATTEN, R_ATTEN
mul R_ATTEN.w, R_ATTEN.w, c[CV_LIGHT2_POSITION_PP].w

min R_ATTEN.w, c[CV_ONE], R_ATTEN.w
max R_ATTEN.w, c[CV_ZERO], R_ATTEN.w

add R_ATTEN.w, c[CV_ONE], -R_ATTEN.w

mul R_ATTEN.w, R_ATTEN.w, R_DIFFUSE2
;mul oD1, c[CV_LIGHT2_DIFFUSE_PP], R_ATTEN.w
mov oD0.w, R_ATTEN.w
