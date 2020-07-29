#version 450
// Backward compatible to #version 130, 330, ...

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
out vec4 outputColor;

 // main procedure, the original name was main
void main()
{

    vec3 value;
    float d;
    float m;
    
    value = texture2DRect(inputImage, gl_FragCoord.xy).xyz;
    cZh0004 = gl_FragCoord.xy + vec2( 1.00000000E+000, 0.00000000E+000);
    m = value.x + texture2DRect(inputImage, cZh0004).x;
    cZh0006 = gl_FragCoord.xy + vec2( 1.00000000E+000, 1.00000000E+000);
    m = m + texture2DRect(inputImage, cZh0006).x;
    cZh0008 = gl_FragCoord.xy + vec2( 0.00000000E+000, 1.00000000E+000);
    m = m + texture2DRect(inputImage, cZh0008).x;
    m = m/4.00000000E+000;
    d = value.x - m;
    d = d*5.00000000E-001;
    xZh0010 = m + d;
    bZh0014 = min(9.21568632E-001, xZh0010);
    ZDtemp9 = max(6.27450943E-002, bZh0014);
    outputColor.x = ZDtemp9;
    bZh0020 = min(vec2( 9.37254906E-001, 9.37254906E-001), value.yz);
    ZDtemp15 = max(vec2( 6.27450943E-002, 6.27450943E-002), bZh0020);
    outputColor.yz = ZDtemp15;
    ret_0 = outputColor;
    
    // Set the alpha to 1.0 so the color is not transparent
    outputColor.a = 1.0;
    
} // main end