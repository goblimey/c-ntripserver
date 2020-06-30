#ifndef THRIFTCLIENT_H
#define THRIFTCLIENT_H

#include <string>
#include <map>
#include <set>
#include <QString>
#include <QThread>
#include <QMutex>

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "gen-cpp/RtnetData.h"

using namespace com::gpssolutions::rtnet;

namespace GnssCenter {
  class t_monitor;
}

namespace GnssCenter {

class t_thriftResult {
 public:
  t_thriftResult() {
    _nGPS = 0;
    _nGLO = 0;
    _x    = 0.0;
    _y    = 0.0;
    _z    = 0.0;
  }
  ~t_thriftResult() {}
  std::string           _name;
  int                   _nGPS;  
  int                   _nGLO;
  double                _x;
  double                _y;
  double                _z;
  std::set<std::string> _prns;
};

class t_thriftSatellite {
 public:
  std::string _prn;
  double      _x;
  double      _y;
  double      _z;
};

class t_thriftHandler : public RtnetDataIf {
 public:
  t_thriftHandler(t_monitor* parent);
  ~t_thriftHandler();
  void startDataStream() {}
  void registerRtnet(const RtnetInformation&) {}
  void handleZDAmb(const std::vector<ZDAmb>&) {}
  void handleDDAmbresBaselines(const std::vector<DDAmbresBaseline>&) {}
  void handleSatelliteXYZ(const std::vector<SatelliteXYZ>& svXYZList);
  void handleStationInfo(const std::vector<StationInfo>& stationList);
  void handleStationAuxInfo(const std::vector<StationAuxInfo>&) {}
  void handleDGPSCorr(const std::vector<DGPSCorr>&) {}
  void handleSatelliteClock(const std::vector<SatelliteClock>&) {}
  void handleEpochResults(const RtnetEpoch& epoch);
 private:
  class t_stationCrd {
   public:
    double _x;
    double _y;
    double _z;
  };
  t_monitor*              _parent;
  std::map<std::string, t_stationCrd> _stationCrd;
};

class t_thriftClient : public QThread {
 Q_OBJECT
 public:
  t_thriftClient(t_monitor* parent, const QString& host, int port);
  ~t_thriftClient();
  virtual void run();
  void stop() {
    QMutexLocker locker(&_mutex);
    _stop = true;
  }
 signals:
  void message(QByteArray msg);
 private:
  QMutex      _mutex;
  std::string _host;
  int         _port;
  t_monitor*  _parent;
  bool        _stop;
};

} // namespace GnssCenter

#endif
