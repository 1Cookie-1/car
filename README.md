# car

基于 STM32F407VET6 的 CMake + Ninja 工程，使用 arm-none-eabi 交叉编译，不引入 RTOS。

当前 Common、Modules、Application 的基础模块已实现，对空指针、非法参数、NaN/Inf
与中间计算溢出等可确认的非法输入做了防护（并发安全与硬件相关行为仍待后续验证），
但仅被 tests 引用，main.c 尚未调用，BSP 与 Config 为空目录。

## 编译

```bash
cmake --preset Debug
cmake --build --preset Debug
```

产物为 `build/Debug/car.elf`（需 `arm-none-eabi-*` 工具链在 PATH，Release 同理）。

## 测试（PC 上运行，无需硬件，共 12 项）

```bash
cmake -S tests -B tests/build
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
```

## 目录

| 目录 | 职责 |
| --- | --- |
| Application | 应用服务与周期任务调度器 |
| Modules | 与硬件解耦的算法模块（麦轮、PID） |
| Common | 通用基础组件（CRC-8、环形缓冲区） |
| Core、Drivers | CubeMX 初始化代码与 ST 官方 CMSIS/HAL 驱动 |
| cmake、tests | 工具链/构建脚本；PC 端独立测试 |
| Docs | 架构、文件地图与开发文档 |

## 先读什么

1. `agent_rules.md`
2. `Docs/software/file-map.md`
3. `Docs/software/architecture.md` 第 8 节
4. 常用命令见 `Docs/development/commands.md`
