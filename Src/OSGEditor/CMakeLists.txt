if (WIN32)
 # MSVC UTF-8 (source + execution): use root /utf-8; former per-config flags were:
 # add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>>:/source-charset:utf-8>")
 # add_compile_options("$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>>:/execution-charset:gbk>")
 add_definitions(-D_UNICODE -DUNICODE)
endif()

set(TARGET_NAME MoldAIOSGEditor)

AI3D_USING_OSG_375()

find_package(OpenGL REQUIRED)
AI3D_USING_GLEW()
AI3D_USING_GEOGRAPHICLIB()
AI3D_USING_GEOS()
AI3D_USING_CERES()
AI3D_USING_VCG()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../	)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Include )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Include/OSGEditor )
	
############################### GRAB HEADERS #####################################
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
									
									${CMAKE_CURRENT_SOURCE_DIR}/../../Include/OSGEditor/*.h
									
									)		
#LIST(REMOVE_ITEM HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/../../Include/Gui/ControlPointsEditorWin.h)
########################################### BEGIN: GRAB SOURCE FILES ##########################################
FILE(GLOB SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"										
									
								)
								

ADD_DEFINITIONS("-DAI3D_MAKE_DLL")

set(SRC_FILES ${SRC_LIST} ${HEADER_LIST})

set(DEPENDS_LIBS ${DEPENDS_LIBS} ${AI3D_CERES_LIB} ${AI3D_GEOS_LIB} MoldAIData ${AI3D_OSG_LIB_375} )

add_library(${TARGET_NAME} SHARED ${SRC_FILES} )


target_link_libraries(${TARGET_NAME} 
				 ${DEPENDS_LIBS}
				# ${EXTER_DEPENDS_LIBS}
				)

set_target_properties(${TARGET_NAME} PROPERTIES  FOLDER "Libs")
add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
  
  COMMAND   ${CMAKE_COMMAND} -E copy_directory ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgPlugins-3.6.5  $<TARGET_FILE_DIR:${TARGET_NAME}>/osgPlugins-3.6.5/\n	
  )
  add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osg.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgText.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgDB.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/OpenThreads.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	  COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgUtil.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	   COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgGA.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	    COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgViewer.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
		 COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgParticle.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
		 
		 
		  COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgUI.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgVolume.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
		 
		 COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgWidget.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgAnimation.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgFX.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	 COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgManipulator.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	  COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgPresentation.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	   COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgShadow.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	    COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgSim.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
		 COMMAND ${CMAKE_COMMAND} -E copy ${OSG_ROOT_DIR_375}/dll/$(Platform)/$(Configuration)/osgTerrain.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
)
