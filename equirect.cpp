#include "OpenB3D.h"
#include <stdio.h>
#include <iostream>

#ifdef linux
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#ifdef WIN32
#include <gl\GLee.h>
#endif

void jo_write_mpeg(FILE *fp, const unsigned char *rgbx, int width, int height, int fps);

class Texture{
public:
	unsigned int texture;
};

class Shader {
public:
	void ProgramAttriBegin();
	void ProgramAttriEnd();
};


struct CubeVideoFile{
	FILE* fp;
	unsigned char* buffer;
	int fps;
	int width, height;
	unsigned int texture, FBO;
};




Shader* CubemapToEquirectandular=NULL;

const char *CubemapToEquirectandular_vert_shader=
"#version 120\r\n"
"attribute vec2 position, text;"
"varying vec2 tcoord;"
"void main(void) {"
"	gl_Position = ftransform();"
"	tcoord=text;"
"}";

const char *CubemapToEquirectandular_frag_shader=
"#version 120\r\n"
"uniform samplerCube tCube;"
"varying vec2 tcoord;"
"void main(void) {"
"	vec2 thetaphi = ((tcoord * 2.0) - vec2(1.0)) * vec2(3.1415926535897932384626433832795, 1.5707963267948966192313216916398);"
"	vec3 rayDirection = vec3(cos(thetaphi.y) * cos(thetaphi.x), sin(thetaphi.y), cos(thetaphi.y) * sin(thetaphi.x));"
"	gl_FragColor=textureCube(tCube, rayDirection);"
"}";



extern "C" CubeVideoFile* Save360Video(char* name, int width, int height, int fps){
	CubeVideoFile* file=new CubeVideoFile;

	file->buffer = new unsigned char[width*height*4];
	file->fp = fopen(name, "wb");
	file->fps = fps;
	file->width = width;
	file->height = height;

	glGenTextures(1, &file->texture);
	glGenFramebuffers(1, &file->FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, file->FBO);
	glBindTexture (GL_TEXTURE_2D, file->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, file->width, file->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, file->texture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return file;
}




extern "C" void Save360Frame(CubeVideoFile* file, Texture* cubemap){

	if (CubemapToEquirectandular==NULL){
		CubemapToEquirectandular=CreateShader((char*)"Equirect", 
			(char*)CubemapToEquirectandular_vert_shader, 
			(char*)CubemapToEquirectandular_frag_shader);
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, file->FBO);
	glViewport(0,0,file->width,file->height);
	glScissor(0,0,file->width,file->height);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	glBindBuffer(GL_ARRAY_BUFFER,0); 

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glLoadIdentity();
	glOrtho(0 , 1 , 1 , 0 , 0 , 1);

	glDisable(GL_DEPTH_TEST);
	glColor4f(1.0, 1.0, 1.0, 1.0);

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_BLEND);

	CubemapToEquirectandular->ProgramAttriBegin();

	GLfloat q3[] = {0,0,0,1,1,1,1,0};
	GLfloat qv[] = {0,1,0,0,1,0,1,1};
	 
	glVertexPointer(2, GL_FLOAT, 0, q3);
	glTexCoordPointer(2, GL_FLOAT, 0, qv);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, q3);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, qv);
	glEnableVertexAttribArray(1);


	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture (GL_TEXTURE_CUBE_MAP, cubemap->texture);


	glDrawArrays(GL_TRIANGLE_FAN,0,4);

	glDisable(GL_TEXTURE_CUBE_MAP);

	CubemapToEquirectandular->ProgramAttriEnd();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();


	glPopClientAttrib();
	glPopAttrib();

	glReadPixels(0, 0, file->width, file->height, GL_RGBA, GL_UNSIGNED_BYTE, file->buffer);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	jo_write_mpeg(file->fp, file->buffer, file->width, file->height, file->fps);

}

extern "C" void Close360Video(CubeVideoFile* file){
	fclose(file->fp);
	delete file->buffer;
	delete file;
}
