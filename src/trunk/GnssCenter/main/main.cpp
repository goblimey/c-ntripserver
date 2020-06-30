
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      Main Program
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "app.h"
#include "mainwin.h"

using namespace GnssCenter;

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {

  t_app app(argc, argv, true);
  app.setApplicationName("GnssCenter");
  app.setOrganizationName("BKG");
  app.setOrganizationDomain("www.bkg.bund.de");
  app.setConfFileName("");

  t_mainWin* mainWin = new t_mainWin();
  mainWin->show();

  return app.exec();
}
