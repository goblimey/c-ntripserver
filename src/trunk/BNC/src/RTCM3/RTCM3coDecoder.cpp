// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      RTCM3coDecoder
 *
 * Purpose:    RTCM3 Clock Orbit Decoder
 *
 * Author:     L. Mervart
 *
 * Created:    05-May-2008
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#include "RTCM3coDecoder.h"
#include "bncutils.h"
#include "bncrinex.h"
#include "bnccore.h"
#include "bncsettings.h"
#include "bnctime.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
RTCM3coDecoder::RTCM3coDecoder(const QString& staID) {

  _staID = staID;

  // File Output
  // -----------
  bncSettings settings;
  QString path = settings.value("corrPath").toString();
  if (!path.isEmpty()) {
    expandEnvVar(path);
    if ( path.length() > 0 && path[path.length()-1] != QDir::separator() ) {
      path += QDir::separator();
    }
    _fileNameSkl = path + staID;
  }
  _out = 0;

  connect(this, SIGNAL(newOrbCorrections(QList<t_orbCorr>)),
          BNC_CORE, SLOT(slotNewOrbCorrections(QList<t_orbCorr>)));

  connect(this, SIGNAL(newClkCorrections(QList<t_clkCorr>)),
          BNC_CORE, SLOT(slotNewClkCorrections(QList<t_clkCorr>)));

  connect(this, SIGNAL(newCodeBiases(QList<t_satCodeBias>)),
          BNC_CORE, SLOT(slotNewCodeBiases(QList<t_satCodeBias>)));

  connect(this, SIGNAL(newPhaseBiases(QList<t_satPhaseBias>)),
          BNC_CORE, SLOT(slotNewPhaseBiases(QList<t_satPhaseBias>)));

  connect(this, SIGNAL(newTec(t_vTec)),
          BNC_CORE, SLOT(slotNewTec(t_vTec)));

  connect(this, SIGNAL(providerIDChanged(QString)),
          BNC_CORE, SIGNAL(providerIDChanged(QString)));

  connect(this, SIGNAL(newMessage(QByteArray,bool)),
          BNC_CORE, SLOT(slotMessage(const QByteArray,bool)));

  reset();

  _providerID[0] = -1;
  _providerID[1] = -1;
  _providerID[2] = -1;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
RTCM3coDecoder::~RTCM3coDecoder() {
  delete _out;
  _IODs.clear();
  _orbCorrections.clear();
  _clkCorrections.clear();
  _lastClkCorrections.clear();
  _codeBiases.clear();
  _phaseBiases.clear();
  _vTecMap.clear();
}

//
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::reset() {
  memset(&_clkOrb,    0, sizeof(_clkOrb));
  memset(&_codeBias,  0, sizeof(_codeBias));
  memset(&_phaseBias, 0, sizeof(_phaseBias));
  memset(&_vTEC,      0, sizeof(_vTEC));
}

// Reopen Output File
////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::reopen() {

  if (!_fileNameSkl.isEmpty()) {

    bncSettings settings;

    QDateTime datTim = currentDateAndTimeGPS();

    QString hlpStr = bncRinex::nextEpochStr(datTim,
                                      settings.value("corrIntr").toString(), false);

    QString fileNameHlp = _fileNameSkl
      + QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0'))
      + hlpStr + datTim.toString(".yyC");

    if (_fileName == fileNameHlp) {
      return;
    }
    else {
      _fileName = fileNameHlp;
    }

    delete _out;
    if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
      _out = new ofstream( _fileName.toLatin1().data(), ios_base::out | ios_base::app );
    }
    else {
      _out = new ofstream( _fileName.toLatin1().data() );
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
t_irc RTCM3coDecoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {

  errmsg.clear();

  _buffer.append(QByteArray(buffer,bufLen));

  t_irc retCode = failure;

  while(_buffer.size()) {

    struct ClockOrbit clkOrbSav;
    struct CodeBias   codeBiasSav;
    struct PhaseBias  phaseBiasSav;
    struct VTEC       vTECSav;
    memcpy(&clkOrbSav,    &_clkOrb,    sizeof(clkOrbSav)); // save state
    memcpy(&codeBiasSav,  &_codeBias,  sizeof(codeBiasSav));
    memcpy(&phaseBiasSav, &_phaseBias, sizeof(phaseBiasSav));
    memcpy(&vTECSav,      &_vTEC,      sizeof(vTECSav));

    int bytesused = 0;
    GCOB_RETURN irc = GetSSR(&_clkOrb, &_codeBias, &_vTEC, &_phaseBias,
                             _buffer.data(), _buffer.size(), &bytesused);

    if      (irc <= -30) { // not enough data - restore state and exit loop
      memcpy(&_clkOrb,    &clkOrbSav,    sizeof(clkOrbSav));
      memcpy(&_codeBias,  &codeBiasSav,  sizeof(codeBiasSav));
      memcpy(&_phaseBias, &phaseBiasSav, sizeof(phaseBiasSav));
      memcpy(&_vTEC,      &vTECSav,      sizeof(vTECSav));
      break;
    }

    else if (irc < 0) {    // error  - skip 1 byte and retry
      reset();
      _buffer = _buffer.mid(bytesused ? bytesused : 1);
    }

    else {                 // OK or MESSAGEFOLLOWS
      _buffer = _buffer.mid(bytesused);

      if (irc == GCOBR_OK || irc == GCOBR_MESSAGEFOLLOWS ) {

        setEpochTime(); // sets _lastTime

        if (_lastTime.valid()) {
          reopen();
          checkProviderID();
          sendResults();
          retCode = success;
        }
        else {
          retCode = failure;
        }

        reset();
      }
    }
  }

  return retCode;
}

//
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::sendResults() {

  // Orbit and clock corrections of all satellites
  // ---------------------------------------------
  for (unsigned ii = 0; ii <  CLOCKORBIT_NUMGPS
                            + CLOCKORBIT_NUMGLONASS
                            + CLOCKORBIT_NUMGALILEO
                            + CLOCKORBIT_NUMQZSS
                            + CLOCKORBIT_NUMSBAS
                            + _clkOrb.NumberOfSat[CLOCKORBIT_SATBDS];
    ii++) {
    char sysCh = ' ';
    int flag = 0;
    if      (ii < _clkOrb.NumberOfSat[CLOCKORBIT_SATGPS]) {
      sysCh = 'G';
    }
    else if (ii >= CLOCKORBIT_OFFSETGLONASS &&
        ii < CLOCKORBIT_OFFSETGLONASS + _clkOrb.NumberOfSat[CLOCKORBIT_SATGLONASS]) {
      sysCh = 'R';
    }
    else if (ii >= CLOCKORBIT_OFFSETGALILEO &&
        ii < CLOCKORBIT_OFFSETGALILEO + _clkOrb.NumberOfSat[CLOCKORBIT_SATGALILEO]) {
      sysCh = 'E';
      flag = 1; // I/NAV clock has been chosen as reference clock for Galileo SSR corrections
    }
    else if (ii >= CLOCKORBIT_OFFSETQZSS &&
        ii < CLOCKORBIT_OFFSETQZSS + _clkOrb.NumberOfSat[CLOCKORBIT_SATQZSS]) {
      sysCh = 'J';
    }
    else if (ii >= CLOCKORBIT_OFFSETSBAS &&
        ii < CLOCKORBIT_OFFSETSBAS + _clkOrb.NumberOfSat[CLOCKORBIT_SATSBAS]) {
      sysCh = 'S';
    }
    else if (ii >= CLOCKORBIT_OFFSETBDS &&
        ii < CLOCKORBIT_OFFSETBDS + _clkOrb.NumberOfSat[CLOCKORBIT_SATBDS]) {
      sysCh = 'C';
    }
    else {
      continue;
    }
    // Orbit correction
    // ----------------
    if ( _clkOrb.messageType == COTYPE_GPSCOMBINED     ||
         _clkOrb.messageType == COTYPE_GLONASSCOMBINED ||
         _clkOrb.messageType == COTYPE_GALILEOCOMBINED ||
         _clkOrb.messageType == COTYPE_QZSSCOMBINED ||
         _clkOrb.messageType == COTYPE_SBASCOMBINED ||
         _clkOrb.messageType == COTYPE_BDSCOMBINED ||
         _clkOrb.messageType == COTYPE_GPSORBIT ||
         _clkOrb.messageType == COTYPE_GLONASSORBIT ||
         _clkOrb.messageType == COTYPE_GALILEOORBIT ||
         _clkOrb.messageType == COTYPE_QZSSORBIT ||
         _clkOrb.messageType == COTYPE_SBASORBIT ||
         _clkOrb.messageType == COTYPE_BDSORBIT ) {

      t_orbCorr orbCorr;
      orbCorr._prn.set(sysCh, _clkOrb.Sat[ii].ID, flag);
      orbCorr._staID     = _staID.toStdString();
      orbCorr._iod       = _clkOrb.Sat[ii].IOD;
      orbCorr._time      = _lastTime;
      orbCorr._updateInt = _clkOrb.UpdateInterval;
      orbCorr._system    = sysCh;
      orbCorr._xr[0]     = _clkOrb.Sat[ii].Orbit.DeltaRadial;
      orbCorr._xr[1]     = _clkOrb.Sat[ii].Orbit.DeltaAlongTrack;
      orbCorr._xr[2]     = _clkOrb.Sat[ii].Orbit.DeltaCrossTrack;
      orbCorr._dotXr[0]  = _clkOrb.Sat[ii].Orbit.DotDeltaRadial;
      orbCorr._dotXr[1]  = _clkOrb.Sat[ii].Orbit.DotDeltaAlongTrack;
      orbCorr._dotXr[2]  = _clkOrb.Sat[ii].Orbit.DotDeltaCrossTrack;

      _orbCorrections[_lastTime].append(orbCorr);

      _IODs[orbCorr._prn] = _clkOrb.Sat[ii].IOD;
    }

    // Clock Corrections
    // -----------------
    if ( _clkOrb.messageType == COTYPE_GPSCOMBINED     ||
         _clkOrb.messageType == COTYPE_GLONASSCOMBINED ||
         _clkOrb.messageType == COTYPE_GALILEOCOMBINED ||
         _clkOrb.messageType == COTYPE_QZSSCOMBINED ||
         _clkOrb.messageType == COTYPE_SBASCOMBINED ||
         _clkOrb.messageType == COTYPE_BDSCOMBINED ||
         _clkOrb.messageType == COTYPE_GPSCLOCK ||
         _clkOrb.messageType == COTYPE_GLONASSCLOCK ||
         _clkOrb.messageType == COTYPE_GALILEOCLOCK ||
         _clkOrb.messageType == COTYPE_QZSSCLOCK ||
         _clkOrb.messageType == COTYPE_SBASCLOCK ||
         _clkOrb.messageType == COTYPE_BDSCLOCK) {

      t_clkCorr clkCorr;
      clkCorr._prn.set(sysCh, _clkOrb.Sat[ii].ID, flag);
      clkCorr._staID      = _staID.toStdString();
      clkCorr._time       = _lastTime;
      clkCorr._updateInt  = _clkOrb.UpdateInterval;
      clkCorr._dClk       = _clkOrb.Sat[ii].Clock.DeltaA0 / t_CST::c;
      clkCorr._dotDClk    = _clkOrb.Sat[ii].Clock.DeltaA1 / t_CST::c;
      clkCorr._dotDotDClk = _clkOrb.Sat[ii].Clock.DeltaA2 / t_CST::c;

      _lastClkCorrections[clkCorr._prn] = clkCorr;

      if (_IODs.contains(clkCorr._prn)) {
        clkCorr._iod = _IODs[clkCorr._prn];
        _clkCorrections[_lastTime].append(clkCorr);
      }
    }

    // High-Resolution Clocks
    // ----------------------
    if ( _clkOrb.messageType == COTYPE_GPSHR     ||
         _clkOrb.messageType == COTYPE_GLONASSHR ||
         _clkOrb.messageType == COTYPE_GALILEOHR ||
         _clkOrb.messageType == COTYPE_QZSSHR ||
         _clkOrb.messageType == COTYPE_SBASHR ||
         _clkOrb.messageType == COTYPE_BDSHR) {
      t_prn prn(sysCh, _clkOrb.Sat[ii].ID, flag);
      if (_lastClkCorrections.contains(prn)) {
        t_clkCorr clkCorr;
        clkCorr            = _lastClkCorrections[prn];
        clkCorr._time      = _lastTime;
        clkCorr._updateInt = _clkOrb.UpdateInterval;
        clkCorr._dClk     += _clkOrb.Sat[ii].hrclock / t_CST::c;
        if (_IODs.contains(clkCorr._prn)) {
          clkCorr._iod = _IODs[clkCorr._prn];
          _clkCorrections[_lastTime].push_back(clkCorr);
        }
      }
    }
  }

  // Code Biases
  // -----------
  for (unsigned ii = 0; ii <  CLOCKORBIT_NUMGPS
                            + CLOCKORBIT_NUMGLONASS
                            + CLOCKORBIT_NUMGALILEO
                            + CLOCKORBIT_NUMQZSS
                            + CLOCKORBIT_NUMSBAS
                            + _codeBias.NumberOfSat[CLOCKORBIT_SATBDS];
    ii++) {
    char sysCh = ' ';
    if      (ii < _codeBias.NumberOfSat[CLOCKORBIT_SATGPS]) {
      sysCh = 'G';
    }
    else if (ii >= CLOCKORBIT_OFFSETGLONASS &&
        ii < CLOCKORBIT_OFFSETGLONASS + _codeBias.NumberOfSat[CLOCKORBIT_SATGLONASS]) {
      sysCh = 'R';
    }
    else if (ii >= CLOCKORBIT_OFFSETGALILEO &&
        ii < CLOCKORBIT_OFFSETGALILEO + _codeBias.NumberOfSat[CLOCKORBIT_SATGALILEO]) {
      sysCh = 'E';
    }
    else if (ii >= CLOCKORBIT_OFFSETQZSS &&
        ii < CLOCKORBIT_OFFSETQZSS + _codeBias.NumberOfSat[CLOCKORBIT_SATQZSS]) {
      sysCh = 'J';
    }
    else if (ii >= CLOCKORBIT_OFFSETSBAS &&
        ii < CLOCKORBIT_OFFSETSBAS + _codeBias.NumberOfSat[CLOCKORBIT_SATSBAS]) {
      sysCh = 'S';
    }
    else if (ii >= CLOCKORBIT_OFFSETBDS &&
        ii < CLOCKORBIT_OFFSETBDS + _codeBias.NumberOfSat[CLOCKORBIT_SATBDS]) {
      sysCh = 'C';
    }
    else {
      continue;
    }
    t_satCodeBias satCodeBias;
    satCodeBias._prn.set(sysCh, _codeBias.Sat[ii].ID);
    satCodeBias._staID     = _staID.toStdString();
    satCodeBias._time      = _lastTime;
    satCodeBias._updateInt = _codeBias.UpdateInterval;
    for (unsigned jj = 0; jj < _codeBias.Sat[ii].NumberOfCodeBiases; jj++) {
      const CodeBias::BiasSat::CodeBiasEntry& biasEntry = _codeBias.Sat[ii].Biases[jj];
      t_frqCodeBias frqCodeBias;
      frqCodeBias._rnxType2ch.assign(codeTypeToRnxType(sysCh, biasEntry.Type));
      frqCodeBias._value      = biasEntry.Bias;
      if (!frqCodeBias._rnxType2ch.empty()) {
        satCodeBias._bias.push_back(frqCodeBias);
      }
    }
    _codeBiases[_lastTime].append(satCodeBias);
  }

  // Phase Biases
  // -----------
  for (unsigned ii = 0; ii <  CLOCKORBIT_NUMGPS
                            + CLOCKORBIT_NUMGLONASS
                            + CLOCKORBIT_NUMGALILEO
                            + CLOCKORBIT_NUMQZSS
                            + CLOCKORBIT_NUMSBAS
                            + _phaseBias.NumberOfSat[CLOCKORBIT_SATBDS];
    ii++) {
    char sysCh = ' ';
    if      (ii < _phaseBias.NumberOfSat[CLOCKORBIT_SATGPS]) {
      sysCh = 'G';
    }
    else if (ii >= CLOCKORBIT_OFFSETGLONASS &&
        ii < CLOCKORBIT_OFFSETGLONASS + _phaseBias.NumberOfSat[CLOCKORBIT_SATGLONASS]) {
      sysCh = 'R';
    }
    else if (ii >= CLOCKORBIT_OFFSETGALILEO &&
        ii < CLOCKORBIT_OFFSETGALILEO + _phaseBias.NumberOfSat[CLOCKORBIT_SATGALILEO]) {
      sysCh = 'E';
    }
    else if (ii >= CLOCKORBIT_OFFSETQZSS &&
        ii < CLOCKORBIT_OFFSETQZSS + _phaseBias.NumberOfSat[CLOCKORBIT_SATQZSS]) {
      sysCh = 'J';
    }
    else if (ii >= CLOCKORBIT_OFFSETSBAS &&
        ii < CLOCKORBIT_OFFSETSBAS + _phaseBias.NumberOfSat[CLOCKORBIT_SATSBAS]) {
      sysCh = 'S';
    }
    else if (ii >= CLOCKORBIT_OFFSETBDS &&
        ii < CLOCKORBIT_OFFSETBDS + _phaseBias.NumberOfSat[CLOCKORBIT_SATBDS]) {
      sysCh = 'C';
    }
    else {
      continue;
    }
    t_satPhaseBias satPhaseBias;
    satPhaseBias._prn.set(sysCh, _phaseBias.Sat[ii].ID);
    satPhaseBias._staID      = _staID.toStdString();
    satPhaseBias._time       = _lastTime;
    satPhaseBias._updateInt  = _phaseBias.UpdateInterval;
    satPhaseBias._dispBiasConstistInd = _phaseBias.DispersiveBiasConsistencyIndicator;
    satPhaseBias._MWConsistInd        = _phaseBias.MWConsistencyIndicator;
    satPhaseBias._yaw     = _phaseBias.Sat[ii].YawAngle;
    satPhaseBias._yawRate = _phaseBias.Sat[ii].YawRate;
    for (unsigned jj = 0; jj < _phaseBias.Sat[ii].NumberOfPhaseBiases; jj++) {
      const PhaseBias::PhaseBiasSat::PhaseBiasEntry& biasEntry = _phaseBias.Sat[ii].Biases[jj];
      t_frqPhaseBias frqPhaseBias;
      frqPhaseBias._rnxType2ch.assign(codeTypeToRnxType(sysCh, biasEntry.Type));
      frqPhaseBias._value                = biasEntry.Bias;
      frqPhaseBias._fixIndicator         = biasEntry.SignalIntegerIndicator;
      frqPhaseBias._fixWideLaneIndicator = biasEntry.SignalsWideLaneIntegerIndicator;
      frqPhaseBias._jumpCounter          = biasEntry.SignalDiscontinuityCounter;
      if (!frqPhaseBias._rnxType2ch.empty()) {
        satPhaseBias._bias.push_back(frqPhaseBias);
      }
    }
    _phaseBiases[_lastTime].append(satPhaseBias);
  }

  // Ionospheric Model
  // -----------------
  if (_vTEC.NumLayers > 0) {
    _vTecMap[_lastTime]._time  = _lastTime;
    _vTecMap[_lastTime]._updateInt =  _vTEC.UpdateInterval;
    _vTecMap[_lastTime]._staID = _staID.toStdString();
    for (unsigned ii = 0; ii < _vTEC.NumLayers; ii++) {
      const VTEC::IonoLayers& ionoLayer = _vTEC.Layers[ii];
      t_vTecLayer layer;
      layer._height = ionoLayer.Height;
      layer._C.ReSize(ionoLayer.Degree+1, ionoLayer.Order+1);
      layer._S.ReSize(ionoLayer.Degree+1, ionoLayer.Order+1);
      for (unsigned iDeg = 0; iDeg <= ionoLayer.Degree; iDeg++) {
        for (unsigned iOrd = 0; iOrd <= ionoLayer.Order; iOrd++) {
          layer._C[iDeg][iOrd] = ionoLayer.Cosinus[iDeg][iOrd];
          layer._S[iDeg][iOrd] = ionoLayer.Sinus[iDeg][iOrd];
        }
      }
      _vTecMap[_lastTime]._layers.push_back(layer);
    }
  }

  // Dump all older epochs
  // ---------------------
  QMutableMapIterator<bncTime, QList<t_orbCorr> > itOrb(_orbCorrections);
  while (itOrb.hasNext()) {
    itOrb.next();
    if (itOrb.key() < _lastTime) {
      emit newOrbCorrections(itOrb.value());
      t_orbCorr::writeEpoch(_out, itOrb.value());
      itOrb.remove();
    }
  }
  QMutableMapIterator<bncTime, QList<t_clkCorr> > itClk(_clkCorrections);
  while (itClk.hasNext()) {
    itClk.next();
    if (itClk.key() < _lastTime) {
      emit newClkCorrections(itClk.value());
      t_clkCorr::writeEpoch(_out, itClk.value());
      itClk.remove();
    }
  }
  QMutableMapIterator<bncTime, QList<t_satCodeBias> > itCB(_codeBiases);
  while (itCB.hasNext()) {
    itCB.next();
    if (itCB.key() < _lastTime) {
      emit newCodeBiases(itCB.value());
      t_satCodeBias::writeEpoch(_out, itCB.value());
      itCB.remove();
    }
  }
  QMutableMapIterator<bncTime, QList<t_satPhaseBias> > itPB(_phaseBiases);
  while (itPB.hasNext()) {
    itPB.next();
    if (itPB.key() < _lastTime) {
      emit newPhaseBiases(itPB.value());
      t_satPhaseBias::writeEpoch(_out, itPB.value());
      itPB.remove();
    }
  }
  QMutableMapIterator<bncTime, t_vTec> itTec(_vTecMap);
  while (itTec.hasNext()) {
    itTec.next();
    if (itTec.key() < _lastTime) {
      emit newTec(itTec.value());
      t_vTec::write(_out, itTec.value());
      itTec.remove();
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::checkProviderID() {

  if (_clkOrb.SSRProviderID == 0 && _clkOrb.SSRSolutionID == 0 && _clkOrb.SSRIOD == 0) {
    return;
  }

  int newProviderID[3];
  newProviderID[0] = _clkOrb.SSRProviderID;
  newProviderID[1] = _clkOrb.SSRSolutionID;
  newProviderID[2] = _clkOrb.SSRIOD;

  bool alreadySet = false;
  bool different  = false;

  for (unsigned ii = 0; ii < 3; ii++) {
    if (_providerID[ii] != -1) {
      alreadySet = true;
    }
    if (_providerID[ii] != newProviderID[ii]) {
      different = true;
    }
    _providerID[ii] = newProviderID[ii];
  }

  if (alreadySet && different) {
    emit newMessage("RTCM3coDecoder: Provider Changed: " + _staID.toLatin1(), true);
    emit providerIDChanged(_staID);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void RTCM3coDecoder::setEpochTime() {

  _lastTime.reset();

  double epoSecGPS  = -1.0;
  double epoSecGlo  = -1.0;
  double epoSecGal  = -1.0;
  double epoSecQzss = -1.0;
  double epoSecSbas = -1.0;
  double epoSecBds  = -1.0;
  if      (_clkOrb.NumberOfSat[CLOCKORBIT_SATGPS] > 0) {
    epoSecGPS = _clkOrb.EpochTime[CLOCKORBIT_SATGPS];        // 0 .. 604799 s
  }
  else if (_codeBias.NumberOfSat[CLOCKORBIT_SATGPS] > 0) {
    epoSecGPS = _codeBias.EpochTime[CLOCKORBIT_SATGPS];      // 0 .. 604799 s
  }
  else if (_phaseBias.NumberOfSat[CLOCKORBIT_SATGPS] > 0) {
    epoSecGPS = _phaseBias.EpochTime[CLOCKORBIT_SATGPS];     // 0 .. 604799 s
  }
  else if (_vTEC.NumLayers > 0) {
    epoSecGPS = _vTEC.EpochTime;                             // 0 .. 604799 s
  }
  else if (_clkOrb.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0) {
    epoSecGlo = _clkOrb.EpochTime[CLOCKORBIT_SATGLONASS];    // 0 .. 86399 s
  }
  else if (_codeBias.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0) {
    epoSecGlo = _codeBias.EpochTime[CLOCKORBIT_SATGLONASS];  // 0 .. 86399 s
  }
  else if (_phaseBias.NumberOfSat[CLOCKORBIT_SATGLONASS] > 0) {
    epoSecGlo = _phaseBias.EpochTime[CLOCKORBIT_SATGLONASS]; // 0 .. 86399 s
  }
  else if (_clkOrb.NumberOfSat[CLOCKORBIT_SATGALILEO] > 0) {
    epoSecGal = _clkOrb.EpochTime[CLOCKORBIT_SATGALILEO];
  }
  else if (_codeBias.NumberOfSat[CLOCKORBIT_SATGALILEO] > 0) {
    epoSecGal = _codeBias.EpochTime[CLOCKORBIT_SATGALILEO];
  }
  else if (_phaseBias.NumberOfSat[CLOCKORBIT_SATGALILEO] > 0) {
    epoSecGal = _phaseBias.EpochTime[CLOCKORBIT_SATGALILEO];
  }
  else if (_clkOrb.NumberOfSat[CLOCKORBIT_SATQZSS] > 0) {
    epoSecQzss = _clkOrb.EpochTime[CLOCKORBIT_SATQZSS];
  }
  else if (_codeBias.NumberOfSat[CLOCKORBIT_SATQZSS] > 0) {
    epoSecQzss = _codeBias.EpochTime[CLOCKORBIT_SATQZSS];
  }
  else if (_phaseBias.NumberOfSat[CLOCKORBIT_SATQZSS] > 0) {
    epoSecQzss = _phaseBias.EpochTime[CLOCKORBIT_SATQZSS];
  }
  else if (_clkOrb.NumberOfSat[CLOCKORBIT_SATSBAS] > 0) {
    epoSecSbas = _clkOrb.EpochTime[CLOCKORBIT_SATSBAS];
  }
  else if (_codeBias.NumberOfSat[CLOCKORBIT_SATSBAS] > 0) {
    epoSecSbas = _codeBias.EpochTime[CLOCKORBIT_SATSBAS];
  }
  else if (_phaseBias.NumberOfSat[CLOCKORBIT_SATSBAS] > 0) {
    epoSecSbas = _phaseBias.EpochTime[CLOCKORBIT_SATSBAS];
  }
  else if (_clkOrb.NumberOfSat[CLOCKORBIT_SATBDS] > 0) {
    epoSecBds = _clkOrb.EpochTime[CLOCKORBIT_SATBDS];
  }
  else if (_codeBias.NumberOfSat[CLOCKORBIT_SATBDS] > 0) {
    epoSecBds = _codeBias.EpochTime[CLOCKORBIT_SATBDS];
  }
  else if (_phaseBias.NumberOfSat[CLOCKORBIT_SATBDS] > 0) {
    epoSecBds = _phaseBias.EpochTime[CLOCKORBIT_SATBDS];
  }

  // Retrieve current time
  // ---------------------
  int    currentWeek = 0;
  double currentSec  = 0.0;
  currentGPSWeeks(currentWeek, currentSec);
  bncTime currentTime(currentWeek, currentSec);

  // Set _lastTime close to currentTime
  // ----------------------------------
  if      (epoSecGPS != -1) {
    _lastTime.set(currentWeek, epoSecGPS);
  }
  else if (epoSecGlo != -1) {
    QDate date = dateAndTimeFromGPSweek(currentTime.gpsw(), currentTime.gpssec()).date();
    epoSecGlo = epoSecGlo - 3 * 3600 + gnumleap(date.year(), date.month(), date.day());
    _lastTime.set(currentWeek, epoSecGlo);
  }
  else if (epoSecGal != -1) {
    _lastTime.set(currentWeek, epoSecGal);
  }
  else if (epoSecQzss != -1) {
    _lastTime.set(currentWeek, epoSecQzss);
  }
  else if (epoSecSbas != -1) {
    _lastTime.set(currentWeek, epoSecSbas);
  }
  else if (epoSecBds != -1) {
    epoSecBds += 14.0;
    if (epoSecBds > 604800.0) {
      epoSecBds -= 7.0*24.0*60.0*60.0;
    }
    _lastTime.set(currentWeek, epoSecBds);
  }

  if (_lastTime.valid()) {
    double maxDiff = 12 * 3600.0;
    while (_lastTime < currentTime - maxDiff) {
      _lastTime = _lastTime + maxDiff;
    }
    while (_lastTime > currentTime + maxDiff) {
      _lastTime = _lastTime - maxDiff;
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
string RTCM3coDecoder::codeTypeToRnxType(char system, CodeType type) const {
  if      (system == 'G') {
    switch (type) {
      case CODETYPEGPS_L1_CA:         return "1C";
      case CODETYPEGPS_L1_P:          return "1P";
      case CODETYPEGPS_L1_Z:          return "1W";
      //case CODETYPEGPS_L1_Y:          return "1Y";
      //case CODETYPEGPS_L1_M:          return "1M";
      case CODETYPEGPS_SEMI_CODELESS: return "2D";
      case CODETYPEGPS_L2_CA:         return "2C";
      case CODETYPEGPS_L2_P:          return "2P";
      case CODETYPEGPS_L2_Z:          return "2W";

      case CODETYPEGPS_L2C_M:         return "2S";
      case CODETYPEGPS_L2C_L:         return "2L";
      case CODETYPEGPS_L2C_ML:        return "2X";

      //case CODETYPEGPS_L2_Y:          return "2Y";
      //case CODETYPEGPS_L2_M:          return "2M";

      case CODETYPEGPS_L5_I:          return "5I";
      case CODETYPEGPS_L5_Q:          return "5Q";
      case CODETYPEGPS_L5_IQ:         return "5X";

      case CODETYPEGPS_L1C_D:         return "1S";
      case CODETYPEGPS_L1C_P:         return "1L";
      case CODETYPEGPS_L1C_DP:        return "1X";
      default: return "";
    }
  }
  else if (system == 'R') {
    switch (type) {
      case CODETYPEGLONASS_L1_CA:     return "1C";
      case CODETYPEGLONASS_L1_P:      return "1P";
      case CODETYPEGLONASS_L2_CA:     return "2C";
      case CODETYPEGLONASS_L2_P:      return "2P";
      case CODETYPEGLONASS_L1a_OCd:   return "4A";
      case CODETYPEGLONASS_L1a_OCp:   return "4B";
      case CODETYPEGLONASS_L1a_OCdp:  return "4X";
      case CODETYPEGLONASS_L2a_CSI:   return "6A";
      case CODETYPEGLONASS_L2a_OCp:   return "6B";
      case CODETYPEGLONASS_L2a_CSIOCp:return "6X";
      case CODETYPEGLONASS_L3_I:      return "3I";
      case CODETYPEGLONASS_L3_Q:      return "3Q";
      case CODETYPEGLONASS_L3_IQ:     return "3X";
      default: return "";
    }
  }
  else if (system == 'E') {
    switch (type) {
      case CODETYPEGALILEO_E1_A:       return "1A";
      case CODETYPEGALILEO_E1_B:       return "1B";
      case CODETYPEGALILEO_E1_C:       return "1C";
      case CODETYPEGALILEO_E1_BC:      return "1X";
      case CODETYPEGALILEO_E1_ABC:     return "1Z";

      case CODETYPEGALILEO_E5A_I:      return "5I";
      case CODETYPEGALILEO_E5A_Q:      return "5Q";
      case CODETYPEGALILEO_E5A_IQ:     return "5X";

      case CODETYPEGALILEO_E5B_I:      return "7I";
      case CODETYPEGALILEO_E5B_Q:      return "7Q";
      case CODETYPEGALILEO_E5B_IQ:     return "7X";

      case CODETYPEGALILEO_E5_I:       return "8I";
      case CODETYPEGALILEO_E5_Q:       return "8Q";
      case CODETYPEGALILEO_E5_IQ:      return "8X";

      case CODETYPEGALILEO_E6_A:       return "6A";
      case CODETYPEGALILEO_E6_B:       return "6B";
      case CODETYPEGALILEO_E6_C:       return "6C";
      case CODETYPEGALILEO_E6_BC:      return "6X";
      case CODETYPEGALILEO_E6_ABC:     return "6Z";
      default: return "";
    }
  }
   else if (system == 'J') {
    switch (type) {
      case CODETYPEQZSS_L1_CA:         return "1C";
      case CODETYPEQZSS_L1C_D:         return "1S";
      case CODETYPEQZSS_L1C_P:         return "1L";

      case CODETYPEQZSS_L2C_M:         return "2S";
      case CODETYPEQZSS_L2C_L:         return "2L";
      case CODETYPEQZSS_L2C_ML:        return "2X";

      case CODETYPEQZSS_L5_I:          return "5I";
      case CODETYPEQZSS_L5_Q:          return "5Q";
      case CODETYPEQZSS_L5_IQ:         return "5X";

      case CODETYPEQZSS_L6_D:          return "6S";
      case CODETYPEQZSS_L6_P:          return "6L";
      case CODETYPEQZSS_L6_DP:         return "6X";

      case CODETYPEQZSS_L1C_DP:        return "1X";
      case CODETYPEQZSS_L1_S:          return "1Z";

      case CODETYPEQZSS_L5_D:          return "5D";
      case CODETYPEQZSS_L5_P:          return "5P";
      case CODETYPEQZSS_L5_DP:         return "5Z";

      case CODETYPEQZSS_L6_E:          return "6E";
      case CODETYPEQZSS_L6_DE:         return "6Z";
      default: return "";
    }
  }
  else if (system == 'S') {
    switch (type) {
      case CODETYPE_SBAS_L1_CA:       return "1C";

      case CODETYPE_SBAS_L5_I:        return "5I";
      case CODETYPE_SBAS_L5_Q:        return "5Q";
      case CODETYPE_SBAS_L5_IQ:       return "5X";
      default: return "";
    }
  }
  else if (system == 'C') {
    switch (type) {
      case CODETYPE_BDS_B1_I:         return "2I";
      case CODETYPE_BDS_B1_Q:         return "2Q";
      case CODETYPE_BDS_B1_IQ:        return "2X";

      case CODETYPE_BDS_B3_I:         return "6I";
      case CODETYPE_BDS_B3_Q:         return "6Q";
      case CODETYPE_BDS_B3_IQ:        return "6X";

      case CODETYPE_BDS_B2_I:         return "7I";
      case CODETYPE_BDS_B2_Q:         return "7Q";
      case CODETYPE_BDS_B2_IQ:        return "7X";

      case CODETYPE_BDS_B1a_D:        return "1D";
      case CODETYPE_BDS_B1a_P:        return "1P";
      case CODETYPE_BDS_B1a_DP:       return "1X";

      case CODETYPE_BDS_B2a_D:        return "5D";
      case CODETYPE_BDS_B2a_P:        return "5P";
      case CODETYPE_BDS_B2a_DP:       return "5X";
      default: return "";
    }
  }
  return "";
};
