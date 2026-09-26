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
#define R_VERTEX_TO_LIGHT r9
#define R_TEMP r7
#define R_DIFFUSE r6
#define R_WORLD_VERTEX r5

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

dp4 R_WORLD_VERTEX.y, V_POS, c[CV_WORLD_1]
mov R_WORLD_VERTEX.z, R_EYE_VERTEX.z

; Transform normal to eye space
dp3 R_EYE_NORMAL.x, v1, c[CV_WORLDVIEW_0]
dp3 R_EYE_NORMAL.y, v1, c[CV_WORLDVIEW_1]
dp3 R_EYE_NORMAL.z, v1, c[CV_WORLDVIEW_2]

; Normalize transformed normal
dp3 R_EYE_NORMAL.w, R_EYE_NORMAL, R_EYE_NORMAL
rsq R_EYE_NORMAL.w, R_EYE_NORMAL.w	
mul R_EYE_NORMAL, R_EYE_NORMAL, R_EYE_NORMAL.w

mov R_DIFFUSE, c[CV_MAT_AMBIENT]
add R_DIFFUSE, R_DIFFUSE, V_COLOR
;mad R_DIFFUSE, V_COLOR, c[CV_HALF], R_DIFFUSE

; *************** Directional Light 1 ************************
dp3 r3, R_EYE_NORMAL, c[CV_LIGHT1_DIRECTION]
max r3, c[CV_ZERO], r3

; add the (diffuse color * diffuse light color * diffuse intensity(r3.x))
mul R_TEMP, c[CV_MAT_DIFFUSE], r3
mad R_DIFFUSE, c[CV_LIGHT1_DIFFUSE], R_TEMP, R_DIFFUSE

; *************** Point Light 1 ************************
; Calculate vector from vertex to light in eye space
add R_VERTEX_TO_LIGHT, c[CV_LIGHT2_POSITION],-R_EYE_VERTEX
dp3 R_TEMP.w, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT	
rsq R_VERTEX_TO_LIGHT.w, R_TEMP.w

; Get the attenuation
dst R_TEMP, R_TEMP.wwww, R_VERTEX_TO_LIGHT.wwww		; (1, d, d*d, 1/d)
dp3 R_TEMP.w, R_TEMP, c[CV_LIGHT2_ATTENUATION]		; (a0 + a1*d + a2*d2)
rcp r8.w, R_TEMP.w									; 1 / (a0 + a1*d + a2*d)	

; Normalize the vertex to the light vector
mul R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT.w

; Dot normal with light vector (from vertex to light)
; This is the intensity of the diffuse component
dp3 r4, R_EYE_NORMAL, R_VERTEX_TO_LIGHT
max r4, c[CV_ZERO], r4

; Scale the factors by the attenuation
mul r4, r4, r8.w

;Spot light attenuation.
dp3 R_TEMP, R_VERTEX_TO_LIGHT, -c[CV_LIGHTMTX2_0]
mad R_TEMP, R_TEMP, c[CV_LIGHTMTX2_1].z, c[CV_LIGHTMTX2_1].y
min R_TEMP, c[CV_ONE], R_TEMP
max R_TEMP, c[CV_ZERO], R_TEMP
mul r4, r4, R_TEMP

; add the (diffuse color * diffuse light color * diffuse intensity(r4.x))
mul R_TEMP, c[CV_MAT_DIFFUSE], r4
;max R_TEMP, c[CV_ZERO], R_TEMP

mad R_DIFFUSE, c[CV_LIGHT2_DIFFUSE], R_TEMP, R_DIFFUSE

; *************** Point Light 2 ************************
; Calculate vector from vertex to light in eye space
add R_VERTEX_TO_LIGHT, c[CV_LIGHT3_POSITION],-R_EYE_VERTEX
dp3 R_TEMP.w, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT	
rsq R_VERTEX_TO_LIGHT.w, R_TEMP.w

; Get the attenuation
dst R_TEMP, R_TEMP.wwww, R_VERTEX_TO_LIGHT.wwww		; (1, d, d*d, 1/d)
dp3 R_TEMP.w, R_TEMP, c[CV_LIGHT3_ATTENUATION]		; (a0 + a1*d + a2*d2)
rcp r8.w, R_TEMP.w									; 1 / (a0 + a1*d + a2*d)	

; Normalize the vertex to the light vector
mul R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT.w

; Dot normal with light vector (from vertex to light)
; This is the intensity of the diffuse component
dp3 r4, R_EYE_NORMAL, R_VERTEX_TO_LIGHT
max r4, c[CV_ZERO], r4

; Scale the factors by the attenuation
mul r4, r4, r8.w

; add the (ambient color * ambient light color)
;mov R_DIFFUSE, c[CV_LIGHT3_AMBIENT]
;Spot light attenuation.
dp3 R_TEMP, R_VERTEX_TO_LIGHT, -c[CV_LIGHTMTX2_2]
mad R_TEMP, R_TEMP, c[CV_LIGHTMTX2_3].z, c[CV_LIGHTMTX2_3].y
max R_TEMP, c[CV_ZERO], R_TEMP
min R_TEMP, c[CV_ONE], R_TEMP
mul r4, r4, R_TEMP

; add the (diffuse color * diffuse light color * diffuse intensity(r4.x))
mul R_TEMP, c[CV_MAT_DIFFUSE], r4
;max R_TEMP, c[CV_ZERO], R_TEMP

mad R_DIFFUSE, c[CV_LIGHT3_DIFFUSE], R_TEMP, R_DIFFUSE

; *************** Point Light 3 ************************
; Calculate vector from vertex to light in eye space
add R_VERTEX_TO_LIGHT, c[CV_LIGHT4_POSITION],-R_EYE_VERTEX
dp3 R_TEMP.w, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT	
rsq R_VERTEX_TO_LIGHT.w, R_TEMP.w

; Get the attenuation
dst R_TEMP, R_TEMP.wwww, R_VERTEX_TO_LIGHT.wwww		; (1, d, d*d, 1/d)
dp3 R_TEMP.w, R_TEMP, c[CV_LIGHT4_ATTENUATION]		; (a0 + a1*d + a2*d2)
rcp r8.w, R_TEMP.w									; 1 / (a0 + a1*d + a2*d)	

; Normalize the vertex to the light vector
mul R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT.w

; Dot normal with light vector (from vertex to light)
; This is the intensity of the diffuse component
dp3 r4, R_EYE_NORMAL, R_VERTEX_TO_LIGHT
max r4, c[CV_ZERO], r4

; Scale the factors by the attenuation
mul r4, r4, r8.w

;Spot light attenuation.
dp3 R_TEMP, R_VERTEX_TO_LIGHT, -c[CV_LIGHTMTX3_0]
mad R_TEMP, R_TEMP, c[CV_LIGHTMTX3_1].z, c[CV_LIGHTMTX3_1].y
max R_TEMP, c[CV_ZERO], R_TEMP
min R_TEMP, c[CV_ONE], R_TEMP
mul r4, r4, R_TEMP

; add the (diffuse color * diffuse light color * diffuse intensity(r4.x))
mul R_TEMP, c[CV_MAT_DIFFUSE], r4
;max R_TEMP, c[CV_ZERO], R_TEMP

mad R_DIFFUSE, c[CV_LIGHT4_DIFFUSE], R_TEMP, R_DIFFUSE

; *************** Point Light 4 ************************
; Calculate vector from vertex to light in eye space
add R_VERTEX_TO_LIGHT, c[CV_LIGHT5_POSITION],-R_EYE_VERTEX
dp3 R_TEMP.w, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT	
rsq R_VERTEX_TO_LIGHT.w, R_TEMP.w

; Get the attenuation
dst R_TEMP, R_TEMP.wwww, R_VERTEX_TO_LIGHT.wwww		; (1, d, d*d, 1/d)
dp3 R_TEMP.w, R_TEMP, c[CV_LIGHT5_ATTENUATION]		; (a0 + a1*d + a2*d2)
rcp r8.w, R_TEMP.w									; 1 / (a0 + a1*d + a2*d)	

; Normalize the vertex to the light vector
mul R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT, R_VERTEX_TO_LIGHT.w

; Dot normal with light vector (from vertex to light)
; This is the intensity of the diffuse component
dp3 r4, R_EYE_NORMAL, R_VERTEX_TO_LIGHT
max r4, c[CV_ZERO], r4

; Scale the factors by the attenuation
mul r4, r4, r8.w

;Spot light attenuation.
dp3 R_TEMP, R_VERTEX_TO_LIGHT, -c[CV_LIGHTMTX3_2]
mad R_TEMP, R_TEMP, c[CV_LIGHTMTX3_3].z, c[CV_LIGHTMTX3_3].y
max R_TEMP, c[CV_ZERO], R_TEMP
min R_TEMP, c[CV_ONE], R_TEMP
mul r4, r4, R_TEMP

; add the (diffuse color * diffuse light color * diffuse intensity(r4.x))
mul R_TEMP, c[CV_MAT_DIFFUSE], r4
;max R_TEMP, c[CV_ZERO], R_TEMP

mad R_DIFFUSE, c[CV_LIGHT5_DIFFUSE], R_TEMP, R_DIFFUSE

; Clamp it just to be safe.
min R_DIFFUSE, c[CV_ONE], R_DIFFUSE
max R_DIFFUSE, c[CV_ZERO], R_DIFFUSE

mov oD0.xyz, R_DIFFUSE
mov oD0.w, V_COLOR.w

mov r0.xy, V_TEX0
mov r0.z, c[CV_ONE]

dp3 r1.x, r0, c[CV_TEXMTX0_0]
dp3 r1.y, r0, c[CV_TEXMTX0_1]

mov oT0.xy, r1
mov oT1.xy, r1
mov oT2.xyz, c[CV_MAT_EMISSIVE]

mad r2.z, R_WORLD_VERTEX.z, c[CV_FOG_DEPTH].x, c[CV_FOG_DEPTH].y
mad r2.w, R_WORLD_VERTEX.y, c[CV_FOG_DEPTH].z, c[CV_FOG_DEPTH].w
min r2.zw, r2, c[CV_ONE]
max r2.zw, r2, c[CV_FOG_MIN]
mad oT2.w, r2.z, r2.w, c[CV_EPS].w
