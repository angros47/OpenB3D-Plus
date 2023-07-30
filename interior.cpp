#include "OpenB3D.h"

const char *interior_vert_shader=
"#version 100\r\n"
"attribute highp vec3 position, normal;"
"attribute highp vec2 texcoord;"

"uniform highp mat4 projection, view, world;"
"uniform highp mat4 LightMatrix;"

"varying highp vec2 tcoord;"
"varying highp vec3 oP; /* surface position in object space */"
"varying highp vec3 oE; /* position of the eye in object space */"
"varying highp vec3 oI; /* incident ray direction in object space */"
"varying lowp float light;"


"mat4 InverseMatrix( mat4 A ) {"

"	float s0 = A[0][0] * A[1][1] - A[1][0] * A[0][1];"
"	float s1 = A[0][0] * A[1][2] - A[1][0] * A[0][2];"
"	float s2 = A[0][0] * A[1][3] - A[1][0] * A[0][3];"
"	float s3 = A[0][1] * A[1][2] - A[1][1] * A[0][2];"
"	float s4 = A[0][1] * A[1][3] - A[1][1] * A[0][3];"
"	float s5 = A[0][2] * A[1][3] - A[1][2] * A[0][3];"
		     
"	float c5 = A[2][2] * A[3][3] - A[3][2] * A[2][3];"
"	float c4 = A[2][1] * A[3][3] - A[3][1] * A[2][3];"
"	float c3 = A[2][1] * A[3][2] - A[3][1] * A[2][2];"
"	float c2 = A[2][0] * A[3][3] - A[3][0] * A[2][3];"
"	float c1 = A[2][0] * A[3][2] - A[3][0] * A[2][2];"
"	float c0 = A[2][0] * A[3][1] - A[3][0] * A[2][1];"
		     
"	float invdet = 1.0 / (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);"
		     
"	mat4 B;"
		     
"	B[0][0] = ( A[1][1] * c5 - A[1][2] * c4 + A[1][3] * c3) * invdet;"
"	B[0][1] = (-A[0][1] * c5 + A[0][2] * c4 - A[0][3] * c3) * invdet;"
"	B[0][2] = ( A[3][1] * s5 - A[3][2] * s4 + A[3][3] * s3) * invdet;"
"	B[0][3] = (-A[2][1] * s5 + A[2][2] * s4 - A[2][3] * s3) * invdet;"
		     
"	B[1][0] = (-A[1][0] * c5 + A[1][2] * c2 - A[1][3] * c1) * invdet;"
"	B[1][1] = ( A[0][0] * c5 - A[0][2] * c2 + A[0][3] * c1) * invdet;"
"	B[1][2] = (-A[3][0] * s5 + A[3][2] * s2 - A[3][3] * s1) * invdet;"
"	B[1][3] = ( A[2][0] * s5 - A[2][2] * s2 + A[2][3] * s1) * invdet;"
		     
"	B[2][0] = ( A[1][0] * c4 - A[1][1] * c2 + A[1][3] * c0) * invdet;"
"	B[2][1] = (-A[0][0] * c4 + A[0][1] * c2 - A[0][3] * c0) * invdet;"
"	B[2][2] = ( A[3][0] * s4 - A[3][1] * s2 + A[3][3] * s0) * invdet;"
"	B[2][3] = (-A[2][0] * s4 + A[2][1] * s2 - A[2][3] * s0) * invdet;"
		     
"	B[3][0] = (-A[1][0] * c3 + A[1][1] * c1 - A[1][2] * c0) * invdet;"
"	B[3][1] = ( A[0][0] * c3 - A[0][1] * c1 + A[0][2] * c0) * invdet;"
"	B[3][2] = (-A[3][0] * s3 + A[3][1] * s1 - A[3][2] * s0) * invdet;"
"	B[3][3] = ( A[2][0] * s3 - A[2][1] * s1 + A[2][2] * s0) * invdet;"
		     
"	return B;"
"}"


"void main(void) {"
"	highp mat4 modelViewMatrixInverse = InverseMatrix( view*world );"

"	/* surface position in object space */"
"	oP = position;"

"	/* position of the eye in object space */"
"	oE = modelViewMatrixInverse[3].xyz;"

"	/* incident ray direction in object space */"
"	oI = oP - oE; "

"	highp vec4 lightdir = LightMatrix*vec4(0.0, 0.0, 1.0,0.0);"
"	highp vec3 directionalVector = normalize(vec3(lightdir*world));"
"	light=clamp(dot(normal, directionalVector), 0.0, 1.0);"

"	gl_Position = projection * view * world * vec4(position, 1.0);"
"	tcoord=texcoord;"
"}";


const char *interior_frag_shader=
"#version 100\r\n"

"varying highp vec2 tcoord;"

"varying highp vec3 oP; /* surface position in object space */"
"varying highp vec3 oE; /* position of the eye in object space */"
"varying highp vec3 oI; /* incident ray direction in object space */"

"varying lowp float light;"

"uniform highp vec3 wallFreq;"

"uniform highp float wallsBias;"

"uniform lowp float ambient, diffuse;"


"uniform sampler2D FloorTexture;"
"uniform sampler2D CeilingTexture;"
"uniform sampler2D WallXYTexture;"
"uniform sampler2D WallZYTexture;"

"uniform sampler2D Exterior;"

"void main(void) {"
"	highp vec3 wallFrequencies = wallFreq / 2.0 - wallsBias;"

"	/*calculate wall locations*/"
"	highp vec3 walls = ( floor( oP * wallFrequencies) + step( vec3( 0.0 ), oI )) / wallFrequencies;"

"	/*how much of the ray is needed to get from the oE to each of the walls*/"
"	highp vec3 rayFractions = ( walls - oE) / oI;"

"	highp vec2 intersectionXY = (oE + rayFractions.z * oI).xy;"
"	highp vec2 intersectionXZ = (oE + rayFractions.y * oI).xz;"
"	highp vec2 intersectionZY = (oE + rayFractions.x * oI).zy;"

"	/*use the intersection as the texture coordinates for the ceiling */"
"	highp vec3 ceilingColour = texture2D(CeilingTexture, intersectionXZ).rgb;"
"	highp vec3 floorColour = texture2D(FloorTexture, intersectionXZ).rgb;"
"	highp vec3 verticalColour = mix(floorColour, ceilingColour, step(0.0, oI.y));"
"	highp vec3 wallXYColour = texture2D(WallXYTexture, intersectionXY).rgb;"
"	highp vec3 wallZYColour = texture2D(WallZYTexture, intersectionZY).rgb;"

"	/* intersect walls */"
"	highp float xVSz = step(rayFractions.x, rayFractions.z);"
"	highp vec3 interiorColour = mix(wallXYColour, wallZYColour, xVSz);"
"	highp float rayFraction_xVSz = mix(rayFractions.z, rayFractions.x, xVSz);"
"	highp float xzVSy = step(rayFraction_xVSz, rayFractions.y);"

"	interiorColour = mix(verticalColour, interiorColour, xzVSy);"
"	highp vec4 exteriorColour = texture2D(Exterior, tcoord);"
"	lowp float tlight=light*diffuse+ambient;"

"	gl_FragColor.rgb=mix(interiorColour, exteriorColour.rgb*tlight, exteriorColour.a);"
"}";

extern "C" Shader* CreateInterior(float XFreq, float YFreq, float ZFreq, Entity* light){
	Shader* interior=CreateShader((char*)"interior", (char*)interior_vert_shader, (char*)interior_frag_shader);
	UseSurface (interior, (char*)"position",0,1);
	UseSurface (interior, (char*)"normal",0,4);
	UseSurface (interior, (char*)"texcoord",0,2);
	UseMatrix (interior, (char*)"projection",2);
	UseMatrix (interior, (char*)"view",1);
	UseMatrix (interior, (char*)"world",0);
	SetInteger (interior, (char*)"Exterior",0);
	SetInteger (interior, (char*)"FloorTexture",1);
	SetInteger (interior, (char*)"CeilingTexture",2);
	SetInteger (interior, (char*)"WallXYTexture",3);
	SetInteger (interior, (char*)"WallZYTexture",4);
	UseEntity (interior, (char*)"LightMatrix",light,0);
	SetFloat(interior, (char*)"ambient",.5);
	SetFloat(interior, (char*)"diffuse",.5);

	SetFloat3(interior, (char*)"wallFreq",XFreq,YFreq,ZFreq);
	SetFloat(interior, (char*)"wallsBias",0.01);

	return interior;
}
