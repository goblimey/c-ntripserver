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
 * Class:      t_availPlot
 *
 * Purpose:    Plot with satellite availability
 *
 * Author:     L. Mervart
 *
 * Created:    30-Aug-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <qwt_scale_draw.h>
#include <qwt_text.h>
#include <qwt_legend.h>
#include <qwt_plot_canvas.h>

#include "availplot.h"
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

//
//////////////////////////////////////////////////////////////////////////////
class t_scaleDrawPrn : public QwtScaleDraw {
 public:
  t_scaleDrawPrn() {}
  virtual QwtText label(double iPrn) const {
    return _yLabels[int(iPrn)];
  }
  QMap<int, QString> _yLabels;
};

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_availPlot::t_availPlot(QWidget* parent, const QMap<t_prn, t_plotData>& plotDataMap) : 
QwtPlot(parent) {

  setCanvasBackground(QColor(Qt::white));
  ((QwtPlotCanvas *)canvas())->setFrameStyle(QFrame::NoFrame | QFrame::Plain);

  // Axes
  // ----
  setAxisScaleDraw(QwtPlot::xBottom, new t_scaleDrawTime());
  setAxisLabelRotation(QwtPlot::xBottom, -10.0);
  setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

  t_scaleDrawPrn* scaleDrawPrn = new t_scaleDrawPrn();
  setAxisScaleDraw(QwtPlot::yLeft, scaleDrawPrn);

  // Smaller Font for y-Axis
  // -----------------------
  QFont yFont = axisFont(QwtPlot::yLeft);
  yFont.setPointSize(yFont.pointSize()/2);
  setAxisFont(QwtPlot::yLeft, yFont);

  // Symbols
  // -------
  QColor red(220,20,60);
  QColor green(150,200,50);
  QColor blue(60,100,200);
  QwtSymbol symbRed(QwtSymbol::Rect, QBrush(red), QPen(red), QSize(2,1));
  QwtSymbol symbGreen(QwtSymbol::Rect, QBrush(green), QPen(green), QSize(2,1));
  QwtSymbol symbBlue (QwtSymbol::Rect, QBrush(blue), QPen(blue), QSize(2,1));

  // Legend
  // ------
  QwtLegend* legend = new QwtLegend;
  insertLegend(legend, QwtPlot::RightLegend);

  QVector<double> xData0(0);
  QVector<double> yData0(0);
  addCurve("OK  ", symbGreen, xData0, yData0);
  addCurve("Gap ", symbBlue,  xData0, yData0);
  addCurve("Slip", symbRed,   xData0, yData0);
 
  // Curves
  // ------
  int iC = 0;
  QMapIterator<t_prn, t_plotData > it(plotDataMap);
  while (it.hasNext()) {
    it.next();
    ++iC;
    QString           prn      = QString(it.key().toString().c_str());
    const t_plotData& plotData = it.value();

    scaleDrawPrn->_yLabels[iC] = prn;

    double eps = -0.1;

    for (QMap<char, t_plotData::t_hlpStatus >::const_iterator it = plotData._status.begin();
      it != plotData._status.end(); it++) {

      const t_plotData::t_hlpStatus& status = it.value();

      // OK Curve
      // --------
      if (status._ok.size()) {
        const QVector<double>& xData = status._ok;
        QVector<double>        yData(xData.size(), double(iC)+eps);
        QwtPlotCurve* curve = addCurve(prn, symbGreen, xData, yData);
        curve->setItemAttribute(QwtPlotItem::Legend, false);
      }
  
      // Gaps Curve
      // ----------
      if (status._gap.size()) {
        const QVector<double>& xData = status._gap;
        QVector<double>        yData(xData.size(), double(iC)+eps);
        QwtPlotCurve* curve = addCurve(prn, symbBlue, xData, yData);
        curve->setItemAttribute(QwtPlotItem::Legend, false);
      }
  
      // Slips Curve
      // -----------
      if (status._slip.size()) {
        const QVector<double>& xData = status._slip;
        QVector<double>        yData(xData.size(), double(iC)+eps);
        QwtPlotCurve* curve = addCurve(prn, symbRed, xData, yData);
        curve->setItemAttribute(QwtPlotItem::Legend, false);
      }

      eps += 0.2;
    }
  }
  
  QList<double> ticks[QwtScaleDiv::NTickTypes];
  QList<double> &majorTicks = ticks[QwtScaleDiv::MajorTick];
  QMapIterator<int, QString> itT(scaleDrawPrn->_yLabels);
  while (itT.hasNext()) {
    itT.next();
    majorTicks << double(itT.key());
  }
  QwtScaleDiv yScaleDiv(majorTicks.first()-0.5, majorTicks.last()+0.5, ticks );
  setAxisScaleDiv(QwtPlot::yLeft, yScaleDiv);

  // Important !!!
  // -------------
  replot();
}

// Add Curve
//////////////////////////////////////////////////////////////////////////////
QwtPlotCurve* t_availPlot::addCurve(const QString& name, 
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
  return curve;
}
