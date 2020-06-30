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
 * Class:      t_pppWidgets
 *
 * Purpose:    This class stores widgets for PPP options
 *
 * Author:     L. Mervart
 *
 * Created:    29-Jul-2014
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableWidget>

#include "pppWidgets.h"
#include "qtfilechooser.h"
#include "bncsettings.h"
#include "bnccore.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_pppWidgets::t_pppWidgets() {

  _dataSource   = new QComboBox();     _dataSource  ->setObjectName("PPP/dataSource");   _widgets << _dataSource;
  _rinexObs     = new qtFileChooser(); _rinexObs    ->setObjectName("PPP/rinexObs");     _widgets << _rinexObs;
  _rinexNav     = new qtFileChooser(); _rinexNav    ->setObjectName("PPP/rinexNav");     _widgets << _rinexNav;
  _corrMount    = new QLineEdit();     _corrMount   ->setObjectName("PPP/corrMount");    _widgets << _corrMount;
  _corrFile     = new qtFileChooser(); _corrFile    ->setObjectName("PPP/corrFile");     _widgets << _corrFile;
  _crdFile      = new qtFileChooser(); _crdFile     ->setObjectName("PPP/crdFile");      _widgets << _crdFile;
  _antexFile    = new qtFileChooser(); _antexFile   ->setObjectName("PPP/antexFile");    _widgets << _antexFile;
  _blqFile      = new qtFileChooser(); _blqFile     ->setObjectName("PPP/blqFile");      _widgets << _blqFile;
  _logPath      = new QLineEdit();     _logPath     ->setObjectName("PPP/logPath");      _widgets << _logPath;
  _nmeaPath     = new QLineEdit();     _nmeaPath    ->setObjectName("PPP/nmeaPath");     _widgets << _nmeaPath;
  _snxtroPath   = new QLineEdit();     _snxtroPath  ->setObjectName("PPP/snxtroPath");   _widgets << _snxtroPath;
  _snxtroSampl  = new QComboBox();      _snxtroSampl->setObjectName("PPP/snxtroSampl");  _widgets << _snxtroSampl;
  _snxtroIntr   = new QComboBox();     _snxtroIntr  ->setObjectName("PPP/snxtroIntr");   _widgets << _snxtroIntr;
  _snxtroAc     = new QLineEdit();     _snxtroAc    ->setObjectName("PPP/snxtroAc");     _widgets << _snxtroAc;
  _snxtroSol    = new QLineEdit();     _snxtroSol   ->setObjectName("PPP/snxtroSol");    _widgets << _snxtroSol;
  _v3filenames  = new QCheckBox();     _v3filenames ->setObjectName("PPP/v3filenames");  _widgets << _v3filenames;
  _staTable     = new QTableWidget();  _staTable    ->setObjectName("PPP/staTable");     _widgets << _staTable;
  _lcGPS        = new QComboBox();     _lcGPS       ->setObjectName("PPP/lcGPS");        _widgets << _lcGPS;
  _lcGLONASS    = new QComboBox();     _lcGLONASS   ->setObjectName("PPP/lcGLONASS");    _widgets << _lcGLONASS;
  _lcGalileo    = new QComboBox();     _lcGalileo   ->setObjectName("PPP/lcGalileo");    _widgets << _lcGalileo;
  _lcBDS        = new QComboBox();     _lcBDS       ->setObjectName("PPP/lcBDS");        _widgets << _lcBDS;
  _modelObs     = new QComboBox();     _modelObs    ->setObjectName("PPP/modelObs");     _widgets << _modelObs;
  _pseudoObs    = new QComboBox();     _pseudoObs   ->setObjectName("PPP/pseudoObs");    _widgets << _pseudoObs;
  _sigmaC1      = new QLineEdit();     _sigmaC1     ->setObjectName("PPP/sigmaC1");      _widgets << _sigmaC1;
  _sigmaL1      = new QLineEdit();     _sigmaL1     ->setObjectName("PPP/sigmaL1");      _widgets << _sigmaL1;
  _maxResC1     = new QLineEdit();     _maxResC1    ->setObjectName("PPP/maxResC1");     _widgets << _maxResC1;
  _maxResL1     = new QLineEdit();     _maxResL1    ->setObjectName("PPP/maxResL1");     _widgets << _maxResL1;
  _minObs       = new QSpinBox();      _minObs      ->setObjectName("PPP/minObs");       _widgets << _minObs;
  _minEle       = new QSpinBox();      _minEle      ->setObjectName("PPP/minEle");       _widgets << _minEle;
  _eleWgtCode   = new QCheckBox();     _eleWgtCode  ->setObjectName("PPP/eleWgtCode");   _widgets << _eleWgtCode;
  _eleWgtPhase  = new QCheckBox();     _eleWgtPhase ->setObjectName("PPP/eleWgtPhase");  _widgets << _eleWgtPhase;
  _seedingTime  = new QLineEdit();     _seedingTime ->setObjectName("PPP/seedingTime");  _widgets << _seedingTime;
  _corrWaitTime = new QSpinBox();      _corrWaitTime->setObjectName("PPP/corrWaitTime"); _widgets << _corrWaitTime;

  _addStaButton = new QPushButton("Add Station");    _widgets << _addStaButton;
  _delStaButton = new QPushButton("Delete Station"); _widgets << _delStaButton;

  _addStaButton->setWhatsThis(tr("<p>Hit the 'Add Station' button to add a new line to the Station table.</p>"));
  _delStaButton->setWhatsThis(tr("<p>Hit the 'Delete Station' button to delete a highlighted row from the Station table.</p>"));

  _plotCoordinates  = new QLineEdit;    _plotCoordinates ->setObjectName("PPP/plotCoordinates");  _widgets << _plotCoordinates;
  _mapWinButton     = new QPushButton;  _mapWinButton    ->setObjectName("PPP/mapWinButton");     _widgets << _mapWinButton;
  _useGoogleMap     = new QRadioButton; _useGoogleMap    ->setObjectName("PPP/useGoogleMap");     _widgets << _useGoogleMap;
  _useOpenStreetMap = new QRadioButton; _useOpenStreetMap->setObjectName("PPP/useOpenStreetMap"); _widgets << _useOpenStreetMap;
  _audioResponse    = new QLineEdit;    _audioResponse   ->setObjectName("PPP/audioResponse");    _widgets << _audioResponse;
  _mapWinDotSize    = new QLineEdit;    _mapWinDotSize   ->setObjectName("PPP/mapWinDotSize");    _widgets << _mapWinDotSize;
  _mapWinDotColor   = new QComboBox;    _mapWinDotColor  ->setObjectName("PPP/mapWinDotColor");   _widgets << _mapWinDotColor;
  _mapSpeedSlider   = new QSlider;      _mapSpeedSlider  ->setObjectName("PPP/mapSpeedSlider");   _widgets << _mapSpeedSlider;

  _dataSource->setEditable(false);
  _dataSource->addItems(QString(",Real-Time Streams,RINEX Files").split(","));
  connect(_dataSource, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(slotEnableWidgets()));
  connect(_modelObs, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(slotEnableWidgets()));
  connect(_snxtroPath, SIGNAL(textChanged(const QString &)), this, SLOT(slotPPPTextChanged()));
  connect(_snxtroAc, SIGNAL(textChanged(const QString &)), this, SLOT(slotPPPTextChanged()));
  connect(_snxtroSol, SIGNAL(textChanged(const QString &)), this, SLOT(slotPPPTextChanged()));

  slotEnableWidgets();

  _lcGPS->setEditable(false);
#ifdef USE_PPP_SSR_I
  _lcGPS->addItems(QString("P3,P3&L3").split(","));
#else
  _lcGPS->addItems(QString("no,Pi,Li,Pi&Li").split(","));
#endif

  _lcGLONASS->setEditable(false);
#ifdef USE_PPP_SSR_I
   _lcGLONASS->addItems(QString("no,P3,L3,P3&L3").split(","));
#else
  _lcGLONASS->addItems(QString("no,Pi,Li,Pi&Li").split(","));
#endif

  _lcGalileo->setEditable(false);
#ifdef USE_PPP_SSR_I
  _lcGalileo->addItems(QString("no,P3,L3,P3&L3").split(","));
#else
  _lcGalileo->addItems(QString("no,Pi,Li,Pi&Li").split(","));
#endif

  _lcBDS->setEditable(false);
#ifdef USE_PPP_SSR_I
  _lcBDS->addItems(QString("no,P3,L3,P3&L3").split(","));
#else
  _lcBDS->addItems(QString("no,Pi,Li,Pi&Li").split(","));
#endif

  _modelObs->setEditable(false);
  _pseudoObs->setEditable(false);
#ifdef USE_PPP_SSR_I
  _modelObs->addItems(QString("Ionosphere-free PPP").split(","));
  _pseudoObs->addItems(QString("no").split(","));
#else
  _modelObs->addItems(QString("Ionosphere-free PPP,Uncombined PPP,PPP-RTK,DCM with Code Biases,DCM with Phase Biases").split(","));
  _pseudoObs->addItems(QString("no,Ionosphere,Iono+Tropo").split(","));
#endif

  _snxtroSampl->setEditable(false);
  _snxtroSampl->addItems(QString("1 sec,5 sec,10 sec,30 sec,60 sec,300 sec").split(","));

  _snxtroIntr->setEditable(false);
  _snxtroIntr->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  _snxtroIntr->setCurrentIndex(6);

  _minObs->setMinimum(4);
  _minObs->setMaximum(6);
  _minObs->setSingleStep(1);

  _minEle->setMinimum(0);
  _minEle->setMaximum(20);
  _minEle->setSingleStep(1);
  _minEle->setSuffix(" deg");

  _corrWaitTime->setMinimum(0);
  _corrWaitTime->setMaximum(20);
  _corrWaitTime->setSingleStep(1);
  _corrWaitTime->setSuffix(" sec");

  _staTable->setColumnCount(10);
  _staTable->setRowCount(0);
  _staTable->setHorizontalHeaderLabels(
     QString("Station,Sigma N,Sigma E,Sigma H,Noise N,Noise E,Noise H,Tropo Sigma,Tropo Noise, NMEA Port").split(","));
  _staTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  _staTable->setSelectionBehavior(QAbstractItemView::SelectRows);
#if QT_VERSION >= 0x050000
  _staTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#endif
  _staTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  connect(_addStaButton, SIGNAL(clicked()), this, SLOT(slotAddStation()));
  connect(_delStaButton, SIGNAL(clicked()), this, SLOT(slotDelStation()));

  _mapWinButton->setText("Open Map");

  _mapWinDotColor->setEditable(false);
  _mapWinDotColor->addItems(QString("red,yellow").split(","));

  _mapSpeedSlider->setOrientation(Qt::Horizontal);
  _mapSpeedSlider->setRange(1, 100);
  _mapSpeedSlider->setTickPosition(QSlider::TicksBelow);
  _mapSpeedSlider->setTickInterval(10);
  connect(_mapSpeedSlider, SIGNAL(valueChanged(int)), BNC_CORE, SIGNAL(mapSpeedSliderChanged(int)));

  // WhatsThis, PPP (2)
  // ------------------
  _staTable->setWhatsThis(tr("<p>Specify values for Sigma and white Noise of the Stations North, East and Height coordinate components in meters. Specify also a Sigma in meters for a priori model based Tropospheric delays and a Sigma in meters per second for the delay's Noise.</p><p>Specifying one record per Station is mandatory. BNC will only process data for stations which are listed here. To define a station, specify the 'Mountpoint' when in 'Real-Time Streams' mode or the 4-character station ID when in 'RINEX Files' mode.</p><p>'Sigma' is meant to describe the uncertainty of a single coordinate or tropospheric delay estimated for one epoch. 'Noise' is meant to describe the variation of estimates from epoch to epoch.</p><p><ul><li>A Sigma of 100.0 meters may be an appropriate choice e.g. for the initial N/E/H coordinates. However, this value may be significantly smaller (i.e. 0.01) for stations with well-known a priori coordinates.</li><li>A Noise of 100.0 meters for the estimated N/E/H coordinates may also be appropriate considering the potential movement of a rover position.</li><li>A value of 0.1 meters may be an appropriate Sigma for the a priori model based Tropospheric delay estimation.</li><li>Specify a Noise to describe the expected variation of the tropospheric effect over time. Supposing 1Hz observation data, specifying a value of 3e-6 would mean that the tropospheric effect may vary 3600 * 3e-6 = 0.01 meters per hour.</li></ul></p><p>You can also specify a 'NMEA Port' to output coordinates in NMEA format through an IP port of your local host. <i>[key: PPP/staTable]</i></p>"));

  // WhatsThis, PPP (3)
  // ------------------
  _corrWaitTime->setWhatsThis(tr("<p>Zero value means that BNC processes each epoch of data immediately after its arrival using satellite clock corrections available at that time.</p><p> Specifying a non-zero value (i.e. 5 sec) means that the epochs of data are buffered and the processing of each epoch is postponed till the satellite clock corrections not older than '5 sec' (example) become available. <i>[key: PPP/corrWaitTime]</i><p>"));
  _seedingTime->setWhatsThis(tr("<p>Enter the length of a startup period in seconds for which you want to fix the PPP solutions to known a priori coordinates as introduced through option 'Coordinates file'. Adjust 'Sigma N/E/H' in the PPP Stations table according to the coordinate's precision. Fixing a priori coordinates is done in BNC through setting 'Sigma N/E/H' temporarily to zero.</p><p>This option allows the PPP solution to rapidly converge. It requires that the antenna remains unmoved on the a priori known position throughout the startup period.</p><p>A value of 60 is likely to be an appropriate choice.</p><p>Default is an empty option field, meaning that you don't want BNC to fix PPP solutions during startup to an a priori coordinate. <i>[key: PPP/seedingTime]</i></p>"));

  // WhatsThis, PPP (4)
  // ------------------
  _plotCoordinates->setWhatsThis(tr("<p>For one of your PPP Stations BNC can produce a time series plot of coordinate displacements in the 'PPP Plot' tab below. Specify a 'Mountpoint' (when in 'Real-Time Streams' mode) or the 4-character station ID (when in 'RINEX Files' mode) to define the station whose coordinate displacements you would like to see plotted.</p><p>Note that this option makes only sense for a stationary receiver with known a priori marker coordinates as specified through PPP option 'Coordinates file'.</p><p>Default is an empty option field, meaning that BNC shall not produce a time series plot of PPP coordinate displacements. <i>[key: PPP/plotCoordinates]</i></p>"));
  _audioResponse->setWhatsThis(tr("<p>Specify an 'Audio response' threshold in meters. A beep is produced by BNC whenever a horizontal PPP coordinate component differs by more than the threshold value from the a priori marker coordinate.</p><p>Default is an empty option field, meaning that you don't want BNC to produce alarm signals. <i>[key: PPP/audioResponse]</i></p>"));
  _mapWinButton->setWhatsThis(tr("<p>You may like to track your rover position using Google Maps or Open Street Map as a background map. A 'Track map' can be produced with BNC in 'Real-Time Streams' or 'RINEX files' PPP mode.</p><p>The 'Open Map' button opens a windows showing a map according to specified options.</p><p>Even in 'RINEX files' post processing mode you should not forget to specify a proxy under the 'Network' tab if that is operated in front of BNC because the program needs to download the map data.</p>"));
  _useGoogleMap->setWhatsThis(tr("<p>Specify Google Maps as the background for your rover positions. <i>[key: PPP/useGoogleMap]</i></p>"));
  _useOpenStreetMap->setWhatsThis(tr("<p>Specify Open Street Map as the background for your rover positions. <i>[key: PPP/useOpenStreetMap]</i></p>"));
  _mapWinDotSize->setWhatsThis(tr("<p>Specify the size of dots showing rover positions on the track map.</p><p>A dot size of '3' may be appropriate. The maximum possible dot size is '10'. An empty option field or a size of '0' would mean that you don't want BNC to show the rover's track on the map. <i>[key: PPP/mapWinDotSize]</i></p>"));
  _mapWinDotColor->setWhatsThis(tr("<p>Specify the color of dots showing the rover track on the map. <i>[key: PPP/mapWinDotColor]</i></p>"));
  _mapSpeedSlider->setWhatsThis(tr("<p>With BNC in 'RINEX files' PPP post processing mode you can specify the speed of computations as appropriate for 'Track map' visualization.</p><p>Note that you can adjust 'Post-processing speed' on-the-fly while BNC is already processing your observations. <i>[key: PPP/mapSpeedSlider]</i></p>"));

  readOptions();
}

//
////////////////////////////////////////////////////////////////////////////
t_pppWidgets::~t_pppWidgets() {
  delete _dataSource;
  delete _rinexObs;
  delete _rinexNav;
  delete _corrMount;
  delete _corrFile;
  delete _crdFile;
  delete _antexFile;
  delete _blqFile;
  delete _logPath;
  delete _nmeaPath;
  delete _snxtroPath;
  delete _snxtroSampl;
  delete _snxtroIntr;
  delete _snxtroAc;
  delete _snxtroSol;
  delete _v3filenames;
  for (int iRow = _staTable->rowCount()-1; iRow >=0; iRow--) {
    _staTable->removeRow(iRow);
  }
  delete _staTable;
  delete _lcGPS;
  delete _lcGLONASS;
  delete _lcGalileo;
  delete _lcBDS;
  delete _modelObs;
  delete _pseudoObs;
  delete _sigmaC1;
  delete _sigmaL1;
  delete _maxResC1;
  delete _maxResL1;
  delete _minObs;
  delete _minEle;
  delete _eleWgtCode;
  delete _eleWgtPhase;
  delete _seedingTime;
  delete _corrWaitTime;
  delete _addStaButton;
  delete _delStaButton;
  delete _plotCoordinates;
  delete _mapWinButton;
  delete _useGoogleMap;
  delete _useOpenStreetMap;
  delete _audioResponse;
  delete _mapWinDotSize;
  delete _mapWinDotColor;
  delete _mapSpeedSlider;
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppWidgets::readOptions() {

  bncSettings settings;

  // ComboBoxes
  // ----------
  int ii = _dataSource->findText(settings.value(_dataSource->objectName()).toString());
  if (ii != -1) {
    _dataSource->setCurrentIndex(ii);
  }
  ii = _lcGPS->findText(settings.value(_lcGPS->objectName()).toString());
  if (ii != -1) {
    _lcGPS->setCurrentIndex(ii);
  }
  ii = _lcGLONASS->findText(settings.value(_lcGLONASS->objectName()).toString());
  if (ii != -1) {
    _lcGLONASS->setCurrentIndex(ii);
  }
  ii = _lcGalileo->findText(settings.value(_lcGalileo->objectName()).toString());
  if (ii != -1) {
    _lcGalileo->setCurrentIndex(ii);
  }
  ii = _lcBDS->findText(settings.value(_lcBDS->objectName()).toString());
  if (ii != -1) {
    _lcBDS->setCurrentIndex(ii);
  }
  ii = _modelObs->findText(settings.value(_modelObs->objectName()).toString());
  if (ii != -1) {
    _modelObs->setCurrentIndex(ii);
  }
  ii = _pseudoObs->findText(settings.value(_pseudoObs->objectName()).toString());
  if (ii != -1) {
    _pseudoObs->setCurrentIndex(ii);
  }
  ii = _snxtroIntr->findText(settings.value(_snxtroIntr->objectName()).toString());
  if (ii != -1) {
    _snxtroIntr->setCurrentIndex(ii);
  }

  // FileChoosers
  // ------------
  _rinexObs ->setFileName(settings.value(_rinexObs ->objectName()).toString());
  _rinexNav ->setFileName(settings.value(_rinexNav ->objectName()).toString());
  _corrFile ->setFileName(settings.value(_corrFile ->objectName()).toString());
  _crdFile  ->setFileName(settings.value(_crdFile  ->objectName()).toString());
  _antexFile->setFileName(settings.value(_antexFile->objectName()).toString());
  _blqFile->setFileName(settings.value(_blqFile->objectName()).toString());

  // LineEdits
  // ---------
  _corrMount  ->setText(settings.value(_corrMount  ->objectName()).toString());
  _logPath    ->setText(settings.value(_logPath    ->objectName()).toString());
  _nmeaPath   ->setText(settings.value(_nmeaPath   ->objectName()).toString());
  _snxtroPath ->setText(settings.value(_snxtroPath ->objectName()).toString());
  _snxtroAc   ->setText(settings.value(_snxtroAc   ->objectName()).toString());
  _snxtroSol  ->setText(settings.value(_snxtroSol  ->objectName()).toString());

  if (!settings.value(_sigmaC1->objectName()).toString().isEmpty()) {
    _sigmaC1->setText(settings.value(_sigmaC1->objectName()).toString());
  }
  else {
    _sigmaC1->setText("2.0");
  }

  if (!settings.value(_sigmaL1->objectName()).toString().isEmpty()) {
    _sigmaL1->setText(settings.value(_sigmaL1->objectName()).toString());
  }
  else {
    _sigmaL1->setText("0.01");
  }

  if (!settings.value(_maxResC1->objectName()).toString().isEmpty()) {
    _maxResC1->setText(settings.value(_maxResC1->objectName()).toString());
  }
  else {
    _maxResC1->setText("4.0");
  }

  if (!settings.value(_maxResL1->objectName()).toString().isEmpty()) {
    _maxResL1->setText(settings.value(_maxResL1->objectName()).toString());
  }
  else {
    _maxResL1->setText("0.03");
  }

  if (!settings.value(_seedingTime->objectName()).toString().isEmpty()) {
    _seedingTime->setText(settings.value(_seedingTime->objectName()).toString());
  }
  else {
    _seedingTime->setText("0");
  }

  // CheckBoxes
  // ----------
  _eleWgtCode ->setCheckState(Qt::CheckState(settings.value(_eleWgtCode ->objectName()).toInt()));
  _eleWgtPhase->setCheckState(Qt::CheckState(settings.value(_eleWgtPhase->objectName()).toInt()));
  _v3filenames->setCheckState(Qt::CheckState(settings.value(_v3filenames->objectName()).toInt()));

  // SpinBoxex
  // ---------
  _minObs->setValue(settings.value(_minObs->objectName()).toInt());
  _minEle->setValue(settings.value(_minEle->objectName()).toInt());
  _corrWaitTime->setValue(settings.value(_corrWaitTime->objectName()).toInt());


  ii = _snxtroSampl->findText(settings.value(_snxtroSampl->objectName()).toString());
  if (ii != -1) {
    _snxtroSampl->setCurrentIndex(ii);
  }

  // Table with stations
  // -------------------
  for (int iRow = _staTable->rowCount()-1; iRow >=0; iRow--) {
    _staTable->removeRow(iRow);
  }
  int iRow = -1;
  QListIterator<QString> it(settings.value(_staTable->objectName()).toStringList());
  while (it.hasNext()) {
    QStringList hlp = it.next().split(",");
    ++iRow;
    _staTable->insertRow(iRow);
    for (int iCol = 0; iCol < hlp.size(); iCol++) {
      _staTable->setItem(iRow, iCol, new QTableWidgetItem(hlp[iCol]));
    }
  }

  // Plots and Maps
  // --------------
  _plotCoordinates ->setText(settings.value(_plotCoordinates->objectName()).toString());
  _audioResponse   ->setText(settings.value(_audioResponse->objectName()).toString());
  _useGoogleMap    ->setChecked(settings.value(_useGoogleMap->objectName()).toBool());
  _useOpenStreetMap->setChecked(settings.value(_useOpenStreetMap->objectName()).toBool());
  _mapWinDotSize   ->setText(settings.value(_mapWinDotSize->objectName()).toString());

  ii = _mapWinDotColor->findText(settings.value(_mapWinDotColor->objectName()).toString());
  if (ii != -1) {
    _mapWinDotColor->setCurrentIndex(ii);
  }

  int speed = settings.value(_mapSpeedSlider->objectName()).toInt();
  if (speed == 0) speed = _mapSpeedSlider->maximum();
  _mapSpeedSlider->setSliderPosition(speed);
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppWidgets::saveOptions() {

  bncSettings settings;

  settings.setValue(_dataSource  ->objectName(), _dataSource  ->currentText());
  settings.setValue(_rinexObs    ->objectName(), _rinexObs    ->fileName());
  settings.setValue(_rinexNav    ->objectName(), _rinexNav    ->fileName());
  settings.setValue(_corrMount   ->objectName(), _corrMount   ->text());
  settings.setValue(_corrFile    ->objectName(), _corrFile    ->fileName());
  settings.setValue(_crdFile     ->objectName(), _crdFile     ->fileName());
  settings.setValue(_antexFile   ->objectName(), _antexFile   ->fileName());
  settings.setValue(_blqFile     ->objectName(), _blqFile   ->fileName());
  settings.setValue(_logPath     ->objectName(), _logPath     ->text());
  settings.setValue(_nmeaPath    ->objectName(), _nmeaPath    ->text());
  settings.setValue(_snxtroPath  ->objectName(), _snxtroPath  ->text());
  settings.setValue(_snxtroSampl ->objectName(), _snxtroSampl ->currentText());
  settings.setValue(_snxtroIntr  ->objectName(), _snxtroIntr  ->currentText());
  settings.setValue(_snxtroAc    ->objectName(), _snxtroAc    ->text());
  settings.setValue(_snxtroSol   ->objectName(), _snxtroSol   ->text());
  settings.setValue(_v3filenames ->objectName(), _v3filenames ->checkState());
  settings.setValue(_lcGPS       ->objectName(), _lcGPS       ->currentText());
  settings.setValue(_lcGLONASS   ->objectName(), _lcGLONASS   ->currentText());
  settings.setValue(_lcGalileo   ->objectName(), _lcGalileo   ->currentText());
  settings.setValue(_lcBDS       ->objectName(), _lcBDS       ->currentText());
  settings.setValue(_modelObs    ->objectName(), _modelObs    ->currentText());
  settings.setValue(_pseudoObs   ->objectName(), _pseudoObs   ->currentText());
  settings.setValue(_sigmaC1     ->objectName(), _sigmaC1     ->text());
  settings.setValue(_sigmaL1     ->objectName(), _sigmaL1     ->text());
  settings.setValue(_corrWaitTime->objectName(), _corrWaitTime->value());
  settings.setValue(_maxResC1    ->objectName(), _maxResC1    ->text());
  settings.setValue(_maxResL1    ->objectName(), _maxResL1    ->text());
  settings.setValue(_seedingTime ->objectName(), _seedingTime ->text());
  settings.setValue(_minObs      ->objectName(), _minObs      ->value());
  settings.setValue(_minEle      ->objectName(), _minEle      ->value());
  settings.setValue(_eleWgtCode  ->objectName(), _eleWgtCode  ->checkState());
  settings.setValue(_eleWgtPhase ->objectName(), _eleWgtPhase ->checkState());

  QStringList staList;
  for (int iRow = 0; iRow < _staTable->rowCount(); iRow++) {
    QString hlp;
    for (int iCol = 0; iCol < _staTable->columnCount(); iCol++) {
      if (_staTable->item(iRow, iCol)) {
        hlp += _staTable->item(iRow, iCol)->text() + ",";
      }
    }
    if (!hlp.isEmpty()) {
      staList << hlp;
    }
  }
  settings.setValue(_staTable->objectName(), staList);

  settings.setValue(_plotCoordinates ->objectName(), _plotCoordinates ->text());
  settings.setValue(_audioResponse   ->objectName(), _audioResponse   ->text());
  settings.setValue(_useGoogleMap    ->objectName(), _useGoogleMap    ->isChecked());
  settings.setValue(_useOpenStreetMap->objectName(), _useOpenStreetMap->isChecked());
  settings.setValue(_mapWinDotSize   ->objectName(), _mapWinDotSize   ->text());
  settings.setValue(_mapWinDotColor  ->objectName(), _mapWinDotColor  ->currentText());
  settings.setValue(_mapSpeedSlider  ->objectName(), _mapSpeedSlider  ->value());
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppWidgets::slotEnableWidgets() {

  const static QPalette paletteWhite(QColor(255, 255, 255));
  const static QPalette paletteGray(QColor(230, 230, 230));

  bool allDisabled = _dataSource->currentText() == "";
  bool realTime    = _dataSource->currentText() == "Real-Time Streams";
  bool rinexFiles  = _dataSource->currentText() == "RINEX Files";
  bool enablePseudoObs;
  if (_modelObs->currentText() == "PPP-RTK" ||
    _modelObs->currentText() == "Ionosphere-free PPP") {
    enablePseudoObs = false;
  }
  else {
    enablePseudoObs = true;
  }

  QListIterator<QWidget*> it(_widgets);
  while (it.hasNext()) {
    QWidget* widget = it.next();
    widget->setEnabled(!allDisabled);
  }

  if      (realTime) {
    _rinexObs->setEnabled(false);
    _rinexNav->setEnabled(false);
    _corrFile->setEnabled(false);
  }
  else if (rinexFiles) {
    _corrMount    ->setEnabled(false);
    _audioResponse->setEnabled(false);
  }

  if ( _snxtroPath->text() != "" && !allDisabled) {
    _snxtroSampl->setEnabled(true);
    _snxtroIntr ->setEnabled(true);
    _snxtroAc   ->setEnabled(true);
    _snxtroSol  ->setEnabled(true);
  }
  else {
    _snxtroSampl->setEnabled(false);
    _snxtroIntr ->setEnabled(false);
    _snxtroAc   ->setEnabled(false);
    _snxtroSol  ->setEnabled(false);
  }

  if (enablePseudoObs) {
    _pseudoObs->setEnabled(true);
  } else {
    _pseudoObs->setEnabled(false);
  }

  _dataSource->setEnabled(true);

  it.toFront();
  while (it.hasNext()) {
    QWidget* widget = it.next();
    if (widget->isEnabled()) {
      widget->setPalette(paletteWhite);
    }
    else {
      widget->setPalette(paletteGray);
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppWidgets::slotAddStation() {
  int iRow = _staTable->rowCount();
  _staTable->insertRow(iRow);
  for (int iCol = 0; iCol < _staTable->columnCount(); iCol++) {
     if (iCol == 0) _staTable->setItem(iRow, iCol, new QTableWidgetItem(""));
     if (iCol == 1) _staTable->setItem(iRow, iCol, new QTableWidgetItem("100.0"));
     if (iCol == 2) _staTable->setItem(iRow, iCol, new QTableWidgetItem("100.0"));
     if (iCol == 3) _staTable->setItem(iRow, iCol, new QTableWidgetItem("100.0"));
     if (iCol == 4) _staTable->setItem(iRow, iCol, new QTableWidgetItem("100.0"));
     if (iCol == 5) _staTable->setItem(iRow, iCol, new QTableWidgetItem("100.0"));
     if (iCol == 6) _staTable->setItem(iRow, iCol, new QTableWidgetItem("100.0"));
     if (iCol == 7) _staTable->setItem(iRow, iCol, new QTableWidgetItem("0.1"));
     if (iCol == 8) _staTable->setItem(iRow, iCol, new QTableWidgetItem("3e-6"));
     if (iCol == 9) _staTable->setItem(iRow, iCol, new QTableWidgetItem("0"));
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppWidgets::slotDelStation() {
  int nRows = _staTable->rowCount();
  bool flg[nRows];
  for (int iRow = 0; iRow < nRows; iRow++) {
    if (_staTable->isItemSelected(_staTable->item(iRow,1))) {
      flg[iRow] = true;
    }
    else {
      flg[iRow] = false;
    }
  }
  for (int iRow = nRows-1; iRow >= 0; iRow--) {
    if (flg[iRow]) {
      _staTable->removeRow(iRow);
    }
  }
}

//  PPP Text
////////////////////////////////////////////////////////////////////////////
void t_pppWidgets::slotPPPTextChanged(){

  const static QPalette paletteWhite(QColor(255, 255, 255));
  const static QPalette paletteGray(QColor(230, 230, 230));

  // SNX TRO file sampling
  // ---------------------
  if (sender() == 0 || sender() == _snxtroPath) {
    if ( _snxtroPath->text() != "" ) {
      _snxtroSampl->setEnabled(true);
      _snxtroIntr ->setEnabled(true);
      _snxtroAc   ->setEnabled(true);
      _snxtroSol  ->setEnabled(true);
      _snxtroSampl->setPalette(paletteWhite);
      _snxtroIntr ->setPalette(paletteWhite);
      _snxtroAc   ->setPalette(paletteWhite);
      _snxtroSol  ->setPalette(paletteWhite);
    }
    else {
    _snxtroSampl->setEnabled(false);
    _snxtroIntr ->setEnabled(false);
    _snxtroAc   ->setEnabled(false);
    _snxtroSol  ->setEnabled(false);
    _snxtroSampl->setPalette(paletteGray);
    _snxtroIntr ->setPalette(paletteGray);
    _snxtroAc   ->setPalette(paletteGray);
    _snxtroSol  ->setPalette(paletteGray);
    }
  }


}
