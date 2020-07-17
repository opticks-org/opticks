#version 450
// Backward compatible to #version 130, 330, ...

uniform sampler2DRect inputImage;
uniform sampler2DRect estimateImage;

out vec4 outputColor;

void main()
{

    vec4 filteredColor;

    filteredColor = texture2DRect(inputImage, gl_FragCoord.xy) - texture2DRect(estimateImage, gl_FragCoord.xy);

    //filteredColor = texture(estimateImage, gl_FragCoord.xy);  // Testing just show background estimate    
	//filteredColor = texture(inputImage, gl_FragCoord.xy);  // Testing just show input image
	
    // Set the alpha to 1.0 so the color is not transparent
    filteredColor.a = 1.0;
    
    outputColor = filteredColor;
} 