
set(TARGET_NAME 3DViewer)
AI3D_USING_EXIV2()
AI3D_USING_GFLAGS()
AI3D_USING_CERES()
AI3D_USING_BOOST()
AI3D_USING_EIGEN()
AI3D_USING_GLOG()
AI3D_USING_CURL()
AI3D_USING_EVENT()
AI3D_USING_OPENSSL()
AI3D_USING_RAPIDJSON()
AI3D_USING_PROJ()
AI3D_USING_OSG_340()
AI3D_USING_FREEIMAGE()
AI3D_USING_EXPAT()
AI3D_USING_ZLIB()
AI3D_USING_PNG()
AI3D_USING_JPEG()
AI3D_USING_TIFF()
AI3D_USING_SQLITE()
find_package(OpenGL REQUIRED)
AI3D_USING_GLEW()
AI3D_USING_GEOGRAPHICLIB()
#AI3D_USING_GDAL()
AI3D_USING_PUGIXML()


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Include/3DViewer	)

		
############################### GRAB HEADERS #####################################
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
									
									${CMAKE_CURRENT_SOURCE_DIR}/../../Include/3DViewer/*.h
							
									
									)		

########################################### BEGIN: GRAB SOURCE FILES ##########################################
FILE(GLOB SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"	
									"${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
									
								)





set(SRC_FILES ${SRC_LIST} ${HEADER_LIST})
set(DEPENDS_LIBS ${AI3D_GLOG_LIB} 	${AI3D_CURL_LIB} ${AI3D_OPENSSL_LIB}  	${AI3D_EVENT_LIB} 
						${AI3D_FREEIMAGE_LIB}
				${AI3D_GFLAGS_LIB} 	${AI3D_BOOST_LIB} 
				${AI3D_CERES_LIB}	${AI3D_EXPAT_LIB}
				${AI3D_PROJ_LIB}	${AI3D_OSG_LIB}
				${AI3D_EXIV2_LIB}		${AI3D_GLEW_LIB}
				${AI3D_JPEG_LIB}		${AI3D_ZLIB_LIB}
				${AI3D_PNG_LIB}			${AI3D_TIFF_LIB}
				${AI3D_SQLITE_LIB}	${OPENGL_LIBRARIES} 
				${AI3D_GEOGRAPHICLIB_LIB} 
				#${AI3D_GDAL_LIB}
				${AI3D_PUGIXML_LIB}
				Core)


if(USE_QT)

	file( GLOB ui_list RELATIVE
	                             "${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/*.ui"
									
								
									)
	file( GLOB qrc_list RELATIVE
	                             "${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/*.qrc"
									
									)
	
				file( GLOB rc_list RELATIVE
	                             "${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/Mohacker.rc"
										"${CMAKE_CURRENT_SOURCE_DIR}/resource.h"
									)
	
	qt5_wrap_ui( generated_ui_list ${ui_list} )
	qt5_add_resources( generated_qrc_list ${qrc_list} )
	
	set(DEPENDS_LIBS ${DEPENDS_LIBS} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Sql Qt5::Xml)
		set(SRC_FILES ${SRC_FILES} ${generated_ui_list} ${generated_qrc_list} ${rc_list})
	
		
SOURCE_GROUP(UI\\Headeres FILES ${generated_ui_list} ${ui_list})
SOURCE_GROUP(UI\\Sources FILES ${generated_qrc_list} ${qrc_list})

endif(USE_QT)

FILE(GLOB 3DVIEW_HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
									
									${CMAKE_CURRENT_SOURCE_DIR}/../../Include/3DViewer/*.h
						
									
									)		


FILE(GLOB 3DVIEW_SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"	
									"${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
									
								)
SOURCE_GROUP(3DVIEW\\Headeres FILES ${3DVIEW_HEADER_LIST} )
SOURCE_GROUP(3DVIEW\\Sources FILES ${3DVIEW_SRC_LIST})

add_library(${TARGET_NAME} STATIC ${SRC_FILES} ) #STATIC  SHARED
#add_executable( ${TARGET_NAME}  ${SRC_FILES})
if (WIN32)
 target_link_libraries(${TARGET_NAME} Qt5::WinMain)
endif()
target_link_libraries(${TARGET_NAME} 
				 ${DEPENDS_LIBS}
				)

Set_Target_Properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
set(COPYDLL OFF)
if(COPYDLL)

if (WIN32)





add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
  COMMAND SET postfix=\n
           IF /I $(Configuration)== Debug ( \n
               SET postfix=d\n
           ) \n
            ${CMAKE_COMMAND} -E copy ${ZLIB_ROOT_DIR}/dll/$(Platform)/$(Configuration)/zlib%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${EXIV2_ROOT_DIR}/dll/$(Platform)/$(Configuration)/exiv2%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${GLOG_ROOT_DIR}/dll/$(Platform)/$(Configuration)/glog%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${EXPAT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libexpat%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${FREEIMAGE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/Freeimage%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${GDAL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/gdal300%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			#${CMAKE_COMMAND} -E copy ${QTDIR}/bin/Qt5Xml%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			
)


add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_core.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_extra.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_openssl.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
)

#qt
if(USE_QT)
add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND SET postfix=\n
            IF /I $(Configuration)== Debug ( \n
                SET postfix=d\n
            ) \n
            ${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt5Core%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt5Gui%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt5OpenGL%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt5Widgets%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt5Xml%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
)
endif(USE_QT)

endif(WIN32)


endif()
#set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
#target_compile_definitions(${TARGET_NAME} PRIVATE  AI3D_MAKE_DLL)
#set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME_DEBUG ${TARGET_NAME}d)