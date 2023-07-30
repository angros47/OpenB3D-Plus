#include "OpenB3D.h"

const char *glass_vert_shader=
"#version 100\r\n"

"attribute highp vec3 position, normal;"
"uniform highp mat4 projection, view, world;"

"uniform highp float mRefractionRatio;"
"uniform highp float mFresnelBias;"
"uniform highp float mFresnelScale;"
"uniform highp float mFresnelPower;"

"varying highp vec3 vReflect;"
"varying highp vec3 vRefract[3];"
"varying lowp float vReflectionFactor;"

"void main(void) {"
"	gl_Position = projection * view * world * vec4(position, 1.0);"
"	highp vec4 worldPosition = view * world*1.3 * vec4( position, 1.0 );"
"	vec3 worldNormal = normalize(  view * world* vec4( normal, 0.0 )).xyz;"

"	vReflect = reflect( worldPosition.xyz, worldNormal );"
"	vRefract[0] = refract( normalize( worldPosition.xyz ), worldNormal, mRefractionRatio*0.986 );"
"	vRefract[1] = refract( normalize( worldPosition.xyz ), worldNormal, mRefractionRatio*0.987 );"
"	vRefract[2] = refract( normalize( worldPosition.xyz ), worldNormal, mRefractionRatio*0.988 );"
"	vReflectionFactor = mFresnelBias + mFresnelScale * pow( 1.0 + dot( normalize( worldPosition.xyz ), worldNormal ), mFresnelPower);"
"}";



const char *glass_frag_shader=
"#version 100\r\n"

"uniform samplerCube tCube;"
"varying highp vec3 vReflect;"
"varying highp vec3 vRefract[3];"
"varying lowp float vReflectionFactor;"

"void main(void) {"
"	lowp vec4 reflectedColor = textureCube( tCube, vec3( -vReflect.x, -vReflect.yz ) );"
"	lowp vec4 refractedColor = vec4( 0.97 );"

"	refractedColor.r = textureCube( tCube, vec3( vRefract[0].x, vRefract[0].yz ) ).r;"
"	refractedColor.g = textureCube( tCube, vec3( vRefract[1].x, vRefract[1].yz ) ).g;"
"	refractedColor.b = textureCube( tCube, vec3( vRefract[2].x, vRefract[2].yz ) ).b;"

"	gl_FragColor=mix( refractedColor*0.94, reflectedColor, clamp( vReflectionFactor, 0.0, 1.0 ) );"
"}";

extern "C" Shader* CreateGlass(void){
	Shader* glass=CreateShader((char*)"glass", (char*)glass_vert_shader, (char*)glass_frag_shader);
	UseSurface (glass,(char*)"position",0,1);
	UseSurface (glass,(char*)"normal",0,4);
	UseMatrix (glass,(char*)"projection",2);
	UseMatrix (glass,(char*)"view",1);
	UseMatrix (glass,(char*)"world",0);

	SetFloat (glass, (char*)"mRefractionRatio", 0.988);
	SetFloat (glass, (char*)"mFresnelBias", 0.1);
	SetFloat (glass, (char*)"mFresnelPower", 1.5);
	SetFloat (glass, (char*)"mFresnelScale", 1.0);

	return glass;
}
