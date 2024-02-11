#version 460 core 
layout (location = 0) in vec2 aPosition; 
layout (location = 1) in vec3 aColor; 

out vec3 vColor; 
void main(void) 
{ 
    gl_Position = vec4(aPosition,0.f, 1.0); 
    vColor = aColor; 
}