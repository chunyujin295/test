
set(TARGET_NAME MoldAIUpload)


AI3D_USING_CURL()
AI3D_USING_EVENT()
AI3D_USING_OPENSSL()


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../include	)



#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../	)

		
############################### GRAB HEADERS #####################################
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
									${CMAKE_CURRENT_SOURCE_DIR}/*.h
									${CMAKE_CURRENT_SOURCE_DIR}/../../../include/Util/OTA.h	
																		
						
									
									)		

########################################### BEGIN: GRAB SOURCE FILES ##########################################
FILE(GLOB SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
									"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"										
							"${CMAKE_CURRENT_SOURCE_DIR}/../../../Src/Util/OTA.cpp"				
								)






#SOURCE_GROUP(Engine\\Headeres FILES ${ENGINE_HEADERS})
#SOURCE_GROUP(Engine\\Sources FILES ${ENGINE_SRCS})




set(SRC_FILES ${SRC_LIST} ${HEADER_LIST})
set(DEPENDS_LIBS  	${AI3D_CURL_LIB} ${AI3D_OPENSSL_LIB}  	${AI3D_EVENT_LIB} 
				${AI3D_EXPAT_LIB}  
				)

set(DEPENDS_LIBS ${DEPENDS_LIBS} Qt5::Core Qt5::Widgets Qt5::OpenGL Qt5::Sql Qt5::Xml )#Qt5::Network

add_executable( ${TARGET_NAME}  ${SRC_FILES}  "MoldAINode.rc")
if(WIN32)
set(DEPENDS_LIBS  ${DEPENDS_LIBS} Qt5::WinMain "ws2_32.lib")
endif(WIN32)
target_link_libraries(${TARGET_NAME} 
				 ${DEPENDS_LIBS}
				)




set_target_properties(${TARGET_NAME} PROPERTIES  FOLDER "App")


#if (WIN32)
#
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
#
#
#add_custom_command(TARGET ${TARGET_NAME}
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy ${AI3DCORE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/expat.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#   
#	
#	
#)
#
#
#
#
add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_core.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_extra.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_openssl.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#	COMMAND ${CMAKE_COMMAND} -E copy ${ICONV_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libiconv.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
	COMMAND ${CMAKE_COMMAND} -E copy $(TargetDir)/$(TargetName).exe  $(TargetDir)/MoldAICopy.exe \n
)

#
##qt
#if(USE_QT)
#add_custom_command(TARGET ${TARGET_NAME}
#    POST_BUILD
#    COMMAND SET postfix=\n
#            IF /I $(Configuration)== Debug ( \n
#                SET postfix=d\n
#            ) \n
#            ${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt5Core%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt5Gui%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt5OpenGL%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt5Widgets%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#			${CMAKE_COMMAND} -E copy $ENV{QTDIR}/bin/Qt5Xml%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#			
#)
#endif(USE_QT)
#
#endif(WIN32)
#
