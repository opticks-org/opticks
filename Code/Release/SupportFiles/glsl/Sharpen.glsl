// glslf output by Cg compiler
// cgc version 2.0.0016, build date Jun  5 2008
// command line args: -profile glslf
// source file: Sharpen.cg
//vendor NVIDIA Corporation
//version 2.0.0.16
//profile glslf
//program main
//semantic main.inputImage
//var samplerRECT inputImage :  : inputImage : 1 : 1
//var float2 coord : $vin.TEXCOORD0 : TEXCOORD0 : 0 : 1
//var float4 main : $vout.COLOR : COLOR : -1 : 1

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : require
#endif

vec4 ret_0;
uniform sampler2DRect inputImage;
vec2 cZh0004;
vec2 cZh0006;
vec2 cZh0008;
float ZDtemp9;
float xZh0010;
float bZh0014;
vec2 ZDtemp15;
vec2 bZh0020;

 // main procedure, the original name was main
void main()
{

    vec3 value;
    float d;
    float m;
    vec4 output;

    value = texture2DRect(inputImage, gl_TexCoord[0].xy).xyz;
    cZh0004 = gl_TexCoord[0].xy + vec2( 1.00000000E+000, 0.00000000E+000);
    m = value.x + texture2DRect(inputImage, cZh0004).x;
    cZh0006 = gl_TexCoord[0].xy + vec2( 1.00000000E+000, 1.00000000E+000);
    m = m + texture2DRect(inputImage, cZh0006).x;
    cZh0008 = gl_TexCoord[0].xy + vec2( 0.00000000E+000, 1.00000000E+000);
    m = m + texture2DRect(inputImage, cZh0008).x;
    m = m/4.00000000E+000;
    d = value.x - m;
    d = d*5.00000000E-001;
    xZh0010 = m + d;
    bZh0014 = min(9.21568632E-001, xZh0010);
    ZDtemp9 = max(6.27450943E-002, bZh0014);
    output.x = ZDtemp9;
    bZh0020 = min(vec2( 9.37254906E-001, 9.37254906E-001), value.yz);
    ZDtemp15 = max(vec2( 6.27450943E-002, 6.27450943E-002), bZh0020);
    output.yz = ZDtemp15;
    ret_0 = output;
    
    // Set the alpha to 1.0 so the color is not transparent
    output.a = 1.0;
    
    gl_FragColor = output;
    return;
} // main end