#include <iostream>
/*
License Agreement
---------------------------------------------------------------------------

Copyright (c) 2023, Christian Hart. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION). HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*/

#include "OpenB3D.h"
#include "stb_image.h"

const char *pbr_vert_shader=
"#version 130\r\n"

"// output variables to fragment shader\r\n"
"out vec2 Vertex_UV;"
"out vec4 Vertex_Color;"
"out vec3 Vertex_Normal;"
"out vec4 Vertex_Position;"
"out vec3 Vertex_Surface_to_Viewer_Direction;"

"// ----------------------------------------------------------------------------\r\n"

"void main()"
"{"
"	Vertex_UV = gl_MultiTexCoord0.xy;"
"	Vertex_Color = gl_Color;"
"	Vertex_Normal = normalize(gl_NormalMatrix * gl_Normal);"
"	Vertex_Position = gl_ModelViewMatrix * gl_Vertex;"
	
"	vec3 vViewModelPosition = vec3(gl_ModelViewMatrixInverse * vec4(0, 0, 0, 1.0));"
"	vec3 vLocalSurfaceToViewerDirection = normalize(vViewModelPosition-gl_Vertex.xyz);"
"	vec3 vvLocalSurfaceNormal = normalize(gl_Normal);"
	
"	Vertex_Surface_to_Viewer_Direction = normalize(reflect(vLocalSurfaceToViewerDirection, vvLocalSurfaceNormal)) ;"
	
"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
"}";


const char *pbr_frag_shader=
"#version 130\r\n"

"#define NUM_LIGHTS 8\r\n"

"// ----------------------------------------------------------------------------\r\n"

"struct FloatArray {"
"    float Float;"
"};"

"const float PI = 3.14159265359;"

"// ----------------------------------------------------------------------------\r\n"

"// Parallax Occlusion values\r\n"
"const float POscale = 0.04;"
"const float POmin = 8;"
"const float POmax = 32;"
	
"// ----------------------------------------------------------------------------\r\n"

"// from the Vertex Shader\r\n"
"in vec2 Vertex_UV;"
"in vec4 Vertex_Color;"
"in vec3 Vertex_Normal;"
"in vec4 Vertex_Position;"
"in vec3 Vertex_Surface_to_Viewer_Direction;"
	
"// ----------------------------------------------------------------------------\r\n"

"out vec4 FragColor;"

"// ----------------------------------------------------------------------------\r\n"

"// variable inputs\r\n"
"uniform float levelscale;   // mesh scales \r\n"
"uniform float gamma;        // user gamma correction \r\n"
"uniform float POmulti;      // Parallax Occlusion multiplicator \r\n"
"uniform vec2 texscale;      // texture scale (0...x) \r\n"
"uniform vec2 texoffset;     // texture offset (0...x) \r\n"
"uniform int timer;          // timing \r\n"

"// ---------------------------------------------------------------------------- \r\n"

"// textures \r\n"
"uniform sampler2D albedoMap;"
"uniform sampler2D normalMap;"
"uniform sampler2D propMap;"
"uniform sampler2D emissionMap;"
"uniform samplerCube envMap;"

"// ---------------------------------------------------------------------------- \r\n"

"// variable Flags \r\n"
"uniform int flagPB;"
"uniform int flagPM;"
"uniform int flagEN;"
"uniform int flagEM;"
"uniform int flagTM;"
"uniform int setENV;"
"uniform int isMetal;"

"// ---------------------------------------------------------------------------- \r\n"

"// texture existance flags \r\n"
"uniform int texAL;"
"uniform int texNM;"
"uniform int texPR;"
"uniform int texEM;"

"// ---------------------------------------------------------------------------- \r\n"

"// light related variables \r\n"
"uniform float A;"
"uniform float B;"
"uniform FloatArray lightradius[NUM_LIGHTS];"

"// ---------------------------------------------------------------------------- \r\n"

"mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)"
"{"
"	vec3 dp1 = dFdx(p);"
"	vec3 dp2 = dFdy(p);"
"	vec2 duv1 = dFdx(uv);"
"	vec2 duv = dFdy(uv);"
 
"	vec3 dp2perp = cross(dp2, N);"
"	vec3 dp1perp = cross(N, dp1);"
"	vec3 T = dp2perp * duv1.x + dp1perp * duv.x;"
"	vec3 B = dp2perp * duv1.y + dp1perp * duv.y;"
 
"	float invmax = inversesqrt(max(dot(T, T), dot(B, B)));"
"	return mat3(T * invmax, B * invmax, N);"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"vec3 perturb_normal(vec3 N, vec3 V, vec3 map, vec2 texcoord)"
"{"
"	map = map * 255.0 / 127.0 - 128.0 / 127.0;"
	
"	mat3 TBN = cotangent_frame(N, -V, texcoord);"
"	return normalize(TBN * map);"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"vec3 ToneMapPBR(vec3 color)"
"{"
"	// HDR tonemapping and gamma correction \r\n"
"	color = color / (color + vec3(1.0));"
"	color = pow(color, vec3(1.0 / gamma));"
	
"	return color;"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"vec3 Uncharted(vec3 x)"
"{"
"	float A = 0.15;"
"	float B = 0.50;"
"	float C = 0.10;"
"	float D = 0.20;"
"	float E = 0.02;"
"	float F = 0.30;"
	
"	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"vec3 ToneMapUncharted(vec3 color)"
"{"
"	color = Uncharted(color * 4.5) * (1.0 / Uncharted(vec3(11.2)));"
"	color = pow(color, vec3(1.0 / gamma));"
"	return color;"

"}"

"// ---------------------------------------------------------------------------- \r\n"

"vec3 ToneMapSCurve(vec3 x)"
"{"
"	float a = 2.51f;"
"	float b = 0.03f;"
"	float c = 2.43f;"
"	float d = 0.59f;"
"	float e = 0.14f;"
"	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"vec3 ToneMapFilmic(vec3 color)"
"{"
"	vec4 vh = vec4(color*0.5, gamma);"
"	vec4 va = 1.425 * vh + 0.05;"
"	vec4 vf = (vh * va + 0.004) / (vh * (va + 0.55) + 0.0491) - 0.0821;"
"	return vf.rgb / vf.www;"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"vec3 ToneMapExposure(vec3 color)"
"{"
"	color = exp(-1.0 / ( 2.72 * color + 0.15 ));"
"	color = pow(color, vec3(1. / gamma));"
"	return color;"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"float DistributionGGX(vec3 N, vec3 H, float roughness)"
"{"
"	float a = roughness * roughness;"
"	float a2 = a * a;"
"	float NdotH = max(dot(N, H), 0.0);"
"	float NdotH2 = NdotH * NdotH;"

"	float nom = a2;"
"	float denom = (NdotH2 * (a2 - 1.0) + 1.0);"
"	denom = PI * denom * denom;"

"	return nom / denom;"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"float GeometrySchlickGGX(float NdotV, float roughness)"
"{"
"	float r = (roughness + 1.0);"
"	float k = (r * r) / 8.0;"

"	float nom = NdotV;"
"	float denom = NdotV * (1.0 - k) + k;"

"	return nom / denom;"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)"
"{"
"	float NdotV = max(dot(N, V), 0.0);"
"	float NdotL = max(dot(N, L), 0.0);"
"	float ggx2 = GeometrySchlickGGX(NdotV, roughness);"
"	float ggx1 = GeometrySchlickGGX(NdotL, roughness);"

"	return ggx1 * ggx2;"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"vec3 fresnelSchlick(float cosTheta, vec3 F0)"
"{"
"	if(cosTheta > 1.0)"
"		cosTheta = 1.0;"
"	float p = pow(1.0 - cosTheta,5.0);"
"	return F0 + (1.0 - F0) * p;"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"float CalcAtt(float distance, float range, float a, float b)"
"{"
"	return 1.0 / (1.0 + a * distance + b * distance * distance);"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir)"
"{"
"    // number of depth layers \r\n"
"    float numLayers = mix(POmax, POmin, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));"
	
"    // calculate the size of each layer \r\n"
"    float layerDepth = 1.0 / numLayers;"

"    // depth of current layer \r\n"
"    float currentLayerDepth = 0.0;"

"    // the amount to shift the texture coordinates per layer (from vector P) \r\n"
"    vec2 P = viewDir.xy / viewDir.z * POscale * POmulti;"
"    vec2 deltaTexCoords = P / numLayers;"

"    // get initial values \r\n"
"    vec2  currentTexCoords     = texCoords;"
"    float currentDepthMapValue = texture(normalMap, currentTexCoords).a;"

"    while(currentLayerDepth < currentDepthMapValue)"
"    {"
"        // shift texture coordinates along direction of P \r\n"
"        currentTexCoords -= deltaTexCoords;"

"        // get depthmap value at current texture coordinates \r\n"
"        currentDepthMapValue = texture(normalMap, currentTexCoords).a;"

"        // get depth of next layer \r\n"
"        currentLayerDepth += layerDepth; \r\n"
"    }"

"    // get texture coordinates before collision (reverse operations) \r\n"
"    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;"

"    // get depth after and before collision for linear interpolation \r\n"
"    float afterDepth  = currentDepthMapValue - currentLayerDepth;"
"    float beforeDepth = texture(normalMap, prevTexCoords).a - currentLayerDepth + layerDepth;"

"    // interpolation of texture coordinates \r\n"
"    float weight = afterDepth / (afterDepth - beforeDepth);"
"    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);"

"    return finalTexCoords;"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"mat3 computeTBN(vec2 tempUv, vec3 worldPos, vec3 worldNormal)"
"{"

"    vec3 Q1  = dFdx(worldPos);"
"    vec3 Q2  = dFdy(worldPos);"
"    vec2 st1 = dFdx(tempUv);"
"    vec2 st2 = dFdy(tempUv);"

"	// normal and tangent \r\n"
"    vec3 n   = normalize(worldNormal);"
"    vec3 t  = normalize(Q1*st2.t - Q2*st1.t);"

"    // bitangent \r\n"
"    vec3 b = normalize(-Q1*st2.s + Q2*st1.s);"

"    return mat3(t, b, n);"
"}"

"// ---------------------------------------------------------------------------- \r\n"

"void main()"
"{"
"	// Texture coordinates \r\n"
"	vec2 ts=texscale;"
"	vec2 uv = Vertex_UV;"
"	uv = (uv * ts) + texoffset;"
	
"	// TBN Matrix \r\n"
"	vec3 VV = -Vertex_Position.xyz;"
"	vec3 VVN = normalize(VV);"
"	vec3 N = Vertex_Normal.xyz;"
"	vec3 VN = normalize(Vertex_Normal);"
"	mat3 TBN = computeTBN(uv.st,-VV,Vertex_Normal);"
	
"	// Parallax Occlusion Mapping \r\n"
"	if(flagPM > 0)"
"	{"
"		uv = ParallaxOcclusionMapping(uv, normalize(-VV * TBN));"
"	}	"

"	// Albedo Texture (sRGB, with gamma correction) \r\n"
"	vec4 albedo = vec4(0.5, 0.5, 0.5, 1.0);"
"	if(texAL > 0)"
"	{"
"		albedo = texture(albedoMap, uv);"
"		albedo.rgb = pow(albedo.rgb, vec3(2.2));"
"	}"

"	// Normalmap Texture \r\n"
"	vec3 nrm = Vertex_Normal;"
"	if(texNM > 0)"
"	{"
"		nrm = texture(normalMap, uv).rgb;"
"	}"
		
"	// 3. Perturbated Normals \r\n"
"	vec3 PN = N;"
"	if(texNM > 0)"
"	{"
"		PN = perturb_normal(VN, VVN, nrm, uv);"
"	}	"
	
"	// PBR Texture \r\n"
"	float ao = 1.0;"
"	float roughness = 0.5;"
"	float metallic = 0.5;"
"	if(texPR > 0)"
"	{"
"		vec3 pbr = texture(propMap, uv).rgb;"
				
"		ao = pbr.r;"
"		roughness = pbr.g;"
"		metallic = pbr.b;"
"	}"
				
"	// Emissive \r\n"
"	vec3 emission = vec3(0.0);"
"	if(texEM > 0 && flagEM > 0)"
"	{"
"		emission = texture(emissionMap, uv).rgb * (1.0+cos(timer/30.0));"
"	}"
	
"	// Ambient \r\n"
"	vec3 ambient = gl_LightModel.ambient.rgb * albedo.rgb;"

"	// PBR Lighting \r\n"
"	vec3 Lo = vec3(0.0);"
"	vec3 irradiance;"
"	vec3 diffuse=albedo.rgb;"
	
"	// Reflection \r\n"
"	vec3 NormalizedRSTVD = normalize(Vertex_Surface_to_Viewer_Direction);"
	
"	if(flagPB > 0)"
"	{"
"		// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0  \r\n"
"		// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)     \r\n"
"		vec3 F0 = vec3(0.04);"
"		F0 = mix(F0, albedo.rgb, metallic);"
		
"		for(int i = 0; i < NUM_LIGHTS; ++i) "
"		{"
"			// calculate per-light radiance \r\n"
"			vec3 L = normalize(gl_LightSource[i].position.xyz - Vertex_Position.xyz);"
"			vec3 H = normalize(VVN + L);"
			
"			float distance = length(L);"
"			float attenuation = CalcAtt(distance, lightradius[i].Float, A, B);"
"			vec3 radiance = gl_LightSource[i].diffuse.rgb * attenuation;"
			
"			// Cook-Torrance BRDF \r\n"
"			float NDF = DistributionGGX(PN, H, roughness);"
"			float G = GeometrySmith(PN, VVN, L, roughness);"
"			vec3 F = fresnelSchlick(max(dot(H, VVN), 0.0), F0);"
	           
"			// specularity \r\n"
"			vec3 nominator = NDF * G * F;"
"			float denominator = 4.0 * max(dot(PN, VVN), 0.0) * max(dot(PN, L), 0.0) + 0.001;"
"			vec3 specular = nominator / denominator;"
			
"			// kS is equal to Fresnel \r\n"
"			vec3 kS = F;"

"			// for energy conservation, the diffuse and specular light can't \r\n"
"			// be above 1.0 (unless the surface emits light); to preserve this \r\n"
"			// relationship the diffuse component (kD) should equal 1.0 - kS. \r\n"
"			vec3 kD = vec3(1.0) - kS;"

"			// multiply kD by the inverse metalness such that only non-metals  \r\n"
"			// have diffuse lighting, or a linear blend if partly metal (pure metals \r\n"
"			// have no diffuse light). \r\n"
"			kD *= 1.0-metallic;"
				
"			// Metallic Reflection \r\n"
"			if(isMetal==1 && flagEN==1 && setENV==1)"
"			{"
"				vec3 v=NormalizedRSTVD;"
"				irradiance = textureCube(envMap, vec3(-v.x,v.y,-v.z)).rgb;"
"				diffuse = (albedo.rgb * (0.0 + (irradiance * 1.0)));"
"			}"
							
"			// check backface lighting \r\n"
"			float NdotL = max(dot(PN, L), 0.0);"
			
"			// sum all together:  \r\n"
"			Lo += (kD * diffuse.rgb / PI + specular) * lightradius[i].Float * radiance * NdotL;"
"		}"
"	}"
"	// PBR off \r\n"
"	else"
"	{"
"		for(int i = 0; i < NUM_LIGHTS; ++i) "
"		{"
"			vec3 L = normalize(gl_LightSource[i].position.xyz - Vertex_Position.xyz);"
"			//vec3 N = Vertex_Normal.xyz; \r\n"

"			float distance = length(L);"
"			float attenuation = CalcAtt(distance, lightradius[i].Float, A, B);"
"			vec3 radiance = gl_LightSource[i].diffuse.rgb * attenuation;		"
			
"			float NdotL = max(dot(N, L), 0.0);"
"			Lo += (diffuse.rgb / PI) * lightradius[i].Float * radiance * NdotL;"
"		}"
"	}"
	
"	// mix final lighting with ambient \r\n"
"	vec3 color=(Lo+ambient);"
		
"	// Ambient Occlusion \r\n"
"	color *= ao;"
	
"	// Tonemapping \r\n"
"	if(flagTM == 1){color = ToneMapPBR(color);}"
"	if(flagTM == 2){color = ToneMapExposure(color);}"
"	if(flagTM == 3){color = ToneMapSCurve(color);}"
"	if(flagTM == 4){color = ToneMapUncharted(color);}"
"	if(flagTM == 5){color = ToneMapFilmic(color);}"
	
"	// Final olor plus Emissive with Alpha \r\n"
"	FragColor = vec4(color+emission, albedo.a);"
	
"}";

struct PBRTexture {
	Texture* basecolor;	//Base Color: RGBA, RGB=color and A=Transparency
	Texture* normals;	//Normals (DirectX): RGBA, RGB=XYZ and A=Bumpmap
	Texture* PBR;		/*PBR Combotexture: RGB
		                 Channel Red = Ambient Occlusion
				 Channel Green = Roughness
				 Channel Blue = Metallic*/
	Texture* EMI;		//Emissive: RGB
};

Texture* ENV=0;			//Evironment cubemap

extern "C" void AmbientCubeTexture(Texture* tex){
	ENV=tex;
}

extern "C" PBRTexture* CreatePBRTexture(int width, int height, unsigned char* BaseColor, unsigned char* Normals, unsigned char* Heights, unsigned char* Roughness, unsigned char* Metallic, unsigned char* AO, unsigned char* Emissive){
	PBRTexture* tex=new PBRTexture;

	tex->basecolor=CreateTexture(width,height,2,0);
	BufferToTex(tex->basecolor,BaseColor,0);

	unsigned char *src1, *src2, *src3, *dst;

	unsigned char* normals_buffer=new unsigned char[width*height*4];
	src1=Normals; src2= Heights; dst=normals_buffer;
	for (int y=0; y<height; y++){
		for (int x=0; x<width; x++){
			if (src1!=0){
				*dst=*src1; dst++; src1++;	//Normal X
				*dst=255-*src1; dst++; src1++;	//Normal Y, OpenGL convention
				*dst=*src1; dst++; src1++;	//Normal Z
			}else{
				*dst=0; dst++;	//Normal X
				*dst=0; dst++;	//Normal Y
				*dst=0; dst++;	//Normal Z
			}
			if (src2!=0){
				*dst=*src2; dst++; src2++;	//Height map
			}else{
				*dst=0; dst++;	//Height map
			}
		}
	}
	tex->normals=CreateTexture(width,height,2,0);
	BufferToTex(tex->normals,normals_buffer,0);


	unsigned char* PBR_buffer=new unsigned char[width*height*4];
	src1=AO; src2= Roughness; src3= Metallic; dst=PBR_buffer;
	for (int y=0; y<height; y++){
		for (int x=0; x<width; x++){
			if (src1!=0){
				*dst=*src1; dst++; src1++;	//Ambient Occlusion
			}else{
				*dst=0; dst++;	//Ambient Occlusion
			}
			if (src2!=0){
				*dst=*src2; dst++; src2++;	//Roughness
			}else{
				*dst=0; dst++;	//Roughness
			}
			if (src3!=0){
				*dst=*src3; dst++; src3++;	//Metallic
			}else{
				*dst=0; dst++;	//Metallic
			}
			*dst=255; dst++;		//unused
		}
	}
	tex->PBR=CreateTexture(width,height,2,0);
	BufferToTex(tex->PBR,PBR_buffer,0);

	tex->EMI=CreateTexture(width,height,2,0);
	if (Emissive!=0){
		BufferToTex(tex->EMI,Emissive,0);
	}

	return tex;
}

extern "C" PBRTexture* LoadPBRTexture(char* BaseColor, char* Normals, char* Heights,  char* Roughness, char* Metallic, char* AO, char* Emissive){
	int width, height, w, h;

	unsigned char* color_buffer;
	if (BaseColor!=0){
		color_buffer=stbi_load(BaseColor ,&width,&height,0,4);
	}else{
		return 0;
	}
	
	unsigned char* normal_buffer;
	if (Normals!=0){
		normal_buffer=stbi_load(Normals ,&w,&h,0,3);
	}else{
		normal_buffer=0;
	}

	unsigned char* height_buffer;
	if (Heights!=0){
		height_buffer=stbi_load(Heights ,&w,&h,0,1);
	}else{
		height_buffer=0;
	}

	unsigned char* roughness_buffer;
	if (Roughness!=0){
		roughness_buffer=stbi_load(Roughness ,&w,&h,0,1);
	}else{
		roughness_buffer=0;
	}

	unsigned char* metallic_buffer;
	if (Metallic!=0){
		metallic_buffer=stbi_load(Metallic ,&w,&h,0,1);
	}else{
		metallic_buffer=0;
	}

	unsigned char* ao_buffer;
	if (AO!=0){
		ao_buffer=stbi_load(AO ,&w,&h,0,1);
	}else{
		ao_buffer=0;
	}

	unsigned char* emissive_buffer;
	if (Emissive!=0){
		emissive_buffer=stbi_load(Emissive ,&w,&h,0,4);
	}else{
		emissive_buffer=0;
	}

	PBRTexture* tex = CreatePBRTexture(width, height, color_buffer, normal_buffer, height_buffer, roughness_buffer, metallic_buffer, ao_buffer, emissive_buffer);

	if (color_buffer!=0) delete color_buffer;
	if (normal_buffer!=0) delete normal_buffer;
	if (height_buffer!=0) delete height_buffer;
	if (roughness_buffer!=0) delete roughness_buffer;
	if (metallic_buffer!=0) delete metallic_buffer;
	if (ao_buffer!=0) delete ao_buffer;
	if (emissive_buffer!=0) delete emissive_buffer;

	return tex;

}
extern "C" void EntityPBRTexture(Entity* ent, PBRTexture* tex){
	EntityTexture (ent, tex->basecolor,0,0);
	EntityTexture (ent, tex->normals,0,1);
	EntityTexture (ent, tex->PBR,0,2);
	EntityTexture (ent, tex->EMI,0,3);
	if (ENV!=0){
		EntityTexture (ent, ENV,0,4);
	}
}

extern "C" Shader* CreatePBR(PBRTexture* tex){
	Shader* pbr=CreateShader((char*)"PBR", (char*)pbr_vert_shader, (char*)pbr_frag_shader);

	if (tex!=0){
		if (tex->basecolor!=0){
			ShaderTexture(pbr,tex->basecolor, (char*)"albedoMap",0);
			SetInteger(pbr, (char*)"texAL",1);
		}else{
			SetInteger(pbr, (char*)"texAL",0);
		}
		if (tex->normals!=0){
			ShaderTexture(pbr,tex->normals, (char*)"normalMap",1);
			SetInteger(pbr, (char*)"texNM",1);
		}else{
			SetInteger(pbr, (char*)"texNM",0);
		}
		if (tex->PBR!=0){
			ShaderTexture(pbr,tex->PBR, (char*)"propMap",2);
			SetInteger(pbr, (char*)"texPR",1);
		}else{
			SetInteger(pbr, (char*)"texPR",0);
		}
		if (tex->EMI!=0){
			ShaderTexture(pbr,tex->EMI, (char*)"emissionMap",3);
			SetInteger(pbr, (char*)"texEM",1);
		}else{
			SetInteger(pbr, (char*)"texEM",0);
		}
		if (ENV!=0){
			ShaderTexture(pbr, ENV, (char*)"envMap",4);
			SetInteger(pbr, (char*)"setENV",1);
		}else{
			SetInteger(pbr, (char*)"envMap",4);
			SetInteger(pbr, (char*)"setENV",0);
		}
	}else{
		SetInteger(pbr, (char*)"albedoMap",0);
		SetInteger(pbr, (char*)"texAL",1);
		SetInteger(pbr, (char*)"normalMap",1);
		SetInteger(pbr, (char*)"texNM",1);
		SetInteger(pbr, (char*)"propMap",2);
		SetInteger(pbr, (char*)"texPR",1);
		SetInteger(pbr, (char*)"emissionMap",3);
		SetInteger(pbr, (char*)"texEM",1);
		SetInteger(pbr, (char*)"envMap",4);
	}

	SetFloat2(pbr, (char*)"texscale", 1,1);
	SetFloat2(pbr, (char*)"texoffset", 0,0);
	SetFloat(pbr, (char*)"levelscale", 128);
	SetFloat(pbr, (char*)"POmulti", 1);
	SetFloat(pbr, (char*)"gamma", 1);
	return pbr;
}