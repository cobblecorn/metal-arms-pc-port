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
#define V_TEX1 v4
#define V_TEX2 v5
#define V_TEX3 v6

#define R_EYE_NORMAL r6
#define R_EYE_VERTEX r7
#define R_VERTEX_TO_LIGHT r9

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

; ************** Per-Pixel Point 2 ***************
mad oT2.xy, c[CV_LIGHT2_ATTENUATION].ww, R_EYE_VERTEX.xz, c[CV_LIGHT2_ATTENUATION].xz
mad oT3.x, c[CV_LIGHT2_ATTENUATION].w, R_EYE_VERTEX.y, c[CV_LIGHT2_ATTENUATION].y
mov oT3.y, c[CV_HALF]

sub r0, c[CV_LIGHT1_POSITION_PP], R_EYE_VERTEX
dp3 r1.w, r0, r0
rsq r0.w, r1.w
;r0 is Vtx -> Light normalized vector.
mul r0, r0, r0.w

sub r1, c[CV_LIGHT2_POSITION_PP], R_EYE_VERTEX
dp3 r2.w, r1, r1
rsq r1.w, r2.w
;r0 is Vtx -> Light normalized vector.
mul r1, r1, r1.w

; Dot normal with light vector (from vertex to light)
; This is the intensity of the diffuse component
dp3 r2, R_EYE_NORMAL, r0
dp3 r3, R_EYE_NORMAL, r1

sge r2, r2, c[CV_EPS].x
sge r3, r3, c[CV_EPS].x

mul oD0, r2, c[CV_LIGHT1_DIFFUSE_POINT_PP]
mul oD1, r3, c[CV_LIGHT2_DIFFUSE_POINT_PP]
