#include "OpenB3D.h"

class Global{
public:
	static int width,height,mode,depth,rate;
};

const char *pixel_vert_shader=
"#version 120\r\n"

"varying vec2 Vertex_UV;"

"void main()"
"{"
"	Vertex_UV = gl_MultiTexCoord0.st;"
"	gl_Position = ftransform();"
"}";

const char *pixel_frag_shader=
"#version 120\r\n"
"uniform sampler2D tex0;"
"varying vec2 Vertex_UV;"

"uniform float depth;"
"uniform int dithering;"
"uniform vec2 screen;"

"void main(){"

"	vec2 UV;"
"	UV.x=floor(Vertex_UV.x*screen.x)/screen.x;"
"	UV.y=floor(Vertex_UV.y*screen.y)/screen.y;"

"	vec4 c = texture2D(tex0, UV);"
"	float r,g,b;"
"	if (dithering!=0){"
"		r=floor(c.r*depth+(mod(floor(Vertex_UV.x*screen.x), 2.)*1.2-mod(floor(Vertex_UV.y*screen.y), 2.)*.8)/depth)/depth;"
"		g=floor(c.g*depth+(mod(floor(Vertex_UV.x*screen.x), 2.)*1.2-mod(floor(Vertex_UV.y*screen.y), 2.)*.8)/depth)/depth;"
"		b=floor(c.b*depth+(mod(floor(Vertex_UV.x*screen.x), 2.)*1.2-mod(floor(Vertex_UV.y*screen.y), 2.)*.8)/depth)/depth;"
"	}else{"
"		r=floor(c.r*depth)/depth;"
"		g=floor(c.g*depth)/depth;"
"		b=floor(c.b*depth)/depth;"
"	}"

"	gl_FragColor = vec4(r,g,b,c.a);"
"}";

extern "C" PostFX* CameraPixelArt(Camera* cam, int width, int height, int colors, int dithering){
	Shader* pixelshader=CreateShader((char*)"pixel", (char*)pixel_vert_shader, (char*)pixel_frag_shader);
	PostFX* pixelart=CreatePostFX(cam,1);
	AddRenderTarget(pixelart,0,1,0,8,1.0);
	PostFXBuffer(pixelart,0,0,1,0);
	PostFXShader(pixelart,0,pixelshader);
	SetInteger(pixelshader, (char*)"dithering", dithering);
	SetFloat(pixelshader, (char*)"depth", colors);
	SetFloat2(pixelshader, (char*)"screen", width, height);

	return pixelart;
}
