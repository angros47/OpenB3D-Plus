const char *gaussian_vert_shader=
"#version 120\r\n"

"varying vec3 Position;"
"varying vec3 Normal;"
"varying vec2 TexCoord;"


"void main()"
"{"
"	TexCoord = gl_MultiTexCoord0.st;"
"	Normal = normalize(gl_NormalMatrix * gl_Normal);"
"	Position = vec3(gl_ModelViewMatrix * gl_Vertex);"
	
"	gl_Position = ftransform();"
"}";


const char *gaussian_frag_shader=
"#version 120\r\n"

"uniform int PassNum;"

"uniform sampler2D RenderTex;"

"uniform int Width;"
"uniform int Height;"

"varying vec3 Position;"
"varying vec3 Normal;"
"varying vec2 TexCoord;"

"float PixOffset[7] = float[] (0.0,1.5,3.0,4.5,6.0,7.5,9.0);"
"float Weight[5] = float[] (0.05,0.14,0.2,0.14,0.05);"

"vec4 Pass1()"
"{"
"	float dy = 1.0 / float(Height);"
"	vec4 sum = texture2D(RenderTex,TexCoord) * Weight[0];"
"	for ( int i = 1; i < 5; i++)"
"	{"
"		sum += texture2D(RenderTex, TexCoord + vec2(0.0,PixOffset[i]) * dy ) * Weight[i];"
"		sum += texture2D(RenderTex, TexCoord - vec2(0.0,PixOffset[i]) * dy ) * Weight[i];"
"	}"
"	return sum;"
"}"

"vec4 Pass2()"
"{"
"	float dx = 1.0 / float(Width);"
"	vec4 sum = texture2D(RenderTex, TexCoord) * Weight[0];"
"	for ( int i = 1; i < 5; i++)"
"	{"
"		sum += texture2D(RenderTex, TexCoord + vec2(PixOffset[i],0.0) * dx ) * Weight[i];"
"		sum += texture2D(RenderTex, TexCoord - vec2(PixOffset[i],0.0) * dx ) * Weight[i];"
"	}"
"	return sum;"
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
