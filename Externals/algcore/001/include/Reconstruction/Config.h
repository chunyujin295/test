#pragma once

#ifdef WIN32
#ifdef S3D_RECONSTRUCTION_EXPORT
#define S3D_RECONSTRUCTION_API __declspec(dllexport)
#else
#define S3D_RECONSTRUCTION_API __declspec(dllimport)
#endif
#else
#define S3D_RECONSTRUCTION_API
#endif

#ifdef WIN32
#ifdef S3D_COMMON_EXPORT
#define S3D_COMMON_API __declspec(dllexport)
#else
#define S3D_COMMON_API __declspec(dllimport)
#endif
#else
#define S3D_COMMON_API
#endif

#ifdef WIN32
#ifdef S3D_ORTHO2D_EXPORT
#define S3D_ORTHO2D_API __declspec(dllexport)
#else
#define S3D_ORTHO2D_API __declspec(dllimport)
#endif
#else
#define S3D_ORTHO2D_EXPORT
#endif