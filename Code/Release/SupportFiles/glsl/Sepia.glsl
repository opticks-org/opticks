#version 130

// source file: Sepia.cg

#ifdef GL_ARB_texture_rectangle
#extension GL_ARB_texture_rectangle : require
#endif

uniform sampler2DRect inputImage;


void main()
{
    vec4 imageColor;

    imageColor = texture2DRect(inputImage, gl_TexCoord[0].xy);
    imageColor.r = dot(imageColor.rgb, vec3(0.299, 0.587, 0.114));
    gl_FragColor = vec4(imageColor.r * vec3(1.2, 1.0, 0.8), imageColor.w);
} 

