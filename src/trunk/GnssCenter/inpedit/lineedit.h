#ifndef GnssCenter_LINEEDIT_H
#define GnssCenter_LINEEDIT_H

#include <QtGui>

namespace GnssCenter {

class t_lineEdit : public QLineEdit {
  Q_OBJECT

 public:
  t_lineEdit(QWidget* parent = 0);
  ~t_lineEdit();
  private:
};

} // namespace GnssCenter

#endif
