#ifndef GnssCenter_INPEDIT_H
#define GnssCenter_INPEDIT_H

#include <QtGui>
#include "plugininterface.h"
#include "tabwidget.h"

namespace GnssCenter {

class t_keyword;
class t_panel;

class t_inpEdit : public QMainWindow {
 Q_OBJECT
 public:
  t_inpEdit();
  ~t_inpEdit();
 private slots:
  void slotOpenFile();
  void slotSaveFile();
 private:
  t_tabWidget* _tabWidget;
};

class t_inpEditFactory : public QObject, public t_pluginFactoryInterface {
 Q_OBJECT
 Q_INTERFACES(GnssCenter::t_pluginFactoryInterface)
 public:
  virtual QWidget* create() {return new t_inpEdit();} 
  virtual QString getName() const {return QString("Edit Input");}
};

} // namespace GnssCenter

#endif

