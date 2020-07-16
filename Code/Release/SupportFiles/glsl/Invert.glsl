#version 450
// Backward compatible to #version 130, 330, ...


uniform sampler2DRect inputImage;
out vec4 ret_0;

void main()
{
   ret_0 = texture2DRect(inputImage, gl_FragCoord.xy);
   ret_0.x = 1.0-ret_0.x;
   ret_0.y = 1.0-ret_0.y;
}