;Per-pixel point light attenuation.

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
#define V_S v4
#define V_T v5

#define R_EYE_NORMAL r0
#define R_EYE_VERTEX r1
#define R_DIFFUSE1 r2

#define LIGHT_DIRECTION r3
#define S_VIEW r6
#define T_VIEW r7
#define LIGHT_LOCAL r8

; Transform position to clip space and output it
dp4 oPos.x, V_POS, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POS, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POS, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POS, c[CV_WORLDVIEWPROJ_3]

; Transform position to eye space
dp4 R_EYE_VERTEX.x, v0, c[CV_WORLDVIEW_0]
dp4 R_EYE_VERTEX.y, v0, c[CV_WORLDVIEW_1]
dp4 R_EYE_VERTEX.z, v0, c[CV_WORLDVIEW_2]
dp4 R_EYE_VERTEX.w, v0, c[CV_WORLDVIEW_3]

; Transform normal to eye space
dp3 R_EYE_NORMAL.x, v1, c[CV_WORLDVIEW_0]
dp3 R_EYE_NORMAL.y, v1, c[CV_WORLDVIEW_1]
dp3 R_EYE_NORMAL.z, v1, c[CV_WORLDVIEW_2]

; Normalize transformed normal
dp3 R_EYE_NORMAL.w, R_EYE_NORMAL, R_EYE_NORMAL
rsq R_EYE_NORMAL.w, R_EYE_NORMAL.w	
mul R_EYE_NORMAL, R_EYE_NORMAL, R_EYE_NORMAL.w

; ************** Per-Pixel Point 1 ***************
mad oT0.xy, c[CV_LIGHT1_ATTENUATION].ww, R_EYE_VERTEX.xz, c[CV_LIGHT1_ATTENUATION].xz
mad oT1.x, c[CV_LIGHT1_ATTENUATION].w, R_EYE_VERTEX.y, c[CV_LIGHT1_ATTENUATION].y
mov oT1.y, c[CV_HALF]

mov oD0, c[CV_LIGHT1_DIFFUSE_POINT_PP]

; ************* BumpMap Code *********************
; Calculate the vector to the light from the world vertex
add LIGHT_DIRECTION, -R_EYE_VERTEX, c[CV_LIGHT1_POSITION_PP]

;*****TEMP CODE HERE**********
;mov V_S.yzw, c[CV_ZERO]
;mov V_S.x, c[CV_ONE]
;mov V_T.xzw, c[CV_ZERO]
;mov V_T.y, c[CV_ONE]
;*****************************

; Transform basis vectors to world space
dp3 S_VIEW.x, V_S, c[CV_WORLDVIEW_0]
dp3 S_VIEW.y, V_S, c[CV_WORLDVIEW_1]
dp3 S_VIEW.z, V_S, c[CV_WORLDVIEW_2]

dp3 T_VIEW.x, V_T, c[CV_WORLDVIEW_0]
dp3 T_VIEW.y, V_T, c[CV_WORLDVIEW_1]
dp3 T_VIEW.z, V_T, c[CV_WORLDVIEW_2]

; transform light by basis vectors to put it
; into texture space
dp3 LIGHT_LOCAL.x, S_VIEW.xyz, LIGHT_DIRECTION
dp3 LIGHT_LOCAL.y, T_VIEW.xyz, LIGHT_DIRECTION
dp3 LIGHT_LOCAL.z, R_EYE_NORMAL.xyz, LIGHT_DIRECTION

; Normalize the light vector
dp3 LIGHT_LOCAL.w, LIGHT_LOCAL, LIGHT_LOCAL
rsq LIGHT_LOCAL.w, LIGHT_LOCAL.w
mul LIGHT_LOCAL, LIGHT_LOCAL, LIGHT_LOCAL.w

; Scale to 0-1
add LIGHT_LOCAL, LIGHT_LOCAL, c[CV_ONE]
mul oT3, LIGHT_LOCAL, c[CV_HALF]
mov oT3.w, c[CV_ONE]

; output tex coords
mul oT2.xy, V_TEX0, c[CV_BUMP_SCALE]
