#include "OpenB3D.h"
#include <stdlib.h>

const char *fur_vert_shader=
"#version 330 compatibility\r\n"
"attribute highp vec3 position, normal;"
"attribute highp vec2 texcoord;"

"uniform highp mat4 view, world;"
"uniform highp mat4 LightMatrix;"

"varying highp vec3 norm;"
"varying highp vec2 tcoord;"
"varying lowp float light;"

"void main(void) {"
"	tcoord=texcoord;"
"	norm=normalize ((view*world*vec4(normal,0.0)).xyz);"
"	gl_Position = view * world * vec4(position, 1.0);"

"	highp vec4 lightdir = LightMatrix*vec4(0.0, 0.0, 1.0,0.0);"
"	highp vec3 directionalVector = normalize(vec3(lightdir*world));"
"	light=clamp(dot(normal, directionalVector), 0.0, 1.0);"
"}";

const char *fur_geom_shader=
"#version 330 compatibility\r\n"

"layout(triangles) in;"
"layout(triangle_strip, max_vertices=64) out;"

"uniform mat4 projection, view;"
"uniform float length;"
"uniform vec3 Gravity;"

"in vec3 norm[3];"
"in vec2 tcoord[3];"
"in float light[3];"
"out vec2 texcoord;"
"out float alpha;"
"out float Light;"


"void main()"
"{	"
"	float shell=.0;"
"	float a=1.0;"
"	vec4 _Gravity=view*vec4(Gravity,0.0);"
"	for(int i=0; i<32; i++){"
"		for(int v=0; v<gl_in.length(); v++){"
"			gl_Position = projection*(gl_in[v].gl_Position+vec4(norm[v]*shell,0.0)+_Gravity*shell*shell);"
"			texcoord=tcoord[v];"
"			alpha=a;"
"			Light=light[v];"
"			EmitVertex();"
"		}"
"		EndPrimitive();"
"		shell+=length/32.0;"
"		a-=1./32.0;"
"	}"
"}";

const char *fur_frag_shader=
"#version 330 compatibility\r\n"

"uniform sampler2D alphamap;"
"varying highp vec2 texcoord;"
"varying float alpha;"
"varying lowp float Light;"

"uniform lowp float ambient, diffuse;"
"uniform sampler2D texture;"
 

"void main(void) {"
"	lowp float aColor;"
"	highp vec2 tcoords=texcoord+vec2(sin(alpha)*.0,0.0);"
"	aColor=texture2D(alphamap, tcoords).a;"
"	aColor*=alpha;"
"	lowp float TotalLight=Light*diffuse+ambient;"
"	gl_FragColor=vec4(texture2D(texture, tcoords).rgb*TotalLight,aColor);"
"	gl_FragDepth=mix(gl_FragCoord.z,0.0, aColor);"
"}";


extern "C" Shader* CreateFur(Entity* light, Texture* tex){
	Shader* fur=CreateShaderVGF((char*)"fur", (char*)fur_vert_shader, (char*)fur_geom_shader, (char*)fur_frag_shader);
	UseSurface (fur,(char*)"position",0,1);
	UseSurface (fur,(char*)"texcoord",0,2);
	UseSurface (fur,(char*)"normal",0,4);
	UseMatrix (fur,(char*)"projection",2);
	UseMatrix (fur,(char*)"view",1);
	UseMatrix (fur,(char*)"world",0);
	UseEntity (fur,(char*)"LightMatrix",light,0);
	SetInteger (fur,(char*)"alphamap",0);
	SetInteger (fur,(char*)"texture",1);
	SetFloat3 (fur,(char*)"Gravity",0,-1,0);
	SetFloat (fur,(char*)"length",.5);
	SetFloat (fur,(char*)"ambient",.5);
	SetFloat (fur,(char*)"diffuse",.5);

	if (tex!=0){
		unsigned char noise[65536*4];

		for (int i=0; i<65536*4; i++){
			noise[i]=rand()%255*rand()%2;
		}

		Texture* NoiseTex=CreateTexture(256,256,12,1);
		BufferToTex (NoiseTex, &noise[0],0);

		ShaderTexture (fur, NoiseTex, (char*)"alphamap",0);
		ShaderTexture (fur, tex, (char*)"texture",1);
	}


	return fur;
}
