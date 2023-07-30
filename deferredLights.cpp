#include "OpenB3D.h"
#include <stdio.h>
#include <iostream>

#ifdef linux
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#ifdef WIN32
#include <gl\GLee.h>
#endif


#include <list>
#include <vector>

class Global{
public:
	static int width,height,mode,depth,rate;
};

using namespace std;

struct DLight{
	Mesh* Volume;
	float red, green, blue;
	float constant, linear, quadratic;
	int isSpot;
};


list<DLight> DeferredLights;
static Camera* camera;

static Shader* deferred_ambient;
static Shader* deferred_light;
static Shader* deferred_surf;

static float light_red, light_green, light_blue;
static float attenuation_constant, attenuation_linear, attenuation_quadratic;
static int light_isSpot;

const char *deferred_surf_vert=
"#version 120\r\n"
"varying vec3 normals;"
"varying vec4 position;"
/*"uniform mat4 ModelMatrix;"*/
"uniform mat4 WorldMatrix;"
/*"uniform mat4 ProjMatrix;"*/

"void main( void )"
"{"
"    mat3 worldRotationInverse = transpose(mat3(WorldMatrix));"
"    gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;"
"    gl_TexCoord[0] = gl_MultiTexCoord0;"
"    normals        = normalize(worldRotationInverse * gl_NormalMatrix * gl_Normal);"
"    position       = gl_ModelViewMatrix * gl_Vertex;"
"    gl_FrontColor  = vec4(1.0, 1.0, 1.0, 1.0);"
"}";

const char *deferred_surf_frag=
"#version 120\r\n"
"varying vec4		position;"
"varying vec3		normals;"
"uniform sampler2D	tDiffuse;"

"void main( void )"
"{"
"	gl_FragData[0] = vec4(texture2D(tDiffuse, gl_TexCoord[0].st).rgb,1.0);"
"	gl_FragData[1] = vec4(position.xyz,1.0);"
"	gl_FragData[2] = vec4(normals.xyz,1.0);"
"}";


const char *deferred_ambient_vert=
"#version 120\r\n"

"varying vec2 Vertex_UV;"

"void main()"
"{"
"	Vertex_UV = gl_MultiTexCoord0.st;"
"	gl_Position = ftransform();"
"}";

const char *deferred_ambient_frag=
"#version 120\r\n"
"uniform sampler2D Tcolor;"
"varying vec2 Vertex_UV;"
"uniform vec3 ambientLight;"

"void main(){"
"	vec3 c = texture2D(Tcolor, Vertex_UV).rgb;"
"	gl_FragColor  = vec4(c * ambientLight,1.0);"
"}";




const char *deferred_light_vert=
"#version 120\r\n"

"attribute vec3 position;"
"uniform mat4 projection, view, world, cameraMat;"
//"varying highp float Z;"
"varying vec3 LightCoord;"
"varying vec3 SpotDir;"

"  void main(void) {"
"	vec4 p=vec4(position, 1.0);"
"	vec4 pos=(view * world * p);"
/*"	vec4 pos0=(view * world* vec4(0.0,0.0,0.0,1.0));"
"	vec4 pos1=(view * world* vec4(0.0,0.0,0.0,1.0));"
"	pos1.z=2.0*pos0.z-pos1.z;"
"	pos=projection * pos;"
"	pos1=projection * pos1;"*/

"	LightCoord=(cameraMat*world)[3].xyz;"
"	SpotDir=normalize(view*world*vec4(0.0, 0.0, 1.0,0.0)).xyz;"

//"	Z=pos1.z+1.0;"
"	gl_Position = projection*pos;"
"  }\r\n";

/*const char *deferred_light_frag=
"#version 120\r\n"
"uniform sampler2D Tcolor;"
"uniform sampler2D Tposition;"
"uniform sampler2D Tnormal;"
"uniform vec2 screen;"
"uniform vec3 color;"
"uniform vec3 attenuation;"
"uniform int isSpot;"

"varying vec3 LightCoord;"
"varying vec3 CameraCoord;"
"varying vec3 SpotDir;"

"void main(void) {"
"	vec2 Vertex_UV=gl_FragCoord.xy/screen;"


"    vec4 image = texture2D(Tcolor, Vertex_UV);"
"    vec4 position = texture2D(Tposition, Vertex_UV);"
"    vec4 normal = texture2D(Tnormal, Vertex_UV);"
    
"    vec3 light = vec3(50,100,50);"
"    vec3 lightDir = light - position.xyz ;"
    
"    normal = normalize(normal);"
"    lightDir = normalize(lightDir);"
    
"    vec3 eyeDir = normalize(CameraCoord-position.xyz);"
"    vec3 vHalfVector = normalize(lightDir.xyz+eyeDir);"
    
"    gl_FragColor = max(dot(normal,lightDir),0) * image + pow(max(dot(normal,vHalfVector),0.0), 100) * 1.5;"

"}\r\n";*/




const char *deferred_light_frag=
"#version 120\r\n"
"uniform sampler2D Tcolor;"
"uniform sampler2D Tposition;"
"uniform sampler2D Tnormal;"
"uniform vec2 screen;"
"uniform vec3 color;"
"uniform vec3 attenuation;"
"uniform int isSpot;"

"varying vec3 LightCoord;"
"varying vec3 SpotDir;"

"void main(void) {"
"	vec2 Vertex_UV=gl_FragCoord.xy/screen;"

"	vec3 lightDir = LightCoord - texture2D(Tposition,Vertex_UV).xyz;"
"	vec3 norm = normalize(texture2D(Tnormal,Vertex_UV).xyz);"
"	float dist=length(lightDir);"
"	lightDir = normalize(lightDir);"
"	vec3 light=clamp(max(dot(norm,lightDir),0.0)/(attenuation.x+attenuation.y*dist+attenuation.z*dist*dist),0.0,1.0) * color;"
"	vec3 SpotLight=light*step(.89,dot(SpotDir, lightDir));"

"	gl_FragColor = vec4(texture2D(Tcolor, Vertex_UV).rgb * (isSpot>0?SpotLight: light),1.0);"
"}\r\n";


void UpdateDeferredLights(){

	glEnable(GL_BLEND);
	AmbientShader(deferred_light);
	list<DLight>::iterator it;
	for(it=DeferredLights.begin();it!=DeferredLights.end();it++){
		DLight light=*it;

		light_red=light.red;
		light_green=light.green;
		light_blue=light.blue;

		attenuation_constant=light.constant;
		attenuation_linear=light.linear;
		attenuation_quadratic=light.quadratic;

		light_isSpot=light.isSpot;

		light.Volume->Render();
	}
	glDisable(GL_BLEND);
	AmbientShader(deferred_surf);
}

extern "C" PostFX* CameraDeferred(Camera* cam){

	deferred_surf=CreateShader((char*)"DeferredSurface", (char*)deferred_surf_vert, (char*)deferred_surf_frag);
	//UseMatrix (deferred_surf,(char*)"ModelMatrix",0);
	UseMatrix (deferred_surf,(char*)"WorldMatrix",1);
	//UseMatrix (deferred_surf,(char*)"ProjMatrix",2);
	SetInteger (deferred_surf,(char*)"tDiffuse",0);
	AmbientShader(deferred_surf);

	deferred_ambient=CreateShader((char*)"DeferredLight", (char*)deferred_ambient_vert, (char*)deferred_ambient_frag);
	SetFloat3 (deferred_ambient,(char*)"ambientLight", .5, .5, .5);

	deferred_light=CreateShader((char*)"DeferredLight", (char*)deferred_light_vert, (char*)deferred_light_frag);
	UseSurface (deferred_light,(char*)"position",0,1);
	UseMatrix (deferred_light,(char*)"projection",2);
	UseMatrix (deferred_light,(char*)"view",1);
	UseMatrix (deferred_light,(char*)"world",0);
	SetInteger (deferred_light,(char*)"Tcolor",0);
	SetInteger (deferred_light,(char*)"Tposition",1);
	SetInteger (deferred_light,(char*)"Tnormal",2);
	UseFloat3 (deferred_light, (char*)"color", &light_red, &light_green, &light_blue);
	UseFloat3 (deferred_light, (char*)"attenuation", &attenuation_constant, &attenuation_linear, &attenuation_quadratic);
	UseInteger (deferred_light,(char*)"isSpot", &light_isSpot);
	SetFloat2 (deferred_light,(char*)"screen", Global::width, Global::height);
	UseEntity (deferred_light, (char*)"cameraMat", cam,1);

	SetInteger (deferred_ambient,(char*)"Tcolor",0);

	PostFX* deferr=CreatePostFX(cam,1);
	AddRenderTarget(deferr,0,3,1,32);
	PostFXBuffer(deferr,0,0,1,0);
	PostFXBuffer(deferr,0,0,2,1);
	PostFXBuffer(deferr,0,0,3,2);
	PostFXBuffer(deferr,0,0,0,3);

	PostFXShader(deferr,0,deferred_ambient);
	PostFXFunction(deferr,0, UpdateDeferredLights);
	camera=cam;

	return deferr;
}

extern "C" Entity* AddDeferredLight(int light_type, Entity* parent){
	DLight light;
	if (light_type==2){
		light.Volume=CreateCone(8, 1, parent);
		RotateMesh(light.Volume,90,0,0);
		PositionMesh(light.Volume,0,0,1);
		light.isSpot=1;
	}else{
		light.Volume=CreateSphere(8, parent);
		light.isSpot=0;
	}
	light.red=1;
	light.green=1;
	light.blue=1;
	DeferredLights.push_back(light);
	EntityBlend(light.Volume,3);
	FlipMesh(light.Volume);
	light.constant=1;
	light.linear=0;
	light.quadratic=0;

	EntityOrder(light.Volume,-1);
	ShadeMesh (light.Volume, deferred_light);
	HideEntity(light.Volume);
	return light.Volume;
}

list<DLight>::iterator GetDLight(Entity* light){
	list<DLight>::iterator it;
	for(it=DeferredLights.begin();it!=DeferredLights.end();it++){
		if (it->Volume==light) return it;
	}
	return DeferredLights.end();
}

extern "C" void DeferredLightColor(Entity* light, float r,float g,float b){
	list<DLight>::iterator l=GetDLight(light);
	l->red=r/255.0;
	l->green=g/255.0;
	l->blue=b/255.0;
}

extern "C" void DeferredLightAttenuation(Entity* light, float constant,float linear,float quadratic){
	list<DLight>::iterator l=GetDLight(light);
	l->constant=constant;
	l->linear=linear;
	l->quadratic=quadratic;
}

extern "C" void DeferredAmbientLight(float r,float g,float b){
	SetFloat3 (deferred_ambient,(char*)"ambientLight", r/255.0, g/255.0, b/255.0);
}

extern "C" void FreeDeferredLight(Entity* light){
	list<DLight>::iterator l=GetDLight(light);
	DeferredLights.erase(l);
	FreeEntity(light);
}
