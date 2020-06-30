// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2009
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

#ifndef BNCSERIALPORT_H
#define BNCSERIALPORT_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QWhatsThis>

class bncSerialPort : public QDialog {
  Q_OBJECT

  public:
    bncSerialPort(QWidget* parent);
    ~bncSerialPort();
    QString*   _serialPort1;

  signals:
    void newMountPoints(QStringList* mountPoints);

  private slots:
    virtual void accept();
    void slotWhatsThis();

  private:
    QLineEdit*   _serialMountpointLineEdit;
    QLineEdit*   _serialPortLineEdit;
    QLineEdit*   _serialFormatLineEdit;
    QLineEdit*   _serialLatLineEdit;
    QLineEdit*   _serialLonLineEdit;
    QLineEdit*   _serialCountryLineEdit;
    QComboBox*   _serialBaudRateComboBox;
    QComboBox*   _serialFlowControlComboBox;
    QComboBox*   _serialDataBitsComboBox;
    QComboBox*   _serialParityComboBox;
    QComboBox*   _serialStopBitsComboBox;

    QPushButton* _buttonGet;
    QPushButton* _buttonCancel;
    QPushButton* _buttonOK;
    QPushButton* _buttonWhatsThis;
};

#endif
