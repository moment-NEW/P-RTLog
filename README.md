# P-RTLog

P-RTLog 是一个面向嵌入式的轻量 tokenized log 示例库。它把日志格式串在编译期保存到 AXF/ELF 的 token section 中，运行时只通过 RTT 发送 token 和参数，主机端再根据 AXF/ELF 还原成人类可读日志。

当前已验证的主链路是：

```text
P_LOG / P_WARN / P_ERROR
  -> 编译期生成 32-bit token
  -> 运行时通过 SEGGER RTT 输出二进制帧
  -> RTTView 读取 AXF 中的 token section 并解码显示
```

运行时 RTT 传输格式：

```text
[2-byte little-endian payload length][4-byte token][encoded args...]
```

## 前言

这个项目的目标是验证并整理一条可用的嵌入式 tokenized log 路径：固件端尽量少传数据，主机端利用 AXF/ELF 中保留的 token 数据库还原日志。

当前版本先以 Keil + STM32H7 + SEGGER RTT + RTTView 为主要验证环境。文档只描述已经跑通的部分，未验证的后端和复杂功能暂不展开。

## 简要原理

普通日志会把完整字符串发出去，例如：

```text
Hard to say,maybe 12 times?
```

Tokenized log 则把格式串编译进 AXF/ELF 的 token section，运行时只发送：

```text
token + encoded arguments
```

例如：

```c
P_ERROR("Hard to say,maybe %d times?", times);
```

固件端 RTT 实际发送的是：

```text
[length][token][encoded times]
```

RTTView 打开同一个 AXF 后，从 token section 取出 token 与格式串的映射，再把 RTT 二进制帧解码成原始文本。

这种方式的优点是：

- RTT 上传输的数据更少。
- 字符串主要保存在 AXF/ELF 中，运行时只传 token 和参数。
- 支持 `%d` 这类参数化日志。

## 当前验证状态

测试工程在 `example/` 下，主要测试代码位于：

- `example/Src/main.c`
- `example/User/log_tokenized/backend/RTT/rtt_backend.c`

当前 Keil 示例已验证 3 条 tokenized log：

```c
P_LOG("I am the storm that is approaching！");
P_WARN("How many times have we fight?");
P_ERROR("Hard to say,maybe %d times?", times);
```

其中：

- `P_LOG`：纯字符串，已通过
- `P_WARN`：纯字符串，已通过
- `P_ERROR`：`%d` 整数参数，已通过

默认 RTT 通道：

```c
#define P_RTT_LOG_CHANNEL 0
```

## 快速测试

1. 打开 Keil 工程：

   `example/MDK-ARM/CubeMX_Config.uvprojx`

2. Rebuild 工程。

3. 烧录到目标板。

4. 使用 RTTView 打开生成的 AXF：

   `example/MDK-ARM/CubeMX_Config/CubeMX_Config.axf`

5. RTTView 中选择 `P-RTLog Tokenized` 显示模式。

正常输出示例：

```text
I am the storm that is approaching！ [module=default, file=../Src/main.c]
How many times have we fight? [module=default, file=../Src/main.c]
Hard to say,maybe 1 times? [module=default, file=../Src/main.c]
```

## 关键文件

```text
example/
├── Src/main.c
├── MDK-ARM/CubeMX_Config.uvprojx
└── User/log_tokenized/
    ├── log_tokenized_light.h
    ├── plog_tokenized_light.c
    ├── p_rtlog_config.h
    ├── backend/RTT/rtt_backend.c
    └── backend/RTT/rtt_backend.h

scripts/linker/p_rtlog_tokenizer_sections.sct
```

## 集成到自己的工程

下面以当前已验证的 RTT 后端为例。

### 1. 拷贝源码

可以参考 `example/User/` 的组织方式，把以下目录/文件放入自己的工程，例如放到 `User/` 下：

```text
log_tokenized/
├── log_tokenized_light.h
├── plog_tokenized_light.c
├── p_rtlog_config.h
├── light_handler.h
├── backend/RTT/
│   ├── rtt_backend.c
│   ├── rtt_backend.h
│   ├── SEGGER_RTT.c
│   ├── SEGGER_RTT.h
│   ├── SEGGER_RTT_Conf.h
│   └── SEGGER_RTT_ConfDefaults.h
└── ...

tokenizer/
p-span/
p-macro.h
p_varint.h
stubs/
```

如果使用仓库根目录的 `src/`，对应关系基本一致：

```text
src/log_tokenized/
src/tokenizer/
src/p-span/
src/p-macro.h
src/p_varint.h
stubs/
```

### 2. 添加源文件

至少需要加入这些 `.c` 文件：

```text
log_tokenized/plog_tokenized_light.c
log_tokenized/backend/RTT/rtt_backend.c
log_tokenized/backend/RTT/SEGGER_RTT.c
```

如果使用汇编优化版本 RTT，也可以按目标架构加入：

```text
log_tokenized/backend/RTT/SEGGER_RTT_ASM_ARMv7M.S
```

### 3. 添加 include 路径

以 `example/User/` 方式集成为例，常用 include 路径包括：

```text
User/
User/log_tokenized/
User/log_tokenized/backend/RTT/
User/tokenizer/
User/tokenizer/internal/
User/p-span/
User/stubs/
```

路径可根据实际工程目录调整。

### 4. 配置后端

当前默认启用 RTT 后端。也可以在工程宏中显式定义：

```text
USING_RTT_BACKEND
```

默认 RTT 输出通道在 `p_rtlog_config.h` 中：

```c
#define P_RTT_LOG_CHANNEL 0
```

### 5. 配置 linker / scatter file

这一步很重要：tokenized log 的格式串必须保留在 AXF/ELF 中，否则主机端无法建 token DB。

#### Keil / ARM Compiler 6

在 scatter file 中包含：

```text
INCLUDE scripts/linker/p_rtlog_tokenizer_sections.sct
```

当前 RTTView 已支持 Keil 输出区：

```text
ER_PW_TOKENIZER
```

#### GCC linker script

参考：

```text
scripts/linker/p_rtlog_tokenizer_sections.ld
```

常见 section 名为：

```text
.pw_tokenizer.entries
```

### 6. 在代码中使用

初始化 RTT 后端：

```c
#include "log_tokenized/log_tokenized_light.h"
#include "log_tokenized/backend/RTT/rtt_backend.h"

int main(void) {
   // HAL / clock / peripheral init ...

   rtt_backend_init();

   while (1) {
      static uint8_t times = 0;
      times++;

      P_LOG("I am the storm that is approaching！");
      P_WARN("How many times have we fight?");
      P_ERROR("Hard to say,maybe %d times?", times);

      HAL_Delay(100);
   }
}
```

### 7. 用 RTTView 查看

1. 编译并烧录固件。
2. 在 RTTView 中选择生成的 AXF/ELF。
3. 显示模式选择 `P-RTLog Tokenized`。
4. 打开连接。

RTTView 会从 AXF/ELF 中读取 token section，并优先通过 `_SEGGER_RTT` 符号定位 RTT control block。

## Keil token section

Keil/ARM Compiler 6 工程需要保留 token section，供 RTTView 从 AXF 中提取 token 数据库。

当前 RTTView 已支持 Keil 输出区：

```text
ER_PW_TOKENIZER
```

也兼容常见 section 名：

```text
.pw_tokenizer.entries
```

## 注意事项

- RTT 后端会把 header 和 payload 组装后一次性写入 RTT，避免高频输出时 length frame 被打断。
- 示例代码中保留了 `HAL_Delay(100)`，用于避免 RTT buffer 被连续日志刷满。
- 当前文档只描述已验证的 RTT tokenized log 路径。

## License

MIT