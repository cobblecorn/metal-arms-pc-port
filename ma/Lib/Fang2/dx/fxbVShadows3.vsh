;Per-pixel shadow projection.

#include "fdx8vshader_const.h"

vs.1.0

#define V_POS v0
#define V_NORMAL v1

#define R_WORLD_NORMAL r1
#define R_WORLD_VERTEX r0
#define R_VERTEX_TO_LIGHT r5
#define R_DIFFUSE1 r6
#define R_TEMP r7
#define R_ATTEN r9

#define R_SPOT r8

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

dp3 R_WORLD_NORMAL.x, V_NORMAL, c[CV_WORLD_0]
dp3 R_WORLD_NORMAL.y, V_NORMAL, c[CV_WORLD_1]
dp3 R_WORLD_NORMAL.z, V_NORMAL, c[CV_WORLD_2]

; ************** Projection ***************
dp4 oT0.x, R_WORLD_VERTEX, c[CV_LIGHTMTX0_0]
dp4 oT0.y, R_WORLD_VERTEX, c[CV_LIGHTMTX0_1]
dp4 oT0.w, R_WORLD_VERTEX, c[CV_LIGHTMTX0_2]
dp4 oT0.z, R_WORLD_VERTEX, c[CV_LIGHTMTX0_3]

dp4 oT1.x, R_WORLD_VERTEX, c[CV_LIGHTMTX1_0]
dp4 oT1.y, R_WORLD_VERTEX, c[CV_LIGHTMTX1_1]
dp4 oT1.w, R_WORLD_VERTEX, c[CV_LIGHTMTX1_2]
dp4 oT1.z, R_WORLD_VERTEX, c[CV_LIGHTMTX1_3]

dp4 oT2.x, R_WORLD_VERTEX, c[CV_LIGHTMTX2_0]
dp4 oT2.y, R_WORLD_VERTEX, c[CV_LIGHTMTX2_1]
dp4 oT2.w, R_WORLD_VERTEX, c[CV_LIGHTMTX2_2]
dp4 oT2.z, R_WORLD_VERTEX, c[CV_LIGHTMTX2_3]

;Light 0
add R_VERTEX_TO_LIGHT, R_WORLD_VERTEX, -c[CV_LIGHT5_POSITION]
dp3 R_ATTEN.w, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT
mov R_TEMP.w, R_ATTEN.w
mul R_ATTEN.w, R_ATTEN.w, c[CV_LIGHT5_POSITION].w

max R_ATTEN.w, c[CV_ZERO], R_ATTEN.w
min R_ATTEN.w, c[CV_ONE], R_ATTEN.w

add R_ATTEN.w, c[CV_ONE], -R_ATTEN.w

rsq R_TEMP.w, R_TEMP.w
mul R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT, R_TEMP.w
dp3 R_SPOT.w, R_VERTEX_TO_LIGHT, c[CV_LIGHT1_DIRECTION]
mad R_SPOT.w, R_SPOT.w, c[CV_LIGHT1_DIFFUSE].z, c[CV_LIGHT1_DIFFUSE].y

max R_SPOT.w, c[CV_ZERO], R_SPOT.w
min R_SPOT.w, c[CV_ONE], R_SPOT.w

;mul oD0.xyz, R_SPOT.w, R_ATTEN.w
mov oD0.xyz, c[CV_ONE]

;Light 1
add R_VERTEX_TO_LIGHT, R_WORLD_VERTEX, -c[CV_LIGHT6_POSITION]
dp3 R_ATTEN.w, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT
mov R_TEMP.w, R_ATTEN.w
mul R_ATTEN.w, R_ATTEN.w, c[CV_LIGHT6_POSITION].w

max R_ATTEN.w, c[CV_ZERO], R_ATTEN.w
min R_ATTEN.w, c[CV_ONE], R_ATTEN.w

add R_ATTEN.w, c[CV_ONE], -R_ATTEN.w

rsq R_TEMP.w, R_TEMP.w
mul R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT, R_TEMP.w
dp3 R_SPOT.w, R_VERTEX_TO_LIGHT, c[CV_LIGHT2_DIRECTION]
mad R_SPOT.w, R_SPOT.w, c[CV_LIGHT2_DIFFUSE].z, c[CV_LIGHT2_DIFFUSE].y

max R_SPOT.w, c[CV_ZERO], R_SPOT.w
min R_SPOT.w, c[CV_ONE], R_SPOT.w

mul oD1.xyz, R_SPOT.w, R_ATTEN.w

;Light 2
add R_VERTEX_TO_LIGHT, R_WORLD_VERTEX, -c[CV_LIGHT7_POSITION]
dp3 R_ATTEN.w, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT
mov R_TEMP.w, R_ATTEN.w
mul R_ATTEN.w, R_ATTEN.w, c[CV_LIGHT7_POSITION].w

max R_ATTEN.w, c[CV_ZERO], R_ATTEN.w
min R_ATTEN.w, c[CV_ONE], R_ATTEN.w

add R_ATTEN.w, c[CV_ONE], -R_ATTEN.w

rsq R_TEMP.w, R_TEMP.w
mul R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT, R_TEMP.w
dp3 R_SPOT.w, R_VERTEX_TO_LIGHT, c[CV_LIGHT3_DIRECTION]
mad R_SPOT.w, R_SPOT.w, c[CV_LIGHT3_DIFFUSE].z, c[CV_LIGHT3_DIFFUSE].y

max R_SPOT.w, c[CV_ZERO], R_SPOT.w
min R_SPOT.w, c[CV_ONE], R_SPOT.w

mul oD0.w, R_SPOT.w, R_ATTEN.w
