# AGENTS.md - RtspServer 开发规范

## 项目概述

RtspServer 是一个基于 C++11 的 RTSP 流媒体服务器学习项目，采用 Reactor 模式的事件驱动网络库（NWTool）实现。

## 构建命令

```bash
# 配置 CMake 项目
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug

# 编译项目
cmake --build build

# 或在 build 目录中使用 make
cd build && make

# 单独编译指定目标
cmake --build build --target RtspServer
cmake --build build --target RtspClient
```

## 代码质量检查

```bash
# 代码格式化（使用 clang-format，LLVM 风格）
make format

# cpplint 代码检查（Google 风格）
make cpplint

# clang-tidy 静态分析
make clang-tidy

# 查看格式化 diff
make check-format
```

## 测试命令

测试代码位于 `3rdpart/NWTool/tests/` 目录：

```bash
# 在 NWTool 目录构建测试
cd 3rdpart/NWTool
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# 运行单个测试
./bin/test_httpcontext
./bin/test_generateSSRC
./bin/test_tcpclient
```

## 代码风格

### 格式化工具
- **clang-format**: 代码格式化（LLVM 风格，120 字符行宽）
- **cpplint**: Google C++ 代码风格检查
- **clang-tidy**: LLVM 静态分析

### 命名约定

| 类型 | 约定 | 示例 |
|------|------|------|
| 类名 | PascalCase | `RtspServer`, `TcpConnection`, `EventLoop` |
| 方法名 | PascalCase | `OnConnection`, `HandleClose`, `setLogLevel` |
| 成员变量 | snake_case + 下划线后缀 | `loop_`, `server_`, `auto_close_conn_` |
| 私有成员 | 下划线前缀 | `_cseq`, `_sessionid`, `_rtp_type` |
| 枚举值 | PascalCase 或全大写 | `eRtpType::RTP_TCP`, `RTCP_TYPE::RTCP_SR` |
| 常量 | k + PascalCase | `kMinSize` |
| 宏 | 全大写 + 下划线 | `DISALLOW_COPY_AND_MOVE`, `LOG_INFO` |

### include 顺序
1. 对应头文件
2. 项目内部头文件（按模块组织）
3. C 系统头文件（`<arpa/inet.h>` 等）
4. C++ 标准库头文件

### 头文件保护
```cpp
#ifndef MODULE_NAME_H
#define MODULE_NAME_H
// ...
#endif // MODULE_NAME_H
```

## 错误处理

### 日志系统（HooLog）
```cpp
#include "HooLog/HooLog.h"

// 日志级别：TRACE < DEBUG < INFO < WARN < ERROR < FATAL
LOG_TRACE << "trace message";
LOG_DEBUG << "debug message";
LOG_INFO  << "info message";
LOG_WARN  << "warning message";
LOG_ERROR << "error message";
LOG_FATAL << "fatal message";  // 会 flush 并 abort 程序

setLogLevel(loglevel::DEBUG);
```

### 错误码枚举
```cpp
enum RC {
  RC_SUCCESS,
  RC_SOCKET_ERROR,
  RC_POLLER_ERROR,
  RC_CONNECTION_ERROR,
  RC_ACCEPTOR_ERROR,
  RC_UNIMPLEMENTED
};
```

### 边界检查宏
```cpp
#define CHECK_MIN_SIZE(size, kMinSize) \
if (size < kMinSize) { \
    throw std::out_of_range("rtcp 包长度不足"); \
}
```

### 返回值约定
- 失败返回 `false`，成功返回 `true` 或实际数据
- 使用 `std::optional` 或智能指针管理可能为空的结果

## 内存管理

### 禁止拷贝和移动
```cpp
class EventLoop {
public:
    DISALLOW_COPY_AND_MOVE(EventLoop);
};
```

### 智能指针优先
- 使用 `std::shared_ptr` / `std::unique_ptr` 管理对象生命周期
- 使用 `std::enable_shared_from_this` + `weak_self` 模式避免循环引用

## 线程安全

- EventLoop 采用单线程事件循环模式
- 避免夸线程调用，使用 `weak_ptr` 打破循环引用
- 使用 `std::lock_guard` 进行 mutex 保护

## 项目结构

```
src/
├── Rtsp/           # RTSP 协议实现
│   ├── RtspServer.h/.cpp
│   ├── RtspSession.h/.cpp
│   ├── RtspClient.h/.cpp
│   ├── Rtsp.h/.cpp
│   └── H264.h/.cpp
├── Rtcp/           # RTCP 实现
│   ├── Rtcp.h/.cpp
│   └── RtcpContext.h/.cpp
server/             # 服务器入口
client/             # 客户端入口
3rdpart/NWTool/     # 网络工具库（子模块）
├── src/
│   ├── Event/      # 事件循环
│   ├── NetWork/    # TCP 网络
│   ├── Http/       # HTTP 解析
│   ├── Timer/      # 定时器
│   ├── Thread/     # 线程池
│   └── HooLog/     # 日志系统
└── tests/          # 单元测试
```

## 调试配置

VSCode 调试配置见 `.vscode/launch.json`：
- `RtspServer`: 调试 RTSP 服务器
- `RtspClient`: 调试 RTSP 客户端
- 自动忽略 SIGPIPE 信号
