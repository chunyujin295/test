if (WIN32)
 add_definitions(-D_UNICODE -DUNICODE)
endif()

set(TARGET_NAME MoldAITask)

AI3D_USING_CERES()
AI3D_USING_CURL()
AI3D_USING_EVENT()
AI3D_USING_OPENSSL()
AI3D_USING_RAPIDJSON()
AI3D_USING_CORE()
AI3D_USING_EXPAT()
AI3D_USING_BOOST()
#AI3D_USING_OSG_340()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include	)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/engine	)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../	)

		
############################### GRAB HEADERS #####################################
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
									${CMAKE_CURRENT_SOURCE_DIR}/*.h
									#${CMAKE_CURRENT_SOURCE_DIR}/../../include/engine/*.h	
									${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/CatchProcess.h									
							${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/Settings.h
									${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/TaskProcess.h
									${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/Statistic.h
									${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/JobMonitor.h
									${CMAKE_CURRENT_SOURCE_DIR}/../../include/Util/OTA.h	
									)		

########################################### BEGIN: GRAB SOURCE FILES ##########################################
FILE(GLOB SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"										
							"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Util/CatchProcess.cpp"	
							"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Util/Settings.cpp"			
							"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Util/TaskProcess.cpp"	
							"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Util/Statistic.cpp"	
							"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Util/JobMonitor.cpp"	
							"${CMAKE_CURRENT_SOURCE_DIR}/../../Src/Util/OTA.cpp"				
								)






set(SRC_FILES ${SRC_LIST} ${HEADER_LIST})
set(DEPENDS_LIBS MoldAIData ${AI3D_CORE_LIB} 	${AI3D_CURL_LIB} ${AI3D_OPENSSL_LIB}  	${AI3D_EVENT_LIB} 
				${AI3D_EXPAT_LIB}  ${AI3D_BOOST_LIB}  ${AI3D_CERES_LIB}
				)

set(DEPENDS_LIBS ${DEPENDS_LIBS} Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::Sql Qt6::Xml Qt6::Network)

add_executable( ${TARGET_NAME}  ${SRC_FILES}  "MoldAINode.rc")
if(WIN32)
set(DEPENDS_LIBS  ${DEPENDS_LIBS} "ws2_32.lib")
endif(WIN32)
target_link_libraries(${TARGET_NAME} 
				 ${DEPENDS_LIBS}
				)


set_target_properties(${TARGET_NAME} PROPERTIES  FOLDER "App")
set(COPY OFF)
if(COPY)
if (WIN32)

#add_custom_command(TARGET ${TARGET_NAME}
#    POST_BUILD
#    COMMAND SET postfix=\n
#            IF /I $(Configuration)== Debug ( \n
#                SET postfix=-d\n
#            ) \n
#            ${CMAKE_COMMAND} -E copy ${CURL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libcurl%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#			#   ${CMAKE_COMMAND} -E copy ${EXPAT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/expat%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#
#)

add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/MoldAIBase.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
    COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/MoldAIProc.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
    COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/expat.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
   
    COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/liblas.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
    COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/opencv_world${OPENCV_VERSION}.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libraw.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/capi.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libcrypto-1_1-x64.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libgcc_s_seh-1.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libgcc_s_sjlj-1.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libgfortran-3.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/liblapack.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/liblas.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libopenblas.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libquadmath-0.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/yunjing_client.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
)




add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_core.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_extra.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_openssl.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy ${ICONV_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libiconv.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
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
		#	${CMAKE_COMMAND} -E copy $(TargetDir)/$(TargetName).exe  $(TargetDir)/MoldAINode.exe \n	
		#	${CMAKE_COMMAND} -E copy $(TargetDir)/$(TargetName).exe  $(TargetDir)/MoldAITask.exe \n
			
)
endif(USE_QT)

endif(WIN32)
endif(COPY)