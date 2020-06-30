#ifndef ELEPLOT_H
#define ELEPLOT_H

#include <QtCore>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_curve.h>
#include "t_prn.h"
#include "reqcanalyze.h"

class t_availData;

class t_elePlot: public QwtPlot {
 Q_OBJECT

public:
  t_elePlot(QWidget* parent, const QMap<t_prn, t_plotData>& plotDataMap);

private:
  QwtPlotCurve* addCurve(const QString& name, const QwtSymbol& symbol,
                         const QVector<double>& xData, 
                         const QVector<double>& yData);
};

#endif
