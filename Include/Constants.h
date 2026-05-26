#ifndef _CONSTANTS_H
#define _CONSTANTS_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus   
#include <cfloat>
#else
#include <float.h>
#endif
#include <Config.h>

#ifdef NULL
#undef NULL
//#define NULL 0
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif






/**
 * WARNINGS SECTION:
 */
#ifdef _MSC_VER /* Quiet a bunch of MSVC warnings... */
#  pragma warning(disable:4786) /* visual c6.0 compiler */
#  pragma warning(disable:4251)/* for std:: member variable to have dll interface */
#  pragma warning(disable:4275) /* for std:: base class to have dll interface */
#  pragma warning(disable:4800) /* int forcing value to bool */
#  pragma warning(disable:4244) /* conversion, possible loss of data */
#endif
#if defined(__GNUC__)
#   define AI3D_DEPRECATE_METHOD(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#   define AI3D_DEPRECATE_METHOD(func)  __declspec(deprecated) func 
#else
#   define AI3D_DEPRECATE_METHOD(func)
#endif
   
/**
 * DLL IMPORT/EXORT SECTION
 */
#if defined(AI3D_STATIC)
#  define AI3DEXPORT
#  define AI3DIMPORT
#  define AI3DDLLEXPORT
#  define AI3D_DLL
#  define AI3DDLLEXPORT_DATA(type) type
#  define AI3D_DLL_DATA(type) type
#  define AI3DDLLEXPORT_CTORFN 
#elif defined(__MINGW32__) || defined(__CYGWIN__) || defined(_MSC_VER) || defined(__VISUALC__) || defined(__BORLANDC__) || defined(__WATCOMC__)
#  define AI3DEXPORT __declspec(dllexport)
#  define AI3DIMPORT __declspec(dllimport)
#  ifdef AI3DMAKINGDLL
#    define AI3DDLLEXPORT AI3DEXPORT
#    define AI3D_DLL       AI3DEXPORT
#    define AI3DDLLEXPORT_DATA(type) AI3DEXPORT type
#    define AI3D_DLL_DATA(type) AI3DEXPORT type
#    define AI3DDLLEXPORT_CTORFN
#  else
#    define AI3DDLLEXPORT AI3DIMPORT
#    define AI3D_DLL      AI3DIMPORT
#    define AI3DDLLEXPORT_DATA(type) AI3DIMPORT type
#    define AI3D_DLL_DATA(type) AI3DIMPORT type
#    define AI3DDLLEXPORT_CTORFN
#  endif
#else /* not #if defined(_MSC_VER) */
#  define AI3DEXPORT
#  define AI3DIMPORT
#  define AI3DDLLEXPORT
#  define AI3D_DLL
#  define AI3DDLLEXPORT_DATA(type) type
#  define AI3D_DLL_DATA(type) type
#  define AI3DDLLEXPORT_CTORFN
#endif /* #if defined(_MSC_VER) */


   
/**
 * Previous DLL import export section.  Commented out, but left just in case.
 */
#if 0 /* Comment out ALL this mess! */
#if defined(_MSC_VER) || defined(__VISUALC__) || defined(__BORLANDC__) || defined(__GNUC__) || defined(__WATCOMC__)
#  if (_MSC_VER >= 1300)	/* MSVC .NET 2003 version */
#    define AI3DEXPORT __declspec(dllexport)
#    define AI3DIMPORT __declspec(dllimport)
#  else
#    define AI3DEXPORT __declspec(dllexport)
#    define AI3DIMPORT __declspec(dllimport)
#  endif
#else /* compiler doesn't support __declspec() */
#   define AI3DEXPORT
#   define AI3DIMPORT
#endif

#if defined(__WXPM__)
#  if defined (__WATCOMC__)
#    define AI3DEXPORT __declspec(dllexport)
 /*
   __declspec(dllimport) prepends __imp to imported symbols. We do NOT
   want that!
 */
#    define AI3DIMPORT
#  elif (!(defined(__VISAGECPP__) && (__IBMCPP__ < 400 || __IBMC__ < 400 )))
#    define AI3DEXPORT _Export
#    define AI3DIMPORT _Export
#  endif
#elif defined(__AI3DMAC__)
#  ifdef __MWERKS__
#    define AI3DEXPORT __declspec(export)
#    define AI3DIMPORT __declspec(import)
#  endif
#endif

#if defined(_MSC_VER)
#  pragma warning(disable:4786) /* visual c6.0 compiler */
#  pragma warning(disable:4251) /* for std:: member variable to have dll interface */
#  pragma warning(disable:4275) /* for std:: base class to have dll interface */
#  pragma warning(disable:4800) /* int forcing value to bool */
#  pragma warning(disable:4244) /* conversion, possible loss of data */

#  ifdef AI3DSINGLEDLL /* one gigantic dll, all declared export */
#    ifdef EXPORT_OSMMATRIX
#      define EXPORT_OSMMATRIX  AI3DEXPORT
#    endif
#    ifndef AI3DDLLEXPORT
#      define AI3DDLLEXPORT    AI3DEXPORT
#    endif
#    ifdef EXPORT_OSMELEV
#      define EXPORT_OSMELEV    AI3DEXPORT
#    endif
#    ifdef EXPORT_OSMSPTDATA
#      define EXPORT_OSMSPTDATA AI3DEXPORT
#    endif
#    ifdef EXPORT_ISO8211
#      define EXPORT_ISO8211    AI3DEXPORT
#    endif
#    ifdef EXPORT_OSMPROJ
#      define EXPORT_OSMPROJ    AI3DEXPORT
#    endif
#    ifndef EXPORT_OSMIMAGING
#      define EXPORT_OSMIMAGING AI3DEXPORT
#    endif
#  else /* individual dlls, each with their own import/export symbols */
#    ifndef EXPORT_OSMMATRIX
#      ifdef OSMMATRIX_EXPORTS
#        define EXPORT_OSMMATRIX AI3DEXPORT
#      else
#        define EXPORT_OSMMATRIX AI3DIMPORT
#      endif
#    endif
#    ifndef AI3DDLLEXPORT
#      ifdef OSMBASE_EXPORTS
#        define AI3DDLLEXPORT AI3DEXPORT
#      else
#        define AI3DDLLEXPORT AI3DIMPORT
#      endif
#    endif
#    ifndef EXPORT_OSMELEV
#      ifdef OSMELEV_EXPORTS
#        define EXPORT_OSMELEV AI3DEXPORT
#      else
#        define EXPORT_OSMELEV AI3DIMPORT
#      endif
#    endif
#    ifndef EXPORT_OSMSPTDATA
#      ifdef OSMSPTDATA_EXPORTS
#        define EXPORT_OSMSPTDATA AI3DEXPORT
#      else
#        define EXPORT_OSMSPTDATA AI3DIMPORT
#      endif
#    endif
#    ifndef EXPORT_OSMPROJ
#      ifdef OSMPROJ_EXPORTS
#        define EXPORT_OSMPROJ AI3DEXPORT
#      else
#        define EXPORT_OSMPROJ AI3DIMPORT
#      endif
#    endif
#    ifndef EXPORT_ISO8211
#      ifdef ISO8211_EXPORTS
#        define EXPORT_ISO8211 AI3DEXPORT
#      else
#        define EXPORT_ISO8211 AI3DIMPORT
#      endif
#    endif
#    ifndef EXPORT_OSMIMAGING
#      ifdef OSMIMAGING_EXPORTS
#        define EXPORT_OSMIMAGING AI3DEXPORT
#      else
#        define EXPORT_OSMIMAGING AI3DIMPORT
#      endif
#    endif
#  endif
#else /* defined(_MSC_VER) */
#  ifdef AI3DMAKINGDLL
#    define AI3DDLLEXPORT AI3DEXPORT
#    define AI3D_DLL       AI3DEXPORT
#    define AI3DDLLEXPORT_DATA(type) AI3DEXPORT type
#    define AI3D_DLL_DATA(type) AI3DEXPORT type
#    define AI3DDLLEXPORT_CTORFN
#  elif defined(AI3DUSINGDLL)
#    define AI3DDLLEXPORT AI3DIMPORT
#    define AI3D_DLL      AI3DIMPORT
#    define AI3DDLLEXPORT_DATA(type) AI3DIMPORT type
#    define AI3D_DLL_DATA(type) AI3DIMPORT type
#    define AI3DDLLEXPORT_CTORFN
#  else /* not making nor using DLL */
#    define AI3DDLLEXPORT
#    define AI3D_DLL
#    define AI3D_DLL_DATA(type) type
#    define AI3DDLLEXPORT_DATA(type) type
#    define AI3DDLLEXPORT_CTORFN
#  endif
#  define EXPORT_OSMMATRIX  AI3DDLLEXPORT
#  define EXPORT_OSMELEV    AI3DDLLEXPORT
#  define EXPORT_OSMSPTDATA AI3DDLLEXPORT
#  define EXPORT_OSMPROJ    AI3DDLLEXPORT
#  define EXPORT_OSMIMAGING AI3DDLLEXPORT
#endif
#endif /* End of commented out "#if 0" old DLL import/export section. */

#ifdef AI3D_MAKE_DLL
	#define AI3D_API AI3DEXPORT 
	#define GUI_API  
#else
	#define AI3D_API AI3DIMPORT
	#define GUI_API 
#endif


#if (!defined(_MSC_VER))
#  ifndef ltoa
#    define ltoa(l,s,len) sprintf(s,"%d",l)
#  endif
#  ifndef itoa
#    define itoa(i,s,len) sprintf(s,"%d",i)
#  endif 
#endif

typedef char                   ai3d_int8;
typedef unsigned char          ai3d_uint8;
typedef signed char            ai3d_sint8;

typedef short                  ai3d_int16;
typedef unsigned short         ai3d_uint16;
typedef signed short           ai3d_sint16;

typedef int                    ai3d_int32;
typedef unsigned int           ai3d_uint32;
typedef signed int             ai3d_sint32;

typedef float                  ai3d_float32;
typedef double                 ai3d_float64;


typedef long long              ai3d_int64;
typedef unsigned long long     ai3d_uint64;
typedef signed long long       ai3d_sint64;

typedef ai3d_int32            EsErrorCode;

typedef unsigned char         BYTE;




   
#ifndef M_PI
#  define M_PI             ((ai3d_float64)3.141592653589793238462643)
#endif
#ifndef TWO_PI
#  define TWO_PI           ((ai3d_float64)(2.0 * M_PI))
#endif
#ifndef DEG_PER_RAD
#  define DEG_PER_RAD      ((ai3d_float64)(180.0/M_PI))
#endif
#ifndef SEC_PER_RAD
#  define SEC_PER_RAD      ((ai3d_float64)206264.8062471)
#endif
#ifndef RAD_PER_DEG
#  define RAD_PER_DEG      ((ai3d_float64)(M_PI/180.0))
#endif
#ifndef MTRS_PER_FT
#  define MTRS_PER_FT      ((ai3d_float64)0.3048)
#endif
#ifndef FT_PER_MTRS
#  define FT_PER_MTRS      ((ai3d_float64)3.280839895013122)
#endif
#ifndef FT_PER_MILE
#  define FT_PER_MILE      ((ai3d_float64)5280.0)
#endif
#ifndef SQRT_TWO_PI
#  define SQRT_TWO_PI      ((ai3d_float64)2.50662827463100050242)
#endif
#ifndef SQRT_TWO_PI_DIV2
#  define SQRT_TWO_PI_DIV2 ((ai3d_float64)7.07106781186547524401E-1)
#endif


#ifdef __cplusplus
}
#endif

#endif //_CONSTANTS_H
