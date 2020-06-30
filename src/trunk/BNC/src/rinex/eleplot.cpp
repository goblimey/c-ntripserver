// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      t_elePlot
 *
 * Purpose:    Plot satellite elevations
 *
 * Author:     L. Mervart
 *
 * Created:    08-Sep-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <qwt_scale_draw.h>
#include <qwt_text.h>
#include <qwt_legend.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>

#include "eleplot.h"
#include "reqcanalyze.h"

//
//////////////////////////////////////////////////////////////////////////////
class t_scaleDrawTime : public QwtScaleDraw {
 public:
  t_scaleDrawTime() {}
  virtual QwtText label(double mjdX24) const {
    bncTime epoTime; epoTime.setmjd(mjdX24/24.0);
    return QwtText(epoTime.timestr(0,':').c_str());
  }
};

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_elePlot::t_elePlot(QWidget* parent, const QMap<t_prn, t_plotData>& plotDataMap) 
: QwtPlot(parent) {

  setCanvasBackground(QColor(Qt::white));
  ((QwtPlotCanvas *)canvas())->setFrameStyle(QFrame::NoFrame | QFrame::Plain);

  // Axes
  // ----
  setAxisScaleDraw(QwtPlot::xBottom, new t_scaleDrawTime());
  setAxisLabelRotation(QwtPlot::xBottom, -10.0);
  setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
  setAxisScale(QwtPlot::yLeft, 0.0, 90.0);

  // Legend
  // ------
  QwtLegend* legend = new QwtLegend;
  insertLegend(legend, QwtPlot::RightLegend);

  // Curves
  // ------
  int numCurves = plotDataMap.size();
  if (numCurves > 0) {
    int iC = 0;
    QMapIterator<t_prn, t_plotData> it(plotDataMap);
    while (it.hasNext()) {
      it.next();
      ++iC;
      QString           prn      = QString(it.key().toString().c_str());
      const t_plotData& plotData = it.value();
    
      // Draw one curve
      // --------------
      if (plotData._mjdX24.size()) {
        const QVector<double>& xData = plotData._mjdX24;
        const QVector<double>& yData = plotData._eleDeg;
        QColor color = QColor::fromHsv(int((iC-1)*(359.0/numCurves)), 255, 255);
        QwtSymbol symbol(QwtSymbol::Rect, QBrush(color), QPen(color), QSize(1,1));
        addCurve(prn, symbol, xData, yData);
      }
    }
  }
  
  // Important !!!
  // -------------
  replot();
}

// Add Curve
//////////////////////////////////////////////////////////////////////////////
QwtPlotCurve* t_elePlot::addCurve(const QString& name, 
                                    const QwtSymbol& symbol,
                                    const QVector<double>& xData,
                                    const QVector<double>& yData) {
  QwtPlotCurve* curve = new QwtPlotCurve(name);
  QwtSymbol *s = new QwtSymbol(symbol.style());
  s->setSize(symbol.size());
  s->setBrush(symbol.brush());
  s->setPen(symbol.pen());
  curve->setSymbol(s);
  curve->setStyle(QwtPlotCurve::NoCurve);
  curve->setXAxis(QwtPlot::xBottom);
  curve->setYAxis(QwtPlot::yLeft);
  curve->setSamples(xData, yData);
  curve->attach(this);

  if (xData.size() > 0 && yData.size() > 0) {
    QwtPlotMarker* marker = new QwtPlotMarker();
    int ii = xData.size() / 2;
    marker->setValue(xData[ii], yData[ii]);
    QwtText text(name);
    text.setColor(symbol.pen().color());
    marker->setLabel(text);
    marker->setLabelAlignment(Qt::AlignTop);
    marker->attach(this);
  }

  return curve;
}
