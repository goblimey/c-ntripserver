
/* -------------------------------------------------------------------------
 * RTNet Monitor
 * -------------------------------------------------------------------------
 *
 * Class:      t_monitor
 *
 * Purpose:    Real-Time Monitoring of RTNet
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <QtSvg>

#include <qwt_symbol.h>
#include <qwt_plot.h>
#include <qwt_plot_svgitem.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_renderer.h>

#include "monitor.h"
#include "dlgconf.h"
#include "utils.h"
#include "worldplot.h"
#include "thriftclient.h"
#include "settings.h"

using namespace std;
using namespace GnssCenter;

Q_EXPORT_PLUGIN2(gnsscenter_monitor, t_monitorFactory)

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_monitor::t_monitor() : QMainWindow() {

  _tabWidget = new QTabWidget();
  setCentralWidget(_tabWidget);

  // World Plots
  // -----------
  _plotStations = new t_worldPlot();
  _tabWidget->addTab(_plotStations, "Stations");

  _plotSatellites = new t_worldPlot();
  _tabWidget->addTab(_plotSatellites, "Satellites");

  // Tool Bar
  // --------
  QToolBar* toolBar = new QToolBar("t_monitor_ToolBar");
  addToolBar(Qt::BottomToolBarArea, toolBar);

  _actConfig = new QAction("Config", 0);
  toolBar->addAction(_actConfig);
  connect(_actConfig, SIGNAL(triggered()), this, SLOT(slotConfig()));

  _actStartThrift = new QAction("Start", 0);
  toolBar->addAction(_actStartThrift);
  connect(_actStartThrift, SIGNAL(triggered()), this, SLOT(slotStartThrift()));

  _actStopThrift = new QAction("Stop", 0);
  toolBar->addAction(_actStopThrift);
  connect(_actStopThrift, SIGNAL(triggered()), this, SLOT(slotStopThrift()));

  // Thrift Client;
  // --------------
  _thriftClient = 0;
  _results      = 0;
  _satellites   = 0;

  // Read Settings, Set Title, Enable/Disable Actions
  // ------------------------------------------------
  readSettings();
  setTitle();
  enableActions();
}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_monitor::~t_monitor() {
  slotStopThrift();
  if (_results) {
    while (!_results->empty()) {
      delete _results->back();
      _results->pop_back();
    }
    delete _results;
  }
  if (_satellites) {
    while (!_satellites->empty()) {
      delete _satellites->back();
      _satellites->pop_back();
    }
    delete _satellites;
  }
}

// Read Settings
/////////////////////////////////////////////////////////////////////////////
void t_monitor::readSettings() {
  t_settings settings(pluginName);
  _host = settings.value("host").toString().trimmed();
  if (_host.isEmpty()) {
    _host = "localhost";
  }
  _port = settings.value("port").toString();
}

// Set title
/////////////////////////////////////////////////////////////////////////////
void t_monitor::setTitle() {
  if (_port.isEmpty()) {
    setWindowTitle(QString(pluginName));
  }
  else {
    setWindowTitle(QString(pluginName) + "   " + _host + ':' + _port);
  }
}

// Enable/Disable Actions
/////////////////////////////////////////////////////////////////////////////
void t_monitor::enableActions() {
  if      (_port.isEmpty()) {
    _actConfig->setEnabled(true);
    _actStartThrift->setEnabled(false);
    _actStopThrift->setEnabled(false);
  }
  else if (_thriftClient) {
    _actConfig->setEnabled(false);
    _actStartThrift->setEnabled(false);
    _actStopThrift->setEnabled(true);
  }
  else {
    _actConfig->setEnabled(true);
    _actStartThrift->setEnabled(true);
    _actStopThrift->setEnabled(false);
  }
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotConfig() {
  t_dlgConf dlg(this);
  dlg.exec();
  readSettings();
  setTitle();
  enableActions();
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotMessage(QByteArray msg) {
  QMessageBox::information(this, "Message", msg);
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotStartThrift() {
  if (!_thriftClient) {
    _thriftClient = new t_thriftClient(this, _host, _port.toInt());
    connect(_thriftClient, SIGNAL(finished()), this, SLOT(slotThriftFinished()));
    connect(_thriftClient, SIGNAL(message(QByteArray)), this, SLOT(slotMessage(QByteArray)));
    _thriftClient->start();
    slotPlot();
  }
  enableActions();
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotStopThrift() {
  if (_thriftClient) {
    _thriftClient->stop();
    _thriftClient = 0;
  }
  enableActions();
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotThriftFinished() {
  sender()->deleteLater();
  _thriftClient = 0;
  enableActions();
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::putThriftResults(std::vector<t_thriftResult*>* results) {
  QMutexLocker locker(&_mutex);
  if (_results) {
    while (!_results->empty()) {
      delete _results->back();
      _results->pop_back();
    }
    delete _results;
  }
  _results = results;
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::putThriftSatellites(std::vector<t_thriftSatellite*>* satellites) {
  QMutexLocker locker(&_mutex);
  if (_satellites) {
    while (!_satellites->empty()) {
      delete _satellites->back();
      _satellites->pop_back();
    }
    delete _satellites;
  }
  _satellites = satellites;
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::slotPlot() {
  QMutexLocker locker(&_mutex);
  plotResults();
  plotSatellites();
  if (_thriftClient) {
    QTimer::singleShot(1000, this, SLOT(slotPlot()));
  }
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::plotResults() {
  if (_results) {
    QList<t_worldPlot::t_point*> points;
    for (unsigned ii = 0; ii < _results->size(); ii++) {
      const t_thriftResult* result = _results->at(ii);

      double xyz[3]; 
      xyz[0] = result->_x;
      xyz[1] = result->_y;
      xyz[2] = result->_z;
    
      double ell[3];

      if (t_utils::xyz2ell(xyz, ell) == t_CST::success) {
        double latDeg = ell[0] * 180.0 / M_PI;
        double lonDeg = ell[1] * 180.0 / M_PI;
        QString str = QString(result->_name.c_str()) + 
                      QString().sprintf("\n%d/%d", result->_nGPS, result->_nGLO);
        QColor color = result->_nGPS >= 4 ? Qt::black : Qt::red;
        t_worldPlot::t_point* point  = new t_worldPlot::t_point(color, str, latDeg, lonDeg);
        points.append(point);
      }
    }
    _plotStations->slotNewPoints(points);

    QListIterator<t_worldPlot::t_point*> it(points);
    while (it.hasNext()) {
      delete it.next();
    }
  }
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_monitor::plotSatellites() {
  if (_satellites) {
    QList<t_worldPlot::t_point*> points;
    for (unsigned ii = 0; ii < _satellites->size(); ii++) {
      const t_thriftSatellite* sat = _satellites->at(ii);

      double xyz[3]; 
      xyz[0] = sat->_x;
      xyz[1] = sat->_y;
      xyz[2] = sat->_z;
    
      double ell[3];

      if (t_utils::xyz2ell(xyz, ell) == t_CST::success) {
        double latDeg = ell[0] * 180.0 / M_PI;
        double lonDeg = ell[1] * 180.0 / M_PI;

        QString str   = sat->_prn.c_str();

        QColor color;
        if      (str[0] == 'G') {
          color = Qt::darkBlue;
        }
        else if (str[0] == 'R') {
          color = Qt::darkGreen;
        }
        else {
          color = Qt::black;
        }

        if (_results) {
          int numSta = 0;
          for (unsigned jj = 0; jj < _results->size(); jj++) {
            const t_thriftResult* result = _results->at(jj);
            if (result->_prns.find(sat->_prn) != result->_prns.end()) {
              numSta += 1;
            }
          }
          str += QString().sprintf("\n%d", numSta);
          if (numSta < 4) {
            color = Qt::red;
          }
        }

        t_worldPlot::t_point* point  = new t_worldPlot::t_point(color, str, latDeg, lonDeg);
        points.append(point);
      }
    }
    _plotSatellites->slotNewPoints(points);

    QListIterator<t_worldPlot::t_point*> it(points);
    while (it.hasNext()) {
      delete it.next();
    }
  }
}
