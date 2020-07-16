#version 130
// This is a mask filter.  

uniform sampler2DRect inputImage;


void main()
{
    vec4 imageColor;

    imageColor = texture2DRect(inputImage, gl_TexCoord[0].xy);
    
    if(imageColor.x > 0.1)
    {
    	// Mask out the pixel with the color black
    	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
    	// Make the pixel clear
    	gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
} 