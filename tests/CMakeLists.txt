cmake_minimum_required(VERSION 3.16)
project(car_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

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

add_executable(test_ring_buffer_invalid_size
    common/test_ring_buffer_invalid_size.c
    ../Common/Src/ring_buffer.c
)
target_include_directories(test_ring_buffer_invalid_size PRIVATE ../Common/Inc)

add_executable(test_pid_invalid_dt
    modules/test_pid_invalid_dt.c
    ../Modules/pid/pid.c
)
target_include_directories(test_pid_invalid_dt PRIVATE ../Modules/pid)

add_executable(test_ring_buffer_null
    common/test_ring_buffer_null.c
    ../Common/Src/ring_buffer.c
)
target_include_directories(test_ring_buffer_null PRIVATE ../Common/Inc)

add_executable(test_pid_invalid_inputs
    modules/test_pid_invalid_inputs.c
    ../Modules/pid/pid.c
)
target_include_directories(test_pid_invalid_inputs PRIVATE ../Modules/pid)

add_executable(test_pid_overflow
    modules/test_pid_overflow.c
    ../Modules/pid/pid.c
)
target_include_directories(test_pid_overflow PRIVATE ../Modules/pid)

add_executable(test_scheduler_extra
    modules/test_scheduler_extra.c
    ../Application/scheduler/scheduler.c
)
target_include_directories(test_scheduler_extra PRIVATE ../Application/scheduler)

add_executable(test_mecanum_extra
    modules/test_mecanum_extra.c
    ../Modules/chassis/mecanum.c
)
target_include_directories(test_mecanum_extra PRIVATE ../Modules/chassis)

add_test(NAME pid_invalid_dt COMMAND test_pid_invalid_dt)
add_test(NAME ring_buffer_invalid_size COMMAND test_ring_buffer_invalid_size)
add_test(NAME ring_buffer COMMAND test_ring_buffer)
add_test(NAME crc8 COMMAND test_crc8)
add_test(NAME mecanum COMMAND test_mecanum)
add_test(NAME pid COMMAND test_pid)
add_test(NAME scheduler COMMAND test_scheduler)
add_test(NAME ring_buffer_null COMMAND test_ring_buffer_null)
add_test(NAME pid_invalid_inputs COMMAND test_pid_invalid_inputs)
add_test(NAME pid_overflow COMMAND test_pid_overflow)
add_test(NAME scheduler_extra COMMAND test_scheduler_extra)
add_test(NAME mecanum_extra COMMAND test_mecanum_extra)
