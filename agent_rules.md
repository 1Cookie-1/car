STM32F407 电控工程架构硬规则
本文件是工程开发、代码生成、代码修改和 Agent 协作的强制规范。
所有 Agent 在修改工程前，必须先阅读并遵守本文件。
如实际工程情况与本规范存在冲突，必须先报告冲突并等待确认，
不得擅自修改芯片配置、硬件配置、构建系统或整体架构。
配套阅读：Docs/development/setup-notes.md
（硬件确认清单、搭建流程与分阶段开发，不覆盖本文件规则）。
规则冲突时以本文件为准；搭建注意事项仅提供流程说明与检查清单。

--------------------
0. 行为总纲
- 以瞎猜接口为耻，以认真查询为荣。
- 以模糊执行为耻，以寻求确认为荣。
- 以臆想业务为耻，以人类确认为荣。
- 以创造接口为耻，以复用现有为荣。
- 以跳过验证为耻，以主动测试为荣。
- 以破坏架构为耻，以遵循规范为荣。
- 以假装理解为耻，以诚实无知为荣。
- 以盲目修改为耻，以慎重重构为荣。
1. 总体架构与职责
工程采用以下分层结构：
Core：CubeMX 生成的 MCU 初始化代码。
Drivers：STM32 官方 CMSIS 和 HAL 驱动。
BSP：具体硬件驱动与板级硬件接口装配。
Modules：与具体硬件解耦的算法和功能模块。
Application：应用服务、调度和比赛业务逻辑。
Common：通用基础组件。
Config：项目配置和硬件资源映射。
tests：PC 端独立测试。
基本依赖原则：
Application 可以组织和调用 Modules、BSP、Common。
Modules 不得直接依赖 HAL 或具体硬件。
BSP 负责具体硬件操作。
BSP 可以依赖 Modules 定义的抽象接口声明，
但不得依赖 Modules 的具体业务实现。
Common 不得依赖 STM32 HAL 或具体硬件。
tests 不得进入 STM32 正式构建。
目录结构不代表功能已经实现。
没有实际需求和硬件依据时，不得为了填充目录而创建空模块。

--------------------

2. 通信模块必须与硬件解耦
Modules/comm 只负责通信协议解析与数据帧处理。
comm.c 禁止 include bsp_uart.h、usart.h、stm32f4xx_hal.h。
comm.c 禁止调用 HALUART、HALDMA 等硬件函数。
comm 模块不得直接读取 UART、CAN 等外设。
comm 模块通过输入字节流推进解析状态。
完整数据帧解析成功后，通过回调或结果队列向上层提供解析结果。
回调函数由 Application 提供，不得在 comm 内部直接调用 BSP。
CRC8 算法统一使用 Common/crc8.c，不得重复实现。
comm 模块只依赖协议定义和通用数据类型，不依赖具体通信硬件。
2.1 接收方向数据流
UART 中断/DMA
    ↓
BSP/uart（内部使用 Common/ring_buffer 缓存字节）
    ↓
Application/service/comm_service
    ↓
Modules/comm
    ↓
Application 处理解析结果
具体职责：
BSP/uart：接收字节、使用 ring_buffer 缓存字节、提供读取接口。
Common/ring_buffer：提供通用环形缓冲区实现。
ring_buffer 由 BSP/uart 使用，但不属于 UART 专属组件。
其他模块有需要时也可以复用 ring_buffer。
ring_buffer 不得依赖 BSP、HAL 或具体硬件。
comm_service：从 BSP 读取字节，交给 comm 解析。
Modules/comm：解析数据帧、校验 CRC、提取命令。
Application：根据解析结果执行相应业务逻辑。
2.2 发送方向数据流
Application
    ↓
Modules/comm
    ↓
组装完整协议数据帧
    ↓
Application 提供的发送回调
    ↓
Application/service/comm_service
    ↓
BSP/uart
    ↓
UART 硬件
具体规则：
comm 负责协议组帧，不负责硬件发送。
发送回调由 Application/service/comm_service 实现并注册。
建议发送回调传递数据缓冲区和数据长度。
comm 不得直接调用 BSP/UART。
如果 BSP 使用 DMA 异步发送，必须保证发送缓冲区在发送完成前有效。
必须明确发送缓存的所有权、生命周期和发送完成处理方式。
具体接口应根据实际通信协议和硬件确定。
2.3 通信服务职责
Application/service/comm_service 负责：
从 BSP/uart 获取接收数据。
将字节流交给 Modules/comm。
接收 comm 解析出的完整数据帧或命令。
将解析结果交给 Application 业务层。
提供 comm 所需的发送回调。
通过 BSP/uart 完成实际数据发送。
comm_service 不负责重新实现协议解析和 CRC 算法。

--------------------

3. 输入类外设归属规则
普通按键、急停按钮、限位开关、拨码开关等输入设备归属 BSP/input。
底层数字输入读取接口归属 digital_input。
button、limit_switch、dip_switch 等具体驱动按实际需求创建。
按键消抖等与具体输入设备相关的处理可以放在对应输入驱动中。
输入驱动只负责硬件读取及必要的信号处理，不负责比赛业务决策。
限位触发后如何控制电机，由上层业务逻辑决定。
急停的软件处理不能替代必要的硬件安全措施。
急停必须在中断或最高优先级路径里直接切断电机输出，
不依赖 scheduler 和状态机；状态机只负责事后记录与复位。
没有确认实际硬件和接线前，禁止编造 GPIO 引脚定义。
没有实际硬件时，可以暂不创建具体输入驱动文件。

--------------------

4. 模块解耦规则
Modules 不得直接依赖 HAL。
Modules 不得直接操作 GPIO、TIM、UART 等具体外设。
电机控制通过 motor_if.h 抽象接口访问底层实现。
BSP 负责具体硬件操作。
Application 负责组织 BSP 与 Modules。
Common 不得依赖 STM32 HAL 或具体硬件。
BSP 可以依赖 Modules 提供的抽象接口声明，
但不得依赖 Modules 的具体业务实现。
业务逻辑不得直接调用具体硬件驱动实现。
Application/service 和 BSP/board 可以调用 BSP 接口，
负责硬件访问与模块之间的衔接。
4.1 chassis 与 mecanum 的职责
Modules/chassis/chassis.c：
负责底盘控制逻辑。
负责组织底盘速度控制。
通过 motor_if.h 操作电机抽象接口。
可以使用 PID 等通用控制模块。
不得直接操作 GPIO、TIM、UART 等具体硬件。
Modules/chassis/mecanum.c：
只负责麦轮运动学解算。
输入底盘运动需求。
输出各轮目标速度或相应运动学结果。
不得直接操作电机。
不得依赖 HAL。
Modules/chassis/motor_if.h：
定义电机抽象接口。
不包含具体 GPIO、TIM 或 HAL 操作。
不依赖具体步进电机驱动实现。

--------------------

5. Common 通用组件规则
Common 只存放与具体硬件无关的通用基础组件。
Common 不得依赖 HAL、BSP 或 Application。
Common 中的组件必须尽可能独立、可复用。
ring_buffer 是通用环形缓冲区组件。
crc8 是通用 CRC8 算法组件。
CRC8 的具体多项式、初始值、输入输出异或值和计算方向，
必须根据实际通信协议确定。
不得在 comm 中重复实现 Common 已提供的 CRC8 算法。
不得在 Common 中加入具体 UART、GPIO 或电机控制逻辑。

--------------------

6. 文件创建与代码管理规则
目录结构不代表功能已经实现。
没有实际硬件依据时，不得生成具体硬件驱动实现。
不得为了填满目录而创建无实际用途的空模块。
新增 .c 文件必须确认已加入 MCU 构建目标。
第一版 project_sources.cmake 使用显式源文件清单。
新增用户 .c 文件时，必须检查并登记到对应源文件清单。
用户头文件所在目录必须加入对应 include 目录。
tests/ 中的源文件不得进入 MCU 正式构建。
tests/ 使用独立 PC GCC 构建，不得引入 HAL。
不得把 PC 测试文件加入 STM32 正式构建。
修改构建配置后必须检查实际编译目标和编译结果。

--------------------

7. Config 配置规则
project_config.h
存放项目版本、功能开关等全局项目配置。
不得放置业务逻辑函数实现。
board_config.h
存放经过硬件确认的引脚、端口和硬件资源映射。
不得编造 GPIO 引脚。
不得把 HAL 句柄和 HAL 类型声明随意暴露给所有模块。
外设句柄应由对应 BSP 或硬件适配接口管理。
task_config.h
存放比赛任务所需的速度、角度、超时和场地参数。
参数必须明确单位。
未经实际测试的参数必须标记为待验证。
不得把业务执行逻辑放在配置文件中。

--------------------

8. 抽象接口接线规则
8.1 电机接口
motor_if.h 定义电机抽象接口。
电机接口的具体实现由 BSP 提供。
BSP/board/board.c 负责将 Stepper_* 等实际驱动函数
绑定到 motor_if 抽象接口。
Board_Init() 必须在相关外设初始化完成后执行。
Board_Init() 内部先初始化所需 BSP 硬件驱动，
再完成 motor_if 等抽象接口绑定。
在接口绑定完成、底层驱动初始化完成之前，
禁止上层调用对应硬件接口。
任何接口被使用前，必须保证其函数指针有效，
且对应硬件已经完成初始化。
接口绑定后，Modules/chassis 通过 motor_if 调用电机功能。
底盘业务逻辑不得直接调用 stepper.c 中的具体实现。
BSP 只能依赖 motor_if 的接口声明，不能依赖底盘业务实现。
8.2 通信发送接口
comm 的发送回调由 Application/service/comm_service 提供。
comm 组帧后，通过回调将完整数据帧交给 Application 服务。
comm_service 负责调用 BSP/uart 的发送接口。
comm 不得直接调用 bsp_uart.c 中的具体实现。
异步发送必须处理缓冲区生命周期和发送完成状态。
8.3 输入接口
BSP/input 提供数字输入及具体输入设备驱动。
Application 或相应应用服务调用输入驱动接口。
输入驱动不得直接决定比赛任务状态。
输入事件应交给 Application 业务层处理。
急停按第 3 节规定走最高优先级直接停机路径，不经过常规事件分发。

--------------------

9. 初始化顺序与主循环规则
9.1 初始化顺序
推荐执行顺序：
HAL_Init()
SystemClock_Config()
CubeMX 生成的 GPIO、TIM、UART 等外设初始化函数。
Board_Init()
App_Init()
进入 main.c 现有 while(1)。
持续调用 App_Loop()。
具体初始化顺序必须根据真实工程的初始化依赖确认。
9.2 Board_Init()
Board_Init() 由 BSP/board 实现。
Board_Init() 内部先初始化所需 BSP 硬件驱动，
再完成 motor_if 等抽象接口绑定。
任何接口被使用前，必须保证其函数指针有效，
且底层驱动已经完成初始化。
如果某项硬件初始化失败，必须明确处理失败状态，
不得假定初始化一定成功。
9.3 App_Init()
App_Init() 负责初始化应用层模块和服务。
App_Init() 必须在 Board_Init() 成功后执行。
App_Init() 中可以初始化 scheduler、comm_service、chassis、
task 等应用模块。
不得在 App_Init() 中重复初始化 CubeMX 已完成的外设。
9.4 App_Loop()
App_Loop() 是单次应用调度入口，不自带 while(1)。
由 main.c 中 CubeMX 生成的现有 while(1) 循环持续调用 App_Loop()。
App_Loop() 每次调用只执行一次应用层调度流程。
不得在 App_Loop() 内部另外创建永久循环。
到期任务执行后应返回，不得长时间阻塞调度器。
禁止在周期任务中使用 HAL_Delay() 实现长时间等待。
9.5 Scheduler 统一时基
scheduler 的统一时基只能有一个来源。
STM32 应用调度优先复用 HAL_GetTick() 作为毫秒时基。
如果使用其他时基，必须明确唯一时基的来源、
更新位置、单位和溢出处理方式。
不得为了 scheduler 重复创建毫秒计数器。
所有周期任务统一使用该时基计算到期时间。
不得混用多个独立计时来源判断任务周期。
scheduler 可以接收当前时间作为参数进行调度，
不必直接依赖 HAL。
电机 PWM、输入捕获等外设仍可使用其他定时器，
但不得将它们作为第二套独立的应用调度时基。

--------------------

10. CMakeLists.txt 接入规则
10.1 根 CMakeLists.txt
根 CMakeLists.txt 由 CubeMX 生成。
不得重写、重排或替换整个根 CMakeLists.txt。
修改前必须检查原始文件内容。
必须先确认 add_executable() 创建的实际目标名称。
不得假定目标一定叫 ${PROJECT_NAME}.elf。
只允许在确认目标创建之后，追加用户代码接入区段。
接入区段应使用明确注释标识，便于 CubeMX 重新生成后核对。
接入内容的基本形式：
include(cmake/project_sources.cmake)
target_sources(实际目标名称 PRIVATE ${USER_SOURCES})
target_include_directories(实际目标名称 PRIVATE ${USER_INCLUDES})
其中实际目标名称必须来自原始 CMakeLists.txt 的检查结果。
除必要接入区段外，工具链、宏定义、链接脚本、
CubeMX 原有源文件列表和构建选项一律不擅自修改。
如果现有构建结构与上述接入方式不兼容，
必须先报告原因并等待确认。
10.2 project_sources.cmake
第一版采用显式源文件清单。
USER_SOURCES 只包含需要加入 STM32 正式构建的用户 .c 文件。
USER_INCLUDES 只包含需要加入 STM32 正式构建的头文件目录。
不得将 tests/ 中的文件加入 USER_SOURCES。
不得把 CubeMX 已经加入目标的 Core 和 Drivers 源文件重复添加。
新增用户 .c 文件时必须更新清单。
不得擅自将显式清单改为 GLOB_RECURSE。
10.3 构建验证
修改 CMake 后必须检查配置和生成过程。
必须确认实际构建目标。
必须检查新增源文件是否进入编译。
必须检查链接结果。
未实际执行编译时必须标注「未编译验证」。

--------------------

11. main.c 修改规则
main.c 由 CubeMX 生成，尽量保持原有结构。
11.1 头文件接入
如果 main.c 尚未包含所需头文件，
只允许在：
/USER CODE BEGIN Includes */
 USER CODE END Includes */
之间添加：
#include "board.h"
#include "app.h"
不得删除、移动或覆盖该区域已有的用户代码。
不得在 USER CODE 区域之外添加头文件。
如果头文件路径或名称与实际工程不一致，
必须先检查实际文件，不得猜测。
11.2 初始化接入
只允许在：
/USER CODE BEGIN 2 */
 USER CODE END 2 */
之间添加：
Board_Init();
App_Init();
要求：
CubeMX 外设初始化必须先完成。
Board_Init() 必须先于 App_Init() 执行。
App_Init() 中会使用已绑定的接口，
顺序颠倒可能导致接口未初始化或函数指针无效。
如果 Board_Init() 有失败返回值，必须按实际接口处理失败状态。
11.3 主循环接入
只允许在：
/USER CODE BEGIN WHILE */
 USER CODE END WHILE */
之间添加：
App_Loop();
要求：
只使用 CubeMX 现有 while(1)。
不得创建第二个永久循环。
不得将 App_Loop() 放到 while(1) 外部。
不得删除、移动或覆盖 USER CODE 区域中已有的用户代码。
11.4 修改边界
USER CODE 标记之外的代码一律不动。
若必须修改标记之外的代码，先说明原因并等待用户确认。
CubeMX 重新生成代码后，必须检查用户代码是否仍然保留。

--------------------

12. Agent 修改纪律
修改现有文件前，先检查原文件内容。
不得擅自重写 CubeMX 生成的 CMakeLists.txt。
不得擅自修改芯片型号、时钟配置、引脚映射和外设配置。
不得擅自引入 RTOS、第三方库或新的构建系统。
不得擅自改变已确定的目录结构和模块职责。
不得擅自删除或覆盖用户已有代码。
发现架构冲突、接口不明确或硬件信息缺失时，
必须先报告，不得自行脑补。
12.1 修改前
必须：
检查相关文件现有内容。
确认修改涉及哪些模块。
确认依赖关系是否符合本规范。
确认是否涉及 CubeMX 生成文件。
确认是否涉及实际硬件配置。
12.2 修改后报告
每次修改后必须报告：
新增文件。
修改文件。
删除文件。
修改原因。
模块依赖变化。
CMake 源文件清单变化。
编译验证结果。
12.3 编译验证纪律
编译验证结果必须如实报告：
实际执行并通过编译：写明已编译验证及具体结果。
实际执行但编译失败：写明失败情况及错误信息。
没有实际编译能力或未执行编译：必须写「未编译验证」。
不得声称「已通过」或省略该项。
代码生成成功不等于编译成功。
CMake 配置成功不等于最终链接成功。
12.4 未经确认禁止的操作
删除或覆盖用户已有代码。
修改芯片型号。
修改时钟配置。
修改引脚映射。
修改外设配置。
重写 CubeMX 构建系统。
引入未经确认的第三方库。
引入未经确认的 RTOS。
改变整体工程架构。
12.5 测试纪律
不得修改 tests/ 中已有的断言值（期望结果）来让测试通过。
测试失败必须先报告：说明是代码错还是测试错，由人判断后再动手。
新增功能应新增测试，不得改写既有测试的语义。
