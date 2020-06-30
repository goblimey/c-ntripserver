#include "ConnectionRequiredRunnable.h"

using namespace apache::thrift::concurrency;
using std::string;

ConnectionRequiredRunnable::ConnectionRequiredRunnable(boost::shared_ptr<ConnectionStatusMonitor>& connectionMonitor, const string& name)
                          : connectionMonitor_(connectionMonitor),
                            threadName_(name),
                            connectMonitor_(),
                            disconnectMonitor_(),
                            stop_(false)
{
}

void
ConnectionRequiredRunnable::connectionLost()
{
}

void
ConnectionRequiredRunnable::connectionEstablished()
{
  connectMonitor_.notifyAll();
}

void
ConnectionRequiredRunnable::disconnected()
{
  connectionMonitor_->disconnected(this);
  disconnectMonitor_.notifyAll();
}

void
ConnectionRequiredRunnable::connectWait()
{
  if (connectionMonitor_->connected()) return;
  connectMonitor_.waitForever();
}

void
ConnectionRequiredRunnable::disconnectWait()
{
  if (!connectionMonitor_->connected()) return;
  disconnectMonitor_.waitForever();
}

void
ConnectionRequiredRunnable::stop()
{
  stop_ = true;
  connectMonitor_.notifyAll();
  disconnectMonitor_.notifyAll();
}

