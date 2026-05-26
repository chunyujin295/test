set(TARGET_NAME testPreview)



############################### GRAB HEADERS #####################################
FILE(GLOB HEADER_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
									${CMAKE_CURRENT_SOURCE_DIR}/*.h
									#${CMAKE_CURRENT_SOURCE_DIR}/Core/*.h
									)
		

########################################### BEGIN: GRAB SOURCE FILES ##########################################
FILE(GLOB SRC_LIST RELATIVE_PATH	"${CMAKE_CURRENT_SOURCE_DIR}"
										"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"	
		
								)

set(SRC_FILES ${SRC_FILES} ${HEADER_LIST} ${SRC_LIST})

add_executable(${TARGET_NAME} ${SRC_FILES} )
AI3D_USING_GLOG()
AI3D_USING_SQLITE()

set(DEPENDS_LIBS ${EXTER_DEPENDS_LIBS}	${AI3D_GLOG_LIB} ${AI3D_SQLITE_LIB}	Core)


target_link_libraries(${TARGET_NAME}  ${DEPENDS_LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES  FOLDER "Test/Core")

set(COPYDLL OFF)
if(COPYDLL)

if (WIN32)

#add_custom_command(TARGET ${TARGET_NAME}
#    POST_BUILD
#    COMMAND SET postfix=\n
#            IF /I $(Configuration)== Debug ( \n
#                SET postfix=-d\n
#            ) \n
#            ${CMAKE_COMMAND} -E copy ${CURL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libcurl%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#
#)



add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
  COMMAND SET postfix=\n
           IF /I $(Configuration)== Debug ( \n
               SET postfix=d\n
           ) \n
            ${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/capi%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libcrypto-1_1-x64%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libssl-1_1-x64%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/dll/$(Platform)/$(Configuration)/padlock%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			
			
)



#add_custom_command(TARGET ${TARGET_NAME}
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_core.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_extra.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_openssl.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#)


endif(WIN32)

endif()
















