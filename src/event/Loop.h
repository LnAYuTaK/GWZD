#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>

class FdEvent;
class EpollFdEvent;
class Loop {
 public:
  static Loop *New();
  //! 获取引擎列表
  enum class Mode {
    kOnce,    //!< 仅执行一次
    kForever  //!< 一直执行
  };
  //! 执行事件循环
  virtual void runLoop(Mode mode = Mode::kForever) = 0;
  //! 退出事件循环
  virtual void exitLoop(const std::chrono::milliseconds &wait_time =
                            std::chrono::milliseconds::zero()) = 0;

  //! 是否与Loop在同一个线程内
  virtual bool isInLoopThread() = 0;
  //! Loop是否正在运行
  virtual bool isRunning() const = 0;

  //! 委托延后执行动作
  using Func = std::function<void()>;

  virtual  EpollFdEvent *creatFdEvent(const std::string &fd_name = "") = 0;

  //! 阈值
  struct WaterLine {
    size_t run_in_loop_queue_size;               //!< runInLoop() 队列长度
    size_t run_next_queue_size;                  //!< runNext() 队列长度
    std::chrono::nanoseconds wake_delay;         //!< loop 唤醒延迟
    std::chrono::nanoseconds loop_cost;          //!< loop 时间消耗
    std::chrono::nanoseconds event_cb_cost;      //!< 事件回调时间消耗
    std::chrono::nanoseconds run_cb_cost;        //!< Run回调时间消耗
    std::chrono::nanoseconds run_in_loop_delay;  //!< runInLoop() 执行延迟
    std::chrono::nanoseconds run_next_delay;     //!< runNext() 执行延迟
    std::chrono::nanoseconds timer_delay;        //!< 定时器延迟
  };
  // virtual WaterLine &water_line() = 0;

 public:
  virtual ~Loop() {}
};