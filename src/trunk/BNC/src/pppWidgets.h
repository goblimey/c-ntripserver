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

#ifndef PPPWIDGETS_H
#define PPPWIDGETS_H

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableWidget>

class qtFileChooser;

class t_pppWidgets : public QObject {
 Q_OBJECT

 public:
  t_pppWidgets();
  ~t_pppWidgets();
  void saveOptions();

  QComboBox*     _dataSource;
  qtFileChooser* _rinexObs;
  qtFileChooser* _rinexNav;
  QLineEdit*     _corrMount;
  qtFileChooser* _corrFile;
  qtFileChooser* _crdFile;
  qtFileChooser* _antexFile;
  qtFileChooser* _blqFile;
  QLineEdit*     _logPath;
  QLineEdit*     _nmeaPath;
  QLineEdit*     _snxtroPath;
  QComboBox*     _snxtroSampl;
  QComboBox*     _snxtroIntr;
  QLineEdit*     _snxtroAc;
  QLineEdit*     _snxtroSol;
  QCheckBox*     _v3filenames;
  QTableWidget*  _staTable;
  QComboBox*     _lcGPS;
  QComboBox*     _lcGLONASS;
  QComboBox*     _lcGalileo;
  QComboBox*     _lcBDS;
  QComboBox*     _modelObs;
  QComboBox*     _pseudoObs;
  QLineEdit*     _sigmaC1;
  QLineEdit*     _sigmaL1;
  QSpinBox*      _minObs;
  QSpinBox*      _minEle;
  QLineEdit*     _maxResC1;
  QLineEdit*     _maxResL1;
  QCheckBox*     _eleWgtCode;
  QCheckBox*     _eleWgtPhase;
  QLineEdit*     _seedingTime;

  QSpinBox*      _corrWaitTime;
  QPushButton*   _addStaButton;
  QPushButton*   _delStaButton;

  QLineEdit*     _plotCoordinates;
  QPushButton*   _mapWinButton;
  QRadioButton*  _useGoogleMap;
  QRadioButton*  _useOpenStreetMap;
  QLineEdit*     _audioResponse;
  QLineEdit*     _mapWinDotSize;
  QComboBox*     _mapWinDotColor;
  QSlider*       _mapSpeedSlider;

 private slots:
  void slotEnableWidgets();
  void slotAddStation();
  void slotDelStation();
  void slotPPPTextChanged();

 private:
  void readOptions();
  QList <QWidget*> _widgets;
};

#endif
