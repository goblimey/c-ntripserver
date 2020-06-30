
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncClockRinex
 *
 * Purpose:    writes RINEX Clock files
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2011
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <iomanip>

#include "bncclockrinex.h"
#include "bncsettings.h"
#include "bncversion.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncClockRinex::bncClockRinex(const QString& sklFileName, const QString& intr,
                             int sampl)
  : bncoutf(sklFileName, intr, sampl) {
  bncSettings settings;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncClockRinex::~bncClockRinex() {
  bncoutf::closeFile();
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
t_irc bncClockRinex::write(int GPSweek, double GPSweeks, const QString& prn,
                           double clkRnx, double clkRnxRate, double clkRnxAcc,
                           double clkRnxSig, double clkRnxRateSig, double clkRnxAccSig) {

  if (reopen(GPSweek, GPSweeks) == success) {

      QDateTime datTim = dateAndTimeFromGPSweek(GPSweek, GPSweeks);
      double sec = fmod(GPSweeks, 60.0);

      int numValues = 1;
      if (clkRnxSig && clkRnxRate && clkRnxRateSig) {
        numValues += 3;
      }
      if (clkRnxAcc && clkRnxAccSig) {
        numValues += 2;
      }

      _out << "AS " << prn.toLatin1().data()
           << datTim.toString("  yyyy MM dd hh mm").toLatin1().data()
           << fixed      << setw(10) << setprecision(6)  << sec
           << "  " << numValues << "   "
           << fortranFormat(clkRnx, 19, 12).toLatin1().data();

      if (numValues >=2) {
        _out << " " << fortranFormat(clkRnxSig, 19, 12).toLatin1().data() << endl;
      }
      if (numValues == 4) {
        _out << fortranFormat(clkRnxRate, 19, 12).toLatin1().data() << " ";
        _out << fortranFormat(clkRnxRateSig, 19, 12).toLatin1().data() << " ";
      }
      if (numValues == 6) {
        _out << fortranFormat(clkRnxAcc, 19, 12).toLatin1().data() << " ";
        _out << " " << fortranFormat(clkRnxAccSig, 19, 12).toLatin1().data();
      }
      _out << endl;

    return success;
  }
  else {
    return failure;
  }
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bncClockRinex::writeHeader(const QDateTime& datTim) {

  _out << "     3.00           C                                       "
       << "RINEX VERSION / TYPE" << endl;


  _out << "BNC v" << BNCVERSION     << "                               "
       << datTim.toString("yyyyMMdd hhmmss").leftJustified(20, ' ', true).toLatin1().data()
       << "PGM / RUN BY / DATE" << endl;

  _out << "     1    AS                                                "
       << "# / TYPES OF DATA" << endl;

  _out << "unknown                                                     "
       << "ANALYSIS CENTER" << endl;

  _out << "    54                                                      "
       << "# OF SOLN SATS" << endl;

  _out << "G01 G02 G03 G04 G05 G06 G07 G08 G09 G10 G11 G12 G13 G14 G15 "
       << "PRN LIST" << endl;

  _out << "G16 G17 G18 G19 G20 G21 G22 G23 G25 G26 G27 G28 G29 G30 G31 "
       << "PRN LIST" << endl;

  _out << "G32 R01 R02 R03 R05 R06 R07 R08 R09 R10 R11 R12 R13 R14 R15 "
       << "PRN LIST" << endl;

  _out << "R16 R17 R18 R19 R20 R21 R22 R23 R24                         "
       << "PRN LIST" << endl;

  _out << "     0    IGS14                                             "
       << "# OF SOLN STA / TRF" << endl;

  _out << "                                                            "
       << "END OF HEADER" << endl;
}

