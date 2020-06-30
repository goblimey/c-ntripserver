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

#ifndef BNCWINDOW_H
#define BNCWINDOW_H

#include <QDialog>
#include <QList>
#include <QMainWindow>
#include <QTextEdit>
#include <QWhatsThis>

#include "bncgetthread.h"
#include "bnccaster.h"
#include "pppWidgets.h"

class bncAboutDlg : public QDialog {
  Q_OBJECT
  public:
    bncAboutDlg(QWidget* parent);
    ~bncAboutDlg();
};

class bncFlowchartDlg : public QDialog {
  Q_OBJECT

  public:
    bncFlowchartDlg(QWidget* parent);
    ~bncFlowchartDlg();
};

class bncFigure;
class bncFigureLate;
class bncFigurePPP;
class bncBytesCounter;
class bncEphUploadCaster;
class qtFileChooser;
class bncMapWin;

class bncWindow : public QMainWindow {
  Q_OBJECT

  public:
    bncWindow();
    ~bncWindow();
    void CreateMenu();
    void AddToolbar();

  public slots:
    void slotMountPointsRead(QList<bncGetThread*>);
    void slotBncTextChanged();

  private slots:
    void slotWindowMessage(const QByteArray msg, bool showOnScreen);
    void slotHelp();
    void slotAbout();
    void slotFlowchart();
    void slotFontSel();
    void slotSaveOptions();
    void slotAddMountPoints();
    void slotMapMountPoints();
    void slotMapPPP();
    void slotMapPPPClosed();
    void slotStart();
    void slotStop();
    void slotNewMountPoints(QStringList* mountPoints);
    void slotDeleteMountPoints();
    void slotGetThreadsFinished();
    void slotSelectionChanged();
    void slotWhatsThis();
    void slotAddCmbRow();
    void slotDelCmbRow();
    void slotAddUploadRow();
    void slotDelUploadRow();
    void slotAddUploadEphRow();
    void slotDelUploadEphRow();
    void slotSetUploadTrafo();
    void slotReqcEditOption();
    void slotPostProcessingFinished();
    void slotPostProcessingProgress(int);

  protected:
    virtual void closeEvent(QCloseEvent *);

  private:
    void saveOptions();
    void populateMountPointsTable();
    void populateCmbTable();
    void populateUploadTable();
    void populateUploadEphTable();
    void enableWidget(bool enable, QWidget* widget);
    void startRealTime();
    void enableStartStop();

    QMenu*     _menuHlp;
    QMenu*     _menuFile;

    QAction*   _actHelp;
    QAction*   _actAbout;
    QAction*   _actFlowchart;
    QAction*   _actFontSel;
    QAction*   _actSaveOpt;
    QAction*   _actQuit;
    QAction*   _actMapMountPoints;
    QAction*   _actStart;
    QAction*   _actStop;
    QAction*   _actAddMountPoints;
    QAction*   _actDeleteMountPoints;
    QAction*   _actwhatsthis;
    QAction*   _actwhatsthismenu;

    QLineEdit* _proxyHostLineEdit;
    QLineEdit* _proxyPortLineEdit;
    QLineEdit* _sslCaCertPathLineEdit;
    QCheckBox* _sslIgnoreErrorsCheckBox;
    QLineEdit* _outFileLineEdit;
    QLineEdit* _outPortLineEdit;
    QLineEdit* _outUPortLineEdit;
    QCheckBox* _outLockTimeCheckBox;
    QLineEdit* _ephOutPortLineEdit;
    QLineEdit* _corrPortLineEdit;
    QLineEdit* _rnxPathLineEdit;
    QLineEdit* _ephPathLineEdit;
    QLineEdit* _corrPathLineEdit;
    QLineEdit* _miscMountLineEdit;
    QLineEdit* _miscPortLineEdit;

    QComboBox*     _reqcActionComboBox;
    QPushButton*   _reqcEditOptionButton;
    qtFileChooser* _reqcObsFileChooser;
    qtFileChooser* _reqcNavFileChooser;
    QLineEdit*     _reqcOutObsLineEdit;
    QLineEdit*     _reqcOutNavLineEdit;
    QLineEdit*     _reqcOutLogLineEdit;
    QLineEdit*     _reqcPlotDirLineEdit;
    QLineEdit*     _reqcSkyPlotSignals;
    QCheckBox*     _reqcLogSummaryOnly;

    qtFileChooser* _sp3CompFileChooser;
    QLineEdit*     _sp3CompExclude;
    QLineEdit*     _sp3CompLogLineEdit;

    QCheckBox* _rnxV3CheckBox;
    QLineEdit* _rnxV2Priority;
    QCheckBox* _ephV3CheckBox;
    QLineEdit* _rnxSkelLineEdit;
    QCheckBox* _rnxFileCheckBox;
    QLineEdit* _rnxScrpLineEdit;
    QLineEdit* _logFileLineEdit;
    QLineEdit* _rawOutFileLineEdit;
    QComboBox* _rnxIntrComboBox;
    QComboBox* _ephIntrComboBox;
    QComboBox* _corrIntrComboBox;
    QComboBox*  _rnxSamplComboBox;
    QComboBox*  _outSamplComboBox;
    QCheckBox* _rnxAppendCheckBox;
    QCheckBox* _autoStartCheckBox;
    QCheckBox* _miscScanRTCMCheckBox;
    QSpinBox*  _outWaitSpinBox;
    QComboBox* _adviseObsRateComboBox;
    QSpinBox*  _adviseFailSpinBox;
    QSpinBox*  _adviseRecoSpinBox;
    QLineEdit* _adviseScriptLineEdit;
    QComboBox* _miscIntrComboBox;
    QTableWidget* _mountPointsTable;

    QLineEdit* _serialPortNameLineEdit;
    QLineEdit* _serialMountPointLineEdit;
    QComboBox* _serialBaudRateComboBox;
    QComboBox* _serialParityComboBox;
    QComboBox* _serialDataBitsComboBox;
    QComboBox* _serialStopBitsComboBox;
    QComboBox* _serialFlowControlComboBox;
    QLineEdit* _serialHeightNMEALineEdit;
    QLineEdit* _serialFileNMEALineEdit;
    QComboBox* _serialAutoNMEAComboBox;
    QSpinBox*  _serialManualNMEASamplingSpinBox;

    QLineEdit*   _LatLineEdit;
    QLineEdit*   _LonLineEdit;

    QComboBox*  _onTheFlyComboBox;

    QTextEdit*  _log;

    QWidget*    _canvas;
    QTabWidget* _aogroup;

    QTabWidget* _loggroup;
    bncFigure*  _bncFigure;
    bncFigureLate*  _bncFigureLate;
    bncFigurePPP*   _bncFigurePPP;

    QTableWidget*  _cmbTable;
    QLineEdit*     _cmbMaxresLineEdit;
    QComboBox*     _cmbMethodComboBox;
    QSpinBox*      _cmbSamplSpinBox;
    QCheckBox*     _cmbUseGlonass;

    QTableWidget*  _uploadTable;
    QComboBox*     _uploadIntrComboBox;
    QSpinBox*      _uploadSamplRtcmEphCorrSpinBox;
    QSpinBox*      _uploadSamplSp3SpinBox;
    QSpinBox*      _uploadSamplClkRnxSpinBox;
    qtFileChooser* _uploadAntexFile;

    QTableWidget*  _uploadEphTable;
    QSpinBox*      _uploadSamplRtcmEphSpinBox;

    bncCaster*          _caster;
    bncEphUploadCaster* _casterEph;

    bool _runningRealTime;
    bool _runningPPP;
    bool _runningEdit;
    bool _runningQC;
    bool _runningSp3Comp;

    bool running() {return _runningRealTime || _runningPPP || _runningEdit ||
                    _runningQC || _runningSp3Comp;}

    bncMapWin*           _mapWin;

    QList<bncGetThread*> _threads;

    t_pppWidgets         _pppWidgets;
};

#ifdef GNSSCENTER_PLUGIN
#include "plugininterface.h"
class t_bncFactory : public QObject, public GnssCenter::t_pluginFactoryInterface {
 Q_OBJECT
 Q_INTERFACES(GnssCenter::t_pluginFactoryInterface)
 public:
  virtual QWidget* create() {return new bncWindow();}
  virtual QString getName() const {return QString("BNC");}
};
#endif

#endif
