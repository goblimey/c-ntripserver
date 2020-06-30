
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_worldPlot
 *
 * Purpose:    Plot map of stations/satellites
 *
 * Author:     L. Mervart
 *
 * Created:    12-Sep-2013
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <QtSvg>

#include <qwt_symbol.h>
#include <qwt_plot_svgitem.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_renderer.h>

#include "worldplot.h"

using namespace std;
using namespace GnssCenter;

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_worldPlot::t_worldPlot() : QwtPlot() {

  // Map in Scalable Vector Graphics (svg) Format
  // --------------------------------------------
  this->setAxisScale(QwtPlot::xBottom, -180.0, 180.0);
  this->setAxisScale(QwtPlot::yLeft,    -90.0,  90.0);

  _zoomer = new QwtPlotZoomer(this->canvas());

  this->canvas()->setFocusPolicy(Qt::WheelFocus);

  QwtPlotSvgItem* mapItem = new QwtPlotSvgItem();
  mapItem->loadFile(QRectF(-180.0, -90.0, 360.0, 180.0), ":world.svg");
  mapItem->attach(this);

  // Important
  // ---------
  this->replot();
}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_worldPlot::~t_worldPlot() { 
}

// 
/////////////////////////////////////////////////////////////////////////////
void t_worldPlot::slotNewPoints(const QList<t_point*>& points) {

  // Remove old markers
  // ------------------
  QListIterator<QwtPlotMarker*> im(_markers);
  while (im.hasNext()) {
    QwtPlotMarker* marker = im.next();
    marker->detach();
    delete marker;
  }
  _markers.clear();

  QListIterator<t_point*> ip(points);
  while (ip.hasNext()) {
    t_point* point = ip.next();

    if (point->_lonDeg > 180.0) point->_lonDeg -= 360.0;
  
    QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Rect, QBrush(point->_color), 
                                      QPen(point->_color), QSize(2,2));
    QwtPlotMarker* marker = new QwtPlotMarker();
    marker->setValue(point->_lonDeg, point->_latDeg);
    if (point->_lonDeg > 170.0) {
      marker->setLabelAlignment(Qt::AlignLeft);
    }
    else {
      marker->setLabelAlignment(Qt::AlignRight);
    }
    QwtText text(point->_name);
    QFont   font = text.font();
    font.setPointSize(font.pointSize()*0.8);
    text.setFont(font);
    text.setColor(point->_color);
    marker->setLabel(text);
    marker->setSymbol(symbol);
    marker->attach(this);
    _markers.append(marker);
  }

  replot();
}

// Print the widget
////////////////////////////////////////////////////////////////////////////
void t_worldPlot::slotPrint() {

  QPrinter printer;
  QPrintDialog* dialog = new QPrintDialog(&printer, this);
  dialog->setWindowTitle(tr("Print Map"));
  if (dialog->exec() != QDialog::Accepted) {
    return;
  }
  else {
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);
    renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, true);
    renderer.renderTo(this, printer);
  }
}


