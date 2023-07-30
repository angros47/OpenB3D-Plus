CCMPILER = g++
CC=g++ -fPIC -Wall -O3 -L./

OBJECTS = bump.o cubecam.o fur.o glass.o interior.o matcap.o shadow.o toon.o pbr.o gaussian.o dof.o fisheye.o hdr.o pixel.o ssao.o deferredLights.o equirect.o jo_mpeg.o saver.o

main:$(OBJECTS) 
ifeq ($(shell uname), Linux)
	$(CC) -shared -o libOB3Dplus.so $(OBJECTS)  -lGLU -lOpenB3D
else
	$(CC) -shared -o ob3dPlus.dll $(OBJECTS) -lOpenB3D -lGLee -lopengl32 -lglu32 -lGDI32 -static
endif


bump.o  : bump.cpp 
	$(CC) -c bump.cpp -o bump.o

cubecam.o  : cubecam.cpp
	$(CC) -c cubecam.cpp -o cubecam.o

fur.o  : fur.cpp 
	$(CC) -c fur.cpp -o fur.o

glass.o  : glass.cpp 
	$(CC) -c glass.cpp -o glass.o

interior.o  : interior.cpp 
	$(CC) -c interior.cpp -o interior.o

matcap.o  : matcap.cpp 
	$(CC) -c matcap.cpp -o matcap.o

shadow.o  : shadow.cpp 
	$(CC) -c shadow.cpp -o shadow.o

toon.o  : toon.cpp 
	$(CC) -c toon.cpp -o toon.o

pbr.o  : pbr.cpp 
	$(CC) -c pbr.cpp -o pbr.o

gaussian.o  : gaussian.cpp
	$(CC) -c gaussian.cpp -o gaussian.o

dof.o  : dof.cpp
	$(CC) -c dof.cpp -o dof.o

fisheye.o  : fisheye.cpp
	$(CC) -c fisheye.cpp -o fisheye.o

hdr.o  : hdr.cpp
	$(CC) -c hdr.cpp -o hdr.o

pixel.o  : pixel.cpp
	$(CC) -c pixel.cpp -o pixel.o

ssao.o  : ssao.cpp
	$(CC) -c ssao.cpp -o ssao.o

deferredLights.o	: deferredLights.cpp
	$(CC) -c deferredLights.cpp -o deferredLights.o


equirect.o  : equirect.cpp
	$(CC) -c equirect.cpp -o equirect.o

jo_mpeg.o  : jo_mpeg.cpp 
	$(CC) -c jo_mpeg.cpp -o jo_mpeg.o

saver.o  : saver.cpp 
	$(CC) -c saver.cpp -o saver.o


