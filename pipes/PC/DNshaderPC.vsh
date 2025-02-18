;
; DNVertexShader for GTA_PC;
;
; 21/02/2005 - Andrzej: - initial;
;
;
;
;
; Registers:
; v0       	- position
; v1      	- normal (if exists)
; v2     	- tex coords0
; v3        - day color (diffuse)
; v4		- night color (specular)
;
; c0->c3	- the transformation matrix
; c4[x,y]	- the interp value between day and night colours
;
;
;
vs_1_1

;
; binding declarations:
;
dcl_position		v0	// v0 is position
;dcl_normal			v1	// v1 is normal
dcl_texcoord0		v2	// v2 is texture coordinates0

dcl_color0			v4	// v3 is day color
dcl_color1			v3	// v4 is night color

;
; constants:
;
def		c10, 1, 0, 0, 1


;
;-------------------------------------------------------------------------------------------------
;
;

; transform the position by the transformation matrix held in c0->c3
m4x4	oPos, v0, c0


; write the texture coordinates
mov		oT0.xy, v2


; calculate final diffuse colour
mul		r1, v3, c4.x		; day color * (1-interp)
mul		r2, v4, c4.y		; night color * (interp)

;mov	r0, c10
;mul	r2, r0, c4.y		; night color * (interp)


add		oD0.xyzw, r1.xyzw, r2.xyzw


;
; the end
;


;
;const DWORD dwDNVertexShaderPC_vs11[] =
;
;
