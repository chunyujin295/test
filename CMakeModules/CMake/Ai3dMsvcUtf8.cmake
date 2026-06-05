# MSVC: UTF-8 source + execution charset (/utf-8).
# Replaces legacy /source-charset:utf-8 + /execution-charset:gbk (do not re-enable GBK execution charset).
if(MSVC)
    foreach(_flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL)
        if(DEFINED ${_flag_var})
            string(REPLACE "/execution-charset:gbk" "" ${_flag_var} "${${_flag_var}}")
            string(REPLACE "/source-charset:utf-8" "" ${_flag_var} "${${_flag_var}}")
            string(REGEX REPLACE " */utf-8" "" ${_flag_var} "${${_flag_var}}")
        endif()
    endforeach()
    add_compile_options(/utf-8)
endif()
