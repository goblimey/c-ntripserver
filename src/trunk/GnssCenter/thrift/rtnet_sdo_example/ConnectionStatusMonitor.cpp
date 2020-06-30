#include "ConnectionStatusMonitor.h"
#include "ConnectionRequiredRunnable.h"

#include <cstdio>

using namespace apache::thrift::transport;
using namespace apache::thrift::concurrency;

ConnectionStatusMonitor::ConnectionStatusMonitor(boost::shared_ptr<TTransport>& transport,
                                                 boost::shared_ptr<TimerManager>& timeMgr)
                       : listeners_(),
                         transport_(transport),
                         timeMgr_(timeMgr),
                         connected_(0)
{
}

ConnectionStatusMonitor::~ConnectionStatusMonitor() {
}

void
ConnectionStatusMonitor::addListener(boost::shared_ptr<ConnectionRequiredRunnable> listener) {
  listeners_.insert(listener);
}

bool
ConnectionStatusMonitor::connected() {
  return (((long)connected_ == 0) ? false : true);
  //return connected_.load(); // if using C++0x
}

void
ConnectionStatusMonitor::disconnected(const ConnectionRequiredRunnable* noticer) {
  bool expected=true;
  if ((long)connected_ != 0) {
  //if (connected_.compare_exchange_strong(expected,false)) // if using C++0x
    --connected_;
    std::set<boost::shared_ptr<ConnectionRequiredRunnable> >::iterator listener;
    for (listener=listeners_.begin(); listener != listeners_.end(); ++listener) {
    //for (auto listener=listeners_.begin(); listener != listeners_.end(); ++listener) // if using C++0x
      // The thread running the noticer is our current execution thread. If we
      // notify it it'll block and we'll be deadlocked. Since it noticed the
      // disconnect it is responsible for initiating its own wait state.
      if (listener->get() == noticer) continue;
      (*listener)->connectionLost();
    }
  }
  // Try to reconnect in five seconds
  timeMgr_->add(boost::shared_ptr<Runnable>(new Reconnect(this)), 5 * 1000);
}

void
ConnectionStatusMonitor::tryOpen() {
  if ((long)connected_ != 0)
  //if (connected_.load()) // if using C++0x
    return;
  
  // make sure it's closed
  transport_->close();
  try {
    transport_->open();
    ++connected_;
    //connected_.store(true); // if using C++0x
    std::set<boost::shared_ptr<ConnectionRequiredRunnable> >::iterator listener;
    for (listener=listeners_.begin(); listener != listeners_.end(); ++listener) {
    //for (auto listener = listeners_.begin(); listener != listeners_.end(); ++listener) { // if using C++0x
      (*listener)->connectionEstablished();
    }
    return;
  } catch (TTransportException& e) {
    fprintf(stderr,"opening transport failed\n");
  }
  // Try to reconnect in five seconds
  timeMgr_->add(boost::shared_ptr<Runnable>(new Reconnect(this)), 5 * 1000);
}

void
ConnectionStatusMonitor::stop() {
  std::set<boost::shared_ptr<ConnectionRequiredRunnable> >::iterator listener;
  for (listener=listeners_.begin(); listener != listeners_.end(); ++listener) {
  //for (auto listener = listeners_.begin(); listener != listeners_.end(); ++listener) // if using C++0x
    (*listener)->stop();
  }
  listeners_.clear();
}

