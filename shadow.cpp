#include "OpenB3D.h"

#ifdef linux
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#ifdef WIN32
#include <gl\GLee.h>
#endif

class Texture{
public:
	unsigned int texture;
};

const char *Shadow_vert_shader=
"#version 140\r\n"
"varying vec4 shadowCoordinate;"
"varying vec3 normal;"
"varying vec4 vpos, lightPos;"
 
"uniform mat4 lightingInvMatrix;"
"uniform mat4 modelMat;"
"uniform mat4 projMatrix;"

"uniform mat4 biasMatrix;"

 
"void main() {"
"	shadowCoordinate = biasMatrix*projMatrix*lightingInvMatrix *modelMat* gl_Vertex;"
"	normal = normalize(gl_NormalMatrix * gl_Normal);"
"	vpos = gl_ModelViewMatrix * gl_Vertex;"
"	lightPos=inverse(lightingInvMatrix)[3];"


"	gl_TexCoord[0] = gl_MultiTexCoord0;"
"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
"}";

const char *Shadow_frag_shader=
"#version 140\r\n"

"varying vec4 shadowCoordinate;"
"varying vec3 normal;"
"varying vec4 vpos, lightPos;"
 
"uniform sampler2DShadow depthSampler;"

"uniform sampler2D Diffuse;"

 
"void main() {"
"	float shadow = 1.0f;"
"	shadow = shadow2DProj(depthSampler, shadowCoordinate).z;"
"	vec4 col=texture2D(Diffuse, gl_TexCoord[0].xy);"
"	vec3 lightDir=vec3(lightPos-vpos);"
"	float NdotL = max(dot(normalize(normal),normalize(lightDir)),0.0)*shadow;"

"	gl_FragColor = col*NdotL;"
"}";

static Camera* CamLight;
static Texture* ShadowSampler;

extern "C" void UpdateShadows(){
	glPolygonOffset (2.0f, 1.0f); glEnable(GL_POLYGON_OFFSET_FILL);
	DepthBufferToTex(ShadowSampler, CamLight);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

extern "C" Shader* CreateShadowMap(Entity* light){
	Shader* ShadowMap=CreateShader((char*)"shadowmap", (char*)Shadow_vert_shader, (char*)Shadow_frag_shader);
	CamLight=CreateCamera(light);
	CameraViewport(CamLight,0,0,512,512);
	HideEntity(CamLight);
	ShadowSampler=CreateTexture(512,512,1,1);

	glBindTexture(GL_TEXTURE_2D,ShadowSampler->texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	ShaderTexture(ShadowMap, 0, (char*)"Diffuse", 0);
	ShaderTexture(ShadowMap, ShadowSampler, (char*)"depthSampler", 1);
	//SetInteger (ShadowMap, (char*)"Diffuse",0);
	SetInteger (ShadowMap, (char*)"depthSampler",1);

	UseEntity(ShadowMap, (char*)"lightingInvMatrix", CamLight, 1);
	UseMatrix(ShadowMap, (char*)"modelMat",0);
	//UseMatrix(ShadowMap, (char*)"projMatrix",2);

	int prog=GetShaderProgram(ShadowMap);

	glUseProgram(prog);

	float biasMatrix[]=	{0.5, 0.0, 0.0, 0.0,
				0.0, 0.5, 0.0, 0.0,
				0.0, 0.0, 0.5, 0.0,
				0.5, 0.5, 0.5, 1.0};

	glUniformMatrix4fv(glGetUniformLocation(prog,"biasMatrix") , 1 , 0, &biasMatrix[0]);
	glUniformMatrix4fv(glGetUniformLocation(prog,"projMatrix") , 1 , 0, CameraProjMatrix(CamLight));

	glUseProgram(0);

	//ShaderFunction(ShadowMap, UpdateSCam,0);
	return ShadowMap;
}


