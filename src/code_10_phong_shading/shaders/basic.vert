#version 460 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 2) in vec3 aNormal; 
 
out vec3 vLDirVS;
out vec3 vPosVS;
out vec3 vNormalVS;
out vec3 vColor;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec3 uLDir;
uniform vec3 uColor;

void main(void) 
{ 
	vColor    = uColor;
	vLDirVS   =  (uView*vec4(uLDir,0.f)).xyz; 
	vNormalVS =  (uView*uModel*vec4(aNormal, 1.0)).xyz; 
	vPosVS = (uView*uModel*vec4(aPosition, 1.0)).xyz; 
    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0); 
}