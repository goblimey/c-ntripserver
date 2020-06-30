#include "Client.h"
#include "ConnectionStatusMonitor.h"
#include "RtnetDataHandler.h"
#include "RtnetDataListener.h"
#include "RtnetDataRequest.h"

#include <transport/TSocket.h>
#include <protocol/TBinaryProtocol.h>

#include <cstdio>

using boost::shared_ptr;

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::concurrency;

Client::Client(const std::string& server,
               int16_t port, boost::shared_ptr<TimerManager>& timeMgr,
               boost::shared_ptr<RtnetDataHandler> dataHandler)
      : connectionMonitor_(),
        transport_(),
        protocol_(),
        sender_(),
        senderThread_(),
        receiver_(),
        receiverThread_()
{
  transport_ = boost::shared_ptr<TTransport>(new TSocket(server, port));
  protocol_ = boost::shared_ptr<TProtocol>(new TBinaryProtocol(transport_));
 
  connectionMonitor_ = boost::shared_ptr<ConnectionStatusMonitor>(new ConnectionStatusMonitor(transport_,timeMgr));
 
  sender_ = boost::shared_ptr<ConnectionRequiredRunnable>(new RtnetDataRequest(protocol_, connectionMonitor_));
  connectionMonitor_->addListener(sender_);
  senderThread_ = timeMgr->threadFactory()->newThread(sender_);
  senderThread_->start();
  receiver_ = boost::shared_ptr<ConnectionRequiredRunnable>(new RtnetDataListener(protocol_, connectionMonitor_, dataHandler));
  connectionMonitor_->addListener(receiver_);
  receiverThread_ = timeMgr->threadFactory()->newThread(receiver_);
  receiverThread_->start();
 
  connectionMonitor_->tryOpen();
}

Client::~Client() {
}

void
Client::stop() {
  connectionMonitor_->stop();
}

