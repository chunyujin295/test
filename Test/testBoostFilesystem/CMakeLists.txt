set(TARGET_NAME testProj)

# all files exclude Test* files
#file(GLOB_RECURSE SRC_FILES *.cpp *.c *.h)

AI3D_USING_PROJ()
AI3D_USING_SQLITE()


############################### GRAB HEADERS #####################################
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
									${CMAKE_CURRENT_SOURCE_DIR}/*.h
									#${CMAKE_CURRENT_SOURCE_DIR}/../*.h
									)
		

########################################### BEGIN: GRAB SOURCE FILES ##########################################
FILE(GLOB SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
										"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"	
	#"${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp"	
#"${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cc"	
								)
set(SRC_FILES ${SRC_FILES} ${HEADER_LIST} ${SRC_LIST})

add_executable(${TARGET_NAME} ${SRC_FILES} )

set(DEPENDS_LIBS  ${DEPENDS_LIBS} ${AI3D_PROJ_LIB}  ${AI3D_SQLITE_LIB} ProjCore )



target_link_libraries(${TARGET_NAME}  ${DEPENDS_LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES  FOLDER "Test")



















