#include "OpenB3D.h"

class Global{
public:
	static int width,height,mode,depth,rate;
};
extern const char *gaussian_vert_shader;
extern const char *gaussian_frag_shader;

const char *dof_vert_shader=
"#version 120\r\n"

"varying vec3 Position;"
"varying vec2 TexCoord;"


"void main()"
"{"
"	TexCoord = gl_MultiTexCoord0.st;"
"	Position = vec3(gl_ModelViewMatrix * gl_Vertex);"
	
"	gl_Position = ftransform();"
"}";


const char *dof_frag_shader=
"#version 120\r\n"
"uniform int PassNum;"

"uniform sampler2D RenderTex;"
"uniform sampler2D BlurTex;"
"uniform sampler2D DepthTex;"

"varying vec3 Position;"
"varying vec2 TexCoord;"

"uniform float focus;"

"float LinearizeDepth(vec2 uv)"
"{"
"  float n = 1.0; /* camera z near*/"
"  float f = 1000.0; /* camera z far*/"
"  float z = texture2D(DepthTex, uv).x;"
"  return (2.0 * n) / (f + n - z * (f - n));"
"}"

"void main()"
"{"
"	float blur=abs(LinearizeDepth(TexCoord)-focus);"
"	gl_FragColor = mix(texture2D(RenderTex, TexCoord), texture2D(BlurTex, TexCoord),blur);"
"}";


extern "C" PostFX* CameraDOF(Camera* cam, float focus){
	Shader* DOFshader=CreateShader((char*)"DOF", (char*)dof_vert_shader, (char*)dof_frag_shader);
	Shader* gaussianshader=CreateShader((char*)"gaussian", (char*)gaussian_vert_shader, (char*)gaussian_frag_shader);
	SetInteger(gaussianshader, (char*)"RenderTex",0);
	SetInteger(gaussianshader, (char*)"Width",Global::width);
	SetInteger(gaussianshader, (char*)"Height",Global::height);

	SetInteger(DOFshader, (char*)"RenderTex",0);
	SetInteger(DOFshader, (char*)"BlurTex",1);
	SetInteger(DOFshader, (char*)"DepthTex",2);
	SetFloat(DOFshader, (char*)"focus",focus);

	PostFX* DOF=CreatePostFX(cam,3);
	AddRenderTarget (DOF,0,1,1,8,1.0);
	PostFXBuffer(DOF,0,0,1,0);
	AddRenderTarget (DOF,1,1,0,8,.5);
	PostFXBuffer(DOF,1,1,1,0);
	AddRenderTarget(DOF,2,1,0,8,1.0);
	PostFXBuffer(DOF,2,0,1,0);
	PostFXBuffer(DOF,2,2,1,1);
	PostFXBuffer(DOF,2,0,0,2);

	PostFXShader(DOF,0,gaussianshader);
	PostFXShaderPass(DOF,0,(char*)"PassNum",1);
	PostFXShader(DOF,1,gaussianshader);
	PostFXShaderPass(DOF,1,(char*)"PassNum",2);

	PostFXShader(DOF,2,DOFshader);


	return DOF;
}
