/* -------------------------------------------------------------------------
 * GnssCenter
 * -------------------------------------------------------------------------
 *
 * Class:      t_app
 *
 * Purpose:    This class implements the main application
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include "app.h"
#include "bnccore.h"

using namespace std;

QCoreApplication* createApplication(int& argc, char* argv[], bool GUIenabled) {

  if (GUIenabled) {
    return new t_app(argc, argv);
  }

  return new QCoreApplication(argc, argv);
}


// Constructor
////////////////////////////////////////////////////////////////////////////
t_app::t_app(int& argc, char* argv[]) : QApplication(argc, argv) {
  setWindowIcon(QPixmap(":ntrip-logo.png"));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_app::~t_app() {
}

// Handling Events (virtual)
////////////////////////////////////////////////////////////////////////////
bool t_app::event(QEvent* ev) {

  if (ev->type() == QEvent::FileOpen) {  // currently happens on Mac only
    QString fileName = static_cast<QFileOpenEvent*>(ev)->file();
    BNC_CORE->setConfFileName(fileName);
    return true;
  }

  return QApplication::event(ev);
}


