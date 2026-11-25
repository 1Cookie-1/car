# 软件文件地图（file-map）

本表只记录当前仓库中真实存在的文件与目录内容；每一行的职责与调用关系均来自实际读取的文件内容和 `grep` 检索结果。

## 一、文件职责表

| 路径 | 实际职责 | 状态 | 被谁调用 |
| --- | --- | --- | --- |
| Application/scheduler/scheduler.h | 周期任务调度器接口声明：`SchedTaskFn` 函数指针类型，`Sched_Init`/`Sched_Register`/`Sched_Run`/`Sched_TaskCount` 原型；注释约定任务执行期间注册表不可修改（运行中 `Sched_Register` 返回 false、`Sched_Init` 无操作）、嵌套 `Sched_Run` 被忽略、接口非 ISR 安全、固定延时策略、时间回绕窗口小于 2^31ms，以及首次运行立即执行 | 只有声明 | 被 `Application/scheduler/scheduler.c`、`tests/modules/test_scheduler.c`、`tests/modules/test_scheduler_extra.c` 包含 |
| Application/scheduler/scheduler.c | 周期任务调度器实现：静态数组 `s_tasks` 最多容纳 8 个任务；`Sched_Register` 做空指针/周期为 0/重复注册检查，并拒绝运行期间注册；`Sched_Run` 用 `(int32_t)(now_ms - due) >= 0` 判断到期，兼容 uint32 时间回绕，按固定延时（now+period）重新锚定，并用 `scheduled` 标志保证 tick 已超过 2^31 时新任务仍能首次触发；嵌套 `Sched_Run` 直接返回；含 `s_run_depth` 运行深度保护 | 已实现 | `Sched_Init`/`Sched_Register`/`Sched_Run`/`Sched_TaskCount` 仅被 `tests/modules/test_scheduler.c`、`tests/modules/test_scheduler_extra.c` 调用；生产代码未被引用 |
| Modules/chassis/motor_if.h | 电机抽象接口声明：`MotorId` 枚举（FL/FR/RL/RR/COUNT，COUNT 非合法 ID），`MotorIf` 结构（set_speed/stop/stop_all/get_position 四个函数指针），全局实例 `g_motor` 声明；注释明确速度单位 steps/s、stop/stop_all/get_position 语义与调用前提，并说明 `g_motor` 当前只有声明没有定义 | 只有声明 | 被 `Modules/chassis/mecanum.h` 包含；`g_motor` 未被引用 |
| Modules/chassis/mecanum.h | 麦轮运动学接口声明：`ChassisVelocity` 结构（vx/vy/wz 单位 mm/s、mm/s、mrad/s），`Mecanum_Solve` 原型；注释明确半长/半宽几何参数须为正有限数且由调用方保证、输出为轮面线速度 mm/s 不涉及轮径、车轮顺序与旋转方向约定、NaN/Inf 传播，以及 NULL 输入时不写任何数据、输出数组保持原值 | 只有声明 | 被 `Modules/chassis/mecanum.c`、`tests/modules/test_mecanum.c`、`tests/modules/test_mecanum_extra.c` 包含 |
| Modules/chassis/mecanum.c | 四轮麦轮逆运动学解算实现 `Mecanum_Solve`：由 vx/vy/wz 和半长、半宽计算四个轮子的线速度；`v` 或 `wheel_mm_s` 为 NULL 时直接返回 | 已实现 | `Mecanum_Solve` 仅被 `tests/modules/test_mecanum.c`、`tests/modules/test_mecanum_extra.c` 调用；生产代码未被引用 |
| Modules/pid/pid.h | 位置式 PID 接口声明：`Pid` 结构（kp/ki/kd、积分限幅、输出限幅、微分先行状态），`Pid_Init`/`Pid_Update`/`Pid_Reset` 原型；注释明确 NULL、NaN/Inf dt 与输入、非法增益/限幅的处理行为，以及 Update 事务式提交（误差/积分/微分/输出任一步溢出即返回 0 且不修改状态） | 只有声明 | 被 `Modules/pid/pid.c`、`tests/modules/test_pid.c`、`tests/modules/test_pid_invalid_dt.c`、`tests/modules/test_pid_invalid_inputs.c`、`tests/modules/test_pid_overflow.c` 包含 |
| Modules/pid/pid.c | 位置式 PID 实现：积分限幅（anti-windup）、微分先行（derivative on measurement）、输出限幅；拒绝 NULL pid、NaN/Inf dt 与输入；Update 事务式提交：误差、积分增量、积分候选值、微分项、最终输出任一步有限输入溢出为 Inf/NaN 时返回 0 且不修改任何内部状态；非法增益/限幅在 Init 时按 0 处理；含静态辅助函数 `pid_is_finite`/`clampf`/`sanitize_gain`/`sanitize_limit` | 已实现 | `Pid_Init`/`Pid_Update` 被 `tests/modules/test_pid.c`、`tests/modules/test_pid_invalid_dt.c`、`tests/modules/test_pid_invalid_inputs.c`、`tests/modules/test_pid_overflow.c` 调用；`Pid_Reset` 被 `tests/modules/test_pid_invalid_inputs.c` 调用；生产代码未被引用 |
| BSP/board | 空 | 空目录 | 未被引用 |
| BSP/input | 空 | 空目录 | 未被引用 |
| BSP/servo | 空 | 空目录 | 未被引用 |
| BSP/stepper | 空 | 空目录 | 未被引用 |
| BSP/uart | 空 | 空目录 | 未被引用 |
| Common/Inc/crc8.h | CRC-8 接口与参数声明：多项式 0x07、初值 0x00、XOROUT 0x00；`Crc8_Update`/`Crc8_Compute` 原型 | 只有声明 | 被 `Common/Src/crc8.c`、`tests/common/test_crc8.c` 包含 |
| Common/Src/crc8.c | CRC-8 实现：`Crc8_Update` 逐字节位运算；`Crc8_Compute` 遍历数据、以初值 0x00 计算并异或 XOROUT | 已实现 | `Crc8_Compute` 被 `tests/common/test_crc8.c` 调用；`Crc8_Update` 仅被本文件 `Crc8_Compute` 内部调用；生产代码未被引用 |
| Common/Inc/ring_buffer.h | 环形缓冲区接口声明：`RingBuffer` 结构（buf/size/volatile head/tail），`RingBuffer_Init`/`Put`/`Get`/`Count`/`IsEmpty`/`IsFull` 原型；注释约定单生产者/单消费者、保留一个空位、size<2 视为无效，并明确 volatile 不提供线程/中断安全保证、NULL 指针与 NULL 存储区按无效缓冲区处理、Get 的 NULL 输出指针不消费数据 | 只有声明 | 被 `Common/Src/ring_buffer.c`、`tests/common/test_ring_buffer.c`、`tests/common/test_ring_buffer_invalid_size.c`、`tests/common/test_ring_buffer_null.c` 包含 |
| Common/Src/ring_buffer.c | 环形缓冲区实现：初始化、读写、计数、空/满判断；对 NULL rb/存储区/输出指针加保护；size<2 或存储区为 NULL 时初始化为无效缓冲区且 Put/Get 返回 false；无锁，单生产者/单消费者 | 已实现 | `RingBuffer_*` 仅被 `tests/common/test_ring_buffer.c`、`tests/common/test_ring_buffer_invalid_size.c`、`tests/common/test_ring_buffer_null.c` 调用；生产代码未被引用 |
| Config | 空 | 空目录 | 未被引用 |
| Core/Src/main.c | CubeMX 生成入口：`HAL_Init`、`SystemClock_Config`（HSE、PLL 开启、FLASH_LATENCY_5）、`MX_GPIO_Init`（使能 GPIOH/GPIOA/GPIOE 时钟）、空的 `while(1)`；USER CODE 区为空，未调用 `Board_Init`/`App_Init`/`App_Loop` | 已实现 | 被 `startup_stm32f407xx.s` 的 `Reset_Handler` 调用（`bl main`）；内部调用 `HAL_Init`/`SystemClock_Config`/`MX_GPIO_Init`/`MX_TIM6_Init`/`MX_TIM1_Init`/`MX_USART2_UART_Init`/`Error_Handler` |
| Core/Inc/main.h | main.c 头文件：包含 `stm32f4xx_hal.h`，声明 `Error_Handler`、`HAL_TIM_MspPostInit` | 只有声明 | 被 `Core/Src/main.c`、`Core/Src/stm32f4xx_it.c`、`Core/Src/stm32f4xx_hal_msp.c` 包含 |
| Core/Inc/stm32f4xx_it.h | 中断处理函数原型声明：NMI/HardFault/MemManage/BusFault/UsageFault/SVC/DebugMon/PendSV/SysTick，以及外设中断 USART2_IRQHandler/TIM6_DAC_IRQHandler（共 11 个） | 只有声明 | 被 `Core/Src/stm32f4xx_it.c` 包含 |
| Core/Inc/stm32f4xx_hal_conf.h | HAL 配置：使能 HAL_TIM/UART/GPIO/EXTI/DMA/RCC/FLASH/PWR/CORTEX 模块；定义 HSE_VALUE/HSI_VALUE/TICK_INT_PRIORITY 等宏，并按使能模块包含对应 HAL 头 | 只有声明（配置宏） | 被 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` 包含 |
| Core/Src/stm32f4xx_it.c | 异常/中断服务例程实现：NMI/HardFault/MemManage/BusFault/UsageFault/SVC/DebugMon/PendSV/SysTick；`SysTick_Handler` 调用 `HAL_IncTick`；`USART2_IRQHandler` 调用 `HAL_UART_IRQHandler(&huart2)`；`TIM6_DAC_IRQHandler` 调用 `HAL_TIM_IRQHandler(&htim6)` | 已实现 | 各处理函数经 `startup_stm32f407xx.s` 向量表由硬件触发；`SysTick_Handler` 调用 `HAL_IncTick`，`USART2_IRQHandler`/`TIM6_DAC_IRQHandler` 分别调用对应 HAL IRQ 分发函数 |
| Core/Src/stm32f4xx_hal_msp.c | HAL MSP 初始化/反初始化：`HAL_MspInit`（SYSCFG/PWR 时钟）；`HAL_TIM_Base_MspInit`（TIM1/TIM6 时钟，TIM6 使能 NVIC）；`HAL_TIM_MspPostInit`（TIM1_CH1 的 PE9 复用配置）；`HAL_TIM_Base_MspDeInit`；`HAL_UART_MspInit`（USART2 时钟、PA2/PA3 复用、USART2 NVIC）；`HAL_UART_MspDeInit` | 已实现 | 各 MSP 函数由 HAL 在对应外设 Init/DeInit 时回调；`HAL_MspInit` 被 `HAL_Init` 调用 |
| Core/Src/system_stm32f4xx.c | CMSIS 系统初始化：`SystemInit`（FPU 访问/向量表）、`SystemCoreClock` 全局变量、`SystemCoreClockUpdate` 按 RCC 寄存器更新时钟频率 | 已实现 | `SystemInit` 被 `startup_stm32f407xx.s` 调用；`SystemCoreClock` 变量被 HAL 使用；`SystemCoreClockUpdate` 未被调用 |
| Core/Src/syscalls.c | Newlib/Picolibc 最小系统调用桩（_read/_write/_exit/_open 等），_read/_write 重定向到 `__io_putchar`/`__io_getchar`；Picolibc 下映射标准 I/O 流 | 已实现 | 由 C 运行库调用；用户源码未显式调用 |
| Core/Src/sysmem.c | 堆内存管理 `_sbrk`：在 `_end` 与 `_estack - _Min_Stack_Size` 之间分配堆空间 | 已实现 | 由 C 运行库 `malloc` 调用；用户源码未显式调用 |
| startup_stm32f407xx.s | 启动文件：初始栈指针、向量表、`Reset_Handler`（`bl SystemInit` 后 `bl main`）、默认异常/中断弱定义 | 已实现 | 复位后由硬件执行；`Reset_Handler` 调用 `SystemInit` 与 `main` |
| STM32F407XX_FLASH.ld | 链接脚本：RAM 128K、CCMRAM 64K、FLASH 512K 内存区域与入口 `Reset_Handler` | 已实现 | 被 `cmake/gcc-arm-none-eabi.cmake` 的 `-T` 参数引用 |
| Drivers/CMSIS | ST 官方 CMSIS 外设访问层头文件（`stm32f4xx.h` 及 Cortex-M 核心头） | 已实现（官方库） | 被 `Core/Src/system_stm32f4xx.c`（`stm32f4xx.h`）及 HAL 头文件包含 |
| Drivers/STM32F4xx_HAL_Driver | ST 官方 HAL 驱动（`stm32f4xx_hal.h` 及各模块 .c/.h） | 已实现（官方库） | 经 `Core/Inc/main.h`（`stm32f4xx_hal.h`）被 main.c/中断/MSP 使用；由 `cmake/stm32cubemx/CMakeLists.txt` 登记为 STM32_Drivers 库 |
| tests | PC GCC 独立测试：`tests/CMakeLists.txt` 构建 12 个可执行文件并注册 CTest（crc8、ring_buffer、ring_buffer_invalid_size、ring_buffer_null、mecanum、mecanum_extra、pid、pid_invalid_dt、pid_invalid_inputs、pid_overflow、scheduler、scheduler_extra） | 已实现 | 由 `tests/CMakeLists.txt` 构建；其中 test_*.c 调用 Common、Modules、Application 的被测函数 |
| tests/common/test_ring_buffer_null.c | PC 测试：NULL rb、NULL 存储区、NULL 输出指针的返回行为与不崩溃 | 已实现 | 被 `tests/CMakeLists.txt` 构建；调用 `Common/Src/ring_buffer.c` 的 `RingBuffer_*` |
| tests/modules/test_mecanum_extra.c | PC 测试：麦轮复合运动数值、NULL 指针不崩溃、NULL v 时输出数组保持不变 | 已实现 | 被 `tests/CMakeLists.txt` 构建；调用 `Modules/chassis/mecanum.c` 的 `Mecanum_Solve` |
| tests/modules/test_pid_invalid_inputs.c | PC 测试：NaN/Inf dt 与输入、NULL pid、非法增益/限幅的边界行为 | 已实现 | 被 `tests/CMakeLists.txt` 构建；调用 `Modules/pid/pid.c` 的 `Pid_*` |
| tests/modules/test_pid_overflow.c | PC 测试：有限输入导致的误差/积分/微分/输出溢出返回 0 且所有内部状态不变，异常后正常输入仍有限 | 已实现 | 被 `tests/CMakeLists.txt` 构建；调用 `Modules/pid/pid.c` 的 `Pid_Update` |
| tests/modules/test_scheduler_extra.c | PC 测试：NULL 函数指针、执行顺序等于注册顺序、运行中修改任务表被拒、最大周期合法、tick 已超过 2^31 时注册仍能触发、嵌套 Sched_Run 被忽略 | 已实现 | 被 `tests/CMakeLists.txt` 构建；调用 `Application/scheduler/scheduler.c` 的 `Sched_*` |

补充：`cmake/project_sources.cmake` 的 `USER_SOURCES` 当前登记了 `Modules/chassis/mecanum.c`、`Modules/pid/pid.c`、`Application/scheduler/scheduler.c`、`Common/Src/ring_buffer.c`、`Common/Src/crc8.c`，这些 .c 已登记进 MCU 正式构建，但生产代码中没有代码调用它们。

## 二、依赖关系图

分层依赖方向（依据 agent_rules.md 第 1 节）：Application 可组织调用 Modules、BSP、Common；BSP 可依赖 Modules 的抽象接口声明与 Common；Modules 不依赖 BSP 或 HAL；Common 不依赖其它层。注意：并非 `Application → Modules → BSP → Common` 的单向链，Modules 并不依赖 BSP。下面只画当前代码中真实存在的调用/包含关系。

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
tests/modules/test_scheduler_extra.c  ──> Application/scheduler/scheduler.h
tests/modules/test_mecanum.c          ──> Modules/chassis/mecanum.h
tests/modules/test_mecanum_extra.c    ──> Modules/chassis/mecanum.h
tests/modules/test_pid.c              ──> Modules/pid/pid.h
tests/modules/test_pid_invalid_dt.c   ──> Modules/pid/pid.h
tests/modules/test_pid_invalid_inputs.c ──> Modules/pid/pid.h
tests/modules/test_pid_overflow.c     ──> Modules/pid/pid.h
tests/common/test_crc8.c              ──> Common/Inc/crc8.h
tests/common/test_ring_buffer.c       ──> Common/Inc/ring_buffer.h
tests/common/test_ring_buffer_invalid_size.c ──> Common/Inc/ring_buffer.h
tests/common/test_ring_buffer_null.c  ──> Common/Inc/ring_buffer.h
```

另：`Core/Src/main.c`（程序入口）当前未调用 Application/Modules/BSP/Common 中的任何代码，因此 Application 层的 scheduler 目前没有生产调用者。
