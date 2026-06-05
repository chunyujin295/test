if (WIN32)
 add_definitions(-D_UNICODE -DUNICODE)
endif()

set(TARGET_NAME MoldAIData)# 设定目标名
############################### GRAB HEADERS #####################################
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
									"${CMAKE_CURRENT_SOURCE_DIR}/../../include/Core/*.h"
									"${CMAKE_CURRENT_SOURCE_DIR}/../../include/Core/*.hpp"	 # 设定头文件路径
									"${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/HttpClient.h"
									"${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/constant.h"
									)



########################################### BEGIN: GRAB SOURCE FILES ##########################################
FILE(GLOB SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
										"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
										"${CMAKE_CURRENT_SOURCE_DIR}/../Util/HttpClient.cpp"
								)




#分组显示end

ADD_DEFINITIONS("-DAI3D_MAKE_DLL") # 如果生成动态库则需要
AI3D_USING_EIGEN3()
AI3D_USING_PROJ()
AI3D_USING_SQLITE()
AI3D_USING_RAPIDJSON()
AI3D_USING_KML()
AI3D_USING_GDAL()
#AI3D_USING_CERES()
AI3D_USING_GEOS()

#set(INCLUDE_PROJ ON)
if(INCLUDE_PROJ)
#	ADD_DEFINITIONS("-DUSE_AI3D_PROJ")
	FILE(GLOB PROJ_HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 									
										"${CMAKE_CURRENT_SOURCE_DIR}/../../include/Core/Proj/*.h"
									)
			FILE(GLOB PROJ_SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"												
	                                         	"${CMAKE_CURRENT_SOURCE_DIR}/Proj/*.cpp"										
									)
	
				MESSAGE("========${PROJ_SRC_LIST}---")																		
									
	SOURCE_GROUP(Proj\\Headers FILES ${PROJ_HEADER_LIST})
	SOURCE_GROUP(Proj\\Sources FILES ${PROJ_SRC_LIST})
	if(USE_QT)
	
		set(DEPENDS_LIBS ${DEPENDS_LIBS} Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::Sql Qt6::Xml Qt6::Network)
			
	endif(USE_QT)
	set(SRC_LIST ${SRC_LIST} ${PROJ_SRC_LIST} )
	set(HEADER_LIST ${HEADER_LIST} ${PROJ_HEADER_LIST} )
endif(INCLUDE_PROJ)


set(SRC_FILES ${SRC_LIST} ${HEADER_LIST})
set(DEPENDS_LIBS ${DEPENDS_LIBS} ${EXTER_DEPENDS_LIBS} ${AI3D_GDAL_LIB} ${AI3D_KML_LIB} ${AI3D_GEOS_LIB} ${AI3D_SQLITE_LIB} ${AI3D_PROJ_LIB})
#message("${AI3D_GEOS_LIBRARY_PATH}    ${AI3D_GEOS_INCLUDE_PATH}   ${AI3D_GEOS_LIB} ")
# Must match glog's google::LogSink::send() (see Core/Logging.cpp). Change via CMake: -DMOLDAI_GLOG_LOGSINK_API=0|1|2
if(NOT DEFINED MOLDAI_GLOG_LOGSINK_API)
	set(MOLDAI_GLOG_LOGSINK_API "1" CACHE STRING "glog LogSink: 0=old, 1=MSVC 6-arg, 2=LogMessageTime (glog>=~0.6)")
endif()
add_library(${TARGET_NAME} SHARED ${SRC_FILES} ) #STATIC  SHARED

target_link_libraries(${TARGET_NAME} 	${DEPENDS_LIBS}	)

set_target_properties(${TARGET_NAME} PROPERTIES
	FOLDER "Libs"
	COMPILE_DEFINITIONS "MOLDAI_GLOG_LOGSINK_API=${MOLDAI_GLOG_LOGSINK_API}"
)
set(COPYDLL ON)
if(COPYDLL)
if (WIN32)
add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
  COMMAND   ${CMAKE_COMMAND} -E copy ${AI3D_DATA_DIR}/cameras/CD.txt $<TARGET_FILE_DIR:${TARGET_NAME}>\n	
  COMMAND   ${CMAKE_COMMAND} -E copy_directory ${AI3D_DATA_DIR}/gdal/data  $<TARGET_FILE_DIR:${TARGET_NAME}>/data/\n	
	COMMAND		${CMAKE_COMMAND} -E copy ${FREEIMAGE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/FreeImage.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
		
#	COMMAND		${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/padlock.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n

	COMMAND		${CMAKE_COMMAND} -E copy ${GLOG_ROOT_DIR}/dll/$(Platform)/$(Configuration)/glog.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#	COMMAND		${CMAKE_COMMAND} -E copy ${ICONV_ROOT_DIR}/dll/$(Platform)/$(Configuration)/charset-1.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND		${CMAKE_COMMAND} -E copy ${ICONV_ROOT_DIR}/dll/$(Platform)/$(Configuration)/iconv-2.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND		${CMAKE_COMMAND} -E copy ${FREEXL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/freexl.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND		${CMAKE_COMMAND} -E copy ${XLNT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/xlnt.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${OPENCV_ROOT_DIR}/dll/$(Platform)/$(Configuration)/opencv_world${OPENCV_VERSION}.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	  COMMAND ${CMAKE_COMMAND} -E copy ${GEOS_ROOT_DIR}/dll/$(Platform)/$(Configuration)/geos_c.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	  COMMAND ${CMAKE_COMMAND} -E copy ${GEOS_ROOT_DIR}/dll/$(Platform)/$(Configuration)/geos.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n		
			

)

add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
 # COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../../data/proj/proj.db $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND		${CMAKE_COMMAND} -E copy ${EXPAT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libexpat.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
)



add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
  COMMAND   ${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/capi.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND		${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libcrypto-1_1-x64.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND		${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libssl-1_1-x64.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
		
			
)

add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
  COMMAND    ${CMAKE_COMMAND} -E copy ${FREEIMAGE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/FreeImage.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND		 ${CMAKE_COMMAND} -E copy ${EXIV2_ROOT_DIR}/dll/$(Platform)/$(Configuration)/exiv2.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND		  ${CMAKE_COMMAND} -E copy ${ZLIB_ROOT_DIR}/dll/$(Platform)/$(Configuration)/zlib.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			  
			
)
endif(WIN32)

endif()