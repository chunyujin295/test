
set(TARGET_NAME MoldAI)
AI3D_USING_RAPIDJSON()
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
#AI3D_USING_OSG_340()
AI3D_USING_FREEIMAGE()
AI3D_USING_EXPAT()
AI3D_USING_ZLIB()
AI3D_USING_PNG()
AI3D_USING_JPEG()
AI3D_USING_TIFF()
AI3D_USING_KML()
AI3D_USING_GEOS()
AI3D_USING_SQLITE()
AI3D_USING_OSG_375()

find_package(OpenGL REQUIRED)
AI3D_USING_GLEW()
AI3D_USING_GEOGRAPHICLIB()

AI3D_USING_PUGIXML()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../	)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Include )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Include/3DViewer )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Util )	
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Include/OSGEditor )

############################### GRAB HEADERS #####################################
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
									
									${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/*.h
									${CMAKE_CURRENT_SOURCE_DIR}/../../Include/3DViewer/*.h
									"${CMAKE_CURRENT_SOURCE_DIR}/../3DViewer/*.cpp"
									"${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/LoginDialog.h"
									${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Util/*.h
									)		
#LIST(REMOVE_ITEM HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/ControlPointsEditorWin.h)
########################################### BEGIN: GRAB SOURCE FILES ##########################################
FILE(GLOB SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"										
									"${CMAKE_CURRENT_SOURCE_DIR}/../3DViewer/*.cpp"
									"${CMAKE_CURRENT_SOURCE_DIR}/LoginDialog.cpp"
									"${CMAKE_CURRENT_SOURCE_DIR}/../Util/*.cpp"
								)
								



set(SRC_FILES ${SRC_LIST} ${HEADER_LIST})
set(DEPENDS_LIBS ${AI3D_GLOG_LIB} 	${AI3D_CURL_LIB} ${AI3D_OPENSSL_LIB}  	${AI3D_EVENT_LIB} 
						${AI3D_FREEIMAGE_LIB} ${AI3D_KML_LIB} 
				${AI3D_GFLAGS_LIB} 	${AI3D_BOOST_LIB} 
				${AI3D_CERES_LIB}	${AI3D_EXPAT_LIB}
				${AI3D_PROJ_LIB}	#${AI3D_OSG_LIB}
				${AI3D_EXIV2_LIB}		${AI3D_GLEW_LIB}
				${AI3D_JPEG_LIB}		${AI3D_ZLIB_LIB}
				${AI3D_PNG_LIB}			${AI3D_TIFF_LIB}
				${AI3D_SQLITE_LIB}	${OPENGL_LIBRARIES} 
				${AI3D_GEOGRAPHICLIB_LIB} 
		        #${AI3D_CORE_LIB}
				${AI3D_OSG_LIB_375}
				${AI3D_PUGIXML_LIB}
				${AI3D_GEOS_LIB}
				MoldAIData MoldAIOSGEditor )#3DViewer ProjCore


if(USE_QT)

	file( GLOB ui_list RELATIVE
	                             "${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/TheFirstWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/NewProjectWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/ProjectInfoWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/BlockWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/BlockWgtCN.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/ConstructionWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/ProductionWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/ReConstructionWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/QComboxSelf.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/ImportPosWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/ImportGcpWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/ExportXmlWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/ExportATColmapWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/ExportRecColmapWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/PosSigmaWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/ProgressCom.ui"
									 "${CMAKE_CURRENT_SOURCE_DIR}/../3DViewer/ui/*.ui"
							"${CMAKE_CURRENT_SOURCE_DIR}/ui/BatchPrepareWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/CommonDelDia.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/AddSigGcpWgt.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/message_box.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/qgsprojectionselectorbase.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/qgsgenericprojectionselectorbase.ui"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/LoginDialog.ui"
									)
									message("${CMAKE_CURRENT_SOURCE_DIR}=+++=== ${ui_list}")
	file( GLOB qrc_list RELATIVE
	                             "${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/*.qrc"
									
									)
	
				file( GLOB rc_list RELATIVE
	                             "${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/MoldAI.rc"
									#	"${CMAKE_CURRENT_SOURCE_DIR}/resource1.h"
									)
	
	Qt6_wrap_ui( generated_ui_list ${ui_list} )
	Qt6_add_resources( generated_qrc_list ${qrc_list} )
	
	set(DEPENDS_LIBS ${DEPENDS_LIBS} Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::Sql Qt6::Xml Qt6::Network)
		set(SRC_FILES ${SRC_FILES} ${generated_ui_list} ${generated_qrc_list} ${rc_list})
	
		




SOURCE_GROUP(UI\\Headeres FILES ${generated_ui_list} ${ui_list} )
SOURCE_GROUP(UI\\Sources FILES ${generated_qrc_list} ${qrc_list} ${rc_list})



endif(USE_QT)



FILE(GLOB GUI_HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 									
									${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/*.h																		
								
									)		


FILE(GLOB GUI_SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"																												
								
								)

SOURCE_GROUP(Gui\\Headeres FILES ${GUI_HEADER_LIST})
SOURCE_GROUP(Gui\\Sources FILES ${GUI_SRC_LIST} )




FILE(GLOB UTIL_HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 									
																										
									${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Util/*.h
									)		


FILE(GLOB UTIL_SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
																																				
									"${CMAKE_CURRENT_SOURCE_DIR}/../Util/*.cpp"
								)

SOURCE_GROUP(Util\\Headeres FILES ${UTIL_HEADER_LIST})
SOURCE_GROUP(Util\\Sources FILES ${UTIL_SRC_LIST} )



FILE(GLOB VIEWER_HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 									
									${CMAKE_CURRENT_SOURCE_DIR}/../../Include/3DViewer/*.h																		
									
									)		


FILE(GLOB VIEWER_SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
																																				
									"${CMAKE_CURRENT_SOURCE_DIR}/../3DViewer/*.cpp"
								)

set(VIEWER_SRC_LIST ${VIEWER_SRC_LIST} ${MODEL_SRC_LIST})
set(VIEWER_HEADER_LIST ${VIEWER_HEADER_LIST} ${MODEL_HEADER_LIST})
SOURCE_GROUP(3DViewer\\Headeres FILES ${VIEWER_HEADER_LIST})
SOURCE_GROUP(3DViewer\\Sources FILES ${VIEWER_SRC_LIST} )



add_executable( ${TARGET_NAME}  ${SRC_FILES})
if (WIN32)
 target_link_libraries(${TARGET_NAME} )
endif()
target_link_libraries(${TARGET_NAME} 
				 ${DEPENDS_LIBS}
				 ${EXTER_DEPENDS_LIBS}
				)
if(USE_WIN_DEBUG)
else(USE_WIN_DEBUG) 
	Set_Target_Properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

endif(USE_WIN_DEBUG)
set_target_properties(${TARGET_NAME} PROPERTIES  FOLDER "App")
set(COPYDLL ON)
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
			#${CMAKE_COMMAND} -E copy ${TP_ROOT_141}/expat/${EXPAT_VERSION}/dll/$(Platform)/$(Configuration)/expat%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${FREEIMAGE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/Freeimage%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${FREEXL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/freexl%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${EXPAT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libexpat%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${XLNT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/xlnt%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			#${CMAKE_COMMAND} -E copy ${QTDIR}/bin/Qt6Xml%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			
)

add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD

     COMMAND ${CMAKE_COMMAND} -E copy ${CURL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libcurl.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${GLOG_ROOT_DIR}/dll/$(Platform)/$(Configuration)/glog.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${ICONV_ROOT_DIR}/dll/$(Platform)/$(Configuration)/iconv-2.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	  COMMAND ${CMAKE_COMMAND} -E copy ${OPENCV_ROOT_DIR}/dll/$(Platform)/$(Configuration)/opencv_world347.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	  COMMAND ${CMAKE_COMMAND} -E copy ${GEOS_ROOT_DIR}/dll/$(Platform)/$(Configuration)/geos_c.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	  COMMAND ${CMAKE_COMMAND} -E copy ${GEOS_ROOT_DIR}/dll/$(Platform)/$(Configuration)/geos.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	  COMMAND ${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libcrypto-1_1-x64.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	  
)

add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
	 COMMAND ${CMAKE_COMMAND} -E copy ${AI3D_THIRDPARTY_DIR}/algcore/001/dll/$(Platform)/$(Configuration)/MoldAIProc.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
     COMMAND ${CMAKE_COMMAND} -E copy ${AI3D_THIRDPARTY_DIR}/algcore/001/dll/$(Platform)/$(Configuration)/MoldAIBase.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${AI3D_THIRDPARTY_DIR}/algcore/001/dll/$(Platform)/$(Configuration)/yunjing_client.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 
)
add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
     #COMMAND ${CMAKE_COMMAND} -E copy ${AI3D_THIRDPARTY_DIR}/packagelibs/JsonConfig.ini $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 #COMMAND ${CMAKE_COMMAND} -E copy ${AI3D_THIRDPARTY_DIR}/packagelibs/version.json $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 # COMMAND ${CMAKE_COMMAND} -E copy ${AI3D_THIRDPARTY_DIR}/packagelibs/moldainode.bat $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND   ${CMAKE_COMMAND} -E copy_directory ${AI3D_THIRDPARTY_DIR}/packagelibs  $<TARGET_FILE_DIR:${TARGET_NAME}>/\n	
	 #COMMAND   ${CMAKE_COMMAND} -E copy_directory ${AI3D_THIRDPARTY_DIR}/packagelibs/platforms  $<TARGET_FILE_DIR:${TARGET_NAME}>/platforms/\n	
	 #COMMAND   ${CMAKE_COMMAND} -E copy_directory ${AI3D_THIRDPARTY_DIR}/packagelibs/imageformats  $<TARGET_FILE_DIR:${TARGET_NAME}>/imageformats/\n	
	 )



#qt
if(USE_QT)
add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND SET postfix=\n
            IF /I $(Configuration)== Debug ( \n
                SET postfix=d\n
            ) \n
            ${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt6Core%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt6Gui%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt6OpenGL%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt6Widgets%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt6Xml%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt6Network%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			
)
endif(USE_QT)

endif(WIN32)


endif()
#set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
#target_compile_definitions(${TARGET_NAME} PRIVATE  AI3D_MAKE_DLL)
#set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME_DEBUG ${TARGET_NAME}d)