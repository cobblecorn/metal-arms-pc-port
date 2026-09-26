;Basic Vertex Lighting: 1 Directional + 1 Point Light

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

#define R_EYE_NORMAL r1
#define R_EYE_VERTEX r0
#define R_HALF_VECTOR r9
#define R_TEMP r7
#define R_SPECULAR r6

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

; *************** Directional Light 1 ************************
; This is the intensity of the specular component
; Calculate half vector (light vector + constant eye vector)
mov R_HALF_VECTOR, c[CV_LIGHT1_DIRECTION_PP]
add R_HALF_VECTOR, R_HALF_VECTOR, c[CV_EYE_VECTOR]

; normalize half-vector
dp3 R_HALF_VECTOR.w, R_HALF_VECTOR, R_HALF_VECTOR
rsq R_HALF_VECTOR.w, R_HALF_VECTOR.w												
mul R_HALF_VECTOR, R_HALF_VECTOR, R_HALF_VECTOR.w	

; Dot normal with half-vector.  
dp3 r3.x, R_EYE_NORMAL, c[CV_LIGHT1_DIRECTION_PP]
dp3 r3.yz, R_EYE_NORMAL, R_HALF_VECTOR
;mov r3.x, c[CV_ONE]
mov r3.w, c[CV_MATPOWER]

;mov r4.x, r3.y
lit r4, r3

; add the (diffuse color * diffuse light color * diffuse intensity(r3.x))
mul R_SPECULAR, c[CV_LIGHT1_DIFFUSE_PP], r4.z

; Clamp it just to be safe.
min R_SPECULAR, c[CV_ONE], R_SPECULAR
max R_SPECULAR, c[CV_ZERO], R_SPECULAR

mov oD0, R_SPECULAR
mov oT0, V_TEX0