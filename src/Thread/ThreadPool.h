#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
private:
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> tasks;
  std::mutex tasks_mtx;
  std::condition_variable cv;
  bool stop;

public:
  ThreadPool(int size = std::thread::hardware_concurrency());
  ~ThreadPool();

  // void add(std::function<void()>);
  template <class F, class... Args>
  auto add(F &&f, Args &&...args)
      -> std::future<typename std::result_of<F(Args...)>::type>;
};

// 不能放在cpp文件，原因是C++编译器不支持模板的分离编译
template <class F, class... Args>
auto ThreadPool::add(F &&f, Args &&...args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
  using return_type = typename std::result_of<F(Args...)>::type;
  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(tasks_mtx);
    if (stop)
      throw std::runtime_error("add to stopped threadpool");
    tasks.emplace([task] { (*task)(); });
  }
  cv.notify_one();
  return res;
}

#endif // THREAD_POOL_H