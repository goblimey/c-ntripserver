#include "RtnetDataHandler.h"

#include <cstdio>

extern "C" {
#include <time.h>
}

using namespace com::gpssolutions::rtnet;

RtnetDataHandler::RtnetDataHandler()
{
}

RtnetDataHandler::~RtnetDataHandler() {
}

void
RtnetDataHandler::startDataStream() {
}

void
RtnetDataHandler::registerRtnet(const RtnetInformation& info) {
  /*registered_ = true;
  jobName_ = info.jobName;
  printf("registerRtnet(): %s\n",jobName_.c_str());
  OneRtnetJobInfo* pInfo = jobStatus_->oneJobInfo(jobName_);
  {
    Guard lock(pInfo->mutex());
    UpdateStatus& status = pInfo->updateStatus();
    status.stationInfoUpdated = 0;
    status.stationAuxInfoUpdated = 0;
    status.rtnetInfoUpdated = 0;
  }*/
}

void
RtnetDataHandler::handleZDAmb(const std::vector<ZDAmb> & ambList) {
  printf("Decoded ZD ambiguity record\n");
  if (ambList.empty()) {
    printf("No ZD ambiguities present\n");
    return;
  }
  std::vector<ZDAmb>::const_iterator zdIt;
  std::string staName = ambList[0].stationID;
  printf ("Received %zd zero-difference ambiguities for station %s\n",ambList.size(),staName.c_str());
}

void
RtnetDataHandler::handleDDAmbresBaselines(const std::vector<DDAmbresBaseline>& ambList) {
  printf("Decoded double difference ambiguities record (%zd DD ambiguities)\n",ambList.size());
}

void
RtnetDataHandler::handleSatelliteXYZ(const std::vector<SatelliteXYZ>& svXYZList) {
  printf("Decoded satellite XYZ record (%zd satellites)\n",svXYZList.size());
}

void
RtnetDataHandler::handleStationInfo(const std::vector<StationInfo> & stationList) {
  printf("Decoded station information record for %zd stations\n",stationList.size());
}

void
RtnetDataHandler::handleStationAuxInfo(const std::vector<StationAuxInfo> & stationAuxList) {
  printf("Decoded auxilary station record for %d stations\n",stationAuxList.size());
}

void
RtnetDataHandler::handleDGPSCorr(const std::vector<DGPSCorr> & dgpsList) {
  printf("Decoded DGPS correction (%zd corrections)\n",dgpsList.size());
}

void
RtnetDataHandler::handleSatelliteClock(const std::vector<SatelliteClock> & svList) {
  printf("Decoded RTNet satellite clock structure (%zd satellite clocks)\n",svList.size());
}

void
RtnetDataHandler::handleEpochResults(const RtnetEpoch& epoch) {
  long mjlDay = epoch.mjlDay;
  if (mjlDay < 0) mjlDay += 65536l;
  long unixSec = (mjlDay-40587L)*24L*60L*60L;
  unixSec += epoch.msecDay / 1000;
  char timeStrBuffer[18];
  struct tm tmEpoch;
  time_t tEpoch = static_cast<time_t>(unixSec);
  gmtime_r(&tEpoch,&tmEpoch);
  if (strftime(timeStrBuffer,17,"%H:%M:%S",&tmEpoch) != 0)
    printf("Decoded RTNet epoch structure for epoch %s.%03d\n",timeStrBuffer,epoch.msecDay % 1000);
  else
    printf("Decoded RTNet epoch structure for epoch %ld %.3f\n",epoch.mjlDay,epoch.msecDay / 1000.0);
}

