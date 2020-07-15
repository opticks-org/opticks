#version 450
// Backward compatible to #version 130, 330, ...

uniform sampler2DRect inputImage;
uniform sampler2DRect filteredImage;
uniform float coefficient;

out vec4 outputColor;

void main()
{

    vec4 estimateColor;
    vec4 frameColor;
    vec4 filteredColor;

    frameColor = texture2DRect(inputImage, gl_FragCoord.xy);
    filteredColor = texture2DRect(filteredImage, gl_FragCoord.xy);
    estimateColor = filteredColor*(coefficient - 1.0) + frameColor;

	// Likely less efficient due to more vector math (Three vector compared to two vector and one scalar)
	//estimateColor = frameColor - filteredColor + filteredColor*coefficient;

    // Set the alpha to 1.0 so the color is not transparent
    estimateColor.a = 1.0;
    
    outputColor = estimateColor;
} 