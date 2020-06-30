
#ifndef POLARPLOT_H
#define POLARPLOT_H

#include <qwt_polar_plot.h>
#include <qwt_polar_curve.h>

//
//////////////////////////////////////////////////////////////////////////////
class t_polarCurve : public QwtPolarCurve {
 public:
  t_polarCurve() {}
  virtual ~t_polarCurve() {}
  void setScaleInterval(const QwtInterval& scaleInterval) {
    _scaleInterval = scaleInterval;
  }
 protected:
  virtual void drawSymbols(QPainter* painter, const QwtSymbol& symbol, 
                           const QwtScaleMap& azimuthMap, 
                           const QwtScaleMap& radialMap, 
                           const QPointF& pole, int from, int to) const;
 private:
  QwtInterval _scaleInterval;
};

//
//////////////////////////////////////////////////////////////////////////////
class t_polarPoint {
 public:
  t_polarPoint(double az, double zen, double value) : 
               _az(az), _zen(zen), _value(value) {}
  double _az;
  double _zen;
  double _value;
};

//
//////////////////////////////////////////////////////////////////////////////
class t_polarData: public QwtSeriesData<QwtPointPolar> {
 public:
  t_polarData(QVector<t_polarPoint*>* data) {
    _data = data;
    _size = data->size();
  }
  ~t_polarData() {
    for (int ii = 0; ii < _data->size(); ii++) {
      delete _data->at(ii);
    }
    delete _data;
  }
  virtual QwtPointPolar sample(size_t ii) const {
    const t_polarPoint* point = _data->at(ii);
    QwtPointPolar qp(point->_az, point->_zen);  qp._value = point->_value;
    return qp;
  }
  virtual size_t size() const {return _size;}
  virtual QRectF boundingRect() const {return d_boundingRect;}
 protected:
  size_t _size;
 private:
  QVector<t_polarPoint*>* _data;
};

//
//////////////////////////////////////////////////////////////////////////////
class t_polarPlot: public QwtPolarPlot {
 Q_OBJECT

 public:
  t_polarPlot(const QwtText& title, const QwtInterval& scaleInterval,
              QWidget* = 0);
  void addCurve(QVector<t_polarPoint*>* data);

 private:
  QwtInterval _scaleInterval;
};

#endif
