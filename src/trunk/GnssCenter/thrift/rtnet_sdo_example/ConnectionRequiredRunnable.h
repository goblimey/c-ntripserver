#ifndef RTNET_SDO_EXAMPLE_CONNECTIONREQUIREDRUNNABLE_H
#define RTNET_SDO_EXAMPLE_CONNECTIONREQUIREDRUNNABLE_H

#include <concurrency/Thread.h>
#include <concurrency/Monitor.h>
#include <string>

#include "ConnectionStatusMonitor.h"

class ConnectionRequiredRunnable : public apache::thrift::concurrency::Runnable
{
public:
  ConnectionRequiredRunnable(boost::shared_ptr<ConnectionStatusMonitor>& connectionMonitor, const std::string& name);
  virtual ~ConnectionRequiredRunnable() {}
  virtual void connectionLost();
  virtual void connectionEstablished();
  virtual void stop();
protected:
  virtual void disconnected();
  virtual void connectWait();
  virtual void disconnectWait();
  boost::shared_ptr<ConnectionStatusMonitor> connectionMonitor_;
  std::string threadName_;
  apache::thrift::concurrency::Monitor connectMonitor_;
  apache::thrift::concurrency::Monitor disconnectMonitor_;
  bool stop_;
};

#endif // RTNET_PLOT_CONNECTIONREQUIREDRUNNABLE_H
