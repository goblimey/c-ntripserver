#ifndef RTNET_SDO_EXAMPLE_CONNECTIONSTATUSMONITOR_H
#define RTNET_SDO_EXAMPLE_CONNECTIONSTATUSMONITOR_H

#include <transport/TTransport.h>
#include <concurrency/TimerManager.h>

#include <set>
//#include <cstdatomic> // needs newer version of g++ using C++0x
#include <boost/detail/atomic_count.hpp>

class ConnectionRequiredRunnable;

class ConnectionStatusMonitor
{
public:
  ConnectionStatusMonitor(boost::shared_ptr<apache::thrift::transport::TTransport>& transport,
                          boost::shared_ptr<apache::thrift::concurrency::TimerManager>& timeMgr);
  ~ConnectionStatusMonitor();

  void addListener(boost::shared_ptr<ConnectionRequiredRunnable> listener);
  void disconnected(const ConnectionRequiredRunnable* noticer);
  void tryOpen();
  void stop();
  bool connected();

private:
  class Reconnect : public apache::thrift::concurrency::Runnable
  {
  public:
    Reconnect(ConnectionStatusMonitor* parent) : parent_(parent) {}
    void run() {
      parent_->tryOpen();
    }
  private:
    ConnectionStatusMonitor* parent_;
  };
  std::set<boost::shared_ptr<ConnectionRequiredRunnable> > listeners_;
  boost::shared_ptr<apache::thrift::transport::TTransport> transport_;
  boost::shared_ptr<apache::thrift::concurrency::TimerManager> timeMgr_;
  boost::shared_ptr<apache::thrift::concurrency::ThreadFactory> thFactory_;
  //std::atomic<bool> connected_;
  //boost::atomic<bool> connected_; // need a higher version of boost or gcc than is available on kurenai
  boost::detail::atomic_count connected_; // need a higher version of boost or gcc than is available on kurenai
};

#endif // RTNET_SDO_EXAMPLE_CONNECTIONSTATUSMONITOR_H
