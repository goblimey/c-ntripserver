#ifndef GnssCenter_DLGCONF_H
#define GnssCenter_DLGCONF_H

#include <QtGui>
#include "const.h"

namespace GnssCenter {

class t_dlgConf : public QDialog {
 Q_OBJECT
 public:
  t_dlgConf(QWidget* parent);
  ~t_dlgConf();
 public slots:
  virtual void accept();
 private:
  QLineEdit* _hostLineEdit;
  QLineEdit* _portLineEdit;
};

} // namespace GnssCenter

#endif
