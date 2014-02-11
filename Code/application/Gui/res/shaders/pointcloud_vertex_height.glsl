#version 150 compatibility // OpenGL 3.2 required for "compatibility"

in vec3 vertex;

uniform float lower;
uniform float upper;
uniform float scaleFactor;
uniform float zExaggeration;

out float ratio;

void main()
{
   float scaledZ = vertex.z * scaleFactor;
   gl_Position = gl_ModelViewProjectionMatrix * vec4(vertex.x * scaleFactor, vertex.y * scaleFactor, scaledZ * zExaggeration, 1.0);
   ratio = smoothstep(lower, upper, vertex.z);
}