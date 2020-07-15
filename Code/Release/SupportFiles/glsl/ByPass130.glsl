#version 130

uniform sampler2DRect inputImage;

void main()
{
   vec4 magnitude = texture2DRect(inputImage, gl_TexCoord[0].xy);   
      
   // Set the alpha to 1.0 so the color is not transparent
   magnitude.a = 1.0;
   
   gl_FragColor = magnitude;
} 