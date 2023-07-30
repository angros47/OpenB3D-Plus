#include "OpenB3D.h"

const char *toon_vert_shader=
"#version 100\r\n"

"attribute highp vec3 position, normal;"
"uniform highp mat4 projection, view, world;"
"uniform highp mat4 LightMatrix;"
"uniform lowp float contour;"

"varying highp vec3 vNormal;"
"varying lowp float n;"
"varying lowp vec3 lightDir;"



"void main()"
"{"
"	highp vec3 aVNormal = normalize(normal);"
"	highp vec4 lightdir = LightMatrix*vec4(0.0, 0.0, 1.0,0.0);"
"	lightDir = normalize(vec3(lightdir*world));"

"	vNormal = normalize(normal);"
"	vec4 n4 = vec4(normal,0.0);"
"	vec4 nView = normalize(view * world * n4);"
"	vec4 p=vec4(position, 1.0);"
"	vec4 v = normalize(view * world * p);"

"	n=dot(v,nView);"
"	if (n>-0.1) {"
"		p.xyz = p.xyz*contour; "
"		n=0.0;"
"	}"

"	gl_Position = projection * view * world * p;"

"}";

const char *toon_frag_shader=
"#version 100\r\n"

"varying lowp vec3 lightDir;"
"varying highp vec3 vNormal;"
"varying lowp float n;"
"uniform sampler2D ramp;"

"void main()"
"{"
"	lowp float intensity;"
"	lowp vec4 color;"
"	intensity = clamp(dot(lightDir,vNormal),0.0,1.0);"

"	color = texture2D(ramp,vec2(intensity,0.0));"

"	gl_FragColor = (n>=-0.15) ? vec4(0.0):color;"

"}";

extern "C" Texture* CreateRamp(int steps, float r, float g, float b, float r2, float g2, float b2 ) {
	struct col{
		char r, g, b, a;
	} color[256];
	float inc_r= (r2-r)/steps;
	float inc_g= (g2-g)/steps;
	float inc_b= (b2-b)/steps;

	int ladder=256/(steps+1);

	for (int i=0; i<255; i++){
		int l=i/ladder;
		color[i].r=r+inc_r*l;
		color[i].g=g+inc_g*l;
		color[i].b=b+inc_b*l;
		color[i].a=255;
	}
	Texture* ramp=CreateTexture (256, 1, 16, 1);
	BufferToTex (ramp, (unsigned char*)&color[0],0);
	return ramp;
}

extern "C" Shader* CreateToon(Entity* light){
	Shader* toon=CreateShader((char*)"toon", (char*)toon_vert_shader, (char*)toon_frag_shader);
	UseSurface (toon,(char*)"position",0,1);
	UseSurface (toon,(char*)"normal",0,4);
	UseMatrix (toon,(char*)"projection",2);
	UseMatrix (toon,(char*)"view",1);
	UseMatrix (toon,(char*)"world",0);
	UseEntity (toon,(char*)"LightMatrix",light,0);
	SetFloat (toon,(char*)"contour",1.02);

	return toon;
}
