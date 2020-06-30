#ifndef RTNET_SDO_EXAMPLE_CLIENT_H
#define RTNET_SDO_EXAMPLE_CLIENT_H

#include <string>

#include <transport/TTransport.h>
#include <protocol/TProtocol.h>
#include <concurrency/TimerManager.h>

class RtnetDataHandler;
class ConnectionRequiredRunnable;
class ConnectionStatusMonitor;

class Client
{
public:
  Client(const std::string& server, int16_t port,
         boost::shared_ptr<apache::thrift::concurrency::TimerManager>& timeMgr,
         boost::shared_ptr<RtnetDataHandler> dataHandler);
  ~Client();
  void stop();
private:
  boost::shared_ptr<ConnectionStatusMonitor> connectionMonitor_;
  
  boost::shared_ptr<apache::thrift::transport::TTransport> transport_;
  boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol_;
  boost::shared_ptr<ConnectionRequiredRunnable> sender_;
  boost::shared_ptr<apache::thrift::concurrency::Thread> senderThread_;
  boost::shared_ptr<ConnectionRequiredRunnable> receiver_;
  boost::shared_ptr<apache::thrift::concurrency::Thread> receiverThread_;
};

#endif // RTNET_SDO_EXAMPLE_CLIENT_H
