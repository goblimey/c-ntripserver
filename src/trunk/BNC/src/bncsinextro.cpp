
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncSinexTro
 *
 * Purpose:    writes SINEX TRO files
 *
 * Author:     A. Stürze
 *
 * Created:    19-Feb-2015
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <iomanip>

#include "bncsinextro.h"

using namespace BNC_PPP;
using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSinexTro::bncSinexTro(const t_pppOptions* opt,
                         const QString& sklFileName, const QString& intr,
                         int sampl)
  : bncoutf(sklFileName, intr, sampl) {

  _opt       = opt;
  _sampl =  sampl;

  _antex = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSinexTro::~bncSinexTro() {
  closeFile();
  if (_antex)
    delete _antex;
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void bncSinexTro::writeHeader(const QDateTime& datTim) {
  int    GPSWeek;
  double GPSWeeks;
  bncSettings settings;
  GPSweekFromDateAndTime(datTim, GPSWeek, GPSWeeks);
  int daysec    = int(fmod(GPSWeeks, 86400.0));
  int dayOfYear = datTim.date().dayOfYear();
  QString yy    = datTim.toString("yy");
  QString creationTime = QString("%1:%2:%3").arg(yy)
                                            .arg(dayOfYear, 3, 10, QLatin1Char('0'))
                                            .arg(daysec   , 5, 10, QLatin1Char('0'));
  QString startTime = creationTime;
  QString intStr = settings.value("PPP/snxtroIntr").toString();
  int intr, indHlp = 0;
  if      ((indHlp = intStr.indexOf("min")) != -1) {
    intr = intStr.left(indHlp-1).toInt();
    intr *= 60;
  }
  else if ((indHlp = intStr.indexOf("hour")) != -1) {
    intr = intStr.left(indHlp-1).toInt();
    intr *= 3600;
  }
  else if ((indHlp = intStr.indexOf("day")) != -1) {
    intr = intStr.left(indHlp-1).toInt();
    intr *= 86400;
  }
  int nominalStartSec = daysec - (int(fmod(double(daysec), double(intr))));
  int nominalEndSec = nominalStartSec + intr - _sampl;
  QString endTime = QString("%1:%2:%3").arg(yy)
                                       .arg(dayOfYear     , 3, 10, QLatin1Char('0'))
                                       .arg(nominalEndSec , 5, 10, QLatin1Char('0'));
  int numEpochs = ((nominalEndSec - daysec) / _sampl) +1;
  QString epo  = QString("%1").arg(numEpochs, 5, 10, QLatin1Char('0'));
  QString ac   = QString("%1").arg(settings.value("PPP/snxtroAc").toString(),3,QLatin1Char(' '));
  QString sol  = QString("%1").arg(settings.value("PPP/snxtroSol").toString(),4,QLatin1Char(' '));
  QString corr = settings.value("PPP/corrMount").toString();

  _out << "%=TRO 2.00 " << ac.toStdString() << " "
       << creationTime.toStdString() << " " << ac.toStdString() << " "
       << startTime.toStdString()    << " " << endTime.toStdString() << " P "
       << epo.toStdString() << " 0 " << " T " << endl;

  _out << "+FILE/REFERENCE" << endl;
  _out << " DESCRIPTION        " << "BNC generated SINEX TRO file" << endl;
  _out << " OUTPUT             " << "Total Troposphere Zenith Path Delay Product" << endl;
  _out << " SOFTWARE           " <<  BNCPGMNAME <<  endl;
  _out << " INPUT              " << "Ntrip streams, additional Orbit and Clock information from "
                                 << corr.toStdString() <<endl;
  _out << "-FILE/REFERENCE" << endl << endl;

  double recEll[3];
  int lonD, lonM,  latD, latM;
  double lonS, latS;
  xyz2ell(_opt->_xyzAprRover.data(), recEll);
  deg2DMS(recEll[0] * 180.0 / M_PI, latD, latM, latS);
  deg2DMS(recEll[1] * 180.0 / M_PI, lonD, lonM, lonS);
  QString country;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    if (hlp.size() < 7)
      continue;
    if (hlp.join(" ").indexOf(QString::fromStdString(_opt->_roverName), 0) != -1) {
      country = hlp[2];
    }
  }
  _out << "+SITE/ID" << endl;
  _out << "*CODE PT DOMES____ T _STATION DESCRIPTION__ APPROX_LON_ APPROX_LAT_ _APP_H_" << endl;
  _out << " " << _opt->_roverName.substr(0,4) << "  A           P "
       << country.toStdString() << "                   "
       << QString(" %1").arg(lonD, 3, 10, QLatin1Char(' ')).toStdString()
       << QString(" %1").arg(lonM, 2, 10, QLatin1Char(' ')).toStdString()
       << QString(" %1").arg(lonS, 4, 'f', 1, QLatin1Char(' ')).toStdString()
       << QString(" %1").arg(latD, 3, 10, QLatin1Char(' ')).toStdString()
       << QString(" %1").arg(latM, 2, 10, QLatin1Char(' ')).toStdString()
       << QString(" %1").arg(latS, 4, 'f', 1, QLatin1Char(' ')).toStdString()
       << QString(" %1").arg(recEll[2], 7, 'f', 1, QLatin1Char(' ')).toStdString()
       << endl;
  _out << "-SITE/ID" << endl << endl;

  if (!_opt->_recNameRover.empty()) {
    _out << "+SITE/RECEIVER" << endl;
    _out << "*SITE PT SOLN T DATA_START__ DATA_END____ DESCRIPTION_________ S/N__ FIRMWARE___" << endl;
    _out << " " << _opt->_roverName.substr(0,4) << "  A "  <<  sol.toStdString() << " P "
         << startTime.toStdString() << " " << endTime.toStdString()
         << QString(" %1").arg(_opt->_recNameRover.c_str(), 20,QLatin1Char(' ')).toStdString()
         << " -----" << " -----------" << endl;
    _out << "-SITE/RECEIVER" << endl << endl;
  }

  _out << "+SITE/ANTENNA" << endl;
  _out << "*SITE PT SOLN T DATA_START__ DATA_END____ DESCRIPTION_________ S/N__" << endl;
  _out << " " << _opt->_roverName.substr(0,4) << "  A "  <<  sol.toStdString() << " P "
       << startTime.toStdString() << " " << endTime.toStdString() << " "
       << _opt->_antNameRover << " -----" << endl;
  _out << "-SITE/ANTENNA" << endl << endl;

  if (!_opt->_antexFileName.empty()) {
    _antex = new bncAntex(_opt->_antexFileName.c_str());
    if (_opt->_LCsGPS.size()) {
      _out << "+SITE/GPS_PHASE_CENTER" << endl;
      _out << "*                           UP____ NORTH_ EAST__ UP____ NORTH_ EAST__" << endl;
      _out << "*DESCRIPTION_________ S/N__ L1->ARP(m)__________ L2->ARP(m)__________ AZ_EL____" << endl;
      _out << QString(" %1").arg(_opt->_antNameRover.c_str(), 20,QLatin1Char(' ')).toStdString()
           <<  " -----"
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::G1).toStdString()
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::G2).toStdString()
           <<  " ---------"
        << endl;
      _out << "-SITE/GPS_PHASE_CENTER" << endl << endl;
    }
    if (_opt->_LCsGLONASS.size()) {
      _out << "+SITE/GLONASS_PHASE_CENTER" << endl;
      _out << "*                           UP____ NORTH_ EAST__ UP____ NORTH_ EAST__" << endl;
      _out << "*DESCRIPTION_________ S/N__ L1->ARP(m)__________ L2->ARP(m)__________ AZ_EL____" << endl;
      _out << QString(" %1").arg(_opt->_antNameRover.c_str(), 20,QLatin1Char(' ')).toStdString()
           <<  " -----"
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::R1).toStdString()
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::R2).toStdString()
           <<  " ---------"
        << endl;
      _out << "-SITE/GLONASS_PHASE_CENTER" << endl << endl;
    }
    if (_opt->_LCsGalileo.size()) {
      _out << "+SITE/GALILEO_PHASE_CENTER" << endl;
      _out << "*                           UP____ NORTH_ EAST__ UP____ NORTH_ EAST__" << endl;
      _out << "*DESCRIPTION_________ S/N__ L1->ARP(m)__________ L2->ARP(m)__________ AZ_EL____" << endl;
      _out << QString(" %1").arg(_opt->_antNameRover.c_str(), 20,QLatin1Char(' ')).toStdString()
           <<  " -----"
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::E1).toStdString()
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::E5).toStdString()
        << endl;
      _out << "-SITE/GALILEO_PHASE_CENTER" << endl << endl;
    }
    if (_opt->_LCsBDS.size()) {
      _out << "+SITE/BEIDOU_PHASE_CENTER" << endl;
      _out << "*                           UP____ NORTH_ EAST__ UP____ NORTH_ EAST__" << endl;
      _out << "*DESCRIPTION_________ S/N__ L1->ARP(m)__________ L2->ARP(m)__________ AZ_EL____" << endl;
      _out << QString(" %1").arg(_opt->_antNameRover.c_str(), 20,QLatin1Char(' ')).toStdString()
           <<  " -----"
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::C2).toStdString()
           << _antex->pcoSinexString(_opt->_antNameRover, t_frequency::C7).toStdString()
        << endl;
      _out << "-SITE/BEIDOU_PHASE_CENTER" << endl << endl;
    }
    delete _antex;
    _antex = 0;
  }

  _out << "+SITE/ECCENTRICITY" << endl;
  _out << "*                                             UP______ NORTH___ EAST____" << endl;
  _out << "*SITE PT SOLN T DATA_START__ DATA_END____ AXE ARP->BENCHMARK(M)_________" << endl;
  _out << " " << _opt->_roverName.substr(0,4) << "  A "  <<  sol.toStdString() << " P "
       << startTime.toStdString() << " " << endTime.toStdString() << " UNE"
       << QString("%1").arg(_opt->_neuEccRover(3), 9, 'f', 4, QLatin1Char(' ')).toStdString()
       << QString("%1").arg(_opt->_neuEccRover(1), 9, 'f', 4, QLatin1Char(' ')).toStdString()
       << QString("%1").arg(_opt->_neuEccRover(2), 9, 'f', 4, QLatin1Char(' ')).toStdString() << endl;
  _out << "-SITE/ECCENTRICITY" << endl << endl;

  _out << "+TROP/COORDINATES" << endl;
  _out << "*SITE PT SOLN T STA_X_______ STA_Y_______ STA_Z_______ SYSTEM REMARK" << endl;
  _out << " " << _opt->_roverName.substr(0,4) << "  A "  <<  sol.toStdString() << " P"
       << QString(" %1").arg(_opt->_xyzAprRover(1), 12, 'f', 3, QLatin1Char(' ')).toStdString()
       << QString(" %1").arg(_opt->_xyzAprRover(2), 12, 'f', 3, QLatin1Char(' ')).toStdString()
       << QString(" %1").arg(_opt->_xyzAprRover(3), 12, 'f', 3, QLatin1Char(' ')).toStdString()
       << " ITRF14 " << ac.toStdString() << endl;
  _out << "-TROP/COORDINATES"<< endl << endl;

  _out << "+TROP/DESCRIPTION" << endl;
  _out << "*KEYWORD______________________ VALUE(S)______________" << endl;
  _out << " SAMPLING INTERVAL                               "
       << setw(4) << _sampl << endl;
  _out << " SAMPLING TROP                                   "
       << setw(4) << _sampl << endl;
  _out << " ELEVATION CUTOFF ANGLE                          "
       << setw(4) <<  int(_opt->_minEle * 180.0/M_PI) << endl;
  _out << " TROP MAPPING FUNCTION         " << "Saastamoinen" << endl;
  _out << " SOLUTION_FIELDS_1             " << "TROTOT STDEV" << endl;
  _out << "-TROP/DESCRIPTION"<< endl << endl;

  _out << "+TROP/SOLUTION" << endl;
  _out << "*SITE EPOCH_______ TROTOT STDEV" << endl;
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
t_irc bncSinexTro::write(QByteArray staID, int GPSWeek, double GPSWeeks,
    double trotot, double stdev) {

  QDateTime datTim = dateAndTimeFromGPSweek(GPSWeek, GPSWeeks);
  int daysec    = int(fmod(GPSWeeks, 86400.0));
  int dayOfYear = datTim.date().dayOfYear();
  QString yy    = datTim.toString("yy");
  QString time  = QString("%1:%2:%3").arg(yy)
                                     .arg(dayOfYear, 3, 10, QLatin1Char('0'))
                                     .arg(daysec   , 5, 10, QLatin1Char('0'));

  if ((reopen(GPSWeek, GPSWeeks) == success) &&
      (fmod(daysec, double(_sampl)) == 0.0)) {
    _out << ' '  << staID.left(4).data() << ' ' << time.toStdString() << ' '
         << noshowpos << setw(6) << setprecision(1) << trotot * 1000.0
         << noshowpos << setw(6) << setprecision(1) << stdev  * 1000.0 << endl;
    _out.flush();
    return success;
  }  else {
    return failure;
  }
}

// Close File (write last lines)
////////////////////////////////////////////////////////////////////////////
void bncSinexTro::closeFile() {
  _out << "-TROP/SOLUTION" << endl;
  _out << "%=ENDTROP" << endl;
  bncoutf::closeFile();
}




