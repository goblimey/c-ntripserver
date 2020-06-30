
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnsWindow
 *
 * Purpose:    This class implements the main application window.
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bnswindow.h" 
#include "bnshlpdlg.h" 
#include "bnssettings.h" 
#include "bnscustomtrafo.h" 
#include "bnsversion.h"

using namespace std;

// About Dialog - Constructor
////////////////////////////////////////////////////////////////////////////
bnsAboutDlg::bnsAboutDlg(QWidget* parent) : 
   QDialog(parent) {

  QTextBrowser* tb = new QTextBrowser;
  QUrl url; url.setPath(":bnsabout.html");
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
  dlgLayout->addWidget(new QLabel("BKG Ntrip State Space Server (BNS) Version " BNSVERSION), 0,1);
  dlgLayout->addWidget(tb,1,0,1,2);
  dlgLayout->addWidget(_closeButton,2,1,Qt::AlignRight);  

  setLayout(dlgLayout);
  resize(60*ww, 60*ww);
  show();
}

// About Dialog - Destructor
////////////////////////////////////////////////////////////////////////////
bnsAboutDlg::~bnsAboutDlg() {
}; 

// Flowchart Dialog - Constructor
////////////////////////////////////////////////////////////////////////////
bnsFlowchartDlg::bnsFlowchartDlg(QWidget* parent) :
   QDialog(parent) {

  int ww = QFontMetrics(font()).width('w');
  QPushButton* _closeButton = new QPushButton("Close");
  _closeButton->setMaximumWidth(10*ww);
  connect(_closeButton, SIGNAL(clicked()), this, SLOT(close()));

  QGridLayout* dlgLayout = new QGridLayout();
  QLabel* img = new QLabel();
  img->setPixmap(QPixmap(":bnsflowchart.png"));
  dlgLayout->addWidget(img, 0,0);
  dlgLayout->addWidget(_closeButton,1,0,Qt::AlignLeft);

  setLayout(dlgLayout);
  show();
}

// Flowchart Dialog - Destructor
////////////////////////////////////////////////////////////////////////////
bnsFlowchartDlg::~bnsFlowchartDlg() {
};

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsWindow::bnsWindow() {

  _bns = 0;

  bnsSettings settings;

  QString fontString = settings.value("font").toString();
  if ( !fontString.isEmpty() ) {
    QFont newFont;
    if (newFont.fromString(fontString)) {
      QApplication::setFont(newFont);
    }
  }
  
  int ww = QFontMetrics(this->font()).width('w');
  setMinimumSize(77*ww, 65*ww);
  setWindowTitle(tr("BKG Ntrip State Space Server (BNS) Version " BNSVERSION));
  setWindowIcon(QPixmap(":ntrip-logo.png"));

  // Create Actions
  // --------------
  _actHelp = new QAction(tr("&Help Contents"),this);
  connect(_actHelp, SIGNAL(triggered()), SLOT(slotHelp()));

  _actAbout = new QAction(tr("&About BNS"),this);
  connect(_actAbout, SIGNAL(triggered()), SLOT(slotAbout()));

  _actFlowchart = new QAction(tr("&Flow Chart"),this);
  connect(_actFlowchart, SIGNAL(triggered()), SLOT(slotFlowchart()));

  _actFontSel = new QAction(tr("Select &Font"),this);
  connect(_actFontSel, SIGNAL(triggered()), SLOT(slotFontSel()));

  _actSaveOpt = new QAction(tr("&Save Options"),this);
  connect(_actSaveOpt, SIGNAL(triggered()), SLOT(slotSaveOptions()));

  _actQuit  = new QAction(tr("&Quit"),this);
  connect(_actQuit, SIGNAL(triggered()), SLOT(close()));

  _actWhatsThis= new QAction(tr("Help=Shift+F1"),this);
  connect(_actWhatsThis, SIGNAL(triggered()), SLOT(slotWhatsThis()));

  _actStart = new QAction(tr("Sta&rt"),this);
  connect(_actStart, SIGNAL(triggered()), SLOT(slotStart()));

  _actStop = new QAction(tr("Sto&p"),this);
  connect(_actStop, SIGNAL(triggered()), SLOT(slotStop()));
  _actStop->setEnabled(false);

  CreateMenu();
  AddToolbar();

  // Canvas with Editable Fields
  // ---------------------------
  _canvas = new QWidget;
  setCentralWidget(_canvas);

  // Proxy Options
  // -------------
  _proxyHostLineEdit  = new QLineEdit(settings.value("proxyHost").toString());
  _proxyPortLineEdit  = new QLineEdit(settings.value("proxyPort").toString());

  // General Options
  // ---------------
  _logFileLineEdit    = new QLineEdit(settings.value("logFile").toString());
  _fileAppendCheckBox  = new QCheckBox();
  _fileAppendCheckBox->setCheckState(Qt::CheckState(settings.value("fileAppend").toInt()));
  _autoStartCheckBox  = new QCheckBox();
  _autoStartCheckBox->setCheckState(Qt::CheckState(
                                    settings.value("autoStart").toInt()));

  // RINEX Ephemeris Options
  // -----------------------
  _ephHostLineEdit  = new QLineEdit(settings.value("ephHost").toString());
  _ephPortLineEdit  = new QLineEdit(settings.value("ephPort").toString());
  _ephEchoLineEdit  = new QLineEdit(settings.value("ephEcho").toString());

  // Clocks & Orbits Options
  // -----------------------
  _clkPortLineEdit  = new QLineEdit(settings.value("clkPort").toString());
  _inpEchoLineEdit  = new QLineEdit(settings.value("inpEcho").toString());


  // Broadcast Corrections I Options
  // -------------------------------
  _outHost_1_LineEdit    = new QLineEdit(settings.value("outHost1").toString());
  _outPort_1_LineEdit    = new QLineEdit(settings.value("outPort1").toString());
  _password_1_LineEdit   = new QLineEdit(settings.value("password1").toString());
  _password_1_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_1_LineEdit = new QLineEdit(settings.value("mountpoint_1").toString());
  _refSys_1_ComboBox = new QComboBox;
  _refSys_1_ComboBox->setEditable(false);
  _refSys_1_ComboBox->addItems(QString("IGS05,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,Custom").split(","));
  int ii = _refSys_1_ComboBox->findText(settings.value("refSys_1").toString());
  if (ii != -1) {
    _refSys_1_ComboBox->setCurrentIndex(ii);
  }
  _outFile_1_LineEdit    = new QLineEdit(settings.value("outFile_1").toString());
  _CoM_1_CheckBox  = new QCheckBox();
  _CoM_1_CheckBox->setCheckState(Qt::CheckState(settings.value("CoM_1").toInt()));

  // Broadcast Corrections II Options
  // --------------------------------
  _outHost_2_LineEdit    = new QLineEdit(settings.value("outHost2").toString());
  _outPort_2_LineEdit    = new QLineEdit(settings.value("outPort2").toString());
  _password_2_LineEdit   = new QLineEdit(settings.value("password2").toString());
  _password_2_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_2_LineEdit = new QLineEdit(settings.value("mountpoint_2").toString());
  _refSys_2_ComboBox = new QComboBox;
  _refSys_2_ComboBox->setEditable(false);
  _refSys_2_ComboBox->addItems(QString("IGS05,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,Custom").split(","));
  ii = _refSys_2_ComboBox->findText(settings.value("refSys_2").toString());
  if (ii != -1) {
    _refSys_2_ComboBox->setCurrentIndex(ii);
  }
  _outFile_2_LineEdit    = new QLineEdit(settings.value("outFile_2").toString());
  _CoM_2_CheckBox  = new QCheckBox();
  _CoM_2_CheckBox->setCheckState(Qt::CheckState(settings.value("CoM_2").toInt()));

  // Broadcast Corrections III Options
  // ---------------------------------
  _outHost_3_LineEdit    = new QLineEdit(settings.value("outHost3").toString());
  _outPort_3_LineEdit    = new QLineEdit(settings.value("outPort3").toString());
  _password_3_LineEdit   = new QLineEdit(settings.value("password3").toString());
  _password_3_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_3_LineEdit = new QLineEdit(settings.value("mountpoint_3").toString());
  _refSys_3_ComboBox = new QComboBox;
  _refSys_3_ComboBox->setEditable(false);
  _refSys_3_ComboBox->addItems(QString("IGS05,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,Custom").split(","));
  ii = _refSys_3_ComboBox->findText(settings.value("refSys_3").toString());
  if (ii != -1) {
    _refSys_3_ComboBox->setCurrentIndex(ii);
  }
  _outFile_3_LineEdit    = new QLineEdit(settings.value("outFile_3").toString());
  _CoM_3_CheckBox  = new QCheckBox();
  _CoM_3_CheckBox->setCheckState(Qt::CheckState(settings.value("CoM_3").toInt()));

  // Broadcast Corrections IV Options
  // --------------------------------
  _outHost_4_LineEdit    = new QLineEdit(settings.value("outHost4").toString());
  _outPort_4_LineEdit    = new QLineEdit(settings.value("outPort4").toString());
  _password_4_LineEdit   = new QLineEdit(settings.value("password4").toString());
  _password_4_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_4_LineEdit = new QLineEdit(settings.value("mountpoint_4").toString());
  _refSys_4_ComboBox = new QComboBox;
  _refSys_4_ComboBox->setEditable(false);
  _refSys_4_ComboBox->addItems(QString("IGS05,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,Custom").split(","));
  ii = _refSys_4_ComboBox->findText(settings.value("refSys_4").toString());
  if (ii != -1) {
    _refSys_4_ComboBox->setCurrentIndex(ii);
  }
  _outFile_4_LineEdit    = new QLineEdit(settings.value("outFile_4").toString());
  _CoM_4_CheckBox  = new QCheckBox();
  _CoM_4_CheckBox->setCheckState(Qt::CheckState(settings.value("CoM_4").toInt()));

  // Broadcast Corrections V Options
  // -------------------------------
  _outHost_5_LineEdit    = new QLineEdit(settings.value("outHost5").toString());
  _outPort_5_LineEdit    = new QLineEdit(settings.value("outPort5").toString());
  _password_5_LineEdit   = new QLineEdit(settings.value("password5").toString());
  _password_5_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_5_LineEdit = new QLineEdit(settings.value("mountpoint_5").toString());
  _refSys_5_ComboBox = new QComboBox;
  _refSys_5_ComboBox->setEditable(false);
  _refSys_5_ComboBox->addItems(QString("IGS05,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,Custom").split(","));
  ii = _refSys_5_ComboBox->findText(settings.value("refSys_5").toString());
  if (ii != -1) {
    _refSys_5_ComboBox->setCurrentIndex(ii);
  }
  _outFile_5_LineEdit    = new QLineEdit(settings.value("outFile_5").toString());
  _CoM_5_CheckBox  = new QCheckBox();
  _CoM_5_CheckBox->setCheckState(Qt::CheckState(settings.value("CoM_5").toInt()));

  // Broadcast Corrections VI Options
  // --------------------------------
  _outHost_6_LineEdit    = new QLineEdit(settings.value("outHost6").toString());
  _outPort_6_LineEdit    = new QLineEdit(settings.value("outPort6").toString());
  _password_6_LineEdit   = new QLineEdit(settings.value("password6").toString());
  _password_6_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_6_LineEdit = new QLineEdit(settings.value("mountpoint_6").toString());
  _refSys_6_ComboBox = new QComboBox;
  _refSys_6_ComboBox->setEditable(false);
  _refSys_6_ComboBox->addItems(QString("IGS05,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,Custom").split(","));
  ii = _refSys_6_ComboBox->findText(settings.value("refSys_6").toString());
  if (ii != -1) {
    _refSys_6_ComboBox->setCurrentIndex(ii);
  }
  _outFile_6_LineEdit    = new QLineEdit(settings.value("outFile_6").toString());
  _CoM_6_CheckBox  = new QCheckBox();
  _CoM_6_CheckBox->setCheckState(Qt::CheckState(settings.value("CoM_6").toInt()));

  // Broadcast Corrections VII Options
  // ---------------------------------
  _outHost_7_LineEdit    = new QLineEdit(settings.value("outHost7").toString());
  _outPort_7_LineEdit    = new QLineEdit(settings.value("outPort7").toString());
  _password_7_LineEdit   = new QLineEdit(settings.value("password7").toString());
  _password_7_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_7_LineEdit = new QLineEdit(settings.value("mountpoint_7").toString());
  _refSys_7_ComboBox = new QComboBox;
  _refSys_7_ComboBox->setEditable(false);
  _refSys_7_ComboBox->addItems(QString("IGS05,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,Custom").split(","));
  ii = _refSys_7_ComboBox->findText(settings.value("refSys_7").toString());
  if (ii != -1) {
    _refSys_7_ComboBox->setCurrentIndex(ii);
  }
  _outFile_7_LineEdit    = new QLineEdit(settings.value("outFile_7").toString());
  _CoM_7_CheckBox  = new QCheckBox();
  _CoM_7_CheckBox->setCheckState(Qt::CheckState(settings.value("CoM_7").toInt()));

  // Broadcast Corrections VIII Options
  // --------------------------------
  _outHost_8_LineEdit    = new QLineEdit(settings.value("outHost8").toString());
  _outPort_8_LineEdit    = new QLineEdit(settings.value("outPort8").toString());
  _password_8_LineEdit   = new QLineEdit(settings.value("password8").toString());
  _password_8_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_8_LineEdit = new QLineEdit(settings.value("mountpoint_8").toString());
  _refSys_8_ComboBox = new QComboBox;
  _refSys_8_ComboBox->setEditable(false);
  _refSys_8_ComboBox->addItems(QString("IGS05,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,Custom").split(","));
  ii = _refSys_8_ComboBox->findText(settings.value("refSys_8").toString());
  if (ii != -1) {
    _refSys_8_ComboBox->setCurrentIndex(ii);
  }
  _outFile_8_LineEdit    = new QLineEdit(settings.value("outFile_8").toString());
  _CoM_8_CheckBox  = new QCheckBox();
  _CoM_8_CheckBox->setCheckState(Qt::CheckState(settings.value("CoM_8").toInt()));

  // Broadcast Corrections IX Options
  // --------------------------------
  _outHost_9_LineEdit    = new QLineEdit(settings.value("outHost9").toString());
  _outPort_9_LineEdit    = new QLineEdit(settings.value("outPort9").toString());
  _password_9_LineEdit   = new QLineEdit(settings.value("password9").toString());
  _password_9_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_9_LineEdit = new QLineEdit(settings.value("mountpoint_9").toString());
  _refSys_9_ComboBox = new QComboBox;
  _refSys_9_ComboBox->setEditable(false);
  _refSys_9_ComboBox->addItems(QString("IGS05,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,Custom").split(","));
  ii = _refSys_9_ComboBox->findText(settings.value("refSys_9").toString());
  if (ii != -1) {
    _refSys_9_ComboBox->setCurrentIndex(ii);
  }
  _outFile_9_LineEdit    = new QLineEdit(settings.value("outFile_9").toString());
  _CoM_9_CheckBox  = new QCheckBox();
  _CoM_9_CheckBox->setCheckState(Qt::CheckState(settings.value("CoM_9").toInt()));

  // Broadcast Corrections X Options
  // -------------------------------
  _outHost_10_LineEdit    = new QLineEdit(settings.value("outHost10").toString());
  _outPort_10_LineEdit    = new QLineEdit(settings.value("outPort10").toString());
  _password_10_LineEdit   = new QLineEdit(settings.value("password10").toString());
  _password_10_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_10_LineEdit = new QLineEdit(settings.value("mountpoint_10").toString());
  _refSys_10_ComboBox = new QComboBox;
  _refSys_10_ComboBox->setEditable(false);
  _refSys_10_ComboBox->addItems(QString("IGS05,ETRF2000,NAD83,GDA94,SIRGAS95,SIRGAS2000,Custom").split(","));
  ii = _refSys_10_ComboBox->findText(settings.value("refSys_10").toString());
  if (ii != -1) {
    _refSys_10_ComboBox->setCurrentIndex(ii);
  }
  _outFile_10_LineEdit    = new QLineEdit(settings.value("outFile_10").toString());
  _CoM_10_CheckBox  = new QCheckBox();
  _CoM_10_CheckBox->setCheckState(Qt::CheckState(settings.value("CoM_10").toInt()));

  // Broadcast Ephemerides
  // ---------------------
  _outHost_Eph_LineEdit    = new QLineEdit(settings.value("outHostEph").toString());
  _outPort_Eph_LineEdit    = new QLineEdit(settings.value("outPortEph").toString());
  _password_Eph_LineEdit   = new QLineEdit(settings.value("passwordEph").toString());
  _password_Eph_LineEdit->setEchoMode(QLineEdit::Password);
  _mountpoint_Eph_LineEdit = new QLineEdit(settings.value("mountpoint_Eph").toString());
  _samplEphSpinBox = new QSpinBox;
  _samplEphSpinBox->setMinimum(0);
  _samplEphSpinBox->setMaximum(60);
  _samplEphSpinBox->setSingleStep(5);
  _samplEphSpinBox->setMaximumWidth(9*ww);
  _samplEphSpinBox->setValue(settings.value("samplEph").toInt());
  _samplEphSpinBox->setSuffix(" sec");

  // RINEX Clocks Options
  // --------------------
  _rnxPathLineEdit = new QLineEdit(settings.value("rnxPath").toString());
  _rnxIntrComboBox = new QComboBox;
  _rnxIntrComboBox->setEditable(false);
  _rnxIntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  ii = _rnxIntrComboBox->findText(settings.value("rnxIntr").toString());
  if (ii != -1) {
    _rnxIntrComboBox->setCurrentIndex(ii);
  }
  _rnxSamplSpinBox = new QSpinBox;
  _rnxSamplSpinBox->setMinimum(0);
  _rnxSamplSpinBox->setMaximum(60);
  _rnxSamplSpinBox->setSingleStep(5);
  _rnxSamplSpinBox->setMaximumWidth(9*ww);
  _rnxSamplSpinBox->setValue(settings.value("rnxSampl").toInt());
  _rnxSamplSpinBox->setSuffix(" sec");

  // SP3 Orbits Options
  // ------------------
  _sp3PathLineEdit = new QLineEdit(settings.value("sp3Path").toString());
  _sp3IntrComboBox = new QComboBox;
  _sp3IntrComboBox->setEditable(false);
  _sp3IntrComboBox->addItems(QString("1 min,2 min,5 min,10 min,15 min,30 min,1 hour,1 day").split(","));
  ii = _sp3IntrComboBox->findText(settings.value("sp3Intr").toString());
  if (ii != -1) {
    _sp3IntrComboBox->setCurrentIndex(ii);
  }
  _sp3SamplSpinBox = new QSpinBox;
  _sp3SamplSpinBox->setMinimum(0);
  _sp3SamplSpinBox->setMaximum(900);
  _sp3SamplSpinBox->setSingleStep(60);
  _sp3SamplSpinBox->setMaximumWidth(9*ww);
  _sp3SamplSpinBox->setValue(settings.value("sp3Sampl").toInt());
  _sp3SamplSpinBox->setSuffix(" sec");

  // Whats This
  // ----------
  _proxyHostLineEdit->setWhatsThis(tr("<p>If you are running BNS within a protected Local Area Network (LAN), you might need to use a proxy server to access the Internet. Enter your proxy server IP and port number in case one is operated in front of BNS. If you do not know the IP and port of your proxy server, check the proxy server settings in your Internet browser or ask your network administrator.</p><p>Note that IP streaming is sometimes not allowed in a LAN. In this case you need to ask your network administrator for an appropriate modification of the local security policy or for the installation of a TCP relay to the NTRIP broadcasters. If these are not possible, you might need to run BNS outside your LAN on a network that has unobstructed connection to the Internet.</p>"));
  _proxyPortLineEdit->setWhatsThis(tr("<p>Enter your proxy server port number in case one is operated in front of BNS.</p>"));
  _logFileLineEdit->setWhatsThis(tr("<p>Records of BNS activities are shown in the Log section on the bottom of this window. They can be saved into a file when a valid path is specified in the 'Logfile (full path)' field.</p>"));
  _fileAppendCheckBox->setWhatsThis(tr("<p>When BNS is started, new files are created by default and any existing files with the same name will be overwritten. However, users might want to append already existing files following a restart of BNS, a system crash or when BNS crashed. Tick 'Append files' to continue with existing files and keep what has been recorded so far.</p>"));
  _inpEchoLineEdit->setWhatsThis(tr("Specify the full path to a file where incoming clocks and orbits are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that incoming clocks and orbits are not saved."));
  _ephHostLineEdit->setWhatsThis(tr("BNS reads Broadcast Ephemeris in RINEX Version 3 Navigation file format from an IP address. Specify the host IP e.g. of a BNC installation providing this information."));
  _ephPortLineEdit->setWhatsThis(tr("BNS reads Broadcast Ephemeris in RINEX Version 3 Navigation file format from an IP address. Specify the IP port e.g. of a BNC installation providing this information."));
  _ephEchoLineEdit->setWhatsThis(tr("Specify the full path to a file where incoming Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that incoming Broadcast Ephemeris are not saved."));
  _clkPortLineEdit->setWhatsThis(tr("BNS reads Clocks & Orbits referring to the IGS system (X,Y,Z, ECEF) in SP3 format from an IP port. Specify a local IP port e.g. for an RTNet installation to provide this information."));

  _outHost_1_LineEdit->setWhatsThis(tr("BNS can stream clock and orbit Corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPort_1_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_1_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_1_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _refSys_1_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_1_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));

  _outHost_2_LineEdit->setWhatsThis(tr("BNS can stream clock and orbit Corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPort_2_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_2_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_2_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _refSys_2_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_2_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));

  _outHost_3_LineEdit->setWhatsThis(tr("BNS can stream clock and orbit Corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPort_3_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_3_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_3_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _refSys_3_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_3_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));

  _outHost_4_LineEdit->setWhatsThis(tr("BNS can stream clock and orbit Corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPort_4_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_4_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_4_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _refSys_4_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_4_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));

  _outHost_5_LineEdit->setWhatsThis(tr("BNS can stream clock and orbit Corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPort_5_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_5_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_5_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _refSys_5_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_5_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));


  _outHost_6_LineEdit->setWhatsThis(tr("BNS can stream clock and orbit Corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPort_6_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_6_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_6_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _refSys_6_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_6_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));

  _outHost_7_LineEdit->setWhatsThis(tr("BNS can stream clock and orbit Corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPort_7_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_7_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_7_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _refSys_7_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_7_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));

  _outHost_8_LineEdit->setWhatsThis(tr("BNS can stream clock and orbit Corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPort_8_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_8_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_8_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _refSys_8_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_8_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));

  _outHost_9_LineEdit->setWhatsThis(tr("BNS can stream clock and orbit Corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPort_9_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_9_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_9_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _refSys_9_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_9_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));

  _outHost_10_LineEdit->setWhatsThis(tr("BNS can stream clock and orbit Corrections to Broadcast Ephemeris (Broadcast Corrections) in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload corrections."));
  _outPort_10_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_10_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_10_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _refSys_10_ComboBox->setWhatsThis(tr("Select the target reference system for outgoing clock and orbit corrections."));
  _outFile_10_LineEdit->setWhatsThis(tr("Specify the full path to a file where outgoing clock and orbit corrections to Broadcast Ephemeris are saved. Beware that the size of this file can rapidly increase. Default is an empty option field meaning that outgoing corrections are not saved."));

  _outHost_Eph_LineEdit->setWhatsThis(tr("BNS can upload a Broadcast Ephemeris stream in RTCM Version 3 format. Specify the host IP of an NTRIP Broadcaster to upload the stream. An empty option field means that you don't want to upload Broadcast Ephemeris."));
  _outPort_Eph_LineEdit->setWhatsThis(tr("Specify the IP port of an NTRIP Broadcaster to upload the stream. Default is port 80."));
  _mountpoint_Eph_LineEdit->setWhatsThis(tr("Specify the mounpoint for stream upload to an NTRIP Broadcaster."));
  _password_Eph_LineEdit->setWhatsThis(tr("Specify the stream upload password protecting the mounpoint on an NTRIP Broadcaster."));
  _samplEphSpinBox->setWhatsThis(tr("Select the Broadcast Ephemeris sampling interval in seconds. Defaut is '5' meaning that a complete set of Broadcast Ephemeris is uploaded every 5 seconds."));
  _rnxPathLineEdit->setWhatsThis(tr("Specify the path for saving the generated clock corrections as Clock RINEX files. If the specified directory does not exist, BNS will not create Clock RINEX files."));
  _rnxIntrComboBox->setWhatsThis(tr("Select the length of the Clock RINEX file."));
  _rnxSamplSpinBox->setWhatsThis(tr("Select the Clock RINEX file sampling interval in seconds. A value of zero '0' tells BNS to store all available samples into Clock RINEX files."));
  _sp3PathLineEdit->setWhatsThis(tr("Specify the path for saving the generated orbit corrections as SP3 orbit files. If the specified directory does not exist, BNS will not create SP3 orbit files."));
  _sp3IntrComboBox->setWhatsThis(tr("Select the length of the SP3 orbit file."));
  _sp3SamplSpinBox->setWhatsThis(tr("Select the SP3 orbit file sampling interval in seconds. A value of zero '0' tells BNS to store all available samples into SP3 orbit files."));
  _autoStartCheckBox->setWhatsThis(tr("<p>Tick 'Auto start' for auto-start of BNS at startup time in window mode with preassigned processing options.</p>"));
  _CoM_1_CheckBox->setWhatsThis(tr("<p>By default orbit and clock corrections refer to APC. Tick 'Center of Mass' to refer uploaded corrections to CoM.</p>"));
  _CoM_2_CheckBox->setWhatsThis(tr("<p>By default orbit and clock corrections refer to APC. Tick 'Center of Mass' to refer uploaded corrections to CoM.</p>"));
  _CoM_3_CheckBox->setWhatsThis(tr("<p>By default orbit and clock corrections refer to APC. Tick 'Center of Mass' to refer uploaded corrections to CoM.</p>"));


  // TabWidget
  // ---------
  tabs = new QTabWidget();

  // Proxy Tab
  // ---------
  QWidget* tab_prx = new QWidget();
  tabs->setMaximumHeight(20*ww);
  tabs->addTab(tab_prx, "Proxy");

  QGridLayout* layout_prx = new QGridLayout;

  layout_prx->setColumnMinimumWidth(0,9*ww);
  _proxyPortLineEdit->setMaximumWidth(9*ww);

  layout_prx->addWidget(new QLabel("Host"),       0, 0);
  layout_prx->addWidget(_proxyHostLineEdit,       0, 1, 1, 10);
  layout_prx->addWidget(new QLabel("Port"),       1, 0);
  layout_prx->addWidget(_proxyPortLineEdit,       1, 1);
  layout_prx->addWidget(new QLabel("Settings for the proxy in protected networks, leave boxes blank if none."),2, 0, 1, 50, Qt::AlignLeft);
  layout_prx->addWidget(new QLabel("    "),       3, 0);

  tab_prx->setLayout(layout_prx);

  connect(_proxyHostLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // General Tab
  // ----------- 
  QWidget* tab_gen = new QWidget();
  tabs->addTab(tab_gen, "General");

  QGridLayout* layout_gen = new QGridLayout;

  layout_gen->setColumnMinimumWidth(0,9*ww);
  _logFileLineEdit->setMaximumWidth(40*ww);

  layout_gen->addWidget(new QLabel("Logfile (full path)  "), 0, 0);
  layout_gen->addWidget(_logFileLineEdit,                    0, 1);
  layout_gen->addWidget(new QLabel("Append files"),          1, 0);
  layout_gen->addWidget(_fileAppendCheckBox,                 1, 1); 
  layout_gen->addWidget(new QLabel("Auto start"),            2, 0);
  layout_gen->addWidget(_autoStartCheckBox,                  2, 1);
  layout_gen->addWidget(new QLabel("General settings for logfile and file handling."), 3, 0, 1, 50, Qt::AlignLeft);

  tab_gen->setLayout(layout_gen);

  // RINEX Ephemeris Tab
  // -------------------
  QWidget* tab_eph = new QWidget();
  tabs->addTab(tab_eph, "RINEX Ephemeris");

  QGridLayout* layout_eph = new QGridLayout;

  layout_eph->setColumnMinimumWidth(0, 9*ww);
  _ephPortLineEdit->setMaximumWidth(9*ww);

  layout_eph->addWidget(new QLabel("Host"),                   0, 0);
  layout_eph->addWidget(_ephHostLineEdit,                     0, 1, 1, 10);
  layout_eph->addWidget(new QLabel("Port"),                   1, 0);
  layout_eph->addWidget(_ephPortLineEdit,                     1, 1);
  layout_eph->addWidget(new QLabel("Save (full path)"),       2, 0);
  layout_eph->addWidget(_ephEchoLineEdit,                     2, 1, 1, 26);
  layout_eph->addWidget(new QLabel("Read broadcast ephemeris in RINEX Version 3 Navigation format."), 3, 0, 1, 50, Qt::AlignLeft);

  tab_eph->setLayout(layout_eph);

  connect(_ephHostLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Clocks & Orbits Tab
  // -------------------
  QWidget* tab_co = new QWidget();
  tabs->addTab(tab_co,"Clocks && Orbits");

  QGridLayout* layout_co = new QGridLayout;

  layout_co->setColumnMinimumWidth(0, 9*ww);
  _clkPortLineEdit->setMaximumWidth(9*ww);

  layout_co->addWidget(new QLabel("Listening port"),                  0, 0);
  layout_co->addWidget(_clkPortLineEdit,                              0, 1);
  layout_co->addWidget(new QLabel("Save (full path)  "),              1, 0);
  layout_co->addWidget(_inpEchoLineEdit,                              1, 1);
  layout_co->addWidget(new QLabel("Read clocks and orbits in SP3 format."),     2, 0, 1, 50, Qt::AlignLeft);
  layout_co->addWidget(new QLabel(""),                                3, 0);

  tab_co->setLayout(layout_co);

  connect(_clkPortLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Corrections I Tab
  // ---------------------------
  QWidget* tab_cas1 = new QWidget();
  tabs->addTab(tab_cas1, "BC I");

  QGridLayout* layout_cas1 = new QGridLayout;

  layout_cas1->setColumnMinimumWidth(0, 9*ww);
  _outPort_1_LineEdit->setMaximumWidth(9*ww);
  _password_1_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_1_LineEdit->setMaximumWidth(12*ww);
  _refSys_1_ComboBox->setMaximumWidth(12*ww);

  layout_cas1->addWidget(new QLabel("Host"),               0, 0);
  layout_cas1->addWidget(_outHost_1_LineEdit,              0, 1, 1, 3);
  layout_cas1->addWidget(new QLabel("  Port"),             0, 4, Qt::AlignRight);
  layout_cas1->addWidget(_outPort_1_LineEdit,              0, 5, 1, 10);
  layout_cas1->addWidget(new QLabel("Mountpoint"),         1, 0);
  layout_cas1->addWidget(_mountpoint_1_LineEdit,           1, 1);
  layout_cas1->addWidget(new QLabel("Password"),           1, 2, Qt::AlignRight);
  layout_cas1->addWidget(_password_1_LineEdit,             1, 3);
  layout_cas1->addWidget(new QLabel(" "),                  1, 4);
  layout_cas1->addWidget(new QLabel(" "),                  1, 5);
  layout_cas1->addWidget(new QLabel("System"),             2, 0);
  layout_cas1->addWidget(_refSys_1_ComboBox,               2, 1);
  layout_cas1->addWidget(new QLabel("  Save (full path)"), 2, 2, Qt::AlignRight);
  layout_cas1->addWidget(_outFile_1_LineEdit,              2, 3, 1, 30);
  layout_cas1->addWidget(new QLabel("Center of Mass"),     3, 0);
  layout_cas1->addWidget(_CoM_1_CheckBox,                  3, 1);
  layout_cas1->addWidget(new QLabel("Produce broadcast ephemeris corrections, upload to caster, reference system, local storage."), 4, 0, 1, 50);

  tab_cas1->setLayout(layout_cas1);

  connect(_refSys_1_ComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(customTrafo(const QString)));

  connect(_outHost_1_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Corrections II Tab
  // ----------------------------
  QWidget* tab_cas2 = new QWidget();
  tabs->addTab(tab_cas2, "BC II");

  QGridLayout* layout_cas2 = new QGridLayout;

  layout_cas2->setColumnMinimumWidth(0, 9*ww);
  _outPort_2_LineEdit->setMaximumWidth(9*ww);
  _password_2_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_2_LineEdit->setMaximumWidth(12*ww);
  _refSys_2_ComboBox->setMaximumWidth(12*ww);

  layout_cas2->addWidget(new QLabel("Host"),               0, 0);
  layout_cas2->addWidget(_outHost_2_LineEdit,              0, 1, 1, 3);
  layout_cas2->addWidget(new QLabel("  Port"),             0, 4, Qt::AlignRight);
  layout_cas2->addWidget(_outPort_2_LineEdit,              0, 5, 1, 10);
  layout_cas2->addWidget(new QLabel("Mountpoint"),         1, 0);
  layout_cas2->addWidget(_mountpoint_2_LineEdit,           1, 1);
  layout_cas2->addWidget(new QLabel("Password"),           1, 2, Qt::AlignRight);
  layout_cas2->addWidget(_password_2_LineEdit,             1, 3);
  layout_cas2->addWidget(new QLabel(" "),                  1, 4);
  layout_cas2->addWidget(new QLabel(" "),                  1, 5);
  layout_cas2->addWidget(new QLabel("System"),             2, 0);
  layout_cas2->addWidget(_refSys_2_ComboBox,               2, 1);
  layout_cas2->addWidget(new QLabel("  Save (full path)"), 2, 2, Qt::AlignRight);
  layout_cas2->addWidget(_outFile_2_LineEdit,              2, 3, 1, 30);
  layout_cas2->addWidget(new QLabel("Center of Mass"),     3, 0);
  layout_cas2->addWidget(_CoM_2_CheckBox,                  3, 1);
  layout_cas2->addWidget(new QLabel("Produce broadcast ephemeris corrections, upload to caster, reference system, local storage."), 4, 0, 1, 50);

  tab_cas2->setLayout(layout_cas2);

  connect(_refSys_2_ComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(customTrafo(const QString)));

  connect(_outHost_2_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Corrections III Tab
  // -----------------------------
  QWidget* tab_cas3 = new QWidget();
  tabs->addTab(tab_cas3, "BC III");

  QGridLayout* layout_cas3 = new QGridLayout;

  layout_cas3->setColumnMinimumWidth(0, 9*ww);
  _outPort_3_LineEdit->setMaximumWidth(9*ww);
  _password_3_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_3_LineEdit->setMaximumWidth(12*ww);
  _refSys_3_ComboBox->setMaximumWidth(12*ww);

  layout_cas3->addWidget(new QLabel("Host"),               0, 0);
  layout_cas3->addWidget(_outHost_3_LineEdit,              0, 1, 1, 3);
  layout_cas3->addWidget(new QLabel("  Port"),             0, 4, Qt::AlignRight);
  layout_cas3->addWidget(_outPort_3_LineEdit,              0, 5, 1, 10);
  layout_cas3->addWidget(new QLabel("Mountpoint"),         1, 0);
  layout_cas3->addWidget(_mountpoint_3_LineEdit,           1, 1);
  layout_cas3->addWidget(new QLabel("Password"),           1, 2, Qt::AlignRight);
  layout_cas3->addWidget(_password_3_LineEdit,             1, 3);
  layout_cas3->addWidget(new QLabel(" "),                  1, 4);
  layout_cas3->addWidget(new QLabel(" "),                  1, 5);
  layout_cas3->addWidget(new QLabel("System"),             2, 0);
  layout_cas3->addWidget(_refSys_3_ComboBox,               2, 1);
  layout_cas3->addWidget(new QLabel("  Save (full path)"), 2, 2, Qt::AlignRight);
  layout_cas3->addWidget(_outFile_3_LineEdit,              2, 3, 1, 30);
  layout_cas3->addWidget(new QLabel("Center of Mass"),     3, 0);
  layout_cas3->addWidget(_CoM_3_CheckBox,                  3, 1);
  layout_cas3->addWidget(new QLabel("Produce broadcast ephemeris corrections, upload to caster, reference system, local storage."), 4, 0, 1, 50);

  tab_cas3->setLayout(layout_cas3);

  connect(_refSys_3_ComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(customTrafo(const QString)));

  connect(_outHost_3_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Corrections IV Tab
  // ----------------------------
  QWidget* tab_cas4 = new QWidget();
  tabs->addTab(tab_cas4, "BC IV");

  QGridLayout* layout_cas4 = new QGridLayout;

  layout_cas4->setColumnMinimumWidth(0, 9*ww);
  _outPort_4_LineEdit->setMaximumWidth(9*ww);
  _password_4_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_4_LineEdit->setMaximumWidth(12*ww);
  _refSys_4_ComboBox->setMaximumWidth(12*ww);

  layout_cas4->addWidget(new QLabel("Host"),               0, 0);
  layout_cas4->addWidget(_outHost_4_LineEdit,              0, 1, 1, 3);
  layout_cas4->addWidget(new QLabel("  Port"),             0, 4, Qt::AlignRight);
  layout_cas4->addWidget(_outPort_4_LineEdit,              0, 5, 1, 10);
  layout_cas4->addWidget(new QLabel("Mountpoint"),         1, 0);
  layout_cas4->addWidget(_mountpoint_4_LineEdit,           1, 1);
  layout_cas4->addWidget(new QLabel("Password"),           1, 2, Qt::AlignRight);
  layout_cas4->addWidget(_password_4_LineEdit,             1, 3);
  layout_cas4->addWidget(new QLabel(" "),                  1, 4);
  layout_cas4->addWidget(new QLabel(" "),                  1, 5);
  layout_cas4->addWidget(new QLabel("System"),             2, 0);
  layout_cas4->addWidget(_refSys_4_ComboBox,               2, 1);
  layout_cas4->addWidget(new QLabel("  Save (full path)"), 2, 2, Qt::AlignRight);
  layout_cas4->addWidget(_outFile_4_LineEdit,              2, 3, 1, 30);
  layout_cas4->addWidget(new QLabel("Center of Mass"),     3, 0);
  layout_cas4->addWidget(_CoM_4_CheckBox,                  3, 1);
  layout_cas4->addWidget(new QLabel("Produce broadcast ephemeris corrections, upload to caster, reference system, local storage."), 4, 0, 1, 50);

  tab_cas4->setLayout(layout_cas4);

  connect(_refSys_4_ComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(customTrafo(const QString)));

  connect(_outHost_4_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Corrections V Tab
  // ---------------------------
  QWidget* tab_cas5 = new QWidget();
  tabs->addTab(tab_cas5, "BC V");

  QGridLayout* layout_cas5 = new QGridLayout;

  layout_cas5->setColumnMinimumWidth(0, 9*ww);
  _outPort_5_LineEdit->setMaximumWidth(9*ww);
  _password_5_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_5_LineEdit->setMaximumWidth(12*ww);
  _refSys_5_ComboBox->setMaximumWidth(12*ww);

  layout_cas5->addWidget(new QLabel("Host"),               0, 0);
  layout_cas5->addWidget(_outHost_5_LineEdit,              0, 1, 1, 3);
  layout_cas5->addWidget(new QLabel("  Port"),             0, 4, Qt::AlignRight);
  layout_cas5->addWidget(_outPort_5_LineEdit,              0, 5, 1, 10);
  layout_cas5->addWidget(new QLabel("Mountpoint"),         1, 0);
  layout_cas5->addWidget(_mountpoint_5_LineEdit,           1, 1);
  layout_cas5->addWidget(new QLabel("Password"),           1, 2, Qt::AlignRight);
  layout_cas5->addWidget(_password_5_LineEdit,             1, 3);
  layout_cas5->addWidget(new QLabel(" "),                  1, 4);
  layout_cas5->addWidget(new QLabel(" "),                  1, 5);
  layout_cas5->addWidget(new QLabel("System"),             2, 0);
  layout_cas5->addWidget(_refSys_5_ComboBox,               2, 1);
  layout_cas5->addWidget(new QLabel("  Save (full path)"), 2, 2, Qt::AlignRight);
  layout_cas5->addWidget(_outFile_5_LineEdit,              2, 3, 1, 30);
  layout_cas5->addWidget(new QLabel("Center of Mass"),     3, 0);
  layout_cas5->addWidget(_CoM_5_CheckBox,                  3, 1);
  layout_cas5->addWidget(new QLabel("Produce broadcast ephemeris corrections, upload to caster, reference system, local storage."), 4, 0, 1, 50);

  tab_cas5->setLayout(layout_cas5);

  connect(_refSys_5_ComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(customTrafo(const QString)));

  connect(_outHost_5_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Corrections VI Tab
  // ----------------------------
  QWidget* tab_cas6 = new QWidget();
  tabs->addTab(tab_cas6, "BC VI");

  QGridLayout* layout_cas6 = new QGridLayout;

  layout_cas6->setColumnMinimumWidth(0, 9*ww);
  _outPort_6_LineEdit->setMaximumWidth(9*ww);
  _password_6_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_6_LineEdit->setMaximumWidth(12*ww);
  _refSys_6_ComboBox->setMaximumWidth(12*ww);

  layout_cas6->addWidget(new QLabel("Host"),               0, 0);
  layout_cas6->addWidget(_outHost_6_LineEdit,              0, 1, 1, 3);
  layout_cas6->addWidget(new QLabel("  Port"),             0, 4, Qt::AlignRight);
  layout_cas6->addWidget(_outPort_6_LineEdit,              0, 5, 1, 10);
  layout_cas6->addWidget(new QLabel("Mountpoint"),         1, 0);
  layout_cas6->addWidget(_mountpoint_6_LineEdit,           1, 1);
  layout_cas6->addWidget(new QLabel("Password"),           1, 2, Qt::AlignRight);
  layout_cas6->addWidget(_password_6_LineEdit,             1, 3);
  layout_cas6->addWidget(new QLabel(" "),                  1, 4);
  layout_cas6->addWidget(new QLabel(" "),                  1, 5);
  layout_cas6->addWidget(new QLabel("System"),             2, 0);
  layout_cas6->addWidget(_refSys_6_ComboBox,               2, 1);
  layout_cas6->addWidget(new QLabel("  Save (full path)"), 2, 2, Qt::AlignRight);
  layout_cas6->addWidget(_outFile_6_LineEdit,              2, 3, 1, 30);
  layout_cas6->addWidget(new QLabel("Center of Mass"),     3, 0);
  layout_cas6->addWidget(_CoM_6_CheckBox,                  3, 1);
  layout_cas6->addWidget(new QLabel("Produce broadcast ephemeris corrections, upload to caster, reference system, local storage."), 4, 0, 1, 50);

  tab_cas6->setLayout(layout_cas6);

  connect(_refSys_6_ComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(customTrafo(const QString)));

  connect(_outHost_6_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Corrections VII Tab
  // -----------------------------
  QWidget* tab_cas7 = new QWidget();
  tabs->addTab(tab_cas7, "BC VII");

  QGridLayout* layout_cas7 = new QGridLayout;

  layout_cas7->setColumnMinimumWidth(0, 9*ww);
  _outPort_7_LineEdit->setMaximumWidth(9*ww);
  _password_7_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_7_LineEdit->setMaximumWidth(12*ww);
  _refSys_7_ComboBox->setMaximumWidth(12*ww);

  layout_cas7->addWidget(new QLabel("Host"),               0, 0);
  layout_cas7->addWidget(_outHost_7_LineEdit,              0, 1, 1, 3);
  layout_cas7->addWidget(new QLabel("  Port"),             0, 4, Qt::AlignRight);
  layout_cas7->addWidget(_outPort_7_LineEdit,              0, 5, 1, 10);
  layout_cas7->addWidget(new QLabel("Mountpoint"),         1, 0);
  layout_cas7->addWidget(_mountpoint_7_LineEdit,           1, 1);
  layout_cas7->addWidget(new QLabel("Password"),           1, 2, Qt::AlignRight);
  layout_cas7->addWidget(_password_7_LineEdit,             1, 3);
  layout_cas7->addWidget(new QLabel(" "),                  1, 4);
  layout_cas7->addWidget(new QLabel(" "),                  1, 5);
  layout_cas7->addWidget(new QLabel("System"),             2, 0);
  layout_cas7->addWidget(_refSys_7_ComboBox,               2, 1);
  layout_cas7->addWidget(new QLabel("  Save (full path)"), 2, 2, Qt::AlignRight);
  layout_cas7->addWidget(_outFile_7_LineEdit,              2, 3, 1, 30);
  layout_cas7->addWidget(new QLabel("Center of Mass"),     3, 0);
  layout_cas7->addWidget(_CoM_7_CheckBox,                  3, 1);
  layout_cas7->addWidget(new QLabel("Produce broadcast ephemeris corrections, upload to caster, reference system, local storage."), 4, 0, 1, 50);

  tab_cas7->setLayout(layout_cas7);

  connect(_refSys_7_ComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(customTrafo(const QString)));

  connect(_outHost_7_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Corrections VIII Tab
  // ------------------------------
  QWidget* tab_cas8 = new QWidget();
  tabs->addTab(tab_cas8, "BC VIII");

  QGridLayout* layout_cas8 = new QGridLayout;

  layout_cas8->setColumnMinimumWidth(0, 9*ww);
  _outPort_8_LineEdit->setMaximumWidth(9*ww);
  _password_8_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_8_LineEdit->setMaximumWidth(12*ww);
  _refSys_8_ComboBox->setMaximumWidth(12*ww);

  layout_cas8->addWidget(new QLabel("Host"),               0, 0);
  layout_cas8->addWidget(_outHost_8_LineEdit,              0, 1, 1, 3);
  layout_cas8->addWidget(new QLabel("  Port"),             0, 4, Qt::AlignRight);
  layout_cas8->addWidget(_outPort_8_LineEdit,              0, 5, 1, 10);
  layout_cas8->addWidget(new QLabel("Mountpoint"),         1, 0);
  layout_cas8->addWidget(_mountpoint_8_LineEdit,           1, 1);
  layout_cas8->addWidget(new QLabel("Password"),           1, 2, Qt::AlignRight);
  layout_cas8->addWidget(_password_8_LineEdit,             1, 3);
  layout_cas8->addWidget(new QLabel(" "),                  1, 4);
  layout_cas8->addWidget(new QLabel(" "),                  1, 5);
  layout_cas8->addWidget(new QLabel("System"),             2, 0);
  layout_cas8->addWidget(_refSys_8_ComboBox,               2, 1);
  layout_cas8->addWidget(new QLabel("  Save (full path)"), 2, 2, Qt::AlignRight);
  layout_cas8->addWidget(_outFile_8_LineEdit,              2, 3, 1, 30);
  layout_cas8->addWidget(new QLabel("Center of Mass"),     3, 0);
  layout_cas8->addWidget(_CoM_8_CheckBox,                  3, 1);
  layout_cas8->addWidget(new QLabel("Produce broadcast ephemeris corrections, upload to caster, reference system, local storage."), 4, 0, 1, 50);

  tab_cas8->setLayout(layout_cas8);

  connect(_refSys_8_ComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(customTrafo(const QString)));

  connect(_outHost_8_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Corrections IX Tab
  // ----------------------------
  QWidget* tab_cas9 = new QWidget();
  tabs->addTab(tab_cas9, "BC IX");

  QGridLayout* layout_cas9 = new QGridLayout;

  layout_cas9->setColumnMinimumWidth(0, 9*ww);
  _outPort_9_LineEdit->setMaximumWidth(9*ww);
  _password_9_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_9_LineEdit->setMaximumWidth(12*ww);
  _refSys_9_ComboBox->setMaximumWidth(12*ww);

  layout_cas9->addWidget(new QLabel("Host"),               0, 0);
  layout_cas9->addWidget(_outHost_9_LineEdit,              0, 1, 1, 3);
  layout_cas9->addWidget(new QLabel("  Port"),             0, 4, Qt::AlignRight);
  layout_cas9->addWidget(_outPort_9_LineEdit,              0, 5, 1, 10);
  layout_cas9->addWidget(new QLabel("Mountpoint"),         1, 0);
  layout_cas9->addWidget(_mountpoint_9_LineEdit,           1, 1);
  layout_cas9->addWidget(new QLabel("Password"),           1, 2, Qt::AlignRight);
  layout_cas9->addWidget(_password_9_LineEdit,             1, 3);
  layout_cas9->addWidget(new QLabel(" "),                  1, 4);
  layout_cas9->addWidget(new QLabel(" "),                  1, 5);
  layout_cas9->addWidget(new QLabel("System"),             2, 0);
  layout_cas9->addWidget(_refSys_9_ComboBox,               2, 1);
  layout_cas9->addWidget(new QLabel("  Save (full path)"), 2, 2, Qt::AlignRight);
  layout_cas9->addWidget(_outFile_9_LineEdit,              2, 3, 1, 30);
  layout_cas9->addWidget(new QLabel("Center of Mass"),     3, 0);
  layout_cas9->addWidget(_CoM_9_CheckBox,                  3, 1);
  layout_cas9->addWidget(new QLabel("Produce broadcast ephemeris corrections, upload to caster, reference system, local storage."), 4, 0, 1, 50);

  tab_cas9->setLayout(layout_cas9);

  connect(_refSys_9_ComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(customTrafo(const QString)));

  connect(_outHost_9_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Corrections X Tab
  // ---------------------------
  QWidget* tab_cas10 = new QWidget();
  tabs->addTab(tab_cas10, "BC X");

  QGridLayout* layout_cas10 = new QGridLayout;

  layout_cas10->setColumnMinimumWidth(0, 9*ww);
  _outPort_10_LineEdit->setMaximumWidth(9*ww);
  _password_10_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_10_LineEdit->setMaximumWidth(12*ww);
  _refSys_10_ComboBox->setMaximumWidth(12*ww);

  layout_cas10->addWidget(new QLabel("Host"),               0, 0);
  layout_cas10->addWidget(_outHost_10_LineEdit,             0, 1, 1, 3);
  layout_cas10->addWidget(new QLabel("  Port"),             0, 4, Qt::AlignRight);
  layout_cas10->addWidget(_outPort_10_LineEdit,             0, 5, 1, 10);
  layout_cas10->addWidget(new QLabel("Mountpoint"),         1, 0);
  layout_cas10->addWidget(_mountpoint_10_LineEdit,          1, 1);
  layout_cas10->addWidget(new QLabel("Password"),           1, 2, Qt::AlignRight);
  layout_cas10->addWidget(_password_10_LineEdit,            1, 3);
  layout_cas10->addWidget(new QLabel(" "),                  1, 4);
  layout_cas10->addWidget(new QLabel(" "),                  1, 5);
  layout_cas10->addWidget(new QLabel("System"),             2, 0);
  layout_cas10->addWidget(_refSys_10_ComboBox,              2, 1);
  layout_cas10->addWidget(new QLabel("  Save (full path)"), 2, 2, Qt::AlignRight);
  layout_cas10->addWidget(_outFile_10_LineEdit,             2, 3, 1, 30);
  layout_cas10->addWidget(new QLabel("Center of Mass"),     3, 0);
  layout_cas10->addWidget(_CoM_10_CheckBox,                 3, 1);
  layout_cas10->addWidget(new QLabel("Produce broadcast ephemeris corrections, upload to caster, reference system, local storage."), 4, 0, 1, 50);

  tab_cas10->setLayout(layout_cas10);

  connect(_refSys_10_ComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(customTrafo(const QString)));

  connect(_outHost_10_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // Broadcast Ephemerides
  // ---------------------
  QWidget* tab_casEph = new QWidget();
  tabs->addTab(tab_casEph, "Broadcast Ephemeris");

  QGridLayout* layout_casEph = new QGridLayout;

  layout_casEph->setColumnMinimumWidth(0, 9*ww);
  _outPort_Eph_LineEdit->setMaximumWidth(9*ww);
  _password_Eph_LineEdit->setMaximumWidth(9*ww);
  _mountpoint_Eph_LineEdit->setMaximumWidth(12*ww);

  layout_casEph->addWidget(new QLabel("Host"),                  0, 0);
  layout_casEph->addWidget(_outHost_Eph_LineEdit,               0, 1, 1, 3);
  layout_casEph->addWidget(new QLabel("  Port"),                0, 4, Qt::AlignRight);
  layout_casEph->addWidget(_outPort_Eph_LineEdit,               0, 5, 1, 10);
  layout_casEph->addWidget(new QLabel("Mountpoint           "), 1, 0);
  layout_casEph->addWidget(_mountpoint_Eph_LineEdit,            1, 1);
  layout_casEph->addWidget(new QLabel("          Password"),    1, 2, Qt::AlignRight);
  layout_casEph->addWidget(_password_Eph_LineEdit,              1, 3);
  layout_casEph->addWidget(new QLabel("Sampling"),              2, 0);
  layout_casEph->addWidget(_samplEphSpinBox,                    2, 1);
  layout_casEph->addWidget(new QLabel("Upload concatenated RTCMv3 Broadcast Ephemeris to caster."), 3, 0, 1, 50);

  tab_casEph->setLayout(layout_casEph);

  connect(_outHost_Eph_LineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // RINEX Clocks Tab
  // ----------------
  QWidget* tab_rin = new QWidget();
  tabs->addTab(tab_rin, "RINEX Clocks ");

  QGridLayout* layout_rin = new QGridLayout;

  layout_rin->setColumnMinimumWidth(0, 9*ww);
  _rnxIntrComboBox->setMaximumWidth(9*ww);

  layout_rin->addWidget(new QLabel("Directory"),                    0, 0);
  layout_rin->addWidget(_rnxPathLineEdit,                           0, 1, 1, 27);
  layout_rin->addWidget(new QLabel("Interval"),                     1, 0);
  layout_rin->addWidget(_rnxIntrComboBox,                           1, 1);
  layout_rin->addWidget(new QLabel("Sampling"),                     2, 0);
  layout_rin->addWidget(_rnxSamplSpinBox,                           2, 1);
  layout_rin->addWidget(new QLabel("Save clock corrections in Clock RINEX file format."),  3, 0, 1, 50, Qt::AlignLeft);
  layout_rin->addWidget(new QLabel("    "),                         3, 0);

  tab_rin->setLayout(layout_rin);

  connect(_rnxPathLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  // SP3 Orbits Tab
  // --------------
  QWidget* tab_sp3 = new QWidget();
  tabs->addTab(tab_sp3, "SP3 Orbits");

  QGridLayout* layout_sp3 = new QGridLayout;

  layout_sp3->setColumnMinimumWidth(0, 9*ww);
  _sp3IntrComboBox->setMaximumWidth(9*ww);

  layout_sp3->addWidget(new QLabel("Directory"),                 0, 0);
  layout_sp3->addWidget(_sp3PathLineEdit,                        0, 1, 1, 27);
  layout_sp3->addWidget(new QLabel("Interval"),                  1, 0);
  layout_sp3->addWidget(_sp3IntrComboBox,                        1, 1);
  layout_sp3->addWidget(new QLabel("Sampling"),                  2, 0);
  layout_sp3->addWidget(_sp3SamplSpinBox,                        2, 1);
  layout_sp3->addWidget(new QLabel("Save orbit corrections in SP3 file format."), 3, 0, 1, 50, Qt::AlignLeft);
  layout_sp3->addWidget(new QLabel("    "),                      3, 0);

  tab_sp3->setLayout(layout_sp3);

  connect(_sp3PathLineEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotBnsTextChanged()));

  tabs->setCurrentIndex(settings.value("startTab").toInt());

  // Log
  // ---
  _log = new QTextBrowser();
  _log->setReadOnly(true);
  _log->setWhatsThis(tr("Records of BNS's activities are shown in the Log section."));

  // Status
  // ------
  _status = new QGroupBox(tr("Status"));
  QGridLayout* layout_status = new QGridLayout;

  _statusLbl[0] = new QLabel("0 byte(s)"); _statusCnt[0] = 0;
  _statusLbl[1] = new QLabel("0 byte(s)"); _statusCnt[1] = 0;
  _statusLbl[2] = new QLabel("0 byte(s)"); _statusCnt[2] = 0;
  _statusLbl[3] = new QLabel("0 byte(s)"); _statusCnt[3] = 0;
  _statusLbl[4] = new QLabel("0 byte(s)"); _statusCnt[4] = 0;
  _statusLbl[5] = new QLabel("0 byte(s)"); _statusCnt[5] = 0;
  _statusLbl[6] = new QLabel("0 byte(s)"); _statusCnt[6] = 0;
  _statusLbl[7] = new QLabel("0 byte(s)"); _statusCnt[7] = 0;
  _statusLbl[8] = new QLabel("0 byte(s)"); _statusCnt[8] = 0;
  _statusLbl[9] = new QLabel("0 byte(s)"); _statusCnt[9] = 0;
  _statusLbl[10] = new QLabel("0 byte(s)"); _statusCnt[10] = 0;
  _statusLbl[11] = new QLabel("0 byte(s)"); _statusCnt[11] = 0;
  _statusLbl[12] = new QLabel("0 byte(s)"); _statusCnt[12] = 0;

  _statusLbl[13]  = new QLabel("RINEX Ephemeris:");  
  _statusLbl[14] = new QLabel("Clocks & Orbits:");
  _statusLbl[15] = new QLabel("Broadcast Corrections I:");  
  _statusLbl[16] = new QLabel("Broadcast Corrections II:");  
  _statusLbl[17] = new QLabel("Broadcast Corrections III:");  
  _statusLbl[18] = new QLabel("Broadcast Corrections IV:");  
  _statusLbl[19] = new QLabel("Broadcast Corrections V:");  
  _statusLbl[20] = new QLabel("Broadcast Corrections VI:");  
  _statusLbl[21] = new QLabel("Broadcast Corrections VII:");  
  _statusLbl[22] = new QLabel("Broadcast Corrections VIII:");  
  _statusLbl[23] = new QLabel("Broadcast Corrections IX:");  
  _statusLbl[24] = new QLabel("Broadcast Corrections X:");  
  _statusLbl[25] = new QLabel("Broadcast Ephemeris:");  

  _statusLbl[0]->setWhatsThis(tr("Status of incoming broadcast ephemeris."));
  _statusLbl[1]->setWhatsThis(tr("Status of incoming stream of clocks and orbits."));
  _statusLbl[2]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster I."));
  _statusLbl[3]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster II."));
  _statusLbl[4]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster III."));
  _statusLbl[5]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster IV."));
  _statusLbl[6]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster V."));
  _statusLbl[7]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster VI."));
  _statusLbl[8]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster VII."));
  _statusLbl[9]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster VIII."));
  _statusLbl[10]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster IX."));
  _statusLbl[11]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster X."));
  _statusLbl[12]->setWhatsThis(tr("Status of outgoing Broadcast Ephemeris to NTRIP broadcaster"));

  _statusLbl[13]->setWhatsThis(tr("Status of incoming broadcast ephemeris."));
  _statusLbl[14]->setWhatsThis(tr("Status of incoming stream of clocks and orbits I."));
  _statusLbl[15]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster I."));
  _statusLbl[16]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster II."));
  _statusLbl[17]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster III."));
  _statusLbl[18]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster IV."));
  _statusLbl[19]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster V."));
  _statusLbl[20]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster VI."));
  _statusLbl[21]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster VII."));
  _statusLbl[22]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster VIII."));
  _statusLbl[23]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster IX."));
  _statusLbl[24]->setWhatsThis(tr("Status of outgoing corrections stream to NTRIP broadcaster X."));
  _statusLbl[25]->setWhatsThis(tr("Status of outgoing Broadcast Ephemeris to NTRIP broadcaster"));

  layout_status->addWidget(_statusLbl[13], 0, 0);
  layout_status->addWidget(_statusLbl[0],  0, 1);
  layout_status->addWidget(_statusLbl[14], 1, 0);
  layout_status->addWidget(_statusLbl[1],  1, 1);
  layout_status->addWidget(_statusLbl[25], 2, 0); 
  layout_status->addWidget(_statusLbl[12], 2, 1); 

  layout_status->addWidget(_statusLbl[15], 0, 2); 
  layout_status->addWidget(_statusLbl[2],  0, 3); 
  layout_status->addWidget(_statusLbl[16], 1, 2); 
  layout_status->addWidget(_statusLbl[3],  1, 3); 
  layout_status->addWidget(_statusLbl[17], 2, 2); 
  layout_status->addWidget(_statusLbl[4],  2, 3); 
  layout_status->addWidget(_statusLbl[18], 3, 2); 
  layout_status->addWidget(_statusLbl[5],  3, 3); 
  layout_status->addWidget(_statusLbl[19], 4, 2); 
  layout_status->addWidget(_statusLbl[6],  4, 3); 
  layout_status->addWidget(_statusLbl[20], 5, 2); 
  layout_status->addWidget(_statusLbl[7],  5, 3); 
  layout_status->addWidget(_statusLbl[21], 6, 2); 
  layout_status->addWidget(_statusLbl[8],  6, 3); 
  layout_status->addWidget(_statusLbl[22], 7, 2); 
  layout_status->addWidget(_statusLbl[9],  7, 3); 
  layout_status->addWidget(_statusLbl[23], 8, 2); 
  layout_status->addWidget(_statusLbl[10], 8, 3); 
  layout_status->addWidget(_statusLbl[24], 9, 2); 
  layout_status->addWidget(_statusLbl[11], 9, 3); 

  _status->setLayout(layout_status);

  // Main Layout
  // -----------
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabs);
  mainLayout->addWidget(_log);
  mainLayout->addWidget(_status);

  _canvas->setLayout(mainLayout);

  // Enable/Disable all Widgets
  // --------------------------
  slotBnsTextChanged();

  // Auto start
  // ----------
  if ( Qt::CheckState(settings.value("autoStart").toInt()) == Qt::Checked) {
    slotStart();
  }

}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsWindow::~bnsWindow() {
}

// Close Application gracefully
////////////////////////////////////////////////////////////////////////////
void bnsWindow::closeEvent(QCloseEvent* event) {

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

  QMainWindow::closeEvent(event);
}

// About Message
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotAbout() {
 new bnsAboutDlg(0);
}

// Flowchart
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotFlowchart() {
 new bnsFlowchartDlg(0);
}

// Help Window
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotHelp() {
  QUrl url; 
  url.setPath(":bnshelp.html");
  new bnsHlpDlg(0, url);
}

// Select Fonts
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotFontSel() {
  bool ok;
  QFont newFont = QFontDialog::getFont(&ok, this->font(), this); 
  if (ok) {
    bnsSettings settings;
    settings.setValue("font", newFont.toString());
    QApplication::setFont(newFont);
    int ww = QFontMetrics(newFont).width('w');
    setMinimumSize(77*ww, 65*ww);
  }
}

// Whats This Help
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotWhatsThis() {
QWhatsThis::enterWhatsThisMode();
}

// Create Menus
////////////////////////////////////////////////////////////////////////////
void bnsWindow::CreateMenu() {
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

// Tool (Command) Bar
////////////////////////////////////////////////////////////////////////////
void bnsWindow::AddToolbar() {
  QToolBar* toolBar = new QToolBar;
  addToolBar(Qt::BottomToolBarArea, toolBar); 
  toolBar->setMovable(false);
  toolBar->addAction(_actStart);
  toolBar->addAction(_actStop);
  toolBar->addWidget(new QLabel("                                   "));
  toolBar->addAction(_actWhatsThis);
}

// Save Options
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotSaveOptions() {
  bnsSettings settings;
  settings.setValue("proxyHost",   _proxyHostLineEdit->text());
  settings.setValue("proxyPort",   _proxyPortLineEdit->text());

  settings.setValue("logFile",     _logFileLineEdit->text());
  settings.setValue("fileAppend",  _fileAppendCheckBox->checkState());
  settings.setValue("autoStart",   _autoStartCheckBox->checkState());

  settings.setValue("ephHost",     _ephHostLineEdit->text());
  settings.setValue("ephPort",     _ephPortLineEdit->text());
  settings.setValue("ephEcho",     _ephEchoLineEdit->text());

  settings.setValue("clkPort",     _clkPortLineEdit->text());
  settings.setValue("inpEcho",     _inpEchoLineEdit->text());

  settings.setValue("outHost1",    _outHost_1_LineEdit->text());
  settings.setValue("outPort1",    _outPort_1_LineEdit->text());
  settings.setValue("mountpoint_1",_mountpoint_1_LineEdit->text());
  settings.setValue("password1",   _password_1_LineEdit->text());
  settings.setValue("refSys_1",    _refSys_1_ComboBox->currentText());
  settings.setValue("outFile_1",   _outFile_1_LineEdit->text());
  settings.setValue("CoM_1",       _CoM_1_CheckBox->checkState());

  settings.setValue("outHost2",    _outHost_2_LineEdit->text());
  settings.setValue("outPort2",    _outPort_2_LineEdit->text());
  settings.setValue("mountpoint_2",_mountpoint_2_LineEdit->text());
  settings.setValue("password2",   _password_2_LineEdit->text());
  settings.setValue("refSys_2",    _refSys_2_ComboBox->currentText());
  settings.setValue("outFile_2",   _outFile_2_LineEdit->text());
  settings.setValue("CoM_2",       _CoM_2_CheckBox->checkState());

  settings.setValue("outHost3",    _outHost_3_LineEdit->text());
  settings.setValue("outPort3",    _outPort_3_LineEdit->text());
  settings.setValue("mountpoint_3",_mountpoint_3_LineEdit->text());
  settings.setValue("password3",   _password_3_LineEdit->text());
  settings.setValue("refSys_3",    _refSys_3_ComboBox->currentText());
  settings.setValue("outFile_3",   _outFile_3_LineEdit->text());
  settings.setValue("CoM_3",       _CoM_3_CheckBox->checkState());

  settings.setValue("outHost4",    _outHost_4_LineEdit->text());
  settings.setValue("outPort4",    _outPort_4_LineEdit->text());
  settings.setValue("mountpoint_4",_mountpoint_4_LineEdit->text());
  settings.setValue("password4",   _password_4_LineEdit->text());
  settings.setValue("refSys_4",    _refSys_4_ComboBox->currentText());
  settings.setValue("outFile_4",   _outFile_4_LineEdit->text());
  settings.setValue("CoM_4",       _CoM_4_CheckBox->checkState());

  settings.setValue("outHost5",    _outHost_5_LineEdit->text());
  settings.setValue("outPort5",    _outPort_5_LineEdit->text());
  settings.setValue("mountpoint_5",_mountpoint_5_LineEdit->text());
  settings.setValue("password5",   _password_5_LineEdit->text());
  settings.setValue("refSys_5",    _refSys_5_ComboBox->currentText());
  settings.setValue("outFile_5",   _outFile_5_LineEdit->text());
  settings.setValue("CoM_5",       _CoM_5_CheckBox->checkState());

  settings.setValue("outHost6",    _outHost_6_LineEdit->text());
  settings.setValue("outPort6",    _outPort_6_LineEdit->text());
  settings.setValue("mountpoint_6",_mountpoint_6_LineEdit->text());
  settings.setValue("password6",   _password_6_LineEdit->text());
  settings.setValue("refSys_6",    _refSys_6_ComboBox->currentText());
  settings.setValue("outFile_6",   _outFile_6_LineEdit->text());
  settings.setValue("CoM_6",       _CoM_6_CheckBox->checkState());

  settings.setValue("outHost7",    _outHost_7_LineEdit->text());
  settings.setValue("outPort7",    _outPort_7_LineEdit->text());
  settings.setValue("mountpoint_7",_mountpoint_7_LineEdit->text());
  settings.setValue("password7",   _password_7_LineEdit->text());
  settings.setValue("refSys_7",    _refSys_7_ComboBox->currentText());
  settings.setValue("outFile_7",   _outFile_7_LineEdit->text());

  settings.setValue("outHost8",    _outHost_8_LineEdit->text());
  settings.setValue("outPort8",    _outPort_8_LineEdit->text());
  settings.setValue("mountpoint_8",_mountpoint_8_LineEdit->text());
  settings.setValue("password8",   _password_8_LineEdit->text());
  settings.setValue("refSys_8",    _refSys_8_ComboBox->currentText());
  settings.setValue("outFile_8",   _outFile_8_LineEdit->text());

  settings.setValue("outHost9",    _outHost_9_LineEdit->text());
  settings.setValue("outPort9",    _outPort_9_LineEdit->text());
  settings.setValue("mountpoint_9",_mountpoint_9_LineEdit->text());
  settings.setValue("password9",   _password_9_LineEdit->text());
  settings.setValue("refSys_9",    _refSys_9_ComboBox->currentText());
  settings.setValue("outFile_9",   _outFile_9_LineEdit->text());

  settings.setValue("outHost10",    _outHost_10_LineEdit->text());
  settings.setValue("outPort10",    _outPort_10_LineEdit->text());
  settings.setValue("mountpoint_10",_mountpoint_10_LineEdit->text());
  settings.setValue("password10",   _password_10_LineEdit->text());
  settings.setValue("refSys_10",    _refSys_10_ComboBox->currentText());
  settings.setValue("outFile_10",   _outFile_10_LineEdit->text());

  settings.setValue("outHostEph",    _outHost_Eph_LineEdit->text());
  settings.setValue("outPortEph",    _outPort_Eph_LineEdit->text());
  settings.setValue("mountpoint_Eph",_mountpoint_Eph_LineEdit->text());
  settings.setValue("passwordEph",   _password_Eph_LineEdit->text());
  settings.setValue("samplEph",      _samplEphSpinBox->value());

  settings.setValue("rnxPath",     _rnxPathLineEdit->text());
  settings.setValue("rnxIntr",     _rnxIntrComboBox->currentText());
  settings.setValue("rnxSampl",    _rnxSamplSpinBox->value());

  settings.setValue("sp3Path",     _sp3PathLineEdit->text());
  settings.setValue("sp3Intr",     _sp3IntrComboBox->currentText());
  settings.setValue("sp3Sampl",    _sp3SamplSpinBox->value());

  settings.setValue("startTab",    tabs->currentIndex());
}

// Display Program Messages 
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotMessage(const QByteArray msg) {

  const int maxBufferSize = 10000;
 
  QString txt = _log->toPlainText() + "\n" + 
     QDateTime::currentDateTime().toUTC().toString("yy-MM-dd hh:mm:ss ") + msg;
  _log->clear();
  _log->append(txt.right(maxBufferSize));
}  

// Delete bns
////////////////////////////////////////////////////////////////////////////
void bnsWindow::deleteBns() {
  _actStart->setEnabled(true);
  _actStop->setEnabled(false);
  _bns->terminate();
  _bns->wait(100);
  delete _bns; 
  _bns = 0;
}  

// Error in bns
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotError(const QByteArray msg) {
  slotMessage(msg);
  deleteBns();
}  

// Stop 
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotStop() {
  int iRet = QMessageBox::question(this, "Stop", "Do you want to stop?", 
                                   QMessageBox::Yes, QMessageBox::No,
                                   QMessageBox::NoButton);
  if (iRet == QMessageBox::Yes) {
    deleteBns();
  }
}

// Start
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotStart() {
  slotSaveOptions();

  _actStart->setEnabled(false);
  _actStop->setEnabled(true);

  _bns = new t_bns(0);

  connect(_bns, SIGNAL(newMessage(QByteArray)),
          this, SLOT(slotMessage(const QByteArray)));

  connect(_bns, SIGNAL(error(QByteArray)),
          this, SLOT(slotError(const QByteArray)));

  connect(_bns, SIGNAL(newEphBytes(int)), this, SLOT(slotEphBytes(int)));
  connect(_bns, SIGNAL(newClkBytes(int)), this, SLOT(slotClkBytes(int)));
  connect(_bns, SIGNAL(newOutBytes1(int)), this, SLOT(slotOutBytes1(int)));
  connect(_bns, SIGNAL(newOutBytes2(int)), this, SLOT(slotOutBytes2(int)));
  connect(_bns, SIGNAL(newOutBytes3(int)), this, SLOT(slotOutBytes3(int)));
  connect(_bns, SIGNAL(newOutBytes4(int)), this, SLOT(slotOutBytes4(int)));
  connect(_bns, SIGNAL(newOutBytes5(int)), this, SLOT(slotOutBytes5(int)));
  connect(_bns, SIGNAL(newOutBytes6(int)), this, SLOT(slotOutBytes6(int)));
  connect(_bns, SIGNAL(newOutEphBytes(int)), this, SLOT(slotOutEphBytes(int)));

  _bns->start();
}

// Input and output bytes statistics
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotEphBytes(int nBytes) {
  updateStatus(0, nBytes);
}
void bnsWindow::slotClkBytes(int nBytes) {
  updateStatus(1, nBytes);
}
void bnsWindow::slotOutBytes1(int nBytes) {
  updateStatus(2, nBytes);
}
void bnsWindow::slotOutBytes2(int nBytes) {
  updateStatus(3, nBytes);
}
void bnsWindow::slotOutBytes3(int nBytes) {
  updateStatus(4, nBytes);
}
void bnsWindow::slotOutBytes4(int nBytes) {
  updateStatus(5, nBytes);
}
void bnsWindow::slotOutBytes5(int nBytes) {
  updateStatus(6, nBytes);
}
void bnsWindow::slotOutBytes6(int nBytes) {
  updateStatus(7, nBytes);
}
void bnsWindow::slotOutEphBytes(int nBytes) {
  updateStatus(8, nBytes);
}

void bnsWindow::updateStatus(int ii, int nBytes) {
  QMutexLocker locker(&_mutex);

  _statusCnt[ii] += nBytes;

  if      (_statusCnt[ii] < 1e3) {
    _statusLbl[ii]->setText(QString("%1 byte(s)").arg((int)_statusCnt[ii]));
  }
  else if (_statusCnt[ii] < 1e6) {
    _statusLbl[ii]->setText(QString("%1 kb").arg(_statusCnt[ii]/1.e3, 5));
  }
  else {
    _statusLbl[ii]->setText(QString("%1 Mb").arg(_statusCnt[ii]/1.e6, 5));
  }
}

// Enable/Disable Widgets according to user input
////////////////////////////////////////////////////////////////////////////
void bnsWindow::slotBnsTextChanged(){

  QPalette palette_white(QColor(255, 255, 255));
  QPalette palette_gray(QColor(230, 230, 230));

  // Enable/disable Proxy Options
  // ----------------------------
  if (sender() == 0 || sender() == _proxyHostLineEdit) {
    if (!_proxyHostLineEdit->text().isEmpty()) {
      _proxyPortLineEdit->setStyleSheet("background-color: white");
      _proxyPortLineEdit->setEnabled(true);
    } 
    else {
      _proxyPortLineEdit->setStyleSheet("background-color: lightGray");
      _proxyPortLineEdit->setEnabled(false);
    }
  }

  // Enable/disable RINEX Ephemeris Options
  // --------------------------------------
  if (sender() == 0 || sender() == _ephHostLineEdit) {
    if (!_ephHostLineEdit->text().isEmpty()) {
      _ephPortLineEdit->setStyleSheet("background-color: white");
      _ephEchoLineEdit->setStyleSheet("background-color: white");
      _ephPortLineEdit->setEnabled(true);
      _ephEchoLineEdit->setEnabled(true);
    } 
    else {
      _ephPortLineEdit->setStyleSheet("background-color: lightGray");
      _ephEchoLineEdit->setStyleSheet("background-color: lightGray");
      _ephPortLineEdit->setEnabled(false);
      _ephEchoLineEdit->setEnabled(false);
    }
  }

  // Enable/disable Clocks & Orbits Options
  // --------------------------------------
  if (sender() == 0 || sender() == _clkPortLineEdit) {
    if (!_clkPortLineEdit->text().isEmpty()) {
      _inpEchoLineEdit->setStyleSheet("background-color: white");
      _inpEchoLineEdit->setEnabled(true);
    } 
    else {
      _inpEchoLineEdit->setStyleSheet("background-color: lightGray");
      _inpEchoLineEdit->setEnabled(false);
    }
  }

  // Enable/disable  Broadcast Corrections I Options
  // -----------------------------------------------
  if (sender() == 0 || sender() == _outHost_1_LineEdit) {
    if (!_outHost_1_LineEdit->text().isEmpty()) {
      _outPort_1_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_1_LineEdit->setStyleSheet("background-color: white");
      _password_1_LineEdit->setStyleSheet("background-color: white");
      _outFile_1_LineEdit->setStyleSheet("background-color: white");
      _refSys_1_ComboBox->setStyleSheet("background-color: white");
      _CoM_1_CheckBox->setPalette(palette_white);
      _outPort_1_LineEdit->setEnabled(true);
      _mountpoint_1_LineEdit->setEnabled(true);
      _password_1_LineEdit->setEnabled(true);
      _outFile_1_LineEdit->setEnabled(true);
      _refSys_1_ComboBox->setEnabled(true);
      _CoM_1_CheckBox->setEnabled(true);
    } 
    else {
      _outPort_1_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_1_LineEdit->setStyleSheet("background-color: lightGray");
      _password_1_LineEdit->setStyleSheet("background-color: lightGray");
      _outFile_1_LineEdit->setStyleSheet("background-color: lightGray");
      _refSys_1_ComboBox->setStyleSheet("background-color: lightGray");
      _CoM_1_CheckBox->setPalette(palette_gray);
      _outPort_1_LineEdit->setEnabled(false);
      _mountpoint_1_LineEdit->setEnabled(false);
      _password_1_LineEdit->setEnabled(false);
      _outFile_1_LineEdit->setEnabled(false);
      _refSys_1_ComboBox->setEnabled(false);
      _CoM_1_CheckBox->setEnabled(false);
    }
  }

  // Enable/disable Broadcast Corrections II Options
  // -----------------------------------------------
  if (sender() == 0 || sender() == _outHost_2_LineEdit) {
    if (!_outHost_2_LineEdit->text().isEmpty()) {
      _outPort_2_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_2_LineEdit->setStyleSheet("background-color: white");
      _password_2_LineEdit->setStyleSheet("background-color: white");
      _outFile_2_LineEdit->setStyleSheet("background-color: white");
      _refSys_2_ComboBox->setStyleSheet("background-color: white");
      _CoM_2_CheckBox->setPalette(palette_white);
      _outPort_2_LineEdit->setEnabled(true);
      _mountpoint_2_LineEdit->setEnabled(true);
      _password_2_LineEdit->setEnabled(true);
      _outFile_2_LineEdit->setEnabled(true);
      _refSys_2_ComboBox->setEnabled(true);
      _CoM_2_CheckBox->setEnabled(true);
    } 
    else {
      _outPort_2_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_2_LineEdit->setStyleSheet("background-color: lightGray");
      _password_2_LineEdit->setStyleSheet("background-color: lightGray");
      _outFile_2_LineEdit->setStyleSheet("background-color: lightGray");
      _refSys_2_ComboBox->setStyleSheet("background-color: lightGray");
      _CoM_2_CheckBox->setPalette(palette_gray);
      _outPort_2_LineEdit->setEnabled(false);
      _mountpoint_2_LineEdit->setEnabled(false);
      _password_2_LineEdit->setEnabled(false);
      _outFile_2_LineEdit->setEnabled(false);
      _refSys_2_ComboBox->setEnabled(false);
      _CoM_2_CheckBox->setEnabled(false);
    }
  }

  // Enable/disable Broadcast Corrections III Options
  // -----------------------------------------------
  if (sender() == 0 || sender() == _outHost_3_LineEdit) {
    if (!_outHost_3_LineEdit->text().isEmpty()) {
      _outPort_3_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_3_LineEdit->setStyleSheet("background-color: white");
      _password_3_LineEdit->setStyleSheet("background-color: white");
      _outFile_3_LineEdit->setStyleSheet("background-color: white");
      _refSys_3_ComboBox->setStyleSheet("background-color: white");
      _CoM_3_CheckBox->setPalette(palette_white);
      _outPort_3_LineEdit->setEnabled(true);
      _mountpoint_3_LineEdit->setEnabled(true);
      _password_3_LineEdit->setEnabled(true);
      _outFile_3_LineEdit->setEnabled(true);
      _refSys_3_ComboBox->setEnabled(true);
      _CoM_3_CheckBox->setEnabled(true);
    } 
    else {
      _outPort_3_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_3_LineEdit->setStyleSheet("background-color: lightGray");
      _password_3_LineEdit->setStyleSheet("background-color: lightGray");
      _outFile_3_LineEdit->setStyleSheet("background-color: lightGray");
      _refSys_3_ComboBox->setStyleSheet("background-color: lightGray");
      _CoM_3_CheckBox->setPalette(palette_gray);
      _outPort_3_LineEdit->setEnabled(false);
      _mountpoint_3_LineEdit->setEnabled(false);
      _password_3_LineEdit->setEnabled(false);
      _outFile_3_LineEdit->setEnabled(false);
      _refSys_3_ComboBox->setEnabled(false);
      _CoM_3_CheckBox->setEnabled(false);
    }
  }

  // Enable/disable Broadcast Corrections IV Options
  // -----------------------------------------------
  if (sender() == 0 || sender() == _outHost_4_LineEdit) {
    if (!_outHost_4_LineEdit->text().isEmpty()) {
      _outPort_4_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_4_LineEdit->setStyleSheet("background-color: white");
      _password_4_LineEdit->setStyleSheet("background-color: white");
      _outFile_4_LineEdit->setStyleSheet("background-color: white");
      _refSys_4_ComboBox->setStyleSheet("background-color: white");
      _CoM_4_CheckBox->setPalette(palette_white);
      _outPort_4_LineEdit->setEnabled(true);
      _mountpoint_4_LineEdit->setEnabled(true);
      _password_4_LineEdit->setEnabled(true);
      _outFile_4_LineEdit->setEnabled(true);
      _refSys_4_ComboBox->setEnabled(true);
      _CoM_4_CheckBox->setEnabled(true);
    } 
    else {
      _outPort_4_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_4_LineEdit->setStyleSheet("background-color: lightGray");
      _password_4_LineEdit->setStyleSheet("background-color: lightGray");
      _outFile_4_LineEdit->setStyleSheet("background-color: lightGray");
      _refSys_4_ComboBox->setStyleSheet("background-color: lightGray");
      _CoM_4_CheckBox->setPalette(palette_gray);
      _outPort_4_LineEdit->setEnabled(false);
      _mountpoint_4_LineEdit->setEnabled(false);
      _password_4_LineEdit->setEnabled(false);
      _outFile_4_LineEdit->setEnabled(false);
      _refSys_4_ComboBox->setEnabled(false);
      _CoM_4_CheckBox->setEnabled(false);
    }
  }

  // Enable/disable Broadcast Corrections V Options
  // ----------------------------------------------
  if (sender() == 0 || sender() == _outHost_5_LineEdit) {
    if (!_outHost_5_LineEdit->text().isEmpty()) {
      _outPort_5_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_5_LineEdit->setStyleSheet("background-color: white");
      _password_5_LineEdit->setStyleSheet("background-color: white");
      _outFile_5_LineEdit->setStyleSheet("background-color: white");
      _refSys_5_ComboBox->setStyleSheet("background-color: white");
      _CoM_5_CheckBox->setPalette(palette_white);
      _outPort_5_LineEdit->setEnabled(true);
      _mountpoint_5_LineEdit->setEnabled(true);
      _password_5_LineEdit->setEnabled(true);
      _outFile_5_LineEdit->setEnabled(true);
      _refSys_5_ComboBox->setEnabled(true);
      _CoM_5_CheckBox->setEnabled(true);
    } 
    else {
      _outPort_5_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_5_LineEdit->setStyleSheet("background-color: lightGray");
      _password_5_LineEdit->setStyleSheet("background-color: lightGray");
      _outFile_5_LineEdit->setStyleSheet("background-color: lightGray");
      _refSys_5_ComboBox->setStyleSheet("background-color: lightGray");
      _CoM_5_CheckBox->setPalette(palette_gray);
      _outPort_5_LineEdit->setEnabled(false);
      _mountpoint_5_LineEdit->setEnabled(false);
      _password_5_LineEdit->setEnabled(false);
      _outFile_5_LineEdit->setEnabled(false);
      _refSys_5_ComboBox->setEnabled(false);
      _CoM_5_CheckBox->setEnabled(false);
    }
  }

  // Enable/disable Broadcast Corrections VI Options
  // -----------------------------------------------
  if (sender() == 0 || sender() == _outHost_6_LineEdit) {
    if (!_outHost_6_LineEdit->text().isEmpty()) {
      _outPort_6_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_6_LineEdit->setStyleSheet("background-color: white");
      _password_6_LineEdit->setStyleSheet("background-color: white");
      _outFile_6_LineEdit->setStyleSheet("background-color: white");
      _refSys_6_ComboBox->setStyleSheet("background-color: white");
      _CoM_6_CheckBox->setPalette(palette_white);
      _outPort_6_LineEdit->setEnabled(true);
      _mountpoint_6_LineEdit->setEnabled(true);
      _password_6_LineEdit->setEnabled(true);
      _outFile_6_LineEdit->setEnabled(true);
      _refSys_6_ComboBox->setEnabled(true);
      _CoM_6_CheckBox->setEnabled(true);
    } 
    else {
      _outPort_6_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_6_LineEdit->setStyleSheet("background-color: lightGray");
      _password_6_LineEdit->setStyleSheet("background-color: lightGray");
      _outFile_6_LineEdit->setStyleSheet("background-color: lightGray");
      _refSys_6_ComboBox->setStyleSheet("background-color: lightGray");
      _CoM_6_CheckBox->setPalette(palette_gray);
      _outPort_6_LineEdit->setEnabled(false);
      _mountpoint_6_LineEdit->setEnabled(false);
      _password_6_LineEdit->setEnabled(false);
      _outFile_6_LineEdit->setEnabled(false);
      _refSys_6_ComboBox->setEnabled(false);
      _CoM_6_CheckBox->setEnabled(false);
    }
  }

  // Enable/disable Broadcast Corrections VII Options
  // ------------------------------------------------
  if (sender() == 0 || sender() == _outHost_7_LineEdit) {
    if (!_outHost_7_LineEdit->text().isEmpty()) {
      _outPort_7_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_7_LineEdit->setStyleSheet("background-color: white");
      _password_7_LineEdit->setStyleSheet("background-color: white");
      _outFile_7_LineEdit->setStyleSheet("background-color: white");
      _refSys_7_ComboBox->setStyleSheet("background-color: white");
      _CoM_7_CheckBox->setPalette(palette_white);
      _outPort_7_LineEdit->setEnabled(true);
      _mountpoint_7_LineEdit->setEnabled(true);
      _password_7_LineEdit->setEnabled(true);
      _outFile_7_LineEdit->setEnabled(true);
      _refSys_7_ComboBox->setEnabled(true);
      _CoM_7_CheckBox->setEnabled(true);
    } 
    else {
      _outPort_7_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_7_LineEdit->setStyleSheet("background-color: lightGray");
      _password_7_LineEdit->setStyleSheet("background-color: lightGray");
      _outFile_7_LineEdit->setStyleSheet("background-color: lightGray");
      _refSys_7_ComboBox->setStyleSheet("background-color: lightGray");
      _CoM_7_CheckBox->setPalette(palette_gray);
      _outPort_7_LineEdit->setEnabled(false);
      _mountpoint_7_LineEdit->setEnabled(false);
      _password_7_LineEdit->setEnabled(false);
      _outFile_7_LineEdit->setEnabled(false);
      _refSys_7_ComboBox->setEnabled(false);
      _CoM_7_CheckBox->setEnabled(false);
    }
  }

  // Enable/disable Broadcast Corrections VIII Options
  // -------------------------------------------------
  if (sender() == 0 || sender() == _outHost_8_LineEdit) {
    if (!_outHost_8_LineEdit->text().isEmpty()) {
      _outPort_8_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_8_LineEdit->setStyleSheet("background-color: white");
      _password_8_LineEdit->setStyleSheet("background-color: white");
      _outFile_8_LineEdit->setStyleSheet("background-color: white");
      _refSys_8_ComboBox->setStyleSheet("background-color: white");
      _CoM_8_CheckBox->setPalette(palette_white);
      _outPort_8_LineEdit->setEnabled(true);
      _mountpoint_8_LineEdit->setEnabled(true);
      _password_8_LineEdit->setEnabled(true);
      _outFile_8_LineEdit->setEnabled(true);
      _refSys_8_ComboBox->setEnabled(true);
      _CoM_8_CheckBox->setEnabled(true);
    } 
    else {
      _outPort_8_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_8_LineEdit->setStyleSheet("background-color: lightGray");
      _password_8_LineEdit->setStyleSheet("background-color: lightGray");
      _outFile_8_LineEdit->setStyleSheet("background-color: lightGray");
      _refSys_8_ComboBox->setStyleSheet("background-color: lightGray");
      _CoM_8_CheckBox->setPalette(palette_gray);
      _outPort_8_LineEdit->setEnabled(false);
      _mountpoint_8_LineEdit->setEnabled(false);
      _password_8_LineEdit->setEnabled(false);
      _outFile_8_LineEdit->setEnabled(false);
      _refSys_8_ComboBox->setEnabled(false);
      _CoM_8_CheckBox->setEnabled(false);
    }
  }

  // Enable/disable Broadcast Corrections IX Options
  // -----------------------------------------------
  if (sender() == 0 || sender() == _outHost_9_LineEdit) {
    if (!_outHost_9_LineEdit->text().isEmpty()) {
      _outPort_9_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_9_LineEdit->setStyleSheet("background-color: white");
      _password_9_LineEdit->setStyleSheet("background-color: white");
      _outFile_9_LineEdit->setStyleSheet("background-color: white");
      _refSys_9_ComboBox->setStyleSheet("background-color: white");
      _CoM_9_CheckBox->setPalette(palette_white);
      _outPort_9_LineEdit->setEnabled(true);
      _mountpoint_9_LineEdit->setEnabled(true);
      _password_9_LineEdit->setEnabled(true);
      _outFile_9_LineEdit->setEnabled(true);
      _refSys_9_ComboBox->setEnabled(true);
      _CoM_9_CheckBox->setEnabled(true);
    } 
    else {
      _outPort_9_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_9_LineEdit->setStyleSheet("background-color: lightGray");
      _password_9_LineEdit->setStyleSheet("background-color: lightGray");
      _outFile_9_LineEdit->setStyleSheet("background-color: lightGray");
      _refSys_9_ComboBox->setStyleSheet("background-color: lightGray");
      _CoM_9_CheckBox->setPalette(palette_gray);
      _outPort_9_LineEdit->setEnabled(false);
      _mountpoint_9_LineEdit->setEnabled(false);
      _password_9_LineEdit->setEnabled(false);
      _outFile_9_LineEdit->setEnabled(false);
      _refSys_9_ComboBox->setEnabled(false);
      _CoM_9_CheckBox->setEnabled(false);
    }
  }

  // Enable/disable Broadcast Corrections X Options
  // ----------------------------------------------
  if (sender() == 0 || sender() == _outHost_10_LineEdit) {
    if (!_outHost_10_LineEdit->text().isEmpty()) {
      _outPort_10_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_10_LineEdit->setStyleSheet("background-color: white");
      _password_10_LineEdit->setStyleSheet("background-color: white");
      _outFile_10_LineEdit->setStyleSheet("background-color: white");
      _refSys_10_ComboBox->setStyleSheet("background-color: white");
      _CoM_10_CheckBox->setPalette(palette_white);
      _outPort_10_LineEdit->setEnabled(true);
      _mountpoint_10_LineEdit->setEnabled(true);
      _password_10_LineEdit->setEnabled(true);
      _outFile_10_LineEdit->setEnabled(true);
      _refSys_10_ComboBox->setEnabled(true);
      _CoM_10_CheckBox->setEnabled(true);
    } 
    else {
      _outPort_10_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_10_LineEdit->setStyleSheet("background-color: lightGray");
      _password_10_LineEdit->setStyleSheet("background-color: lightGray");
      _outFile_10_LineEdit->setStyleSheet("background-color: lightGray");
      _refSys_10_ComboBox->setStyleSheet("background-color: lightGray");
      _CoM_10_CheckBox->setPalette(palette_gray);
      _outPort_10_LineEdit->setEnabled(false);
      _mountpoint_10_LineEdit->setEnabled(false);
      _password_10_LineEdit->setEnabled(false);
      _outFile_10_LineEdit->setEnabled(false);
      _refSys_10_ComboBox->setEnabled(false);
      _CoM_10_CheckBox->setEnabled(false);
    }
  }

  // Enable/disable Broadcast Ephemerides
  // ------------------------------------
  if (sender() == 0 || sender() == _outHost_Eph_LineEdit) {
    if (!_outHost_Eph_LineEdit->text().isEmpty()) {
      _outPort_Eph_LineEdit->setStyleSheet("background-color: white");
      _mountpoint_Eph_LineEdit->setStyleSheet("background-color: white");
      _password_Eph_LineEdit->setStyleSheet("background-color: white");
      _samplEphSpinBox->setStyleSheet("background-color: white");
      _outPort_Eph_LineEdit->setEnabled(true);
      _mountpoint_Eph_LineEdit->setEnabled(true);
      _password_Eph_LineEdit->setEnabled(true);
      _samplEphSpinBox->setEnabled(true);
    } 
    else {
      _outPort_Eph_LineEdit->setStyleSheet("background-color: lightGray");
      _mountpoint_Eph_LineEdit->setStyleSheet("background-color: lightGray");
      _password_Eph_LineEdit->setStyleSheet("background-color: lightGray");
      _samplEphSpinBox->setStyleSheet("background-color: lightGray");
      _outPort_Eph_LineEdit->setEnabled(false);
      _mountpoint_Eph_LineEdit->setEnabled(false);
      _password_Eph_LineEdit->setEnabled(false);
      _samplEphSpinBox->setEnabled(false);
    }
  }

  // Enable/disable RINEX Clocks Options
  // -----------------------------------
  if (sender() == 0 || sender() == _rnxPathLineEdit) {
    if (!_rnxPathLineEdit->text().isEmpty()) {
      _rnxIntrComboBox->setStyleSheet("background-color: white");
      _rnxSamplSpinBox->setStyleSheet("background-color: white");
      _rnxIntrComboBox->setEnabled(true);
      _rnxSamplSpinBox->setEnabled(true);
    } 
    else {
      _rnxIntrComboBox->setStyleSheet("background-color: lightGray");
      _rnxSamplSpinBox->setStyleSheet("background-color: lightGray");
      _rnxIntrComboBox->setEnabled(false);
      _rnxSamplSpinBox->setEnabled(false);
    }
  }

  // Enable/disable SP3 Orbits Options
  // ---------------------------------
  if (sender() == 0 || sender() == _sp3PathLineEdit) {
    if (!_sp3PathLineEdit->text().isEmpty()) {
      _sp3IntrComboBox->setStyleSheet("background-color: white");
      _sp3SamplSpinBox->setStyleSheet("background-color: white");
      _sp3IntrComboBox->setEnabled(true);
      _sp3SamplSpinBox->setEnabled(true);
    } 
    else {
      _sp3IntrComboBox->setStyleSheet("background-color: lightGray");
      _sp3SamplSpinBox->setStyleSheet("background-color: lightGray");
      _sp3IntrComboBox->setEnabled(false);
      _sp3SamplSpinBox->setEnabled(false);
    }
  }

}

//  Custom transformation parameters
////////////////////////////////////////////////////////////////////////////
void bnsWindow::customTrafo(const QString &text){
  if (text == "Custom" ) {
    bnsCustomTrafo* dlg = new bnsCustomTrafo(this);
    dlg->exec();
    delete dlg;
  }
}
  
