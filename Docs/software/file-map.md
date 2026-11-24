# 软件文件地图（file-map）

本表只记录当前仓库中真实存在的文件与目录内容；每一行的职责与调用关系均来自实际读取的文件内容和 `grep` 检索结果。

## 一、文件职责表

| 路径 | 实际职责 | 状态 | 被谁调用 |
| --- | --- | --- | --- |
| Application/scheduler/scheduler.h | 周期任务调度器接口声明：`SchedTaskFn` 函数指针类型，`Sched_Init`/`Sched_Register`/`Sched_Run`/`Sched_TaskCount` 原型，以及“任务执行期间注册表不可修改”的注释约定 | 只有声明 | 被 `Application/scheduler/scheduler.c`、`tests/modules/test_scheduler.c` 包含 |
| Application/scheduler/scheduler.c | 周期任务调度器实现：静态数组 `s_tasks` 最多容纳 8 个任务；`Sched_Register` 做空指针/周期为 0/重复注册检查；`Sched_Run` 用 `(int32_t)(now_ms - due) >= 0` 判断到期，兼容 uint32 时间回绕；`Sched_TaskCount` 返回注册数 | 已实现 | `Sched_Init`/`Sched_Register`/`Sched_Run`/`Sched_TaskCount` 仅被 `tests/modules/test_scheduler.c` 调用；生产代码未被引用 |
| Modules/chassis/motor_if.h | 电机抽象接口声明：`MotorId` 枚举（FL/FR/RL/RR/COUNT），`MotorIf` 结构（set_speed/stop/stop_all/get_position 四个函数指针），全局实例 `g_motor` 声明 | 只有声明 | 被 `Modules/chassis/mecanum.h` 包含；`g_motor` 未被引用 |
| Modules/chassis/mecanum.h | 麦轮运动学接口声明：`ChassisVelocity` 结构（vx/vy/wz 及单位注释），`Mecanum_Solve` 原型 | 只有声明 | 被 `Modules/chassis/mecanum.c`、`tests/modules/test_mecanum.c` 包含 |
| Modules/chassis/mecanum.c | 四轮麦轮逆运动学解算实现 `Mecanum_Solve`：由 vx/vy/wz 和半长、半宽计算四个轮子的线速度 | 已实现 | `Mecanum_Solve` 仅被 `tests/modules/test_mecanum.c` 调用；生产代码未被引用 |
| Modules/pid/pid.h | 位置式 PID 接口声明：`Pid` 结构（kp/ki/kd、积分限幅、输出限幅、微分先行状态），`Pid_Init`/`Pid_Update`/`Pid_Reset` 原型 | 只有声明 | 被 `Modules/pid/pid.c`、`tests/modules/test_pid.c`、`tests/modules/test_pid_invalid_dt.c` 包含 |
| Modules/pid/pid.c | 位置式 PID 实现：积分限幅（anti-windup）、微分先行（derivative on measurement）、输出限幅；`dt <= 0` 时返回 0 且不修改状态；含静态辅助函数 `clampf` | 已实现 |`Pid_Init`/`Pid_Update` 仅被 `tests`/`modules`/`test_pid.c`、`tests`/`modules`/`test_pid_invalid_dt.c` 调用；`Pid_Reset` 无调用点，仅 `pid.h:26` 声明与 `pid.c:27` 定义；生产代码未被引用 |
| BSP/board | 空 | 空目录 | 未被引用 |
| BSP/input | 空 | 空目录 | 未被引用 |
| BSP/servo | 空 | 空目录 | 未被引用 |
| BSP/stepper | 空 | 空目录 | 未被引用 |
| BSP/uart | 空 | 空目录 | 未被引用 |
| Common/Inc/crc8.h | CRC-8 接口与参数声明：多项式 0x07、初值 0x00、XOROUT 0x00；`Crc8_Update`/`Crc8_Compute` 原型 | 只有声明 | 被 `Common/Src/crc8.c`、`tests/common/test_crc8.c` 包含 |
| Common/Src/crc8.c | CRC-8 实现：`Crc8_Update` 逐字节位运算；`Crc8_Compute` 遍历数据、以初值 0x00 计算并异或 XOROUT | 已实现 | `Crc8_Compute` 被 `tests/common/test_crc8.c` 调用；`Crc8_Update` 仅被本文件 `Crc8_Compute` 内部调用；生产代码未被引用 |
| Common/Inc/ring_buffer.h | 环形缓冲区接口声明：`RingBuffer` 结构（buf/size/volatile head/tail），`RingBuffer_Init`/`Put`/`Get`/`Count`/`IsEmpty`/`IsFull` 原型；注释约定单生产者/单消费者、保留一个空位、size<2 视为无效 | 只有声明 | 被 `Common/Src/ring_buffer.c`、`tests/common/test_ring_buffer.c`、`tests/common/test_ring_buffer_invalid_size.c` 包含 |
| Common/Src/ring_buffer.c | 环形缓冲区实现：初始化、读写、计数、空/满判断；size<2 时初始化为无效缓冲区且 Put/Get 返回 false；无锁，单生产者/单消费者 | 已实现 | `RingBuffer_*` 仅被 `tests/common/test_ring_buffer.c`、`tests/common/test_ring_buffer_invalid_size.c` 调用；生产代码未被引用 |
| Config | 空 | 空目录 | 未被引用 |
| Core/Src/main.c | CubeMX 生成入口：`HAL_Init`、`SystemClock_Config`（HSE、PLL 开启、FLASH_LATENCY_5）、`MX_GPIO_Init`（使能 GPIOH/GPIOA/GPIOE 时钟）、空的 `while(1)`；USER CODE 区为空，未调用 `Board_Init`/`App_Init`/`App_Loop` | 已实现 | 被 `startup_stm32f407xx.s` 的 `Reset_Handler` 调用（`bl main`）；内部调用 `HAL_Init`/`SystemClock_Config`/`MX_GPIO_Init`/`MX_TIM6_Init`/`MX_TIM1_Init`/`MX_USART2_UART_Init`/`Error_Handler` |
| Core/Inc/main.h | main.c 头文件：包含 `stm32f4xx_hal.h`，声明 `Error_Handler`、`HAL_TIM_MspPostInit` | 只有声明 | 被 `Core/Src/main.c`、`Core/Src/stm32f4xx_it.c`、`Core/Src/stm32f4xx_hal_msp.c` 包含 |
| Core/Inc/stm32f4xx_it.h | 中断处理函数原型声明：NMI/HardFault/MemManage/BusFault/UsageFault/SVC/DebugMon/PendSV/SysTick，以及外设中断 USART2_IRQHandler/TIM6_DAC_IRQHandler（共 11 个） | 只有声明 | 被 `Core/Src/stm32f4xx_it.c` 包含 |
| Core/Inc/stm32f4xx_hal_conf.h | HAL 配置：使能 HAL_TIM/UART/GPIO/EXTI/DMA/RCC/FLASH/PWR/CORTEX 模块；定义 HSE_VALUE/HSI_VALUE/TICK_INT_PRIORITY 等宏，并按使能模块包含对应 HAL 头 | 只有声明（配置宏） | 被 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` 包含 |
| Core/Src/stm32f4xx_it.c | 异常/中断服务例程实现：NMI/HardFault/MemManage/BusFault/UsageFault/SVC/DebugMon/PendSV/SysTick；`SysTick_Handler` 调用 `HAL_IncTick` | 已实现 | 各处理函数经 `startup_stm32f407xx.s` 向量表由硬件触发；`SysTick_Handler` 调用 `HAL_IncTick` |
| Core/Src/stm32f4xx_hal_msp.c | HAL MSP 初始化：`HAL_MspInit` 使能 SYSCFG 与 PWR 时钟 | 已实现 | `HAL_MspInit` 被 `Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c` 的 `HAL_Init` 调用 |
| Core/Src/system_stm32f4xx.c | CMSIS 系统初始化：`SystemInit`（FPU 访问/向量表）、`SystemCoreClock` 全局变量、`SystemCoreClockUpdate` 按 RCC 寄存器更新时钟频率 | 已实现 | `SystemInit` 被 `startup_stm32f407xx.s` 调用；`SystemCoreClock` 变量被 HAL 使用；`SystemCoreClockUpdate` 未被调用 |
| Core/Src/syscalls.c | Newlib/Picolibc 最小系统调用桩（_read/_write/_exit/_open 等），_read/_write 重定向到 `__io_putchar`/`__io_getchar`；Picolibc 下映射标准 I/O 流 | 已实现 | 由 C 运行库调用；用户源码未显式调用 |
| Core/Src/sysmem.c | 堆内存管理 `_sbrk`：在 `_end` 与 `_estack - _Min_Stack_Size` 之间分配堆空间 | 已实现 | 由 C 运行库 `malloc` 调用；用户源码未显式调用 |
| startup_stm32f407xx.s | 启动文件：初始栈指针、向量表、`Reset_Handler`（`bl SystemInit` 后 `bl main`）、默认异常/中断弱定义 | 已实现 | 复位后由硬件执行；`Reset_Handler` 调用 `SystemInit` 与 `main` |
| STM32F407XX_FLASH.ld | 链接脚本：RAM 128K、CCMRAM 64K、FLASH 512K 内存区域与入口 `Reset_Handler` | 已实现 | 被 `cmake/gcc-arm-none-eabi.cmake` 的 `-T` 参数引用 |
| Drivers/CMSIS | ST 官方 CMSIS 外设访问层头文件（`stm32f4xx.h` 及 Cortex-M 核心头） | 已实现（官方库） | 被 `Core/Src/system_stm32f4xx.c`（`stm32f4xx.h`）及 HAL 头文件包含 |
| Drivers/STM32F4xx_HAL_Driver | ST 官方 HAL 驱动（`stm32f4xx_hal.h` 及各模块 .c/.h） | 已实现（官方库） | 经 `Core/Inc/main.h`（`stm32f4xx_hal.h`）被 main.c/中断/MSP 使用；由 `cmake/stm32cubemx/CMakeLists.txt` 登记为 STM32_Drivers 库 |
| tests | PC GCC 独立测试：`tests/CMakeLists.txt` 构建 7 个可执行文件并注册 CTest（crc8、ring_buffer、ring_buffer_invalid_size、mecanum、pid、pid_invalid_dt、scheduler） | 已实现 | 由 `tests/CMakeLists.txt` 构建；其中 test_*.c 调用 Common、Modules、Application 的被测函数 |

补充：`cmake/project_sources.cmake` 的 `USER_SOURCES` 当前登记了 `Modules/chassis/mecanum.c`、`Modules/pid/pid.c`、`Application/scheduler/scheduler.c`、`Common/Src/ring_buffer.c`、`Common/Src/crc8.c`，这些 .c 已登记进 MCU 正式构建，但生产代码中没有代码调用它们。

## 二、依赖关系图

目标分层方向（文档定义）为：`Application → Modules → BSP → Common`。下面只画当前代码中真实存在的调用/包含关系。

```text
Application/
  scheduler.c ──> scheduler.h          (同层包含)
  无对 Modules / BSP / Common 的任何调用

Modules/
  mecanum.h ──> motor_if.h             (同层，chassis 内部)
  mecanum.c ──> mecanum.h
  pid.c     ──> pid.h
  无对 BSP / Common 的任何调用

BSP/
  board / input / servo / stepper / uart 均为空目录
  无任何文件，无任何依赖

Common/
  crc8.c        ──> crc8.h
  ring_buffer.c ──> ring_buffer.h
  无对其它层的依赖
```

四层之间的真实调用边汇总：

```text
Application → Modules   无
Application → BSP       无
Application → Common    无
Modules     → BSP       无
Modules     → Common    无
BSP         → Common    无
```

当前唯一真实存在的跨边界依赖来自测试层：

```text
tests/modules/test_scheduler.c        ──> Application/scheduler/scheduler.h
tests/modules/test_mecanum.c          ──> Modules/chassis/mecanum.h
tests/modules/test_pid.c              ──> Modules/pid/pid.h
tests/modules/test_pid_invalid_dt.c   ──> Modules/pid/pid.h
tests/common/test_crc8.c              ──> Common/Inc/crc8.h
tests/common/test_ring_buffer.c       ──> Common/Inc/ring_buffer.h
tests/common/test_ring_buffer_invalid_size.c ──> Common/Inc/ring_buffer.h
```

另：`Core/Src/main.c`（程序入口）当前未调用 Application/Modules/BSP/Common 中的任何代码，因此 Application 层的 scheduler 目前没有生产调用者。
