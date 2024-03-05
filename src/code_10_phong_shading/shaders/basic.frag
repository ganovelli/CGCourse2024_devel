#version 460 core  
out vec4 color; 
in vec3 vColor;
in vec3 vPosVS;
in vec3 vNormalVS;
in vec3 vLDirVS;

uniform vec3 uLDir;
uniform vec3 uColor;

uniform int	 uShadingMode;
uniform vec3 uDiffuseColor;
uniform vec3 uAmbientColor;
uniform vec3 uSpecularColor;
uniform vec3 uEmissiveColor;
uniform vec3 uLightColor;
uniform float uShininess;

/* phong */
vec3 phong ( vec3 L, vec3 V, vec3 N){
	float LN = max(0.0,dot(L,N));
	vec3 R = -L+2*dot(L,N)*N;

	float spec = ((LN>0.f)?1.f:0.f) * max(0.0,pow(dot(V,R),uShininess));

	return (uAmbientColor+LN*uDiffuseColor + spec * uSpecularColor)*uLightColor;
}

void main(void) 
{    
	if(uShadingMode == 1){
		vec3 N = normalize(cross(dFdx(vPosVS),dFdy(vPosVS)));
		color = vec4(phong(vLDirVS,normalize(-vPosVS),N),1.0);
	}
 	else
	if(uShadingMode == 2){
		color = vec4(vColor,1.0);
	}
 	else
	if(uShadingMode == 3){
		color = vec4(phong(vLDirVS,normalize(-vPosVS),normalize(vNormalVS)),1.0);
	}
	else
	color = vec4(normalize(vNormalVS)*0.5+0.5,1.0);
//	color = vec4(1,1,1,1.0);
	 
//	color = vec4(1,1,1,1);

   // this part uses concept we haven't covered yet. Please ignore it
//   vec3 N = normalize(cross(dFdx(vPos),dFdy(vPos)));
//   vec3 L0 = normalize(uLDir);
 
//   float contrib = max(0.f,dot(N,L0));

//   color = vec4(uColor*contrib, 1.0); 

} 