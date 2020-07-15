#version 130

uniform sampler2DRect inputImage;
uniform sampler2DRect colorMap;
uniform float numColors;
uniform float dataMax;
uniform float lowerValue;
uniform float upperValue;
uniform float alphaValue;
uniform float maxBound;
uniform float minBound;

void main()
{
    float x;
    vec4 outputColor;
    vec4 imageColor;

    imageColor = texture2DRect(inputImage, gl_TexCoord[0].xy);
    x = max(0.0, min(1.0, (imageColor.x - lowerValue/dataMax)/((upperValue - lowerValue)/dataMax)));

    outputColor.x = texture2DRect(colorMap, vec2(x*numColors,0.5) ).x;
    outputColor.y = texture2DRect(colorMap, vec2(x*numColors,0.5) ).y;
    outputColor.z = texture2DRect(colorMap, vec2(x*numColors,0.5) ).z;
    outputColor.w = (alphaValue/255.0)*texture2DRect(colorMap, vec2(x*numColors,0.5) ).w;
    
	gl_FragColor = outputColor;
} 