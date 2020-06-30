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
 * Class:      bncWindow
 *
 * Purpose:    This class implements the main application window.
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QFontDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QToolBar>

#include <unistd.h>
#include "bncwindow.h"
#include "bnccore.h"
#include "bncgetthread.h"
#include "bnctabledlg.h"
#include "bncipport.h"
#include "bncudpport.h"
#include "bncserialport.h"
#include "bnchlpdlg.h"
#include "bnchtml.h"
#include "bnctableitem.h"
#include "bncsettings.h"
#include "bncfigure.h"
#include "bncfigurelate.h"
#include "bncfigureppp.h"
#include "bncversion.h"
#include "bncbytescounter.h"
#include "bncsslconfig.h"
#include "upload/bnccustomtrafo.h"
#include "upload/bncephuploadcaster.h"
#include "qtfilechooser.h"
#include "reqcdlg.h"
#include "bncmap.h"
#include "rinex/reqcedit.h"
#include "rinex/reqcanalyze.h"
#include "orbComp/sp3Comp.h"
#ifdef QT_WEBKIT
#  include "map/bncmapwin.h"
#endif

using namespace std;

#ifdef GNSSCENTER_PLUGIN
Q_EXPORT_PLUGIN2(gnsscenter_bnc, t_bncFactory)
#endif

// Constructor
////////////////////////////////////////////////////////////////////////////
bncWindow::bncWindow() {

  const static QPalette paletteWhite(QColor(255, 255, 255));
  const static QPalette paletteGray(QColor(230, 230, 230));

#ifdef GNSSCENTER_PLUGIN
  BNC_CORE->setConfFileName("");
#endif

  _caster    = 0;
  _casterEph = 0;

  _bncFigure     = new bncFigure(this);
  _bncFigureLate = new bncFigureLate(this);
  _bncFigurePPP  = new bncFigurePPP(this);

  connect(BNC_CORE, SIGNAL(newPosition(QByteArray, bncTime, QVector<double>)),
          _bncFigurePPP, SLOT(slotNewPosition(QByteArray, bncTime, QVector<double>)));

  connect(BNC_CORE, SIGNAL(progressRnxPPP(int)), this, SLOT(slotPostProcessingProgress(int)));
  connect(BNC_CORE, SIGNAL(finishedRnxPPP()),    this, SLOT(slotPostProcessingFinished()));

  _runningRealTime    = false;
  _runningPPP         = false;
  _runningEdit        = false;
  _runningQC          = false;
  _runningSp3Comp     = false;
  _reqcActionComboBox = 0; // necessary for enableStartStop()

  _mapWin = 0;

  int ww = QFontMetrics(this->font()).width('w');

  static const QStringList labels = QString("account, Streams:   resource loader / mountpoint, decoder, country, lat, long, nmea, ntrip, bytes").split(",");

  setMinimumSize(100*ww, 70*ww);

  setWindowTitle(tr("BKG Ntrip Client (BNC) Version " BNCVERSION));

  connect(BNC_CORE, SIGNAL(newMessage(QByteArray,bool)),
          this, SLOT(slotWindowMessage(QByteArray,bool)));

  // Create Actions
  // --------------
  _actHelp = new QAction(tr("&Help Contents"),this);
  connect(_actHelp, SIGNAL(triggered()), SLOT(slotHelp()));

  _actAbout = new QAction(tr("&About BNC"),this);
  connect(_actAbout, SIGNAL(triggered()), SLOT(slotAbout()));

  _actFlowchart = new QAction(tr("&Flow Chart"),this);
  connect(_actFlowchart, SIGNAL(triggered()), SLOT(slotFlowchart()));

  _actFontSel = new QAction(tr("Select &Font"),this);
  connect(_actFontSel, SIGNAL(triggered()), SLOT(slotFontSel()));

  _actSaveOpt = new QAction(tr("&Reread && Save Configuration"),this);
  connect(_actSaveOpt, SIGNAL(triggered()), SLOT(slotSaveOptions()));

  _actQuit  = new QAction(tr("&Quit"),this);
  connect(_actQuit, SIGNAL(triggered()), SLOT(close()));

  _actAddMountPoints = new QAction(tr("Add &Stream"),this);
  connect(_actAddMountPoints, SIGNAL(triggered()), SLOT(slotAddMountPoints()));

  _actDeleteMountPoints = new QAction(tr("&Delete Stream"),this);
  connect(_actDeleteMountPoints, SIGNAL(triggered()), SLOT(slotDeleteMountPoints()));
  _actDeleteMountPoints->setEnabled(false);

  _actMapMountPoints = new QAction(tr("&Map"),this);
  connect(_actMapMountPoints, SIGNAL(triggered()), SLOT(slotMapMountPoints()));

  _actStart = new QAction(tr("Sta&rt"),this);
  connect(_actStart, SIGNAL(triggered()), SLOT(slotStart()));

  _actStop = new QAction(tr("Sto&p"),this);
  connect(_actStop, SIGNAL(triggered()), SLOT(slotStop()));

  _actwhatsthis= new QAction(tr("Help?=Shift+F1"),this);
  connect(_actwhatsthis, SIGNAL(triggered()), SLOT(slotWhatsThis()));

  CreateMenu();
  AddToolbar();

  bncSettings settings;

  // Network Options
  // ---------------
  _proxyHostLineEdit  = new QLineEdit(settings.value("proxyHost").toString());
  _proxyPortLineEdit  = new QLineEdit(settings.value("proxyPort").toString());

  connect(_proxyHostLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  _sslCaCertPathLineEdit   = new QLineEdit(settings.value("sslCaCertPath").toString());
  _sslIgnoreErrorsCheckBox = new QCheckBox();
  _sslIgnoreErrorsCheckBox->setCheckState(Qt::CheckState(
                                          settings.value("sslIgnoreErrors").toInt()));

  // General Options
  // ---------------
  _logFileLineEdit    = new QLineEdit(settings.value("logFile").toString());
  _rawOutFileLineEdit = new QLineEdit(settings.value("rawOutFile").toString());
  _rnxAppendCheckBox  = new QCheckBox();
  _rnxAppendCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("rnxAppend").toInt()));
  _onTheFlyComboBox = new QComboBox();
  _onTheFlyComboBox->setEditable(false);
  _onTheFlyComboBox->addItems(QString("no,1 day,1 hour,5 min,1 min").split(","));
  int ii = _onTheFlyComboBox->findText(settings.value("onTheFlyInterval").toString());
  if (ii != -1) {
    _onTheFlyComboBox->setCurrentIndex(ii);
  }
  _autoStartCheckBox  = new QCheckBox();
  _autoStartCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("autoStart").toInt()));

  // RINEX Observations Options
  // --------------------------
  _rnxPathLineEdit    = new QLineEdit(settings.value("rnxPath").toString());
  _rnxIntrComboBox    = new QComboBox();
  _rnxIntrComboBox->setEditable(false);
  _rnxIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  ii = _rnxIntrComboBox->findText(settings.value("rnxIntr").toString());
  if (ii != -1) {
    _rnxIntrComboBox->setCurrentIndex(ii);
  }
  _rnxSamplComboBox    = new QComboBox();
  _rnxSamplComboBox->setEditable(false);
  _rnxSamplComboBox->addItems(QString("0.1 sec,1 sec,5 sec,10 sec,15 sec,30 sec,60 sec").split(","));
  int ij = _rnxSamplComboBox->findText(settings.value("rnxSampl").toString());
  if (ij != -1) {
    _rnxSamplComboBox->setCurrentIndex(ij);
  }
  _rnxFileCheckBox = new QCheckBox();
  _rnxFileCheckBox->setCheckState(Qt::CheckState(settings.value("rnxOnlyWithSKL").toInt()));
  _rnxSkelLineEdit    = new QLineEdit(settings.value("rnxSkel").toString());
  _rnxSkelLineEdit->setMaximumWidth(5*ww);
  _rnxScrpLineEdit    = new QLineEdit(settings.value("rnxScript").toString());
  _rnxV3CheckBox      = new QCheckBox();
  _rnxV3CheckBox->setCheckState(Qt::CheckState(settings.value("rnxV3").toInt()));
  QString hlp = settings.value("rnxV2Priority").toString();
  if (hlp.isEmpty()) {
    hlp = "G:12&PWCSLX G:5&IQX R:12&PC R:3&IQX R:46&ABX E:16&BCXZ E:578&IQX J:1&SLXCZ J:26&SLX J:5&IQX C:267&IQX C:128&DPX C:7&DPZ I:ABCX S:1&C S:5&IQX";
  }
  _rnxV2Priority = new QLineEdit(hlp);

  connect(_rnxPathLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));
  connect(_rnxV3CheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(slotBncTextChanged()));

  // RINEX Ephemeris Options
  // -----------------------
  _ephPathLineEdit    = new QLineEdit(settings.value("ephPath").toString());
  _ephIntrComboBox    = new QComboBox();
  _ephIntrComboBox->setEditable(false);
  _ephIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int jj = _ephIntrComboBox->findText(settings.value("ephIntr").toString());
  if (jj != -1) {
    _ephIntrComboBox->setCurrentIndex(jj);
  }
  _ephOutPortLineEdit    = new QLineEdit(settings.value("ephOutPort").toString());
  _ephV3CheckBox = new QCheckBox();
  _ephV3CheckBox->setCheckState(Qt::CheckState(settings.value("ephV3").toInt()));

  connect(_ephOutPortLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_ephPathLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Broadcast Corrections Options
  // -----------------------------
  _corrPathLineEdit    = new QLineEdit(settings.value("corrPath").toString());
  _corrIntrComboBox    = new QComboBox();
  _corrIntrComboBox->setEditable(false);
  _corrIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  int mm = _corrIntrComboBox->findText(settings.value("corrIntr").toString());
  if (mm != -1) {
    _corrIntrComboBox->setCurrentIndex(mm);
  }
  _corrPortLineEdit    = new QLineEdit(settings.value("corrPort").toString());

  connect(_corrPathLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_corrPortLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Feed Engine Options
  // -------------------
  _outPortLineEdit    = new QLineEdit(settings.value("outPort").toString());
  _outWaitSpinBox   = new QSpinBox();
  _outWaitSpinBox->setMinimum(1);
  _outWaitSpinBox->setMaximum(30);
  _outWaitSpinBox->setSingleStep(1);
  _outWaitSpinBox->setSuffix(" sec");
  _outWaitSpinBox->setValue(settings.value("outWait").toInt());
  _outSamplComboBox    = new QComboBox();
  _outSamplComboBox->addItems(QString("0.1 sec,1 sec,5 sec,10 sec,15 sec,30 sec,60 sec").split(","));
  int nn = _outSamplComboBox->findText(settings.value("outSampl").toString());
    if (nn != -1) {
      _outSamplComboBox->setCurrentIndex(nn);
    }
  _outFileLineEdit    = new QLineEdit(settings.value("outFile").toString());
  _outUPortLineEdit   = new QLineEdit(settings.value("outUPort").toString());
  _outLockTimeCheckBox = new QCheckBox();
  _outLockTimeCheckBox->setCheckState(Qt::CheckState(settings.value("outLockTime").toInt()));

  connect(_outPortLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_outFileLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_outLockTimeCheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(slotBncTextChanged()));

  // Serial Output Options
  // ---------------------
  _serialMountPointLineEdit = new QLineEdit(settings.value("serialMountPoint").toString());
  _serialPortNameLineEdit = new QLineEdit(settings.value("serialPortName").toString());
  _serialBaudRateComboBox = new QComboBox();
  _serialBaudRateComboBox->addItems(QString("110,300,600,"
            "1200,2400,4800,9600,19200,38400,57600,115200").split(","));
  int kk = _serialBaudRateComboBox->findText(settings.value("serialBaudRate").toString());
  if (kk != -1) {
    _serialBaudRateComboBox->setCurrentIndex(kk);
  }
  _serialFlowControlComboBox = new QComboBox();
  _serialFlowControlComboBox->addItems(QString("OFF,XONXOFF,HARDWARE").split(","));
  kk = _serialFlowControlComboBox->findText(settings.value("serialFlowControl").toString());
  if (kk != -1) {
    _serialFlowControlComboBox->setCurrentIndex(kk);
  }
  _serialDataBitsComboBox = new QComboBox();
  _serialDataBitsComboBox->addItems(QString("5,6,7,8").split(","));
  kk = _serialDataBitsComboBox->findText(settings.value("serialDataBits").toString());
  if (kk != -1) {
    _serialDataBitsComboBox->setCurrentIndex(kk);
  }
  _serialParityComboBox   = new QComboBox();
  _serialParityComboBox->addItems(QString("NONE,ODD,EVEN,SPACE").split(","));
  kk = _serialParityComboBox->findText(settings.value("serialParity").toString());
  if (kk != -1) {
    _serialParityComboBox->setCurrentIndex(kk);
  }
  _serialStopBitsComboBox = new QComboBox();
  _serialStopBitsComboBox->addItems(QString("1,2").split(","));
  kk = _serialStopBitsComboBox->findText(settings.value("serialStopBits").toString());
  if (kk != -1) {
    _serialStopBitsComboBox->setCurrentIndex(kk);
  }
  _serialAutoNMEAComboBox  = new QComboBox();
  _serialAutoNMEAComboBox->addItems(QString("no,Auto,Manual GPGGA,Manual GNGGA").split(","));
  kk = _serialAutoNMEAComboBox->findText(settings.value("serialAutoNMEA").toString());
  if (kk != -1) {
    _serialAutoNMEAComboBox->setCurrentIndex(kk);
  }
  _serialFileNMEALineEdit    = new QLineEdit(settings.value("serialFileNMEA").toString());
  _serialHeightNMEALineEdit  = new QLineEdit(settings.value("serialHeightNMEA").toString());

  _serialManualNMEASamplingSpinBox = new QSpinBox();
  _serialManualNMEASamplingSpinBox->setMinimum(0);
  _serialManualNMEASamplingSpinBox->setMaximum(300);
  _serialManualNMEASamplingSpinBox->setSingleStep(10);
  _serialManualNMEASamplingSpinBox->setValue(settings.value("serialManualNMEASampling").toInt());
  _serialManualNMEASamplingSpinBox->setSuffix(" sec");

  connect(_serialMountPointLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  connect(_serialAutoNMEAComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Outages Options
  // ---------------
  _adviseObsRateComboBox    = new QComboBox();
  _adviseObsRateComboBox->setEditable(false);
  _adviseObsRateComboBox->addItems(QString(",0.1 Hz,0.2 Hz,0.5 Hz,1 Hz,5 Hz").split(","));
  kk = _adviseObsRateComboBox->findText(settings.value("adviseObsRate").toString());
  if (kk != -1) {
    _adviseObsRateComboBox->setCurrentIndex(kk);
  }
  _adviseFailSpinBox = new QSpinBox();
  _adviseFailSpinBox->setMinimum(0);
  _adviseFailSpinBox->setMaximum(60);
  _adviseFailSpinBox->setSingleStep(1);
  _adviseFailSpinBox->setSuffix(" min");
  _adviseFailSpinBox->setValue(settings.value("adviseFail").toInt());
  _adviseRecoSpinBox = new QSpinBox();
  _adviseRecoSpinBox->setMinimum(0);
  _adviseRecoSpinBox->setMaximum(60);
  _adviseRecoSpinBox->setSingleStep(1);
  _adviseRecoSpinBox->setSuffix(" min");
  _adviseRecoSpinBox->setValue(settings.value("adviseReco").toInt());
  _adviseScriptLineEdit    = new QLineEdit(settings.value("adviseScript").toString());

  connect(_adviseObsRateComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Miscellaneous Options
  // ---------------------
  _miscMountLineEdit  = new QLineEdit(settings.value("miscMount").toString());
  _miscPortLineEdit   = new QLineEdit(settings.value("miscPort").toString());
  _miscIntrComboBox   = new QComboBox();
  _miscIntrComboBox->setEditable(false);
  _miscIntrComboBox->addItems(QString(",2 sec,10 sec,1 min,5 min,15 min,1 hour,6 hours,1 day").split(","));
  int ll = _miscIntrComboBox->findText(settings.value("miscIntr").toString());
  if (ll != -1) {
    _miscIntrComboBox->setCurrentIndex(ll);
  }
  _miscScanRTCMCheckBox  = new QCheckBox();
  _miscScanRTCMCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("miscScanRTCM").toInt()));

  connect(_miscMountLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Streams
  // -------
  _mountPointsTable   = new QTableWidget(0,9);

  _mountPointsTable->horizontalHeader()->resizeSection(1,34*ww);
  _mountPointsTable->horizontalHeader()->resizeSection(2,9*ww);
  _mountPointsTable->horizontalHeader()->resizeSection(3,9*ww);
  _mountPointsTable->horizontalHeader()->resizeSection(4,7*ww);
  _mountPointsTable->horizontalHeader()->resizeSection(5,7*ww);
  _mountPointsTable->horizontalHeader()->resizeSection(6,5*ww);
  _mountPointsTable->horizontalHeader()->resizeSection(7,5*ww);
#if QT_VERSION < 0x050000
  _mountPointsTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#else
  _mountPointsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#endif
  _mountPointsTable->horizontalHeader()->setStretchLastSection(true);
  _mountPointsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  _mountPointsTable->setHorizontalHeaderLabels(labels);
  _mountPointsTable->setGridStyle(Qt::NoPen);
  _mountPointsTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  _mountPointsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  _mountPointsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  _mountPointsTable->hideColumn(0);
  _mountPointsTable->hideColumn(3);
  connect(_mountPointsTable, SIGNAL(itemSelectionChanged()),
          SLOT(slotSelectionChanged()));
  populateMountPointsTable();

  _log = new QTextEdit();
  _log->setReadOnly(true);
  QFont msFont(""); msFont.setStyleHint(QFont::TypeWriter); // default monospace font
  _log->setFont(msFont);
  _log->document()->setMaximumBlockCount(1000);

  // Combine Corrections
  // -------------------
  _cmbTable = new QTableWidget(0,3);
  _cmbTable->setHorizontalHeaderLabels(QString("Mountpoint, AC Name, Weight").split(","));
  _cmbTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  _cmbTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  _cmbTable->setMaximumWidth(30*ww);
  _cmbTable->horizontalHeader()->resizeSection(0,10*ww);
  _cmbTable->horizontalHeader()->resizeSection(1,8*ww);
  _cmbTable->horizontalHeader()->resizeSection(2,8*ww);
#if QT_VERSION < 0x050000
  _cmbTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#else
  _cmbTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#endif
  _cmbTable->horizontalHeader()->setStretchLastSection(true);
  _cmbTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  _cmbMaxresLineEdit = new QLineEdit(settings.value("cmbMaxres").toString());
  _cmbUseGlonass     = new QCheckBox();
  _cmbUseGlonass->setCheckState(Qt::CheckState(settings.value("cmbUseGlonass").toInt()));

  _cmbSamplSpinBox = new QSpinBox;
  _cmbSamplSpinBox->setMinimum(10);
  _cmbSamplSpinBox->setMaximum(60);
  _cmbSamplSpinBox->setSingleStep(10);
  _cmbSamplSpinBox->setMaximumWidth(9*ww);
  _cmbSamplSpinBox->setValue(settings.value("cmbSampl").toInt());
  _cmbSamplSpinBox->setSuffix(" sec");

  QPushButton* addCmbRowButton = new QPushButton("Add Row");
  QPushButton* delCmbRowButton = new QPushButton("Delete");

  connect(_cmbTable, SIGNAL(itemSelectionChanged()),
          SLOT(slotBncTextChanged()));

  _cmbMethodComboBox = new QComboBox();
  _cmbMethodComboBox->setEditable(false);
  _cmbMethodComboBox->addItems(QString("Filter,Single-Epoch").split(","));
  int im = _cmbMethodComboBox->findText(settings.value("cmbMethod").toString());
  if (im != -1) {
    _cmbMethodComboBox->setCurrentIndex(im);
  }

  int iRow = _cmbTable->rowCount();
  if (iRow > 0) {
    enableWidget(true, _cmbMethodComboBox);
    enableWidget(true, _cmbMaxresLineEdit);
    enableWidget(true, _cmbSamplSpinBox);
    enableWidget(true, _cmbUseGlonass);
  }
  else {
    enableWidget(false, _cmbMethodComboBox);
    enableWidget(false, _cmbMaxresLineEdit);
    enableWidget(false, _cmbSamplSpinBox);
    enableWidget(false, _cmbUseGlonass);
  }

  // Upload Results
  // -------------
  _uploadTable = new QTableWidget(0,14);
  _uploadTable->setHorizontalHeaderLabels(QString("Host, Port, Mount, Ntrip, User, Password, System, CoM, SP3 File, RNX File, PID, SID, IOD, bytes").split(","));
  _uploadTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  _uploadTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  _uploadTable->horizontalHeader()->resizeSection( 0,13*ww);
  _uploadTable->horizontalHeader()->resizeSection( 1, 5*ww);
  _uploadTable->horizontalHeader()->resizeSection( 2, 6*ww);
  _uploadTable->horizontalHeader()->resizeSection( 3, 6*ww);
  _uploadTable->horizontalHeader()->resizeSection( 4, 8*ww);
  _uploadTable->horizontalHeader()->resizeSection( 5, 8*ww);
  _uploadTable->horizontalHeader()->resizeSection( 6,11*ww);
  _uploadTable->horizontalHeader()->resizeSection( 7, 4*ww);
  _uploadTable->horizontalHeader()->resizeSection( 8,15*ww);
  _uploadTable->horizontalHeader()->resizeSection( 9,15*ww);
  _uploadTable->horizontalHeader()->resizeSection(10, 4*ww);
  _uploadTable->horizontalHeader()->resizeSection(11, 4*ww);
  _uploadTable->horizontalHeader()->resizeSection(12, 4*ww);
  _uploadTable->horizontalHeader()->resizeSection(13,12*ww);
#if QT_VERSION < 0x050000
  _uploadTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#else
  _uploadTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#endif
  _uploadTable->horizontalHeader()->setStretchLastSection(true);
  _uploadTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  connect(_uploadTable, SIGNAL(itemSelectionChanged()),
          SLOT(slotBncTextChanged()));

  QPushButton* addUploadRowButton = new QPushButton("Add Row");
  QPushButton* delUploadRowButton = new QPushButton("Del Row");
  QPushButton* setUploadTrafoButton = new QPushButton("Custom Trafo");
  _uploadIntrComboBox = new QComboBox;
  _uploadIntrComboBox->setEditable(false);
  _uploadIntrComboBox->addItems(QString("1 day,1 hour, 30 min,15 min,10 min,5 min,2 min,1 min").split(","));
  ii = _uploadIntrComboBox->findText(settings.value("uploadIntr").toString());
  if (ii != -1) {
    _uploadIntrComboBox->setCurrentIndex(ii);
  }

  _uploadAntexFile      = new qtFileChooser(0, qtFileChooser::File);
  _uploadAntexFile->setFileName(settings.value("uploadAntexFile").toString());

  _uploadSamplRtcmEphCorrSpinBox = new QSpinBox;
  _uploadSamplRtcmEphCorrSpinBox->setMinimum(0);
  _uploadSamplRtcmEphCorrSpinBox->setMaximum(60);
  _uploadSamplRtcmEphCorrSpinBox->setSingleStep(5);
  _uploadSamplRtcmEphCorrSpinBox->setMaximumWidth(9*ww);
  _uploadSamplRtcmEphCorrSpinBox->setValue(settings.value("uploadSamplRtcmEphCorr").toInt());
  _uploadSamplRtcmEphCorrSpinBox->setSuffix(" sec");

  _uploadSamplSp3SpinBox = new QSpinBox;
  _uploadSamplSp3SpinBox->setMinimum(0);
  _uploadSamplSp3SpinBox->setMaximum(15);
  _uploadSamplSp3SpinBox->setSingleStep(1);
  _uploadSamplSp3SpinBox->setMaximumWidth(9*ww);
  _uploadSamplSp3SpinBox->setValue(settings.value("uploadSamplSp3").toInt());
  _uploadSamplSp3SpinBox->setSuffix(" min");

  _uploadSamplClkRnxSpinBox = new QSpinBox;
  _uploadSamplClkRnxSpinBox->setMinimum(0);
  _uploadSamplClkRnxSpinBox->setMaximum(60);
  _uploadSamplClkRnxSpinBox->setSingleStep(5);
  _uploadSamplClkRnxSpinBox->setMaximumWidth(9*ww);
  _uploadSamplClkRnxSpinBox->setValue(settings.value("uploadSamplClkRnx").toInt());
  _uploadSamplClkRnxSpinBox->setSuffix(" sec");

  int iRowT = _uploadTable->rowCount();
  if (iRowT > 0) {
    enableWidget(true, _uploadIntrComboBox);
    enableWidget(true, _uploadSamplRtcmEphCorrSpinBox);
    enableWidget(true, _uploadSamplSp3SpinBox);
    enableWidget(true, _uploadSamplClkRnxSpinBox);
    enableWidget(true, _uploadAntexFile);
  }
  else {
    enableWidget(false, _uploadIntrComboBox);
    enableWidget(false, _uploadSamplRtcmEphCorrSpinBox);
    enableWidget(false, _uploadSamplSp3SpinBox);
    enableWidget(false, _uploadSamplClkRnxSpinBox);
    enableWidget(false, _uploadAntexFile);
  }

  // Upload RTCM3 Ephemeris
  // ----------------------
  _uploadEphTable = new QTableWidget(0,6);
  _uploadEphTable->setColumnCount(8);
  _uploadEphTable->setRowCount(0);
  _uploadEphTable->setHorizontalHeaderLabels(QString("Host, Port, Mount,  Ntrip, User, Password, System, bytes").split(","));
  _uploadEphTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  _uploadEphTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  _uploadEphTable->horizontalHeader()->resizeSection( 0,13*ww);
  _uploadEphTable->horizontalHeader()->resizeSection( 1, 5*ww);
  _uploadEphTable->horizontalHeader()->resizeSection( 2, 8*ww);
  _uploadEphTable->horizontalHeader()->resizeSection( 3, 6*ww);
  _uploadEphTable->horizontalHeader()->resizeSection( 4, 8*ww);
  _uploadEphTable->horizontalHeader()->resizeSection( 3, 8*ww);
  _uploadEphTable->horizontalHeader()->resizeSection( 5,10*ww);
  _uploadEphTable->horizontalHeader()->resizeSection( 6,12*ww);
#if QT_VERSION < 0x050000
  _uploadEphTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#else
  _uploadEphTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#endif
  _uploadEphTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  connect(_uploadEphTable, SIGNAL(itemSelectionChanged()),
          SLOT(slotBncTextChanged()));

  QPushButton* addUploadEphRowButton = new QPushButton("Add Row");
  addUploadEphRowButton->setMaximumWidth(9*ww);
  QPushButton* delUploadEphRowButton = new QPushButton("Del Row");
  delUploadEphRowButton->setMaximumWidth(9*ww);

  _uploadSamplRtcmEphSpinBox = new QSpinBox;
  _uploadSamplRtcmEphSpinBox->setMinimum(0);
  _uploadSamplRtcmEphSpinBox->setMaximum(60);
  _uploadSamplRtcmEphSpinBox->setSingleStep(5);
  _uploadSamplRtcmEphSpinBox->setMaximumWidth(9*ww);
  _uploadSamplRtcmEphSpinBox->setValue(settings.value("uploadSamplRtcmEph").toInt());
  _uploadSamplRtcmEphSpinBox->setSuffix(" sec");

  iRowT = _uploadEphTable->rowCount();
  if (iRowT > 0) {
    enableWidget(true, _uploadSamplRtcmEphSpinBox);
  }
  else {
    enableWidget(false, _uploadSamplRtcmEphSpinBox);
  }

  // Canvas with Editable Fields
  // ---------------------------
  _canvas = new QWidget;
  setCentralWidget(_canvas);

  _aogroup = new QTabWidget();
  _aogroup->setElideMode(Qt::ElideNone);
  _aogroup->setUsesScrollButtons(true);
  QWidget* pgroup = new QWidget();
  QWidget* ggroup = new QWidget();
  QWidget* sgroup = new QWidget();
  QWidget* egroup = new QWidget();
  QWidget* agroup = new QWidget();
  QWidget* cgroup = new QWidget();
  QWidget* ogroup = new QWidget();
  QWidget* rgroup = new QWidget();
  QWidget* sergroup = new QWidget();
  QWidget* pppGroup1 = new QWidget();
  QWidget* pppGroup2 = new QWidget();
  QWidget* pppGroup3 = new QWidget();
  QWidget* pppGroup4 = new QWidget();
  QWidget* reqcgroup = new QWidget();
  QWidget* sp3CompGroup = new QWidget();
  QWidget* cmbgroup = new QWidget();
  QWidget* uploadgroup = new QWidget();
  QWidget* uploadEphgroup = new QWidget();
  _aogroup->addTab(pgroup,tr("Network"));
  _aogroup->addTab(ggroup,tr("General"));
  _aogroup->addTab(ogroup,tr("RINEX Observations"));
  _aogroup->addTab(egroup,tr("RINEX Ephemeris"));
  _aogroup->addTab(reqcgroup,tr("RINEX Editing && QC"));
  _aogroup->addTab(sp3CompGroup,tr("SP3 Comparison"));
  _aogroup->addTab(cgroup,tr("Broadcast Corrections"));
  _aogroup->addTab(sgroup,tr("Feed Engine"));
  _aogroup->addTab(sergroup,tr("Serial Output"));
  _aogroup->addTab(agroup,tr("Outages"));
  _aogroup->addTab(rgroup,tr("Miscellaneous"));
  _aogroup->addTab(pppGroup1,tr("PPP (1)"));
  _aogroup->addTab(pppGroup2,tr("PPP (2)"));
  _aogroup->addTab(pppGroup3,tr("PPP (3)"));
  _aogroup->addTab(pppGroup4,tr("PPP (4)"));
  _aogroup->addTab(cmbgroup,tr("Combine Corrections"));
  _aogroup->addTab(uploadgroup,tr("Upload Corrections"));
  _aogroup->addTab(uploadEphgroup,tr("Upload Ephemeris"));

  // Log Tab
  // -------
  _loggroup = new QTabWidget();
  _loggroup->addTab(_log,tr("Log"));
  _loggroup->addTab(_bncFigure,tr("Throughput"));
  _loggroup->addTab(_bncFigureLate,tr("Latency"));
  _loggroup->addTab(_bncFigurePPP,tr("PPP Plot"));

  // Netowork (Proxy and SSL) Tab
  // ----------------------------
  QGridLayout* pLayout = new QGridLayout;
  pLayout->setColumnMinimumWidth(0,13*ww);
  _proxyPortLineEdit->setMaximumWidth(9*ww);

  pLayout->addWidget(new QLabel("Settings for proxy in protected networks and for SSL authorization, leave boxes blank if none.<br>"),0, 0, 1, 50);
  pLayout->addWidget(new QLabel("Proxy host"),                               1, 0);
  pLayout->addWidget(_proxyHostLineEdit,                                     1, 1, 1,10);
  pLayout->addWidget(new QLabel("Proxy port"),                               2, 0);
  pLayout->addWidget(_proxyPortLineEdit,                                     2, 1);
  pLayout->addWidget(new QLabel("Path to SSL certificates"),                 3, 0);
  pLayout->addWidget(_sslCaCertPathLineEdit,                                 3, 1, 1,10);
  pLayout->addWidget(new QLabel("Default:  " + bncSslConfig::defaultPath()), 3,11, 1,20);
  pLayout->addWidget(new QLabel("Ignore SSL authorization errors"),          4, 0);
  pLayout->addWidget(_sslIgnoreErrorsCheckBox,                               4, 1, 1,10);
  pLayout->addWidget(new QLabel(""),                                         5, 1);
  pLayout->setRowStretch(6, 999);

  pgroup->setLayout(pLayout);

  // General Tab
  // -----------
  QGridLayout* gLayout = new QGridLayout;
  gLayout->setColumnMinimumWidth(0,14*ww);
  _onTheFlyComboBox->setMaximumWidth(9*ww);

  gLayout->addWidget(new QLabel("General settings for logfile, file handling, configuration on-the-fly, auto-start, and raw file output.<br>"),0, 0, 1, 50);
  gLayout->addWidget(new QLabel("Logfile (full path)"),          1, 0);
  gLayout->addWidget(_logFileLineEdit,                           1, 1, 1,20);
  gLayout->addWidget(new QLabel("Append files"),                 2, 0);
  gLayout->addWidget(_rnxAppendCheckBox,                         2, 1);
  gLayout->addWidget(new QLabel("Reread configuration"),         3, 0);
  gLayout->addWidget(_onTheFlyComboBox,                          3, 1);
  gLayout->addWidget(new QLabel("Auto start"),                   4, 0);
  gLayout->addWidget(_autoStartCheckBox,                         4, 1);
  gLayout->addWidget(new QLabel("Raw output file (full path)"),  5, 0);
  gLayout->addWidget(_rawOutFileLineEdit,                        5, 1, 1,20);
  gLayout->addWidget(new QLabel(""),                             6, 1);
  gLayout->setRowStretch(7, 999);

  ggroup->setLayout(gLayout);

  // RINEX Observations
  // ------------------
  QGridLayout* oLayout = new QGridLayout;
  oLayout->setColumnMinimumWidth(0,14*ww);
  _rnxIntrComboBox->setMaximumWidth(9*ww);
  _rnxSamplComboBox->setMaximumWidth(9*ww);

  oLayout->addWidget(new QLabel("Saving RINEX observation files.<br>"),0, 0, 1,50);
  oLayout->addWidget(new QLabel("Directory"),                      1, 0);
  oLayout->addWidget(_rnxPathLineEdit,                             1, 1, 1, 15);
  oLayout->addWidget(new QLabel("Interval"),                       2, 0);
  oLayout->addWidget(_rnxIntrComboBox,                             2, 1);
  oLayout->addWidget(new QLabel("  Sampling"),                     2, 2, Qt::AlignRight);
  oLayout->addWidget(_rnxSamplComboBox,                            2, 3, Qt::AlignLeft);
  oLayout->addWidget(new QLabel("Skeleton extension"),             3, 0);
  oLayout->addWidget(_rnxSkelLineEdit,                             3, 1, Qt::AlignLeft);
  oLayout->addWidget(new QLabel("Skeleton mandatory"),             3, 2);
  oLayout->addWidget(_rnxFileCheckBox,                             3, 3);
  oLayout->addWidget(new QLabel("Script (full path)"),             4, 0);
  oLayout->addWidget(_rnxScrpLineEdit,                             4, 1, 1, 15);
  oLayout->addWidget(new QLabel("Version 2 signal priority"),      5, 0);
  oLayout->addWidget(_rnxV2Priority,                               5, 1, 1, 15);
  oLayout->addWidget(new QLabel("Version 3"),                      6, 0);
  oLayout->addWidget(_rnxV3CheckBox,                               6, 1);
  oLayout->addWidget(new QLabel(""),                               7, 1);
  oLayout->setRowStretch(8, 999);

  ogroup->setLayout(oLayout);

  // RINEX Ephemeris
  // ---------------
  QGridLayout* eLayout = new QGridLayout;
  eLayout->setColumnMinimumWidth(0,14*ww);
  _ephIntrComboBox->setMaximumWidth(9*ww);
  _ephOutPortLineEdit->setMaximumWidth(9*ww);

  eLayout->addWidget(new QLabel("Saving RINEX navigation files and ephemeris output through IP port.<br>"),0,0,1,70);
  eLayout->addWidget(new QLabel("Directory"),                     1, 0);
  eLayout->addWidget(_ephPathLineEdit,                            1, 1, 1,30);
  eLayout->addWidget(new QLabel("Interval"),                      2, 0);
  eLayout->addWidget(_ephIntrComboBox,                            2, 1);
  eLayout->addWidget(new QLabel("Port"),                          3, 0);
  eLayout->addWidget(_ephOutPortLineEdit,                         3, 1);
  eLayout->addWidget(new QLabel("Version 3"),                     4, 0);
  eLayout->addWidget(_ephV3CheckBox,                              4, 1);
  eLayout->setRowStretch(5, 999);

  egroup->setLayout(eLayout);


  // Broadcast Corrections
  // ---------------------
  QGridLayout* cLayout = new QGridLayout;
  cLayout->setColumnMinimumWidth(0,14*ww);
  _corrIntrComboBox->setMaximumWidth(9*ww);
  _corrPortLineEdit->setMaximumWidth(9*ww);

  cLayout->addWidget(new QLabel("Saving Broadcast Ephemeris correction files and correction output through IP port.<br>"),0,0,1,70);
  cLayout->addWidget(new QLabel("Directory, ASCII"),              1, 0);
  cLayout->addWidget(_corrPathLineEdit,                           1, 1, 1,30);
  cLayout->addWidget(new QLabel("Interval"),                      2, 0);
  cLayout->addWidget(_corrIntrComboBox,                           2, 1);
  cLayout->addWidget(new QLabel("Port"),                          3, 0);
  cLayout->addWidget(_corrPortLineEdit,                           3, 1);
  cLayout->addWidget(new QLabel(""),                              4, 1);
  cLayout->setRowStretch(7, 999);
  cgroup->setLayout(cLayout);

  // Feed Engine
  // -----------
  QGridLayout* sLayout = new QGridLayout;
  sLayout->setColumnMinimumWidth(0,14*ww);
  _outPortLineEdit->setMaximumWidth(9*ww);
  _outWaitSpinBox->setMaximumWidth(9*ww);
  _outSamplComboBox->setMaximumWidth(9*ww);
  _outUPortLineEdit->setMaximumWidth(9*ww);

  sLayout->addWidget(new QLabel("Output decoded observations in ASCII format to feed a real-time GNSS network engine.<br>"),0,0,1,50);
  sLayout->addWidget(new QLabel("Port"),                            1, 0);
  sLayout->addWidget(_outPortLineEdit,                              1, 1);
  sLayout->addWidget(new QLabel("       Wait for full obs epoch"),  1, 2, Qt::AlignRight);
  sLayout->addWidget(_outWaitSpinBox,                               1, 3, Qt::AlignLeft);
  sLayout->addWidget(new QLabel("Sampling"),                        2, 0);
  sLayout->addWidget(_outSamplComboBox,                             2, 1, Qt::AlignLeft);
  sLayout->addWidget(new QLabel("File (full path)"),                3, 0);
  sLayout->addWidget(_outFileLineEdit,                              3, 1, 1, 10);
  sLayout->addWidget(new QLabel("Port (unsynchronized)"),           4, 0);
  sLayout->addWidget(_outUPortLineEdit,                             4, 1);
  sLayout->addWidget(new QLabel("Print lock time"),                 5, 0);
  sLayout->addWidget(_outLockTimeCheckBox,                        5, 1);
  sLayout->addWidget(new QLabel(""),                                6, 1);
  sLayout->setRowStretch(7, 999);

  sgroup->setLayout(sLayout);

  // Serial Output
  // -------------
  QGridLayout* serLayout = new QGridLayout;
  serLayout->setColumnMinimumWidth(0,12*ww);
  _serialBaudRateComboBox->setMaximumWidth(9*ww);
  _serialFlowControlComboBox->setMaximumWidth(11*ww);
  _serialDataBitsComboBox->setMaximumWidth(5*ww);
  _serialParityComboBox->setMaximumWidth(9*ww);
  _serialStopBitsComboBox->setMaximumWidth(5*ww);
  _serialAutoNMEAComboBox->setMaximumWidth(14*ww);
  _serialHeightNMEALineEdit->setMaximumWidth(8*ww);
  _serialManualNMEASamplingSpinBox->setMaximumWidth(8*ww);

  serLayout->addWidget(new QLabel("Port settings to feed a serial connected receiver.<br>"),0,0,1,30);
  serLayout->addWidget(new QLabel("Mountpoint"),                  1, 0, Qt::AlignLeft);
  serLayout->addWidget(_serialMountPointLineEdit,                 1, 1, 1, 2);
  serLayout->addWidget(new QLabel("Port name"),                   2, 0, Qt::AlignLeft);
  serLayout->addWidget(_serialPortNameLineEdit,                   2, 1, 1, 2);
  serLayout->addWidget(new QLabel("Baud rate"),                   3, 0, Qt::AlignLeft);
  serLayout->addWidget(_serialBaudRateComboBox,                   3, 1);
  serLayout->addWidget(new QLabel("Flow control"),                3, 2, Qt::AlignRight);
  serLayout->addWidget(_serialFlowControlComboBox,                3, 3);
  serLayout->addWidget(new QLabel("Data bits"),                   4, 0, Qt::AlignLeft);
  serLayout->addWidget(_serialDataBitsComboBox,                   4, 1);
  serLayout->addWidget(new QLabel("Parity"),                      4, 2, Qt::AlignRight);
  serLayout->addWidget(_serialParityComboBox,                     4, 3);
  serLayout->addWidget(new QLabel("   Stop bits"),                4, 4, Qt::AlignRight);
  serLayout->addWidget(_serialStopBitsComboBox,                   4, 5);
  serLayout->addWidget(new QLabel("NMEA"),                        5, 0);
  serLayout->addWidget(_serialAutoNMEAComboBox,                   5, 1);
  serLayout->addWidget(new QLabel("    File (full path)"),        5, 2, Qt::AlignRight);
  serLayout->addWidget(_serialFileNMEALineEdit,                   5, 3, 1,10);
  serLayout->addWidget(new QLabel("Height"),                      5,14, Qt::AlignRight);
  serLayout->addWidget(_serialHeightNMEALineEdit,                 5,15, 1,11);
  serLayout->addWidget(new QLabel("Sampling"),                    5,25, Qt::AlignRight);
  serLayout->addWidget(_serialManualNMEASamplingSpinBox,          5,26, 1,12);
  serLayout->addWidget(new QLabel(""),                            6, 1);
  serLayout->setRowStretch(7, 999);

  sergroup->setLayout(serLayout);

  // Outages
  // -------
  QGridLayout* aLayout = new QGridLayout;
  aLayout->setColumnMinimumWidth(0,14*ww);
  _adviseObsRateComboBox->setMaximumWidth(9*ww);
  _adviseFailSpinBox->setMaximumWidth(9*ww);
  _adviseRecoSpinBox->setMaximumWidth(9*ww);

  aLayout->addWidget(new QLabel("Failure and recovery reports, advisory notes.<br>"),0,0,1,50,Qt::AlignLeft);
  aLayout->addWidget(new QLabel("Observation rate"),              1, 0);
  aLayout->addWidget(_adviseObsRateComboBox,                      1, 1);
  aLayout->addWidget(new QLabel("Failure threshold"),             2, 0);
  aLayout->addWidget(_adviseFailSpinBox,                          2, 1);
  aLayout->addWidget(new QLabel("Recovery threshold"),            3, 0);
  aLayout->addWidget(_adviseRecoSpinBox,                          3, 1);
  aLayout->addWidget(new QLabel("Script (full path)"),            4, 0);
  aLayout->addWidget(_adviseScriptLineEdit,                       4, 1, 1,20);
  aLayout->addWidget(new QLabel(""),                              5, 1);
  aLayout->setRowStretch(6, 999);

  agroup->setLayout(aLayout);

  // Miscellaneous
  // -------------
  QGridLayout* rLayout = new QGridLayout;
  rLayout->setColumnMinimumWidth(0,14*ww);
  _miscIntrComboBox->setMaximumWidth(9*ww);
  _miscPortLineEdit->setMaximumWidth(9*ww);

  rLayout->addWidget(new QLabel("Log latencies or scan RTCM streams for message types and antenna information or output raw data through TCP/IP port.<br>"),0, 0,1,50);
  rLayout->addWidget(new QLabel("Mountpoint"),                    1, 0);
  rLayout->addWidget(_miscMountLineEdit,                          1, 1, 1, 7);
  rLayout->addWidget(new QLabel("Log latency"),                   2, 0);
  rLayout->addWidget(_miscIntrComboBox,                           2, 1);
  rLayout->addWidget(new QLabel("Scan RTCM"),                     3, 0);
  rLayout->addWidget(_miscScanRTCMCheckBox,                       3, 1);
  rLayout->addWidget(new QLabel("Port"),                          4, 0);
  rLayout->addWidget(_miscPortLineEdit,                           4, 1);
  rLayout->addWidget(new QLabel(""),                              5, 1);
  rLayout->setRowStretch(6, 999);

  rgroup->setLayout(rLayout);

  // PPP
  // ---
  _pppWidgets._dataSource->setMaximumWidth(15*ww);
  _pppWidgets._corrMount->setMaximumWidth(15*ww);
  _pppWidgets._nmeaPath->setMaximumWidth(35*ww);
  _pppWidgets._logPath->setMaximumWidth(35*ww);
  _pppWidgets._snxtroPath->setMaximumWidth(35*ww);
  _pppWidgets._snxtroIntr->setMaximumWidth(10*ww);
  _pppWidgets._snxtroAc  ->setMaximumWidth(10*ww);
  _pppWidgets._snxtroSol ->setMaximumWidth(7*ww);

  QGridLayout* pppLayout1 = new QGridLayout();
  int ir = 0;
  pppLayout1->addWidget(new QLabel("Precise Point Positioning - Input and Output.<br>"), ir, 0, 1, 7, Qt::AlignLeft);
  ++ir;
  pppLayout1->addWidget(new QLabel("Data source"),           ir, 0);
  pppLayout1->addWidget(_pppWidgets._dataSource,             ir, 1);
  pppLayout1->addWidget(new QLabel("   Version 3 filenames"),ir, 4);
  pppLayout1->addWidget(_pppWidgets._v3filenames,            ir, 5, 1, 3);
  ++ir;
  pppLayout1->addWidget(new QLabel("Corrections stream"),    ir, 0);
  pppLayout1->addWidget(_pppWidgets._corrMount,              ir, 1);
  pppLayout1->addWidget(new QLabel("Corrections file"),      ir, 2);
  pppLayout1->addWidget(_pppWidgets._corrFile,               ir, 3);
  pppLayout1->addWidget(new QLabel("   Logfile directory"),  ir, 4);
  pppLayout1->addWidget(_pppWidgets._logPath,                ir, 5, 1, 3);
  ++ir;
  pppLayout1->addWidget(new QLabel("RINEX Obs file"),        ir, 0);
  pppLayout1->addWidget(_pppWidgets._rinexObs,               ir, 1);
  pppLayout1->addWidget(new QLabel("RINEX Nav file"),        ir, 2);
  pppLayout1->addWidget(_pppWidgets._rinexNav,               ir, 3);
  pppLayout1->addWidget(new QLabel("   NMEA directory"),     ir, 4);
  pppLayout1->addWidget(_pppWidgets._nmeaPath,               ir, 5, 1, 3);
  ++ir;
  pppLayout1->addWidget(new QLabel("Coordinates file"),      ir, 0);
  pppLayout1->addWidget(_pppWidgets._crdFile,                ir, 1);
  pppLayout1->addWidget(new QLabel("ANTEX file"),            ir, 2);
  pppLayout1->addWidget(_pppWidgets._antexFile,              ir, 3);
  pppLayout1->addWidget(new QLabel("   SNX TRO directory"),  ir, 4);
  pppLayout1->addWidget(_pppWidgets._snxtroPath,             ir, 5, 1, 3);
  ++ir;
#ifdef USE_PPP
  pppLayout1->addWidget(new QLabel("BLQ file"),              ir, 0);
  pppLayout1->addWidget(_pppWidgets._blqFile,                ir, 1);
#endif
  pppLayout1->addWidget(new QLabel("   SNX TRO interval"),   ir, 4);
  pppLayout1->addWidget(_pppWidgets._snxtroIntr,             ir, 5);
  pppLayout1->addWidget(new QLabel("   SNX TRO sampling"),   ir, 6);
  pppLayout1->addWidget(_pppWidgets._snxtroSampl,            ir, 7, Qt::AlignRight);
  ++ir;
  pppLayout1->addWidget(new QLabel("   SNX TRO AC"),         ir, 4);
  pppLayout1->addWidget(_pppWidgets._snxtroAc,               ir, 5);
  pppLayout1->addWidget(new QLabel("   SNX TRO solution"),   ir, 6);
  pppLayout1->addWidget(_pppWidgets._snxtroSol,              ir, 7, Qt::AlignRight);
  pppLayout1->setRowStretch(ir+1, 999);
  pppGroup1->setLayout(pppLayout1);

  QVBoxLayout* pppLayout2 = new QVBoxLayout();
  pppLayout2->addWidget(new QLabel("Precise Point Positioning - Processed Stations.<br>"));
  pppLayout2->addWidget(_pppWidgets._staTable, 99);
  QHBoxLayout* pppLayout2sub = new QHBoxLayout();
  pppLayout2sub->addWidget(_pppWidgets._addStaButton);
  pppLayout2sub->addWidget(_pppWidgets._delStaButton);
  pppLayout2sub->addStretch(99);

  pppLayout2->addLayout(pppLayout2sub);

  pppGroup2->setLayout(pppLayout2);

  QGridLayout* pppLayout3 = new QGridLayout();
  ir = 0;
  pppLayout3->addWidget(new QLabel("Precise Point Positioning - Options.<br>"), ir, 0, 1, 2, Qt::AlignLeft);
  ++ir;
  pppLayout3->addWidget(new QLabel("GPS LCs"),              ir, 0, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._lcGPS,                 ir, 1);
  pppLayout3->addItem(new QSpacerItem(8*ww, 0),             ir, 2);
  pppLayout3->addWidget(new QLabel("Sigma C1"),             ir, 3, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._sigmaC1,               ir, 4); _pppWidgets._sigmaC1->setMaximumWidth(8*ww);
  pppLayout3->addItem(new QSpacerItem(8*ww, 0),             ir, 5);
  pppLayout3->addWidget(new QLabel("Sigma L1"),             ir, 6, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._sigmaL1,               ir, 7); _pppWidgets._sigmaL1->setMaximumWidth(8*ww);
  ++ir;
  pppLayout3->addWidget(new QLabel("GLONASS LCs"),          ir, 0, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._lcGLONASS,             ir, 1);
  pppLayout3->addWidget(new QLabel("Max Res C1"),           ir, 3, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._maxResC1,              ir, 4); _pppWidgets._maxResC1->setMaximumWidth(8*ww);
  pppLayout3->addWidget(new QLabel("Max Res L1"),           ir, 6, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._maxResL1,              ir, 7); _pppWidgets._maxResL1->setMaximumWidth(8*ww);
  ++ir;
  pppLayout3->addWidget(new QLabel("Galileo LCs"),          ir, 0, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._lcGalileo,             ir, 1);
  pppLayout3->addWidget(new QLabel("Ele Wgt Code"),         ir, 3, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._eleWgtCode,            ir, 4);
  pppLayout3->addWidget(new QLabel("Ele Wgt Phase"),        ir, 6, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._eleWgtPhase,           ir, 7);
  ++ir;
  pppLayout3->addWidget(new QLabel("BDS LCs"),              ir, 0, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._lcBDS,                 ir, 1);
  pppLayout3->addWidget(new QLabel("Min # of Obs"),         ir, 3, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._minObs,                ir, 4);
  pppLayout3->addWidget(new QLabel("Min Elevation"),        ir, 6, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._minEle,                ir, 7);
  ++ir;
  pppLayout3->addWidget(new QLabel("Model Obs"),            ir, 0, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._modelObs,              ir, 1);
  pppLayout3->addWidget(new QLabel("Wait for clock corr."), ir, 3, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._corrWaitTime,          ir, 4);
  pppLayout3->addWidget(new QLabel("Seeding (sec)"),        ir, 6, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._seedingTime,           ir, 7);
  ++ir;
  pppLayout3->addWidget(new QLabel("Pseudo Obs"),           ir, 0, Qt::AlignLeft);
  pppLayout3->addWidget(_pppWidgets._pseudoObs,             ir, 1);
  ++ir;
  pppLayout3->addWidget(new QLabel(""),                     ir, 8);
  pppLayout3->setColumnStretch(8, 999);
  ++ir;
  pppLayout3->addWidget(new QLabel(""),                     ir, 1);
  pppLayout3->setRowStretch(ir, 999);

  pppGroup3->setLayout(pppLayout3);

  // ------------------------
  connect(_pppWidgets._mapWinButton, SIGNAL(clicked()), SLOT(slotMapPPP()));
  _pppWidgets._mapSpeedSlider->setMinimumWidth(33*ww);
  _pppWidgets._audioResponse->setMaximumWidth(8*ww);

  QGridLayout* pppLayout4 = new QGridLayout();
  ir = 0;
  pppLayout4->addWidget(new QLabel("Precise Point Positioning - Plots.<br>"), ir, 0, 1, 50, Qt::AlignLeft);
  ++ir;
  pppLayout4->addWidget(new QLabel("PPP Plot"),                          ir, 0, Qt::AlignLeft);
  pppLayout4->addWidget(_pppWidgets._plotCoordinates,                    ir, 1, Qt::AlignLeft);
  pppLayout4->addWidget(new QLabel("Mountpoint"),                        ir, 2, 1, 10, Qt::AlignLeft);
  pppLayout4->addWidget(_pppWidgets._audioResponse,                      ir, 4, Qt::AlignLeft);
  pppLayout4->addWidget(new QLabel("Audio response"),                    ir, 5, Qt::AlignRight);

  ++ir;
  pppLayout4->addWidget(new QLabel("Track map"),                         ir, 0, Qt::AlignLeft);
  pppLayout4->addWidget(_pppWidgets._mapWinButton,                       ir, 1, Qt::AlignLeft);
  pppLayout4->addWidget(new QLabel("OSM"),                               ir, 2, Qt::AlignLeft);
  pppLayout4->addWidget(_pppWidgets._useOpenStreetMap,                   ir, 3, Qt::AlignLeft);
  pppLayout4->addWidget(new QLabel("Google"),                            ir, 3, Qt::AlignRight);
  pppLayout4->addWidget(_pppWidgets._useGoogleMap,                       ir, 4, Qt::AlignLeft);
  ++ir;
  pppLayout4->addWidget(new QLabel("Dot-properties"),                    ir, 0, Qt::AlignLeft);
  pppLayout4->addWidget(_pppWidgets._mapWinDotSize,                      ir, 1, Qt::AlignLeft);
  pppLayout4->addWidget(new QLabel("Size    "),                          ir, 2, Qt::AlignLeft);
  pppLayout4->addWidget(_pppWidgets._mapWinDotColor,                     ir, 3, Qt::AlignLeft);
  pppLayout4->addWidget(new QLabel("Color"),                             ir, 4, Qt::AlignLeft);
  ++ir;
  pppLayout4->addWidget(new QLabel("Post-processing speed"),             ir, 0, Qt::AlignLeft);
  pppLayout4->addWidget(_pppWidgets._mapSpeedSlider,                     ir, 1, 1, 20, Qt::AlignLeft);
  ++ir;
  pppLayout4->addWidget(new QLabel(""),                                  ir, 1);
  pppLayout4->setRowStretch(ir, 999);

  pppGroup4->setLayout(pppLayout4);

  // Reqc Processing
  // ---------------
  _reqcActionComboBox = new QComboBox();
  _reqcActionComboBox->setEditable(false);
  _reqcActionComboBox->addItems(QString(",Edit/Concatenate,Analyze").split(","));
  int ik = _reqcActionComboBox->findText(settings.value("reqcAction").toString());
  if (ik != -1) {
    _reqcActionComboBox->setCurrentIndex(ik);
  }
  connect(_reqcActionComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  QGridLayout* reqcLayout = new QGridLayout;
  _reqcActionComboBox->setMinimumWidth(15*ww);
  _reqcActionComboBox->setMaximumWidth(20*ww);

  _reqcObsFileChooser = new qtFileChooser(0, qtFileChooser::Files);
  _reqcObsFileChooser->setFileName(settings.value("reqcObsFile").toString());

  _reqcNavFileChooser = new qtFileChooser(0, qtFileChooser::Files);
  _reqcNavFileChooser->setFileName(settings.value("reqcNavFile").toString());
  _reqcOutObsLineEdit = new QLineEdit(settings.value("reqcOutObsFile").toString());
  _reqcOutNavLineEdit = new QLineEdit(settings.value("reqcOutNavFile").toString());
  _reqcOutLogLineEdit = new QLineEdit(settings.value("reqcOutLogFile").toString());
  _reqcPlotDirLineEdit = new QLineEdit(settings.value("reqcPlotDir").toString());
  _reqcSkyPlotSignals = new QLineEdit(settings.value("reqcSkyPlotSignals").toString());

  connect(_reqcSkyPlotSignals, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  _reqcLogSummaryOnly = new QCheckBox();
  _reqcLogSummaryOnly->setCheckState(Qt::CheckState(settings.value("reqcLogSummaryOnly").toInt()));

  ir = 0;
  reqcLayout->addWidget(new QLabel("RINEX file editing, concatenation and quality check.<br>"),ir, 0, 1, 8);
  ++ir;
  reqcLayout->addWidget(new QLabel("Action"),                    ir, 0);
  reqcLayout->addWidget(_reqcActionComboBox,                     ir, 1);
  _reqcEditOptionButton = new QPushButton("Set Edit Options");
  _reqcEditOptionButton->setMinimumWidth(15*ww);
  _reqcEditOptionButton->setMaximumWidth(20*ww);

  reqcLayout->addWidget(_reqcEditOptionButton,                   ir, 3);
  ++ir;
  reqcLayout->addWidget(new QLabel("Input files (full path)"),   ir, 0);
  reqcLayout->addWidget(_reqcObsFileChooser,                     ir, 1);
  _reqcObsFileChooser->setMaximumWidth(40*ww);
  reqcLayout->addWidget(new QLabel("  Obs"),                     ir, 2);
  reqcLayout->addWidget(_reqcNavFileChooser,                     ir, 3);
  _reqcNavFileChooser->setMaximumWidth(40*ww);
  reqcLayout->addWidget(new QLabel("  Nav"),                     ir, 4);
  ++ir;
  reqcLayout->addWidget(new QLabel("Output file (full path)"),   ir, 0);
  reqcLayout->addWidget(_reqcOutObsLineEdit,                     ir, 1);
  _reqcOutObsLineEdit->setMaximumWidth(40*ww);
  reqcLayout->addWidget(new QLabel("  Obs"),                     ir, 2);
  reqcLayout->addWidget(_reqcOutNavLineEdit,                     ir, 3);
  _reqcOutNavLineEdit->setMaximumWidth(40*ww);
  reqcLayout->addWidget(new QLabel("  Nav"),                     ir, 4);
  ++ir;
  reqcLayout->addWidget(new QLabel("Logfile"),                   ir, 0);
  reqcLayout->addWidget(_reqcOutLogLineEdit,                     ir, 1);
  _reqcOutLogLineEdit->setMaximumWidth(40*ww);
  reqcLayout->addWidget(new QLabel("  Summary only"),            ir, 2);
  reqcLayout->addWidget(_reqcLogSummaryOnly,                     ir, 3);
  ++ir;
  reqcLayout->addWidget(new QLabel("Plots for signals"),         ir, 0);
  reqcLayout->addWidget(_reqcSkyPlotSignals,                     ir, 1);
  _reqcSkyPlotSignals->setMaximumWidth(40*ww);
  ++ir;
  reqcLayout->addWidget(new QLabel("Directory for plots"),       ir, 0);
  reqcLayout->addWidget(_reqcPlotDirLineEdit,                    ir, 1);
  _reqcPlotDirLineEdit->setMaximumWidth(40*ww);
  ++ir;
  reqcLayout->setRowStretch(ir, 999);

  reqcLayout->setColumnMinimumWidth(2, 8*ww);
  reqcLayout->setColumnMinimumWidth(4, 8*ww);

  reqcgroup->setLayout(reqcLayout);

  connect(_reqcEditOptionButton, SIGNAL(clicked()),
          this, SLOT(slotReqcEditOption()));

  QGridLayout* sp3CompLayout = new QGridLayout;

  _sp3CompFileChooser = new qtFileChooser(0, qtFileChooser::Files);
  _sp3CompFileChooser->setFileName(settings.value("sp3CompFile").toString());
  _sp3CompFileChooser->setMinimumWidth(15*ww);
  _sp3CompFileChooser->setMaximumWidth(40*ww);

  _sp3CompExclude = new QLineEdit(settings.value("sp3CompExclude").toString());
  _sp3CompExclude->setMinimumWidth(18*ww);
  _sp3CompExclude->setMaximumWidth(18*ww);

  _sp3CompLogLineEdit = new QLineEdit(settings.value("sp3CompOutLogFile").toString());
  _sp3CompLogLineEdit->setMinimumWidth(18*ww);
  _sp3CompLogLineEdit->setMaximumWidth(18*ww);

  ir = 0;
  sp3CompLayout->addWidget(new QLabel("Orbit and clock comparison.<br>"),  ir, 0, 1, 40);
  ++ir;
  sp3CompLayout->addWidget(new QLabel("Input SP3 files (full path)"),  ir, 0, Qt::AlignLeft);
  sp3CompLayout->addWidget(_sp3CompFileChooser,                        ir, 1, 1, 20);
  ++ir;
  sp3CompLayout->addWidget(new QLabel("Exclude satellites"),           ir, 0, Qt::AlignLeft);
  sp3CompLayout->addWidget(_sp3CompExclude,                            ir, 1, Qt::AlignRight);
  ++ir;
  sp3CompLayout->addWidget(new QLabel("Logfile"),                      ir, 0, Qt::AlignLeft);
  sp3CompLayout->addWidget(_sp3CompLogLineEdit,                        ir, 1, Qt::AlignRight);
  ++ir;
  sp3CompLayout->addWidget(new QLabel(""),                             ir, 1);
  ++ir;
  sp3CompLayout->setRowStretch(ir, 999);

  sp3CompLayout->setColumnMinimumWidth(2, 8*ww);
  sp3CompLayout->setColumnMinimumWidth(4, 8*ww);

  sp3CompGroup->setLayout(sp3CompLayout);

  connect(_sp3CompFileChooser, SIGNAL(fileNameChanged(const QString &)),
          this, SLOT(slotBncTextChanged()));

  // Combine Corrections
  // -------------------
  QGridLayout* cmbLayout = new QGridLayout;

  populateCmbTable();
  cmbLayout->addWidget(_cmbTable,                                            0, 0, 8, 3);
  cmbLayout->addWidget(new QLabel(" Combine Broadcast Correction streams.<br>"), 1, 6, 1, 10);
  cmbLayout->addWidget(addCmbRowButton,                                      2, 6);
  cmbLayout->addWidget(delCmbRowButton,                                      2, 7);
  cmbLayout->addWidget(new QLabel("Method"),                                 3, 6, Qt::AlignRight);
  cmbLayout->addWidget(_cmbMethodComboBox,                                   3, 7);
  cmbLayout->addWidget(new QLabel(" Maximal residuum"),                      4, 6, Qt::AlignRight);
  cmbLayout->addWidget(_cmbMaxresLineEdit,                                   4, 7, Qt::AlignRight);
  cmbLayout->addWidget(new QLabel("Sampling"),                               5, 6, Qt::AlignRight);
  cmbLayout->addWidget(_cmbSamplSpinBox,                                     5, 7);
  cmbLayout->addWidget(new QLabel("   Use GLONASS"),                         6, 6, Qt::AlignRight);
  cmbLayout->addWidget(_cmbUseGlonass,                                       6, 7);
  cmbLayout->setRowStretch(7, 999);

  connect(addCmbRowButton, SIGNAL(clicked()), this, SLOT(slotAddCmbRow()));
  connect(delCmbRowButton, SIGNAL(clicked()), this, SLOT(slotDelCmbRow()));

  cmbgroup->setLayout(cmbLayout);

  // Upload Layout (Clocks)
  // ----------------------
  QGridLayout* uploadHlpLayout = new QGridLayout();

  connect(addUploadRowButton, SIGNAL(clicked()), this, SLOT(slotAddUploadRow()));
  connect(delUploadRowButton, SIGNAL(clicked()), this, SLOT(slotDelUploadRow()));
  connect(setUploadTrafoButton, SIGNAL(clicked()), this, SLOT(slotSetUploadTrafo()));

  uploadHlpLayout->addWidget(addUploadRowButton,                  0, 0);
  uploadHlpLayout->addWidget(delUploadRowButton,                  0, 1);
  uploadHlpLayout->addWidget(new QLabel("Interval"),              0, 2, Qt::AlignRight);
  uploadHlpLayout->addWidget(_uploadIntrComboBox,                 0, 3);
  uploadHlpLayout->addWidget(new QLabel("     Sampling:    Orb"), 0, 4, Qt::AlignRight);
  uploadHlpLayout->addWidget(_uploadSamplRtcmEphCorrSpinBox,      0, 5);
  uploadHlpLayout->addWidget(new QLabel("SP3"),                   0, 6, Qt::AlignRight);
  uploadHlpLayout->addWidget(_uploadSamplSp3SpinBox,              0, 7);
  uploadHlpLayout->addWidget(new QLabel("RNX"),                   0, 8, Qt::AlignRight);
  uploadHlpLayout->addWidget(_uploadSamplClkRnxSpinBox,           0, 9);
  uploadHlpLayout->addWidget(setUploadTrafoButton,                0,10);
  uploadHlpLayout->addWidget(new QLabel("ANTEX file"),            1, 0, Qt::AlignLeft);
  uploadHlpLayout->addWidget(_uploadAntexFile,                    1, 1, 1, 4);

  QBoxLayout* uploadLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  populateUploadTable();

  uploadLayout->addWidget(new QLabel("Upload RTCM Version 3 Broadcast Corrections to caster.<br>"));
  uploadLayout->addWidget(_uploadTable);
  uploadLayout->addLayout(uploadHlpLayout);

  uploadgroup->setLayout(uploadLayout);

  // Upload Layout (Ephemeris)
  // -------------------------
  QGridLayout* uploadHlpLayoutEph = new QGridLayout();

  connect(addUploadEphRowButton, SIGNAL(clicked()), this, SLOT(slotAddUploadEphRow()));
  connect(delUploadEphRowButton, SIGNAL(clicked()), this, SLOT(slotDelUploadEphRow()));

  uploadHlpLayoutEph->addWidget(addUploadEphRowButton,               0, 0);
  uploadHlpLayoutEph->addWidget(delUploadEphRowButton,               0, 1);
  uploadHlpLayoutEph->addWidget(new QLabel("     Sampling"),         0, 2, Qt::AlignRight);
  uploadHlpLayoutEph->addWidget(_uploadSamplRtcmEphSpinBox,          0, 3);

  QBoxLayout* uploadLayoutEph = new QBoxLayout(QBoxLayout::TopToBottom);
  populateUploadEphTable();

  uploadLayoutEph->addWidget(new QLabel("Upload concatenated RTCMv3 Broadcast Ephemeris to caster.<br>"));
  uploadLayoutEph->addWidget(_uploadEphTable);
  uploadLayoutEph->addLayout(uploadHlpLayoutEph);

  uploadEphgroup->setLayout(uploadLayoutEph);


  // Main Layout
  // -----------
  QGridLayout* mLayout = new QGridLayout;
  _aogroup->setCurrentIndex(settings.value("startTab").toInt());
  mLayout->addWidget(_aogroup,            0,0);
  mLayout->addWidget(_mountPointsTable,   1,0);
  _loggroup->setCurrentIndex(settings.value("statusTab").toInt());
  mLayout->addWidget(_loggroup,           2,0);

  _canvas->setLayout(mLayout);

  // WhatsThis, Network
  // ------------------
  _proxyHostLineEdit->setWhatsThis(tr("<p>If you are running BNC within a protected Local Area Network (LAN), you may need to use a proxy server to access the Internet. Enter your proxy server IP and port number in case one is operated in front of BNC. If you do not know the IP and port of your proxy server, check the proxy server settings in your Internet browser or ask your network administrator.</p><p>Note that IP streaming is sometimes not allowed in a LAN. In this case you need to ask your network administrator for an appropriate modification of the local security policy or for the installation of a TCP relay to the Ntrip Broadcasters. If this is not possible, you may need to run BNC outside your LAN on a network that has unobstructed connection to the Internet. <i>[key: proxyHost]</i></p>"));
  _proxyPortLineEdit->setWhatsThis(tr("<p>Enter your proxy server port number in case a proxy is operated in front of BNC. <i>[key: proxyPort]</i></p>"));
  _sslCaCertPathLineEdit->setWhatsThis(tr("<p>Communication with an Ntrip Broadcaster over SSL requires the exchange of client and/or server certificates. Specify the path to a directory where you save certificates on your system. Don't try communication via SSL if you are not sure whether this is supported by the involved Ntrip Broadcaster.</p><p>Note that SSL communication is usually done over port 443. <i>[key: sslCaCertPath]</i></p>"));
  _sslIgnoreErrorsCheckBox->setWhatsThis(tr("<p>SSL communication may involve queries coming from the Ntrip Broadcaster. Tick 'Ignore SSL authorization errors' if you don't want to be bothered with this. <i>[key: sslIgnoreErrors]</i></p>"));

  // WhatsThis, General
  // ------------------
  _logFileLineEdit->setWhatsThis(tr("<p>Records of BNC's activities are shown in the 'Log' tab on the bottom of this window. They can be saved into a file when a valid path for that is specified in the 'Logfile (full path)' field.</p><p>The logfile name will automatically be extended by a string '_YYMMDD' carrying the current date. <i>[key: logFile]</i></p>"));
  _rnxAppendCheckBox->setWhatsThis(tr("<p>When BNC is started, new files are created by default and file content already available under the same name will be overwritten. However, users might want to append already existing files following a regular restart or a crash of BNC or its platform.</p><p>Tick 'Append files' to continue with existing files and keep what has been recorded so far. <i>[key: rnxAppend]</i></p>"));
  _onTheFlyComboBox->setWhatsThis(tr("<p>When operating BNC online in 'no window' mode, some configuration parameters can be changed on-the-fly without interrupting the running process. For that BNC rereads parts of its configuration in pre-defined intervals. The default entry is 'no' that means the reread function is switched of. <p></p>Select '1 min', '5 min', '1 hour', or '1 day' to force BNC to reread its configuration every full minute, five minutes, hour, or day and let in between edited configuration options become effective on-the-fly without terminating uninvolved threads.</p><p>Note that when operating BNC in window mode, on-the-fly changeable configuration options become effective immediately via button 'Save & Reread Configuration'. <i>[key: onTheFlyInterval]</i></p>"));
  _autoStartCheckBox->setWhatsThis(tr("<p>Tick 'Auto start' for auto-start of BNC at startup time in window mode with preassigned processing options. <i>[key: autoStart]</i></p>"));
  _rawOutFileLineEdit->setWhatsThis(tr("<p>Save all data coming in through various streams in the received order and format in one file.</p><p>This option is primarily meant for debugging purposes. <i>[key: rawOutFile]</i></p>"));

  // WhatsThis, RINEX Observations
  // -----------------------------
  _rnxPathLineEdit->setWhatsThis(tr("<p>Here you specify the path to where the RINEX Observation files will be stored.</p><p>If the specified directory does not exist, BNC will not create RINEX Observation files. <i>[key: rnxPath]</i></p>"));
  _rnxIntrComboBox->setWhatsThis(tr("<p>Select the length of the RINEX Observation file. <i>[key: rnxIntr]</i></p>"));
  _rnxSamplComboBox->setWhatsThis(tr("<p>Select the RINEX Observation sampling interval in seconds. <i>[key: rnxSampl]</i></p>"));
  _rnxSkelLineEdit->setWhatsThis(tr("<p>BNC allows using personal RINEX skeleton files that contain the RINEX header records you would like to include. You can derive a skeleton file from information given in an up to date sitelog.</p><p>A file in the RINEX Observations 'Directory' with a 'Skeleton extension' skl or SKL is interpreted by BNC as a personal RINEX header skeleton file for the corresponding stream. <i>[key: rnxSkel]</i></p>"));
  _rnxFileCheckBox->setWhatsThis(tr("<p>Tick check box 'Skeleton mandatory' in case you want that RINEX files are only produced if skeleton files are available for BNC. If no skeleton file is available for a particular source then no RINEX Observation file will be produced from the affected stream.</p><p>Note that a skeleton file contains RINEX header information such as receiver and antenna types. In case of stream conversion to RINEX Version 3, a skeleton file should also contain information on potentially available observation types. A missing skeleton file will therefore enforce BNC to only save a default set of RINEX 3 observation types. <i>[key: rnxOnlyWithSKL]</i></p>"));
  _rnxScrpLineEdit->setWhatsThis(tr("<p>Whenever a RINEX Observation file is finally saved, you may want to compress, copy or upload it immediately, for example via FTP. BNC allows you to execute a script/batch file to carry out such operation.</p><p>Specify the full path of a script or batch file. BNC will pass the full RINEX Observation file path to the script as command line parameter (%1 on Windows systems, $1 on Unix/Linux/Mac systems). <i>[key: rnxScript]</i></p>"));
  _rnxV2Priority->setWhatsThis(tr("<p>Specify a priority list of characters defining signal attributes as defined in RINEX Version 3. Priorities will be used to map observations with RINEX Version 3 attributes from incoming streams to Version 2. The underscore character '_' stands for undefined attributes. A question mark '?' can be used as wildcard which represents any one character.</p><p>Signal priorities can be specified as equal for all systems, as system specific or as system and freq. specific. For example: </li><ul><li>'CWPX_?' (General signal priorities valid for all GNSS) </li><li>'I:ABCX' (System specific signal priorities for IRNSS) </li><li>'G:12&PWCSLX G:5&IQX R:12&PC R:3&IQX' (System and frequency specific signal priorities) </li></ul>Default is the following priority list 'G:12&PWCSLX G:5&IQX R:12&PC R:3&IQX R:46&ABX E:16&BCXZ E:578&IQX J:1&SLXCZ J:26&SLX J:5&IQX C:267&IQX C:128&DPX C:7&DPZ I:ABCX S:1&C S:5&IQX'. <i>[key: rnxV2Priority]</i></p>"));
  _rnxV3CheckBox->setWhatsThis(tr("<p>The default format for RINEX Observation files is RINEX Version 2.</p><p>Select 'Version 3' if you want to save observations in RINEX Version 3 format. <i>[key: rnxV3]</i></p>"));

  // WhatsThis, RINEX Ephemeris
  // --------------------------
  _ephPathLineEdit->setWhatsThis(tr("<p>Specify the path for saving Broadcast Ephemeris data as RINEX Navigation files.</p><p>If the specified directory does not exist, BNC will not create RINEX Navigation files. <i>[key: ephPath]</i></p>"));
  _ephIntrComboBox->setWhatsThis(tr("<p>Select the length of the RINEX Navigation file. <i>[key: ephIntr]</i></p>"));
  _ephOutPortLineEdit->setWhatsThis(tr("<p>BNC can produce ephemeris data in RINEX Navigation ASCII format on your local host through an IP port.</p><p>Specify a port number here to activate this function. <i>[key: ephOutPort]</i></p>"));
  _ephV3CheckBox->setWhatsThis(tr("<p>The default format for output of RINEX Navigation data containing Broadcast Ephemeris is RINEX Version 2.</p><p>Select 'Version 3' if you want to output ephemeris in RINEX Version 3 format. <i>[key: ephV3]</i></p>"));

  // WhatsThis, RINEX Editing & QC
  // -----------------------------
  _reqcActionComboBox->setWhatsThis(tr("<p>BNC allows to 'Edit or Concatenate' RINEX Version 2 or 3 files or to perform a Quality Check (QC) and 'Analyze' data following UNAVCO's famous 'teqc' program. <i>[key: reqcAction]</i></p>"));
  _reqcEditOptionButton->setWhatsThis(tr("<p>Specify options for editing RINEX Version 2 or 3 files.</p>"));
  _reqcObsFileChooser->setWhatsThis(tr("<p>Specify the full path to input observation files in RINEX Version 2 or 3 format.</p><p>Note that when in 'Analyze' mode, specifying at least one RINEX observation file is mandatory. <i>[key: reqcObsFile]</i></p>"));
  _reqcNavFileChooser->setWhatsThis(tr("<p>Specify the full path to input navigation files in RINEX Version 2 or 3 format.</p><p>Note that when in 'Analyze' mode, specifying at least one RINEX navigation file is mandatory. <i>[key: reqcNavFile]</i></p>"));
  _reqcOutObsLineEdit->setWhatsThis(tr("<p>Specify the full path to a RINEX Observation output file.</p><p>Default is an empty option field, meaning that no RINEX Observation output file will be produced. <i>[key: reqcOutObsFile]</i></p>"));
  _reqcOutNavLineEdit->setWhatsThis(tr("<p>Specify the full path to a RINEX Navigation output file.</p><p>Default is an empty option field, meaning that no RINEX Navigation output file will be produced. <i>[key: reqcOutNavFile]</i></p>"));
  _reqcOutLogLineEdit->setWhatsThis(tr("<p>Specify the full path to a logfile.</p><p>Default is an empty option field, meaning that no 'RINEX Editing & QC' logfile will be produced. <i>[key: reqcOutLogFile]</i></p>"));
  _reqcLogSummaryOnly->setWhatsThis(tr("<p>By default BNC produces a detailed 'Logfile' providing all information resulting from editing or analyzing RINEX data. If that is too much information, you can limit the logfile content to a short summary.</p><p>Tick 'Summary only' to suppress full logfile output and instead produce a logfile containing only summary information. <i>[key: reqcLogSummaryOnly]</i></p>"));
  _reqcPlotDirLineEdit->setWhatsThis(tr("<p>Specify a directory for saving plots in PNG format.</p><p>Default is an empty option field, meaning that plots will not be saved on disk. <i>[key: reqcPlotDir]</i></p>"));
  _reqcSkyPlotSignals->setWhatsThis(tr("<p>BNC can produce plots for multipath, signal-to-noise ratio, satellite availability, satellite elevation, and PDOP values. The 'Plots for signals' option lets you exactly specify observation signals to be used for that and also enables the plot generation. You can specify the navigation system, the frequency, and the tracking mode or channel as defined in RINEX Version 3. Specifications for frequency and tracking mode or channel must be separated by ampersand character '&'. Specifications for navigation systems must be separated by blank character ' '.</p><p>Examples for 'Plots for signals' option:<ul><li> G:1&2&5 R:1&2&3 J:1&2 E:1&7 C:1&6 S:1<br>(Plots will be based on GPS observations on 1st and 2nd frequency, GLONASS observations on 1st and 2nd frequency, QZSS observations on 1st and 2nd frequency, Galileo observations on 1st and 7th frequency, BeiDou observations on 1st and 6th frequency, SBAS observations on 1st frequency.)</li><li>G:1C&5X<br>(Plots will be based on GPS observations on 1st frequency in C tracking mode and GPS observations on 5th frequency in X tracking mode.)</li><li>C:6I&7I<br>(Plots will be based on BeiDou observations on 6th frequency in I tracking mode and BeiDou observations on 7th frequency in I tracking mode.)<li></ul></p><p>Default is 'C:2&7 E:1&5 G:1&2 J:1&2 R:1&2 S:1&5 I:5&9'. Specifying an empty option string would be overruled by this default. <i>[key: reqcSkyPlotSignals]</i></p>"));

  // WhatsThis, SP3 Comparison
  // -------------------------
  _sp3CompFileChooser->setWhatsThis(tr("<p>BNC can compare two SP3 files containing GNSS satellite orbit and clock information.</p></p>Specify the full path to two files with orbits and clocks in SP3 format, separate them by comma. <i>[key: sp3CompFile]</i></p>"));
  _sp3CompExclude->setWhatsThis(tr("<p>Specify satellites to exclude them from orbit and clock comparison. Example:<p>G04,G31,R</p><p>This excludes GPS satellites PRN 4 and 31 as well as all GLONASS satellites from the comparison.</p><p>Default is an empty option field, meaning that no satellite is excluded from the comparison. <i>[key: sp3CompExclude]</i></p>"));
  _sp3CompLogLineEdit->setWhatsThis(tr("<p>Specify the full path to a logfile saving comparison results.</p><p>Specifying a logfile is mandatory. Comparing SP3 files and not saving comparison results on disk would be useless. <i>[key: sp3CompOutLogFile]</i></p>"));

  // WhatsThis, Broadcast Corrections
  // --------------------------------
  _corrPathLineEdit->setWhatsThis(tr("<p>Specify a directory for saving Broadcast Ephemeris Correction files.</p><p>If the specified directory does not exist, BNC will not create the files. <i>[key: corrPath]</i></p>"));
  _corrIntrComboBox->setWhatsThis(tr("<p>Select the length of Broadcast Ephemeris Correction files. <i>[key: corrIntr]</i></p>"));
  _corrPortLineEdit->setWhatsThis(tr("<p>BNC can produce Broadcast Ephemeris Corrections on your local host through an IP port.</p><p>Specify a port number here to activate this function. <i>[key: corrPort]</i></p>"));

  // WhatsThis, Feed Engine
  // ----------------------
  _outPortLineEdit->setWhatsThis(tr("<p>BNC can produce synchronized observations in a plain ASCII format on your local host via IP port.</p><p>Specify a port number to activate this function. <i>[key: outPort]</i></p>"));
  _outWaitSpinBox->setWhatsThis(tr("<p>When feeding a real-time GNSS network engine waiting for synchronized input epoch by epoch, BNC drops whatever is received later than 'Wait for full obs epoch' seconds.</p><p>A value of 3 to 5 seconds is recommended, depending on the latency of the incoming streams and the delay acceptable to your real-time GNSS network engine or product. <i>[key: outWait]</i></p>"));
  _outSamplComboBox->setWhatsThis(tr("<p>Select a synchronized observation sampling interval in seconds. <i>[key: outSampl]</i></p>"));
  _outFileLineEdit->setWhatsThis(tr("<p>Specify the full path to a file where synchronized observations are saved in plain ASCII format.</p><p>Beware that the size of this file can rapidly increase depending on the number of incoming streams. <i>[key: outFile]</i></p>"));
  _outUPortLineEdit->setWhatsThis(tr("<p>BNC can produce unsynchronized observations in a plain ASCII format on your local host via IP port.</p><p>Specify a port number to activate this function. <i>[key: outUPort]</i></p>"));
  _outLockTimeCheckBox->setWhatsThis(tr("<p>Print the lock time in seconds in the feed engine output.<i>[key: outLockTime]</i></p>"));

  // WhatsThis, Serial Output
  // ------------------------
  _serialMountPointLineEdit->setWhatsThis(tr("<p>Enter a 'Mountpoint' to forward the corresponding stream to a serial connected receiver.</p><p>Depending on the stream content, the receiver may use it for example for Differential GNSS, Precise Point Positioning or any other purpose supported by its firmware. <i>[key: serialMountPoint]</i></p>"));
  _serialPortNameLineEdit->setWhatsThis(tr("<p>Enter the serial 'Port name' selected for communication with your serial connected receiver. Valid port names are</p><pre>Windows:       COM1, COM2<br>Linux:         /dev/ttyS0, /dev/ttyS1<br>FreeBSD:       /dev/ttyd0, /dev/ttyd1<br>Digital Unix:  /dev/tty01, /dev/tty02<br>HP-UX:         /dev/tty1p0, /dev/tty2p0<br>SGI/IRIX:      /dev/ttyf1, /dev/ttyf2<br>SunOS/Solaris: /dev/ttya, /dev/ttyb</pre><p>Note that before you start BNC, you must plug a serial cable in the port defined here. <i>[key: serialPortName]</i></p>"));
  _serialBaudRateComboBox->setWhatsThis(tr("<p>Select a 'Baud rate' for the serial output link.</p><p>Note that your selection must equal the baud rate configured to the serial connected receiver. Using a high baud rate is recommended. <i>[key: serialBaudRate]</i></p>"));
  _serialFlowControlComboBox->setWhatsThis(tr("<p>Select a 'Flow control' for the serial output link.</p><p>Note that your selection must equal the flow control configured to the serial connected receiver. Select 'OFF' if you don't know better. <i>[key: serialFlowControl]</i></p>"));
  _serialDataBitsComboBox->setWhatsThis(tr("<p>Select the number of 'Data bits' for the serial output link.</p><p>Note that your selection must equal the number of data bits configured to the serial connected receiver. Note further that often 8 data bits are used. <i>[key: serialDataBits]</i></p>"));
  _serialParityComboBox->setWhatsThis(tr("<p>Select a 'Parity' for the serial output link.</p><p>Note that your selection must equal the parity selection configured to the serial connected receiver. The parity is often set to 'NONE'. <i>[key: serialParity]</i></p>"));
  _serialStopBitsComboBox->setWhatsThis(tr("<p>Select the number of 'Stop bits' for the serial output link.</p><p>Note that your selection must equal the number of stop bits configured to the serial connected receiver. Note further that often 1 stop bit is used. <i>[key: serialStopBits]</i></p>"));
  _serialAutoNMEAComboBox->setWhatsThis(tr("<p>The 'NMEA' option supports the so-called 'Virtual Reference Station' (VRS) concept which requires the receiver to send approximate position information to the Ntrip Broadcaster. Select 'no' if you don't want BNC to forward or upload any NMEA message to the Ntrip Broadcaster in support of VRS.</p><p>Select 'Auto' to automatically forward NMEA messages of type GGA from your serial connected receiver to the Ntrip Broadcaster and/or save them in a file.</p><p>Select 'Manual GPGGA' or 'Manual GNGGA' if you want BNC to produce and upload GPGGA or GNGGA NMEA messages to the Ntrip Broadcaster because your serial connected receiver doesn't generate these messages. A Talker ID 'GP' preceding the GGA string stands for GPS solutions while a Talker ID 'GN' stands for multi constellation solutions.</p><p>Note that selecting 'Auto' or 'Manual' works only for VRS streams which show up under the 'Streams' canvas on BNC's main window with 'nmea' stream attribute set to 'yes'. This attribute is either extracted from the Ntrip Broadcaster's source-table or introduced by the user via editing the BNC configuration file. <i>[key: serialAutoNMEA]</i></p>"));
  _serialFileNMEALineEdit->setWhatsThis(tr("<p>Specify the full path to a file where NMEA messages coming from your serial connected receiver are saved.</p><p>Default is an empty option field, meaning that NMEA messages will not be saved on disk. <i>[key: serialFileNMEA]</i></p>"));
  _serialHeightNMEALineEdit->setWhatsThis(tr("<p>Specify an approximate 'Height' above mean sea level in meters for the reference station introduced by option 'Mountpoint'. Together with the latitude and longitude from the source-table, the height information is used to build GGA messages to be sent to the Ntrip Broadcaster.</p><p>For adjusting latitude and longitude values of a VRS stream given in the 'Streams' canvas, you can double click the latitude/longitude data fields, specify appropriate values and then hit Enter.</p><p>This option is only relevant when option 'NMEA' is set to 'Manual GPGGA' or 'Manual GNGGA' respectively. <i>[key: serialHeightNMEA]</i></p>"));
  _serialManualNMEASamplingSpinBox->setWhatsThis(tr("<p>Select a sampling interval in seconds for manual generation and upload of NMEA GGA sentences.</p><p>A sampling rate of '0' means, the a GGA sentence will be send only once to initialize the requested VRS stream. Note that some VRS systems need GGA sentences at regular intervals. <i>[key: serialManualNMEASampling]</i></p>"));

  // WhatsThis, Outages
  // ------------------
  _adviseObsRateComboBox->setWhatsThis(tr("<p>BNC can collect all returns (success or failure) coming from a decoder within a certain short time span to then decide whether a stream has an outage or its content is corrupted. The procedure needs a rough estimate of the expected 'Observation rate' of the incoming streams. When a continuous problem is detected, BNC can inform its operator about this event through an advisory note.</p><p>Default is an empty option field, meaning that you don't want BNC to report on stream failures or recoveries when exceeding a threshold time span. <i>[key: adviseObsRate]</i></p>"));
  _adviseFailSpinBox->setWhatsThis(tr("<p>An advisory note is generated when no (or only corrupted) observations are seen throughout the 'Failure threshold' time span. A value of 15 min (default) is recommended.</p><p>A value of zero '0' means that for any stream failure, however short, BNC immediately generates an advisory note. <i>[key: adviseFail]</i></p>"));
  _adviseRecoSpinBox->setWhatsThis(tr("<p>Following a stream outage or a longer series of bad observations, an advisory note is generated when valid observations are received again throughout the 'Recovery threshold' time span. A value of about 5min (default) is recommended.</p><p>A value of zero '0' means that for any stream recovery, however short, BNC immediately generates an advisory note. <i>[key: adviseReco]</i></p>"));
  _adviseScriptLineEdit->setWhatsThis(tr("<p>Specify the full path to a script or batch file to handle advisory notes generated in the event of corrupted streams or stream outages. The affected mountpoint and a comment 'Begin_Outage', 'End_Outage', 'Begin_Corrupted', or 'End_Corrupted' are passed on to the script as command line parameters.</p><p>The script may have the task to send the advisory notes by email to BNC's operator and/or to the affected stream provider.</p><p>An empty option field (default) or invalid path means that you don't want to use this option. <i>[key: adviseScript]</i></p>"));

  // WhatsThis, Miscellaneous
  // ------------------------
  _miscMountLineEdit->setWhatsThis(tr("<p>Specify a mountpoint to apply any of the options shown below. Enter 'ALL' if you want to apply these options to all configured streams.</p><p>An empty option field (default) means that you don't want BNC to apply any of these options. <i>[key: miscMount]</i></p>"));
  _miscIntrComboBox->setWhatsThis(tr("<p>BNC can average latencies per stream over a certain period of GPS time. The resulting mean latencies are recorded in the 'Log' tab at the end of each 'Log latency' interval together with results of a statistical evaluation (approximate number of covered epochs, data gaps).</p><p>Select a 'Log latency' interval or select the empty option field if you do not want BNC to log latencies and statistical information. <i>[key: miscIntr]</i></p>"));
  _miscScanRTCMCheckBox->setWhatsThis(tr("<p>Tick 'Scan RTCM' to log the numbers of incoming message types as well as contained antenna coordinates, antenna height, and antenna descriptor.</p><p>In case of RTCM Version 3 MSM streams, BNC will also log contained RINEX Version 3 observation types. <i>[key: miscScanRTCM]</i></p>"));
  _miscPortLineEdit->setWhatsThis(tr("<p>BNC can output an incoming stream through an IP port of your local host.</p><p>Specify a port number to activate this function. <i>[key: miscPort]</i></p>"));

  // WhatsThis, PPP (1)
  // ------------------
  _pppWidgets._dataSource->setWhatsThis(tr("<p>Select 'Real-time Streams' for real-time PPP from RTCM streams or 'RINEX Files' for post processing PPP from RINEX files.</p><p><ul><li>Real-time PPP requires that you pull a RTCM stream carrying GNSS observations plus a stream providing corrections to Broadcast Ephemeris. If the observations stream does not contain Broadcast Ephemeris then you must in addition pull a Broadcast Ephemeris stream like 'RTCM3EPH' from Ntrip Broadcaster <u>products.igs-ip.net</u>.<br></li><li>Post processing PPP requires RINEX Observation files, RINEX Navigation files and a file with corrections to Broadcast Ephemeris in plain ASCII format as saved beforehand using BNC.</li></ul></p><p>Note that BNC allows to carry out PPP solutions simultaneously for several stations.</p>"));
  _pppWidgets._rinexObs->setWhatsThis(tr("<p>Specify the RINEX Observation file.</p>"));
  _pppWidgets._rinexNav->setWhatsThis(tr("<p>Specify the RINEX Navigation file.</p>"));
  _pppWidgets._corrMount->setWhatsThis(tr("<p>Specify a 'mountpoint' from the 'Streams' canvas below which provides corrections to Broadcast Ephemeris.</p><p>If you don't specify a corrections stream via this option, BNC will fall back to Single Point Positioning (SPP, positioning from observations and Broadcast Ephemeris only) instead of doing PPP.</p>"));
  _pppWidgets._corrFile->setWhatsThis(tr("<p>Specify the Broadcast Ephemeris Corrections file as saved beforehand using BNC.</p><p>If you don't specify corrections by this option, BNC will fall back to Single Point Positioning (SPP, positioning from RINEX Obs and RINEX Nav files only) instead of doing PPP.</p>"));
  _pppWidgets._antexFile->setWhatsThis(tr("<p>Observations in RTCM streams or RINEX files should be referred to the receiver's and to the satellite's Antenna Phase Center (APC) and therefore be corrected for<ul><li>Receiver APC offsets</li><li>Receiver APC variations</li><li>Satellite APC offsets.</li></ul> Specify the full path to an IGS 'ANTEX file' which contains APC offsets and variations.</p><p>If you don't specify an 'ANTEX file' then observations will not be corrected for APC offsets and variations.</p>"));
  _pppWidgets._crdFile->setWhatsThis(tr("<p>Enter the full path to an ASCII file which specifies the streams or files of those stations you want to process. Specifying a 'Coordinates file' is optional. If it exists, it should contain one record per station with the following parameters separated by blank character:<p><ul><li>Specify the station either by<ul><li>the 'Mountpoint' of the station's RTCM stream (when in real-time PPP mode), or</li><li>the 4-charater station ID of the RINEX Observations file (when in post processing PPP mode).</li></ul></li><li>Approximate X,Y,Z coordinate of station's Antenna Reference Point [m] (ARP, specify '0.0 0.0 0.0' if unknown).</li><li>North, East and Up component of antenna eccentricity [m] (specify '0.0 0.0 0.0' if unknown).</li><li>20 Characters describing the antenna type and radome following the IGS 'ANTEX file' standard (leave blank if unknown).</li><li>Receiver type following the naming conventions for IGS equipment.</li></ul></p><p>Records with exclamation mark '!' in the first column or blank records will be interpreted as comment lines and ignored.</p>"));
  _pppWidgets._v3filenames->setWhatsThis(tr("<p>Tick 'Version 3 filenames' to let BNC create so-called extended filenames for PPP logfiles, NMEA files and SINEX Troposphere files following the RINEX Version 3 standard.</p><p>Default is an empty check box, meaning to create filenames following the RINEX Version 2 standard. The file content is not affected by this option. It only concerns the filenames.</p>"));
  _pppWidgets._logPath->setWhatsThis(tr("<p>Specify a directory for saving daily PPP logfiles. If the specified directory does not exist, BNC will not create such files.</p><p>Default is an empty option field, meaning that no PPP logfiles shall be produced.</p>"));
  _pppWidgets._nmeaPath->setWhatsThis(tr("<p>Specify a directory for saving coordinates in daily NMEA files. If the specified directory does not exist, BNC will not create such files.</p><p>Default is an empty option field, meaning that no NMEA file shall be produced.</p>"));
  _pppWidgets._snxtroPath->setWhatsThis(tr("<p>Specify a directory for saving SINEX Troposphere files. If the specified directory does not exist, BNC will not create such files.</p><p>Default is an empty option field, meaning that no SINEX Troposphere files shall be produced.</p>"));
  _pppWidgets._snxtroIntr->setWhatsThis(tr("<p>Select a length for SINEX Troposphere files.</p><p>Default 'SNX TRO interval' for saving SINEX Troposphere files on disk is '1 hour'.</p>"));
  _pppWidgets._snxtroSampl->setWhatsThis(tr("<p>Select a 'Sampling' rate for saving troposphere parameters.</p>"));
  _pppWidgets._snxtroAc->setWhatsThis(tr("<p>Specify a 3-character abbreviation describing you as the generating Analysis Center (AC) in your SINEX troposphere files.</p>"));
  _pppWidgets._snxtroSol->setWhatsThis(tr("<p>Specify a 4-character solution ID to allow a distingtion between different solutions per AC.</p>"));

  // WhatsThis, PPP (3)
  // ------------------
  _pppWidgets._lcGPS->setWhatsThis(tr("<p>Specify which kind of GPS observations you want to use and on which kind of ionosphere-free linear combination of GPS observations you want to base ambiguity resolutions.</p><p><ul><li>Specifying 'P3' means that you request BNC to use code data and so-called P3 ionosphere-free linear combination of code observations.</li><li>'P3&L3' means that you request BNC to use both, code and phase data and so-called P3 and L3 ionosphere-free linear combination of code and phase observations.</li></ul></p><p>Note that most geodetic receivers support the observation of GPS code and phase data. Hence specifying 'P3&L3' would be a good choice when processing data from such a receiver.</p><p>Specifying 'no' means that you don't want BNC to use GPS data. <i>[key: PPP/lcGPS]</i></p>"));
  _pppWidgets._lcGLONASS->setWhatsThis(tr("<p>Specify which kind of GLONASS observations you want to use and on which kind of ionosphere-free linear combination of GLONASS observations you want to base ambiguity resolutions.</p><p><ul><li>Specifying 'P3' means that you request BNC to use code data and so-called P3 ionosphere-free linear combination of code observations.</li><li>'L3' means that you request BNC to use phase data and so-called L3 ionosphere-free linear combination of phase observations.</li> <li>'P3&L3' means that you request BNC to use both, code and phase data and so-called P3 and L3 ionosphere-free linear combination of code and phase observations.</li></ul></p><p>Specifying 'no' means that you don't want BNC to use GLONASS data. <i>[key: PPP/lcGLONASS]</i></p>"));
  _pppWidgets._lcGalileo->setWhatsThis(tr("<p>Specify which kind of Galileo observations you want to use and on which kind of of ionosphere-free linear combination of Galileo observations you want to base ambiguity resolutions.</p><p><ul><li>Specifying 'P3' means that you request BNC to use code data and so-called P3 ionosphere-free linear combination of code observations.</li><li>'L3' means that you request BNC to use phase data and so-called L3 ionosphere-free linear combination of phase observations.</li> <li>'P3&L3' means that you request BNC to use both, code and phase data and so-called P3 and L3 ionosphere-free linear combination of code and phase observations.</li></ul></p><p>Specifying on of these options makes only sense if Galileo data are part of the processed observation stream.</p><p>Specifying 'no' means that you don't want BNC to use Galileo data. <i>[key: PPP/lcGalileo]</i></p>"));
  _pppWidgets._lcBDS->setWhatsThis(tr("<p>Specify which kind of model for observations you want to use and on which kind of ionosphere-free linear combination of BDS observations you want to base ambiguity resolutions.</p><p><ul><li>Specifying 'P3' means that you request BNC to use code data and so-called P3 ionosphere-free linear combination of code observations.</li><li>'L3' means that you request BNC to use phase data and so-called L3 ionosphere-free linear combination of phase observations.</li> <li>'P3&L3' means that you request BNC to use both, code and phase data and so-called P3 and L3 ionosphere-free linear combination of code and phase observations.</li></ul></p><p>Specifying on of these options makes only sense if BDS data are part of the processed observation stream.</p><p>Specifying 'no' means that you don't want to use BDS data. <i>[key: PPP/lcBDS]</i></p>"));
  _pppWidgets._modelObs->setWhatsThis(tr("<p>Specify which kind of PPP model you want to use. <i>[key: PPP/modelObs]</i></p>"));
  _pppWidgets._sigmaC1->setWhatsThis(tr("<p>Enter a Sigma for GNSS C1 code observations in meters.</p><p>The higher the sigma you enter, the less the contribution of C1 code observations to a PPP solution from combined code and phase data. 2.0 is likely to be an appropriate choice.</p><p>Default is an empty option field, meaning<br>'Sigma C1 = 2.0' <i>[key: PPP/sigmaC1]</i></p>"));
  _pppWidgets._sigmaL1->setWhatsThis(tr("<p>Enter a Sigma for GNSS L1 phase observations in meters.</p><p>The higher the sigma you enter, the less the contribution of L1 phase observations to a PPP solutions from combined code and phase data. 0.01 is likely to be an appropriate choice.</p><p>Default is an empty option field, meaning<br>'Sigma L1 = 0.01' <i>[key: PPP/sigmaL1]</i></p>"));
  _pppWidgets._maxResC1->setWhatsThis(tr("<p>Specify a maximum for residuals from C1 code observations in a PPP solution. '3.0' meters may be an appropriate choice for that.</p><p>If the maximum is exceeded, contributions from the corresponding observation will be ignored in the PPP solution.</p><p>Default is an empty option field, meaning<br>'Max Rex C1 = 3.0' <i>[key: PPP/maxResC1]</i></p>"));
  _pppWidgets._maxResL1->setWhatsThis(tr("<p>Specify a maximum for residuals from L1 code observations in a PPP solution. '0.03' meters may be an appropriate choice for that.</p><p>If the maximum is exceeded, contributions from the corresponding observation will be ignored in the PPP solution.</p><p>Default is an empty option field, meaning<br>'Max Rex L1 = 0.03' <i>[key: PPP/maxResL1]</i></p>"));
  _pppWidgets._eleWgtCode->setWhatsThis(tr("<p>Tic 'Ele Wgt Code' to use satellite Elevation depending Weights for Code observations in the PPP solution. <i>[key: PPP/eleWgtCode]</i></p>"));
  _pppWidgets._eleWgtPhase->setWhatsThis(tr("<p>Tic 'Ele Wgt Phase' to use satellite Elevation depending Weights for Phase observations in the PPP solution. <i>[key: PPP/eleWgtPhase]</i></p>"));
  _pppWidgets._minObs->setWhatsThis(tr("<p>Select a Minimum Number of Observations per epoch for a PPP solution.</p><p>BNC will only process epochs with observation numbers reaching or exceeding this minimum. <i>[key: PPP/minObs]</i></p>"));
  _pppWidgets._minEle->setWhatsThis(tr("<p>Select a Minimum satellite Elevation for observations.</p><p>BNC will ignore an observation if the associated satellite Elevation does not reach or exceed this minimum.</p><p>Selecting '10 deg' may be an appropriate choice in order to avoid too noisy observations. <i>[key: PPP/minEle]</i></p>"));

  // WhatsThis, Combine Corrections
  // ------------------------------
  _cmbTable->setWhatsThis(tr("<p>BNC allows to process several orbit and clock correction streams in real-time to produce, encode, upload and save a combination of correctors coming from different providers. To add a line to the 'Combine Corrections' table hit the 'Add Row' button, double click on the 'Mountpoint' field to specify a Broadcast Ephemeris Correction mountpoint from the 'Streams' section below and hit Enter. Then double click on the 'AC Name' field to enter your choice of an abbreviation for the Analysis Center (AC) providing the stream. Finally, double click on the 'Weight' field to enter a weight to be applied for this stream in the combination.</p><p>Note that the orbit information in the resulting combination stream is just copied from one of the incoming streams. The stream used for providing the orbits may vary over time: if the orbit providing stream has an outage then BNC switches to the next remaining stream for getting hold of the orbit information.</p><p>The combination process requires Broadcast Ephemeris. Besides orbit and clock correction streams BNC should therefore pull a stream carrying Broadcast Ephemeris in the form of RTCM Version 3 messages.</p><p>It is possible to specify only one Broadcast Ephemeris Correction stream in the 'Combine Corrections' table. Instead of combining corrections BNC will then add the corrections to the Broadcast Ephemeris with the possibility to save final orbit and clock results in SP3 and/or Clock RINEX format. <i>[key: cmbStreams]</i></p>"));
  addCmbRowButton->setWhatsThis(tr("<p>Hit 'Add Row' button to add another line to the 'Combine Corrections' table.</p>"));
  delCmbRowButton->setWhatsThis(tr("<p>Hit 'Delete' button to delete the highlighted line(s) from the 'Combine Corrections' table.</p>"));
  _cmbMethodComboBox->setWhatsThis(tr("<p>Select a clock combination approach. Options are 'Single-Epoch' and Kalman 'Filter'.</p><p>It is suggested to use the Kalman filter approach for the purpose of Precise Point Positioning. <i>[key: cmbMethod]</i></p>"));
  _cmbMaxresLineEdit->setWhatsThis(tr("<p>BNC combines all incoming clocks according to specified weights. Individual clock estimates that differ by more than 'Maximal residuum' meters from the average of all clocks will be ignored.<p></p>It is suggested to specify a value of about 0.2 m for the Kalman filter combination approach and a value of about 3.0 meters for the Single-Epoch combination approach.</p><p>Default is a value of '999.0'. <i>[key: cmbMaxres]</i></p>"));
  _cmbSamplSpinBox->setWhatsThis(tr("<p>Select a combination Sampling interval for the clocks. Clock corrections will be produced following that interval.</p><p>A value of 10 sec may be an appropriate choice. <i>[key: cmbSampl]</i></p>"));
  _cmbUseGlonass->setWhatsThis(tr("<p>In case the incoming orbit and clock correction stream(s) support GLONASS, you can tick 'Use GLONASS' to produce a GPS plus GLONASS combination solution.</p><p>Default is a GPS-only combination. <i>[key: cmbUseGlonass]</i></p>"));

  // WhatsThis, Upload Corrections
  // -----------------------------
  _uploadTable->setWhatsThis(tr("<p>BNC can upload clock and orbit corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 SSR format. You may have a situation where clocks and orbits come from an external Real-time Network Engine (1) or a situation where clock and orbit corrections are combined within BNC (2).</p><p>(1) BNC identifies a stream as coming from a Real-time Network Engine if its format is specified as 'RTNET' and hence its decoder string in the 'Streams' canvas is 'RTNET'. It encodes and uploads that stream to the specified Ntrip Broadcaster Host and Port</p><p>(2) BNC understands that it is expected to encode and upload combined Broadcast Ephemeris Corrections if you specify correction streams in the 'Combine Corrections' table.</p><p>To fill the 'Upload Corrections' table, hit the 'Add Row' button, double click on the 'Host' field to enter the IP or URL of an Ntrip Broadcaster and hit Enter. Select the Ntrip Version that shall be used for data upload. Then double click on the 'Port', 'Mount' and 'Password' fields to enter the Ntrip Broadcaster IP port (default is 80), the mountpoint and the stream upload password. If Ntrip Version 2 is chosen, click to the 'User' field to enter a stream upload user name. An empty 'Host' option field means that you don't want to upload corrections.</p><p>Select a target coordinate reference System (e.g. IGS14) for outgoing clock and orbit corrections.</p><p>By default orbit and clock corrections refer to Antenna Phase Center (APC). Tick 'CoM' to refer uploaded corrections to Center of Mass instead of APC.</p><p>Specify a path for saving generated Broadcast Corrections plus Broadcast Ephemeris as SP3 orbit files. If the specified directory does not exist, BNC will not create such files. The following is a path example for a Linux system: /home/user/BNC${GPSWD}.sp3<br>Note that '${GPSWD}' produces the GPS Week and Day number in the filename.</p><p>Specify a path for saving generated Broadcast Correction clocks plus Broadcast Ephemeris clocks as Clock RINEX files. If the specified directory does not exist, BNC will not create Clock RINEX files. The following is a path example for a Linux system: /home/user/BNC${GPSWD}.clk<br>Note that '${GPSWD}' produces the GPS Week and Day number in the filename.</p><p>Finally, specify a SSR Provider ID (issued by RTCM), SSR Solution ID, and SSR Issue of Data number.</p><p>In case the 'Combine Corrections' table contains only one Broadcast Correction stream, BNC will add that stream content to the Broadcast Ephemeris to save results in files specified via SP3 and/or Clock RINEX file path. You should then define only the SP3 and Clock RINEX file path and no further option in the 'Upload Corrections' table. <i>[key: uploadMountpointsOut]</i></p>"));
  addUploadRowButton->setWhatsThis(tr("<p>Hit 'Add Row' button to add another line to the 'Upload Corrections' table.</p>"));
  delUploadRowButton->setWhatsThis(tr("<p>Hit 'Del Row' button to delete the highlighted line(s) from the 'Upload Corrections' table.</p>"));
  _uploadIntrComboBox->setWhatsThis(tr("<p>Select the length of the SP3 and Clock RINEX files. <i>[key: uploadIntr]</i></p>"));
  _uploadSamplRtcmEphCorrSpinBox->setWhatsThis(tr("<p>Select a stream's orbit correction sampling interval in seconds.</p><p>A value of zero '0' tells BNC to upload all available orbit and clock correction samples together in combined messages. <i>[key: uploadSamplRtcmEphCorr]</i></p>"));
  _uploadSamplSp3SpinBox->setWhatsThis(tr("<p>Select a SP3 orbit file sampling interval in minutes.</p><p>A value of zero '0' tells BNC to store all available samples into SP3 orbit files. <i>[key: uploadSamplSp3]</i></p>"));
  _uploadSamplClkRnxSpinBox->setWhatsThis(tr("<p>Select a Clock RINEX file sampling interval in seconds.</p><p>A value of zero '0' tells BNC to store all available samples into Clock RINEX files. <i>[key: uploadSamplClkRnx]</i></p>"));
  setUploadTrafoButton->setWhatsThis(tr("<p>Hit 'Custom Trafo' to specify your own 14 parameter Helmert Transformation instead of selecting a predefined transformation via option 'System'.</p>"));
  _uploadAntexFile->setWhatsThis(tr("<p>When producing SP3 files or referring orbit and clock corrections to the satellite's Center of Mass (CoM) instead Antenna Phase Center (APC), an offset has to be applied which is available from the IGS 'ANTEX file'. You must therefore specify an 'ANTEX file' path if you want to save the stream content in SP3 format and/or refer correctors to CoM.</p><p>If you don't specify an 'ANTEX file' path, the SP3 file content as well as the orbit and clock correctors will be referred to satellite APCs. <i>[key: uploadAntexFile]</i></p>"));

  // WhatsThis, Upload Ephemeris
  // ---------------------------
  _uploadEphTable->setWhatsThis(tr("<p>BNC can upload a Broadcast Ephemeris stream in RTCM Version 3 format. Specify the IP number or URL of an Ntrip Broadcaster to upload the respective stream. <i>[key: uploadEphHost]</i></p>"));
  addUploadEphRowButton->setWhatsThis(tr("<p>Hit 'Add Row' button to add another line to the 'Upload Ephemeris' table.</p>"));
  delUploadEphRowButton->setWhatsThis(tr("<p>Hit 'Del Row' button to delete the highlighted line(s) from the 'Upload Ephemeris' table.</p>"));
  _uploadSamplRtcmEphSpinBox->setWhatsThis(tr("<p>Select the Broadcast Ephemeris sampling interval in seconds.</p><p>Default is '5', meaning that a complete set of Broadcast Ephemeris is uploaded every 5 seconds. <i>[key: uploadSamplRtcmEph]</i></p>"));

  // WhatsThis, Streams Canvas
  // -------------------------
  _mountPointsTable->setWhatsThis(tr("<p>Streams selected for retrieval are listed in the 'Streams' section. Clicking on 'Add Stream' button opens a window that allows the user to select data streams from an Ntrip Broadcaster according to their mountpoints. To remove a stream from the 'Streams' list, highlight it by clicking on it and hit the 'Delete Stream' button. You can also remove multiple streams by highlighting them using +Shift and +Ctrl.</p><p>BNC automatically allocates one of its internal decoders to a stream based on the stream's 'format' as given in the source-table. BNC allows users to change this selection by editing the decoder string. Double click on the 'decoder' field, enter your preferred decoder and then hit Enter. Accepted decoder strings are 'RTCM_2.x', 'RTCM_3.x' and 'RTNET'.</p><p>In case you need to log raw data as is, BNC allows to by-pass its decoders and directly save the input in daily log files. To do this specify the decoder string as 'ZERO'.</p><p>BNC can also retrieve streams from virtual reference stations (VRS). VRS streams are indicated by a 'yes' in the 'nmea' column. To initiate such stream, the approximate latitude/longitude rover position is sent to the Ntrip Broadcaster together with an approximation for the height. Default values for latitude and longitude can be change according to your requirement. Double click on 'lat' and 'long' fields, enter the values you wish to send and then hit Enter. <i>[key: mountPoints]</i></p>"));
  _actAddMountPoints->setWhatsThis(tr("<p>Add stream(s) to selection presented in the 'Streams' canvas.</p>"));
  _actDeleteMountPoints->setWhatsThis(tr("<p>Delete stream(s) from selection presented in the 'Streams' canvas.</p>"));
  _actMapMountPoints->setWhatsThis(tr("<p> Draw distribution map of stream selection presented in the 'Streams' canvas. Use mouse to zoom in or out.</p><p>Left button: Draw rectangle to zoom in.<br>Right button: Zoom out.<br>Middle button: Zoom back.</p>"));
  _actStart->setWhatsThis(tr("<p> Start running BNC.</p>"));
  _actStop->setWhatsThis(tr("<p> Stop running BNC.</p>"));

  // WhatsThis, Log Canvas
  // ---------------------
  _log->setWhatsThis(tr("<p>Records of BNC's activities are shown in the 'Log' tab. The message log covers the communication status between BNC and the Ntrip Broadcaster as well as problems that occur in the communication link, stream availability, stream delay, stream conversion etc.</p>"));
  _bncFigure->setWhatsThis(tr("<p>The bandwidth consumption per stream is shown in the 'Throughput' tab in bits per second (bps) or kilobits per second (kbps).</p>"));
  _bncFigureLate->setWhatsThis(tr("<p>The individual latency of observations of incoming streams is shown in the 'Latency' tab. Streams not carrying observations (e.g. those providing only Broadcast Ephemeris) remain unconsidered.</p><p>Note that the calculation of correct latencies requires the clock of the host computer to be properly synchronized.</p>"));
  _bncFigurePPP->setWhatsThis(tr("<p>PPP time series of North (red), East (green) and Up (blue) displacements are shown in the 'PPP Plot' tab when the corresponding option is selected.</p><p>Values are referred to an XYZ a priori coordinate. The sliding PPP time series window covers the period of the latest 5 minutes.</p>"));


  // Enable/Disable all Widgets
  // --------------------------
  slotBncTextChanged();
  enableStartStop();

  // Auto start
  // ----------
  if ( Qt::CheckState(settings.value("autoStart").toInt()) == Qt::Checked) {
    slotStart();
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncWindow::~bncWindow() {
  if (_caster) {
    delete _caster; BNC_CORE->setCaster(0);
  }
  if (_casterEph) {
    delete _casterEph;
  }
  delete _bncFigureLate;
  delete _bncFigurePPP;
  delete _actHelp;
  delete _actAbout;
  delete _actFlowchart;
  delete _actFontSel;
  delete _actSaveOpt;
  delete _actQuit;
  delete _actAddMountPoints;
  delete _actDeleteMountPoints;
  delete _actMapMountPoints;
  delete _actStart;
  delete _actStop;
  delete _actwhatsthis;
  delete _proxyHostLineEdit;
  delete _proxyPortLineEdit;
  delete _sslCaCertPathLineEdit;
  delete _sslIgnoreErrorsCheckBox;
  delete _logFileLineEdit;
  delete _rawOutFileLineEdit;
  delete _rnxAppendCheckBox;
  delete _onTheFlyComboBox;
  delete _autoStartCheckBox;
  delete _rnxPathLineEdit;
  delete _rnxIntrComboBox;
  delete _rnxSamplComboBox;
  delete _rnxFileCheckBox;
  delete _rnxSkelLineEdit;
  delete _rnxScrpLineEdit;
  delete _rnxV3CheckBox;
  delete _rnxV2Priority;
  delete _ephPathLineEdit;
  delete _ephIntrComboBox;
  delete _ephOutPortLineEdit;
  delete _ephV3CheckBox;
  delete _corrPathLineEdit;
  delete _corrIntrComboBox;
  delete _corrPortLineEdit;
  delete _outPortLineEdit;
  delete _outWaitSpinBox;
  delete _outSamplComboBox;
  delete _outFileLineEdit;
  delete _outUPortLineEdit;
  delete _outLockTimeCheckBox;
  delete _serialMountPointLineEdit;
  delete _serialPortNameLineEdit;
  delete _serialBaudRateComboBox;
  delete _serialFlowControlComboBox;
  delete _serialDataBitsComboBox;
  delete _serialParityComboBox;
  delete _serialStopBitsComboBox;
  delete _serialAutoNMEAComboBox;
  delete _serialFileNMEALineEdit;
  delete _serialHeightNMEALineEdit;
  delete _serialManualNMEASamplingSpinBox;
  delete _adviseObsRateComboBox;
  delete _adviseFailSpinBox;
  delete _adviseRecoSpinBox;
  delete _adviseScriptLineEdit;
  delete _miscMountLineEdit;
  delete _miscPortLineEdit;
  delete _miscIntrComboBox;
  delete _miscScanRTCMCheckBox;
  _mountPointsTable->deleteLater();
  delete _log;
  delete _loggroup;
  _cmbTable->deleteLater();
  delete _cmbMaxresLineEdit;
  delete _cmbUseGlonass;
  delete _cmbSamplSpinBox;
  delete _cmbMethodComboBox;;
  _uploadEphTable->deleteLater();
  delete _uploadSamplRtcmEphCorrSpinBox;
  _uploadTable->deleteLater();
  delete _uploadIntrComboBox;
  delete _uploadAntexFile;
  delete _uploadSamplRtcmEphSpinBox;
  delete _uploadSamplSp3SpinBox;
  delete _uploadSamplClkRnxSpinBox;
  delete _reqcActionComboBox;
  delete _reqcObsFileChooser;
  delete _reqcNavFileChooser;
  delete _reqcOutObsLineEdit;
  delete _reqcOutNavLineEdit;
  delete _reqcOutLogLineEdit;
  delete _reqcPlotDirLineEdit;
  delete _reqcSkyPlotSignals;
  delete _reqcLogSummaryOnly;
  delete _reqcEditOptionButton;
  delete _sp3CompFileChooser;
  delete _sp3CompExclude;
  delete _sp3CompLogLineEdit;
  //delete _canvas;
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::populateMountPointsTable() {

  for (int iRow = _mountPointsTable->rowCount()-1; iRow >=0; iRow--) {
    _mountPointsTable->removeRow(iRow);
  }

  bncSettings settings;

  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  int iRow = 0;
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    if (hlp.size() < 7) continue;
    _mountPointsTable->insertRow(iRow);

    QUrl    url(hlp[0]);

    QString fullPath = url.host() + QString(":%1").arg(url.port()) + url.path();
    QString format(hlp[1]); QString country(hlp[2]); QString latitude(hlp[3]); QString longitude(hlp[4]);
    QString nmea(hlp[5]);
    if (hlp[6] == "S") {
      fullPath = hlp[0].replace(0,2,"");
    }
    QString ntripVersion = "2";
    if (hlp.size() >= 7) {
      ntripVersion = (hlp[6]);
    }

    QTableWidgetItem* it;
    it = new QTableWidgetItem(url.userInfo());
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 0, it);

    it = new QTableWidgetItem(fullPath);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 1, it);

    it = new QTableWidgetItem(format);
    _mountPointsTable->setItem(iRow, 2, it);

    it = new QTableWidgetItem(country);
    _mountPointsTable->setItem(iRow, 3, it);

    if      (nmea == "yes") {
      it = new QTableWidgetItem(latitude);
      _mountPointsTable->setItem(iRow, 4, it);
      it = new QTableWidgetItem(longitude);
      _mountPointsTable->setItem(iRow, 5, it);
    } else {
      it = new QTableWidgetItem(latitude);
      it->setFlags(it->flags() & ~Qt::ItemIsEditable);
      _mountPointsTable->setItem(iRow, 4, it);

      it = new QTableWidgetItem(longitude);
      it->setFlags(it->flags() & ~Qt::ItemIsEditable);
      _mountPointsTable->setItem(iRow, 5, it);
    }

    it = new QTableWidgetItem(nmea);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 6, it);

    it = new QTableWidgetItem(ntripVersion);
    ////    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 7, it);

    bncTableItem* bncIt = new bncTableItem();
    bncIt->setFlags(bncIt->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 8, bncIt);

    iRow++;
  }

  _mountPointsTable->sortItems(1);

  enableStartStop();
}

// Retrieve Table
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAddMountPoints() {

  bncSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();
  if (proxyHost != _proxyHostLineEdit->text()         ||
      proxyPort != _proxyPortLineEdit->text().toInt()) {
    int iRet = QMessageBox::question(this, "Question", "Proxy options "
                                     "changed. Use the new ones?",
                                     QMessageBox::Yes, QMessageBox::No,
                                     QMessageBox::NoButton);
    if      (iRet == QMessageBox::Yes) {
      settings.setValue("proxyHost",   _proxyHostLineEdit->text());
      settings.setValue("proxyPort",   _proxyPortLineEdit->text());
    }
  }

  settings.setValue("sslCaCertPath",   _sslCaCertPathLineEdit->text());
  settings.setValue("sslIgnoreErrors", _sslIgnoreErrorsCheckBox->checkState());

  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setWindowTitle("Add Stream");
  msgBox.setText("Add stream(s) coming from:");

  QPushButton* buttonNtrip  = msgBox.addButton(tr("Caster"), QMessageBox::ActionRole);
  QPushButton* buttonIP     = msgBox.addButton(tr("TCP/IP port"), QMessageBox::ActionRole);
  QPushButton* buttonUDP    = msgBox.addButton(tr("UDP port"), QMessageBox::ActionRole);
  QPushButton* buttonSerial = msgBox.addButton(tr("Serial port"), QMessageBox::ActionRole);
  QPushButton* buttonCancel = msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);

  msgBox.exec();

  if (msgBox.clickedButton() == buttonNtrip) {
    bncTableDlg* dlg = new bncTableDlg(this);
    dlg->move(this->pos().x()+50, this->pos().y()+50);
    connect(dlg, SIGNAL(newMountPoints(QStringList*)),
          this, SLOT(slotNewMountPoints(QStringList*)));
    dlg->exec();
    delete dlg;
  } else if (msgBox.clickedButton() == buttonIP) {
    bncIpPort* ipp = new bncIpPort(this);
    connect(ipp, SIGNAL(newMountPoints(QStringList*)),
          this, SLOT(slotNewMountPoints(QStringList*)));
    ipp->exec();
    delete ipp;
  } else if (msgBox.clickedButton() == buttonUDP) {
    bncUdpPort* udp = new bncUdpPort(this);
    connect(udp, SIGNAL(newMountPoints(QStringList*)),
          this, SLOT(slotNewMountPoints(QStringList*)));
    udp->exec();
    delete udp;
  } else if (msgBox.clickedButton() == buttonSerial) {
    bncSerialPort* sep = new bncSerialPort(this);
    connect(sep, SIGNAL(newMountPoints(QStringList*)),
          this, SLOT(slotNewMountPoints(QStringList*)));
    sep->exec();
    delete sep;
  } else if (msgBox.clickedButton() == buttonCancel) {
    // Cancel
  }

  enableStartStop();
}

// Delete Selected Mount Points
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotDeleteMountPoints() {

  int nRows = _mountPointsTable->rowCount();
  bool flg[nRows];
  for (int iRow = 0; iRow < nRows; iRow++) {
    if (_mountPointsTable->isItemSelected(_mountPointsTable->item(iRow,1))) {
      flg[iRow] = true;
    }
    else {
      flg[iRow] = false;
    }
  }
  for (int iRow = nRows-1; iRow >= 0; iRow--) {
    if (flg[iRow]) {
      _mountPointsTable->removeRow(iRow);
    }
  }
  _actDeleteMountPoints->setEnabled(false);

  enableStartStop();
}

// New Mount Points Selected
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotNewMountPoints(QStringList* mountPoints) {
  int iRow = 0;
  QListIterator<QString> it(*mountPoints);
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    QUrl    url(hlp[0]);
    QString fullPath = url.host() + QString(":%1").arg(url.port()) + url.path();
    QString format(hlp[1]); QString country(hlp[2]); QString latitude(hlp[3]); QString longitude(hlp[4]);
    QString nmea(hlp[5]);
    if (hlp[6] == "S") {
      fullPath = hlp[0].replace(0,2,"");
    }
    QString ntripVersion = "2";
    if (hlp.size() >= 7) {
      ntripVersion = (hlp[6]);
    }

    _mountPointsTable->insertRow(iRow);

    QTableWidgetItem* it;
    it = new QTableWidgetItem(url.userInfo());
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 0, it);

    it = new QTableWidgetItem(fullPath);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 1, it);

    it = new QTableWidgetItem(format);
    _mountPointsTable->setItem(iRow, 2, it);

    it = new QTableWidgetItem(country);
    _mountPointsTable->setItem(iRow, 3, it);

    if      (nmea == "yes") {
    it = new QTableWidgetItem(latitude);
    _mountPointsTable->setItem(iRow, 4, it);
    it = new QTableWidgetItem(longitude);
    _mountPointsTable->setItem(iRow, 5, it);
    } else {
    it = new QTableWidgetItem(latitude);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 4, it);
    it = new QTableWidgetItem(longitude);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 5, it);
    }

    it = new QTableWidgetItem(nmea);
    it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 6, it);

    it = new QTableWidgetItem(ntripVersion);
    ////it->setFlags(it->flags() & ~Qt::ItemIsEditable);
    _mountPointsTable->setItem(iRow, 7, it);

    bncTableItem* bncIt = new bncTableItem();
    _mountPointsTable->setItem(iRow, 8, bncIt);

    iRow++;
  }
  _mountPointsTable->hideColumn(0);
  _mountPointsTable->hideColumn(3);
  _mountPointsTable->sortItems(1);
  delete mountPoints;

  enableStartStop();
}

// Save Options (serialize)
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotSaveOptions() {
  saveOptions();
  bncSettings settings;
  settings.sync();
}

// Save Options (memory only)
////////////////////////////////////////////////////////////////////////////
void bncWindow::saveOptions() {

  QStringList mountPoints;
  for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {

    if (_mountPointsTable->item(iRow, 6)->text() != "S") {
      QUrl url( "//" + _mountPointsTable->item(iRow, 0)->text() +
                "@"  + _mountPointsTable->item(iRow, 1)->text() );

      mountPoints.append(url.toString() + " " +
                         _mountPointsTable->item(iRow, 2)->text()
                 + " " + _mountPointsTable->item(iRow, 3)->text()
                 + " " + _mountPointsTable->item(iRow, 4)->text()
                 + " " + _mountPointsTable->item(iRow, 5)->text()
                 + " " + _mountPointsTable->item(iRow, 6)->text()
                 + " " + _mountPointsTable->item(iRow, 7)->text());
    } else {
      mountPoints.append(
                  "//" + _mountPointsTable->item(iRow, 1)->text()
                 + " " + _mountPointsTable->item(iRow, 2)->text()
                 + " " + _mountPointsTable->item(iRow, 3)->text()
                 + " " + _mountPointsTable->item(iRow, 4)->text()
                 + " " + _mountPointsTable->item(iRow, 5)->text()
                 + " " + _mountPointsTable->item(iRow, 6)->text()
                 + " " + _mountPointsTable->item(iRow, 7)->text());
    }
  }

  QStringList cmbStreams;
  for (int iRow = 0; iRow < _cmbTable->rowCount(); iRow++) {
    QString hlp;
    for (int iCol = 0; iCol < _cmbTable->columnCount(); iCol++) {
      if (_cmbTable->item(iRow, iCol)) {
        hlp += _cmbTable->item(iRow, iCol)->text() + " ";
      }
    }
    if (!hlp.isEmpty()) {
      cmbStreams << hlp;
    }
  }

  QStringList uploadMountpointsOut;
  for (int iRow = 0; iRow < _uploadTable->rowCount(); iRow++) {
    QString hlp;
    for (int iCol = 0; iCol < _uploadTable->columnCount(); iCol++) {
      if (_uploadTable->cellWidget(iRow, iCol) &&
          (iCol == 3 || iCol == 4 || iCol == 5 || iCol == 6 || iCol == 7)) {
        if       (iCol == 3) {
          QComboBox* ntripversion = (QComboBox*)(_uploadTable->cellWidget(iRow, iCol));
          hlp += ntripversion->currentText() + ",";
        }
        else if (iCol == 4 ) {
          QLineEdit* user = (QLineEdit*)(_uploadTable->cellWidget(iRow, iCol));
          hlp += user->text() + ",";
        }
        else if (iCol == 5) {
          QLineEdit* passwd = (QLineEdit*)(_uploadTable->cellWidget(iRow, iCol));
          hlp += passwd->text() + ",";
        }
        else if (iCol == 6) {
          QComboBox* system = (QComboBox*)(_uploadTable->cellWidget(iRow, iCol));
          hlp += system->currentText() + ",";
        }
        else if (iCol == 7) {
          QCheckBox* com    = (QCheckBox*)(_uploadTable->cellWidget(iRow, iCol));
          QString state; state.setNum(com->checkState());
          hlp +=  state + ",";
        }
      }
      else if (_uploadTable->item(iRow, iCol)) {
        hlp += _uploadTable->item(iRow, iCol)->text() + ",";
      }
    }
    if (!hlp.isEmpty()) {
      uploadMountpointsOut << hlp;
    }
  }

  QStringList uploadEphMountpointsOut;
  for (int iRow = 0; iRow < _uploadEphTable->rowCount(); iRow++) {
    QString hlp;
    for (int iCol = 0; iCol < _uploadEphTable->columnCount(); iCol++) {
      if (_uploadEphTable->cellWidget(iRow, iCol) &&
          (iCol == 3 || iCol == 4 || iCol == 5 || iCol == 6)) {
        if       (iCol == 3) {
          QComboBox* ntripversion = (QComboBox*)(_uploadEphTable->cellWidget(iRow, iCol));
          hlp += ntripversion->currentText() + ",";
        }
        else if (iCol == 4 ) {
          QLineEdit* user = (QLineEdit*)(_uploadEphTable->cellWidget(iRow, iCol));
          hlp += user->text() + ",";
        }
        else if (iCol == 5) {
          QLineEdit* passwd = (QLineEdit*)(_uploadEphTable->cellWidget(iRow, iCol));
          hlp += passwd->text() + ",";
        }
        else if (iCol == 6) {
          QComboBox* system = (QComboBox*)(_uploadEphTable->cellWidget(iRow, iCol));
          hlp += system->currentText() + ",";
        }
      }
      else if (_uploadEphTable->item(iRow, iCol)) {
        hlp += _uploadEphTable->item(iRow, iCol)->text() + ",";
      }
    }
    if (!hlp.isEmpty()) {
      uploadEphMountpointsOut << hlp;
    }
  }

  bncSettings settings;

  settings.setValue("startTab",    _aogroup->currentIndex());
  settings.setValue("statusTab",   _loggroup->currentIndex());
  settings.setValue("mountPoints", mountPoints);
// Network
  settings.setValue("proxyHost",   _proxyHostLineEdit->text());
  settings.setValue("proxyPort",   _proxyPortLineEdit->text());
  settings.setValue("sslCaCertPath",   _sslCaCertPathLineEdit->text());
  settings.setValue("sslIgnoreErrors",  _sslIgnoreErrorsCheckBox->checkState());
// General
  settings.setValue("logFile",     _logFileLineEdit->text());
  settings.setValue("rnxAppend",   _rnxAppendCheckBox->checkState());
  settings.setValue("onTheFlyInterval", _onTheFlyComboBox->currentText());
  settings.setValue("autoStart",   _autoStartCheckBox->checkState());
  settings.setValue("rawOutFile",  _rawOutFileLineEdit->text());
// RINEX Observations
  settings.setValue("rnxPath",      _rnxPathLineEdit->text());
  settings.setValue("rnxIntr",      _rnxIntrComboBox->currentText());
  settings.setValue("rnxSampl",     _rnxSamplComboBox->currentText());
  settings.setValue("rnxSkel",      _rnxSkelLineEdit->text());
  settings.setValue("rnxOnlyWithSKL",_rnxFileCheckBox->checkState());
  settings.setValue("rnxScript",    _rnxScrpLineEdit->text());
  settings.setValue("rnxV2Priority",_rnxV2Priority->text());
  settings.setValue("rnxV3",        _rnxV3CheckBox->checkState());
// RINEX Ephemeris
  settings.setValue("ephPath",       _ephPathLineEdit->text());
  settings.setValue("ephIntr",       _ephIntrComboBox->currentText());
  settings.setValue("ephOutPort",    _ephOutPortLineEdit->text());
  settings.setValue("ephV3",         _ephV3CheckBox->checkState());
// Broadcast Corrections
  settings.setValue("corrPath",    _corrPathLineEdit->text());
  settings.setValue("corrIntr",    _corrIntrComboBox->currentText());
  settings.setValue("corrPort",    _corrPortLineEdit->text());
// Feed Engine
  settings.setValue("outPort",     _outPortLineEdit->text());
  settings.setValue("outWait",     _outWaitSpinBox->value());
  settings.setValue("outSampl",    _outSamplComboBox->currentText());
  settings.setValue("outFile",     _outFileLineEdit->text());
  settings.setValue("outLockTime",_outLockTimeCheckBox->checkState());    settings.setValue("outUPort",    _outUPortLineEdit->text());

// Serial Output
  settings.setValue("serialMountPoint",_serialMountPointLineEdit->text());
  settings.setValue("serialPortName",  _serialPortNameLineEdit->text());
  settings.setValue("serialBaudRate",  _serialBaudRateComboBox->currentText());
  settings.setValue("serialFlowControl",_serialFlowControlComboBox->currentText());
  settings.setValue("serialDataBits",  _serialDataBitsComboBox->currentText());
  settings.setValue("serialParity",    _serialParityComboBox->currentText());
  settings.setValue("serialStopBits",  _serialStopBitsComboBox->currentText());
  settings.setValue("serialAutoNMEA",  _serialAutoNMEAComboBox->currentText());
  settings.setValue("serialFileNMEA",    _serialFileNMEALineEdit->text());
  settings.setValue("serialHeightNMEA",  _serialHeightNMEALineEdit->text());
  settings.setValue("serialManualNMEASampling", _serialManualNMEASamplingSpinBox->value());
// Outages
  settings.setValue("adviseObsRate", _adviseObsRateComboBox->currentText());
  settings.setValue("adviseFail",    _adviseFailSpinBox->value());
  settings.setValue("adviseReco",    _adviseRecoSpinBox->value());
  settings.setValue("adviseScript",  _adviseScriptLineEdit->text());
// Miscellaneous
  settings.setValue("miscMount",   _miscMountLineEdit->text());
  settings.setValue("miscPort",    _miscPortLineEdit->text());
  settings.setValue("miscIntr",    _miscIntrComboBox->currentText());
  settings.setValue("miscScanRTCM", _miscScanRTCMCheckBox->checkState());
// Reqc
  settings.setValue("reqcAction",     _reqcActionComboBox->currentText());
  settings.setValue("reqcObsFile",    _reqcObsFileChooser->fileName());
  settings.setValue("reqcNavFile",    _reqcNavFileChooser->fileName());
  settings.setValue("reqcOutObsFile", _reqcOutObsLineEdit->text());
  settings.setValue("reqcOutNavFile", _reqcOutNavLineEdit->text());
  settings.setValue("reqcOutLogFile", _reqcOutLogLineEdit->text());
  settings.setValue("reqcPlotDir",    _reqcPlotDirLineEdit->text());
  settings.setValue("reqcSkyPlotSignals", _reqcSkyPlotSignals->text());
  settings.setValue("reqcLogSummaryOnly", _reqcLogSummaryOnly->checkState());
// SP3 Comparison
  settings.setValue("sp3CompFile",       _sp3CompFileChooser->fileName());
  settings.setValue("sp3CompExclude",    _sp3CompExclude->text());
  settings.setValue("sp3CompOutLogFile", _sp3CompLogLineEdit->text());
// Combine Corrections
  if (!cmbStreams.isEmpty()) {
    settings.setValue("cmbStreams", cmbStreams);
  }
  else {
    settings.setValue("cmbStreams", "");
  }
  settings.setValue("cmbMethod",     _cmbMethodComboBox->currentText());
  settings.setValue("cmbMaxres",     _cmbMaxresLineEdit->text());
  settings.setValue("cmbSampl",      _cmbSamplSpinBox->value());
  settings.setValue("cmbUseGlonass", _cmbUseGlonass->checkState());
// Upload Corrections
  if (!uploadMountpointsOut.isEmpty()) {
    settings.setValue("uploadMountpointsOut", uploadMountpointsOut);
  }
  else {
    settings.setValue("uploadMountpointsOut", "");
  }
  settings.setValue("uploadIntr",             _uploadIntrComboBox->currentText());
  settings.setValue("uploadSamplRtcmEphCorr", _uploadSamplRtcmEphCorrSpinBox->value());
  settings.setValue("uploadSamplSp3",         _uploadSamplSp3SpinBox->value());
  settings.setValue("uploadSamplClkRnx",      _uploadSamplClkRnxSpinBox->value());
  settings.setValue("uploadAntexFile",        _uploadAntexFile->fileName());
// Upload Ephemeris
  if (!uploadEphMountpointsOut.isEmpty()) {
    settings.setValue("uploadEphMountpointsOut", uploadEphMountpointsOut);
  }
  else {
    settings.setValue("uploadEphMountpointsOut", "");
  }
  settings.setValue("uploadSamplRtcmEph", _uploadSamplRtcmEphSpinBox->value());

  if (_caster) {
    _caster->readMountPoints();
  }

  _pppWidgets.saveOptions();
}

// All get slots terminated
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotGetThreadsFinished() {
  BNC_CORE->slotMessage("All Get Threads Terminated", true);
  delete _caster;    _caster    = 0; BNC_CORE->setCaster(0);
  delete _casterEph; _casterEph = 0;
  _runningRealTime = false;
}

// Start It!
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotStart() {
  saveOptions();
  if      ( _pppWidgets._dataSource->currentText() == "RINEX Files") {
    _runningPPP = true;
    enableStartStop();
    _caster = new bncCaster(); BNC_CORE->setCaster(_caster);
    BNC_CORE->startPPP();
    _bncFigurePPP->reset();
  }
  else if ( !_reqcActionComboBox->currentText().isEmpty() ) {
    if (_reqcActionComboBox->currentText() == "Analyze") {
      _runningQC = true;
      t_reqcAnalyze* reqcAnalyze = new t_reqcAnalyze(this);
      connect(reqcAnalyze, SIGNAL(finished()), this, SLOT(slotPostProcessingFinished()));
      reqcAnalyze->start();
    }
    else {
      _runningEdit = true;
      t_reqcEdit* reqcEdit = new t_reqcEdit(this);
      connect(reqcEdit, SIGNAL(finished()), this, SLOT(slotPostProcessingFinished()));
      reqcEdit->start();
    }
    enableStartStop();
  }
  else if (!_sp3CompFileChooser->fileName().isEmpty()) {
    _runningSp3Comp = true;
    t_sp3Comp* sp3Comp = new t_sp3Comp(this);
    connect(sp3Comp, SIGNAL(finished()), this, SLOT(slotPostProcessingFinished()));
    sp3Comp->start();
    enableStartStop();
  }
  else {
    startRealTime();
    BNC_CORE->startPPP();
  }
}

// Start Real-Time (Retrieve Data etc.)
////////////////////////////////////////////////////////////////////////////
void bncWindow::startRealTime() {

  _runningRealTime = true;

  _bncFigurePPP->reset();

  _actDeleteMountPoints->setEnabled(false);

  enableStartStop();

  _caster = new bncCaster();

  BNC_CORE->setCaster(_caster);
  BNC_CORE->setPortEph(_ephOutPortLineEdit->text().toInt());
  BNC_CORE->setPortCorr(_corrPortLineEdit->text().toInt());
  BNC_CORE->initCombination();

  connect(_caster, SIGNAL(getThreadsFinished()),
          this, SLOT(slotGetThreadsFinished()));

  connect (_caster, SIGNAL(mountPointsRead(QList<bncGetThread*>)),
           this, SLOT(slotMountPointsRead(QList<bncGetThread*>)));

  BNC_CORE->slotMessage("========== Start BNC v" BNCVERSION " (" BNC_OS ") ==========", true);

  bncSettings settings;

  // Active panels
  // -------------
  if (!_rnxPathLineEdit->text().isEmpty())
      BNC_CORE->slotMessage("Panel 'RINEX Observations' active", true);
  if (!_ephPathLineEdit->text().isEmpty())
      BNC_CORE->slotMessage("Panel 'RINEX Ephemeris' active", true);
  if (!_corrPathLineEdit->text().isEmpty())
      BNC_CORE->slotMessage("Panel 'Broadcast Corrections' active", true);
  if (!_outPortLineEdit->text().isEmpty())
      BNC_CORE->slotMessage("Panel 'Feed Engine' active", true);
  if (!_serialMountPointLineEdit->text().isEmpty())
      BNC_CORE->slotMessage("Panel 'Serial Output' active", true);
  if (!_adviseObsRateComboBox->currentText().isEmpty())
      BNC_CORE->slotMessage("Panel 'Outages' active", true);
  if (!_miscMountLineEdit->text().isEmpty())
      BNC_CORE->slotMessage("Panel 'Miscellaneous' active", true);
  if (_pppWidgets._dataSource->currentText() == "Real-Time Streams")
      BNC_CORE->slotMessage("Panel 'PPP' active", true);
  if (_cmbTable->rowCount() > 0)
      BNC_CORE->slotMessage("Panel 'Combine Corrections' active", true);
  if (_uploadTable->rowCount() > 0)
      BNC_CORE->slotMessage("Panel 'Upload Corrections' active", true);
  if (_uploadEphTable->rowCount() > 0)
      BNC_CORE->slotMessage("Panel 'UploadEphemeris' active", true);

  QDir rnxdir(settings.value("rnxPath").toString());
  if (!rnxdir.exists()) BNC_CORE->slotMessage("Cannot find RINEX Observations directory", true);

  QString rnx_file = settings.value("rnxScript").toString();
  if ( !rnx_file.isEmpty() ) {
    QFile rnxfile(settings.value("rnxScript").toString());
    if (!rnxfile.exists()) BNC_CORE->slotMessage("Cannot find RINEX Observations script", true);
  }

  QDir ephdir(settings.value("ephPath").toString());
  if (!ephdir.exists()) BNC_CORE->slotMessage("Cannot find RINEX Ephemeris directory", true);

  QDir corrdir(settings.value("corrPath").toString());
  if (!corrdir.exists()) BNC_CORE->slotMessage("Cannot find Broadcast Corrections directory", true);

  QString advise_file = settings.value("adviseScript").toString();
  if ( !advise_file.isEmpty() ) {
    QFile advisefile(settings.value("adviseScript").toString());
    if (!advisefile.exists()) BNC_CORE->slotMessage("Cannot find Outages script", true);
  }

  _caster->readMountPoints();

  _casterEph = new bncEphUploadCaster();
}

// Retrieve Data
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotStop() {
  int iRet = QMessageBox::question(this, "Stop", "Stop retrieving/processing data?",
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::NoButton);
  if (iRet == QMessageBox::Yes) {
    BNC_CORE->stopPPP();
    BNC_CORE->stopCombination();
    delete _caster;    _caster    = 0; BNC_CORE->setCaster(0);
    delete _casterEph; _casterEph = 0;
    _runningRealTime = false;
    _runningPPP      = false;
    enableStartStop();
  }
}

// Close Application gracefully
////////////////////////////////////////////////////////////////////////////
void bncWindow::closeEvent(QCloseEvent* event) {

  int iRet = QMessageBox::question(this, "Close", "Save Options?",
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::Cancel);

  if      (iRet == QMessageBox::Cancel) {
    event->ignore();
    return;
  }
  else if (iRet == QMessageBox::Yes) {
    slotSaveOptions();
  }

  BNC_CORE->stopPPP();

  QMainWindow::closeEvent(event);
}

// User changed the selection of mountPoints
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotSelectionChanged() {
  if (_mountPointsTable->selectedItems().isEmpty()) {
    _actDeleteMountPoints->setEnabled(false);
  }
  else {
    _actDeleteMountPoints->setEnabled(true);
  }
}

// Display Program Messages
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotWindowMessage(const QByteArray msg, bool showOnScreen) {
  if (showOnScreen ) {
    _log->append(QDateTime::currentDateTime().toUTC().toString("yy-MM-dd hh:mm:ss ") + msg);
  }
}

// About Message
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAbout() {
 new bncAboutDlg(0);
}

//Flowchart
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotFlowchart() {
 new bncFlowchartDlg(0);
}

// Help Window
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotHelp() {
  QUrl url = QUrl::fromLocalFile(":/bnchelp.html");
  new bncHlpDlg(0, url);
}

// Select Fonts
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotFontSel() {
  bool ok;
  QFont newFont = QFontDialog::getFont(&ok, this->font(), this);
  if (ok) {
    bncSettings settings;
    settings.setValue("font", newFont.toString());
    QApplication::setFont(newFont);
    int ww = QFontMetrics(newFont).width('w');
    setMinimumSize(60*ww, 80*ww);
    resize(60*ww, 80*ww);
  }
}

// Whats This Help
void bncWindow::slotWhatsThis() {
  QWhatsThis::enterWhatsThisMode();
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotMountPointsRead(QList<bncGetThread*> threads) {
  _threads = threads;

  _bncFigure->updateMountPoints();
  _bncFigureLate->updateMountPoints();

  populateMountPointsTable();
  bncSettings settings;
  _outSamplComboBox->findText(settings.value("outSampl").toString());
  _outWaitSpinBox->setValue(settings.value("outWait").toInt());
  QListIterator<bncGetThread*> iTh(threads);
  while (iTh.hasNext()) {
    bncGetThread* thread = iTh.next();
    for (int iRow = 0; iRow < _mountPointsTable->rowCount(); iRow++) {
      QUrl url( "//" + _mountPointsTable->item(iRow, 0)->text() +
                "@"  + _mountPointsTable->item(iRow, 1)->text() );
      if (url                                      == thread->mountPoint() &&
          _mountPointsTable->item(iRow, 4)->text() == thread->latitude()   &&
          _mountPointsTable->item(iRow, 5)->text() == thread->longitude() ) {
        ((bncTableItem*) _mountPointsTable->item(iRow, 8))->setGetThread(thread);
        disconnect(thread, SIGNAL(newBytes(QByteArray, double)),
                  _bncFigure, SLOT(slotNewData(QByteArray, double)));
        connect(thread, SIGNAL(newBytes(QByteArray, double)),
                _bncFigure, SLOT(slotNewData(QByteArray, double)));
        disconnect(thread, SIGNAL(newLatency(QByteArray, double)),
                   _bncFigureLate, SLOT(slotNewLatency(QByteArray, double)));
        connect(thread, SIGNAL(newLatency(QByteArray, double)),
                _bncFigureLate, SLOT(slotNewLatency(QByteArray, double)));
        break;
      }
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::CreateMenu() {
  // Create Menus
  // ------------
  _menuFile = menuBar()->addMenu(tr("&File"));
  _menuFile->addAction(_actFontSel);
  _menuFile->addSeparator();
  _menuFile->addAction(_actSaveOpt);
  _menuFile->addSeparator();
  _menuFile->addAction(_actQuit);

  _menuHlp = menuBar()->addMenu(tr("&Help"));
  _menuHlp->addAction(_actHelp);
  _menuHlp->addAction(_actFlowchart);
  _menuHlp->addAction(_actAbout);
}

// Toolbar
////////////////////////////////////////////////////////////////////////////
void bncWindow::AddToolbar() {
  QToolBar* toolBar = new QToolBar;
  addToolBar(Qt::BottomToolBarArea, toolBar);
  toolBar->setMovable(false);
  toolBar->addAction(_actAddMountPoints);
  toolBar->addAction(_actDeleteMountPoints);
  toolBar->addAction(_actMapMountPoints);
  toolBar->addAction(_actStart);
  toolBar->addAction(_actStop);
  toolBar->addWidget(new QLabel("                                           "));
  toolBar->addAction(_actwhatsthis);
}

// About
////////////////////////////////////////////////////////////////////////////
bncAboutDlg::bncAboutDlg(QWidget* parent) :
   QDialog(parent) {

  QTextBrowser* tb = new QTextBrowser;
  QUrl url = QUrl::fromLocalFile(":/bncabout.html");
  tb->setSource(url);
  tb->setReadOnly(true);

  int ww = QFontMetrics(font()).width('w');
  QPushButton* _closeButton = new QPushButton("Close");
  _closeButton->setMaximumWidth(10*ww);
  connect(_closeButton, SIGNAL(clicked()), this, SLOT(close()));

  QGridLayout* dlgLayout = new QGridLayout();
  QLabel* img = new QLabel();
  img->setPixmap(QPixmap(":ntrip-logo.png"));
  dlgLayout->addWidget(img, 0,0);
  dlgLayout->addWidget(new QLabel("BKG Ntrip Client (BNC) Version " BNCVERSION), 0,1);
  dlgLayout->addWidget(tb,1,0,1,2);
  dlgLayout->addWidget(_closeButton,2,1,Qt::AlignRight);

  setLayout(dlgLayout);
  resize(60*ww, 60*ww);
  setWindowTitle("About BNC");
  show();
}

//
////////////////////////////////////////////////////////////////////////////
bncAboutDlg::~bncAboutDlg() {

};

// Flowchart
////////////////////////////////////////////////////////////////////////////
bncFlowchartDlg::bncFlowchartDlg(QWidget* parent) :
   QDialog(parent) {

  int ww = QFontMetrics(font()).width('w');
  QPushButton* _closeButton = new QPushButton("Close");
  _closeButton->setMaximumWidth(10*ww);
  connect(_closeButton, SIGNAL(clicked()), this, SLOT(close()));

  QGridLayout* dlgLayout = new QGridLayout();
  QLabel* img = new QLabel();
  img->setPixmap(QPixmap(":bncflowchart.png"));
  dlgLayout->addWidget(img, 0,0);
  dlgLayout->addWidget(_closeButton,1,0,Qt::AlignLeft);

  setLayout(dlgLayout);
  setWindowTitle("Flow Chart");
  show();
}

//
////////////////////////////////////////////////////////////////////////////
bncFlowchartDlg::~bncFlowchartDlg() {
};

// Enable/Disable Widget (and change its color)
////////////////////////////////////////////////////////////////////////////
void bncWindow::enableWidget(bool enable, QWidget* widget) {

  const static QPalette paletteWhite(QColor(255, 255, 255));
  const static QPalette paletteGray(QColor(230, 230, 230));

  widget->setEnabled(enable);
  if (enable) {
    widget->setPalette(paletteWhite);
  }
  else {
    widget->setPalette(paletteGray);
  }
}

//  Bnc Text
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotBncTextChanged(){

  const static QPalette paletteWhite(QColor(255, 255, 255));
  const static QPalette paletteGray(QColor(230, 230, 230));

  bool enable = true;

  // Proxy
  //------
  if (sender() == 0 || sender() == _proxyHostLineEdit) {
    enable = !_proxyHostLineEdit->text().isEmpty();
    enableWidget(enable, _proxyPortLineEdit);
  }

  // RINEX Observations
  // ------------------
  if (sender() == 0 || sender() == _rnxPathLineEdit) {
    enable = !_rnxPathLineEdit->text().isEmpty();
    enableWidget(enable, _rnxIntrComboBox);
    enableWidget(enable, _rnxSamplComboBox);
    enableWidget(enable, _rnxSkelLineEdit);
    enableWidget(enable, _rnxFileCheckBox);
    enableWidget(enable, _rnxScrpLineEdit);
    enableWidget(enable, _rnxV2Priority);
    enableWidget(enable, _rnxV3CheckBox);

    bool enable1 = true;
    enable1 = _rnxV3CheckBox->isChecked();
    if (enable && enable1) {
      enableWidget(false, _rnxV2Priority);
    }
    if (enable && !enable1) {
      enableWidget(true, _rnxV2Priority);
    }
  }

  // RINEX Observations, Signal Priority
  // -----------------------------------
  if (sender() == 0 || sender() == _rnxV3CheckBox) {
    if (!_rnxPathLineEdit->text().isEmpty()) {
      enableWidget(enable, _rnxIntrComboBox);
      enable = !_rnxV3CheckBox->isChecked();
      enableWidget(enable, _rnxV2Priority);
    }
  }

  // RINEX Ephemeris
  // ---------------
  if (sender() == 0 || sender() == _ephPathLineEdit || sender() == _ephOutPortLineEdit) {
    enable = !_ephPathLineEdit->text().isEmpty() || !_ephOutPortLineEdit->text().isEmpty();
    enableWidget(enable, _ephIntrComboBox);
    enableWidget(enable, _ephV3CheckBox);
  }

  // Broadcast Corrections
  // ---------------------
  if (sender() == 0 || sender() == _corrPathLineEdit || sender() == _corrPortLineEdit) {
    enable = !_corrPathLineEdit->text().isEmpty() || !_corrPortLineEdit->text().isEmpty();
    enableWidget(enable, _corrIntrComboBox);
  }

  // Feed Engine
  // -----------
  if (sender() == 0 || sender() == _outPortLineEdit || sender() == _outFileLineEdit) {
    enable = !_outPortLineEdit->text().isEmpty() || !_outFileLineEdit->text().isEmpty();
    enableWidget(enable, _outWaitSpinBox);
    enableWidget(enable, _outSamplComboBox);
  }

  // Serial Output
  // -------------
  if (sender() == 0 || sender() == _serialMountPointLineEdit ||
      sender() == _serialAutoNMEAComboBox) {
    enable = !_serialMountPointLineEdit->text().isEmpty();
    enableWidget(enable, _serialPortNameLineEdit);
    enableWidget(enable, _serialBaudRateComboBox);
    enableWidget(enable, _serialParityComboBox);
    enableWidget(enable, _serialDataBitsComboBox);
    enableWidget(enable, _serialStopBitsComboBox);
    enableWidget(enable, _serialFlowControlComboBox);
    enableWidget(enable, _serialAutoNMEAComboBox);
    bool enable2 = enable && _serialAutoNMEAComboBox->currentText() == "Auto";
    enableWidget(enable2, _serialFileNMEALineEdit);
    bool enable3 = enable && _serialAutoNMEAComboBox->currentText().contains("Manual");
    enableWidget(enable3, _serialHeightNMEALineEdit);
    enableWidget(enable3, _serialManualNMEASamplingSpinBox);
  }

  // Outages
  // -------
  if (sender() == 0 || sender() == _adviseObsRateComboBox) {
    enable = !_adviseObsRateComboBox->currentText().isEmpty();
    enableWidget(enable, _adviseFailSpinBox);
    enableWidget(enable, _adviseRecoSpinBox);
    enableWidget(enable, _adviseScriptLineEdit);
  }

  // Miscellaneous
  // -------------
  if (sender() == 0 || sender() == _miscMountLineEdit) {
    enable = !_miscMountLineEdit->text().isEmpty();
    enableWidget(enable, _miscIntrComboBox);
    enableWidget(enable, _miscScanRTCMCheckBox);
    enableWidget(enable, _miscPortLineEdit);
  }

  // Combine Corrections
  // -------------------
  if (sender() == 0 || sender() == _cmbTable) {
    int iRow = _cmbTable->rowCount();
    if (iRow > 0) {
      enableWidget(true, _cmbMethodComboBox);
      enableWidget(true, _cmbMaxresLineEdit);
      enableWidget(true, _cmbSamplSpinBox);
      enableWidget(true, _cmbUseGlonass);
    }
    else {
      enableWidget(false, _cmbMethodComboBox);
      enableWidget(false, _cmbMaxresLineEdit);
      enableWidget(false, _cmbSamplSpinBox);
      enableWidget(false, _cmbUseGlonass);
    }
  }

  // Upload(clk)
  // -----------
  int iRow = _uploadTable->rowCount();
  if (iRow > 0) {
    enableWidget(true, _uploadIntrComboBox);
    enableWidget(true, _uploadSamplRtcmEphCorrSpinBox);
    enableWidget(true, _uploadSamplClkRnxSpinBox);
    enableWidget(true, _uploadSamplSp3SpinBox);
    enableWidget(true, _uploadAntexFile);
  }
  else {
    enableWidget(false, _uploadIntrComboBox);
    enableWidget(false, _uploadSamplRtcmEphCorrSpinBox);
    enableWidget(false, _uploadSamplClkRnxSpinBox);
    enableWidget(false, _uploadSamplSp3SpinBox);
    enableWidget(false, _uploadAntexFile);
  }

  // Upload(eph)
  // -----------
  iRow = _uploadEphTable->rowCount();
  if (iRow > 0) {
    enableWidget(true, _uploadSamplRtcmEphSpinBox);
  }
  else {
    enableWidget(false, _uploadSamplRtcmEphSpinBox);
  }

  // QC
  // --
  if (sender() == 0 || sender() == _reqcActionComboBox || sender() == _reqcSkyPlotSignals) {
    enable = !_reqcActionComboBox->currentText().isEmpty();
    bool enable10   = _reqcActionComboBox->currentText() == "Edit/Concatenate";
//  bool enablePlot = !_reqcSkyPlotSignals->text().isEmpty();
    enableWidget(enable,                            _reqcObsFileChooser);
    enableWidget(enable,                            _reqcNavFileChooser);
    enableWidget(enable,                            _reqcOutLogLineEdit);
    enableWidget(enable &&  enable10,               _reqcEditOptionButton);
    enableWidget(enable &&  enable10,               _reqcOutObsLineEdit);
    enableWidget(enable &&  enable10,               _reqcOutNavLineEdit);
    enableWidget(enable && !enable10,               _reqcLogSummaryOnly);
    enableWidget(enable && !enable10,               _reqcSkyPlotSignals);
//  enableWidget(enable && !enable10 && enablePlot, _reqcPlotDirLineEdit);
    enableWidget(enable && !enable10,               _reqcPlotDirLineEdit);
  }

  // SP3 File Comparison
  // -------------------
  if (sender() == 0 || sender() == _sp3CompFileChooser) {
    enable = !_sp3CompFileChooser->fileName().isEmpty();
    enableWidget(enable, _sp3CompLogLineEdit);
    enableWidget(enable, _sp3CompExclude);
  }

  enableStartStop();
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAddCmbRow() {
  int iRow = _cmbTable->rowCount();
  _cmbTable->insertRow(iRow);
  for (int iCol = 0; iCol < _cmbTable->columnCount(); iCol++) {
    _cmbTable->setItem(iRow, iCol, new QTableWidgetItem(""));
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotDelCmbRow() {

  const static QPalette paletteWhite(QColor(255, 255, 255));
  const static QPalette paletteGray(QColor(230, 230, 230));

  int nRows = _cmbTable->rowCount();
  bool flg[nRows];
  for (int iRow = 0; iRow < nRows; iRow++) {
    if (_cmbTable->isItemSelected(_cmbTable->item(iRow,1))) {
      flg[iRow] = true;
    }
    else {
      flg[iRow] = false;
    }
  }
  for (int iRow = nRows-1; iRow >= 0; iRow--) {
    if (flg[iRow]) {
      _cmbTable->removeRow(iRow);
    }
  }
  nRows = _cmbTable->rowCount();
  if (nRows < 1) {
    enableWidget(false, _cmbMethodComboBox);
    enableWidget(false, _cmbMaxresLineEdit);
    enableWidget(false, _cmbSamplSpinBox);
    enableWidget(false, _cmbUseGlonass);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::populateCmbTable() {

  for (int iRow = _cmbTable->rowCount()-1; iRow >=0; iRow--) {
    _cmbTable->removeRow(iRow);
  }

  bncSettings settings;

  int iRow = -1;
  QListIterator<QString> it(settings.value("cmbStreams").toStringList());
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    if (hlp.size() > 2) {
      ++iRow;
      _cmbTable->insertRow(iRow);
    }
    for (int iCol = 0; iCol < hlp.size(); iCol++) {
      _cmbTable->setItem(iRow, iCol, new QTableWidgetItem(hlp[iCol]));
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAddUploadRow() {
  int iRow = _uploadTable->rowCount();
  _uploadTable->insertRow(iRow);
  for (int iCol = 0; iCol < _uploadTable->columnCount(); iCol++) {
    if (iCol == 3) {
      QComboBox* ntripversion = new QComboBox();
      ntripversion->setEditable(false);
      ntripversion->addItems(QString("1,2,2s").split(","));
      ntripversion->setFrame(false);
      _uploadTable->setCellWidget(iRow, iCol, ntripversion);

    }
    else if (iCol == 4) {
      QLineEdit* user = new QLineEdit();
      user->setFrame(false);
      _uploadTable->setCellWidget(iRow, iCol, user);
    }
    else if (iCol == 5) {
      QLineEdit* passwd = new QLineEdit();
      passwd->setFrame(false);
      passwd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
      _uploadTable->setCellWidget(iRow, iCol, passwd);
    }
    else if (iCol == 6) {
      QComboBox* system = new QComboBox();
      system->setEditable(false);
      system->addItems(QString("IGS14,ETRF2000,GDA2020,SIRGAS2000,DREF91,Custom").split(","));
      system->setFrame(false);
      _uploadTable->setCellWidget(iRow, iCol, system);
    }
    else if (iCol == 7) {
      QCheckBox* com = new QCheckBox();
      _uploadTable->setCellWidget(iRow, iCol, com);
    }
    else if (iCol == 13) {
      bncTableItem* bncIt = new bncTableItem();
      bncIt->setFlags(bncIt->flags() & ~Qt::ItemIsEditable);
      _uploadTable->setItem(iRow, iCol, bncIt);
      BNC_CORE->_uploadTableItems[iRow] = bncIt;
    }
    else {
      _uploadTable->setItem(iRow, iCol, new QTableWidgetItem(""));
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotDelUploadRow() {
  BNC_CORE->_uploadTableItems.clear();
  int nRows = _uploadTable->rowCount();
  bool flg[nRows];
  for (int iRow = 0; iRow < nRows; iRow++) {
    if (_uploadTable->isItemSelected(_uploadTable->item(iRow,1))) {
      flg[iRow] = true;
    }
    else {
      flg[iRow] = false;
    }
  }
  for (int iRow = nRows-1; iRow >= 0; iRow--) {
    if (flg[iRow]) {
      _uploadTable->removeRow(iRow);
    }
  }
  for (int iRow = 0; iRow < _uploadTable->rowCount(); iRow++) {
    BNC_CORE->_uploadTableItems[iRow] =
                                (bncTableItem*) _uploadTable->item(iRow, 13);
  }
  nRows = _uploadTable->rowCount();
  if (nRows < 1) {
    enableWidget(false, _uploadIntrComboBox);
    enableWidget(false, _uploadSamplRtcmEphCorrSpinBox);
    enableWidget(false, _uploadSamplSp3SpinBox);
    enableWidget(false, _uploadSamplClkRnxSpinBox);
    enableWidget(false, _uploadAntexFile);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::populateUploadTable() {
  for (int iRow = _uploadTable->rowCount()-1; iRow >=0; iRow--) {
    _uploadTable->removeRow(iRow);
  }

  bncSettings settings;

  int iRow = -1;
  QListIterator<QString> it(settings.value("uploadMountpointsOut").toStringList());

  while (it.hasNext()) {
    QStringList hlp = it.next().split(",");
    if (hlp.size() > 6) {
      ++iRow;
      _uploadTable->insertRow(iRow);
    }
    for (int iCol = 0; iCol < hlp.size(); iCol++) {
      if      (iCol == 3) {
        QComboBox* ntripversion = new QComboBox();
        ntripversion->setEditable(false);
        ntripversion->addItems(QString("1,2,2s").split(","));
        ntripversion->setFrame(false);
        ntripversion->setCurrentIndex(ntripversion->findText(hlp[iCol]));
        _uploadTable->setCellWidget(iRow, iCol, ntripversion);
      }
      else if (iCol == 4) {
        QLineEdit* user = new QLineEdit();
        user->setFrame(false);
        user->setText(hlp[iCol]);
        _uploadTable->setCellWidget(iRow, iCol, user);
      }
      else if (iCol == 5) {
        QLineEdit* passwd = new QLineEdit();
        passwd->setFrame(false);
        passwd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
        passwd->setText(hlp[iCol]);
        _uploadTable->setCellWidget(iRow, iCol, passwd);
      }
      else if (iCol == 6) {
        QComboBox* system = new QComboBox();
        system->setEditable(false);
        system->addItems(QString("IGS14,ETRF2000,GDA2020,SIRGAS2000,DREF91,Custom").split(","));
        system->setFrame(false);
        system->setCurrentIndex(system->findText(hlp[iCol]));
        _uploadTable->setCellWidget(iRow, iCol, system);
      }
      else if (iCol == 7) {
        QCheckBox* com = new QCheckBox();
        if (hlp[iCol].toInt() == Qt::Checked) {
          com->setCheckState(Qt::Checked);
        }
        _uploadTable->setCellWidget(iRow, iCol, com);
      }
      else if (iCol == 13) {
        bncTableItem* bncIt = new bncTableItem();
        bncIt->setFlags(bncIt->flags() & ~Qt::ItemIsEditable);
        _uploadTable->setItem(iRow, iCol, bncIt);
        BNC_CORE->_uploadTableItems[iRow] = bncIt;
      }
      else {
        _uploadTable->setItem(iRow, iCol, new QTableWidgetItem(hlp[iCol]));
      }
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotSetUploadTrafo() {
  bncCustomTrafo* dlg = new bncCustomTrafo(this);
  dlg->exec();
  delete dlg;
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotAddUploadEphRow() {
  int iRow = _uploadEphTable->rowCount();
  _uploadEphTable->insertRow(iRow);
  for (int iCol = 0; iCol < _uploadEphTable->columnCount(); iCol++) {
    if      (iCol == 3) {
      QComboBox* ntripversion = new QComboBox();
      ntripversion->setEditable(false);
      ntripversion->addItems(QString("1,2,2s").split(","));
      ntripversion->setFrame(false);
      _uploadEphTable->setCellWidget(iRow, iCol, ntripversion);

    }
    else if (iCol == 4) {
      QLineEdit* user = new QLineEdit();
      user->setFrame(false);
      _uploadEphTable->setCellWidget(iRow, iCol, user);
    }
    else if (iCol == 5) {
      QLineEdit* passwd = new QLineEdit();
      passwd->setFrame(false);
      passwd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
      _uploadEphTable->setCellWidget(iRow, iCol, passwd);
    }
    else if (iCol == 6) {
      QComboBox* system = new QComboBox();
      system->setEditable(false);
      system->addItems(QString("ALL,GPS,GLONASS,Galileo,BDS,QZSS,SBAS,IRNSS").split(","));
      system->setFrame(false);
      _uploadEphTable->setCellWidget(iRow, iCol, system);
    }
    else if (iCol == 7) {
      bncTableItem* bncIt = new bncTableItem();
      bncIt->setFlags(bncIt->flags() & ~Qt::ItemIsEditable);
      _uploadEphTable->setItem(iRow, iCol, bncIt);
      BNC_CORE->_uploadEphTableItems[iRow] = bncIt;
    }
    else {
      _uploadEphTable->setItem(iRow, iCol, new QTableWidgetItem(""));
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotDelUploadEphRow() {
  BNC_CORE->_uploadTableItems.clear();
  int nRows = _uploadEphTable->rowCount();
  bool flg[nRows];
  for (int iRow = 0; iRow < nRows; iRow++) {
    if (_uploadEphTable->isItemSelected(_uploadEphTable->item(iRow,1))) {
      flg[iRow] = true;
    }
    else {
      flg[iRow] = false;
    }
  }
  for (int iRow = nRows-1; iRow >= 0; iRow--) {
    if (flg[iRow]) {
      _uploadEphTable->removeRow(iRow);
    }
  }
  for (int iRow = 0; iRow < _uploadTable->rowCount(); iRow++) {
    BNC_CORE->_uploadTableItems[iRow] =
                                (bncTableItem*) _uploadEphTable->item(iRow, 7);
  }
  nRows = _uploadEphTable->rowCount();
  if (nRows < 1) {
    enableWidget(false, _uploadSamplRtcmEphSpinBox);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncWindow::populateUploadEphTable() {
  for (int iRow = _uploadEphTable->rowCount()-1; iRow >=0; iRow--) {
    _uploadEphTable->removeRow(iRow);
  }

  bncSettings settings;

  int iRow = -1;
  QListIterator<QString> it(settings.value("uploadEphMountpointsOut").toStringList());
  while (it.hasNext()) {
    QStringList hlp = it.next().split(",");
    if (hlp.size() > 6) {
      ++iRow;
      _uploadEphTable->insertRow(iRow);
    }
    for (int iCol = 0; iCol < hlp.size(); iCol++) {
      if (iCol == 3) {
        QComboBox* ntripversion = new QComboBox();
        ntripversion->setEditable(false);
        ntripversion->addItems(QString("1,2,2s").split(","));
        ntripversion->setFrame(false);
        ntripversion->setCurrentIndex(ntripversion->findText(hlp[iCol]));
        _uploadEphTable->setCellWidget(iRow, iCol, ntripversion);
      }
      else if (iCol == 4) {
        QLineEdit* user = new QLineEdit();
        user->setFrame(false);
        _uploadEphTable->setCellWidget(iRow, iCol, user);
      }
      else if (iCol == 5) {
        QLineEdit* passwd = new QLineEdit();
        passwd->setFrame(false);
        passwd->setEchoMode(QLineEdit::PasswordEchoOnEdit);
        passwd->setText(hlp[iCol]);
        _uploadEphTable->setCellWidget(iRow, iCol, passwd);
      }
      else if (iCol == 6) {
        QComboBox* system = new QComboBox();
        system->setEditable(false);
        system->addItems(QString("ALL,GPS,GLONASS,Galileo,BDS,QZSS,SBAS,IRNSS").split(","));
        system->setFrame(false);
        system->setCurrentIndex(system->findText(hlp[iCol]));
        _uploadEphTable->setCellWidget(iRow, iCol, system);
      }
      else if (iCol == 7) {
        bncTableItem* bncIt = new bncTableItem();
        bncIt->setFlags(bncIt->flags() & ~Qt::ItemIsEditable);
        _uploadEphTable->setItem(iRow, iCol, bncIt);
        BNC_CORE->_uploadEphTableItems[iRow] = bncIt;
      }
      else {
        _uploadEphTable->setItem(iRow, iCol, new QTableWidgetItem(hlp[iCol]));
      }
    }
  }
}


// Progress Bar Change
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotPostProcessingProgress(int nEpo) {
  _actStart->setText(QString("%1 Epochs").arg(nEpo));
}

// Post-Processing Reqc Finished
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotPostProcessingFinished() {
  delete _caster; _caster = 0; BNC_CORE->setCaster(0);
  _runningPPP     = false;
  _runningEdit    = false;
  _runningQC      = false;
  _runningSp3Comp = false;
  _actStart->setText(tr("Sta&rt"));
  enableStartStop();
}

// Edit teqc-like editing options
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotReqcEditOption() {
  saveOptions();
  reqcDlg* dlg = new reqcDlg(this);
  dlg->move(this->pos().x()+50, this->pos().y()+50);
  dlg->exec();
  delete dlg;
}

// Enable/Disable Start and Stop Buttons
////////////////////////////////////////////////////////////////////////////
void bncWindow::enableStartStop() {
  if ( running() ) {
    _actStart->setEnabled(false);
    if (_runningRealTime || _runningPPP) {
      _actStop->setEnabled(true);
    }
  }
  else {
    _actStart->setEnabled(true);
    _actStop->setEnabled(false);
  }
}

// Show Map
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotMapMountPoints() {
  saveOptions();
  t_bncMap* bncMap = new t_bncMap(this);
  bncMap->setMinimumSize(800, 600);
  bncMap->setWindowTitle("Selected Mountpoints");

  bncSettings settings;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    if (hlp.size() < 5) continue;
    QUrl   url(hlp[0]);
    double latDeg = hlp[3].toDouble();
    double lonDeg = hlp[4].toDouble();
    bncMap->slotNewPoint(QFileInfo(url.path()).fileName(), latDeg, lonDeg);
  }

  bncMap->show();
}

// Show Map
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotMapPPP() {
#ifdef QT_WEBKIT
  saveOptions();
  enableWidget(false, _pppWidgets._mapWinButton);
  enableWidget(false, _pppWidgets._useGoogleMap);
  enableWidget(false, _pppWidgets._useOpenStreetMap);
  enableWidget(false, _pppWidgets._mapWinDotSize);
  enableWidget(false, _pppWidgets._mapWinDotColor);

  if (!_mapWin) {
    _mapWin = new bncMapWin(this);
    connect(_mapWin, SIGNAL(mapClosed()), this, SLOT(slotMapPPPClosed()));
    connect(BNC_CORE, SIGNAL(newPosition(QByteArray, bncTime, QVector<double>)),
            _mapWin, SLOT(slotNewPosition(QByteArray, bncTime, QVector<double>)));
  }
  _mapWin->show();
#else
  QMessageBox::information(this, "Information",
                           "Qt Library compiled without QtWebKit");
#endif
}

// Show Map
////////////////////////////////////////////////////////////////////////////
void bncWindow::slotMapPPPClosed() {
#ifdef QT_WEBKIT
  enableWidget(true, _pppWidgets._mapWinButton);
  enableWidget(true, _pppWidgets._useGoogleMap);
  enableWidget(true, _pppWidgets._useOpenStreetMap);
  enableWidget(true, _pppWidgets._mapWinDotSize);
  enableWidget(true, _pppWidgets._mapWinDotColor);
  if (_mapWin) {
    QListIterator<bncGetThread*> it(_threads);
    while (it.hasNext()) {
      bncGetThread* thread = it.next();
      thread->disconnect(_mapWin);
    }
    _mapWin->deleteLater();
    _mapWin = 0;
  }
#endif
}
