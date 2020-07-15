// glslf output by Cg compiler
// cgc version 2.0.0016, build date Jun  5 2008
// command line args: -profile glslf
// source file: bloom.cg
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
vec2 cZh0003;

 // main procedure, the original name was main
void main()
{

    vec4 Color;

    cZh0003 = gl_TexCoord[0].xy + vec2( -5.99999987E-002, 0.00000000E+000);
    Color = texture2DRect(inputImage, cZh0003)*2.21600011E-003;
    cZh0003 = gl_TexCoord[0].xy + vec2( -5.00000007E-002, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*8.76399968E-003;
    cZh0003 = gl_TexCoord[0].xy + vec2( -3.99999991E-002, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*2.69949995E-002;
    cZh0003 = gl_TexCoord[0].xy + vec2( -2.99999993E-002, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*1.20985001E-001;
    cZh0003 = gl_TexCoord[0].xy + vec2( -1.99999996E-002, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*1.76033005E-001;
    cZh0003 = gl_TexCoord[0].xy + vec2( -9.99999978E-003, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*1.99470997E-001;
    Color = Color + texture2DRect(inputImage, gl_TexCoord[0].xy)*1.76033005E-001;
    cZh0003 = gl_TexCoord[0].xy + vec2( 9.99999978E-003, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*1.20985001E-001;
    cZh0003 = gl_TexCoord[0].xy + vec2( 1.99999996E-002, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*6.47590011E-002;
    cZh0003 = gl_TexCoord[0].xy + vec2( 2.99999993E-002, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*2.69949995E-002;
    cZh0003 = gl_TexCoord[0].xy + vec2( 3.99999991E-002, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*8.76399968E-003;
    cZh0003 = gl_TexCoord[0].xy + vec2( 5.00000007E-002, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*8.76399968E-003;
    cZh0003 = gl_TexCoord[0].xy + vec2( 5.99999987E-002, 0.00000000E+000);
    Color = Color + texture2DRect(inputImage, cZh0003)*1.20985001E-001;
    ret_0 = Color*1.50000000E+000;
    
    // Set the alpha to 1.0 so the color is not transparent 
    ret_0.a = 1.0;
    
    gl_FragColor = ret_0;
    return;
} // main end