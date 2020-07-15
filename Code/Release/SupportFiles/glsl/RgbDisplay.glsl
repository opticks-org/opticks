#version 130
// source file: RgbDisplay.cg

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : require
#endif

uniform sampler2DRect inputImage;
uniform float dataMax;
uniform float redDataMax;
uniform float greenDataMax;
uniform float blueDataMax;
uniform float redLowerValue;
uniform float redUpperValue;
uniform float greenLowerValue;
uniform float greenUpperValue;
uniform float blueLowerValue;
uniform float blueUpperValue;
uniform float alphaValue;
uniform float minBound;
uniform float maxBound;

void main()
{
    vec4 outputColor;
    vec4 imageColor;
    vec4 modifiedColorZh0013;

    imageColor = texture2DRect(inputImage, gl_TexCoord[0].xy);
    modifiedColorZh0013.x = (imageColor.x - redLowerValue/redDataMax)/((redUpperValue - redLowerValue)/redDataMax);
    modifiedColorZh0013.y = (imageColor.y - greenLowerValue/greenDataMax)/((greenUpperValue - greenLowerValue)/greenDataMax);
    modifiedColorZh0013.z = (imageColor.z - blueLowerValue/blueDataMax)/((blueUpperValue - blueLowerValue)/blueDataMax);

    if(imageColor.x < (minBound/dataMax) || imageColor.x > (maxBound/dataMax)){
		outputColor.x = 0;
		outputColor.y = 0;
		outputColor.z = 0;
		outputColor.w = 0;
	}else{
       outputColor.x = max(0.0, min(1.0, modifiedColorZh0013.x));
       outputColor.y = max(0.0, min(1.0, modifiedColorZh0013.y));
       outputColor.z = max(0.0, min(1.0, modifiedColorZh0013.z));
       outputColor.w = (alphaValue/255.0)*imageColor.w;
	}
    gl_FragColor = outputColor;
}
