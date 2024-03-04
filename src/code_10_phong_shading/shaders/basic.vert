#version 460 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 2) in vec3 aNormal; 
 
out vec3 vPos;
out vec3 vNormal;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

void main(void) 
{ 
	vNormal = (uView*uModel*vec4(aNormal, 1.0)).xyz; 
	vPos = (uView*uModel*vec4(aPosition, 1.0)).xyz; 
    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0); 
}