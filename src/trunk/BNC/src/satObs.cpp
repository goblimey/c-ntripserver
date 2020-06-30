#include <iostream>
#include <iomanip>
#include <sstream>
#include <newmatio.h>

#include "satObs.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_clkCorr::t_clkCorr() {
  _updateInt  = 0;
  _iod        = 0;
  _dClk       = 0.0;
  _dotDClk    = 0.0;
  _dotDotDClk = 0.0;
}

//
////////////////////////////////////////////////////////////////////////////
void t_clkCorr::writeEpoch(ostream* out, const QList<t_clkCorr>& corrList) {
  if (!out || corrList.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime;
  QListIterator<t_clkCorr> it(corrList);
  while (it.hasNext()) {
    const t_clkCorr& corr = it.next();
    if (!epoTime.valid()) {
      epoTime = corr._time;
      *out << "> CLOCK " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << " "
          <<  corr._updateInt <<  " "
           << corrList.size() << ' ' << corr._staID << endl;
    }
    *out << corr._prn.toString() << ' ' << setw(11) << corr._iod << ' '
         << setw(10) << setprecision(4) << corr._dClk       * t_CST::c << ' '
         << setw(10) << setprecision(4) << corr._dotDClk    * t_CST::c << ' '
         << setw(10) << setprecision(4) << corr._dotDotDClk * t_CST::c << endl;
  }
  out->flush();
}

//
////////////////////////////////////////////////////////////////////////////
void t_clkCorr::readEpoch(const string& epoLine, istream& inStream, QList<t_clkCorr>& corrList) {
  bncTime      epoTime;
  unsigned int updateInt;
  int          numCorr;
  string       staID;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, updateInt, numCorr, staID) != t_corrSSR::clkCorr) {
    return;
  }
  for (int ii = 0; ii < numCorr; ii++) {
    t_clkCorr corr;
    corr._time      = epoTime;
    corr._updateInt = updateInt;
    corr._staID     = staID;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());

    in >> corr._prn >> corr._iod >> corr._dClk >> corr._dotDClk >> corr._dotDotDClk;
    if (corr._prn.system() == 'E') {
      corr._prn.setFlags(1);// I/NAV
    }
    corr._dClk       /= t_CST::c;
    corr._dotDClk    /= t_CST::c;
    corr._dotDotDClk /= t_CST::c;

    corrList.push_back(corr);
  }
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_orbCorr::t_orbCorr() {
  _updateInt = 0;
  _iod       = 0;
  _system    = 'R';
  _xr.ReSize(3);    _xr    = 0.0;
  _dotXr.ReSize(3); _dotXr = 0.0;
}

//
////////////////////////////////////////////////////////////////////////////
void t_orbCorr::writeEpoch(ostream* out, const QList<t_orbCorr>& corrList) {
  if (!out || corrList.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime;
  QListIterator<t_orbCorr> it(corrList);
  while (it.hasNext()) {
    const t_orbCorr& corr = it.next();
    if (!epoTime.valid()) {
      epoTime = corr._time;
      *out << "> ORBIT " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << " "
           << corr._updateInt <<  " "
           << corrList.size() << ' ' << corr._staID << endl;
    }
    *out << corr._prn.toString() << ' ' << setw(11) << corr._iod << ' '
         << setw(10) << setprecision(4) << corr._xr[0]     << ' '
         << setw(10) << setprecision(4) << corr._xr[1]     << ' '
         << setw(10) << setprecision(4) << corr._xr[2]     << "    "
         << setw(10) << setprecision(4) << corr._dotXr[0]  << ' '
         << setw(10) << setprecision(4) << corr._dotXr[1]  << ' '
         << setw(10) << setprecision(4) << corr._dotXr[2]  << endl;
  }
  out->flush();
}

//
////////////////////////////////////////////////////////////////////////////
void t_orbCorr::readEpoch(const string& epoLine, istream& inStream, QList<t_orbCorr>& corrList) {
  bncTime      epoTime;
  unsigned int updateInt;
  int          numCorr;
  string       staID;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, updateInt, numCorr, staID) != t_corrSSR::orbCorr) {
    return;
  }
  for (int ii = 0; ii < numCorr; ii++) {
    t_orbCorr corr;
    corr._time      = epoTime;
    corr._updateInt = updateInt;
    corr._staID     = staID;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());

    in >> corr._prn      >> corr._iod
       >> corr._xr[0]    >> corr._xr[1]    >> corr._xr[2]
       >> corr._dotXr[0] >> corr._dotXr[1] >> corr._dotXr[2];

    if (corr._prn.system() == 'E') {
      corr._prn.setFlags(1);// I/NAV
    }
    corrList.push_back(corr);
  }
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_URA::t_URA() {
  _updateInt  = 0;
  _iod        = 0;
  _ura        = 0.0;
}

//
////////////////////////////////////////////////////////////////////////////
void t_URA::writeEpoch(ostream* out, const QList<t_URA>& corrList) {
  if (!out || corrList.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime;
  QListIterator<t_URA> it(corrList);
  while (it.hasNext()) {
    const t_URA& corr = it.next();
    if (!epoTime.valid()) {
      epoTime = corr._time;
      *out << "> URA " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << " "
          <<  corr._updateInt <<  " "
           << corrList.size() << ' ' << corr._staID << endl;
    }
    *out << corr._prn.toString() << ' ' << setw(11) << corr._iod << ' '
         << setw(10) << setprecision(4) << corr._ura << endl;
  }
  out->flush();
}

//
////////////////////////////////////////////////////////////////////////////
void t_URA::readEpoch(const string& epoLine, istream& inStream, QList<t_URA>& corrList) {
  bncTime      epoTime;
  unsigned int updateInt;
  int          numCorr;
  string       staID;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, updateInt, numCorr, staID) != t_corrSSR::URA) {
    return;
  }
  for (int ii = 0; ii < numCorr; ii++) {
    t_URA corr;
    corr._time      = epoTime;
    corr._updateInt = updateInt;
    corr._staID     = staID;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());

    in >> corr._prn >> corr._iod >> corr._ura;

    corrList.push_back(corr);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_satCodeBias::writeEpoch(ostream* out, const QList<t_satCodeBias>& biasList) {
  if (!out || biasList.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime;
  QListIterator<t_satCodeBias> it(biasList);
  while (it.hasNext()) {
    const t_satCodeBias& satCodeBias = it.next();
    if (!epoTime.valid()) {
      epoTime = satCodeBias._time;
      *out << "> CODE_BIAS " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << " "
           << satCodeBias._updateInt <<  " "
           << biasList.size() << ' ' << satCodeBias._staID << endl;
    }
    if (!satCodeBias._bias.size()) {
      continue;
    }
    *out << satCodeBias._prn.toString() << "   " << setw(2) << satCodeBias._bias.size();
    for (unsigned ii = 0; ii < satCodeBias._bias.size(); ii++) {
      const t_frqCodeBias& frqCodeBias = satCodeBias._bias[ii];
      *out << "   " << frqCodeBias._rnxType2ch << ' '
           << setw(10) << setprecision(4) << frqCodeBias._value;
    }
    *out << endl;
  }
  out->flush();
}

//
////////////////////////////////////////////////////////////////////////////
void t_satCodeBias::readEpoch(const string& epoLine, istream& inStream, QList<t_satCodeBias>& biasList) {
  bncTime      epoTime;
  unsigned int updateInt;
  int          numSat;
  string       staID;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, updateInt, numSat, staID) != t_corrSSR::codeBias) {
    return;
  }
  for (int ii = 0; ii < numSat; ii++) {
    t_satCodeBias satCodeBias;
    satCodeBias._time      = epoTime;
    satCodeBias._updateInt = updateInt;
    satCodeBias._staID     = staID;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());

    int numBias;
    in >> satCodeBias._prn >> numBias;

    while (in.good()) {
      t_frqCodeBias frqCodeBias;
      in >> frqCodeBias._rnxType2ch >> frqCodeBias._value;
      if (!frqCodeBias._rnxType2ch.empty()) {
        satCodeBias._bias.push_back(frqCodeBias);
      }
    }

    biasList.push_back(satCodeBias);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_satPhaseBias::writeEpoch(ostream* out, const QList<t_satPhaseBias>& biasList) {
  if (!out || biasList.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime;
  QListIterator<t_satPhaseBias> it(biasList);
  while (it.hasNext()) {
    const t_satPhaseBias& satPhaseBias = it.next();
    if (!epoTime.valid()) {
      epoTime = satPhaseBias._time;
      *out << "> PHASE_BIAS " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << " "
           << satPhaseBias._updateInt <<  " "
           << biasList.size() << ' ' << satPhaseBias._staID <<  endl;
      *out << " " << satPhaseBias._dispBiasConstistInd << "   "
           << satPhaseBias._MWConsistInd << endl;
    }
    *out << satPhaseBias._prn.toString() << ' '
         << setw(12) << setprecision(8) << satPhaseBias._yaw  * 180.0 / M_PI << ' '
         << setw(12) << setprecision(8) << satPhaseBias._yawRate  * 180.0 / M_PI<< "   "
         << setw(2) << satPhaseBias._bias.size();
    for (unsigned ii = 0; ii < satPhaseBias._bias.size(); ii++) {
      const t_frqPhaseBias& frqPhaseBias = satPhaseBias._bias[ii];
      *out << "   " << frqPhaseBias._rnxType2ch << ' '
           << setw(10) << setprecision(4) << frqPhaseBias._value << ' '
           << setw(3) << frqPhaseBias._fixIndicator << ' '
           << setw(3) << frqPhaseBias._fixWideLaneIndicator << ' '
           << setw(3) << frqPhaseBias._jumpCounter;
    }
    *out << endl;
  }
  out->flush();
}

//
////////////////////////////////////////////////////////////////////////////
void t_satPhaseBias::readEpoch(const string& epoLine, istream& inStream, QList<t_satPhaseBias>& biasList) {
  bncTime      epoTime;
  unsigned int updateInt;
  int          numSat;
  string       staID;
  unsigned int dispInd;
  unsigned int mwInd;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, updateInt, numSat, staID) != t_corrSSR::phaseBias) {
    return;
  }
  for (int ii = 0; ii <= numSat; ii++) {
    t_satPhaseBias satPhaseBias;
    satPhaseBias._time      = epoTime;
    satPhaseBias._updateInt = updateInt;
    satPhaseBias._staID     = staID;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());

    if (ii == 0) {
      in >> dispInd >> mwInd;
      continue;
    }
    satPhaseBias._dispBiasConstistInd = dispInd;
    satPhaseBias._MWConsistInd = mwInd;

    int numBias;
    double yawDeg, yawDegRate;
    in >> satPhaseBias._prn  >> yawDeg >> yawDegRate  >> numBias;
    satPhaseBias._yaw = yawDeg * M_PI / 180.0;
    satPhaseBias._yawRate = yawDegRate * M_PI / 180.0;

    while (in.good()) {
      t_frqPhaseBias frqPhaseBias;
      in >> frqPhaseBias._rnxType2ch >> frqPhaseBias._value
         >> frqPhaseBias._fixIndicator >> frqPhaseBias._fixWideLaneIndicator
         >> frqPhaseBias._jumpCounter;
      if (!frqPhaseBias._rnxType2ch.empty()) {
        satPhaseBias._bias.push_back(frqPhaseBias);
      }
    }

    biasList.push_back(satPhaseBias);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_vTec::write(ostream* out, const t_vTec& vTec) {
  if (!out || vTec._layers.size() == 0) {
    return;
  }
  out->setf(ios::fixed);
  bncTime epoTime = vTec._time;
  *out << "> VTEC " << epoTime.datestr(' ') << ' ' << epoTime.timestr(1,' ') << " "
       << vTec._updateInt <<  " "
       << vTec._layers.size() << ' ' << vTec._staID << endl;
  for (unsigned ii = 0; ii < vTec._layers.size(); ii++) {
    const t_vTecLayer& layer = vTec._layers[ii];
    *out << setw(2)  << ii+1 << ' '
         << setw(2)  << layer._C.Nrows()-1 << ' '
         << setw(2)  << layer._C.Ncols()-1 << ' '
         << setw(10) << setprecision(1) << layer._height << endl
         << setw(10) << setprecision(4) << layer._C
         << setw(10) << setprecision(4) << layer._S;
  }
  out->flush();
}

//
////////////////////////////////////////////////////////////////////////////
void t_vTec::read(const string& epoLine, istream& inStream, t_vTec& vTec) {
  bncTime      epoTime;
  unsigned int updateInt;
  int          numLayers;
  string       staID;
  if (t_corrSSR::readEpoLine(epoLine, epoTime, updateInt, numLayers, staID) != t_corrSSR::vTec) {
    return;
  }
  if (numLayers <= 0) {
    return;
  }
  vTec._time      = epoTime;
  vTec._updateInt = updateInt;
  vTec._staID     = staID;
  for (int ii = 0; ii < numLayers; ii++) {
    t_vTecLayer layer;

    string line;
    getline(inStream, line);
    istringstream in(line.c_str());

    int dummy, maxDeg, maxOrd;
    in >> dummy >> maxDeg >> maxOrd >> layer._height;

    layer._C.ReSize(maxDeg+1, maxOrd+1);
    layer._S.ReSize(maxDeg+1, maxOrd+1);

    for (int iDeg = 0; iDeg <= maxDeg; iDeg++) {
      for (int iOrd = 0; iOrd <= maxOrd; iOrd++) {
        inStream >> layer._C[iDeg][iOrd];
      }
    }
    for (int iDeg = 0; iDeg <= maxDeg; iDeg++) {
      for (int iOrd = 0; iOrd <= maxOrd; iOrd++) {
        inStream >> layer._S[iDeg][iOrd];
      }
    }

    vTec._layers.push_back(layer);
  }
}

//
////////////////////////////////////////////////////////////////////////////
t_corrSSR::e_type t_corrSSR::readEpoLine(const string& line, bncTime& epoTime,
                                         unsigned int& updateInt, int& numEntries,
                                         string& staID) {

  istringstream inLine(line.c_str());

  char   epoChar;
  string typeString;
  int    year, month, day, hour, min;
  double sec;

  inLine >> epoChar >> typeString
         >> year >> month >> day >> hour >> min >> sec >> updateInt >> numEntries >> staID;

  if (epoChar == '>') {
    epoTime.set(year, month, day, hour, min, sec);
    if      (typeString == "CLOCK") {
      return clkCorr;
    }
    else if (typeString == "ORBIT") {
      return orbCorr;
    }
    else if (typeString == "CODE_BIAS") {
      return codeBias;
    }
    else if (typeString == "PHASE_BIAS") {
      return phaseBias;
    }
    else if (typeString == "VTEC") {
      return vTec;
    }
    else if (typeString == "URA") {
      return URA;
    }
  }

  return unknown;
}
