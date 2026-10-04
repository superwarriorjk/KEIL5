# STM32 街霸宏手柄工程

基于 STM32F103C8T6 的格斗手柄宏控制项目，使用 Keil MDK 和 ARM Compiler 5。
单片机读取低电平有效的机械按键，通过 PA0～PA9 开漏输出模拟手柄按键。

## 目录

| 路径 | 内容 |
| --- | --- |
| `SF6/STREETFIGHT6.uvprojx` | 主工程入口 |
| `SF6/main.c` | 主工程输入、动作队列和输出逻辑 |
| `SF6/RTE/` | 启动代码、时钟配置及运行环境配置 |
| `SF6/beifen.c` | 备份源码，目前整份代码处于注释中 |
| `SF6/jb6.docx` | 接线表和历史代码记录 |
| `SF6/BEIFEN/sangerfu/laosang.uvprojx` | 保留的另一套宏动作工程 |
| `AGENTS.md` | 项目架构约束与修改要求 |

## 打开工程

使用 Keil µVision 打开 `SF6/STREETFIGHT6.uvprojx`。
工程配置使用 ARM Compiler 5.06 update 3 和 Keil STM32F1xx_DFP 1.0.5。
另一个版本可单独打开 `SF6/BEIFEN/sangerfu/laosang.uvprojx`。

源码时钟配置以外部 8 MHz 晶振为基础，目标主频为 72 MHz。
在该主频下，SysTick 周期约为 16.7 ms。
保留长度为 4 的动作队列、`MAX_FRAMES = 4` 和 `HOLD_THRESHOLD = 10`。

## 当前状态

此仓库保存现有工程快照，未修改控制逻辑，也未进行硬件烧录。
当前按键映射与 `AGENTS.md`、历史接线文档存在差异，部分宏触发和输出行为仍需修复。
实际飞线不能仅凭文件判断，修改接线映射前应先核对硬件。

编译结果、缓存、包含许可证信息的编译日志及个人 IDE 布局不纳入版本管理。
打开和编译工程时，Keil 会重新生成相应文件。

## 本地版本管理

仓库工作目录为 `D:\UINIVERSITY\KEIL5\GITKU`。
本仓库由原目录中的项目文件复制建立，原始 `SF6` 目录仍保留。
两份目录是独立副本；后续需要提交的修改应在本仓库中进行。
