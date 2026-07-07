# What is P-RTLog

P-RTLog 是 P-Momentum 工具集中的日志模块。它集成并裁剪了 Pigweed 的哈希化日志（tokenized log）与 SEGGER RTT 的实时传输能力，提供最大可配置性与开箱即用性。

由于 RTT Viewer 原生不支持 tokenized log，建议配合工具集中的 **RTT-View Ciallo** 使用（它还支持 DAPLink 哦）。

# 前言

项目的初衷是实现轻量化的 tokenized 库，没想到谷歌的 pigweed 依赖十分的繁杂，一个函数可以套五六层，然后用 inline 展开这种惊为天人的操作。本库因为是移植的 pw_tokenized_log，多少依然有类似的依赖情况，只是删除和改写了部分文件，没有全部封装到一起。后续会按照 P-Momentum 的思想逐步改进为更轻量化，方便配置的形式。
原本我也想实现一些简洁优雅的代码，但是最终还是以非常简陋的形式呈现了。或许之后如果常用到的话，会有时间慢慢完善吧
## 工作原理

```
应用代码 → P_LOG() → hash token + 编码参数 → RTT ring buffer → J-Link → 主机端解码显示
```

- **Pigweed pw_log_tokenized**：格式串在编译期哈希化为 32-bit token，运行时只传输 token + 变长编码的参数，极大节省带宽。
- **SEGGER RTT**：通过共享内存 ring buffer 实现零拷贝、非侵入式日志传输，J-Link 通过调试接口读取，不占用外设引脚。

## 目录结构

```
P-RTLog/
├── src/                          # 核心源码
│   ├── log_tokenized/            # Tokenized logging 实现
│   │   ├── backend/RTT/          # RTT 后端（rtt_backend.h/.c）
│   │   ├── config.h              # 配置 + 后端选择宏
│   │   ├── log_tokenized_light.h # 公共 API（P_LOG 宏）
│   │   └── log_tokenized_light.cc
│   ├── tokenizer/                # Tokenizer 核心（hash、encode、section）
│   ├── p-macro.h                 # 通用宏工具
│   ├── p_varint.h                # Varint 编码
│   └── p-span/                   # Span 实现
├── config/                       # 用户自定义配置
├── scripts/
│   ├── linker/                   # Linker script 片段（Keil .sct）
│   └── decode_rtt_log.py         # 主机端解码脚本
├── stubs/                        # Pigweed 依赖 stub（to_array, span 等）
├── third_party/
│   ├── pigweed/                  # pw_tokenizer + pw_log_tokenized
│   └── segger_rtt/               # SEGGER RTT 源码
└── CMakeLists.txt                # CMake 构建（Ninja）
```

## 快速开始

### 1. 编译验证（主机端，GCC + Ninja）

```bash
# 配置
cmake -B build -G Ninja

# 编译
cmake --build build
```

编译成功后会生成 `build/libp_rtlog.a` 静态库。

### 2. 集成到你的嵌入式项目

#### 添加源文件

将以下文件加入你的工程：

| 文件 | 说明 |
|------|------|
| `src/log_tokenized/log_tokenized_light.cc` | Tokenized logging 核心 |
| `src/log_tokenized/backend/RTT/rtt_backend.c` | RTT 后端实现 |
| `third_party/segger_rtt/RTT/SEGGER_RTT.c` | RTT 驱动 |
| `third_party/segger_rtt/RTT/SEGGER_RTT.h` | RTT 头文件 |
| `third_party/segger_rtt/RTT/SEGGER_RTT_ConfDefaults.h` | RTT 默认配置 |
| `third_party/segger_rtt/Config/SEGGER_RTT_Conf.h` | RTT 用户配置 |

#### 添加 Include 路径

```
src/
src/p-span/
src/log_tokenized/
src/log_tokenized/backend/RTT/
third_party/pigweed/pw_tokenizer/pw_tokenizer/public/
third_party/pigweed/pw_log_tokenized/public/
third_party/pigweed/pw_log_tokenized/public_overrides/
third_party/pigweed/pw_log_tokenized/light_public_overrides/
third_party/segger_rtt/RTT/
third_party/segger_rtt/Config/
stubs/
```

#### 添加编译宏

```
-DUSING_RTT_BACKEND
-DP_TOKENIZER_CFG_ARG_TYPES_SIZE_BYTES=4
-DP_TOKENIZER_CFG_C_HASH_LENGTH=128
```

> **后端选择**：在编译宏中定义 `USING_RTT_BACKEND` 启用 RTT 后端。同时定义多个后端会触发编译错误。

#### 配置 Linker Script

将 pigweed 的 section 定义加入你的 linker script，使 tokenized 字符串保留在 ELF 中供主机端解码：

**GCC (arm-none-eabi-ld)**：在 linker 命令中添加：
```
-T third_party/pigweed/pw_tokenizer/pw_tokenizer/pw_tokenizer_linker_sections.ld
```

**Keil (ARM Compiler 6)**：在 scatter file 中 include：
```
INCLUDE scripts/linker/p_rtlog_tokenizer_sections.sct
```

> 这些 section 类型为 `INFO`，**不会烧录到 MCU**，仅保留在 ELF 文件中。

### 3. 在代码中使用

```c
#include "log_tokenized/log_tokenized_light.h"
#include "log_tokenized/backend/RTT/rtt_backend.h"

int main(void) {
    rtt_backend_init();  // 初始化 RTT（可选，SEGGER_RTT 会自动初始化）

    P_LOG_INFO("System started, version %d.%d", 1, 0);
    P_LOG_WARN("Temperature: %f C", 85.3f);
    P_LOG_ERROR("Motor %d stalled!", 2);

    while (1) {
        // ...
    }
}
```

### 4. 主机端解码

```bash
# 安装依赖
pip install pyelftools

# 解码 RTT 日志
python scripts/decode_rtt_log.py firmware.elf rtt_capture.bin
```

解码脚本会：
1. 从 ELF 的 `.pw_tokenizer.entries` section 提取 token → 格式串数据库
2. 从 RTT 捕获数据中读取 `[2字节长度][token+参数]` 帧
3. 查表还原原始日志

## 配置

### RTT 通道

默认使用 RTT Channel 0（与 RTT Viewer Terminal 共享）。如需分离日志和终端 I/O，在编译宏中指定：

```
-DP_RTT_LOG_CHANNEL=1
```

### 后端扩展

在 `src/log_tokenized/config.h` 中通过编译宏选择后端：

| 宏 | 说明 | 状态 |
|---|---|---|
| `USING_RTT_BACKEND` | SEGGER RTT (J-Link) | ✅ 已实现 |
| `USING_UART_BACKEND` | UART 串口 | 🔜 预留 |
| `USING_SWO_BACKEND` | SWO/ITM | 🔜 预留 |

## 参考项目

- [pigweed-project/pigweed — pw_log_tokenized](https://github.com/pigweed-project/pigweed/tree/main/pw_log_tokenized)
- [pigweed-project/pigweed — pw_log_zephyr](https://github.com/pigweed-project/pigweed/tree/main/pw_log_zephyr)
- [SEGGERMicro/RTT](https://github.com/SEGGERMicro/RTT)

## License

MIT