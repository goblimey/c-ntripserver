include "rtnet_data.thrift"

namespace cpp com.gpssolutions.rtnet
namespace java com.gpssolutions.rtnet
namespace perl gpss

service RtnetData {
  // used for plots
  oneway void startDataStream(),
  oneway void registerRtnet(1: rtnet_data.RtnetInformation info),
  oneway void handleZDAmb(1: list<rtnet_data.ZDAmb> ambList),
  oneway void handleDDAmbresBaselines(1: list<rtnet_data.DDAmbresBaseline> ambList),
  oneway void handleSatelliteXYZ(1: list<rtnet_data.SatelliteXYZ> svXYZList),
  oneway void handleStationInfo(1: list<rtnet_data.StationInfo> stationList),
  oneway void handleStationAuxInfo(1: list<rtnet_data.StationAuxInfo> stationAuxList),

  // used for everything else
  oneway void handleDGPSCorr(1: list<rtnet_data.DGPSCorr> dgpsList),
  oneway void handleSatelliteClock(1: list<rtnet_data.SatelliteClock> svList),
  oneway void handleEpochResults(1: rtnet_data.RtnetEpoch epoch)
}

