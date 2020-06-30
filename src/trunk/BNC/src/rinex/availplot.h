#ifndef AVAILPLOT_H
#define AVAILPLOT_H

#include <QtCore>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_curve.h>
#include "t_prn.h"
#include "reqcanalyze.h"

class t_availPlot: public QwtPlot {
 Q_OBJECT

public:
  t_availPlot(QWidget* parent, const QMap<t_prn, t_plotData>& plotDataMap);

private:
  QwtPlotCurve* addCurve(const QString& name, const QwtSymbol& symbol,
                         const QVector<double>& xData, 
                         const QVector<double>& yData);
};

#endif
