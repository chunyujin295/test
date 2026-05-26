# Install script for directory: D:/Code/moldaiengine/Test

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/MoldAI")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/Code/moldaiengine/out/build/debug/Test/testAI3DBlock/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testOSGNodeCollect/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testRenameModelName/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testAI3DImportGcp/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testSimplifyBlock/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/Core/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testHist/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testTiling/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testConvertTo3DView/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testOSGEditor/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testProjTemp/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testMergeModel/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testPackage/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testXML2Colmap/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testXML2Colmap2/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testKMLIO/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testExifIO/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/MokTools/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testBoostFilesystem/cmake_install.cmake")
  include("D:/Code/moldaiengine/out/build/debug/Test/testDeviceInfo/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "D:/Code/moldaiengine/out/build/debug/Test/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
