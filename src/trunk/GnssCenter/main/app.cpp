/* -------------------------------------------------------------------------
 * RTNet GUI
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

using namespace std;
using namespace GnssCenter;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_app::t_app(int& argc, char* argv[], bool GUIenabled) : 
  QApplication(argc, argv, GUIenabled) {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_app::~t_app() {
}

// 
////////////////////////////////////////////////////////////////////////////
void t_app::setConfFileName(const QString& confFileName) {
  if (confFileName.isEmpty()) {
    _confFileName = QDir::homePath() + QDir::separator() 
                  + ".config" + QDir::separator()
                  + organizationName() + QDir::separator()
                  + applicationName() + ".conf";
  }
  else {
    _confFileName = confFileName;
  }
}

//  
////////////////////////////////////////////////////////////////////////////
void t_app::slotMessage(QByteArray msg) {
  qDebug() << msg;
}
