#ifndef GnssCenter_MDIAREA_H
#define GnssCenter_MDIAREA_H

#include <QtGui>

namespace GnssCenter {

class t_mdiArea : public QMdiArea {
 Q_OBJECT
 public:
  t_mdiArea(QWidget* parent);
  virtual ~t_mdiArea();
};

} // namespace GnssCenter

#endif
