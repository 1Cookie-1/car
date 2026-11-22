# 用户代码源文件清单。新增 .c 必须在这里登记。
set(USER_SOURCES
    # BSP
    # Modules
    ${CMAKE_CURRENT_SOURCE_DIR}/Modules/chassis/mecanum.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Modules/pid/pid.c
    # Application
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/scheduler/scheduler.c
    # Common
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Src/ring_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Src/crc8.c
)

set(USER_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/BSP/board
    ${CMAKE_CURRENT_SOURCE_DIR}/BSP/stepper
    ${CMAKE_CURRENT_SOURCE_DIR}/BSP/servo
    ${CMAKE_CURRENT_SOURCE_DIR}/BSP/uart
    ${CMAKE_CURRENT_SOURCE_DIR}/BSP/input
    ${CMAKE_CURRENT_SOURCE_DIR}/Modules/pid
    ${CMAKE_CURRENT_SOURCE_DIR}/Modules/chassis
    ${CMAKE_CURRENT_SOURCE_DIR}/Modules/comm
    ${CMAKE_CURRENT_SOURCE_DIR}/Application
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/service
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/scheduler
    ${CMAKE_CURRENT_SOURCE_DIR}/Application/task
)
