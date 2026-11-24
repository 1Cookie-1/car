# STM32F407 电控工程架构说明

## 1. 文档目的

本文档描述工程的整体软件架构、分层职责、模块依赖关系、关键数据流、
初始化与运行顺序、构建接入方式和测试边界。

本文档只说明「结构」和「规则」，不包含具体驱动、算法或状态机实现代码。
具体实现将在硬件信息确认后，按 `Docs/development/setup-notes.md` 的分阶段
顺序逐步补充。

配套文件：

- `agent_rules.md`：工程硬规则，发生冲突时以它为准。
- `Docs/development/setup-notes.md`：硬件确认清单、搭建流程与分阶段开发。

## 2. 分层总览

工程按依赖方向自上而下分为以下层次：

| 层次 | 目录 | 职责 |
| --- | --- | --- |
| 应用层 | `Application/` | 组织系统运行：应用服务、周期调度、比赛任务与状态机 |
| 功能模块层 | `Modules/` | 与硬件解耦的算法和功能模块（PID、麦轮运动学、底盘、通信协议） |
| 板级支持层 | `BSP/` | 具体硬件驱动与板级硬件接口装配 |
| 配置层 | `Config/` | 项目配置与硬件资源映射 |
| 通用组件层 | `Common/` | 与硬件无关的可复用基础组件（环形缓冲区、CRC8） |
| 生成代码层 | `Core/`、`Drivers/` | CubeMX 生成的 MCU 初始化代码与 STM32 官方驱动 |
| 测试层 | `tests/` | PC 端独立测试，不进入 MCU 正式构建 |
| 文档层 | `Docs/` | 硬件、软件、开发流程文档 |

`Core/` 与 `Drivers/` 由 CubeMX / ST 官方提供，原则上尽量不手动修改。
其余层次属于用户代码，按下方依赖规则逐步实现。

## 3. 各层职责

### 3.1 Core 与 Drivers

- `Core/`：CubeMX 生成的 MCU 初始化代码，包括 `main.c`、中断入口、
  `system_stm32f4xx.c`、MSP 初始化和系统时钟配置。
- `Drivers/`：STM32 官方 CMSIS 与 HAL 驱动。

这两层只做基础硬件初始化，不承载业务逻辑。修改必须遵守
`agent_rules.md` 第 10、11 节对 CMake 和 `main.c` 的约束。

### 3.2 BSP（板级支持层）

- `board/`：板级初始化 `Board_Init()`，并把具体驱动绑定到抽象接口。
- `stepper/`：步进电机硬件驱动。
- `servo/`：舵机硬件驱动。
- `uart/`：UART 字节收发与接收缓存。
- `input/`：数字输入、按键、限位开关、拨码开关等输入设备驱动。

BSP 负责具体硬件操作，可以使用 HAL 或寄存器，但不负责比赛业务决策。

### 3.3 Modules（功能模块层）

- `pid/`：通用 PID 算法。
- `chassis/`：底盘控制、速度组织、麦轮运动学解算和电机抽象接口。
- `comm/`：通信协议拆帧、组帧、CRC 校验和命令解析。

Modules 不得直接依赖 HAL 或具体硬件；电机通过 `motor_if.h` 抽象接口访问，
通信模块通过字节流和发送回调工作。

### 3.4 Application（应用层）

- `app.c`：提供 `App_Init()` 与 `App_Loop()`，组织系统运行。
- `service/`：连接驱动与功能模块的应用服务，例如 `comm_service`。
- `scheduler/`：周期任务调度。
- `task/`：比赛任务与状态机。

Application 组织和调用 Modules、BSP、Common，是唯一承载业务逻辑的层次。

### 3.5 Common（通用组件层）

- `ring_buffer`：通用环形缓冲区。
- `crc8`：通用 CRC8 算法。

Common 不依赖 HAL、BSP 或 Application，保持独立可复用。

### 3.6 Config（配置层）

- `project_config.h`：版本与功能开关。
- `board_config.h`：经过硬件确认的引脚、端口和硬件资源映射。
- `task_config.h`：速度、角度、超时、场地参数（必须注明单位）。

Config 只存放配置，不存放业务逻辑；未经确认的硬件映射不得编造。

### 3.7 tests（测试层）

- `tests/` 使用独立 CMake 工程与 PC GCC 构建，不引入 HAL。
- 当前 `tests/CMakeLists.txt` 中真实存在的测试对象为：`test_ring_buffer`、
  `test_ring_buffer_invalid_size`、`test_crc8`、`test_mecanum`、
  `test_pid`、`test_pid_invalid_dt`、`test_scheduler`。
- tests 不进入 MCU 正式构建。

## 4. 依赖规则

允许的依赖方向如下：

- Application 可组织、调用 Modules、BSP、Common。
- Modules 不依赖 HAL 或具体硬件。
- BSP 负责硬件操作，可依赖 Modules 定义的抽象接口声明，
  但不依赖 Modules 的具体业务实现。
- Common 不依赖 STM32 HAL、BSP 或 Application。
- tests 不进入 STM32 正式构建。

禁止方向：

- 业务逻辑直接调用具体硬件驱动实现。
- comm 直接包含 `bsp_uart.h` 或调用 HAL UART/DMA 函数。
- 底盘算法直接依赖具体步进电机驱动。

## 5. 关键数据流

### 5.1 通信接收

```text
UART 中断/DMA
    ↓
BSP/uart（内部使用 Common/ring_buffer 缓存字节）
    ↓
Application/service/comm_service
    ↓
Modules/comm
    ↓
Application 处理解析结果
```

### 5.2 通信发送

```text
Application
    ↓
Modules/comm（协议组帧）
    ↓
Application 提供的发送回调
    ↓
Application/service/comm_service
    ↓
BSP/uart
    ↓
UART 硬件
```

comm 只处理协议与帧，不负责硬件收发；发送回调由 `comm_service` 实现并注册。

### 5.3 电机控制链路

```text
Modules/chassis（运动学与底盘控制）
    ↓ 通过 motor_if.h
BSP/board（Board_Init 完成接口绑定）
    ↓
BSP/stepper 或 BSP/servo（具体硬件驱动）
```

### 5.4 输入链路

```text
外部输入
    ↓
BSP/input（读取与必要信号处理）
    ↓
Application（业务决策）
```

急停例外：急停必须在中断或最高优先级路径直接切断电机输出，不经过
scheduler 和状态机；状态机只负责事后记录与复位。

## 6. 初始化与运行顺序

推荐顺序：

1. `HAL_Init()`
2. `SystemClock_Config()`
3. CubeMX 生成的 GPIO、TIM、UART 等外设初始化
4. `Board_Init()`（初始化 BSP 驱动并绑定 motor_if 等抽象接口）
5. `App_Init()`（初始化 scheduler、comm_service、chassis、task 等应用模块）
6. 进入 `main.c` 现有 `while(1)`，持续调用 `App_Loop()`

`App_Loop()` 是单次调度入口，不自带永久循环，不得长时间阻塞。
应用调度统一使用一个时基，优先复用 `HAL_GetTick()`。

## 7. 构建接入方式

根 `CMakeLists.txt` 保留 CubeMX 原有结构。在确认 `add_executable()`
实际目标名称后，仅追加用户代码接入区段：

```text
include(cmake/project_sources.cmake)
target_sources(实际目标名称 PRIVATE ${USER_SOURCES})
target_include_directories(实际目标名称 PRIVATE ${USER_INCLUDES})
```

`cmake/project_sources.cmake` 第一版使用显式源文件清单，只登记进入 MCU
正式构建的用户 `.c` 文件与头文件目录；tests 与 CubeMX 已登记源文件不得重复加入。

## 8. 当前状态与演进记录

### 8.1 当前状态

| 层次 | 状态 | 具体文件 |
| --- | --- | --- |
| Common | 已实现 | `Common/Inc/crc8.h`、`Common/Src/crc8.c`、`Common/Inc/ring_buffer.h`、`Common/Src/ring_buffer.c` |
| Modules | 部分实现 | `Modules/chassis/motor_if.h`、`Modules/chassis/mecanum.h`、`Modules/chassis/mecanum.c`、`Modules/pid/pid.h`、`Modules/pid/pid.c`；`Modules/comm/` 未开始 |
| Application | 部分实现 | `Application/scheduler/scheduler.h`、`Application/scheduler/scheduler.c`；`app.c`、`service/`、`task/` 未开始 |
| BSP | 未开始 | `BSP/board`、`BSP/stepper`、`BSP/servo`、`BSP/uart`、`BSP/input` 均为空目录，无入库文件 |
| Config | 未开始 | `Config/` 为空目录，无入库文件 |
| Core | 已实现（CubeMX 生成） | `Core/Src/main.c`、`Core/Src/stm32f4xx_it.c`、`Core/Src/stm32f4xx_hal_msp.c`、`Core/Src/system_stm32f4xx.c`、`Core/Src/syscalls.c`、`Core/Src/sysmem.c` 及 `Core/Inc/` 对应头文件 |

### 8.2 演进记录

| 日期 | 内容 | 依据(提交号) |
| --- | --- | --- |
| 2026-11-23 | 168MHz 时钟/TIM6/TIM1/USART2 | 5936fbe |
| 2026-11-22 | 麦轮解算修正 | 097d449 |
| 2026-11-22 | 增加文件地图 | d542d42 |
| 2026-11-22 | ring_buffer/PID 边界修正 | c7c8824 |
| 2026-11-22 | 阶段1-7 文档/CMake/算法 | d6e82cd |
| 2026-11-21 | 移除 led 模块 | da9d367 |
| 2026-11-21 | 新增 led 模块 | 8c613c7 |
| 2026-11-20 | CubeMX 基线 | b386639 |

## 9. 后续阶段

按 `Docs/development/setup-notes.md` 第十二节执行：检查原工程 → 建立目录框架 →
接入 CMake → 实现 Common → 实现 BSP → 实现 Modules → 实现 Application → 系统集成。
