#include "openb3d.bi"

#inclib "OB3Dplus"
declare function CreateBump cdecl alias "CreateBump" (byval light as any ptr, byval show_diffuse as integer=1, byval mode as integer=4) as any ptr
declare function CreateFur cdecl alias "CreateFur" (byval light as any ptr, byval tex as any ptr=0) as any ptr
declare function CreateGlass cdecl alias "CreateGlass" () as any ptr
declare function CreateInterior cdecl alias "CreateInterior" (byval XFreq as single, byval YFreq as single, byval ZFreq as single, byval light as any ptr) as any ptr
declare function CreateMatcap cdecl alias "CreateMatcap" (byval mode as integer=0) as any ptr
declare function CreateRamp cdecl alias "CreateRamp" (byval steps as integer, byval r as single, byval g as single, byval b as single, byval r1 as single, byval g1 as single, byval b1 as single ) as any ptr
declare function CreateToon cdecl alias "CreateToon" (byval light as any ptr) as any ptr

declare function CameraBloom cdecl alias "CameraBloom" (byval cam as any ptr, byval exposure as single, byval cutoff as single) as any ptr
declare function CameraDOF cdecl alias "CameraDOF" (byval cam as any ptr, byval aperture as single=.5) as any ptr
declare function CameraFisheye cdecl alias "CameraFisheye" (byval cam as any ptr, byval aperture as integer=178) as any ptr
declare function CameraHDR cdecl alias "CameraHDR" (byval cam as any ptr) as any ptr
declare function CameraPixelArt cdecl alias "CameraPixelArt" (byval cam as any ptr, byval width as integer, byval height as integer, byval colors as integer, byval dithering as integer) as any ptr
declare function CameraSSAO cdecl alias "CameraSSAO" (byval cam as any ptr, byval Strength as single=1.5, byval sbase as single=0.75, byval area as single=0.025, byval falloff as single=0.000001, byval radius as single=0.025) as any ptr

declare function AddDeferredLight cdecl alias "AddDeferredLight" (byval light_type as integer, byval ent as any ptr) as any ptr
declare function CameraDeferred cdecl alias "CameraDeferred" (byval cam as any ptr) as any ptr
declare function DeferredAmbientLight cdecl alias "DeferredAmbientLight" (byval r as single, byval g as single, byval b as single) as integer
declare function DeferredLightAttenuation cdecl alias "DeferredLightAttenuation" (byval light as any ptr, byval constant as single, byval linear as single, byval quadratic as single) as integer
declare function DeferredLightColor cdecl alias "DeferredLightColor" (byval light as any ptr, byval red as single, byval green as single, byval blue as single) as integer
declare sub FreeDeferredLight cdecl alias "FreeDeferredLight" (byval light as any ptr) 


declare function CubeCameraToTex cdecl alias "CubeCameraToTex" (byval tex as any ptr, byval cam as any ptr) as any ptr
declare function CreateCubeCamera cdecl alias "CreateCubeCamera" (byval res as integer, byval parent as any ptr=0) as any ptr


declare function SaveVideo cdecl alias "SaveVideo" (byval name as zstring ptr, byval fps as integer=30) as any ptr
declare sub SaveFrame cdecl alias "SaveFrame" (byval file as any ptr)
declare sub CloseVideo cdecl alias "CloseVideo" (byval file as any ptr)

declare function Save360Video cdecl alias "Save360Video" (byval name as zstring ptr, byval width as integer, byval height as integer, byval fps as integer=30) as any ptr
declare sub Save360Frame cdecl alias "Save360Frame" (byval file as any ptr, byval tex as any ptr)
declare sub Close360Video cdecl alias "Close360Video" (byval file as any ptr)


type PBRTexture
	basecolor as any ptr
	normals as any ptr
	PBR as any ptr
	EMI as any ptr
end type


declare sub AmbientCubeTexture cdecl alias "AmbientCubeTexture" (byval tex as any ptr) 
declare sub EntityPBRTexture cdecl alias "EntityPBRTexture" (byval ent as any ptr, byval tex as any ptr) 
declare function CreatePBR cdecl alias "CreatePBR" (t as any ptr=0) as any ptr
declare function LoadPBRTexture cdecl alias "LoadPBRTexture" (BaseColor as zstring ptr, Normals as zstring ptr, Heights as zstring ptr, Roughness as zstring ptr, Metallic as zstring ptr, AO as zstring ptr, Emissive as zstring ptr) as any ptr
