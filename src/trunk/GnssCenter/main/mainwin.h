#ifndef GnssCenter_MAINWIN_H
#define GnssCenter_MAINWIN_H

#include <QtGui>
#include "plugininterface.h" 

namespace GnssCenter {

class t_mdiArea;

class t_pluginAction : public QAction {
 public:
  t_pluginAction(QObject* parent, t_pluginFactoryInterface* factIface) :
                QAction(factIface->getName(), parent), _factIface(factIface) {}
  t_pluginFactoryInterface* _factIface;
};

class t_mainWin : public QMainWindow {
 Q_OBJECT

 public:
  t_mainWin(QWidget* parent = 0, Qt::WindowFlags flags = 0);  
  ~t_mainWin();

 private slots:
  void slotFontSel();
  void slotSaveOptions();
  void slotStartPlugin();
  void slotHelp();
  void slotAbout();

 protected:
  virtual void closeEvent(QCloseEvent* event);

 private:
  void createMenu();
  void createToolBar();
  void createStatusBar();

  t_mdiArea* _mdi;

  QMenu*     _menuFile;
  QMenu*     _menuPlugins;
  QMenu*     _menuHlp;

  QToolBar*  _toolBar;
};

}  // namespace GnssCenter

#endif
