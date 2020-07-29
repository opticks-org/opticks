#version 450
// Backward compatible to #version 130, 330, ...
// source file: GrayscaleDisplay.cg

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : require
#endif

uniform sampler2DRect inputImage;
out vec4 imageColor;
void main()
{
    

    imageColor = texture2DRect(inputImage, gl_FragCoord.xy);
    imageColor.r = dot(imageColor.rgb, vec3(0.299, 0.587, 0.114));
    imageColor.g = imageColor.x;
    imageColor.b = imageColor.x;
    
} 
