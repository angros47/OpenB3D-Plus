#include "OpenB3D.h"
#include <stdlib.h>

class Global{
public:
	static int width,height,mode,depth,rate;
};


const char *ssao_vert_shader=
"#version 120\r\n"

"varying vec3 Position;"
"varying vec2 TexCoord;"


"void main()"
"{"
"	TexCoord = gl_MultiTexCoord0.st;"
"	Position = vec3(gl_ModelViewMatrix * gl_Vertex);"
	
"	gl_Position = ftransform();"
"}";

const char *ssao_frag_shader=
"#version 120\r\n"

"uniform sampler2D RenderTex;"
"uniform sampler2D DepthTex;"
"uniform sampler2D noise;"

"varying vec2 TexCoord;"

"vec3 GetNormalFromDepth(float depth, vec2 uv);"

"uniform vec2 agk_resolution;"
"uniform float ssaoStrength=1.5;"
"uniform float ssaoBase=0.75;"
"uniform float ssaoArea=.025;"
"uniform float ssaoFalloff=0.000001;"
"uniform float ssaoRadius=0.025;"

"const int samples = 16;"
"vec3 sampleSphere[samples];"

"void main()"
"{"
"    float depth = texture2D(DepthTex, TexCoord).r;"
"    vec3 random = normalize( texture2D(noise, TexCoord * agk_resolution / 64.0).rgb );"

"    vec3 position = vec3(TexCoord, depth);"
"    vec3 normal = GetNormalFromDepth(depth, TexCoord);"

"    sampleSphere[0] = vec3( 0.5381, 0.1856,-0.4319);"
"    sampleSphere[1] = vec3( 0.1379, 0.2486, 0.4430);"
"    sampleSphere[2] = vec3( 0.3371, 0.5679,-0.0057);"
"    sampleSphere[3] = vec3(-0.6999,-0.0451,-0.0019);"
"    sampleSphere[3] = vec3( 0.0689,-0.1598,-0.8547);"
"    sampleSphere[5] = vec3( 0.0560, 0.0069,-0.1843);"
"    sampleSphere[6] = vec3(-0.0146, 0.1402, 0.0762);"
"    sampleSphere[7] = vec3( 0.0100,-0.1924,-0.0344);"
"    sampleSphere[8] = vec3(-0.3577,-0.5301,-0.4358);"
"    sampleSphere[9] = vec3(-0.3169, 0.1063, 0.0158);"
"    sampleSphere[10] = vec3( 0.0103,-0.5869, 0.0046);"
"    sampleSphere[11] = vec3(-0.0897,-0.4940, 0.3287);"
"    sampleSphere[12] = vec3( 0.7119,-0.0154,-0.0918);"
"    sampleSphere[13] = vec3(-0.0533, 0.0596,-0.5411);"
"    sampleSphere[14] = vec3( 0.0352,-0.0631, 0.5460);"
"    sampleSphere[15] = vec3(-0.4776, 0.2847,-0.0271);"

"    float radiusDepth = ssaoRadius/depth;"
"    float occlusion = 0.0;"
"    for(int i=0; i < samples; i++)"
"    {"
"        vec3 ray = radiusDepth * reflect(sampleSphere[i], random);"
"        vec3 hemiRay = position + sign(dot(ray, normal)) * ray;"

"        float occDepth = texture2D(DepthTex, clamp(hemiRay.xy, 0.0, 1.0)).r;"
"        float difference = depth - occDepth;"

"        occlusion += step(ssaoFalloff, difference) * (1.0 - smoothstep(ssaoFalloff, ssaoArea, difference));"

"        /* float rangeCheck = abs(difference) < radiusDepth ? 1.0 : 0.0;"
"        occlusion += (occDepth <= position.z ? 1.0 : 0.0) * rangeCheck;*/"
"    }"

"    float ao = 1.0 - ssaoStrength * occlusion * (1.0 / float(samples));"

"    gl_FragColor = vec4(texture2D(RenderTex, TexCoord).rgb*clamp(ao + ssaoBase, 0.0, 1.0),1.0);"
"}"

"vec3 GetNormalFromDepth(float depth, vec2 uv)"
"{"
"    vec2 offset1 = vec2(0.0,1.0/agk_resolution.y);"
"    vec2 offset2 = vec2(1.0/agk_resolution.x,0.0);"

"    float depth1 = texture2D(DepthTex, uv + offset1).r;"
"    float depth2 = texture2D(DepthTex, uv + offset2).r;"

"    vec3 p1 = vec3(offset1, depth1 - depth);"
"    vec3 p2 = vec3(offset2, depth2 - depth);"

"    vec3 normal = cross(p1, p2);"
"    normal.z = -normal.z;"

"    return normalize(normal);"
"}";


extern "C" PostFX* CameraSSAO(Camera* cam, float strength, float base, float area, float falloff, float radius){
	Shader* SSAOshader=CreateShader((char*)"SSAO", (char*)ssao_vert_shader, (char*)ssao_frag_shader);
	PostFX* SSAO=CreatePostFX(cam,1);

	unsigned char noise[4096*4];

	for (int i=0; i<4096*4; i++){
		noise[i]=rand()%255;
	}

	Texture* NoiseTex=CreateTexture(64,64,9,1);
	BufferToTex (NoiseTex, &noise[0],0);


	AddRenderTarget (SSAO,0,1,1,8,1.0);
	PostFXTexture (SSAO,0,NoiseTex,2);
	PostFXBuffer (SSAO,0,0,1,0);
	PostFXBuffer (SSAO,0,0,0,1);

	PostFXShader(SSAO,0,SSAOshader);

	SetInteger (SSAOshader, (char*)"RenderTex",0);
	SetInteger (SSAOshader, (char*)"DepthTex",1);
	SetInteger (SSAOshader, (char*)"noise",2);
	SetFloat2 (SSAOshader, (char*)"agk_resolution",(float)Global::width,(float)Global::height);

	SetFloat (SSAOshader, (char*)"ssaoStrength", strength);
	SetFloat (SSAOshader, (char*)"ssaoBase", base);
	SetFloat (SSAOshader, (char*)"ssaoArea",area);
	SetFloat (SSAOshader, (char*)"ssaoFalloff", falloff);
	SetFloat (SSAOshader, (char*)"ssaoRadius", radius);



	return SSAO;
}
