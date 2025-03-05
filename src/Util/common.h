#ifndef COMMON_H
#define COMMON_H

#include <random>
#include <string>

class Epoller;
class Channel;
class Buffer;
class EventLoop;
class TcpConnection;
class ThreadPool;
class Acceptor;
class TcpServer;

class HttpServer;
class HttpContext;
class HttpRequest;
class HttpResponse;


// Macros to disable copying and moving
#define DISALLOW_COPY(cname)     \
  cname(const cname &) = delete; \
  cname &operator=(const cname &) = delete;

#define DISALLOW_MOVE(cname) \
  cname(cname &&) = delete;  \
  cname &operator=(cname &&) = delete;

#define DISALLOW_COPY_AND_MOVE(cname) \
  DISALLOW_COPY(cname);               \
  DISALLOW_MOVE(cname);

// #define ASSERT(expr, message) assert((expr) && (message))

// #define UNREACHABLE(message) throw std::logic_error(message)

enum RC {
  RC_UNDEFINED,
  RC_SUCCESS,
  RC_SOCKET_ERROR,
  RC_POLLER_ERROR,
  RC_CONNECTION_ERROR,
  RC_ACCEPTOR_ERROR,
  RC_UNIMPLEMENTED
};

std::string makeRandStr(int sz, bool printable = true);

template<typename T>
T generateRandomInt() {
    //使用std::numeric_limits获取类型 T 的最大值ֵ
    T maxVal = std::numeric_limits<T>::max();
    // 创建一个随机设备，用于生成随机种子
    std::random_device rd;
    std::mt19937 gen(rd());
    // 创建一个均匀整数分布，范围是 0 到 maxVal
    std::uniform_int_distribution<T> dis(0, maxVal);
    return dis(gen);
}


#endif //COMMON_H