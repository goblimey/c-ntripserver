/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncComb
 *
 * Purpose:    Combinations of Orbit/Clock Corrections
 *
 * Author:     L. Mervart
 *
 * Created:    22-Jan-2011
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <newmatio.h>
#include <iomanip>
#include <sstream>

#include "bnccomb.h"
#include "bnccore.h"
#include "upload/bncrtnetdecoder.h"
#include "bncsettings.h"
#include "bncutils.h"
#include "bncsp3.h"
#include "bncantex.h"
#include "t_prn.h"

const double sig0_offAC    = 1000.0;
const double sig0_offACSat =  100.0;
const double sigP_offACSat =   0.01;
const double sig0_clkSat   =  100.0;

const double sigObs        =   0.05;

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncComb::cmbParam::cmbParam(parType type_, int index_, const QString& ac_, const QString& prn_) {

  type   = type_;
  index  = index_;
  AC     = ac_;
  prn    = prn_;
  xx     = 0.0;
  eph    = 0;

  if      (type == offACgps) {
    epoSpec = true;
    sig0    = sig0_offAC;
    sigP    = sig0;
  }
  else if (type == offACglo) {
    epoSpec = true;
    sig0    = sig0_offAC;
    sigP    = sig0;
  }
  else if (type == offACSat) {
    epoSpec = false;
    sig0    = sig0_offACSat;
    sigP    = sigP_offACSat;
  }
  else if (type == clkSat) {
    epoSpec = true;
    sig0    = sig0_clkSat;
    sigP    = sig0;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncComb::cmbParam::~cmbParam() {
}

// Partial
////////////////////////////////////////////////////////////////////////////
double bncComb::cmbParam::partial(const QString& AC_, const QString& prn_) {

  if      (type == offACgps) {
    if (AC == AC_ && prn_[0] == 'G') {
      return 1.0;
    }
  }
  else if (type == offACglo) {
    if (AC == AC_ && prn_[0] == 'R') {
      return 1.0;
    }
  }
  else if (type == offACSat) {
    if (AC == AC_ && prn == prn_) {
      return 1.0;
    }
  }
  else if (type == clkSat) {
    if (prn == prn_) {
      return 1.0;
    }
  }

  return 0.0;
}

//
////////////////////////////////////////////////////////////////////////////
QString bncComb::cmbParam::toString() const {

  QString outStr;

  if      (type == offACgps) {
    outStr = "AC offset GPS " + AC;
  }
  else if (type == offACglo) {
    outStr = "AC offset GLO " + AC;
  }
  else if (type == offACSat) {
    outStr = "Sat Offset " + AC + " " + prn.mid(0,3);
  }
  else if (type == clkSat) {
    outStr = "Clk Corr " + prn.mid(0,3);
  }

  return outStr;
}

// Singleton
////////////////////////////////////////////////////////////////////////////
bncComb* bncComb::instance() {
  static bncComb _bncComb;
  return &_bncComb;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
bncComb::bncComb() : _ephUser(true) {

  bncSettings settings;

  QStringList cmbStreams = settings.value("cmbStreams").toStringList();

  _cmbSampl = settings.value("cmbSampl").toInt();
  if (_cmbSampl <= 0) {
    _cmbSampl = 10;
  }

  _masterMissingEpochs = 0;

  if (cmbStreams.size() >= 1 && !cmbStreams[0].isEmpty()) {
    QListIterator<QString> it(cmbStreams);
    while (it.hasNext()) {
      QStringList hlp = it.next().split(" ");
      cmbAC* newAC = new cmbAC();
      newAC->mountPoint = hlp[0];
      newAC->name       = hlp[1];
      newAC->weight     = hlp[2].toDouble();
      if (_masterOrbitAC.isEmpty()) {
        _masterOrbitAC = newAC->name;
      }
      _ACs.append(newAC);
    }
  }

  _rtnetDecoder = 0;

  connect(this,     SIGNAL(newMessage(QByteArray,bool)),
          BNC_CORE, SLOT(slotMessage(const QByteArray,bool)));

  connect(BNC_CORE, SIGNAL(providerIDChanged(QString)),
          this,     SLOT(slotProviderIDChanged(QString)));

  connect(BNC_CORE, SIGNAL(newOrbCorrections(QList<t_orbCorr>)),
          this,     SLOT(slotNewOrbCorrections(QList<t_orbCorr>)));

  connect(BNC_CORE, SIGNAL(newClkCorrections(QList<t_clkCorr>)),
          this,     SLOT(slotNewClkCorrections(QList<t_clkCorr>)));

  // Combination Method
  // ------------------
  if (settings.value("cmbMethod").toString() == "Single-Epoch") {
    _method = singleEpoch;
  }
  else {
    _method = filter;
  }

  // Use Glonass
  // -----------
  if ( Qt::CheckState(settings.value("cmbUseGlonass").toInt()) == Qt::Checked) {
    _useGlonass = true;
  }
  else {
    _useGlonass = false;
  }

  // Initialize Parameters (model: Clk_Corr = AC_Offset + Sat_Offset + Clk)
  // ----------------------------------------------------------------------
  if (_method == filter) {
    int nextPar = 0;
    QListIterator<cmbAC*> it(_ACs);
    while (it.hasNext()) {
      cmbAC* AC = it.next();
      _params.push_back(new cmbParam(cmbParam::offACgps, ++nextPar, AC->name, ""));
      for (unsigned iGps = 1; iGps <= t_prn::MAXPRN_GPS; iGps++) {
        QString prn = QString("G%1_0").arg(iGps, 2, 10, QChar('0'));
        _params.push_back(new cmbParam(cmbParam::offACSat, ++nextPar,
                                       AC->name, prn));
      }
      if (_useGlonass) {
        _params.push_back(new cmbParam(cmbParam::offACglo, ++nextPar, AC->name, ""));
        for (unsigned iGlo = 1; iGlo <= t_prn::MAXPRN_GLONASS; iGlo++) {
          QString prn = QString("R%1_0").arg(iGlo, 2, 10, QChar('0'));
          _params.push_back(new cmbParam(cmbParam::offACSat, ++nextPar,
                                         AC->name, prn));
        }
      }
    }
    for (unsigned iGps = 1; iGps <= t_prn::MAXPRN_GPS; iGps++) {
      QString prn = QString("G%1_0").arg(iGps, 2, 10, QChar('0'));
      _params.push_back(new cmbParam(cmbParam::clkSat, ++nextPar, "", prn));
    }
    if (_useGlonass) {
      for (unsigned iGlo = 1; iGlo <= t_prn::MAXPRN_GLONASS; iGlo++) {
        QString prn = QString("R%1_0").arg(iGlo, 2, 10, QChar('0'));
        _params.push_back(new cmbParam(cmbParam::clkSat, ++nextPar, "", prn));
      }
    }

    // Initialize Variance-Covariance Matrix
    // -------------------------------------
    _QQ.ReSize(_params.size());
    _QQ = 0.0;
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      cmbParam* pp = _params[iPar-1];
      _QQ(iPar,iPar) = pp->sig0 * pp->sig0;
    }
  }

  // ANTEX File
  // ----------
  _antex = 0;
  QString antexFileName = settings.value("uploadAntexFile").toString();
  if (!antexFileName.isEmpty()) {
    _antex = new bncAntex();
    if (_antex->readFile(antexFileName) != success) {
      emit newMessage("wrong ANTEX file", true);
      delete _antex;
      _antex = 0;
    }
  }

  // Maximal Residuum
  // ----------------
  _MAXRES = settings.value("cmbMaxres").toDouble();
  if (_MAXRES <= 0.0) {
    _MAXRES = 999.0;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncComb::~bncComb() {
  QListIterator<cmbAC*> icAC(_ACs);
  while (icAC.hasNext()) {
    delete icAC.next();
  }
  delete _rtnetDecoder;
  delete _antex;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    delete _params[iPar-1];
  }
  QListIterator<bncTime> itTime(_buffer.keys());
  while (itTime.hasNext()) {
    bncTime epoTime = itTime.next();
    _buffer.remove(epoTime);
  }
}

// Remember orbit corrections
////////////////////////////////////////////////////////////////////////////
void bncComb::slotNewOrbCorrections(QList<t_orbCorr> orbCorrections) {
  QMutexLocker locker(&_mutex);

  for (int ii = 0; ii < orbCorrections.size(); ii++) {
    t_orbCorr& orbCorr = orbCorrections[ii];
    QString    staID(orbCorr._staID.c_str());
    QString    prn(orbCorr._prn.toInternalString().c_str());

    // Find/Check the AC Name
    // ----------------------
    QString acName;
    QListIterator<cmbAC*> icAC(_ACs);
    while (icAC.hasNext()) {
      cmbAC* AC = icAC.next();
      if (AC->mountPoint == staID) {
        acName = AC->name;
        break;
      }
    }
    if (acName.isEmpty()) {
      continue;
    }

    // Store the correction
    // --------------------
    QMap<t_prn, t_orbCorr>& storage = _orbCorrections[acName];
    storage[orbCorr._prn] = orbCorr;
  }
}

// Process clock corrections
////////////////////////////////////////////////////////////////////////////
void bncComb::slotNewClkCorrections(QList<t_clkCorr> clkCorrections) {
  QMutexLocker locker(&_mutex);

  bncTime lastTime;

  for (int ii = 0; ii < clkCorrections.size(); ii++) {
    t_clkCorr& clkCorr = clkCorrections[ii];
    QString    staID(clkCorr._staID.c_str());
    QString    prn(clkCorr._prn.toInternalString().c_str());

    // Set the last time
    // -----------------
    if (lastTime.undef() || clkCorr._time > lastTime) {
      lastTime = clkCorr._time;
    }

    // Find/Check the AC Name
    // ----------------------
    QString acName;
    QListIterator<cmbAC*> icAC(_ACs);
    while (icAC.hasNext()) {
      cmbAC* AC = icAC.next();
      if (AC->mountPoint == staID) {
        acName = AC->name;
        break;
      }
    }
    if (acName.isEmpty()) {
      continue;
    }

    // Check GLONASS
    // -------------
    if (!_useGlonass && clkCorr._prn.system() == 'R') {
      continue;
    }

    if (
        clkCorr._prn.system() == 'E' ||
        clkCorr._prn.system() == 'C' ||
        clkCorr._prn.system() == 'J' ||
        clkCorr._prn.system() == 'I' ||
        clkCorr._prn.system() == 'S' )   {
      continue;
    }

    // Check Modulo Time
    // -----------------
    int sec = int(nint(clkCorr._time.gpssec()*10));
    if (sec % (_cmbSampl*10) != 0.0) {
      continue;
    }

    // Check Correction Age
    // --------------------
    if (_resTime.valid() && clkCorr._time <= _resTime) {
      emit newMessage("bncComb: old correction: " + acName.toLatin1() + " " + prn.mid(0,3).toLatin1(), true);
      continue;
    }

    // Create new correction
    // ---------------------
    cmbCorr* newCorr  = new cmbCorr();
    newCorr->_prn     = prn;
    newCorr->_time    = clkCorr._time;
    newCorr->_iod     = clkCorr._iod;
    newCorr->_acName  = acName;
    newCorr->_clkCorr = clkCorr;

    // Check orbit correction
    // ----------------------
    if (!_orbCorrections.contains(acName)) {
      delete newCorr;
      continue;
    }
    else {
      QMap<t_prn, t_orbCorr>& storage = _orbCorrections[acName];
      if (!storage.contains(clkCorr._prn)  || storage[clkCorr._prn]._iod != newCorr->_iod) {
        delete newCorr;
        continue;
      }
      else {
        newCorr->_orbCorr = storage[clkCorr._prn];
      }
    }

    // Check the Ephemeris
    //--------------------
    t_eph* ephLast = _ephUser.ephLast(prn);
    t_eph* ephPrev = _ephUser.ephPrev(prn);
    if (ephLast == 0) {
      emit newMessage("bncComb: eph not found for "  + prn.mid(0,3).toLatin1(), true);
      delete newCorr;
      continue;
    }
    else {
      if      (ephLast->IOD() == newCorr->_iod) {
        newCorr->_eph = ephLast;
      }
      else if (ephPrev && ephPrev->IOD() == newCorr->_iod) {
        newCorr->_eph = ephPrev;
        switchToLastEph(ephLast, newCorr);
      }
      else {
        emit newMessage("bncComb: eph not found for "  + prn.mid(0,3).toLatin1() +
                        QString(" with IOD %1").arg(newCorr->_iod).toLatin1(), true);
        delete newCorr;
        continue;
      }
    }

    // Store correction into the buffer
    // --------------------------------
    QVector<cmbCorr*>& corrs = _buffer[newCorr->_time].corrs;
    corrs.push_back(newCorr);
  }

  // Process previous Epoch(s)
  // -------------------------
  const double outWait = 1.0 * _cmbSampl;
  QListIterator<bncTime> itTime(_buffer.keys());
  while (itTime.hasNext()) {
    bncTime epoTime = itTime.next();
    if (epoTime < lastTime - outWait) {
      _resTime = epoTime;
      processEpoch();
    }
  }
}

// Change the correction so that it refers to last received ephemeris
////////////////////////////////////////////////////////////////////////////
void bncComb::switchToLastEph(t_eph* lastEph, cmbCorr* corr) {

  if (corr->_eph == lastEph) {
    return;
  }

  ColumnVector oldXC(6);
  ColumnVector oldVV(3);
  corr->_eph->getCrd(corr->_time, oldXC, oldVV, false);

  ColumnVector newXC(6);
  ColumnVector newVV(3);
  lastEph->getCrd(corr->_time, newXC, newVV, false);

  ColumnVector dX = newXC.Rows(1,3) - oldXC.Rows(1,3);
  ColumnVector dV = newVV           - oldVV;
  double       dC = newXC(4)        - oldXC(4);

  ColumnVector dRAO(3);
  XYZ_to_RSW(newXC.Rows(1,3), newVV, dX, dRAO);

  ColumnVector dDotRAO(3);
  XYZ_to_RSW(newXC.Rows(1,3), newVV, dV, dDotRAO);

  QString msg = "switch corr " + corr->_prn.mid(0,3)
    + QString(" %1 -> %2 %3").arg(corr->_iod,3).arg(lastEph->IOD(),3).arg(dC*t_CST::c, 8, 'f', 4);

  emit newMessage(msg.toLatin1(), false);

  corr->_iod = lastEph->IOD();
  corr->_eph = lastEph;

  corr->_orbCorr._xr    += dRAO;
  corr->_orbCorr._dotXr += dDotRAO;
  corr->_clkCorr._dClk  -= dC;
}

// Process Epoch
////////////////////////////////////////////////////////////////////////////
void bncComb::processEpoch() {

  _log.clear();

  QTextStream out(&_log, QIODevice::WriteOnly);

  out << endl <<           "Combination:" << endl
      << "------------------------------" << endl;

  // Observation Statistics
  // ----------------------
  bool masterPresent = false;
  QListIterator<cmbAC*> icAC(_ACs);
  while (icAC.hasNext()) {
    cmbAC* AC = icAC.next();
    AC->numObs = 0;
    QVectorIterator<cmbCorr*> itCorr(corrs());
    while (itCorr.hasNext()) {
      cmbCorr* corr = itCorr.next();
      if (corr->_acName == AC->name) {
        AC->numObs += 1;
        if (AC->name == _masterOrbitAC) {
          masterPresent = true;
        }
      }
    }
    out << AC->name.toLatin1().data() << ": " << AC->numObs << endl;
  }

  // If Master not present, switch to another one
  // --------------------------------------------
  const unsigned switchMasterAfterGap = 1;
  if (masterPresent) {
    _masterMissingEpochs = 0;
  }
  else {
    ++_masterMissingEpochs;
    if (_masterMissingEpochs < switchMasterAfterGap) {
      out << "Missing Master, Epoch skipped" << endl;
      _buffer.remove(_resTime);
      emit newMessage(_log, false);
      return;
    }
    else {
      _masterMissingEpochs = 0;
      QListIterator<cmbAC*> icAC(_ACs);
      while (icAC.hasNext()) {
        cmbAC* AC = icAC.next();
        if (AC->numObs > 0) {
          out << "Switching Master AC "
              << _masterOrbitAC.toLatin1().data() << " --> "
              << AC->name.toLatin1().data()   << " "
              << _resTime.datestr().c_str()    << " "
              << _resTime.timestr().c_str()    << endl;
          _masterOrbitAC = AC->name;
          break;
        }
      }
    }
  }

  QMap<QString, cmbCorr*> resCorr;

  // Perform the actual Combination using selected Method
  // ----------------------------------------------------
  t_irc irc;
  ColumnVector dx;
  if (_method == filter) {
    irc = processEpoch_filter(out, resCorr, dx);
  }
  else {
    irc = processEpoch_singleEpoch(out, resCorr, dx);
  }

  // Update Parameter Values, Print Results
  // --------------------------------------
  if (irc == success) {
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      cmbParam* pp = _params[iPar-1];
      pp->xx += dx(iPar);
      if (pp->type == cmbParam::clkSat) {
        if (resCorr.find(pp->prn) != resCorr.end()) {
          resCorr[pp->prn]->_dClkResult = pp->xx / t_CST::c;
        }
      }
      out << _resTime.datestr().c_str() << " "
          << _resTime.timestr().c_str() << " ";
      out.setRealNumberNotation(QTextStream::FixedNotation);
      out.setFieldWidth(8);
      out.setRealNumberPrecision(4);
      out << pp->toString() << " "
          << pp->xx << " +- " << sqrt(_QQ(pp->index,pp->index)) << endl;
      out.setFieldWidth(0);
    }
    printResults(out, resCorr);
    dumpResults(resCorr);
  }

  // Delete Data, emit Message
  // -------------------------
  _buffer.remove(_resTime);
  emit newMessage(_log, false);
}

// Process Epoch - Filter Method
////////////////////////////////////////////////////////////////////////////
t_irc bncComb::processEpoch_filter(QTextStream& out,
                                   QMap<QString, cmbCorr*>& resCorr,
                                   ColumnVector& dx) {

  // Prediction Step
  // ---------------
  int nPar = _params.size();
  ColumnVector x0(nPar);
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    cmbParam* pp  = _params[iPar-1];
    if (pp->epoSpec) {
      pp->xx = 0.0;
      _QQ.Row(iPar)    = 0.0;
      _QQ.Column(iPar) = 0.0;
      _QQ(iPar,iPar) = pp->sig0 * pp->sig0;
    }
    else {
      _QQ(iPar,iPar) += pp->sigP * pp->sigP;
    }
    x0(iPar) = pp->xx;
  }

  // Check Satellite Positions for Outliers
  // --------------------------------------
  if (checkOrbits(out) != success) {
    return failure;
  }

  // Update and outlier detection loop
  // ---------------------------------
  SymmetricMatrix QQ_sav = _QQ;
  while (true) {

    Matrix         AA;
    ColumnVector   ll;
    DiagonalMatrix PP;

    if (createAmat(AA, ll, PP, x0, resCorr) != success) {
      return failure;
    }

    dx.ReSize(nPar); dx = 0.0;
    kalman(AA, ll, PP, _QQ, dx);

    ColumnVector vv = ll - AA * dx;

    int     maxResIndex;
    double  maxRes = vv.maximum_absolute_value1(maxResIndex);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(3);
    out << _resTime.datestr().c_str() << " " << _resTime.timestr().c_str()
        << " Maximum Residuum " << maxRes << ' '
        << corrs()[maxResIndex-1]->_acName << ' ' << corrs()[maxResIndex-1]->_prn.mid(0,3);
    if (maxRes > _MAXRES) {
      for (int iPar = 1; iPar <= _params.size(); iPar++) {
        cmbParam* pp = _params[iPar-1];
        if (pp->type == cmbParam::offACSat            &&
            pp->AC   == corrs()[maxResIndex-1]->_acName &&
            pp->prn  == corrs()[maxResIndex-1]->_prn.mid(0,3)) {
          QQ_sav.Row(iPar)    = 0.0;
          QQ_sav.Column(iPar) = 0.0;
          QQ_sav(iPar,iPar)   = pp->sig0 * pp->sig0;
        }
      }

      out << "  Outlier" << endl;
      _QQ = QQ_sav;
      corrs().remove(maxResIndex-1);
    }
    else {
      out << "  OK" << endl;
      out.setRealNumberNotation(QTextStream::FixedNotation);
      out.setRealNumberPrecision(4);
      for (int ii = 0; ii < corrs().size(); ii++) {
      const cmbCorr* corr = corrs()[ii];
        out << _resTime.datestr().c_str() << ' '
            << _resTime.timestr().c_str() << " "
            << corr->_acName << ' ' << corr->_prn.mid(0,3);
        out.setFieldWidth(10);
        out <<  " res = " << vv[ii] << endl;
        out.setFieldWidth(0);
      }
      break;
    }
  }

  return success;
}

// Print results
////////////////////////////////////////////////////////////////////////////
void bncComb::printResults(QTextStream& out,
                           const QMap<QString, cmbCorr*>& resCorr) {

  QMapIterator<QString, cmbCorr*> it(resCorr);
  while (it.hasNext()) {
    it.next();
    cmbCorr* corr = it.value();
    const t_eph* eph = corr->_eph;
    if (eph) {
      ColumnVector xc(6);
      ColumnVector vv(3);
      eph->getCrd(_resTime, xc, vv, false);

      out << _resTime.datestr().c_str() << " "
          << _resTime.timestr().c_str() << " ";
      out.setFieldWidth(3);
      out << "Full Clock " << corr->_prn.mid(0,3) << " " << corr->_iod << " ";
      out.setFieldWidth(14);
      out << (xc(4) + corr->_dClkResult) * t_CST::c << endl;
      out.setFieldWidth(0);
    }
    else {
      out << "bncComb::printResuls bug" << endl;
    }
  }
}

// Send results to RTNet Decoder and directly to PPP Client
////////////////////////////////////////////////////////////////////////////
void bncComb::dumpResults(const QMap<QString, cmbCorr*>& resCorr) {

  QList<t_orbCorr> orbCorrections;
  QList<t_clkCorr> clkCorrections;

  QString     outLines;
  QStringList corrLines;

  unsigned year, month, day, hour, minute;
  double   sec;
  _resTime.civil_date(year, month, day);
  _resTime.civil_time(hour, minute, sec);

  outLines.sprintf("*  %4d %2d %2d %d %d %12.8f\n",
                   year, month, day, hour, minute, sec);

  QMapIterator<QString, cmbCorr*> it(resCorr);
  while (it.hasNext()) {
    it.next();
    cmbCorr* corr = it.value();

    t_orbCorr orbCorr(corr->_orbCorr);
    orbCorr._staID = "INTERNAL";
    orbCorrections.push_back(orbCorr);

    t_clkCorr clkCorr(corr->_clkCorr);
    clkCorr._staID      = "INTERNAL";
    clkCorr._dClk       = corr->_dClkResult;
    clkCorr._dotDClk    = 0.0;
    clkCorr._dotDotDClk = 0.0;
    clkCorrections.push_back(clkCorr);

    ColumnVector xc(6);
    ColumnVector vv(3);
    corr->_eph->setClkCorr(dynamic_cast<const t_clkCorr*>(&clkCorr));
    corr->_eph->setOrbCorr(dynamic_cast<const t_orbCorr*>(&orbCorr));
    corr->_eph->getCrd(_resTime, xc, vv, true);

    // Correction Phase Center --> CoM
    // -------------------------------
    ColumnVector dx(3); dx = 0.0;
    if (_antex) {
      double Mjd = _resTime.mjd() + _resTime.daysec()/86400.0;
      if (_antex->satCoMcorrection(corr->_prn, Mjd, xc.Rows(1,3), dx) != success) {
        dx = 0;
        _log += "antenna not found " + corr->_prn.mid(0,3).toLatin1() + '\n';
      }
    }

    outLines += corr->_prn.mid(0,3);
    QString hlp;
    hlp.sprintf(" APC 3 %15.4f %15.4f %15.4f"
                " Clk 1 %15.4f"
                " Vel 3 %15.4f %15.4f %15.4f"
                " CoM 3 %15.4f %15.4f %15.4f\n",
                xc(1), xc(2), xc(3),
                xc(4) *  t_CST::c,
                vv(1), vv(2), vv(3),
                xc(1)-dx(1), xc(2)-dx(2), xc(3)-dx(3));
    outLines += hlp;

    QString line;
    int messageType   = COTYPE_GPSCOMBINED;
    int updateInt     = 0;
    line.sprintf("%d %d %d %.1f %s"
                 "   %lu"
                 "   %8.3f %8.3f %8.3f %8.3f"
                 "   %10.5f %10.5f %10.5f %10.5f"
                 "   %10.5f INTERNAL",
                 messageType, updateInt, _resTime.gpsw(), _resTime.gpssec(),
                 corr->_prn.mid(0,3).toLatin1().data(),
                 corr->_iod,
                 corr->_dClkResult * t_CST::c,
                 corr->_orbCorr._xr[0],
                 corr->_orbCorr._xr[1],
                 corr->_orbCorr._xr[2],
                 0.0,
                 corr->_orbCorr._dotXr[0],
                 corr->_orbCorr._dotXr[1],
                 corr->_orbCorr._dotXr[2],
                 0.0);
    corrLines << line;

    delete corr;
  }

  outLines += "EOE\n"; // End Of Epoch flag

  if (!_rtnetDecoder) {
    _rtnetDecoder = new bncRtnetDecoder();
  }

  vector<string> errmsg;
  _rtnetDecoder->Decode(outLines.toLatin1().data(), outLines.length(), errmsg);

  // Send new Corrections to PPP etc.
  // --------------------------------
  if (orbCorrections.size() > 0 && clkCorrections.size() > 0) {
    emit newOrbCorrections(orbCorrections);
    emit newClkCorrections(clkCorrections);
  }
}

// Create First Design Matrix and Vector of Measurements
////////////////////////////////////////////////////////////////////////////
t_irc bncComb::createAmat(Matrix& AA, ColumnVector& ll, DiagonalMatrix& PP,
                          const ColumnVector& x0,
                          QMap<QString, cmbCorr*>& resCorr) {

  unsigned nPar = _params.size();
  unsigned nObs = corrs().size();

  if (nObs == 0) {
    return failure;
  }

  int maxSat = t_prn::MAXPRN_GPS;
//  if (_useGlonass) {
//    maxSat = t_prn::MAXPRN_GPS + t_prn::MAXPRN_GLONASS;
//  }

  const int nCon = (_method == filter) ? 1 + maxSat : 0;

  AA.ReSize(nObs+nCon, nPar);  AA = 0.0;
  ll.ReSize(nObs+nCon);        ll = 0.0;
  PP.ReSize(nObs+nCon);        PP = 1.0 / (sigObs * sigObs);

  int iObs = 0;
  QVectorIterator<cmbCorr*> itCorr(corrs());
  while (itCorr.hasNext()) {
    cmbCorr* corr = itCorr.next();
    QString  prn  = corr->_prn;

    ++iObs;

    if (corr->_acName == _masterOrbitAC && resCorr.find(prn) == resCorr.end()) {
      resCorr[prn] = new cmbCorr(*corr);
    }

    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      cmbParam* pp = _params[iPar-1];
      AA(iObs, iPar) = pp->partial(corr->_acName, prn);
    }

    ll(iObs) = corr->_clkCorr._dClk * t_CST::c - DotProduct(AA.Row(iObs), x0);
  }

  // Regularization
  // --------------
  if (_method == filter) {
    const double Ph = 1.e6;
    PP(nObs+1) = Ph;
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      cmbParam* pp = _params[iPar-1];
      if ( AA.Column(iPar).maximum_absolute_value() > 0.0 &&
           pp->type == cmbParam::clkSat ) {
        AA(nObs+1, iPar) = 1.0;
      }
    }
    int iCond = 1;
    for (unsigned iGps = 1; iGps <= t_prn::MAXPRN_GPS; iGps++) {
      QString prn = QString("G%1_0").arg(iGps, 2, 10, QChar('0'));
      ++iCond;
      PP(nObs+iCond) = Ph;
      for (int iPar = 1; iPar <= _params.size(); iPar++) {
        cmbParam* pp = _params[iPar-1];
        if ( pp &&
             AA.Column(iPar).maximum_absolute_value() > 0.0 &&
             pp->type == cmbParam::offACSat                 &&
             pp->prn == prn) {
          AA(nObs+iCond, iPar) = 1.0;
        }
      }
    }
//    if (_useGlonass) {// liefert schlechtere Höhenkomponente im PPP
//      for (unsigned iGlo = 1; iGlo <= t_prn::MAXPRN_GLONASS; iGlo++) {
//        QString prn = QString("R%1_0").arg(iGlo, 2, 10, QChar('0'));
//        ++iCond;
//        PP(nObs+iCond) = Ph;
//        for (int iPar = 1; iPar <= _params.size(); iPar++) {
//          cmbParam* pp = _params[iPar-1];
//          if ( pp &&
//               AA.Column(iPar).maximum_absolute_value() > 0.0 &&
//               pp->type == cmbParam::offACSat                 &&
//               pp->prn == prn) {
//            AA(nObs+iCond, iPar) = 1.0;
//          }
//        }
//      }
//    }
  }

  return success;
}

// Process Epoch - Single-Epoch Method
////////////////////////////////////////////////////////////////////////////
t_irc bncComb::processEpoch_singleEpoch(QTextStream& out,
                                        QMap<QString, cmbCorr*>& resCorr,
                                        ColumnVector& dx) {

  // Check Satellite Positions for Outliers
  // --------------------------------------
  if (checkOrbits(out) != success) {
    return failure;
  }

  // Outlier Detection Loop
  // ----------------------
  while (true) {

    // Remove Satellites that are not in Master
    // ----------------------------------------
    QMutableVectorIterator<cmbCorr*> it(corrs());
    while (it.hasNext()) {
      cmbCorr* corr = it.next();
      QString  prn  = corr->_prn;
      bool foundMaster = false;
      QVectorIterator<cmbCorr*> itHlp(corrs());
      while (itHlp.hasNext()) {
        cmbCorr* corrHlp = itHlp.next();
        QString  prnHlp  = corrHlp->_prn;
        QString  ACHlp   = corrHlp->_acName;
        if (ACHlp == _masterOrbitAC && prn == prnHlp) {
          foundMaster = true;
          break;
        }
      }
      if (!foundMaster) {
        delete corr;
        it.remove();
      }
    }

    // Count Number of Observations per Satellite and per AC
    // -----------------------------------------------------
    QMap<QString, int> numObsPrn;
    QMap<QString, int> numObsAC;
    QVectorIterator<cmbCorr*> itCorr(corrs());
    while (itCorr.hasNext()) {
      cmbCorr* corr = itCorr.next();
      QString  prn  = corr->_prn;
      QString  AC   = corr->_acName;
      if (numObsPrn.find(prn) == numObsPrn.end()) {
        numObsPrn[prn]  = 1;
      }
      else {
        numObsPrn[prn] += 1;
      }
      if (numObsAC.find(AC) == numObsAC.end()) {
        numObsAC[AC]  = 1;
      }
      else {
        numObsAC[AC] += 1;
      }
    }

    // Clean-Up the Paramters
    // ----------------------
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      delete _params[iPar-1];
    }
    _params.clear();

    // Set new Parameters
    // ------------------
    int nextPar = 0;

    QMapIterator<QString, int> itAC(numObsAC);
    while (itAC.hasNext()) {
      itAC.next();
      const QString& AC     = itAC.key();
      int            numObs = itAC.value();
      if (AC != _masterOrbitAC && numObs > 0) {
        _params.push_back(new cmbParam(cmbParam::offACgps, ++nextPar, AC, ""));
        if (_useGlonass) {
          _params.push_back(new cmbParam(cmbParam::offACglo, ++nextPar, AC, ""));
        }
      }
    }

    QMapIterator<QString, int> itPrn(numObsPrn);
    while (itPrn.hasNext()) {
      itPrn.next();
      const QString& prn    = itPrn.key();
      int            numObs = itPrn.value();
      if (numObs > 0) {
        _params.push_back(new cmbParam(cmbParam::clkSat, ++nextPar, "", prn));
      }
    }

    int nPar = _params.size();
    ColumnVector x0(nPar);
    x0 = 0.0;

    // Create First-Design Matrix
    // --------------------------
    Matrix         AA;
    ColumnVector   ll;
    DiagonalMatrix PP;
    if (createAmat(AA, ll, PP, x0, resCorr) != success) {
      return failure;
    }

    ColumnVector vv;
    try {
      Matrix          ATP = AA.t() * PP;
      SymmetricMatrix NN; NN << ATP * AA;
      ColumnVector    bb = ATP * ll;
      _QQ = NN.i();
      dx  = _QQ * bb;
      vv  = ll - AA * dx;
    }
    catch (Exception& exc) {
      out << exc.what() << endl;
      return failure;
    }

    int     maxResIndex;
    double  maxRes = vv.maximum_absolute_value1(maxResIndex);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(3);
    out << _resTime.datestr().c_str() << " " << _resTime.timestr().c_str()
        << " Maximum Residuum " << maxRes << ' '
        << corrs()[maxResIndex-1]->_acName << ' ' << corrs()[maxResIndex-1]->_prn.mid(0,3);

    if (maxRes > _MAXRES) {
      out << "  Outlier" << endl;
      delete corrs()[maxResIndex-1];
      corrs().remove(maxResIndex-1);
    }
    else {
      out << "  OK" << endl;
      out.setRealNumberNotation(QTextStream::FixedNotation);
      out.setRealNumberPrecision(3);
      for (int ii = 0; ii < vv.Nrows(); ii++) {
        const cmbCorr* corr = corrs()[ii];
        out << _resTime.datestr().c_str() << ' '
            << _resTime.timestr().c_str() << " "
            << corr->_acName << ' ' << corr->_prn.mid(0,3);
        out.setFieldWidth(6);
        out << " res = " << vv[ii] << endl;
        out.setFieldWidth(0);
      }
      return success;
    }

  }

  return failure;
}

// Check Satellite Positions for Outliers
////////////////////////////////////////////////////////////////////////////
t_irc bncComb::checkOrbits(QTextStream& out) {

  const double MAX_DISPLACEMENT = 0.20;

  // Switch to last ephemeris (if possible)
  // --------------------------------------
  QMutableVectorIterator<cmbCorr*> im(corrs());
  while (im.hasNext()) {
    cmbCorr* corr = im.next();
    QString  prn  = corr->_prn;

    t_eph* ephLast = _ephUser.ephLast(prn);
    t_eph* ephPrev = _ephUser.ephPrev(prn);

    if      (ephLast == 0) {
      out << "checkOrbit: missing eph (not found) " << corr->_prn.mid(0,3) << endl;
      delete corr;
      im.remove();
    }
    else if (corr->_eph == 0) {
      out << "checkOrbit: missing eph (zero) " << corr->_prn.mid(0,3) << endl;
      delete corr;
      im.remove();
    }
    else {
      if ( corr->_eph == ephLast || corr->_eph == ephPrev ) {
        switchToLastEph(ephLast, corr);
      }
      else {
        out << "checkOrbit: missing eph (deleted) " << corr->_prn.mid(0,3) << endl;
        delete corr;
        im.remove();
      }
    }
  }

  while (true) {

    // Compute Mean Corrections for all Satellites
    // -------------------------------------------
    QMap<QString, int>          numCorr;
    QMap<QString, ColumnVector> meanRao;
    QVectorIterator<cmbCorr*> it(corrs());
    while (it.hasNext()) {
      cmbCorr* corr = it.next();
      QString  prn  = corr->_prn;
      if (meanRao.find(prn) == meanRao.end()) {
        meanRao[prn].ReSize(4);
        meanRao[prn].Rows(1,3) = corr->_orbCorr._xr;
        meanRao[prn](4)        = 1;
      }
      else {
        meanRao[prn].Rows(1,3) += corr->_orbCorr._xr;
        meanRao[prn](4)        += 1;
      }
      if (numCorr.find(prn) == numCorr.end()) {
        numCorr[prn] = 1;
      }
      else {
        numCorr[prn] += 1;
      }
    }

    // Compute Differences wrt Mean, find Maximum
    // ------------------------------------------
    QMap<QString, cmbCorr*> maxDiff;
    it.toFront();
    while (it.hasNext()) {
      cmbCorr* corr = it.next();
      QString  prn  = corr->_prn;
      if (meanRao[prn](4) != 0) {
        meanRao[prn] /= meanRao[prn](4);
        meanRao[prn](4) = 0;
      }
      corr->_diffRao = corr->_orbCorr._xr - meanRao[prn].Rows(1,3);
      if (maxDiff.find(prn) == maxDiff.end()) {
        maxDiff[prn] = corr;
      }
      else {
        double normMax = maxDiff[prn]->_diffRao.NormFrobenius();
        double norm    = corr->_diffRao.NormFrobenius();
        if (norm > normMax) {
          maxDiff[prn] = corr;
        }
      }
    }

    if (_ACs.size() == 1) {
      break;
    }

    // Remove Outliers
    // ---------------
    bool removed = false;
    QMutableVectorIterator<cmbCorr*> im(corrs());
    while (im.hasNext()) {
      cmbCorr* corr = im.next();
      QString  prn  = corr->_prn;
      if      (numCorr[prn] < 2) {
        delete corr;
        im.remove();
      }
      else if (corr == maxDiff[prn]) {
        double norm = corr->_diffRao.NormFrobenius();
        if (norm > MAX_DISPLACEMENT) {
          out << _resTime.datestr().c_str()    << " "
              << _resTime.timestr().c_str()    << " "
              << "Orbit Outlier: "
              << corr->_acName.toLatin1().data() << " "
              << prn.mid(0,3).toLatin1().data()           << " "
              << corr->_iod                     << " "
              << norm                           << endl;
          delete corr;
          im.remove();
          removed = true;
        }
      }
    }

    if (!removed) {
      break;
    }
  }

  return success;
}

//
////////////////////////////////////////////////////////////////////////////
void bncComb::slotProviderIDChanged(QString mountPoint) {
  QMutexLocker locker(&_mutex);

  QTextStream out(&_log, QIODevice::WriteOnly);

  // Find the AC Name
  // ----------------
  QString acName;
  QListIterator<cmbAC*> icAC(_ACs);
  while (icAC.hasNext()) {
    cmbAC* AC = icAC.next();
    if (AC->mountPoint == mountPoint) {
      acName = AC->name;
      out << "Provider ID changed: AC " << AC->name.toLatin1().data()   << " "
          << _resTime.datestr().c_str()    << " "
          << _resTime.timestr().c_str()    << endl;
      break;
    }
  }
  if (acName.isEmpty()) {
    return;
  }

  // Remove all corrections of the corresponding AC
  // ----------------------------------------------
  QListIterator<bncTime> itTime(_buffer.keys());
  while (itTime.hasNext()) {
    bncTime epoTime = itTime.next();
    QVector<cmbCorr*>& corrVec = _buffer[epoTime].corrs;
    QMutableVectorIterator<cmbCorr*> it(corrVec);
    while (it.hasNext()) {
      cmbCorr* corr = it.next();
      if (acName == corr->_acName) {
        delete corr;
        it.remove();
      }
    }
  }

  // Reset Satellite Offsets
  // -----------------------
  if (_method == filter) {
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      cmbParam* pp = _params[iPar-1];
      if (pp->AC == acName && pp->type == cmbParam::offACSat) {
        pp->xx = 0.0;
        _QQ.Row(iPar)    = 0.0;
        _QQ.Column(iPar) = 0.0;
        _QQ(iPar,iPar) = pp->sig0 * pp->sig0;
      }
    }
  }
}
