// glslf output by Cg compiler
// cgc version 2.0.0016, build date Jun  5 2008
// command line args: -profile glslf
// source file: GrayscaleDisplay.cg
//vendor NVIDIA Corporation
//version 2.0.0.16
//profile glslf
//program main
//semantic main.inputImage
//semantic main.dataMax
//semantic main.lowerValue
//semantic main.upperValue
//semantic main.alpha
//var samplerRECT inputImage :  : inputImage : 2 : 1
//var float dataMax :  : dataMax : 3 : 1
//var float lowerValue :  : lowerValue : 4 : 1
//var float upperValue :  : upperValue : 5 : 1
//var float alpha :  : alpha : 6 : 1
//var float2 texCoord : $vin.TEXCOORD0 : TEXCOORD0 : 0 : 1
//var float4 outputColor : $vout.COLOR : COLOR : 1 : 1

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : require
#endif

uniform sampler2DRect inputImage;
uniform float dataMax;
uniform float lowerValue;
uniform float upperValue;
uniform float alpha;
vec4 modifiedColorZh0007;
float ZDtemp8;
float bZh0013;

 // main procedure, the original name was main
void main()
{

    vec4 outputColor;
    vec4 imageColor;

    imageColor = texture2DRect(inputImage, gl_TexCoord[0].xy);
    modifiedColorZh0007.x = (imageColor.x - lowerValue/dataMax)/((upperValue - lowerValue)/dataMax);
    bZh0013 = min(1.00000000E+000, modifiedColorZh0007.x);
    ZDtemp8 = max(0.00000000E+000, bZh0013);
    outputColor.x = ZDtemp8;
    outputColor.y = ZDtemp8;
    outputColor.z = ZDtemp8;
    
    // Set the alpha to 1.0 so the color is not transparent
    outputColor.w = 1.0;
    
    gl_FragColor = outputColor;
} // main end