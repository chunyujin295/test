set(TARGET_NAME testKMLIO)

# all files exclude Test* files
#file(GLOB_RECURSE SRC_FILES *.cpp *.c *.h)

AI3D_USING_GLOG()
AI3D_USING_BOOST()
AI3D_USING_EXPAT()
AI3D_USING_GDAL()
AI3D_USING_KML()
AI3D_USING_PROJ()


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

set(DEPENDS_LIBS  ${AI3D_SQLITE_LIB}  ${AI3D_GDAL_LIB}  
		${AI3D_GFLAGS_LIB}  ${AI3D_PROJ_LIB} ${AI3D_KML_LIB} ${AI3D_EXPAT_LIB}  ${AI3D_BOOST_LIB} )
#set(DEPENDS_LIBS ${DEPENDS_LIBS} Qt6::Core Qt6::Gui)

#if (WIN32)
 #target_link_libraries(${TARGET_NAME} Qt6::WinMain)
#endif()

target_link_libraries(${TARGET_NAME}  ${DEPENDS_LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES  FOLDER "Test")


set(COPYDLL ON)
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
            ${CMAKE_COMMAND} -E copy ${ZLIB_ROOT_DIR}/dll/$(Platform)/$(Configuration)/zlib%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${EXIV2_ROOT_DIR}/dll/$(Platform)/$(Configuration)/exiv2%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			#${CMAKE_COMMAND} -E copy ${GLOG_ROOT_DIR}/dll/$(Platform)/$(Configuration)/glog%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${EXPAT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/libexpat%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			${CMAKE_COMMAND} -E copy ${FREEIMAGE_ROOT_DIR}/dll/$(Platform)/$(Configuration)/freeimage%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			#    ${CMAKE_COMMAND} -E copy ${QTDIR}/bin/Qt6Xml%postfix%.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
			
)



#add_custom_command(TARGET ${TARGET_NAME}
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_core.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_extra.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#    COMMAND ${CMAKE_COMMAND} -E copy ${EVENT_ROOT_DIR}/dll/$(Platform)/$(Configuration)/event_openssl.dll $<TARGET_FILE_DIR:${TARGET_NAME}>\n
#)

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
)
endif(USE_QT)

endif(WIN32)
endif()

















