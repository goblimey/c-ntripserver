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

#ifndef REQCDLG_H
#define REQCDLG_H

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>

#include "bncconst.h"

class reqcDlg : public QDialog {

  Q_OBJECT

  public:
    reqcDlg(QWidget* parent);
    ~reqcDlg();

  public slots:
    void slotReqcTextChanged();

  signals:

  private slots:
    void slotOK();
    void slotWhatsThis();

  protected:
    virtual void closeEvent(QCloseEvent *);

  private:
   void saveOptions();

   QComboBox*     _reqcRnxVersion;
   QComboBox*     _reqcSampling;
   QDateTimeEdit* _reqcStartDateTime;
   QDateTimeEdit* _reqcEndDateTime;
   QLineEdit*     _reqcRunBy;
   QLineEdit*     _reqcV2Priority;
   QLineEdit*     _reqcUseObsTypes;
   QLineEdit*     _reqcComment;
   QLineEdit*     _reqcOldMarkerName;
   QLineEdit*     _reqcNewMarkerName;
   QLineEdit*     _reqcOldAntennaName;
   QLineEdit*     _reqcNewAntennaName;
   QLineEdit*     _reqcOldAntennaNumber;
   QLineEdit*     _reqcNewAntennaNumber;
   QLineEdit*     _reqcOldAntennadN;
   QLineEdit*     _reqcNewAntennadN;
   QLineEdit*     _reqcOldAntennadE;
   QLineEdit*     _reqcNewAntennadE;
   QLineEdit*     _reqcOldAntennadU;
   QLineEdit*     _reqcNewAntennadU;
   QLineEdit*     _reqcOldReceiverName;
   QLineEdit*     _reqcNewReceiverName;
   QLineEdit*     _reqcOldReceiverNumber;
   QLineEdit*     _reqcNewReceiverNumber;
   QPushButton*   _buttonOK;
   QPushButton*   _buttonCancel;
   QPushButton*   _buttonWhatsThis;
};

#endif
