#ifndef RTNET_SDO_EXAMPLE_RTNETDATALISTENER_H
#define RTNET_SDO_EXAMPLE_RTNETDATALISTENER_H

#include "ConnectionRequiredRunnable.h"
#include "RtnetDataHandler.h"
#include "common/thrift/RtnetData.h"

#include <protocol/TProtocol.h>

class RtnetDataListener : public ConnectionRequiredRunnable
{
public:
  RtnetDataListener(boost::shared_ptr<apache::thrift::protocol::TProtocol>& protocol,
                    boost::shared_ptr<ConnectionStatusMonitor>& connectionMonitor,
                    boost::shared_ptr<com::gpssolutions::rtnet::RtnetDataIf> dataHandler);
  ~RtnetDataListener();

  void run();
  void stop();
private:
  boost::shared_ptr<com::gpssolutions::rtnet::RtnetDataIf> dataHandler_;
  boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol_;
  boost::shared_ptr<com::gpssolutions::rtnet::RtnetDataProcessor> processor_;
};

#endif // RTNET_SDO_EXAMPLE_RTNETDATALISTENER_H
