
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "thriftclient.h"
#include "monitor.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace com::gpssolutions::rtnet;
using namespace std;
using namespace boost;

using namespace GnssCenter;

// Auxiliary Function
//////////////////////////////////////////////////////////////////////////////
string id2prn(ConstellationType::type constellation, int ID) {
  char ch;
  switch(constellation) {
    case (ConstellationType::GPS):     ch =  'G'; break;
    case (ConstellationType::GLONASS): ch =  'R'; break;
    case (ConstellationType::SBAS):    ch =  'S'; break;
    case (ConstellationType::GALILEO): ch =  'E'; break;
    case (ConstellationType::QZSS):    ch =  'J'; break;
    case (ConstellationType::COMPASS): ch =  'C'; break;
    default: return "";
  }
  char prn[3];
  sprintf(prn, "%c%2.2d", ch, ID);
  return string(prn);
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_thriftClient::t_thriftClient(t_monitor* parent, const QString& host, int port) {
  _stop   = false;
  _parent = parent;
  _host   = host.toAscii().data();
  _port   = port;
}

// Destructor
//////////////////////////////////////////////////////////////////////////////
t_thriftClient::~t_thriftClient() {
}

// Run (virtual)
//////////////////////////////////////////////////////////////////////////////
void t_thriftClient::run() {

  shared_ptr<TSocket>     socket(new TSocket(_host, _port));
  shared_ptr<TTransport>  transport(new TBufferedTransport(socket)); 
  shared_ptr<TProtocol>   protocol(new TBinaryProtocol(transport));
  shared_ptr<RtnetDataIf> dataHandler(new t_thriftHandler(_parent));
  shared_ptr<TProcessor>  processor(new RtnetDataProcessor(dataHandler));

  try {
    transport->open();
    while (true) {
      {
        QMutexLocker locker(&_mutex);
        if (_stop) {
          break;
        }
      }
      if (processor->process(protocol,protocol,0) == 0) {
        break;
      }
    }
    transport->close();
  } 
  catch (TException& e) {
    emit message(e.what());
  } 
  catch (...) {
    emit message("Unknown exception");
  }
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_thriftHandler::t_thriftHandler(t_monitor* parent) {
  _parent = parent;
}

// Destructor
//////////////////////////////////////////////////////////////////////////////
t_thriftHandler::~t_thriftHandler() {
}

// Handle Satellite Positions
//////////////////////////////////////////////////////////////////////////////
void t_thriftHandler::
handleSatelliteXYZ(const vector<SatelliteXYZ>& svXYZList) {
  vector<t_thriftSatellite*>* satellites = new vector<t_thriftSatellite*>;
  for (unsigned ii = 0; ii < svXYZList.size(); ii++) {
    const SatelliteXYZ& sat = svXYZList[ii];
    t_thriftSatellite* satellite = new t_thriftSatellite;
    satellite->_prn = id2prn(sat.constellation, sat.ID);
    satellite->_x   = sat.xyz.x;
    satellite->_y   = sat.xyz.y;
    satellite->_z   = sat.xyz.z;
    satellites->push_back(satellite);
  }
  _parent->putThriftSatellites(satellites);
}

// Handle Station Info
//////////////////////////////////////////////////////////////////////////////
void t_thriftHandler::
handleStationInfo(const vector<StationInfo>& stationList) {
  for (unsigned ii = 0; ii < stationList.size(); ii++) {
    const StationInfo& staInfo = stationList[ii];
    _stationCrd[staInfo.ID]._x = staInfo.xyz.x;
    _stationCrd[staInfo.ID]._y = staInfo.xyz.y;
    _stationCrd[staInfo.ID]._z = staInfo.xyz.z;
  }
}

// Handle Epoch Results
//////////////////////////////////////////////////////////////////////////////
void t_thriftHandler::
handleEpochResults(const RtnetEpoch& epoch) {
  vector<t_thriftResult*>* results = new vector<t_thriftResult*>;
  for (unsigned ii = 0; ii < epoch.stationResultList.size(); ii++) {
    const StationResults& staRes = epoch.stationResultList[ii];

    t_thriftResult* res = new t_thriftResult;

    res->_name = staRes.stationName;
    res->_nGPS = staRes.nsv_gps_used;
    res->_nGLO = staRes.nsv_glonass_used;
    if (_stationCrd.find(staRes.stationName) != _stationCrd.end()) {
      res->_x = _stationCrd[staRes.stationName]._x;
      res->_y = _stationCrd[staRes.stationName]._y;
      res->_z = _stationCrd[staRes.stationName]._z;
    }

    for (unsigned ic = 0; ic < staRes.residList.size(); ic++) {
      const ResidualInfo& info = staRes.residList[ic];
      for (unsigned is = 0; is < info.svResid.size(); is++) {
        const SvResidualInfo& sat = info.svResid[is];
        string prn = id2prn(sat.constellation, sat.ID);
        res->_prns.insert(prn);
      }
    }

    results->push_back(res);
  }
  _parent->putThriftResults(results);
}
