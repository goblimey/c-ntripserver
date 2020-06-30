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
 * Class:      reqcDlg
 *
 * Purpose:    Displays the teqc-like editing options
 *
 * Author:     L. Mervart
 *
 * Created:    28-Mar-2012
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include <QCloseEvent>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QWhatsThis>

#include "reqcdlg.h"
#include "bncsettings.h"

using namespace std;


// Constructor
////////////////////////////////////////////////////////////////////////////
reqcDlg::reqcDlg(QWidget* parent) : QDialog(parent) {

  setWindowTitle(tr("RINEX Editing Options"));

  int ww = QFontMetrics(font()).width('w');

  const QString timeFmtString = "yyyy-MM-dd hh:mm:ss";

  _reqcRnxVersion        = new QComboBox(this);
  _reqcSampling          = new QComboBox(this);
  _reqcStartDateTime     = new QDateTimeEdit(this);
  _reqcStartDateTime->setDisplayFormat(timeFmtString);
  _reqcEndDateTime       = new QDateTimeEdit(this);
  _reqcEndDateTime->setDisplayFormat(timeFmtString);
  _reqcRunBy             = new QLineEdit(this);
  _reqcUseObsTypes       = new QLineEdit(this);
  _reqcComment           = new QLineEdit(this);
  _reqcOldMarkerName     = new QLineEdit(this);
  _reqcNewMarkerName     = new QLineEdit(this);
  _reqcOldAntennaName    = new QLineEdit(this);
  _reqcNewAntennaName    = new QLineEdit(this);
  _reqcOldAntennaNumber  = new QLineEdit(this);
  _reqcNewAntennaNumber  = new QLineEdit(this);
  _reqcOldAntennadN      = new QLineEdit(this);
  _reqcNewAntennadN      = new QLineEdit(this);
  _reqcOldAntennadE      = new QLineEdit(this);
  _reqcNewAntennadE      = new QLineEdit(this);
  _reqcOldAntennadU      = new QLineEdit(this);
  _reqcNewAntennadU      = new QLineEdit(this);
  _reqcOldReceiverName   = new QLineEdit(this);
  _reqcNewReceiverName   = new QLineEdit(this);
  _reqcOldReceiverNumber = new QLineEdit(this);
  _reqcNewReceiverNumber = new QLineEdit(this);

  _reqcRnxVersion->setEditable(false);
  _reqcRnxVersion->addItems(QString("2,3").split(","));
  _reqcRnxVersion->setMaximumWidth(7*ww);

  _reqcSampling->setEditable(false);
  _reqcSampling->addItems(QString("0.1 sec,1 sec,5 sec,10 sec,15 sec,30 sec,60 sec").split(","));
  bncSettings settings;
  int ll = _reqcSampling->findText(settings.value("reqcSampling").toString());
  if (ll != -1) {
    _reqcSampling->setCurrentIndex(ll);
  }

  // Read Options
  // ------------
  int kk = _reqcRnxVersion->findText(settings.value("reqcRnxVersion").toString());
  if (kk != -1) {
    _reqcRnxVersion->setCurrentIndex(kk);
  }
  _reqcSampling->findText(settings.value("reqcSampling").toString());
  if (settings.value("reqcStartDateTime").toString().isEmpty()) {
    _reqcStartDateTime->setDateTime(QDateTime::fromString("1967-11-02T00:00:00", Qt::ISODate));
  }
  else {
    _reqcStartDateTime->setDateTime(settings.value("reqcStartDateTime").toDateTime());
  }
  if (settings.value("reqcEndDateTime").toString().isEmpty()) {
    _reqcEndDateTime->setDateTime(QDateTime::fromString("2099-01-01T00:00:00", Qt::ISODate));
  }
  else {
    _reqcEndDateTime->setDateTime(settings.value("reqcEndDateTime").toDateTime());
  }
  _reqcRunBy->setText(settings.value("reqcRunBy").toString());
  _reqcUseObsTypes->setText(settings.value("reqcUseObsTypes").toString());
  _reqcComment->setText(settings.value("reqcComment").toString());
  _reqcOldMarkerName->setText(settings.value("reqcOldMarkerName").toString());
  _reqcNewMarkerName->setText(settings.value("reqcNewMarkerName").toString());
  _reqcOldAntennaName->setText(settings.value("reqcOldAntennaName").toString());
  _reqcNewAntennaName->setText(settings.value("reqcNewAntennaName").toString());
  _reqcOldAntennaNumber->setText(settings.value("reqcOldAntennaNumber").toString());
  _reqcNewAntennaNumber->setText(settings.value("reqcNewAntennaNumber").toString());
  _reqcOldAntennadN->setText(settings.value("reqcOldAntennadN").toString());
  _reqcNewAntennadN->setText(settings.value("reqcNewAntennadN").toString());
  _reqcOldAntennadE->setText(settings.value("reqcOldAntennadE").toString());
  _reqcNewAntennadE->setText(settings.value("reqcNewAntennadE").toString());
  _reqcOldAntennadU->setText(settings.value("reqcOldAntennadU").toString());
  _reqcNewAntennadU->setText(settings.value("reqcNewAntennadU").toString());
  _reqcOldReceiverName->setText(settings.value("reqcOldReceiverName").toString());
  _reqcNewReceiverName->setText(settings.value("reqcNewReceiverName").toString());
  _reqcOldReceiverNumber->setText(settings.value("reqcOldReceiverNumber").toString());
  _reqcNewReceiverNumber->setText(settings.value("reqcNewReceiverNumber").toString());


  QString hlp = settings.value("reqcV2Priority").toString();
  if (hlp.isEmpty()) {
    hlp = "G:12&PWCSLX G:5&IQX R:12&PC R:3&IQX R:46&ABX E:16&BCXZ E:578&IQX J:1&SLXCZ J:26&SLX J:5&IQX C:267&IQX C:128&DPX C:7&DPZ I:ABCX S:1&C S:5&IQX";
  }
  _reqcV2Priority = new QLineEdit(hlp);

  // Dialog Layout
  // -------------
  QGridLayout* grid = new QGridLayout;

  int ir = 0;
  grid->addWidget(new QLabel("RINEX Version"),    ir, 1);
  grid->addWidget(_reqcRnxVersion,                ir, 2);
  grid->addWidget(new QLabel("Sampling"),         ir, 3, Qt::AlignRight);
  grid->addWidget(_reqcSampling,                  ir, 4);
  ++ir;
  grid->addWidget(new QLabel("Version 2 signal priority"),  ir, 1);
  grid->addWidget(_reqcV2Priority,                          ir, 2, 1, 4);
  ++ir;
  grid->addWidget(new QLabel("Start"),            ir, 1);
  grid->addWidget(_reqcStartDateTime,             ir, 2);
  grid->addWidget(new QLabel("End"),              ir, 3, Qt::AlignRight);
  grid->addWidget(_reqcEndDateTime,               ir, 4);
  ++ir;
  grid->addWidget(new QLabel("Run By"),           ir, 0);
  grid->addWidget(_reqcRunBy,                     ir, 1);
  ++ir;
  grid->addWidget(new QLabel("Use Obs. Types"),   ir, 0);
  grid->addWidget(_reqcUseObsTypes,               ir, 1, 1, 4);
  ++ir;
  grid->addWidget(new QLabel("Comment(s)"),       ir, 0);
  grid->addWidget(_reqcComment,                   ir, 1, 1, 4);
  ++ir;
  grid->addWidget(new QLabel("Old"),              ir, 1, 1, 2, Qt::AlignCenter);
  grid->addWidget(new QLabel("New"),              ir, 3, 1, 2, Qt::AlignCenter);
  ++ir;
  grid->addWidget(new QLabel("Marker Name"),      ir, 0);
  grid->addWidget(_reqcOldMarkerName,             ir, 1, 1, 2);
  grid->addWidget(_reqcNewMarkerName,             ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Antenna Name"),     ir, 0);
  grid->addWidget(_reqcOldAntennaName,            ir, 1, 1, 2);
  grid->addWidget(_reqcNewAntennaName,            ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Antenna Number"),   ir, 0);
  grid->addWidget(_reqcOldAntennaNumber,          ir, 1, 1, 2);
  grid->addWidget(_reqcNewAntennaNumber,          ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Antenna ecc. dN"),  ir, 0);
  grid->addWidget(_reqcOldAntennadN,              ir, 1, 1, 2);
  grid->addWidget(_reqcNewAntennadN,              ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Antenna ecc. dE"),  ir, 0);
  grid->addWidget(_reqcOldAntennadE,              ir, 1, 1, 2);
  grid->addWidget(_reqcNewAntennadE,              ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Antenna ecc. dU"),  ir, 0);
  grid->addWidget(_reqcOldAntennadU,              ir, 1, 1, 2);
  grid->addWidget(_reqcNewAntennadU,              ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Receiver Name"),    ir, 0);
  grid->addWidget(_reqcOldReceiverName,           ir, 1, 1, 2);
  grid->addWidget(_reqcNewReceiverName,           ir, 3, 1, 2);
  ++ir;
  grid->addWidget(new QLabel("Receiver Number"),  ir, 0);
  grid->addWidget(_reqcOldReceiverNumber,         ir, 1, 1, 2);
  grid->addWidget(_reqcNewReceiverNumber,         ir, 3, 1, 2);


  slotReqcTextChanged();
  connect(_reqcRnxVersion, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(slotReqcTextChanged()));

  _buttonWhatsThis = new QPushButton(tr("Help=Shift+F1"), this);
  connect(_buttonWhatsThis, SIGNAL(clicked()), this, SLOT(slotWhatsThis()));

  _buttonOK = new QPushButton(tr("OK / Save"), this);
  connect(_buttonOK, SIGNAL(clicked()), this, SLOT(slotOK()));

  _buttonCancel = new QPushButton(tr("Cancel"), this);
  connect(_buttonCancel, SIGNAL(clicked()), this, SLOT(close()));

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(_buttonWhatsThis);
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(_buttonOK);
  buttonLayout->addWidget(_buttonCancel);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(grid);
  mainLayout->addLayout(buttonLayout);

  // WhatsThis, RINEX Editing & QC
  // -----------------------------
  _reqcRnxVersion->setWhatsThis(tr("<p>Select version number of emerging new RINEX file.</p><p>Note the following:</p><p>When converting <u>RINEX Version 2 to Version 3 </u>Observation files, the tracking mode or channel information (signal attribute, see RINEX Version 3 documentation) in the (last out of the three characters) observation code is left blank if unknown.</p><p>When converting <u>RINEX Version 3 to Version 2</u>, the mapping of observations follows a 'Signal priority list' with signal attributes as defined in RINEX Version 3. <i>[key: reqcRnxVersion]</i></p>"));
  _reqcSampling->setWhatsThis(tr("<p>Select sampling rate of emerging new RINEX Observation file. <i>[key: reqcSampling]</i></p>"));
  _reqcV2Priority->setWhatsThis(tr("<p>Specify a priority list of characters defining signal attributes as defined in RINEX Version 3. Priorities will be used to map observations with RINEX Version 3 attributes from incoming streams to Version 2. The underscore character '_' stands for undefined attributes. A question mark '?' can be used as wildcard which represents any one character.</p><p>Signal priorities can be specified as equal for all systems, as system specific or as system and freq. specific. For example: </li><ul><li>'CWPX_?' (General signal priorities valid for all GNSS) </li><li>'C:IQX I:ABCX' (System specific signal priorities for BDS and IRNSS) </li><li>'G:12&PWCSLXYN G:5&IQX R:12&PC R:3&IQX' (System and frequency specific signal priorities) </li></ul>Default is the following priority list 'G:12&PWCSLX G:5&IQX R:12&PC R:3&IQX R:46&ABX E:16&BCXZ E:578&IQX J:1&SLXCZ J:26&SLX J:5&IQX C:267&IQX C:128&DPX C:7&DPZ I:ABCX S:1&C S:5&IQX'. <i>[key: reqcV2Priority]</i></p>"));
  _reqcStartDateTime->setWhatsThis(tr("<p>Specify begin of emerging new RINEX Observation file. <i>[key: reqcStartDateTime]</i></p>"));
  _reqcEndDateTime->setWhatsThis(tr("<p>Specify end of emerging new RINEX Observation file. <i>[key: reqcEndDateTime]</i></p>"));
  _reqcRunBy->setWhatsThis(tr("<p>Specify a 'RUN BY' string to be included in the emerging new RINEX file header.</p><p>Default is an empty option field, meaning the operator's user ID is used as 'RUN BY' string. <i>[key: reqcRunBy]</i></p>"));
  _reqcComment->setWhatsThis(tr("<p>Specifying a comment line text to be added to the emerging new RINEX file header is an option. Any introduction of newline specification '\\n' in this enforces the beginning of a further comment line. Comment line(s) will be added to the header after the 'PGM / RUN BY / DATE' record.</p><p>Default is an empty option field meaning that no additional comment line is added to the RINEX header. <i>[key: reqcComment]</i></p>"));
  _reqcUseObsTypes->setWhatsThis(tr("<p>This option lets you limit the RINEX output to specific observation types. Examples:</p><p><ul><li>G:C1C G:L1C R:C1C R:C1P S:C1C C:C1I C:L1I E:C1X E:L1X<br>(Valid for output of RINEX Version 3; output contains GPS C1C and L1C, GLONASS C1C and C1P, SBAS C1C, BeiDou C1C, C1I andL1I, Galileo C1X and L1X.)</li><li>C1 L2 L5<br>(Valid for output of RINEX Version 2 with mapping of Version 3 signals to Version 2 according to 'Version 2 Signal Priority'; output contains C1, L2 and L5 observations from any GNSS system.)</li></ul></p><p>Default is an empty option field, meaning that the RINEX output file contains all observations made available through RINEX input file. <i>[key: reqcUseObsTypes]</i></p>"));
  _reqcOldMarkerName->setWhatsThis(tr("<p>Enter old Marker Name in RINEX Observation file.</p><p>Default is an empty option field. <i>[key: reqcOldMarkerName]</i></p>"));
  _reqcNewMarkerName->setWhatsThis(tr("<p>Enter new Marker Name in RINEX Observation file.</p><p>If option 'Old Marker Name' is either left blank or its content is specified as given in the RINEX input file, then the marker name in the RINEX output file will be specified by 'New Marker Name'</p><p>Default is an empty option field, meaning that the content of the Marker Name data field in the RINEX file will not be changed. <i>[key: reqcNewMarkerName]</i></p>"));
  _reqcOldAntennaName->setWhatsThis(tr("<p>Enter old Antenna Name in RINEX Observation file.</p><p>Default is an empty option field. <i>[key: reqcOldAntennaName]</i></p>"));
  _reqcNewAntennaName->setWhatsThis(tr("<p>Enter new Antenna Name in RINEX Observation file.</p><p>If option 'Old Antenna Name' is either left blank or its content is specified as given in the RINEX input file, then the antenna name in the RINEX output file will be specified by 'New Antenna Name'</p><p>Default is an empty option field, meaning that the content of the Antenna Name data field in the RINEX file will not be changed. <i>[key: reqcNewAntennaName]</i></p>"));
  _reqcOldAntennaNumber->setWhatsThis(tr("<p>Enter old Antenna Number in RINEX Observation file.</p><p>Default is an empty option field. <i>[key: reqcOldAntennaNumber]</i></p>"));
  _reqcNewAntennaNumber->setWhatsThis(tr("<p>Enter new Antenna Number in RINEX Observation file.</p><p>If option 'Old Antenna Number' is either left blank or its content is specified as given in the RINEX input file, then the antenna number in the RINEX output file will be specified by 'New Antenna Number'</p><p>Default is an empty option field, meaning that the content of the Antenna Number data field in the RINEX file will not be changed. <i>[key: reqcNewAntennaNumber]</i></p>"));
  _reqcOldAntennadN->setWhatsThis(tr("<p>Enter old North Antenna Eccentricity in RINEX Observation file.</p><p>Default is an empty option field. <i>[key: reqcOldAntennadN]</i></p>"));
  _reqcNewAntennadN->setWhatsThis(tr("<p>Enter new North Antenna Eccentricity in RINEX Observation file.</p><p>If option 'Old Antenna North Eccentricity' is either left blank or its content is specified as given in the RINEX input file, then the north antenna eccentricity in the RINEX output file will be specified by 'New North Antenna Eccentricity'</p><p>Default is an empty option field, meaning that the content of the North Antenna Eccentricity data field in the RINEX file will not be changed. <i>[key: reqcNewAntennadN]</i></p>"));
  _reqcOldAntennadE->setWhatsThis(tr("<p>Enter old East Antenna Eccentricity in RINEX Observation file.</p><p>Default is an empty option field. <i>[key: reqcOldAntennadE]</i></p>"));
  _reqcNewAntennadE->setWhatsThis(tr("<p>Enter new East Antenna Eccentricity in RINEX Observation file.</p><p>If option 'Old Antenna East Eccentricity' is either left blank or its content is specified as given in the RINEX input file, then the east antenna eccentricity in the RINEX output file will be specified by 'New East Antenna Eccentricity'</p><p>Default is an empty option field, meaning that the content of the East Antenna Eccentricity data field in the RINEX file will not be changed. <i>[key: reqcNewAntennadE]</i></p>"));
  _reqcOldAntennadU->setWhatsThis(tr("<p>Enter old Up Antenna Eccentricity in RINEX Observation file.</p><p>Default is an empty option field. <i>[key: reqcOldAntennadU]</i></p>"));
  _reqcNewAntennadU->setWhatsThis(tr("<p>Enter new Up Antenna Eccentricity in RINEX Observation file.</p><p>If option 'Old Antenna Up Eccentricity' is either left blank or its content is specified as given in the RINEX input file, then the up antenna eccentricity in the RINEX output file will be specified by 'New Up Antenna Eccentricity'</p><p>Default is an empty option field, meaning that the content of the Up Antenna Eccentricity data field in the RINEX file will not be changed. <i>[key: reqcNewAntennadU]</i></p>"));
  _reqcOldReceiverName->setWhatsThis(tr("<p>Enter old Receiver Name in RINEX Observation file.<p>Default is an empty option field. <i>[key: reqcOldReceiverName]</i></p>"));
  _reqcNewReceiverName->setWhatsThis(tr("<p>Enter new Receiver Name in RINEX Observation file.</p><p>If option 'Old Receiver Name' is either left blank or its content is specified as given in the RINEX input file, then the receiver name in the RINEX output file will be specified by 'New Receiver Name'</p><p>Default is an empty option field, meaning that the content of the Receiver Name data field in the RINEX file will not be changed. <i>[key: reqcNewReceiverName]</i></p>"));
  _reqcOldReceiverNumber->setWhatsThis(tr("<p>Enter old Receiver Number in RINEX Observation file.<p>Default is an empty option field. <i>[key: reqcOldReceiverNumber]</i></p>"));
  _reqcNewReceiverNumber->setWhatsThis(tr("<p>Enter new Receiver Number in RINEX Observation file.</p><p>If option 'Old Receiver Number' is either left blank or its content is specified as given in the RINEX input file, then the receiver number in the RINEX output file will be specified by 'New Receiver Number'</p><p>Default is an empty option field, meaning that the content of the Receiver Number data field in the RINEX file will not be changed. <i>[key: reqcNewReceiverNumber]</i></p>"));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
reqcDlg::~reqcDlg() {
  delete _buttonOK;
  delete _buttonCancel;
  delete _buttonWhatsThis;
}

// Accept the Options
////////////////////////////////////////////////////////////////////////////
void reqcDlg::slotOK() {
  saveOptions();
  done(0);
}

// Whats This Help
////////////////////////////////////////////////////////////////////////////
void reqcDlg::slotWhatsThis() {
  QWhatsThis::enterWhatsThisMode();
}

// Close Dialog gracefully
////////////////////////////////////////////////////////////////////////////
void reqcDlg::closeEvent(QCloseEvent* event) {

  int iRet = QMessageBox::question(this, "Close", "Save Options?",
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::Cancel);

  if      (iRet == QMessageBox::Cancel) {
    event->ignore();
    return;
  }
  else if (iRet == QMessageBox::Yes) {
    saveOptions();
  }

  QDialog::closeEvent(event);
}

// Save Selected Options
////////////////////////////////////////////////////////////////////////////
void reqcDlg::saveOptions() {

  bncSettings settings;

  settings.setValue("reqcRnxVersion"       , _reqcRnxVersion->currentText());
  settings.setValue("reqcSampling"         , _reqcSampling->currentText());
  settings.setValue("reqcV2Priority"       , _reqcV2Priority->text());
  settings.setValue("reqcStartDateTime"    , _reqcStartDateTime->dateTime().toString(Qt::ISODate));
  settings.setValue("reqcEndDateTime"      , _reqcEndDateTime->dateTime().toString(Qt::ISODate));
  settings.setValue("reqcRunBy"            , _reqcRunBy->text());
  settings.setValue("reqcUseObsTypes"      , _reqcUseObsTypes->text());
  settings.setValue("reqcComment"          , _reqcComment->text());
  settings.setValue("reqcOldMarkerName"    , _reqcOldMarkerName->text());
  settings.setValue("reqcNewMarkerName"    , _reqcNewMarkerName->text());
  settings.setValue("reqcOldAntennaName"   , _reqcOldAntennaName->text());
  settings.setValue("reqcNewAntennaName"   , _reqcNewAntennaName->text());
  settings.setValue("reqcOldAntennaNumber" , _reqcOldAntennaNumber->text());
  settings.setValue("reqcNewAntennaNumber" , _reqcNewAntennaNumber->text());
  settings.setValue("reqcOldAntennadN"     , _reqcOldAntennadN->text());
  settings.setValue("reqcNewAntennadN"     , _reqcNewAntennadN->text());
  settings.setValue("reqcOldAntennadE"     , _reqcOldAntennadE->text());
  settings.setValue("reqcNewAntennadE"     , _reqcNewAntennadE->text());
  settings.setValue("reqcOldAntennadU"     , _reqcOldAntennadU->text());
  settings.setValue("reqcNewAntennadU"     , _reqcNewAntennadU->text());
  settings.setValue("reqcNewAntennaNumber" , _reqcNewAntennaNumber->text());
  settings.setValue("reqcOldReceiverName"  , _reqcOldReceiverName->text());
  settings.setValue("reqcNewReceiverName"  , _reqcNewReceiverName->text());
  settings.setValue("reqcOldReceiverNumber", _reqcOldReceiverNumber->text());
  settings.setValue("reqcNewReceiverNumber", _reqcNewReceiverNumber->text());
}

//  Reqc Text Changed
////////////////////////////////////////////////////////////////////////////
void reqcDlg::slotReqcTextChanged(){

  const static QPalette paletteWhite(QColor(255, 255, 255));
  const static QPalette paletteGray(QColor(230, 230, 230));

  if (sender() == 0 || sender() == _reqcRnxVersion) {
    if (_reqcRnxVersion->currentText() == "2") {
      _reqcV2Priority->setPalette(paletteWhite);
      _reqcV2Priority->setEnabled(true);
    }
    else {
      _reqcV2Priority->setPalette(paletteGray);
      _reqcV2Priority->setEnabled(false);
    }
  }
}

