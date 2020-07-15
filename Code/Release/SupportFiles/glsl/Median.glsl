// glslf output by Cg compiler
// cgc version 2.0.0016, build date Jun  5 2008
// command line args: -profile glslf
// source file: Median.cg
//vendor NVIDIA Corporation
//version 2.0.0.16
//profile glslf
//program main
//semantic main.inputImage
//var samplerRECT inputImage :  : inputImage : 1 : 1
//var float2 pos : $vin.TEXCOORD0 : TEXCOORD0 : 0 : 1
//var float4 main : $vout.COLOR : COLOR : -1 : 1

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : require
#endif

vec4 ret_0;
uniform sampler2DRect inputImage;
vec2 cZh0004;
vec2 cZh0006;
vec2 cZh0008;
vec2 cZh0010;
vec2 cZh0014;
vec2 cZh0016;
vec2 cZh0018;
vec2 cZh0020;
vec4 minZh0021;
vec4 bZh0021;
vec4 maxZh0021;

 // main procedure, the original name was main
void main()
{

    vec4 c[9];

    cZh0004 = gl_TexCoord[0].xy + vec2( -2.00000000E+000, -2.00000000E+000);
    c[0] = texture2DRect(inputImage, cZh0004);
    cZh0006 = gl_TexCoord[0].xy + vec2( 0.00000000E+000, -2.00000000E+000);
    c[1] = texture2DRect(inputImage, cZh0006);
    cZh0008 = gl_TexCoord[0].xy + vec2( 2.00000000E+000, -2.00000000E+000);
    c[2] = texture2DRect(inputImage, cZh0008);
    cZh0010 = gl_TexCoord[0].xy + vec2( -2.00000000E+000, 0.00000000E+000);
    c[3] = texture2DRect(inputImage, cZh0010);
    c[4] = texture2DRect(inputImage, gl_TexCoord[0].xy);
    cZh0014 = gl_TexCoord[0].xy + vec2( 2.00000000E+000, 0.00000000E+000);
    c[5] = texture2DRect(inputImage, cZh0014);
    cZh0016 = gl_TexCoord[0].xy + vec2( -2.00000000E+000, 2.00000000E+000);
    c[6] = texture2DRect(inputImage, cZh0016);
    cZh0018 = gl_TexCoord[0].xy + vec2( 0.00000000E+000, 2.00000000E+000);
    c[7] = texture2DRect(inputImage, cZh0018);
    cZh0020 = gl_TexCoord[0].xy + vec2( 2.00000000E+000, 2.00000000E+000);
    c[8] = texture2DRect(inputImage, cZh0020);
    minZh0021 = vec4(c[0].x < c[1].x ? c[0].x : c[1].x, c[0].y < c[1].y ? c[0].y : c[1].y, c[0].z < c[1].z ? c[0].z : c[1].z, c[0].w < c[1].w ? c[0].w : c[1].w);
    maxZh0021 = vec4(c[0].x >= c[1].x ? c[0].x : c[1].x, c[0].y >= c[1].y ? c[0].y : c[1].y, c[0].z >= c[1].z ? c[0].z : c[1].z, c[0].w >= c[1].w ? c[0].w : c[1].w);
    c[0] = minZh0021;
    minZh0021 = vec4(maxZh0021.x < c[2].x ? maxZh0021.x : c[2].x, maxZh0021.y < c[2].y ? maxZh0021.y : c[2].y, maxZh0021.z < c[2].z ? maxZh0021.z : c[2].z, maxZh0021.w < c[2].w ? maxZh0021.w : c[2].w);
    maxZh0021 = vec4(maxZh0021.x >= c[2].x ? maxZh0021.x : c[2].x, maxZh0021.y >= c[2].y ? maxZh0021.y : c[2].y, maxZh0021.z >= c[2].z ? maxZh0021.z : c[2].z, maxZh0021.w >= c[2].w ? maxZh0021.w : c[2].w);
    c[2] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[0].x < minZh0021.x ? c[0].x : minZh0021.x, c[0].y < minZh0021.y ? c[0].y : minZh0021.y, c[0].z < minZh0021.z ? c[0].z : minZh0021.z, c[0].w < minZh0021.w ? c[0].w : minZh0021.w);
    maxZh0021 = vec4(c[0].x >= bZh0021.x ? c[0].x : bZh0021.x, c[0].y >= bZh0021.y ? c[0].y : bZh0021.y, c[0].z >= bZh0021.z ? c[0].z : bZh0021.z, c[0].w >= bZh0021.w ? c[0].w : bZh0021.w);
    c[0] = minZh0021;
    c[1] = maxZh0021;
    minZh0021 = vec4(c[2].x < c[3].x ? c[2].x : c[3].x, c[2].y < c[3].y ? c[2].y : c[3].y, c[2].z < c[3].z ? c[2].z : c[3].z, c[2].w < c[3].w ? c[2].w : c[3].w);
    maxZh0021 = vec4(c[2].x >= c[3].x ? c[2].x : c[3].x, c[2].y >= c[3].y ? c[2].y : c[3].y, c[2].z >= c[3].z ? c[2].z : c[3].z, c[2].w >= c[3].w ? c[2].w : c[3].w);
    c[3] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[1].x < minZh0021.x ? c[1].x : minZh0021.x, c[1].y < minZh0021.y ? c[1].y : minZh0021.y, c[1].z < minZh0021.z ? c[1].z : minZh0021.z, c[1].w < minZh0021.w ? c[1].w : minZh0021.w);
    maxZh0021 = vec4(c[1].x >= bZh0021.x ? c[1].x : bZh0021.x, c[1].y >= bZh0021.y ? c[1].y : bZh0021.y, c[1].z >= bZh0021.z ? c[1].z : bZh0021.z, c[1].w >= bZh0021.w ? c[1].w : bZh0021.w);
    c[2] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[0].x < minZh0021.x ? c[0].x : minZh0021.x, c[0].y < minZh0021.y ? c[0].y : minZh0021.y, c[0].z < minZh0021.z ? c[0].z : minZh0021.z, c[0].w < minZh0021.w ? c[0].w : minZh0021.w);
    maxZh0021 = vec4(c[0].x >= bZh0021.x ? c[0].x : bZh0021.x, c[0].y >= bZh0021.y ? c[0].y : bZh0021.y, c[0].z >= bZh0021.z ? c[0].z : bZh0021.z, c[0].w >= bZh0021.w ? c[0].w : bZh0021.w);
    c[0] = minZh0021;
    c[1] = maxZh0021;
    minZh0021 = vec4(c[3].x < c[4].x ? c[3].x : c[4].x, c[3].y < c[4].y ? c[3].y : c[4].y, c[3].z < c[4].z ? c[3].z : c[4].z, c[3].w < c[4].w ? c[3].w : c[4].w);
    maxZh0021 = vec4(c[3].x >= c[4].x ? c[3].x : c[4].x, c[3].y >= c[4].y ? c[3].y : c[4].y, c[3].z >= c[4].z ? c[3].z : c[4].z, c[3].w >= c[4].w ? c[3].w : c[4].w);
    c[4] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[2].x < minZh0021.x ? c[2].x : minZh0021.x, c[2].y < minZh0021.y ? c[2].y : minZh0021.y, c[2].z < minZh0021.z ? c[2].z : minZh0021.z, c[2].w < minZh0021.w ? c[2].w : minZh0021.w);
    maxZh0021 = vec4(c[2].x >= bZh0021.x ? c[2].x : bZh0021.x, c[2].y >= bZh0021.y ? c[2].y : bZh0021.y, c[2].z >= bZh0021.z ? c[2].z : bZh0021.z, c[2].w >= bZh0021.w ? c[2].w : bZh0021.w);
    c[3] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[1].x < minZh0021.x ? c[1].x : minZh0021.x, c[1].y < minZh0021.y ? c[1].y : minZh0021.y, c[1].z < minZh0021.z ? c[1].z : minZh0021.z, c[1].w < minZh0021.w ? c[1].w : minZh0021.w);
    maxZh0021 = vec4(c[1].x >= bZh0021.x ? c[1].x : bZh0021.x, c[1].y >= bZh0021.y ? c[1].y : bZh0021.y, c[1].z >= bZh0021.z ? c[1].z : bZh0021.z, c[1].w >= bZh0021.w ? c[1].w : bZh0021.w);
    c[2] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[0].x < minZh0021.x ? c[0].x : minZh0021.x, c[0].y < minZh0021.y ? c[0].y : minZh0021.y, c[0].z < minZh0021.z ? c[0].z : minZh0021.z, c[0].w < minZh0021.w ? c[0].w : minZh0021.w);
    maxZh0021 = vec4(c[0].x >= bZh0021.x ? c[0].x : bZh0021.x, c[0].y >= bZh0021.y ? c[0].y : bZh0021.y, c[0].z >= bZh0021.z ? c[0].z : bZh0021.z, c[0].w >= bZh0021.w ? c[0].w : bZh0021.w);
    c[0] = minZh0021;
    c[1] = maxZh0021;
    minZh0021 = vec4(c[4].x < c[5].x ? c[4].x : c[5].x, c[4].y < c[5].y ? c[4].y : c[5].y, c[4].z < c[5].z ? c[4].z : c[5].z, c[4].w < c[5].w ? c[4].w : c[5].w);
    maxZh0021 = vec4(c[4].x >= c[5].x ? c[4].x : c[5].x, c[4].y >= c[5].y ? c[4].y : c[5].y, c[4].z >= c[5].z ? c[4].z : c[5].z, c[4].w >= c[5].w ? c[4].w : c[5].w);
    c[5] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[3].x < minZh0021.x ? c[3].x : minZh0021.x, c[3].y < minZh0021.y ? c[3].y : minZh0021.y, c[3].z < minZh0021.z ? c[3].z : minZh0021.z, c[3].w < minZh0021.w ? c[3].w : minZh0021.w);
    maxZh0021 = vec4(c[3].x >= bZh0021.x ? c[3].x : bZh0021.x, c[3].y >= bZh0021.y ? c[3].y : bZh0021.y, c[3].z >= bZh0021.z ? c[3].z : bZh0021.z, c[3].w >= bZh0021.w ? c[3].w : bZh0021.w);
    c[4] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[2].x < minZh0021.x ? c[2].x : minZh0021.x, c[2].y < minZh0021.y ? c[2].y : minZh0021.y, c[2].z < minZh0021.z ? c[2].z : minZh0021.z, c[2].w < minZh0021.w ? c[2].w : minZh0021.w);
    maxZh0021 = vec4(c[2].x >= bZh0021.x ? c[2].x : bZh0021.x, c[2].y >= bZh0021.y ? c[2].y : bZh0021.y, c[2].z >= bZh0021.z ? c[2].z : bZh0021.z, c[2].w >= bZh0021.w ? c[2].w : bZh0021.w);
    c[3] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[1].x < minZh0021.x ? c[1].x : minZh0021.x, c[1].y < minZh0021.y ? c[1].y : minZh0021.y, c[1].z < minZh0021.z ? c[1].z : minZh0021.z, c[1].w < minZh0021.w ? c[1].w : minZh0021.w);
    maxZh0021 = vec4(c[1].x >= bZh0021.x ? c[1].x : bZh0021.x, c[1].y >= bZh0021.y ? c[1].y : bZh0021.y, c[1].z >= bZh0021.z ? c[1].z : bZh0021.z, c[1].w >= bZh0021.w ? c[1].w : bZh0021.w);
    c[2] = maxZh0021;
    maxZh0021 = vec4(c[0].x >= minZh0021.x ? c[0].x : minZh0021.x, c[0].y >= minZh0021.y ? c[0].y : minZh0021.y, c[0].z >= minZh0021.z ? c[0].z : minZh0021.z, c[0].w >= minZh0021.w ? c[0].w : minZh0021.w);
    c[1] = maxZh0021;
    minZh0021 = vec4(c[5].x < c[6].x ? c[5].x : c[6].x, c[5].y < c[6].y ? c[5].y : c[6].y, c[5].z < c[6].z ? c[5].z : c[6].z, c[5].w < c[6].w ? c[5].w : c[6].w);
    maxZh0021 = vec4(c[5].x >= c[6].x ? c[5].x : c[6].x, c[5].y >= c[6].y ? c[5].y : c[6].y, c[5].z >= c[6].z ? c[5].z : c[6].z, c[5].w >= c[6].w ? c[5].w : c[6].w);
    c[6] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[4].x < minZh0021.x ? c[4].x : minZh0021.x, c[4].y < minZh0021.y ? c[4].y : minZh0021.y, c[4].z < minZh0021.z ? c[4].z : minZh0021.z, c[4].w < minZh0021.w ? c[4].w : minZh0021.w);
    maxZh0021 = vec4(c[4].x >= bZh0021.x ? c[4].x : bZh0021.x, c[4].y >= bZh0021.y ? c[4].y : bZh0021.y, c[4].z >= bZh0021.z ? c[4].z : bZh0021.z, c[4].w >= bZh0021.w ? c[4].w : bZh0021.w);
    c[5] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[3].x < minZh0021.x ? c[3].x : minZh0021.x, c[3].y < minZh0021.y ? c[3].y : minZh0021.y, c[3].z < minZh0021.z ? c[3].z : minZh0021.z, c[3].w < minZh0021.w ? c[3].w : minZh0021.w);
    maxZh0021 = vec4(c[3].x >= bZh0021.x ? c[3].x : bZh0021.x, c[3].y >= bZh0021.y ? c[3].y : bZh0021.y, c[3].z >= bZh0021.z ? c[3].z : bZh0021.z, c[3].w >= bZh0021.w ? c[3].w : bZh0021.w);
    c[4] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[2].x < minZh0021.x ? c[2].x : minZh0021.x, c[2].y < minZh0021.y ? c[2].y : minZh0021.y, c[2].z < minZh0021.z ? c[2].z : minZh0021.z, c[2].w < minZh0021.w ? c[2].w : minZh0021.w);
    maxZh0021 = vec4(c[2].x >= bZh0021.x ? c[2].x : bZh0021.x, c[2].y >= bZh0021.y ? c[2].y : bZh0021.y, c[2].z >= bZh0021.z ? c[2].z : bZh0021.z, c[2].w >= bZh0021.w ? c[2].w : bZh0021.w);
    c[3] = maxZh0021;
    maxZh0021 = vec4(c[1].x >= minZh0021.x ? c[1].x : minZh0021.x, c[1].y >= minZh0021.y ? c[1].y : minZh0021.y, c[1].z >= minZh0021.z ? c[1].z : minZh0021.z, c[1].w >= minZh0021.w ? c[1].w : minZh0021.w);
    c[2] = maxZh0021;
    minZh0021 = vec4(c[6].x < c[7].x ? c[6].x : c[7].x, c[6].y < c[7].y ? c[6].y : c[7].y, c[6].z < c[7].z ? c[6].z : c[7].z, c[6].w < c[7].w ? c[6].w : c[7].w);
    maxZh0021 = vec4(c[6].x >= c[7].x ? c[6].x : c[7].x, c[6].y >= c[7].y ? c[6].y : c[7].y, c[6].z >= c[7].z ? c[6].z : c[7].z, c[6].w >= c[7].w ? c[6].w : c[7].w);
    c[7] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[5].x < minZh0021.x ? c[5].x : minZh0021.x, c[5].y < minZh0021.y ? c[5].y : minZh0021.y, c[5].z < minZh0021.z ? c[5].z : minZh0021.z, c[5].w < minZh0021.w ? c[5].w : minZh0021.w);
    maxZh0021 = vec4(c[5].x >= bZh0021.x ? c[5].x : bZh0021.x, c[5].y >= bZh0021.y ? c[5].y : bZh0021.y, c[5].z >= bZh0021.z ? c[5].z : bZh0021.z, c[5].w >= bZh0021.w ? c[5].w : bZh0021.w);
    c[6] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[4].x < minZh0021.x ? c[4].x : minZh0021.x, c[4].y < minZh0021.y ? c[4].y : minZh0021.y, c[4].z < minZh0021.z ? c[4].z : minZh0021.z, c[4].w < minZh0021.w ? c[4].w : minZh0021.w);
    maxZh0021 = vec4(c[4].x >= bZh0021.x ? c[4].x : bZh0021.x, c[4].y >= bZh0021.y ? c[4].y : bZh0021.y, c[4].z >= bZh0021.z ? c[4].z : bZh0021.z, c[4].w >= bZh0021.w ? c[4].w : bZh0021.w);
    c[5] = maxZh0021;
    bZh0021 = minZh0021;
    minZh0021 = vec4(c[3].x < minZh0021.x ? c[3].x : minZh0021.x, c[3].y < minZh0021.y ? c[3].y : minZh0021.y, c[3].z < minZh0021.z ? c[3].z : minZh0021.z, c[3].w < minZh0021.w ? c[3].w : minZh0021.w);
    maxZh0021 = vec4(c[3].x >= bZh0021.x ? c[3].x : bZh0021.x, c[3].y >= bZh0021.y ? c[3].y : bZh0021.y, c[3].z >= bZh0021.z ? c[3].z : bZh0021.z, c[3].w >= bZh0021.w ? c[3].w : bZh0021.w);
    c[4] = maxZh0021;
    maxZh0021 = vec4(c[2].x >= minZh0021.x ? c[2].x : minZh0021.x, c[2].y >= minZh0021.y ? c[2].y : minZh0021.y, c[2].z >= minZh0021.z ? c[2].z : minZh0021.z, c[2].w >= minZh0021.w ? c[2].w : minZh0021.w);
    minZh0021 = vec4(c[7].x < c[8].x ? c[7].x : c[8].x, c[7].y < c[8].y ? c[7].y : c[8].y, c[7].z < c[8].z ? c[7].z : c[8].z, c[7].w < c[8].w ? c[7].w : c[8].w);
    minZh0021 = vec4(c[6].x < minZh0021.x ? c[6].x : minZh0021.x, c[6].y < minZh0021.y ? c[6].y : minZh0021.y, c[6].z < minZh0021.z ? c[6].z : minZh0021.z, c[6].w < minZh0021.w ? c[6].w : minZh0021.w);
    minZh0021 = vec4(c[5].x < minZh0021.x ? c[5].x : minZh0021.x, c[5].y < minZh0021.y ? c[5].y : minZh0021.y, c[5].z < minZh0021.z ? c[5].z : minZh0021.z, c[5].w < minZh0021.w ? c[5].w : minZh0021.w);
    minZh0021 = vec4(c[4].x < minZh0021.x ? c[4].x : minZh0021.x, c[4].y < minZh0021.y ? c[4].y : minZh0021.y, c[4].z < minZh0021.z ? c[4].z : minZh0021.z, c[4].w < minZh0021.w ? c[4].w : minZh0021.w);
    maxZh0021 = vec4(maxZh0021.x >= minZh0021.x ? maxZh0021.x : minZh0021.x, maxZh0021.y >= minZh0021.y ? maxZh0021.y : minZh0021.y, maxZh0021.z >= minZh0021.z ? maxZh0021.z : minZh0021.z, maxZh0021.w >= minZh0021.w ? maxZh0021.w : minZh0021.w);
    ret_0 = maxZh0021;
    
    // Set the alpha to 1.0 so the color is not transparent
    maxZh0021.a = 1.0;
    
    gl_FragColor = maxZh0021;
    return;
} // main end