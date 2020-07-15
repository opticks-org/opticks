#version 130

uniform sampler2DRect inputImage;
uniform sampler2DRect estimateImage;

 // main procedure, the original name was main
void main()
{

    vec4 filteredColor;

    filteredColor = texture2DRect(inputImage, gl_TexCoord[0].xy) - texture2DRect(estimateImage, gl_TexCoord[0].xy);

    //filteredColor = texture(estimateImage, gl_TexCoord[0].xy);  // Testing just show background estimate
    
	//filteredColor = texture(inputImage, gl_TexCoord[0].xy);  // Testing just show input image
	
    // Set the alpha to 1.0 so the color is not transparent
    filteredColor.a = 1.0;
    
    gl_FragColor = filteredColor;
} 