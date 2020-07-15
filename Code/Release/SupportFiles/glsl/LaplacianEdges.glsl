// glslf output by Cg compiler
// cgc version 2.0.0016, build date Jun  5 2008
// command line args: -profile glslf
// source file: LaplacianEdges.cg
//vendor NVIDIA Corporation
//version 2.0.0.16
//profile glslf
//program main
//semantic main.inputImage
//var samplerRECT inputImage :  : inputImage : 1 : 1
//var float2 coords : $vin.TEXCOORD0 : TEXCOORD0 : 0 : 1
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
vec2 cZh0012;
vec2 cZh0014;
vec2 cZh0016;
vec2 cZh0018;

 // main procedure, the original name was main
void main()
{

    vec4 c;
    vec4 bl;
    vec4 l;
    vec4 tl;
    vec4 t;
    vec4 tr;
    vec4 r;
    vec4 br;
    vec4 b;

    c = texture2DRect(inputImage, gl_TexCoord[0].xy);
    cZh0004 = gl_TexCoord[0].xy + vec2( -5.00000000E-001, -5.00000000E-001);
    bl = texture2DRect(inputImage, cZh0004);
    cZh0006 = gl_TexCoord[0].xy + vec2( -5.00000000E-001, 0.00000000E+000);
    l = texture2DRect(inputImage, cZh0006);
    cZh0008 = gl_TexCoord[0].xy + vec2( -5.00000000E-001, 5.00000000E-001);
    tl = texture2DRect(inputImage, cZh0008);
    cZh0010 = gl_TexCoord[0].xy + vec2( 0.00000000E+000, 5.00000000E-001);
    t = texture2DRect(inputImage, cZh0010);
    cZh0012 = gl_TexCoord[0].xy + vec2( 5.00000000E-001, 5.00000000E-001);
    tr = texture2DRect(inputImage, cZh0012);
    cZh0014 = gl_TexCoord[0].xy + vec2( 5.00000000E-001, 0.00000000E+000);
    r = texture2DRect(inputImage, cZh0014);
    cZh0016 = gl_TexCoord[0].xy + vec2( 5.00000000E-001, -5.00000000E-001);
    br = texture2DRect(inputImage, cZh0016);
    cZh0018 = gl_TexCoord[0].xy + vec2( 0.00000000E+000, -5.00000000E-001);
    b = texture2DRect(inputImage, cZh0018);
    ret_0 = 3.20000000E+001*(c + -1.25000000E-001*(bl + l + tl + t + tr + r + br + b));
    
    // Set the alpha to 1.0 so the color is not transparent
    ret_0.a = 1.0;
    
    gl_FragColor = ret_0;
    return;
} // main end