# What is P-RTLog

P-RTLog 是 P-Momentum 工具集中的日志模块。它集成并裁剪了 Pigweed 的哈希化日志（tokenized log）与 SEGGER RTT 的实时传输能力，提供最大可配置性与开箱即用性。

由于 RTT Viewer 原生不支持 tokenized log，建议配合工具集中的 **RTT-View Ciallo** 使用（它还支持 DAPLink 哦）。
# 前言
项目的初衷是实现轻量化的tokenized库，没想到谷歌的pigweed依赖十分的繁杂，一个函数可以套五六层，然后用inline展开这种惊为天人的操作。本库因为是移植的pw_tokenized_log，多少依然有类似的依赖情况，只是删除和改写了部分文件，没有全部封装到一起。后续会按照P-Momentum的思想逐步改进为更轻量化，方便配置的形式。
此外，目前还没实现MVP，但是可以作为参考。后续完成了再更新
## 工作原理

```
应用代码 → PW_LOG() → hash token + 编码参数 → RTT ring buffer → J-Link → 主机端解码显示
```

- **Pigweed pw_log_tokenized**：格式串在编译期哈希化为 32-bit token，运行时只传输 token + 变长编码的参数，极大节省带宽。
- **SEGGER RTT**：通过共享内存 ring buffer 实现零拷贝、非侵入式日志传输，J-Link 通过调试接口读取，不占用外设引脚。

## 目录结构

```
P-RTLog/
├── include/prtlog/          # 公共 API 头文件
├── src/                     # 核心实现（HandleLog 回调 + RTT 后端）
├── config/                  # 默认配置（RTT buffer 大小、log level 等）
├── tools/                   # 主机端工具（detokenize、实时日志查看）
├── examples/                # 使用示例
└── third_party/             # 第三方依赖
    ├── pigweed/             # pw_log_tokenized + pw_log_zephyr
    └── segger_rtt/          # SEGGER RTT 源码
```

## 快速开始

> TODO

## 参考项目

- [pigweed-project/pigweed — pw_log_tokenized](https://github.com/pigweed-project/pigweed/tree/main/pw_log_tokenized)
- [pigweed-project/pigweed — pw_log_zephyr](https://github.com/pigweed-project/pigweed/tree/main/pw_log_zephyr)
- [SEGGERMicro/RTT](https://github.com/SEGGERMicro/RTT)

## License

MIT