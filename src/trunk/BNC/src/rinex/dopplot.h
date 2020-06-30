#ifndef DOPPLOT_H
#define DOPPLOT_H

#include <QtCore>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_curve.h>
#include "reqcanalyze.h"

class t_dopPlot: public QwtPlot {
 Q_OBJECT

public:
 t_dopPlot(QWidget* parent, const t_plotData& plotData);

private:
};

#endif
