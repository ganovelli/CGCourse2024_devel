#version 460 core  
out vec4 color; 
in vec3 vColor;
in vec3 vPos;
in  vec3 vNormal;

uniform vec3 uLDir;
uniform vec3 uColor;

void main(void) 
{    
   // this part uses concept we haven't covered yet. Please ignore it
   vec3 N = normalize(cross(dFdx(vPos),dFdy(vPos)));
   vec3 L0 = normalize(uLDir);
 
   float contrib = max(0.f,dot(N,L0));

   color = vec4(uColor*contrib, 1.0); 

} 