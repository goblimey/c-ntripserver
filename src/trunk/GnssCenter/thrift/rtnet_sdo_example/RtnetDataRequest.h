#ifndef RTNET_SDO_EXAMPLE_RTNETDATAREQUEST_H
#define RTNET_SDO_EXAMPLE_RTNETDATAREQUEST_H

#include "ConnectionRequiredRunnable.h"
#include "common/thrift/RtnetData.h"

#include <protocol/TProtocol.h>

class RtnetDataRequest : public ConnectionRequiredRunnable
{
public:
  RtnetDataRequest(boost::shared_ptr<apache::thrift::protocol::TProtocol>& protocol,
                   boost::shared_ptr<ConnectionStatusMonitor>& connectionMonitor);
  ~RtnetDataRequest() {}

  void run();
private:
  boost::shared_ptr<com::gpssolutions::rtnet::RtnetDataClient> client_;
};

#endif // RTNET_SDO_EXAMPLE_RTNETDATAREQUEST_H
