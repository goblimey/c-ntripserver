
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnsRinex
 *
 * Purpose:    writes RINEX Clock files
 *
 * Author:     L. Mervart
 *
 * Created:    25-Apr-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <iomanip>

#include "bnsrinex.h"
#include "bnssettings.h"

using namespace std;
using namespace BNS;

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsRinex::bnsRinex(const QString& prep, const QString& ext, const QString& path,
               const QString& intr, int sampl) 
  : bnsoutf(prep, ext, path, intr, sampl) {
  bnsSettings settings;
  _append = Qt::CheckState(settings.value("fileAppend").toInt()) == Qt::Checked;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsRinex::~bnsRinex() {
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
t_irc bnsRinex::write(int GPSweek, double GPSweeks, const QString& prn, 
                   const ColumnVector& xx) {

  if (bnsoutf::write(GPSweek, GPSweeks, prn, xx, _append) == success) {

      QDateTime datTim = dateAndTimeFromGPSweek(GPSweek, GPSweeks);
      double sec = fmod(GPSweeks, 60.0);
    
      _out << "AS " << prn.toAscii().data()
           << datTim.toString("  yyyy MM dd hh mm").toAscii().data()
           << fixed      << setw(10) << setprecision(6)  << sec 
           << "  1   "
           << scientific << setw(19) << setprecision(12) << xx(4) << endl;

    return success;
  }
  else {
    return failure;
  }
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bnsRinex::writeHeader(const QDateTime& datTim) {

  _out << "     3.00           C                                       "
       << "RINEX VERSION / TYPE" << endl;

  _out << "BNS                                     " 
       << datTim.toString("yyyyMMdd hhmmss").leftJustified(20, ' ', true).toAscii().data()
       << "PGM / RUN BY / DATE" << endl;

  _out << "     1    AS                                                "
       << "# / TYPES OF DATA" << endl;

  _out << "                                                            "
       << "END OF HEADER" << endl;
}

