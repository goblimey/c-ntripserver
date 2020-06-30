
/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      t_polarPlot
 *
 * Purpose:    Polar Plot
 *
 * Author:     L. Mervart
 *
 * Created:    23-Jun-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <qpen.h>
#include <qwt_symbol.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_canvas.h>

#include "polarplot.h"
#include "graphwin.h"

// Draw Symbols (virtual) - change symbol's color
////////////////////////////////////////////////////////////////////////////
void t_polarCurve::drawSymbols(QPainter* painter, const QwtSymbol& symbol, 
                               const QwtScaleMap& azimuthMap, 
                               const QwtScaleMap& radialMap, 
                               const QPointF& pole, int from, int to) const {
  t_colorMap colorMap;
  for (int ii = from; ii <= to; ii++) {
    QwtSymbol ss(symbol.style());
    ss.setSize(symbol.size());
    const QwtPointPolar& point = sample(ii);
    const QColor color = colorMap.color(_scaleInterval, point._value);
    ss.setBrush(QBrush(color));
    ss.setPen(QPen(color));
    QwtPolarCurve::drawSymbols(painter, ss, azimuthMap, radialMap, pole, ii,ii);
  }
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_polarPlot::t_polarPlot(const QwtText& title, const QwtInterval& scaleInterval,
                         QWidget* parent) : QwtPolarPlot(title, parent) {

  _scaleInterval = scaleInterval;

  if (false) {
    setPlotBackground(Qt::white); // sets the background of the circle only
  }
  else {
    canvas()->setAutoFillBackground(true);
    QPalette palette = canvas()->palette();
    palette.setColor(QPalette::Window, Qt::white);
    canvas()->setPalette(palette);
  }

  setAzimuthOrigin(M_PI/2.0);

  // Scales
  // ------
  setScale(QwtPolar::Radius,    0.0, 90.0);
  setScale(QwtPolar::Azimuth, 360.0,  0.0, 30.0);

  // Grids, Axes
  // -----------
  QwtPolarGrid* grid = new QwtPolarGrid();
  grid->setPen(QPen(Qt::black));
  for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ ) {
    grid->showGrid(scaleId);
  }

  grid->setAxisPen(QwtPolar::AxisAzimuth, QPen(Qt::black));

  grid->showAxis(QwtPolar::AxisAzimuth, true);
  grid->showAxis(QwtPolar::AxisTop,     true);
  grid->showAxis(QwtPolar::AxisBottom,  false);
  grid->showAxis(QwtPolar::AxisLeft,    false);
  grid->showAxis(QwtPolar::AxisRight,   false);

  grid->showGrid(QwtPolar::Azimuth, true);
  grid->showGrid(QwtPolar::Radius,  true);

  grid->attach(this);
}

// 
////////////////////////////////////////////////////////////////////////////
void t_polarPlot::addCurve(QVector<t_polarPoint*>* data) {
  t_polarCurve* curve = new t_polarCurve();
  curve->setScaleInterval(_scaleInterval);
  curve->setStyle(QwtPolarCurve::NoCurve);  // draw only symbols
  curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::red), 
                                 QPen(Qt::red), QSize(2, 2)));
  t_polarData* polarData = new t_polarData(data);
  curve->setData(polarData);
  curve->attach(this);
}
