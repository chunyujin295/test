IF(USE_QT)
# ------------------------------------------------------------------------------
# Qt
# ------------------------------------------------------------------------------
## we will use cmake automoc feature
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( QT6_ROOT_PATH CACHE PATH "Qt6 root directory (i.e. where the 'bin' folder lies)" )
if ( QT6_ROOT_PATH )
	list( APPEND CMAKE_PREFIX_PATH ${QT6_ROOT_PATH} )
endif()

# find qt6 components
find_package(Qt6 COMPONENTS OpenGL Widgets Core Gui PrintSupport Concurrent Svg Xml Sql Network REQUIRED)
#find_package(Qt6Widgets REQUIRED)
#find_package(Qt6Core REQUIRED)
#find_package(Qt6Gui REQUIRED)
#find_package(Qt6PrintSupport REQUIRED)
#find_package(Qt6Concurrent REQUIRED)
#find_package(Qt6OpenGL REQUIRED)
#find_package(Qt6OpenGLExtensions REQUIRED)
#find_package(Qt6Svg REQUIRED)
#find_package(Qt6Xml REQUIRED)
#find_package(Qt6Sql REQUIRED)
#find_package(Qt6Network REQUIRED)
# in the case no Qt6Config.cmake file could be found, cmake will explicitly ask the user for the QT6_DIR containing it!
# thus no need to keep additional variables and checks

# Starting with the QtCore lib, find the bin and root directories
get_target_property(QT6_LIB_LOCATION Qt6::Core LOCATION_${CMAKE_BUILD_TYPE})
get_filename_component(QT_BINARY_DIR ${QT6_LIB_LOCATION} DIRECTORY)

# Apple uses frameworks - move up until we get to the base directory to set the bin directory properly
if ( APPLE )
	get_filename_component(QT_BINARY_DIR ${QT_BINARY_DIR} DIRECTORY)
	get_filename_component(QT_BINARY_DIR ${QT_BINARY_DIR} DIRECTORY)
	set(QT_BINARY_DIR "${QT_BINARY_DIR}/bin")	

	set( MACDEPLOYQT "${QT_BINARY_DIR}/macdeployqt" )
endif()

# set QT6_ROOT_PATH if it wasn't set by the user
if ( NOT QT6_ROOT_PATH )
	get_filename_component(QT6_ROOT_PATH ${QT_BINARY_DIR} DIRECTORY)
endif()

include_directories(${Qt6OpenGL_INCLUDE_DIRS}
                    ${Qt6Widgets_INCLUDE_DIRS}
                    ${Qt6Core_INCLUDE_DIRS}
                    ${Qt6Gui_INCLUDE_DIRS}
					${Qt6Xml_INCLUDE_DIRS}
					${Qt6Sql_INCLUDE_DIRS}
						${Qt6Network_INCLUDE_DIRS}
                    ${Qt6Concurrent_INCLUDE_DIRS}
                    ${Qt6PrintSupport_INCLUDE_DIRS}
					)

# turn on QStringBuilder for more efficient string construction
#	see https://doc.qt.io/qt-5/qstring.html#more-efficient-string-construction
add_definitions( -DQT_USE_QSTRINGBUILDER )
				
endif(USE_QT)

if(USE_OPENGL)
# ------------------------------------------------------------------------------
# OpenGL
# ------------------------------------------------------------------------------
if ( MSVC )
	# Where to find OpenGL libraries
	set(WINDOWS_OPENGL_LIBS "C:\\Program Files (x86)\\Windows Kits\\8.0\\Lib\\win8\\um\\x64" CACHE PATH "WindowsSDK libraries" )
	list( APPEND CMAKE_PREFIX_PATH ${WINDOWS_OPENGL_LIBS} )
endif()
endif(USE_OPENGL)	

if(USE_OPENMP)			
# ------------------------------------------------------------------------------
# OpenMP
# ------------------------------------------------------------------------------
find_package(OpenMP QUIET)
if (OPENMP_FOUND)
	message("OpenMP found")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	ADD_DEFINITIONS(-DUSE_OPENMP)						  
endif()
endif(USE_OPENMP)
# Intel's Threading Building Blocks (TBB)
if (USE_TBB)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_TBB")
endif()

