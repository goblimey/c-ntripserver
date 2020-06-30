#ifndef GnssCenter_WORLDPLOT_H
#define GnssCenter_WORLDPLOT_H

#include <qwt_plot.h>

class QwtPlotZoomer;
class QwtPlotMarker;

namespace GnssCenter {

class t_worldPlot : public QwtPlot {
 Q_OBJECT
 public:
  t_worldPlot();
  ~t_worldPlot();
  
  class t_point {
   public:
    t_point(const QColor& color, const QString& name, double latDeg, double lonDeg) {
      _color  = color;
      _name   = name;
      _latDeg = latDeg;
      _lonDeg = lonDeg;
    }
    ~t_point() {}
    QColor  _color;
    QString _name;
    double  _latDeg;
    double  _lonDeg;
  };

 public slots:
  void slotNewPoints(const QList<t_point*>& points);
  void slotPrint();

 private:
  QwtPlotZoomer*        _zoomer;
  QList<QwtPlotMarker*> _markers;
  
};

} // namespace GnssCenter

#endif
