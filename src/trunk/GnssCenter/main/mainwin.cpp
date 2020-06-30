
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_mainWin
 *
 * Purpose:    Re-Implements QMainWindow
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "mainwin.h" 
#include "settings.h" 
#include "mdiarea.h" 
#include "app.h" 

using namespace std;
using namespace GnssCenter;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_mainWin::t_mainWin(QWidget* parent, Qt::WindowFlags flags) : 
  QMainWindow(parent, flags) {

  _mdi = new t_mdiArea(0);
  setCentralWidget(_mdi);

  createMenu();
  createToolBar();
  createStatusBar();

  // Handle Plugins
  // --------------
  QDir pluginsDir = QDir(qApp->applicationDirPath());
  
  pluginsDir.cd("plugins");

  foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
    QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
    QObject* object = loader.instance();
    if (!object) {
      qDebug() << loader.errorString();
    }
    else {
      t_pluginFactoryInterface* plugin = qobject_cast<t_pluginFactoryInterface*>(object);
      if (plugin) {
        t_pluginAction* action = new t_pluginAction(this, plugin);
        _menuPlugins->addAction(action);
        connect(action, SIGNAL(triggered()), SLOT(slotStartPlugin()));
      }
    }
  }

}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_mainWin::~t_mainWin() {
}

// 
////////////////////////////////////////////////////////////////////////////
void t_mainWin::createToolBar() {
  _toolBar = new QToolBar("Tool Bar", this);
  _toolBar->addWidget(new QLabel("Tool Bar"));
  addToolBar(Qt::BottomToolBarArea, _toolBar);
}

// 
////////////////////////////////////////////////////////////////////////////
void t_mainWin::createStatusBar() {
  statusBar()->addPermanentWidget(new QLabel("Status Bar"));
}

// 
////////////////////////////////////////////////////////////////////////////
void t_mainWin::createMenu() {

  // Create Actions
  // --------------
  QAction* actFontSel = new QAction(tr("Select &Font"),this);
  connect(actFontSel, SIGNAL(triggered()), SLOT(slotFontSel()));

  QAction* actSaveOpt = new QAction(tr("&Save && Reread Configuration"),this);
  connect(actSaveOpt, SIGNAL(triggered()), SLOT(slotSaveOptions()));

  QAction* actQuit  = new QAction(tr("&Quit"),this);
  connect(actQuit, SIGNAL(triggered()), SLOT(close()));

  QAction* actHelp = new QAction(tr("&Help Contents"),this);
  connect(actHelp, SIGNAL(triggered()), SLOT(slotHelp()));

  QAction* actAbout = new QAction(tr("&About"),this);
  connect(actAbout, SIGNAL(triggered()), SLOT(slotAbout()));

  // Create Menu
  // -----------
  _menuFile = menuBar()->addMenu(tr("&File"));
  _menuFile->addAction(actFontSel);
  _menuFile->addSeparator();
  _menuFile->addAction(actSaveOpt);
  _menuFile->addSeparator();
  _menuFile->addAction(actQuit);

  _menuPlugins = menuBar()->addMenu(tr("&Plugins"));

  _menuHlp = menuBar()->addMenu(tr("&Help"));
  _menuHlp->addAction(actHelp);
  _menuHlp->addAction(actAbout);
}

// Select Fonts
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotFontSel() {
  bool ok;
  QFont newFont = QFontDialog::getFont(&ok, this->font(), this); 
  if (ok) {
    t_settings settings;
    settings.setValue("font", newFont.toString());
    QApplication::setFont(newFont);
  }
}

// Save Options (serialize)
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotSaveOptions() {
  t_settings settings;
  settings.sync();
}

// 
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotStartPlugin() {
  t_pluginAction* action = dynamic_cast<t_pluginAction*>(sender());
  QWidget* widget = action->_factIface->create();
  t_app* app = dynamic_cast<t_app*>(qApp);
  if (app) {
    const QMetaObject* metaObj = widget->metaObject();
    QByteArray bncMessageName = QMetaObject::normalizedSignature("bncMessage(QByteArray)");
    if (metaObj->indexOfSignal(bncMessageName) != -1) {
      connect(widget, SIGNAL(bncMessage(QByteArray)), app, SLOT(slotMessage(QByteArray)));
    }
  }
  widget->setMinimumSize(500, 300);
  QMdiSubWindow* subWindow = _mdi->addSubWindow((QWidget*) widget);
  subWindow->show();
}

// Help Window
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotHelp() {
}

// About Message
////////////////////////////////////////////////////////////////////////////
void t_mainWin::slotAbout() {
}

// Close Application gracefully
////////////////////////////////////////////////////////////////////////////
void t_mainWin::closeEvent(QCloseEvent* event) {
  int iRet = QMessageBox::question(this, "Close", "Save Options?", 
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::Cancel);
  if      (iRet == QMessageBox::Cancel) {
    event->ignore();
    return;
  }
  else if (iRet == QMessageBox::Yes) {
    slotSaveOptions();
  }
  QMainWindow::closeEvent(event);
}
