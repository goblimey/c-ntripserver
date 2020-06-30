/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      main
 *
 * Purpose:    Application starts here
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bns.h"
#include "bnswindow.h"
#include "bnsutils.h"
#include "bnsapp.h"
#include "bnssettings.h"

using namespace std;

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

  QString confFileName;

  // Command-Line Options
  // --------------------
  bool GUIenabled = true;

  for (int ii = 1; ii < argc; ii++) {
    if (QByteArray(argv[ii]) == "-nw") {
      GUIenabled = false;
    }
    if (ii + 1 < argc) {
      if (QByteArray(argv[ii]).indexOf("-conf")   != -1) {
        confFileName = QString(argv[ii+1]);
      }
    }
  }
  if (argc == 2 && GUIenabled) {
    confFileName = QString(argv[1]);
  }

  // Main Qt Class
  // -------------
//QApplication app(argc, argv, GUIenabled);
  bnsApp app(argc, argv, GUIenabled);

  app.setApplicationName("BNS");
  app.setOrganizationName("BKG");
  app.setOrganizationDomain("www.bkg.bund.de");
  app.setConfFileName( confFileName );

  // Interactive Mode - open the main window
  // ---------------------------------------
  if (GUIenabled) {
    bnsWindow* bnsWin = new bnsWindow();
    bnsWin->show();
  }

  // Non-Interactive (Batch) Mode
  // ----------------------------
  else {
    t_bns* bns = new t_bns();
    bns->start();
  }

  return app.exec();
}
