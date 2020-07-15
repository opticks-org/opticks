#version 130
//#version 140
//#version 150
//#version 330

uniform sampler2DRect inputImage;
uniform sampler2DRect filteredImage;
uniform float coefficient;

void main()
{

    vec4 estimateColor;
    vec4 frameColor;
    vec4 filteredColor;
	float co = 0.02;

    frameColor = texture2DRect(inputImage, gl_TexCoord[0].xy);
    filteredColor = texture2DRect(filteredImage, gl_TexCoord[0].xy);
    //estimateColor = filteredColor*(coefficient - 1.0) + frameColor;
	estimateColor = filteredColor*(co - 1.0) + frameColor;

	// Likely less efficient due to more vector math (Three vector compared to two vector and one scalar)
	//estimateColor = frameColor - filteredColor + filteredColor*coefficient;

    // Set the alpha to 1.0 so the color is not transparent
    estimateColor.a = 1.0;
    
    gl_FragColor = estimateColor;
}