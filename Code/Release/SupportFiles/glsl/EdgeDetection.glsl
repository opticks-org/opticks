#version 130

uniform sampler2DRect inputImage;

void main()
{
   vec4 gx = -1.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2(-1, -1)) +
               -2.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2(-1,  0)) +
               -1.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2(-1,  1)) +
                1.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2( 1, -1)) +
                2.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2( 1,  0)) +
                1.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2( 1,  1));
      
   vec4 gy = -1.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2(-1, -1)) +
               -2.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2( 0, -1)) +
               -1.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2( 1, -1)) +
                1.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2(-1,  1)) +
                2.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2( 0,  1)) +
                1.0 * texture2DRect(inputImage, gl_TexCoord[0].xy + vec2( 1,  1));
				
   vec4 magnitude = sqrt(gx * gx + gy * gy);
   
   // Set the alpha to 1.0 so the color is not transparent
   magnitude.a = 1.0;
   
   gl_FragColor = magnitude;
} 