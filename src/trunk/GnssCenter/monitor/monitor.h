#ifndef GnssCenter_MONITOR_H
#define GnssCenter_MONITOR_H

#include <QtGui>
#include <QWhatsThis>
#include "plugininterface.h"
#include "const.h"

namespace GnssCenter {
  class t_worldPlot;
  class t_thriftClient;
  class t_thriftResult;
  class t_thriftSatellite;
}

namespace GnssCenter {

class t_monitor : public QMainWindow {
 Q_OBJECT
 public:
  t_monitor();
  ~t_monitor();

  void putThriftResults(std::vector<t_thriftResult*>* results);
  void putThriftSatellites(std::vector<t_thriftSatellite*>* satellites);

 private slots:
  void slotConfig();
  void slotStartThrift();
  void slotStopThrift();
  void slotThriftFinished();
  void slotPlot();
  void slotMessage(QByteArray msg);

 private:
  void readSettings();
  void setTitle();
  void enableActions();
  void plotResults();
  void plotSatellites();
  QMutex                           _mutex;
  QTabWidget*                      _tabWidget;
  QAction*                         _actConfig;
  QAction*                         _actStartThrift;
  QAction*                         _actStopThrift;
  QString                          _host;
  QString                          _port;
  t_thriftClient*                  _thriftClient;
  t_worldPlot*                     _plotStations;
  t_worldPlot*                     _plotSatellites;
  std::vector<t_thriftResult*>*    _results;
  std::vector<t_thriftSatellite*>* _satellites;
};

class t_monitorFactory : public QObject, public t_pluginFactoryInterface {
 Q_OBJECT
 Q_INTERFACES(GnssCenter::t_pluginFactoryInterface)
 public:
  virtual QWidget* create() {return new t_monitor();} 
  virtual QString getName() const {return pluginName;}
};

} // namespace GnssCenter

#endif
