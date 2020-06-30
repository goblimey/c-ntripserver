#ifndef RTNET_SDO_EXAMPLE_RTNETDATAHANDLER_H
#define RTNET_SDO_EXAMPLE_RTNETDATAHANDLER_H

#include "common/thrift/RtnetData.h"

class RtnetDataHandler : public com::gpssolutions::rtnet::RtnetDataIf
{
public:
  RtnetDataHandler();
  ~RtnetDataHandler();
  void startDataStream();
  void registerRtnet(const  ::com::gpssolutions::rtnet::RtnetInformation& info);
  void handleZDAmb(const std::vector< ::com::gpssolutions::rtnet::ZDAmb> & ambList);
  void handleDDAmbresBaselines(const std::vector< ::com::gpssolutions::rtnet::DDAmbresBaseline>& ambList);
  void handleSatelliteXYZ(const std::vector< ::com::gpssolutions::rtnet::SatelliteXYZ>& svXYZList);
  void handleStationInfo(const std::vector< ::com::gpssolutions::rtnet::StationInfo> & stationList);
  void handleStationAuxInfo(const std::vector< ::com::gpssolutions::rtnet::StationAuxInfo> & stationAuxList);
  void handleDGPSCorr(const std::vector< ::com::gpssolutions::rtnet::DGPSCorr> & dgpsList);
  void handleSatelliteClock(const std::vector< ::com::gpssolutions::rtnet::SatelliteClock> & svList);
  void handleEpochResults(const  ::com::gpssolutions::rtnet::RtnetEpoch& epoch);
};

#endif // RTNET_SDO_EXAMPLE_RTNETDATAHANDLER_H
