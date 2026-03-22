# RTSP 客户端开发进度

## 当前状态
- 模式: Build Mode
- 进度: 第一步完成，准备第二步

---

## 学习方式
1. 用户实现代码，我提问引导思考
2. 阶段性审查代码，提出问题
3. 优化完成后进入下一部分

---

## 第一步：RtspPlayerContext ✅ 完成

### 文件位置
```
src/Rtsp/RtspPlayerContext.h
src/Rtsp/RtspPlayerContext.cpp
```

### 单元测试
```
tests/test_rtsp_player_context.cpp
tests/CMakeLists.txt
```

### 测试命令
```bash
cd tests/build
cmake .. && make
LD_LIBRARY_PATH=/path/to/3rdpart/NWTool/bin /home/workSpace/RtspServer/bin/test_rtsp_player_context
```

### 成员变量

| 变量 | 类型 | 说明 |
|------|------|------|
| `_url` | std::string | 完整 URL |
| `_schema` | std::string | 协议类型 (rtsp/rtsps) |
| `_host` | std::string | 主机地址 |
| `_port` | uint16_t | 端口（默认 554） |
| `_app` | std::string | 应用路径 |
| `_streamid` | std::string | 流 ID |
| `_username` | std::string | 用户名 |
| `_password` | std::string | 密码 |
| `_cseq` | int | 请求序列号 |
| `_session_id` | std::string | Session ID |
| `_content_base` | std::string | 基准路径（末尾带/） |
| `_tracks` | vector<Track::Ptr> | Track 列表 |

### 错误码枚举

```cpp
enum class ParseUrlError {
    kSuccess = 0,
    kProtocolError,      // 协议头错误
    kMissingIp,          // 缺少 IP
    kAddressFormatError  // 地址格式错误
};
```

### URL 解析流程

```
ParseUrl()
    │
    ├─► ParseProtocol()   // 1. 验证 rtsp:// 或 rtsps://
    │
    ├─► ParseUserInfo()   // 2. 提取 username:password
    │
    ├─► ParseHostPort()  // 3. 提取 host:port
    │
    └─► ParsePath()      // 4. 提取 app/streamid
```

### 需要实现的方法

| 方法 | 说明 |
|------|------|
| `ParseUrl()` | 主入口，返回 ParseUrlError |
| `GetCurrentUrl()` | 重新组装 URL |
| `GetControlUrl()` | 拼接 Track URL |
| `NextCSeq()` | 递增并返回 CSeq |
| `GetAuthHeader()` | 生成 Basic 认证头 |
| `SetSessionId()` / `GetSessionId()` | Session ID 管理 |
| `AddTrack()` / `GetTrack()` / `GetTrackCount()` | Track 管理 |
| `Reset()` | 重置所有状态 |

### 实现注意事项

| 要点 | 说明 |
|------|------|
| 默认端口 | rtsp=554, rtsps=322 |
| Base64 | Basic 认证需要手动实现 |
| _content_base | 末尾带 `/`，用于拼接 Track URL |
| schema | 统一转小写比较 |
| _app | 最后一个 / 之前的所有路径（带前导 /） |
| _streamid | 最后一个 / 之后的内容 |

### 发现的 Bug 及修复

| Bug | 修复 |
|-----|------|
| _app 缺少前导 / | 修改 ParsePath()：`"/" + tmp_part.substr(0,last_slash)` |

---

## 下一步

**第二步：SdpParser**

| 内容 | 说明 |
|------|------|
| SDP 解析 | 从 DESCRIBE 响应中提取 Track 信息 |
| Track 信息 | payload type, 编码格式, URL |

---

## RTSP 协议关键点备忘

### RTSP 状态机
```
OPTIONS → DESCRIBE → SETUP(每个Track) → PLAY → (PAUSE) → TEARDOWN
```

### Track URL 拼接
```cpp
_content_base = "rtsp://host:port/app/stream/"  // 末尾带 /
track_url = "trackID=0"
full_url = _content_base + track_url
```

### SETUP 需要每个 Track 单独发送
- 视频 Track → interleaved=0-1
- 音频 Track → interleaved=2-3

### Basic 认证
```
Authorization: Basic Base64(username:password)
```

### SDP 格式示例
```sdp
v=0
o=- 1234567890 1234567890 IN IP4 127.0.0.1
s=Test Stream
c=IN IP4 0.0.0.0
t=0 0
m=video 0 RTP/AVP 96
a=rtpmap:96 H264/90000
a=fmtp:96 profile-level-id=42e01e;sprop-parameter-sets=Z00AHp2oKAi6whE=,aO4Ehlg=
a=control:trackID=0
m=audio 0 RTP/AVP 97
a=rtpmap:97 mpeg4-generic/44100/2
a=control:trackID=1
```
