include "rtnet_data.thrift"

namespace cpp com.gpssolutions.rtnet_js
namespace java com.gpssolutions.rtnet_js

struct UpdateStatus {
  1: byte stationInfoUpdated,
  2: byte stationAuxInfoUpdated,
  3: byte rtnetInfoUpdated,
  4: i32  lastEpoch
}

exception InvalidArgument {
  1: string why
}

service RtnetDataJs {
  list<rtnet_data.RtnetInformation> getJobList(),
  list<rtnet_data.ZDAmb> lastZDAmb(1: string jobName) throws (1:InvalidArgument err),
  list<rtnet_data.DDAmbresBaseline> lastDDAmbresBaselines(1: string jobName) throws (1:InvalidArgument err),
  list<rtnet_data.SatelliteXYZ> getSatelliteXYZ(1: string jobName) throws (1:InvalidArgument err),
  list<rtnet_data.StationInfo> getStationInfo(1: string jobName) throws (1:InvalidArgument err),
  list<rtnet_data.StationAuxInfo> getStationAuxInfo(1: string jobName) throws (1:InvalidArgument err),
  UpdateStatus getUpdateStatus(1: string jobName) throws (1:InvalidArgument err)
}

