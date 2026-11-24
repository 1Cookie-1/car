# 常用命令速查

> 所有命令都在仓库根目录 `/d/STM/car` 下执行，提示符后面应能看到 `(main)`。
> 除标注「未实测」的条目外，其余均在本机验证过。

## 1. 编译固件

```bash
cd /d/STM/car
cmake --preset Debug                # 只在第一次 / 改过 CMakeLists 后跑
cmake --build --preset Debug -j4
```

成功标志：最后一行是 `[100%] Built target car`
产物路径：`build/Debug/car.elf`

## 2. 干净重建（提交前必做）

```bash
rm -rf build
cmake --preset Debug
cmake --build --preset Debug -j4
```

为什么要删：增量构建输出 `ninja: no work to do` 时，等于没验证。

## 3. 看 Flash / RAM 占用

```bash
arm-none-eabi-size build/Debug/car.elf
```

输出三列：`text`（Flash）、`data` + `bss`（RAM）。
当前基线：Flash 约 14.6 KB / 512 KB，RAM 约 1.8 KB / 128 KB。

## 4. 跑 PC 测试（7 项）

```bash
cmake -S tests -B tests/build       # 只在第一次跑
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
```

成功标志：`100% tests passed, 0 tests failed out of 7`

只跑其中一项（例如只看 PID）：

```bash
ctest --test-dir tests/build -R pid --output-on-failure
```

注意：这里测的是纯算法，**不测硬件、不测 main.c**。

## 5. 烧录（ST-LINK V2 + OpenOCD，未实测）

先探测芯片在不在（只读，不改任何东西）：

```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "init; halt; mdw 0xE0042000 1; shutdown"
```

期望输出含：`0xe0042000: 10076413`（低 12 位 `0x413` = F407）

确认能连上再烧：

```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program build/Debug/car.elf verify reset exit"
```

期望看到 `** Programming Finished **` 和 `** Verified OK **`

坑：
- Git Bash 里换行续行用 `\`，不是 `^`（`^` 是 CMD 的）
- `Error: open failed` = 没接板子 / 板子没供电 / GND 没接

## 6. git 提交

```bash
git status --short          # 先看改了什么
git diff --stat             # 看改了哪几个文件、几行
git add -A
git commit -m "fix(pid): dt<=0 不再改写状态"
git push
```

改动较大时带正文（每多一个 `-m` 就是一段）：

```bash
git commit -m "fix(pid): dt<=0 不再改写状态" -m "Why: 采样异常时微分项冲击。
Verify: 已编译验证 0 error 0 warning；测试 7/7 通过。"
```

看历史：

```bash
git log --oneline -5        # 最近 5 条，一行一条
git log -1 --format=%B      # 最近一条的完整信息（含正文）
```

撤销：

```bash
git restore 文件名              # 丢弃未提交的改动（不可恢复，慎用）
git restore --staged 文件名     # 取消暂存，改动还留着
```

## 7. 报错对照表

| 报错 | 原因 | 处理 |
| --- | --- | --- |
| `not a git repository` | 不在仓库目录 | `cd /d/STM/car` |
| `nothing to commit` | 文件没保存 | VS Code 里 Ctrl+S |
| `Unexpected command line argument: ^` | 用了 CMD 的续行符 | 改 `\` 或写一行 |
| `Error: open failed` | 没接板子 / 没供电 / GND 没接 | 查线，查供电 |
| 编译报 `arm-none-eabi-gcc not found` | 工具链不在 PATH | 检查是否装了 xPack 工具链 |

## 8. 提交前检查清单

- [ ] 干净构建：0 error 0 warning
- [ ] 涉及算法时：测试 7/7 通过
- [ ] `git status` 里没有意外的文件（比如临时脚本、截图）
- [ ] 没验证的部分，提交信息里写明「未编译验证」
## 9. 撤销与还原

| 处境 | 命令 |
| --- | --- |
| 改了没 add | `git restore 文件` |
| add 了没 commit | `git restore --staged .` |
| commit 了没 push，改信息 | `git commit --amend -m "新信息"` |
| commit 了没 push，补漏的文件 | `git add 文件` 后 `git commit --amend --no-edit` |
| commit 了没 push，整个撤掉 | `git reset --soft HEAD~1`（改动留在暂存区） |
| 已经 push | `git revert HEAD` 后 `git push`（生成反向提交，历史保留） |
| 误删文件 | `git restore 文件` |
| 误 reset / 提交找不到了 | `git reflog` 查哈希，再 `git reset --hard 哈希` |

**铁律**：只有 `--hard` 和 `--force` 会真丢东西，其余撤销都能再撤回来。

- `git reset --hard` 丢你的改动
- `git push --force` 丢远端历史