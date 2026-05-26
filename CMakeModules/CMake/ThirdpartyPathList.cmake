#set(TP_ROOT "E:/Gitlab_Project/third_party/Windows/vc141/")#$ENV{DEPS_DIR_VC14})
set(TP_ROOT $ENV{DEPS_DIR_VC142})
set(TP_ROOT_141 $ENV{DEPS_DIR_VC142}/../vc141)
#


set(COMPILER_USED vc142)
set(AI3D_ARCHITECTURE ${AI3D_PLATFORM_NAME})


#chy 宏目前linux只有release版本用于控制 是否只使用release版本
if(UNIX)
	set(AI3D_USE_DEPS_FAST ON CACHE BOOL "Enable use third libraries's release version .")
endif(UNIX)

if(WIN32)
	set(AI3D_USE_DEPS_FAST OFF CACHE BOOL "Enable use third libraries's release version .")
endif(WIN32)


#chy 因不同平台的库的后缀名不一样，所以在此设置
set(LIB_POSTFIX "")
set(BIN_POSTFIX "")
if(WIN32)
	set(LIB_POSTFIX .lib)
	set(BIN_POSTFIX .dll)
elseif(UNIX)
	set(LIB_POSTFIX .a)
endif(WIN32)









#chy 以下为各个第三方库的存放路径， 每增加一个库，可往后添加
#本地库
	#
SET(USE_AI3DCORE ON)	
IF(USE_AI3DCORE)
	set(AI3DCORE_VERSION 001)
	set(AI3DCORE_ROOT_DIR     ${AI3D_THIRDPARTY_DIR}/algcore/${AI3DCORE_VERSION})
	MESSAGE("---${AI3DCORE_ROOT_DIR}-")
	set(AI3D_CORE_INCLUDE_PATH   ${AI3DCORE_ROOT_DIR}/include)
	if(WIN32)
		
		set(AI3D_CORE_LIBRARY_PATH   ${AI3DCORE_ROOT_DIR}/lib/$(Platform))
	
	set(AI3D_CORE_LIB_DEBUG MoldAIProcd.lib MoldAIBased.lib MoldAISfMd.lib MoldAIACCd.lib)
	set(AI3D_CORE_LIB_RELEASE MoldAIProc.lib MoldAIBase.lib MoldAISfM.lib MoldAIACC.lib)
	message("----${AI3D_CORE_LIB_RELEASE}")
			set(AI3D_CORE_LIB 
			${AI3D_CORE_LIB_RELEASE}#MokReconstruction.lib MokCommon.lib MokSfM.lib MokSimd.lib
				)
				
	
	endif(WIN32)
	function(AI3D_USING_CORE)
			include_directories( ${AI3D_CORE_INCLUDE_PATH} )
			link_directories(    ${AI3D_CORE_LIBRARY_PATH} )
			message("--1--${AI3D_CORE_LIBRARY_PATH}")
	endfunction()
ENDIF(USE_AI3DCORE)

	#SIMPLYGON

SET(USE_SIMPLYGON ON)	
IF(USE_SIMPLYGON)
		set(SIMPLYGON_VERSION )
		set(SIMPLYGON_ROOT_DIR     ${AI3D_THIRDPARTY_DIR}/Simplygon/${SIMPLYGON_VERSION})
		
		set(AI3D__INCLUDE_PATH   ${SIMPLYGON_ROOT_DIR}/include)
		if(WIN32)
			
			set(AI3D_SIMPLYGON_LIBRARY_PATH   ${SIMPLYGON_ROOT_DIR}/$(Platform)/lib)
		
				set(AI3D_SIMPLYGON_LIB )
		
		endif(WIN32)
		function(AI3D_USING_SIMPLYGON)
		
			include_directories( ${AI3D_SIMPLYGON_INCLUDE_PATH} )
			link_directories(    ${AI3D_SIMPLYGON_LIBRARY_PATH} )
		endfunction()
ENDIF(USE_SIMPLYGON)


#基本库
SET(USE_GLOG ON)
IF(USE_GLOG)
	set(GLOG_VERSION 0.4.0)
	set(GLOG_ROOT_DIR           ${TP_ROOT}/glog/${GLOG_VERSION})
	
	set(AI3D_GLOG_INCLUDE_PATH   ${GLOG_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_GLOG_LIBRARY_PATH   ${GLOG_ROOT_DIR}/lib/$(Platform))
		set(AI3D_GLOG_LIB
				debug glogd.lib
				optimized glog.lib )
				
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_GLOG_LIBRARY_PATH   ${GLOG_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			
			set(AI3D_GLOG_LIB glog
	#${LIB_POSTFIX}
	)
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	function(AI3D_USING_GLOG)
		include_directories( ${AI3D_GLOG_INCLUDE_PATH} )
		link_directories(    ${AI3D_GLOG_LIBRARY_PATH} )
		if(POLICY CMP0005)
			cmake_policy(SET CMP0005 NEW)
		endif()
		#add_definitions(-DGOOGLE_GLOG_DLL_DECL=)
		ADD_DEFINITIONS(-DGLOG_NO_ABBREVIATED_SEVERITIES)
	endfunction()
ENDIF(USE_GLOG)

#GFLAGS
SET(USE_GFLAGS ON)
IF(USE_GFLAGS)

	set(GFLAGS_VERSION 2.2.0)
	set(GFLAGS_ROOT_DIR          ${TP_ROOT}/gflags/${GFLAGS_VERSION})
set(AI3D_GFLAGS_INCLUDE_PATH   ${GFLAGS_ROOT_DIR}/include)
#${LIB_POSTFIX}
	if(WIN32)
		set(AI3D_GFLAGS_LIBRARY_PATH   ${GFLAGS_ROOT_DIR}/lib/$(Platform))
		set(AI3D_GFLAGS_LIB
				debug gflags_static.lib
				optimized gflags_static.lib )
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_GFLAGS_LIBRARY_PATH   ${GFLAGS_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			
			set(AI3D_GFLAGS_LIB gflags_static${LIB_POSTFIX} ) #_nothreadslibgflags
		
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	function(AI3D_USING_GFLAGS)
		include_directories( ${AI3D_GFLAGS_INCLUDE_PATH} )
		link_directories(    ${AI3D_GFLAGS_LIBRARY_PATH} )
		if(POLICY CMP0005)
			cmake_policy(SET CMP0005 NEW)
		endif()
		add_definitions(-DAI3D_GFLAGS_NAMESPACE=${GFLGAS_NAMESPACE})
  add_definitions(-DAI3D_GFLAGS_NAMESPACE=${GFLAGS_NAMESPACE})
	endfunction()
ENDIF(USE_GFLAGS)


 #libxml2
 SET(USE_LIBXML2 ON) 
	IF(USE_LIBXML2)
	set(LIBXML2_VERSION )
	set(LIBXML2_ROOT_DIR          ${TP_ROOT}/libxml/${LIBXML2_VERSION})
	set(AI3D_LIBXML2_INCLUDE_PATH   ${LIBXML2_ROOT_DIR}/include)
	if(WIN32)
	
		set(AI3D_LIBXML2_LIBRARY_PATH   ${LIBXML2_ROOT_DIR}/lib/$(Platform))
		set(AI3D_LIBXML2_LIB
				debug libxml2d.lib    
				optimized libxml2.lib 
	)
	endif(WIN32)
	function(AI3D_USING_LIBXML)
		include_directories( ${AI3D_LIBXML2_INCLUDE_PATH} )
		link_directories(    ${AI3D_LIBXML2_LIBRARY_PATH} )		
	endfunction()
	
	ENDIF(USE_LIBXML2)

   #libiconv
 SET(USE_ICONV ON) 
	IF(USE_ICONV)
	set(LIBICONV_VERSION )
	set(LIBICONV_ROOT_DIR          ${TP_ROOT}/libiconv/${LIBICONV_VERSION})
	set(AI3D_LIBICONV_INCLUDE_PATH   ${LIBICONV_ROOT_DIR}/include)
	if(WIN32)
	
		set(AI3D_LIBICONV_LIBRARY_PATH   ${LIBICONV_ROOT_DIR}/lib/$(Platform))
		set(AI3D_LIBICONV_LIB
				debug libiconvd.lib    
				optimized libiconv.lib 
	)
	endif(WIN32)
	function(AI3D_USING_LIBICONV)
		include_directories( ${AI3D_LIBICONV_INCLUDE_PATH} )
		link_directories(    ${AI3D_LIBICONV_LIBRARY_PATH} )		
	endfunction()
	
	ENDIF(USE_ICONV)


#expat
	
	 SET(USE_EXPAT ON) 
	IF(USE_EXPAT)
	set(EXPAT_VERSION 2.4.1)
	set(EXPAT_ROOT_DIR          ${TP_ROOT}/expat/${EXPAT_VERSION})
	set(AI3D_EXPAT_INCLUDE_PATH   ${EXPAT_ROOT_DIR}/include)
	if(WIN32)
	
		set(AI3D_EXPAT_LIBRARY_PATH   ${EXPAT_ROOT_DIR}/lib/$(Platform))
		set(AI3D_EXPAT_LIB
				debug libexpatd.lib    
				optimized libexpat.lib 
	)
	endif(WIN32)
	function(AI3D_USING_EXPAT)
		include_directories( ${AI3D_EXPAT_INCLUDE_PATH} )
		link_directories(    ${AI3D_EXPAT_LIBRARY_PATH} )		
	endfunction()
	
	ENDIF(USE_EXPAT)
# BOOST	
SET(USE_BOOST ON)
IF(USE_BOOST)
set(BOOST_VERSION_STRING 1_71)
	set(BOOST_VERSION 1.71.0)
#set(BOOST_VERSION_STRING 1_90)
#	set(BOOST_VERSION 1.90.0)
	set(BOOST_ROOT_DIR          ${TP_ROOT}/boost/${BOOST_VERSION})

set(AI3D_BOOST_INCLUDE_PATH	${BOOST_ROOT_DIR}/include)
    
if(WIN32)
	
  set(BOOST_FIND_COMPONENTS    iostreams  program_options  
				filesystem  system       regex
				) #chrono
		#serialization		
  
	set(AI3D_BOOST_LIBRARY_PATH   ${BOOST_ROOT_DIR}/lib/$(Platform))
	set(BOOST_USE_STATIC ON)
	set(BOOST_LIB_PREFIX "")
  if(BOOST_USE_STATIC)
  	set(BOOST_LIB_PREFIX lib)
  endif(BOOST_USE_STATIC)
		
	set(_BOOST_COMPILER ${COMPILER_USED})#${MSVC_PLATFORM}
	set(_BOOST_MULTITHREADED "-mt")
	 set(Boost_USE_MULTITHREADED ON)
  if( NOT Boost_USE_MULTITHREADED )
    set (_BOOST_MULTITHREADED "")
  endif()
 
 	set(AI3D_ARCHITECTURE ${AI3D_PLATFORM_NAME})#$(Platform) 需测试
  set(_BOOST_DEBUG_TAG "-gd")

	set(AI3D_BOOST_LIB "")
 	foreach(COMPONENT ${BOOST_FIND_COMPONENTS})
      set(AI3D_BOOST_LIB ${AI3D_BOOST_LIB} 
      debug ${BOOST_LIB_PREFIX}boost_${COMPONENT}-${_BOOST_COMPILER}${_BOOST_MULTITHREADED}${_BOOST_DEBUG_TAG}-$(Platform)-${BOOST_VERSION_STRING}${LIB_POSTFIX}
      optimized ${BOOST_LIB_PREFIX}boost_${COMPONENT}-${_BOOST_COMPILER}${_BOOST_MULTITHREADED}-$(Platform)-${BOOST_VERSION_STRING}${LIB_POSTFIX}
      )
	  message("==${BOOST_LIB_PREFIX}boost_${COMPONENT}-${_BOOST_COMPILER}${_BOOST_MULTITHREADED}${_BOOST_DEBUG_TAG}-${BOOST_VERSION_STRING}${LIB_POSTFIX}")
  endforeach()
	
elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
	#
	if(AI3D_USE_DEPS_FAST)
		set(AI3D_BOOST_LIBRARY_PATH   ${BOOST_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
		set(BOOST_USE_STATIC ON)
	
	set(BOOST_LIB_PREFIX "")
  if(BOOST_USE_STATIC)
  	set(BOOST_LIB_PREFIX lib)
  endif(BOOST_USE_STATIC)
	set(BOOST_FIND_COMPONENTS   iostreams  program_options  
				filesystem  system  serialization     regex
				)

	set(AI3D_BOOST_LIB "")
 	foreach(COMPONENT ${BOOST_FIND_COMPONENTS})
      set(AI3D_BOOST_LIB ${AI3D_BOOST_LIB} 
     	${BOOST_LIB_PREFIX}boost_${COMPONENT}${LIB_POSTFIX}
      )
  endforeach()
	endif(AI3D_USE_DEPS_FAST)
endif(WIN32)

function(AI3D_USING_BOOST)
	include_directories( ${AI3D_BOOST_INCLUDE_PATH} )
	MESSAGE("- AI3D_BOOST_INCLUDE_PATH- ${AI3D_BOOST_LIB}---")
	link_directories(    ${AI3D_BOOST_LIBRARY_PATH} )
	ADD_DEFINITIONS(${Boost_DEFINITIONS} -D_USE_BOOST)
	SET(_USE_BOOST TRUE)

endfunction()
ENDIF(USE_BOOST)



SET(USE_GEOS ON)
IF(USE_GEOS)
	set(GEOS_VERSION 3.7.1)
	set(GEOS_ROOT_DIR           ${TP_ROOT}/geos/${GEOS_VERSION})
	set(AI3D_GEOS_INCLUDE_PATH   ${GEOS_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_GEOS_LIBRARY_PATH   ${GEOS_ROOT_DIR}/lib/$(Platform))
		set(AI3D_GEOS_LIB
				geos.lib  
				geos_c.lib   libgeos.lib
		)
	endif(WIN32)
	function(AI3D_USING_GEOS)
	ADD_DEFINITIONS(-DHAVE_GEOS)
		include_directories( ${AI3D_GEOS_INCLUDE_PATH} )
		link_directories(    ${AI3D_GEOS_LIBRARY_PATH} )
	endfunction()
ENDIF(USE_GEOS)
	
	

#网络通信相关

	#CURL
SET(USE_CURL ON)
IF(USE_CURL)
	set(CURL_VERSION 7.77.0)
	set(CURL_ROOT_DIR           ${TP_ROOT}/curl/${CURL_VERSION})
	set(AI3D_CURL_INCLUDE_PATH   ${CURL_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_CURL_LIBRARY_PATH   ${CURL_ROOT_DIR}/lib/$(Platform))
		set(AI3D_CURL_LIB
				debug libcurl-d_imp.lib    
				optimized libcurl_imp.lib 
		)
	endif(WIN32)
	function(AI3D_USING_CURL)
		include_directories( ${AI3D_CURL_INCLUDE_PATH} )
		link_directories(    ${AI3D_CURL_LIBRARY_PATH} )
	endfunction()
	ENDIF(USE_CURL)
	
	#EVENT
	SET(USE_EVENT ON)
IF(USE_EVENT)
	set(EVENT_VERSION 2.1.12)
	set(EVENT_ROOT_DIR   ${TP_ROOT}/libevent/${EVENT_VERSION})	
	set(AI3D_EVENT_INCLUDE_PATH   ${EVENT_ROOT_DIR}/include)
	if(WIN32)
	
		set(AI3D_EVENT_LIBRARY_PATH   ${EVENT_ROOT_DIR}/lib/$(Platform))
		set(AI3D_EVENT_LIB event_core.lib  event_extra.lib	event_openssl.lib)
			#  debug event_core.lib  event_extra.lib	event_openssl.lib	
			#  optimized event_core.lib  event_extra.lib	event_openssl.lib)
		
	endif(WIN32)
	function(AI3D_USING_EVENT)
		include_directories( ${AI3D_EVENT_INCLUDE_PATH} )
		link_directories(    ${AI3D_EVENT_LIBRARY_PATH} )
	endfunction()
ENDIF(USE_EVENT)

	#OPENSSL
	SET(USE_OPENSSL ON)
IF(USE_OPENSSL)
	set(OPENSSL_VERSION 1.1.1k)
	set(OPENSSL_ROOT_DIR   ${TP_ROOT}/openssl/${OPENSSL_VERSION})
	set(AI3D_OPENSSL_INCLUDE_PATH   ${OPENSSL_ROOT_DIR}/include)
	if(WIN32)
		
		set(AI3D_OPENSSL_LIBRARY_PATH   ${OPENSSL_ROOT_DIR}/lib/$(Platform))
	
			set(AI3D_OPENSSL_LIB 
			debug libcrypto.lib  libssl.lib
			optimized libcrypto.lib  libssl.lib)
	
	endif(WIN32)
	function(AI3D_USING_OPENSSL)
		include_directories( ${AI3D_OPENSSL_INCLUDE_PATH} )
		link_directories(    ${AI3D_OPENSSL_LIBRARY_PATH} )
	endfunction()
ENDIF(USE_OPENSSL)

#显示
    #OSG
SET(USE_OSG ON)
IF(USE_OSG)

	set(OSG_VERSION_STRING 340)
	
	set(OSG_VERSION_340 3.4.0)
	
	set(OSG_ROOT_DIR_340            ${TP_ROOT}/OSG/${OSG_VERSION_340})
	
	# OSG
	set(AI3D_OSG_INCLUDE_PATH_340	${OSG_ROOT_DIR_340}/include)   
	if(WIN32)

		set(OSG_FIND_COMPONENTS_340 osg	OpenThreads	osgDB osgViewer osgUtil osgGA	osgText osgManipulator
	osgParticle osgShadow	osgSim	osgTerrain   osgText  osgQt   )# osgQt 
			
		#serialization	
		

		set(AI3D_OSG_LIBRARY_PATH_340   ${OSG_ROOT_DIR_340}/lib/$(Platform))

		set(AI3D_OSG_LIB_340 "")
 		foreach(COMPONENT ${OSG_FIND_COMPONENTS_340})
			set(AI3D_OSG_LIB_340 ${AI3D_OSG_LIB_340} 
			debug ${COMPONENT}d${LIB_POSTFIX}
			optimized ${COMPONENT}${LIB_POSTFIX}
		)
		endforeach()
	
	endif(WIN32)

	function(AI3D_USING_OSG_${OSG_VERSION_STRING})
	
		include_directories( ${AI3D_OSG_INCLUDE_PATH_340} )
		link_directories(    ${AI3D_OSG_LIBRARY_PATH_340} )
	endfunction()

	set(OSG_VERSION_STRING 365)
	set(OSG_VERSION_365 3.6.5)
	set(OSG_ROOT_DIR_365            ${TP_ROOT}/OSG/${OSG_VERSION_365})
	# OSG
	set(OSG_ROOT_DIR ${OSG_ROOT_DIR_365})
	set(AI3D_OSG_INCLUDE_PATH_365	${OSG_ROOT_DIR_365}/include)  
set(OSG_FIND_COMPONENTS_365 osg	OpenThreads	osgDB osgViewer osgUtil osgGA	osgText osgManipulator
	osgParticle osgShadow	osgSim	osgTerrain   osgText  osgQt6   )# osgQt 	
	if(WIN32)
		set(AI3D_OSG_LIBRARY_PATH_365   ${OSG_ROOT_DIR_365}/lib/$(Platform))

		set(AI3D_OSG_LIB_365 "")
 	foreach(COMPONENT ${OSG_FIND_COMPONENTS_365})
		set(AI3D_OSG_LIB_365 ${AI3D_OSG_LIB_365} 
		debug ${COMPONENT}d${LIB_POSTFIX}
		optimized ${COMPONENT}${LIB_POSTFIX}
		)
	endforeach()
	
	endif(WIN32)

	function(AI3D_USING_OSG_365)
		include_directories( ${AI3D_OSG_INCLUDE_PATH_365} )
		link_directories(    ${AI3D_OSG_LIBRARY_PATH_365} )
	endfunction()

set(OSG_VERSION_STRING 375)
	set(OSG_VERSION_375 3.7.5)
	set(OSG_ROOT_DIR_375            ${TP_ROOT}/OSG/${OSG_VERSION_375})
	# OSG
	set(OSG_ROOT_DIR ${OSG_ROOT_DIR_375})
	set(AI3D_OSG_INCLUDE_PATH_375	${OSG_ROOT_DIR_375}/include)  
set(OSG_FIND_COMPONENTS_375 OpenThreads  osg  osgDB osgGA osgManipulator osgSim osgTerrain  osgUtil
osgViewer osgText osgFX   )# osgQt 	
	if(WIN32)
		set(AI3D_OSG_LIBRARY_PATH_375   ${OSG_ROOT_DIR_375}/lib/$(Platform))

		set(AI3D_OSG_LIB_375 "")
 	foreach(COMPONENT ${OSG_FIND_COMPONENTS_375})
		set(AI3D_OSG_LIB_375 ${AI3D_OSG_LIB_375} 
		debug ${COMPONENT}d${LIB_POSTFIX}
		optimized ${COMPONENT}${LIB_POSTFIX}
		)
	endforeach()
	
	endif(WIN32)

	function(AI3D_USING_OSG_375)
		include_directories( ${AI3D_OSG_INCLUDE_PATH_375} )
		link_directories(    ${AI3D_OSG_LIBRARY_PATH_375} )
	endfunction()


ENDIF(USE_OSG)
	


	
	


	#GLEW
	SET(USE_GLEW ON)
IF(USE_GLEW)
	set(GLEW_VERSION 2.1.0)
	set(GLEW_ROOT_DIR          ${TP_ROOT}/glew/${GLEW_VERSION})
	set(AI3D_GLEW_INCLUDE_PATH   ${GLEW_ROOT_DIR}/include)
	if(WIN32)
	
		set(AI3D_GLEW_LIBRARY_PATH   ${GLEW_ROOT_DIR}/lib/$(Platform))
		set(AI3D_GLEW_LIB
				debug glew32d${LIB_POSTFIX}    
				optimized glew32${LIB_POSTFIX} 
	)
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
		
		set(AI3D_GLEW_LIB GLEW )
		
	#	endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	function(AI3D_USING_GLEW)
		include_directories( ${AI3D_GLEW_INCLUDE_PATH} )
		link_directories(    ${AI3D_GLEW_LIBRARY_PATH} )		
	endfunction()
ENDIF(USE_GLEW)



#数学库
SET(USE_CERES ON)
IF(USE_CERES)
	set(CERES_VERSION 2.0.0)#1.14 1.14.x
	set(CERES_ROOT_DIR          ${TP_ROOT}/ceres/${CERES_VERSION})
	#CERES
	set(AI3D_CERES_INCLUDE_PATH   ${CERES_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_CERES_LIBRARY_PATH   ${CERES_ROOT_DIR}/lib/$(Platform))
	set(AI3D_CERES_LIB
				debug ceres-debug.lib     optimized ceres.lib)
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_CERES_LIBRARY_PATH   ${CERES_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			
		set(AI3D_CERES_LIB libceres${LIB_POSTFIX} )
		
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	function(AI3D_USING_CERES)
		include_directories( ${AI3D_CERES_INCLUDE_PATH} )
		
		link_directories(    ${AI3D_CERES_LIBRARY_PATH} )
	
	endfunction()
ENDIF(USE_CERES)
	
	
	
	#
	SET(USE_VCG ON)
IF(USE_VCG)
	set(VCG_VERSION 0)
	set(VCG_ROOT_DIR          ${TP_ROOT}/vcg/${VCG_VERSION})
	set(AI3D_VCG_INCLUDE_PATH ${VCG_ROOT_DIR}/include)
	function(AI3D_USING_VCG)
		include_directories( ${AI3D_VCG_INCLUDE_PATH} )
		ADD_DEFINITIONS(${VCG_DEFINITIONS})
		
	endfunction()
ENDIF(USE_VCG)

function(AI3D_USING_EIGEN3)
set(EIGEN3_ROOT_DIR          ${TP_ROOT}/eigen3)
	set(AI3D_EIGEN3_INCLUDE_PATH ${EIGEN3_ROOT_DIR})
		include_directories( ${AI3D_EIGEN3_INCLUDE_PATH} )		
		SET(_USE_EIGEN3 TRUE)
	endfunction()
	
	#EIGEN
	SET(USE_EIGEN ON)
IF(USE_EIGEN)
	set(EIGEN_VERSION 3.3.4)
	set(EIGEN_ROOT_DIR          ${TP_ROOT}/Eigen/${EIGEN_VERSION})
	set(AI3D_EIGEN_INCLUDE_PATH ${EIGEN_ROOT_DIR}/include)
	function(AI3D_USING_EIGEN)
		include_directories( ${AI3D_EIGEN_INCLUDE_PATH} )
		ADD_DEFINITIONS(${EIGEN_DEFINITIONS} -D_USE_EIGEN)
		SET(_USE_EIGEN TRUE)
	endfunction()
ENDIF(USE_EIGEN)

function(AI3D_USING_EIGEN3)
set(EIGEN3_ROOT_DIR          ${TP_ROOT}/eigen3)
	set(AI3D_EIGEN3_INCLUDE_PATH ${EIGEN3_ROOT_DIR})
		include_directories( ${AI3D_EIGEN3_INCLUDE_PATH} )		
		SET(_USE_EIGEN3 TRUE)
	endfunction()

#GIS
SET(USE_QGIS ON)
IF(USE_QGIS)

 set(QGIS_VERSION ) #3.3.0
 set(QGIS_ROOT_DIR          ${TP_ROOT}/qgis/${QGIS_VERSION})


 set(AI3D_QGIS_INCLUDE_PATH   ${QGIS_ROOT_DIR}/include)
 if(WIN32)
  set(AI3D_QGIS_LIBRARY_PATH   ${QGIS_ROOT_DIR}/lib/$(Platform))
  set(AI3D_QGIS_LIB #debug  qgis_core.lib  qgis_app.lib  qgis_gui.lib
#   optimized
   qgis_core.lib  qgis_app.lib  qgis_gui.lib
   
  )
 
endif(WIN32)
 function(AI3D_USING_QGIS)
  include_directories( ${AI3D_QGIS_INCLUDE_PATH} )
  link_directories(    ${AI3D_QGIS_LIBRARY_PATH} )
  message("-${AI3D_QGIS_LIBRARY_PATH}--99-")
 endfunction()
ENDIF(USE_QGIS)
#gdal
SET(USE_GDAL ON)
IF(USE_GDAL)

	if(USE_PROJ)
	
		set(GDAL_VERSION 3.3.0)#
		set(GDAL_ROOT_DIR          ${TP_ROOT}/gdal/${GDAL_VERSION})
		if(WIN32)
			set(AI3D_GDAL_LIBRARY_PATH   ${GDAL_ROOT_DIR}/lib/$(Platform))
			set(AI3D_GDAL_LIB gdal.lib  )
		endif(WIN32)
	else(USE_PROJ)
		set(GDAL_VERSION 3.0.0)#
		set(GDAL_ROOT_DIR          ${TP_ROOT_141}/gdal/${GDAL_VERSION})
		if(WIN32)
			set(AI3D_GDAL_LIBRARY_PATH   ${GDAL_ROOT_DIR}/lib/$(Platform))
			set(AI3D_GDAL_LIB gdal_i.lib  )
		endif(WIN32)
	endif(USE_PROJ)

	 set(AI3D_GDAL_INCLUDE_PATH   ${GDAL_ROOT_DIR}/include)

	function(AI3D_USING_GDAL)
	ADD_DEFINITIONS(-DHAVE_GEOS)
	include_directories( ${AI3D_GDAL_INCLUDE_PATH} )
	link_directories(    ${AI3D_GDAL_LIBRARY_PATH} )
 endfunction()
ENDIF(USE_GDAL)
#PROJ
SET(USE_PROJ6 ON)
IF(USE_PROJ6)


	if(USE_PROJ)

			set(PROJ_VERSION 6.3.2)#6.3.2
			set(PROJ_ROOT_DIR          ${TP_ROOT}/proj/${PROJ_VERSION})
		if(WIN32)
				set(AI3D_PROJ_LIBRARY_PATH   ${PROJ_ROOT_DIR}/lib/$(Platform))
				set(AI3D_PROJ_LIB
				debug proj_d.lib
				optimized proj.lib 
				)
		elseif(UNIX)
	
			if(AI3D_USE_DEPS_FAST)
			set(AI3D_PROJ_LIBRARY_PATH   ${PROJ_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
		
			set(AI3D_PROJ_LIB libproj${LIB_POSTFIX})	
			endif(AI3D_USE_DEPS_FAST)
		endif(WIN32)
	else(USE_PROJ)


		set(PROJ_VERSION 6.0.0)#6.3.2
		set(PROJ_ROOT_DIR          ${TP_ROOT_141}/proj/${PROJ_VERSION})
		if(WIN32)
			set(AI3D_PROJ_LIBRARY_PATH   ${PROJ_ROOT_DIR}/lib/$(Platform))
			set(AI3D_PROJ_LIB
			debug proj_d.lib
			optimized proj_6_0.lib 
			)
		elseif(UNIX)
	
			if(AI3D_USE_DEPS_FAST)
				set(AI3D_PROJ_LIBRARY_PATH   ${PROJ_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
		
				set(AI3D_PROJ_LIB libproj${LIB_POSTFIX})	
			endif(AI3D_USE_DEPS_FAST)
		endif(WIN32)
	endif(USE_PROJ)


	


	set(AI3D_PROJ_INCLUDE_PATH   ${PROJ_ROOT_DIR}/include)
	

	function(AI3D_USING_PROJ)
		include_directories( ${AI3D_PROJ_INCLUDE_PATH} )
		link_directories(    ${AI3D_PROJ_LIBRARY_PATH} )
		ADD_DEFINITIONS(-DACCEPT_USE_OF_DEPRECATED_PROJ_API_H)
	endfunction()
 endif(USE_PROJ6)



  SET(USE_LIBRAW ON)
 
IF(USE_LIBRAW)
	set(LIBRAW_VERSION 0.20.2)
	set(LIBRAW_ROOT_DIR           ${TP_ROOT}/libraw/${LIBRAW_VERSION})
	
	set(AI3D_LIBRAW_INCLUDE_PATH   ${LIBRAW_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_LIBRAW_LIBRARY_PATH   ${LIBRAW_ROOT_DIR}/lib/$(Platform))
		set(AI3D_LIBRAW_LIB libraw.lib 				 )
				
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_LIBRAW_LIBRARY_PATH   ${GEOGRAPHICLIB_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	function(AI3D_USING_LIBRAW)
		include_directories( ${AI3D_LIBRAW_INCLUDE_PATH} )
		link_directories(    ${AI3D_LIBRAW_LIBRARY_PATH} )
		

	endfunction()
ENDIF(USE_LIBRAW)

 SET(USE_KML ON)


IF(USE_KML)
	set(KML_VERSION 0.0.2)
	set(KML_ROOT_DIR           ${TP_ROOT}/libkml/${KML_VERSION})
	
	set(AI3D_KML_INCLUDE_PATH   ${KML_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_KML_LIBRARY_PATH   ${KML_ROOT_DIR}/lib/$(Platform))
		set(AI3D_KML_LIB kmlbase.lib kmlconvenience.lib kmldom.lib kmlengine.lib
				kmlregionator.lib kmlxsd.lib 
				#libkmlbase.lib libkmlconvenience.lib libkmldom.lib libkmlengine.lib
				#libkmlregionator.lib libkmlxsd.lib uriparser.lib
				 )
				
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_KML_LIBRARY_PATH   ${KML_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	function(AI3D_USING_KML)
		include_directories( ${AI3D_KML_INCLUDE_PATH} )
		link_directories(    ${AI3D_KML_LIBRARY_PATH} )
		

	endfunction()
ENDIF(USE_KML)

#
SET(USE_GEOGRAPHICLIB ON)
IF(USE_GEOGRAPHICLIB)
	set(GEOGRAPHICLIB_VERSION 1.50.1)
	set(GEOGRAPHICLIB_ROOT_DIR           ${TP_ROOT}/geographiclib/${GEOGRAPHICLIB_VERSION})
	
	set(AI3D_GEOGRAPHICLIB_INCLUDE_PATH   ${GEOGRAPHICLIB_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_GEOGRAPHICLIB_LIBRARY_PATH   ${GEOGRAPHICLIB_ROOT_DIR}/lib/$(Platform))
		set(AI3D_GEOGRAPHICLIB_LIB
				debug Geographic_d.lib
				optimized Geographic.lib )
				
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_GEOGRAPHICLIB_LIBRARY_PATH   ${GEOGRAPHICLIB_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			
			set(AI3D_GEOGRAPHICLIB_LIB Geographic
	#${LIB_POSTFIX}
	)
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	function(AI3D_USING_GEOGRAPHICLIB)
		include_directories( ${AI3D_GEOGRAPHICLIB_INCLUDE_PATH} )
		link_directories(    ${AI3D_GEOGRAPHICLIB_LIBRARY_PATH} )
		

	endfunction()
ENDIF(USE_GEOGRAPHICLIB)

#图像处理相关


SET(USE_EXIV2 ON)
IF(USE_EXIV2)
	#set(EXIV2_VERSION 0.26.0)
	set(EXIV2_VERSION 0.28.0)
	set(EXIV2_ROOT_DIR          ${TP_ROOT}/exiv2/${EXIV2_VERSION})

	set(AI3D_EXIV2_INCLUDE_PATH   ${EXIV2_ROOT_DIR}/include)
	if(WIN32)
  
  		set(AI3D_EXIV2_LIBRARY_PATH   ${EXIV2_ROOT_DIR}/lib/$(Platform))
		set(AI3D_EXIV2_LIB
			debug exiv2d.lib    
			optimized exiv2.lib 
		)
	endif(WIN32)
	function(AI3D_USING_EXIV2)
	include_directories( ${AI3D_EXIV2_INCLUDE_PATH} )
	link_directories(    ${AI3D_EXIV2_LIBRARY_PATH} )
		
	endfunction()

ENDIF(USE_EXIV2)


#FREEIMAGE
SET(USE_FREEIMAGE ON)
IF(USE_FREEIMAGE)
	set(FREEIMAGE_VERSION 3.18.0)
	set(FREEIMAGE_ROOT_DIR   ${TP_ROOT}/Freeimage/${FREEIMAGE_VERSION})

	set(AI3D_FREEIMAGE_INCLUDE_PATH   ${FREEIMAGE_ROOT_DIR}/include)
	if(WIN32)
  
  		set(AI3D_FREEIMAGE_LIBRARY_PATH   ${FREEIMAGE_ROOT_DIR}/lib/$(Platform))
		set(AI3D_FREEIMAGE_LIB
			debug FreeImaged.lib    
			optimized FreeImage.lib 
		)
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_FREEIMAGE_LIBRARY_PATH   ${FREEIMAGE_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
		
			set(AI3D_FREEIMAGE_LIB libfreeimage${LIB_POSTFIX} )
	
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	function(AI3D_USING_FREEIMAGE)
		include_directories( ${AI3D_FREEIMAGE_INCLUDE_PATH} )
		link_directories(    ${AI3D_FREEIMAGE_LIBRARY_PATH} )
		
	endfunction()
ENDIF(USE_FREEIMAGE)

	#FreeGLut
	SET(USE_FREEGLUT ON)
	IF(USE_FREEGLUT)
	SET(FREEGLUT_VERSION  3.0.0)
	set(FREEGLUT_ROOT_DIR          ${TP_ROOT}/freeglut/${FREEGLUT_VERSION})
	set(AI3D_FREEGLUT_INCLUDE_PATH   ${FREEGLUT_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_FREEGLUT_LIBRARY_PATH   ${FREEGLUT_ROOT_DIR}/lib/$(Platform))
		set(AI3D_FREEGLUT_LIB	debug  freeglutd.lib optimized  freeglut.lib) #freeglut.lib  freeglutd.lib
	endif(WIN32)
	
			if(AI3D_USE_DEPS_FAST)
			set(AI3D_FREEGLUT_LIBRARY_PATH   ${JPEG_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			
		set(AI3D_FREEGLUT_LIB freeglut${LIB_POSTFIX} )
		
		endif(AI3D_USE_DEPS_FAST)


	function(AI3D_USING_FREEGLUT)
		include_directories( ${AI3D_FREEGLUT_INCLUDE_PATH} )
		link_directories(    ${AI3D_FREEGLUT_LIBRARY_PATH} )
			ADD_DEFINITIONS(${FREEGLUT_DEFINITIONS} -D_USE_FREEGLUT)
		SET(_USE_FREEGLUT TRUE CACHE INTERNAL "")
	endfunction()
	ENDIF(USE_FREEGLUT)

	#PNG
	SET(USE_PNG ON)
	IF(USE_PNG)
	SET(PNG_VERSION  1.6.37)
	set(PNG_ROOT_DIR          ${TP_ROOT}/libpng/${PNG_VERSION})
	set(AI3D_PNG_INCLUDE_PATH   ${PNG_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_PNG_LIBRARY_PATH   ${PNG_ROOT_DIR}/lib/$(Platform))
		set(AI3D_PNG_LIB	debug  libpng16d.lib optimized  libpng16.lib) #libpng.lib  libpngd.lib
	elseif(UNIX)
		
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_PNG_LIBRARY_PATH   ${PNG_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			
			set(AI3D_PNG_LIB libpng16${LIB_POSTFIX})	
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	function(AI3D_USING_PNG)
		include_directories( ${AI3D_PNG_INCLUDE_PATH} )
		link_directories(    ${AI3D_PNG_LIBRARY_PATH} )
			ADD_DEFINITIONS(${PNG_DEFINITIONS} -D_USE_PNG)
		SET(_USE_PNG TRUE CACHE INTERNAL "")
	endfunction()
	ENDIF(USE_PNG)


	#JPEG	
	SET(USE_JPEG ON)
	IF(USE_JPEG)
	set(JPEG_VERSION 9.0.4) 
	set(JPEG_ROOT_DIR          ${TP_ROOT}/jpeg/${JPEG_VERSION})
	set(AI3D_JPEG_INCLUDE_PATH   ${JPEG_ROOT_DIR}/include)
	if(WIN32)
	
		set(AI3D_JPEG_LIBRARY_PATH   ${JPEG_ROOT_DIR}/lib/$(Platform))
		set(AI3D_JPEG_LIB
				debug jpegd${LIB_POSTFIX}
				optimized jpeg${LIB_POSTFIX} 
	)	
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_JPEG_LIBRARY_PATH   ${JPEG_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			
		set(AI3D_JPEG_LIB libjpeg${LIB_POSTFIX} )
		
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	
	function(AI3D_USING_JPEG)
		include_directories( ${AI3D_JPEG_INCLUDE_PATH} )
		link_directories(    ${AI3D_JPEG_LIBRARY_PATH} )
			ADD_DEFINITIONS(${JPEG_DEFINITIONS} -D_USE_JPG)
			SET(_USE_JPG TRUE CACHE INTERNAL "")
	endfunction()
	ENDIF(USE_JPEG)


	#TIFF
	SET(USE_TIFF ON)

IF(USE_TIFF)
	set(TIFF_VERSION 4.0.9)
	set(TIFF_ROOT_DIR          ${TP_ROOT}/tiff/${TIFF_VERSION})
	
	set(AI3D_TIFF_INCLUDE_PATH   ${TIFF_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_TIFF_LIBRARY_PATH   ${TIFF_ROOT_DIR}/lib/$(Platform))
		set(AI3D_TIFF_LIB
				debug tiffd.lib
				optimized tiff.lib 
		)
	
	elseif(UNIX)
	
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_TIFF_LIBRARY_PATH   ${TIFF_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			set(AI3D_TIFF_LIB
							libtiff${LIB_POSTFIX})
							
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	function(AI3D_USING_TIFF)
		include_directories( ${AI3D_TIFF_INCLUDE_PATH} )
		link_directories(    ${AI3D_TIFF_LIBRARY_PATH} )
		ADD_DEFINITIONS(${TIFF_DEFINITIONS} -D_USE_TIFF)
		SET(_USE_TIFF TRUE CACHE INTERNAL "")
	endfunction()
ENDIF(USE_TIFF)
	
#zlib
	SET(USE_ZLIB ON)
IF(USE_ZLIB)
	set(ZLIB_VERSION 1.2.11)
	set(ZLIB_ROOT_DIR          ${TP_ROOT}/zlib/${ZLIB_VERSION})

	set(AI3D_ZLIB_INCLUDE_PATH   ${ZLIB_ROOT_DIR}/include)
	if(WIN32)
	set(AI3D_ZLIB_LIBRARY_PATH   ${ZLIB_ROOT_DIR}/lib/$(Platform))
	set(AI3D_ZLIB_LIB
				debug zlibd.lib          optimized zlib.lib
	#			debug zlibstaticd.lib    optimized zlibstatic.lib 
	)
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
	
		if(AI3D_USE_DEPS_FAST)
	
			set(AI3D_ZLIB_LIBRARY_PATH   ${ZLIB_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
	
			set(AI3D_ZLIB_LIB libz${LIB_POSTFIX} ) 
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	function(AI3D_USING_ZLIB)
		include_directories( ${AI3D_ZLIB_INCLUDE_PATH} )
		link_directories(    ${AI3D_ZLIB_LIBRARY_PATH} )
	endfunction()

ENDIF(USE_ZLIB)

#OPENCV
SET(USE_OPENCV ON)
IF(USE_OPENCV)
	set(OPENCV_VERSION 4100)
	set(OPENCV_ROOT_DIR   ${TP_ROOT}/opencv/${OPENCV_VERSION})
	set(AI3D_OPENCV_INCLUDE_PATH	${OPENCV_ROOT_DIR}/include)
	set(OPENCV_FIND_COMPONENTS  world
				)#xfeatures2d
	
	if(WIN32)

		set(AI3D_OPENCV_LIBRARY_PATH   ${OPENCV_ROOT_DIR}/lib/$(Platform))

		set(AI3D_OPENCV_LIB "")
 		foreach(COMPONENT ${OPENCV_FIND_COMPONENTS})
		  set(AI3D_OPENCV_LIB ${AI3D_OPENCV_LIB} 
		  debug opencv_${COMPONENT}${OPENCV_VERSION}d${LIB_POSTFIX}
		  optimized opencv_${COMPONENT}${OPENCV_VERSION}${LIB_POSTFIX}
		  )
		endforeach()
			
		elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
 
		if(AI3D_USE_DEPS_FAST)
		set(AI3D_OPENCV_LIBRARY_PATH   ${OPENCV_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
		set(OPENCV_USE_STATIC ON)
		
		
 		foreach(COMPONENT ${OPENCV_FIND_COMPONENTS})
		  set(AI3D_OPENCV_LIB ${AI3D_OPENCV_LIB} 
     	opencv_${COMPONENT}
		  )
		  endforeach()
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	function(AI3D_USING_OPENCV)
	include_directories( ${AI3D_OPENCV_INCLUDE_PATH} )
	link_directories(    ${AI3D_OPENCV_LIBRARY_PATH} )
	ADD_DEFINITIONS(${OpenCV_DEFINITIONS})
	SET(_USE_OPENCV TRUE)
	MESSAGE(STATUS "OpenCV ${OpenCV_VERSION} found (include: ${OpenCV_INCLUDE_DIRS})")
	endfunction()

ENDIF(USE_OPENCV)




#数据库相关
SET(USE_SQLITE ON)
IF(USE_SQLITE)
	set(SQLITE_VERSION 3.36)
	set(SQLITE_ROOT_DIR          ${TP_ROOT}/sqlite3/${SQLITE_VERSION})
	
	set(AI3D_SQLITE_INCLUDE_PATH   ${SQLITE_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_SQLITE_LIBRARY_PATH   ${SQLITE_ROOT_DIR}/lib/$(Platform))
		set(AI3D_SQLITE_LIB
				debug sqlite3.lib   
				optimized sqlite3.lib
	)
	endif(WIN32)
	function(AI3D_USING_SQLITE)
		include_directories( ${AI3D_SQLITE_INCLUDE_PATH} )
		link_directories(    ${AI3D_SQLITE_LIBRARY_PATH} )
			
	endfunction()
ENDIF(USE_SQLITE)

#JSON
SET(USE_RAPIDJSON ON)
IF(USE_RAPIDJSON)
	set(RAPIDJSON_VERSION )
	set(RAPIDJSON_ROOT_DIR   ${TP_ROOT}/rapidjson/${RAPIDJSON_VERSION})


	set(AI3D_RAPIDJSON_INCLUDE_PATH ${RAPIDJSON_ROOT_DIR}/include)
	function(AI3D_USING_RAPIDJSON)
		include_directories( ${AI3D_RAPIDJSON_INCLUDE_PATH} )
	endfunction()
ENDIF(USE_RAPIDJSON)

#xml
set(USE_PUGIXML ON)
IF(USE_PUGIXML)
	set(PUGIXML_VERSION 1.11.0)
	set(PUGIXML_ROOT_DIR ${TP_ROOT}/pugixml/${PUGIXML_VERSION})
	set(AI3D_PUGIXML_INCLUDE_PATH ${PUGIXML_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_PUGIXML_LIBRARY_PATH ${PUGIXML_ROOT_DIR}/lib/$(Platform))
		set(AI3D_PUGIXML_LIB 
			debug pugixml.lib
			optimized pugixml.lib
		)
	endif(WIN32)
	function(AI3D_USING_PUGIXML)
		include_directories(${AI3D_PUGIXML_INCLUDE_PATH})
	
		link_directories(${AI3D_PUGIXML_LIBRARY_PATH})
	endfunction()
ENDIF(USE_PUGIXML)

#MESH操作相关
     #OPENMESH
SET(USE_OPENMESH ON)
IF(USE_OPENMESH)
	set(OPENMESH_VERSION 8.1)
	set(OPENMESH_ROOT_DIR           ${TP_ROOT}/openmesh/${OPENMESH_VERSION})
	set(AI3D_OPENMESH_INCLUDE_PATH   ${OPENMESH_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_OPENMESH_LIBRARY_PATH   ${OPENMESH_ROOT_DIR}/lib/$(Platform))
		set(AI3D_OPENMESH_LIB
				debug OpenMeshToolsd.lib  OpenMeshCored.lib  
				optimized OpenMeshTools.lib  OpenMeshCore.lib
		)
	endif(WIN32)
	function(AI3D_USING_OPENMESH)
		include_directories( ${AI3D_OPENMESH_INCLUDE_PATH} )
		link_directories(    ${AI3D_OPENMESH_LIBRARY_PATH} )
	endfunction()
ENDIF(USE_OPENMESH)

#OpenXLSX
SET(USE_OpenXLSX ON)
IF(USE_OpenXLSX)
	set(OpenXLSX_VERSION 0.4.1)
	set(OpenXLSX_ROOT_DIR           ${TP_ROOT}/OpenXLSX/${OpenXLSX_VERSION})
	
	set(AI3D_OpenXLSX_INCLUDE_PATH   ${OpenXLSX_ROOT_DIR}/include)
	if(WIN32)
		set(AI3D_OpenXLSX_LIBRARY_PATH   ${OpenXLSX_ROOT_DIR}/lib/$(Platform))
		set(AI3D_OpenXLSX_LIB
				debug OpenXLSXd.lib
				optimized OpenXLSX.lib)
				
	elseif(UNIX)  #此处未完还未考虑debug版本//根据需求添加
		if(AI3D_USE_DEPS_FAST)
			set(AI3D_OpenXLSX_LIBRARY_PATH   ${OpenXLSX_ROOT_DIR}/lib/${AI3D_PLATFORM_NAME}/Release)
			
			set(AI3D_OpenXLSX_LIB OpenXLSX
	#${LIB_POSTFIX}
	)
		endif(AI3D_USE_DEPS_FAST)
	endif(WIN32)
	
	function(AI3D_USING_OpenXLSX)
		include_directories( ${AI3D_OpenXLSX_INCLUDE_PATH} )
		link_directories(    ${AI3D_OpenXLSX_LIBRARY_PATH} )
	endfunction()
ENDIF(USE_OpenXLSX)

#libiconv	
SET(USE_ICONV ON) 
IF(USE_ICONV)
set(ICONV_VERSION 1.17)
set(ICONV_ROOT_DIR          ${TP_ROOT}/libiconv/${ICONV_VERSION})
set(AI3D_ICONV_INCLUDE_PATH   ${ICONV_ROOT_DIR}/include)
if(WIN32)	
	set(AI3D_ICONV_LIBRARY_PATH   ${ICONV_ROOT_DIR}/lib/$(Platform))
	set(AI3D_ICONV_LIB
			#debug     
			optimized iconv.lib charset.lib
	)
endif(WIN32)
function(AI3D_USING_ICONV)
	include_directories( ${AI3D_ICONV_INCLUDE_PATH} )
	link_directories(    ${AI3D_ICONV_LIBRARY_PATH} )		
endfunction()
	
ENDIF(USE_ICONV)

#libfreexl	
SET(USE_FREEXL ON) 
IF(USE_FREEXL)
set(FREEXL_VERSION 1.0.6)
set(FREEXL_ROOT_DIR          ${TP_ROOT}/libfreexl/${FREEXL_VERSION})
set(AI3D_FREEXL_INCLUDE_PATH   ${FREEXL_ROOT_DIR}/include)
if(WIN32)	
	set(AI3D_FREEXL_LIBRARY_PATH   ${FREEXL_ROOT_DIR}/lib/$(Platform))
	set(AI3D_FREEXL_LIB
			#debug     
			optimized freexl.lib
	)
endif(WIN32)
function(AI3D_USING_FREEXL)
	include_directories( ${AI3D_FREEXL_INCLUDE_PATH} )
	link_directories(    ${AI3D_FREEXL_LIBRARY_PATH} )		
endfunction()
ENDIF(USE_FREEXL)

#xlnt
SET(USE_XLNT ON) 
IF(USE_XLNT)
set(XLNT_VERSION 1.5.0)
set(XLNT_ROOT_DIR          ${TP_ROOT}/xlnt/${XLNT_VERSION})
set(AI3D_XLNT_INCLUDE_PATH   ${XLNT_ROOT_DIR}/include)
if(WIN32)	
	set(AI3D_XLNT_LIBRARY_PATH   ${XLNT_ROOT_DIR}/lib/$(Platform))
	set(AI3D_XLNT_LIB
			#debug libexpatd.lib    
			optimized xlnt.lib
	)
endif(WIN32)
function(AI3D_USING_XLNT)
	include_directories( ${AI3D_XLNT_INCLUDE_PATH} )
	link_directories(    ${AI3D_XLNT_LIBRARY_PATH} )		
endfunction()
	
ENDIF(USE_XLNT)

# #htmlDoc
# SET(USE_HTMLDOC ON)
# IF(USE_HTMLDOC)
# 	set(USE_HTMLDOC_ROOT_DIR          ${TP_ROOT}/htmlDoc)
# 	set(AI3D_HTMLDOC_INCLUDE_PATH ${USE_HTMLDOC_ROOT_DIR})
# 	message("*******HTMLDOC PATH**********: "${AI3D_HTMLDOC_INCLUDE_PATH})
# 	function(AI3D_USING_HTMLDOC)
# 		include_directories( ${AI3D_HTMLDOC_INCLUDE_PATH} )
# 		#ADD_DEFINITIONS(${HTMLDOC_DEFINITIONS} -D_USE_HTMLDOC)
# 		#SET(_USE_HTMLDOC TRUE)
# 	endfunction()
# ENDIF(USE_HTMLDOC)

# #htmlDoc
# SET(USE_HISTOGRAM ON)
# IF(USE_HISTOGRAM)
# 	set(USE_HISTOGRAM_ROOT_DIR          ${TP_ROOT}/histogram)
# 	set(AI3D_HISTOGRAM_INCLUDE_PATH ${USE_HISTOGRAM_ROOT_DIR})
# 	message("*******HISTOGRAM PATH**********: "${AI3D_HISTOGRAM_INCLUDE_PATH})
# 	function(AI3D_USING_HISTOGRAM)
# 		include_directories( ${AI3D_HISTOGRAM_INCLUDE_PATH} )
# 		#ADD_DEFINITIONS(${HISTOGRAM_DEFINITIONS} -D_USE_HISTOGRAM)
# 		#SET(_USE_HISTOGRAM TRUE)
# 	endfunction()
# ENDIF(USE_HISTOGRAM)












