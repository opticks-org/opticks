vec4 ret_0;
uniform sampler2DRect inputImage;

void main()
{
   ret_0 = texture2DRect(inputImage, gl_TexCoord[0].xy);
   ret_0.x = 1.0-ret_0.x;
   ret_0.y = 1.0-ret_0.y;
   gl_FragColor = ret_0;
}