set(TARGET_NAME MoldAITools)


AI3D_USING_EIGEN()
AI3D_USING_CERES()
AI3D_USING_BOOST()
AI3D_USING_EXIV2()
AI3D_USING_OPENCV()	
AI3D_USING_PROJ()
AI3D_USING_GEOGRAPHICLIB()
AI3D_USING_SQLITE()

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

set(SRC_FILES ${SRC_FILES} ${HEADER_LIST} ${SRC_LIST}
        ../testPoints/testPointManager.cpp
		../testPoints/testFreezeSubmit.cpp
		../testPoints/testSettleEngine.cpp
		../testPoints/testPointsDistributed.cpp)

add_executable(${TARGET_NAME} ${SRC_FILES} )

set(DEPENDS_LIBS  MoldAIData ${AI3D_OPENCV_LIB}   ${AI3D_EXIV2_LIB} ${AI3D_GEOGRAPHICLIB_LIB}) 
	
set(DEPENDS_LIBS ${DEPENDS_LIBS} )

target_link_libraries(${TARGET_NAME}  ${DEPENDS_LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES  FOLDER "Test")
