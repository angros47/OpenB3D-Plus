#include "OpenB3D.h"

const char *bump_vert_shader=
"#version 100\r\n"

"precision highp float;"

"attribute vec3 vert_pos;"
"attribute vec3 vert_norm;"
"attribute vec2 vert_uv;"

"uniform mat4 world;"
"uniform mat4 view;"
"uniform mat4 projection;"
"uniform highp mat4 LightMatrix;"

"varying vec2 frag_uv;"
"varying vec3 varyingNormal;"
"varying vec3 lightDirection;"
"varying vec3 viewDirection;"

"void main(void)"
"{"
"	gl_Position =  projection * view * world * vec4(vert_pos, 1.0);"
"	vec3 vertexWorld = vec3(view * world * vec4(vert_pos, 1.0));"

"	varyingNormal = normalize((view * world * vec4( vert_norm, 0.0 ) ).xyz);"

"	highp vec4 lightdir = LightMatrix*vec4(0.0, 0.0, 1.0,0.0);"
"	lightDirection = normalize(vec3(view*lightdir));"
"	viewDirection = vertexWorld;"

 
"	frag_uv = vert_uv;"
"}";


const char *bump_frag_shader=
"#version 100\r\n"
"#extension GL_OES_standard_derivatives : enable\r\n"
"precision highp float;"

"uniform sampler2D tex_norm;"
"uniform sampler2D tex_diffuse;"
"uniform sampler2D tex_depth;"

"uniform int type;"
"uniform int show_tex;"
"uniform float depth_scale;"
"uniform float num_layers;"

"varying vec2 frag_uv;"
"varying vec3 varyingNormal;"
"varying vec3 lightDirection;"
"varying vec3 viewDirection;"


"vec2 parallax_uv(vec2 uv, vec3 view_dir)"
"{"
"	if (type == 2) {"
"		/* Parallax mapping*/"
"		float depth = texture2D(tex_depth, uv).r;    "
"		vec2 p = view_dir.xy * (depth * depth_scale) / view_dir.z;"
"		return uv - p;  "
"	} else {"
"		float layer_depth = 1.0 / num_layers;"
"		float cur_layer_depth = 0.0;"
"		vec2 delta_uv = view_dir.xy * depth_scale / (view_dir.z * num_layers);"
"		vec2 cur_uv = uv;"

"		float depth_from_tex = texture2D(tex_depth, cur_uv).r;"

"		for (int i = 0; i < 32; i++) {"
"			cur_layer_depth += layer_depth;"
"			cur_uv -= delta_uv;"
"			depth_from_tex = texture2D(tex_depth, cur_uv).r;"
"			if (depth_from_tex < cur_layer_depth) {"
"				break;"
"			}"
"		}"

"		if (type == 3) {"
"			/* Steep parallax mapping */"
"			return cur_uv;"
"		} else {"
"			/* Parallax occlusion mapping */"
"			vec2 prev_uv = cur_uv + delta_uv;"
"			float next = depth_from_tex - cur_layer_depth;"
"			float prev = texture2D(tex_depth, prev_uv).r - cur_layer_depth + layer_depth;"
"			float weight = next / (next - prev);"
"			return mix(cur_uv, prev_uv, weight);"
"		}"
"	}"
"}"


"mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)"
"{"
"	/* get edge vectors of the pixel triangle */"
"	vec3 dp1 = dFdx( p );"
"	vec3 dp2 = dFdy( p );"
"	vec2 duv1 = dFdx( uv );"
"	vec2 duv2 = dFdy( uv );"

"	/* solve the linear system */"
"	vec3 dp2perp = cross( dp2, N );"
"	vec3 dp1perp = cross( N, dp1 );"
"	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;"
"	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;"

"	/* construct a scale-invariant frame */"
"	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );"
"	return mat3( T * invmax, B * invmax, N );"
"}"

"highp mat3 transpose(in highp mat3 inMatrix) "
"{"
"	highp vec3 i0 = inMatrix[0];"
"	highp vec3 i1 = inMatrix[1];"
"	highp vec3 i2 = inMatrix[2];"

"	highp mat3 outMatrix = mat3("
"		vec3(i0.x, i1.x, i2.x),"
"		vec3(i0.y, i1.y, i2.y),"
"		vec3(i0.z, i1.z, i2.z)"
"	);"
"	return outMatrix;"
"}"



"void main(void)"
"{"

"	mat3 TBN = cotangent_frame(varyingNormal ,  viewDirection, frag_uv );"

"	/* Only perturb the texture coordinates if a parallax technique is selected*/"
"	vec2 uv = (type < 2) ? frag_uv : parallax_uv(frag_uv, transpose(TBN) * viewDirection);"

"	vec3 albedo = texture2D(tex_diffuse, uv).rgb;"
"	if (show_tex == 0) { albedo = vec3(1,1,1); }"
"	vec3 ambient = 0.3 * albedo;"

"	if (type == 0) {"
"		/* No bump mapping */"
"		vec3 norm = vec3(0,0,1);"
"		float diffuse = max(dot(lightDirection, norm), 0.0);"
"		gl_FragColor = vec4(diffuse * albedo + ambient, 1.0);"

"	} else {"
"		/* Normal mapping */"
"		vec3 norm = TBN*normalize(texture2D(tex_norm, uv).rgb * 2.0 - 1.0);"
"		float diffuse = max(dot(lightDirection, norm), 0.0);"
"		gl_FragColor = vec4(diffuse * albedo + ambient, 1.0);"
"	}"
"}";






/*
    0 = No bump mapping
    1 = Normal mapping
    2 = Parallax mapping
    3 = Steep parallax mapping
    4 = Parallax occlusion mapping
*/

extern "C" Shader* CreateBump(Entity* light, int show_diffuse, int mode){
	Shader* bump=CreateShader((char*)"bump", (char*)bump_vert_shader, (char*)bump_frag_shader);
	UseSurface (bump,(char*)"vert_pos",0,1);
	UseSurface (bump,(char*)"vert_norm",0,4);
	UseSurface (bump,(char*)"vert_uv",0,2);
	UseMatrix (bump,(char*)"projection",2);
	UseMatrix (bump,(char*)"view",1);
	UseMatrix (bump,(char*)"world",0);

	SetInteger(bump, (char*)"type", mode);
	SetInteger(bump, (char*)"tex_norm", 0);
	SetInteger(bump, (char*)"tex_diffuse", 1);
	SetInteger(bump, (char*)"tex_depth", 2);

	SetInteger(bump, (char*)"show_tex", show_diffuse);
	SetFloat(bump, (char*)"depth_scale", .1);
	SetFloat(bump, (char*)"num_layers", 3);

	UseEntity (bump, (char*)"LightMatrix",light,0);

	return bump;
}
