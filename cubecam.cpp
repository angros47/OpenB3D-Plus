#include "OpenB3D.h"

extern "C" Entity* CreateCubeCamera(int res, Entity* parent){
	Entity* cubecam=CreatePivot(parent);

	Camera* cam;

	cam=CreateCamera(cubecam);
	CameraViewport (cam,0,0, res, res);
	HideEntity(cam);

	cam=CreateCamera(cubecam);
	CameraViewport (cam,0,0, res, res);
	RotateEntity(cam,0,90,0,0);
	HideEntity(cam);

	cam=CreateCamera(cubecam);
	CameraViewport (cam,0,0, res, res);
	RotateEntity(cam,0,180,0,0);
	HideEntity(cam);

	cam=CreateCamera(cubecam);
	CameraViewport (cam,0,0, res, res);
	RotateEntity(cam,0,270,0,0);
	HideEntity(cam);

	cam=CreateCamera(cubecam);
	CameraViewport (cam,0,0, res, res);
	RotateEntity(cam,90,0,0,0); 
	HideEntity(cam);

	cam=CreateCamera(cubecam);
	CameraViewport (cam,0,0, res, res);
	RotateEntity(cam,-90,0,0,0);
	HideEntity(cam);

	return cubecam;
}

extern "C" void CubeCameraToTex(Texture* tex, Camera* camera){
	SetCubeFace(tex,0);
	CameraToTex(tex, (Camera*)GetChild(camera,2),0);
	SetCubeFace(tex,1);
	CameraToTex(tex, (Camera*)GetChild(camera,1),0);
	SetCubeFace(tex,2);
	CameraToTex(tex, (Camera*)GetChild(camera,4),0);
	SetCubeFace(tex,3);
	CameraToTex(tex, (Camera*)GetChild(camera,3),0);
	SetCubeFace(tex,4);
	CameraToTex(tex, (Camera*)GetChild(camera,5),0);
	SetCubeFace(tex,5);
	CameraToTex(tex, (Camera*)GetChild(camera,6),0);
}
