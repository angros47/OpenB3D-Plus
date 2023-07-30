#include "OpenB3D.h"

class Global{
public:
	static int width,height,mode,depth,rate;
};
extern const char *gaussian_vert_shader;
extern const char *gaussian_frag_shader;

const char *hdr_vert_shader=
"#version 120\r\n"

"varying vec2 Vertex_UV;"

"void main()"
"{"
"	Vertex_UV = gl_MultiTexCoord0.st;"
"	gl_Position = ftransform();"
"}";

const char *hdr_frag_shader=
"#version 120\r\n"
"uniform sampler2D tex0;"
"varying vec2 Vertex_UV;"

"void main(){"

"	vec4 c = texture2D(tex0, Vertex_UV);"
"	const float gamma = 2.2;"
"	vec3 hdrColor = c.rgb;"
  
"	/* reinhard tone mapping*/"
"	vec3 mapped = hdrColor / (hdrColor + vec3(1.0));"
"	/* gamma correction */"
"	mapped = pow(mapped, vec3(1.0 / gamma));"
  
"	gl_FragColor = vec4(mapped, 1.0);"
"	/*gl_FragColor = c;*/"
"}";


const char *bloom_frag_shader=
"uniform sampler2D texture0;"
"uniform sampler2D texture1;"

"uniform float exposure;"
"uniform float cutoff;"
"uniform int PassNum;"
"varying vec2 Vertex_UV;"


"vec4 Pass1(){"
"    vec4 texel = texture2D(texture0, Vertex_UV);"
"    float brightness = dot(texel.rgb, vec3(0.2126, 0.7152, 0.0722));"

"    return vec4(texel.rgb*step(cutoff,brightness),1.0);"
"}"


"vec4 Pass2(){"
"    vec4 texel = texture2D(texture0, Vertex_UV);"
"    vec4 bloom = texture2D(texture1, Vertex_UV);"

"    float Y = dot(vec4(0.30, 0.59, 0.11, 0.0), texel);"
"    Y = Y * exposure;"
"    Y = Y / (Y + 1.0);"
"    texel.rgb = texel.rgb * Y;"
"    vec4 hdr = vec4(texel.rgb, 1.0);"


"    return hdr+bloom;"
"}"


"void main()"
"{"

"	if (PassNum == 1)"
"	{"
"		gl_FragColor = Pass1();"
"	}"
"	if (PassNum == 2)"
"	{"
"		gl_FragColor = Pass2();"
"	}"



"}";


extern "C" PostFX* CameraHDR(Camera* cam){
	Shader* hdrshader=CreateShader((char*)"HDR", (char*)hdr_vert_shader, (char*)hdr_frag_shader);
	PostFX* hdr=CreatePostFX(cam,1);
	AddRenderTarget(hdr,0,1,0,32,1.0);
	PostFXBuffer(hdr,0,0,1,0);
	PostFXShader(hdr,0,hdrshader);

	return hdr;
}

extern "C" PostFX* CameraBloom(Camera* cam, float exposure, float cutoff){
	Shader* bloomshader=CreateShader((char*)"Bloom", (char*)hdr_vert_shader, (char*)bloom_frag_shader);
	Shader* gaussianshader=CreateShader((char*)"gaussian", (char*)gaussian_vert_shader, (char*)gaussian_frag_shader);
	SetInteger(gaussianshader, (char*)"RenderTex",0);
	SetInteger(gaussianshader, (char*)"Width",Global::width);
	SetInteger(gaussianshader, (char*)"Height",Global::height);

	SetFloat(bloomshader, (char*)"exposure", exposure);
	SetFloat(bloomshader, (char*)"cutoff", cutoff);
	SetInteger(bloomshader, (char*)"texture0", 0);
	SetInteger(bloomshader, (char*)"texture1", 1);

	PostFX* bloom=CreatePostFX(cam,4);
	AddRenderTarget(bloom,0,1,0,32,1.0);
	PostFXBuffer(bloom,0,0,1,0);

	AddRenderTarget(bloom,1,1,0,8,1.0);
	PostFXBuffer(bloom,1,1,1,0);

	AddRenderTarget(bloom,2,1,0,8,.5);
	PostFXBuffer(bloom,2,2,1,0);
	AddRenderTarget(bloom,3,1,0,8,1.0);
	PostFXBuffer(bloom,3,0,1,0);
	PostFXBuffer(bloom,3,3,1,1);


	PostFXShader(bloom,0,bloomshader);
	PostFXShaderPass (bloom,0,(char*)"PassNum",1);

	PostFXShader(bloom,1,gaussianshader);
	PostFXShaderPass (bloom,1,(char*)"PassNum",1);
	PostFXShader(bloom,2,gaussianshader);
	PostFXShaderPass (bloom,2,(char*)"PassNum",2);

	PostFXShader(bloom,3,bloomshader);
	PostFXShaderPass (bloom,3,(char*)"PassNum",2);


	return bloom;
}
