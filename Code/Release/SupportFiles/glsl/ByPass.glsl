#version 450
// Backward compatible to #version 130, 330, ...
uniform sampler2DRect inputImage;
out vec4 outputColor;

void main()
{
   outputColor = texture(inputImage, gl_FragCoord.xy);
}