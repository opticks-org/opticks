#version 450
// Backward compatible to #version 130, 330, ...
// This is a mask filter.  

uniform sampler2DRect inputImage;

out vec4 outputColor;

void main()
{
    vec4 imageColor;

    imageColor = texture2DRect(inputImage, gl_FragCoord.xy);
    
    if(imageColor.x > 0.1)
    {
    	// Mask out the pixel with the color black
    	outputColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
    	// Make the pixel clear
    	outputColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
} 