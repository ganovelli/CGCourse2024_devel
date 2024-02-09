#version 460 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 1) in vec3 aColor; 

out vec3 vColor;

uniform mat4 uP;
uniform mat4 uV;
uniform mat4 uT;
uniform mat4 uM;

void main(void) 
{ 
	vColor = aColor;
    gl_Position = uP*uV*uT*uM*vec4(aPosition, 1.0); 
}