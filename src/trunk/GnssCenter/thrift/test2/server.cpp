#include <iostream>
#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/Thread.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/concurrency/Mutex.h>

#include "gen-cpp/myService.h"

using namespace std;
using namespace boost;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

class t_connection {
 public:
  shared_ptr<myServiceClient> _client;
  shared_ptr<TProcessor>      _processor;
  shared_ptr<TProtocol>       _protocolInp;
  shared_ptr<TProtocol>       _protocolOut;
  shared_ptr<TTransport>      _transport;
};

Mutex MUTEX;
shared_ptr<t_connection> CONNECTION;

class myProcessorFactory : public TProcessorFactory {
 public:
  myProcessorFactory() {};
  shared_ptr<TProcessor>   getProcessor(const TConnectionInfo& info) {

    Guard m(MUTEX);

    shared_ptr<myServiceClient> client(new myServiceClient(info.output));
    shared_ptr<TProcessor>      processor(new myServiceProcessor(client));

    cout << "new connection " << endl;

    CONNECTION.reset(new t_connection);   
    CONNECTION->_client      = client;
    CONNECTION->_processor   = processor;
    CONNECTION->_protocolInp = info.input;
    CONNECTION->_protocolOut = info.output;
    CONNECTION->_transport   = info.transport;
   
    return processor;
  }
};

class t_serverThread : public apache::thrift::concurrency::Runnable {
 public:
  t_serverThread() {}
  ~t_serverThread() {}
  void run() {
    int port = 9090;
    shared_ptr<TServerSocket>      serverTransport(new TServerSocket(port));
    shared_ptr<myProcessorFactory> processorFactory(new myProcessorFactory());
    shared_ptr<TTransportFactory>  transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory>   protocolFactory(new TBinaryProtocolFactory());

    TThreadedServer server(processorFactory, serverTransport,
                           transportFactory, protocolFactory);
    server.serve();
  }   
};

int main(int argc, char **argv) {

  shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory);

  shared_ptr<t_serverThread> serverThread(new t_serverThread);  

  shared_ptr<Thread> thread = threadFactory->newThread(serverThread);
  thread->start();

  try {
    while (true) {
      {
        Guard m(MUTEX);
        if (CONNECTION) {
          CONNECTION->_client->answer("How are you?");
        }
      }
      sleep(1);
    }
  } catch (TException& exc) {
    cout << "Exception: " << exc.what() << endl;
  }

  return 0;
}

