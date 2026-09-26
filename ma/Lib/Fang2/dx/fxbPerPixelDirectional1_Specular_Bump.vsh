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
#define V_S v4
#define V_T v5

#define R_WORLD_VERTEX r0
#define R_WORLD_NORMAL r1
#define R_DIFFUSE1 r2
#define S_WORLD r3
#define T_WORLD r4
#define EYE_VECTOR r5
#define R_TEXKILL r6

#define HALF_ANGLE r9

; Transform position to clip space and output it
dp4 oPos.x, V_POS, c[CV_WORLDVIEWPROJ_0]
dp4 oPos.y, V_POS, c[CV_WORLDVIEWPROJ_1]
dp4 oPos.z, V_POS, c[CV_WORLDVIEWPROJ_2]
dp4 oPos.w, V_POS, c[CV_WORLDVIEWPROJ_3]

; Transform position to world space
dp4 R_WORLD_VERTEX.x, V_POS, c[CV_WORLDVIEW_0]
dp4 R_WORLD_VERTEX.y, V_POS, c[CV_WORLDVIEW_1]
dp4 R_WORLD_VERTEX.z, V_POS, c[CV_WORLDVIEW_2]
dp4 R_WORLD_VERTEX.w, V_POS, c[CV_WORLDVIEW_3]

; Transform normal to world space
dp3 R_WORLD_NORMAL.x, V_NORMAL, c[CV_WORLDVIEW_0]
dp3 R_WORLD_NORMAL.y, V_NORMAL, c[CV_WORLDVIEW_1]
dp3 R_WORLD_NORMAL.z, V_NORMAL, c[CV_WORLDVIEW_2]

; Normalize transformed normal
dp3 R_WORLD_NORMAL.w, R_WORLD_NORMAL, R_WORLD_NORMAL
rsq R_WORLD_NORMAL.w, R_WORLD_NORMAL.w	
mul R_WORLD_NORMAL, R_WORLD_NORMAL, R_WORLD_NORMAL.w

;*****TEMP CODE HERE**********
;mov V_S.yzw, c[CV_ZERO]
;mov V_S.x, c[CV_ONE]
;mov V_T.xzw, c[CV_ZERO]
;mov V_T.y, c[CV_ONE]
;*****************************

; Transform basis vectors to world space
dp3 S_WORLD.x, V_S, c[CV_WORLDVIEW_0]
dp3 S_WORLD.y, V_S, c[CV_WORLDVIEW_1]
dp3 S_WORLD.z, V_S, c[CV_WORLDVIEW_2]

dp3 T_WORLD.x, V_T, c[CV_WORLDVIEW_0]
dp3 T_WORLD.y, V_T, c[CV_WORLDVIEW_1]
dp3 T_WORLD.z, V_T, c[CV_WORLDVIEW_2]

/////////////////////////////////////
// Calculate half angle vector

// transform vertex position to world space
//  to calculate V, vector to viewer in world
//  space.

; Half angle vector is  (L+V)/||L+V||  or Normalize( L+V )
;  ||a|| is magnitude of a
;  L = vec to light from vertex point
;  V = vec to viewer from vertex point

// vertex position - eye position
// eye position - vertex position
add EYE_VECTOR,  c[ CV_ZERO ], -R_WORLD_VERTEX.xyz

; Normalize eye vec
dp3 EYE_VECTOR.w, EYE_VECTOR, EYE_VECTOR
rsq EYE_VECTOR.w, EYE_VECTOR.w
mul EYE_VECTOR,   EYE_VECTOR, EYE_VECTOR.w

// Add them to average & create half angle vector
add HALF_ANGLE,  c[CV_LIGHT1_DIRECTION_PP], EYE_VECTOR

;  Normalize it
dp3 HALF_ANGLE.w, HALF_ANGLE, HALF_ANGLE
rsq HALF_ANGLE.w, HALF_ANGLE.w
mul HALF_ANGLE,   HALF_ANGLE, HALF_ANGLE.w

/////////////////////////////////////////////////
; Transform half angle vector to local space and
;  output to tex coord set 3 for specular calc

dp3 oT1.x, HALF_ANGLE, S_WORLD
dp3 oT1.y, HALF_ANGLE, T_WORLD
dp3 oT1.z, HALF_ANGLE, R_WORLD_NORMAL
mov oT1.w, c[ CV_ONE ]

mov oT2, c[CV_ZERO]

mov oT0, V_TEX0

mov oD0, c[CV_LIGHT1_DIFFUSE_PP]
