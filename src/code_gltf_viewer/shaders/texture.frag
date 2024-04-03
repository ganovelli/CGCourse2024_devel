#version 410 core  
out vec4 color; 

in vec2 vTexCoord;
in vec3 vLdirVS;
in vec3 vLdirTS;
in vec3 vVdirTS;
in vec3 vNormalTS;

uniform int uRenderMode;
uniform int uUseColorImage;
uniform int uUseOcclusionMap;
uniform int uUseNormalMap;
uniform int uUseMask;

uniform vec3 uDiffuseColor;
uniform float uAlphaCutoff;

uniform sampler2D uColorImage;
uniform sampler2D uBumpmapImage;
uniform sampler2D uNormalmapImage;
uniform sampler2D uOcclusionmapImage;



/* Diffuse */
float diffuse( vec3 L, vec3 N){
	return  max(0.0,dot(L,N));
}

// this produce the Hue for v:0..1 (for debug purposes)
vec3 hsv2rgb(float  v)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(v,v,v) + K.xyz) * 6.0 - K.www);
    return   mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0),1.0);
}

void main(void) 
{ 
	if(uUseColorImage==1) 
		color =  vec4(diffuse(normalize(vLdirTS),normalize(vNormalTS))* texture2D(uColorImage,vTexCoord.xy).xyz,1.0);
		 
	if(uUseNormalMap==1) // normal mapping
	{
		vec3 N =  texture2D(uNormalmapImage,vTexCoord.xy).xyz ;
 		N = normalize(N*2.0-1.0);
 		color *=  vec4(vec3(diffuse(normalize(vLdirTS),N)),1.0);
	}

	if(uUseOcclusionMap == 1)
		color*= vec4(texture2D(uOcclusionmapImage,vTexCoord.xy).xxx,1.0);

	if(uUseMask==1){
	  color = texture2D(uColorImage,vTexCoord.xy);
	  color = (color.w>uAlphaCutoff)?color:vec4(vec3(0),0.f);
	  color.x = 1.0;
	}
		
} 