#include "OpenB3D.h"

class Global{
public:
	static int width,height,mode,depth,rate;
};

const char *fisheye_vert_shader=
"#version 120\r\n"

"varying vec2 Vertex_UV;"

"void main()"
"{"
"	Vertex_UV = gl_MultiTexCoord0.st;"
"	gl_Position = ftransform();"
"}";

const char *fisheye_frag_shader=
"#version 120\r\n"
"uniform sampler2D tex0;"
"uniform vec2 ratio;"
"uniform float aperture;"
"varying vec2 Vertex_UV;"
"const float PI = 3.1415926535;"

"void main()"
"{"
"  float apertureHalf = 0.5 * aperture * (PI / 180.0);"
"  float maxFactor = sin(apertureHalf);"
  
"  vec2 uv;"
"  vec2 xy = 2.0 * Vertex_UV - 1.0;"
"  float d = length(xy*ratio);"
"  if (d < (2.0-maxFactor))"
"  {"
"    d = length(xy * maxFactor);"
"    float z = sqrt(1.0 - d * d);"
"    float r = atan(d, z) / PI;"
"    float phi = atan(xy.y, xy.x);"
    
"    uv.x = r * cos(phi) + 0.5;"
"    uv.y = r * sin(phi) + 0.5;"
"  }"
"  else"
"  {"
"	discard;"
"  }"
"  vec4 c = texture2D(tex0, uv);"
"  gl_FragColor = c;"
"}";


extern "C" PostFX* CameraFisheye(Camera* cam, int aperture){
	Shader* fisheyeshader=CreateShader((char*)"fisheye", (char*)fisheye_vert_shader, (char*)fisheye_frag_shader);
	PostFX* fisheye=CreatePostFX(cam,1);
	AddRenderTarget(fisheye,0,1,0,8,1.0);
	PostFXBuffer(fisheye,0,0,1,0);
	PostFXShader(fisheye,0,fisheyeshader);
	SetFloat2 (fisheyeshader, (char*)"ratio",((float)Global::width/(float)Global::height),1);
	SetFloat (fisheyeshader, (char*)"aperture", aperture);
	return fisheye;
}
