if(USE_QT AND POLICY CMP0020)
	cmake_policy(SET CMP0020 NEW)
endif()
set(TARGET_NAME testAI3DImportGcp)

#AI3D_USING_GLOG()
AI3D_USING_SQLITE()
AI3D_USING_EXIV2()
#AI3D_USING_GFLAGS()
#AI3D_USING_FREEIMAGE()
#AI3D_USING_BOOST()
#AI3D_USING_EXIV2()
AI3D_USING_GDAL()
AI3D_USING_PROJ()
#AI3D_USING_GEOGRAPHICLIB()
#AI3D_USING_EXPAT()
#AI3D_USING_CURL()
	
############################### GRAB HEADERS #####################################
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
									${CMAKE_CURRENT_SOURCE_DIR}/*.h	
									#${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/ControlPointsEditorWin.h
									#${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/GraphicsView.h
									#${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/QSensorIntDelegate.h
									#${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/QReadOnlyDelegate.h
									#${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/ControlPoint2DNode.h
									#${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/ControlPointCircleCross.h
									#${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Core/AlgorithmBase.h
									#${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Core/CoordinateSystem.h									
)		

########################################### BEGIN: GRAB SOURCE FILES ##########################################
FILE(GLOB SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
									#"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Gui/ControlPointsEditorWin.cpp"	
									#"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Gui/GraphicsView.cpp"	
									#"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Gui/QSensorIntDelegate.cpp"	
									#"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Gui/QReadOnlyDelegate.cpp"
									#"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Gui/ControlPoint2DNode.cpp"	
									#"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Gui/ControlPointCircleCross.cpp"
									"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"	
								)


set(SRC_FILES ${SRC_LIST} ${HEADER_LIST})
#set(DEPENDS_LIBS CORE)


if(USE_QT)

	file( GLOB ui_list RELATIVE
	                             "${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/ui/*.ui"
								
									)
	#file( GLOB qrc_list RELATIVE
	 #                           "${CMAKE_CURRENT_SOURCE_DIR}"
								#	"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Gui/*.qrc"
									
		#							)
	

	find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
	qt6_wrap_ui( generated_ui_list ${ui_list} )
	#Qt6_add_resources( generated_qrc_list ${qrc_list} )
	message("**** ${generated_ui_list}")
	set(DEPENDS_LIBS ${DEPENDS_LIBS} Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::Sql Qt6::Xml)
		set(SRC_FILES ${SRC_FILES} ${generated_ui_list} )# ${rc_list}) ${generated_qrc_list}
	
		




SOURCE_GROUP(UI\\Headeres FILES ${generated_ui_list} ${ui_list})
#SOURCE_GROUP(UI\\Sources FILES ${generated_qrc_list} ${qrc_list})






endif(USE_QT)
#add_library(${TARGET_NAME} SHARED ${SRC_FILES} ) #STATIC  SHARED
add_executable( ${TARGET_NAME}  ${SRC_FILES})

set(DEPENDS_LIBS   ${DEPENDS_LIBS}#${AI3D_GLOG_LIB} ${AI3D_CURL_LIB}
${AI3D_SQLITE_LIB} ${AI3D_EXIV2_LIB}   ${AI3D_GDAL_LIB}#${AI3D_GFLAGS_LIB} ${AI3D_FREEIMAGE_LIB} ${AI3D_BOOST_LIB} ${DEPENDS_LIBS} 
 #${AI3D_EXPAT_LIB} ${AI3D_GDAL_LIB} ${AI3D_GEOGRAPHICLIB_LIB}
 ${AI3D_PROJ_LIB} MoldAIData )#Core

if (WIN32)
	find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
	target_link_libraries(${TARGET_NAME})
endif()

target_link_libraries(${TARGET_NAME} 
				 ${DEPENDS_LIBS}
				)


set_target_properties(${TARGET_NAME} PROPERTIES  FOLDER "Test")
#set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)
#target_compile_definitions(${TARGET_NAME} PRIVATE  AI3D_MAKE_DLL)
#set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME_DEBUG ${TARGET_NAME}d)