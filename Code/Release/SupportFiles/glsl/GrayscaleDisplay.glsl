#version 130
// source file: GrayscaleDisplay.cg

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : require
#endif

// These inputs need to match the parameters in GrayscaleGpuProgram::initialize()

uniform sampler2DRect inputImage;
uniform float dataMax;
uniform float lowerValue;
uniform float upperValue;
uniform float alphaValue;
uniform float minBound;
uniform float maxBound;


void main()
{
    float x; 
    vec4 outputColor;
    vec4 imageColor;

    imageColor = texture2DRect(inputImage, gl_TexCoord[0].xy);
    x = max(0.0, min(1.0, (imageColor.x - lowerValue/dataMax)/((upperValue - lowerValue)/dataMax)));
  
  
  if(imageColor.x < (minBound/dataMax) || imageColor.x > (maxBound/dataMax)){
		outputColor.x = 0;
		outputColor.y = 0;
		outputColor.z = 0;
		outputColor.w = 0;
	}else{
    	outputColor.x = x;
    	outputColor.y = x;
    	outputColor.z = x;
    	outputColor.w = (alphaValue/255.0)*imageColor.w;
    }
    gl_FragColor = outputColor;
} 
