#version 130

uniform sampler2DRect inputImage;


void main()
{
   gl_FragColor = texture(inputImage, gl_TexCoord[0].xy);
}