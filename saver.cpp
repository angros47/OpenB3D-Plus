#include "OpenB3D.h"
#include <stdio.h>

#ifdef linux
#include <GL/gl.h>
#endif

#ifdef WIN32
#include <gl\GLee.h>
#endif

void jo_write_mpeg(FILE *fp, const unsigned char *rgbx, int width, int height, int fps);

class Global{
public:
	static int width,height,mode,depth,rate;
};

struct VideoFile{
	FILE* fp;
	unsigned char* buffer;
	int fps;
};


extern "C" VideoFile* SaveVideo(char* name, int fps){
	VideoFile* file=new VideoFile;

	file->buffer = new unsigned char[Global::width*Global::height*4];
	file->fp = fopen(name, "wb");
	file->fps = fps;
	return file;
}


extern "C" void SaveFrame(VideoFile* file){
	glReadPixels(0, 0, Global::width, Global::height, GL_RGBA, GL_UNSIGNED_BYTE, file->buffer);

	int h=Global::height/2;
	int step=Global::width*4;
	unsigned char* top;
	unsigned char* bottom;
	top=file->buffer;
	bottom=&file->buffer[Global::width*Global::height*4];
	for (int y=0; y<h; y++){
		bottom-=step;
		for (int x=0; x<step; x++){
			unsigned char temp;
			temp=*bottom;
			*bottom=*top;
			*top=temp;
			bottom++; top++;
		}
		bottom-=step;
	}

	jo_write_mpeg(file->fp, file->buffer, Global::width, Global::height, file->fps);
}

extern "C" void CloseVideo(VideoFile* file){
	fclose(file->fp);
	delete file->buffer;
	delete file;
}
