cmake_minimum_required(VERSION 3.16)
project(car_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_testing()

add_executable(test_ring_buffer
    common/test_ring_buffer.c
    ../Common/Src/ring_buffer.c
)
target_include_directories(test_ring_buffer PRIVATE ../Common/Inc)

add_executable(test_crc8
    common/test_crc8.c
    ../Common/Src/crc8.c
)
target_include_directories(test_crc8 PRIVATE ../Common/Inc)

add_executable(test_mecanum
    modules/test_mecanum.c
    ../Modules/chassis/mecanum.c
)
target_include_directories(test_mecanum PRIVATE ../Modules/chassis)

add_executable(test_pid
    modules/test_pid.c
    ../Modules/pid/pid.c
)
target_include_directories(test_pid PRIVATE ../Modules/pid)

add_executable(test_scheduler
    modules/test_scheduler.c
    ../Application/scheduler/scheduler.c
)
target_include_directories(test_scheduler PRIVATE ../Application/scheduler)

add_test(NAME ring_buffer COMMAND test_ring_buffer)
add_test(NAME crc8 COMMAND test_crc8)
add_test(NAME mecanum COMMAND test_mecanum)
add_test(NAME pid COMMAND test_pid)
add_test(NAME scheduler COMMAND test_scheduler)
