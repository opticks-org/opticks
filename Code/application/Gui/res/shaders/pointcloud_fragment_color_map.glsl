#version 150 // OpenGL 3.2
#extension GL_ARB_explicit_attrib_location : enable // need this extension or version 3.3

in float ratio;
uniform sampler2D colorMap;

uniform int stretchType; // 0=linear, 1=log, 2=exponential, 3=equalization

layout(location=0) out vec4 fragColor;

void main()
{
   vec2 texCoord;
   texCoord.x = 0.5;
   switch (stretchType)
   {
   case 0: // linear
   default: // unknown or unsupported, fall back to linear
      texCoord.y = ratio;
      break;
   case 1: // log
      texCoord.y = (2+log(ratio))/2;
      break;
   case 2: // exp
      texCoord.y = pow(10, 2*ratio+2) / 10000;
      break;
   }
   fragColor = texture2D(colorMap, texCoord);
}