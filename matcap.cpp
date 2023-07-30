#include "OpenB3D.h"

const char *matcap_vert_shader=
"#version 100\r\n"

"attribute highp vec3 position, normal;"
"uniform highp mat4 projection, view, world;"
"varying highp vec3 vNormal;"

"void main(void) {"
"	gl_Position = projection * view * world * vec4(position, 1.0);"
"	vNormal = normalize((world*vec4(normal,0.0)).xyz);"
"}";

const char *matcap_vert_shader_flat=
"#version 100\r\n"

"attribute highp vec3 position, normal;"
"uniform highp mat4 projection, view, world;"
"varying highp vec3 vNormal;"

"void main(void) {"
"	gl_Position = projection * view * world * vec4(position, 1.0);"
"	lowp vec3 pos_eye=normalize(vec3(view * world * vec4(position, 1.0)));"
"	lowp vec3 norm_eye=normalize(vec3(view * world * vec4(normal, 0.0)));"
"	vNormal = vec3(vec4(reflect( pos_eye, norm_eye),0.0) * view);"
"}";




const char *matcap_frag_shader=
"#version 100\r\n"

"uniform highp mat4 view;"
"uniform sampler2D matcapTexture;"
"varying highp vec3 vNormal;"

"void main(void) {"
"	highp vec2 muv = vec2(view * vec4(normalize(vNormal), 0))*0.5 - vec2(0.5,0.5);"
"	gl_FragColor = texture2D(matcapTexture, vec2(muv.x, 1.0-muv.y));"
"}";


extern "C" Shader* CreateMatcap(int mode){
	Shader* matcap;
	if (mode==0){
		matcap=CreateShader((char*)"matcap", (char*)matcap_vert_shader, (char*)matcap_frag_shader);
	}else{
		matcap=CreateShader((char*)"matcap", (char*)matcap_vert_shader_flat, (char*)matcap_frag_shader);
	}

	UseSurface (matcap,(char*)"position",0,1);
	UseSurface (matcap,(char*)"normal",0,4);
	UseMatrix (matcap,(char*)"projection",2);
	UseMatrix (matcap,(char*)"view",1);
	UseMatrix (matcap,(char*)"world",0);

	return matcap;
}
