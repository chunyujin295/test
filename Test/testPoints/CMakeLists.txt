# Test/testPoints/CMakeLists.txt

# ===== target 1: PointManager API 单元测试 =====
set(TARGET_NAME testPointManager)
FILE(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/testPointManager.cpp")
add_executable(${TARGET_NAME} ${SRC})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Include)
target_compile_definitions(${TARGET_NAME} PRIVATE MOCK_POINT_MANAGER)
AI3D_USING_GLOG()
set(DEPENDS_LIBS ${EXTER_DEPENDS_LIBS} ${AI3D_GLOG_LIB} Core Qt6::Core Qt6::Network)
target_link_libraries(${TARGET_NAME} ${DEPENDS_LIBS})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Test")

# ===== target 2: 提交时冻结流程测试 =====
set(TARGET_NAME testFreezeSubmit)
FILE(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/testFreezeSubmit.cpp")
add_executable(${TARGET_NAME} ${SRC})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Include)
target_compile_definitions(${TARGET_NAME} PRIVATE MOCK_POINT_MANAGER)
AI3D_USING_GLOG()
set(DEPENDS_LIBS ${EXTER_DEPENDS_LIBS} ${AI3D_GLOG_LIB} Core Qt6::Core Qt6::Network Qt6::Gui Qt6::Widgets)
target_link_libraries(${TARGET_NAME} ${DEPENDS_LIBS})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Test")

# ===== target 3: Engine 结算流程测试 (含哨兵机制) =====
set(TARGET_NAME testSettleEngine)
FILE(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/testSettleEngine.cpp")
add_executable(${TARGET_NAME} ${SRC})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Include)
target_compile_definitions(${TARGET_NAME} PRIVATE MOCK_POINT_MANAGER)
AI3D_USING_GLOG()
set(DEPENDS_LIBS ${EXTER_DEPENDS_LIBS} ${AI3D_GLOG_LIB} Core Qt6::Core Qt6::Network Qt6::Gui Qt6::Widgets)
target_link_libraries(${TARGET_NAME} ${DEPENDS_LIBS})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Test")

# ===== target 4: 分布式 + 异常场景测试 =====
set(TARGET_NAME testPointsDistributed)
FILE(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/testPointsDistributed.cpp")
add_executable(${TARGET_NAME} ${SRC})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Include)
target_compile_definitions(${TARGET_NAME} PRIVATE MOCK_POINT_MANAGER)
AI3D_USING_GLOG()
set(DEPENDS_LIBS ${EXTER_DEPENDS_LIBS} ${AI3D_GLOG_LIB} Core Qt6::Core Qt6::Network Qt6::Gui Qt6::Widgets)
target_link_libraries(${TARGET_NAME} ${DEPENDS_LIBS})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Test")