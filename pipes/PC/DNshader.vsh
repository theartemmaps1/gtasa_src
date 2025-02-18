; Registers:
; v0        - position
; v1        - normal (this contains the night colour)
; v2        - diffuse color (this contains the day colour)
; v3        - texture coords 
; c0->c3	- the transformation matrix
; c4		- the interp value between day and night colours
; c5		- 1 - the interp value between day and night colours

; specify that this is a basic XBox vertex shader
xvs.1.1     ; Version instruction

; Transform the position by the transformation matrix held in c0->c3
m4x4 oPos, v0, c0

;trick from the xds.graphics newsgroup - copy the output pos.w to fog
mov oFog.x, r12.w                                 

; interpolate between the day and night colours
; day is the diffuse colour, night is stored in the normal
mul r1, v1, c4.x; mul night colour by interp value
mul r2, v2, c5.x; mul day colour by 1-interp value

; Write the texture coordinates
mov oT0.xy, v3

; calculate the final colour
add oD0.xyz, r1.xyz, r2.xyz


