set(TARGET_NAME testGenTaskPhase1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(/Zc:__cplusplus)
add_compile_options(/permissive-)

############################### GRAB HEADERS #####################################
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
									${CMAKE_CURRENT_SOURCE_DIR}/*.h
									#${CMAKE_CURRENT_SOURCE_DIR}/Core/*.h
									)


########################################### BEGIN: GRAB SOURCE FILES ##########################################
FILE(GLOB SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
										"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp"
#"${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cc"	 _adjustment
								)

set(SRC_FILES ${SRC_FILES} ${HEADER_LIST} ${SRC_LIST})

add_executable(${TARGET_NAME} ${SRC_FILES} )

AI3D_USING_GLOG()

set(DEPENDS_LIBS MoldAIData ${EXTER_DEPENDS_LIBS})

target_link_libraries(${TARGET_NAME}  ${DEPENDS_LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES  FOLDER "Test")