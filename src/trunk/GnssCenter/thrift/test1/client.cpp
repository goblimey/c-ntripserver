#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/myService.h"

using namespace std;
using namespace boost;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

int main(int argc, char** argv) {
  int port = 9090;
  shared_ptr<TSocket>    socket(new TSocket("localhost", port));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol>  protocol(new TBinaryProtocol(transport));
  myServiceClient        client(protocol);

  try {
    transport->open();

    while (true) {
      string answ;
      client.answer(answ, "How are you?");
      cout << "Server answers: " << answ << endl;
      sleep(1);
    }

    transport->close();
  } catch (TException& exc) {
    cout << "Exception: " << exc.what() << endl;
  }

  return 0;
}
