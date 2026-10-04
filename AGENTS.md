# STM32 Fighting Controller Project Instructions

## Project overview

本项目为基于 STM32F103C8T6 的格斗手柄宏控制系统。单片机通过飞线连接手柄主板，读取机械按键输入，并通过开漏输出模拟手柄信号线与地短接。

## Mandatory architecture

后续修改必须严格保留以下结构：

- STM32F103C8T6，目标主频 72 MHz。
- SysTick 驱动约 60 FPS、每帧约 16.7 ms 的主循环。
- 保留以下主要函数：
  - GPIO_Init_All()
  - ReadInputs()
  - Logic_Process(in)
  - Output_Update(out)
- 保留 Action Queue 动作队列设计。
- 保留 QUEUE_LEN = 4。
- 保留 MAX_FRAMES = 4。
- 保留 HOLD_THRESHOLD = 10。
- 保留 PA0～PA9 开漏输出结构。
- 不得擅自改成推挽输出、阻塞式 delay 或完全不同的状态机架构。

## GPIO rules

- 输出 PA0～PA9 为开漏输出。
- 保留：
  - GPIOA->CRL = 0x77777777
  - GPIOA->CRH = 0x77
  - GPIOA->ODR |= 0x03FF
- 按键一端接 IO，另一端接 GND，输入为低电平有效。
- 输入读取必须保持低电平取反逻辑。
- 关闭 JTAG、保留 SWD，以释放相关引脚：
  - AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1
- PA13 用于 IN_LA1 时，不得破坏 SWD 调试能力。
- PB2 存在实际接线问题，不应恢复为 LEFT 输入。

## Input definitions

- IN_UP      = 1 << 0
- IN_DOWN    = 1 << 1
- IN_LEFT    = 1 << 2
- IN_RIGHT   = 1 << 3
- IN_LP      = 1 << 4
- IN_MP      = 1 << 5
- IN_HP      = 1 << 6
- IN_LK      = 1 << 7
- IN_MK      = 1 << 8
- IN_HK      = 1 << 9
- IN_SUPER   = 1 << 10
- IN_SP1     = 1 << 11
- IN_SP2     = 1 << 12
- IN_SP3     = 1 << 13
- IN_SP4     = 1 << 14
- IN_X       = 1 << 15
- IN_COMBO1  = 1 << 16
- IN_COMBO2  = 1 << 17
- IN_COMBO3  = 1 << 18
- IN_LA1     = 1 << 19

## Output definitions

- OUT_UP     = 1 << 0
- OUT_DOWN   = 1 << 1
- OUT_LEFT   = 1 << 2
- OUT_RIGHT  = 1 << 3
- OUT_LP     = 1 << 4
- OUT_MP     = 1 << 5
- OUT_HP     = 1 << 6
- OUT_LK     = 1 << 7
- OUT_MK     = 1 << 8
- OUT_HK     = 1 << 9

## Input mapping

- PB0～PB9：UP、DOWN、LEFT、RIGHT、LP、MP、HP、LK、MK、HK。
- PB10：SUPER。
- PB11～PB13：COMBO1、COMBO2、COMBO3。
- PB15：X 镜像键。
- PA13：LA1。
- 如果实际工程代码与本说明存在接线差异，先报告差异，不得自行猜测和重映射。

## Trigger behavior

- SP、SUPER、COMBO 等宏动作必须使用按下沿触发。
- 必须使用当前输入与 prev 输入比较，避免按住按键时重复入队。
- 普通方向输入不得受 X 镜像键影响。
- X 只镜像 SP 和 SUPER 的左右方向。
- 动作在 4 帧窗口内通过 Action Queue 逐帧输出。
- 不得用长时间阻塞延时实现连招。

## Macro behavior

- SP1：下 → 下左 → 左 → 拳。
- SP2：下 → 下右 → 右 → 拳。
- SP3：使用 SP1 的方向，拳替换为脚。
- SP4：使用 SP2 的方向，拳替换为脚。
- SP1 + DOWN：最终中拳。
- SP1 + LEFT：最终轻拳。
- SP1 + RIGHT：最终重拳。
- 无方向时默认中拳 + 重拳。
- SUPER 为两次方向序列后输出最终按键。
- RIGHT + SUPER 的最终按键为中拳。
- 其他 SUPER 情况最终按键为中腿。
- COMBO1 = LP + LK。
- COMBO2 = MP + MK。
- COMBO3 = HP + HK。

## Debugging

排查输入或动作问题时优先检查：

1. SystemCoreClock 是否等于 72 MHz。
2. SysTick 是否确实约为 60 FPS。
3. debug_in 中对应输入位是否正确变化。
4. prev 输入是否在正确时间更新。
5. 是否使用按下沿而不是持续电平触发。
6. queue_head、queue_tail 是否变化。
7. 队列是否已满。
8. Output_Update 是否保持开漏逻辑。
9. 实际飞线是否与输入位定义一致。

保留并优先使用：

- debug_in
- queue_head
- queue_tail
- TESTFLAG

## Change policy

- 修改前先展示相关函数和调用关系。
- 一次只修复一个明确问题。
- 不得在没有说明的情况下整体重写工程。
- 不得擅自修改 GPIO、帧率、队列长度、动作帧数或输入位定义。
- 如果确实需要改变架构，必须先解释原因、兼容性影响和迁移方案，等待用户确认。
- 修改后必须说明具体改动、预期输入、逐帧输出和验证方法。

## User collaboration preference

- 用户不具备专业技术背景。对已经明确提出的任务，直接完成检查、实施和验证，不要求用户先理解技术方案。
- 发现可以自行解决的问题时，先排查并解决；不得只报告问题而把解决过程留给用户。
- 完成后只需用通俗中文汇报结果、实际改动、验证结果和仍存在的风险。
- 常规的依赖安装、项目内检查、测试和修复可以直接执行。
- 涉及删除或覆盖用户资料、重置 Git 历史、修改系统级配置或环境变量、硬件下载/烧录、向远程仓库推送或对外发送内容时，必须先确认。
