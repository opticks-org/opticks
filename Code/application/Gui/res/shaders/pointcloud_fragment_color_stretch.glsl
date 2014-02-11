#version 150 // OpenGL 3.2
#extension GL_ARB_explicit_attrib_location : enable // need this extension or version 3.3

in float ratio;
uniform vec4 color1;
uniform vec4 color2;

uniform int stretchType; // 0=linear, 1=log, 2=exponential, 3=equalization

layout(location=0) out vec4 fragColor; // requires version 330

void main()
{
   float tmp;
   switch (stretchType)
   {
   case 0: // linear
   default: // unknown or unsupported, fall back to linear
      tmp = ratio;
      break;
   case 1: // log
      tmp = (2+log(ratio))/2;
      break;
   case 2: // exp
      tmp = pow(10, 2*ratio+2) / 10000;
      break;
   }
   fragColor = mix(color1, color2, tmp);
}