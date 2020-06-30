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

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncSerialPort
 *
 * Purpose:    Select serial port for stream retrieval
 *
 * Author:     G. Weber
 *
 * Created:    18-Feb-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include "bncserialport.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSerialPort::bncSerialPort(QWidget* parent) : QDialog(parent) {

  setMinimumSize(400,150);
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  QGridLayout* editLayout = new QGridLayout;

  setWindowTitle(tr("Add Stream from Serial Port"));

  _serialMountpointLineEdit = new QLineEdit();
  _serialPortLineEdit = new QLineEdit();
  _serialFormatLineEdit = new QLineEdit();
  _serialBaudRateComboBox = new QComboBox();
  _serialFlowControlComboBox = new QComboBox();
  _serialDataBitsComboBox = new QComboBox();
  _serialParityComboBox = new QComboBox();
  _serialStopBitsComboBox = new QComboBox();
  _serialLatLineEdit = new QLineEdit();
  _serialLonLineEdit = new QLineEdit();
  _serialCountryLineEdit = new QLineEdit();

  _serialBaudRateComboBox->addItems(QString("110,300,600,"
            "1200,2400,4800,9600,19200,38400,57600,115200").split(","));
  _serialFlowControlComboBox->addItems(QString("OFF,XONXOFF,HARDWARE").split(","));
  _serialDataBitsComboBox->addItems(QString("5,6,7,8").split(","));
  _serialParityComboBox->addItems(QString("NONE,ODD,EVEN,SPACE").split(","));
  _serialStopBitsComboBox->addItems(QString("1,2").split(","));

  _serialBaudRateComboBox->setCurrentIndex(7);
  _serialDataBitsComboBox->setCurrentIndex(3);

  int ww = QFontMetrics(font()).width('w');
  _serialMountpointLineEdit->setMaximumWidth(11*ww);
  _serialPortLineEdit->setMaximumWidth(11*ww);
  _serialBaudRateComboBox->setMaximumWidth(9*ww);
  _serialFlowControlComboBox->setMaximumWidth(11*ww);
  _serialDataBitsComboBox->setMaximumWidth(5*ww);
  _serialParityComboBox->setMaximumWidth(9*ww);
  _serialStopBitsComboBox->setMaximumWidth(5*ww);
  _serialLatLineEdit->setMaximumWidth(11*ww);
  _serialLonLineEdit->setMaximumWidth(9*ww);
  _serialFormatLineEdit->setMaximumWidth(9*ww);
  _serialCountryLineEdit->setMaximumWidth(11*ww);

  // WhatsThis, Add Stream from Serial Port
  // --------------------------------------
  _serialMountpointLineEdit->setWhatsThis(tr("<p>BNC allows to retrieve streams via serial port without using the Ntrip transport protocol.</p><p>Specify a mountpoint. Recommended is a 4-character station ID.<br>Example: FFMJ</p>"));
  _serialFormatLineEdit->setWhatsThis(tr("<p>Specify the stream format.</p><p>Available options are 'RTCM_2', 'RTCM_3', 'RTNET', and 'ZERO'.</p>"));
  _serialLatLineEdit->setWhatsThis(tr("<p>Enter the approximate latitude of the stream providing receiver in degrees.<p></p>Example: 45.32</p>"));
  _serialLonLineEdit->setWhatsThis(tr("<p>Enter the approximate latitude of the stream providing receiver in degrees.<p></p>Example: 45.32</p>"));
  _serialCountryLineEdit->setWhatsThis(tr("<p>Specify the country code.</p><p>Follow the ISO 3166-1 alpha-3a code.<br>Example, Germany: DEU</p>"));
  _serialPortLineEdit->setWhatsThis(tr("<p>Enter the serial 'Port name' selected for communication with your serial connected device. Valid port names are</p><pre>Windows:       COM1, COM2<br>Linux:         /dev/ttyS0, /dev/ttyS1<br>FreeBSD:       /dev/ttyd0, /dev/ttyd1<br>Digital Unix:  /dev/tty01, /dev/tty02<br>HP-UX:         /dev/tty1p0, /dev/tty2p0<br>SGI/IRIX:      /dev/ttyf1, /dev/ttyf2<br>SunOS/Solaris: /dev/ttya, /dev/ttyb</pre><p>You must plug a serial cable in the port defined here before you start BNC.</p>"));
  _serialBaudRateComboBox->setWhatsThis(tr("<p>Select a 'Baud rate' for the serial input link.</p><p>Your selection must equal the baud rate configured to the serial connected device. Note that using a high baud rate is recommended.</p>"));
  _serialDataBitsComboBox->setWhatsThis(tr("<p>Select the number of 'Data bits' for the serial input link.</p><p>Your selection must equal the number of data bits configured to the serial connected device. Note that often 8 data bits are used.</p>"));
  _serialParityComboBox->setWhatsThis(tr("<p>Select the 'Parity' for the serial input link.</p><p>Your selection must equal the parity selection configured to the serial connected device. Note that parity is often set to 'NONE'.</p>"));
  _serialStopBitsComboBox->setWhatsThis(tr("<p>Select the number of 'Stop bits' for the serial input link.</p><p>Your selection must equal the number of stop bits configured to the serial connected device. Note that often 1 stop bit is used.</p>"));
  _serialFlowControlComboBox->setWhatsThis(tr("<p>Select a 'Flow control' for the serial input link.</p><p>Your selection must equal the flow control configured to the serial connected device. Select 'OFF' if you don't know better.</p>"));

  editLayout->addWidget(new QLabel(tr("Mountpoint")),  0, 0, Qt::AlignRight);
  editLayout->addWidget(_serialMountpointLineEdit,     0, 1);
  editLayout->addWidget(new QLabel(tr("Format")),      0, 2, Qt::AlignRight);
  editLayout->addWidget(_serialFormatLineEdit,         0, 3);
  editLayout->addWidget(new QLabel(tr("Latitude")),    1, 0, Qt::AlignRight);
  editLayout->addWidget(_serialLatLineEdit,            1, 1);
  editLayout->addWidget(new QLabel(tr("Longitude")),   1, 2, Qt::AlignRight);
  editLayout->addWidget(_serialLonLineEdit,            1, 3);
  editLayout->addWidget(new QLabel(tr("Country")),     2, 0, Qt::AlignRight);
  editLayout->addWidget(_serialCountryLineEdit,        2, 1);
  editLayout->addWidget(new QLabel(tr("Port name")),   3, 0, Qt::AlignRight);
  editLayout->addWidget(_serialPortLineEdit,           3, 1);
  editLayout->addWidget(new QLabel(tr("Baud rate")),   3, 2, Qt::AlignRight);
  editLayout->addWidget(_serialBaudRateComboBox,       3, 3);
  editLayout->addWidget(new QLabel(tr("Data bits")),   4, 0, Qt::AlignRight);
  editLayout->addWidget(_serialDataBitsComboBox,       4, 1);
  editLayout->addWidget(new QLabel(tr("Parity")),      4, 2, Qt::AlignRight);
  editLayout->addWidget(_serialParityComboBox,         4, 3);
  editLayout->addWidget(new QLabel(tr("Stop bits")),   5, 0, Qt::AlignRight);
  editLayout->addWidget(_serialStopBitsComboBox,       5, 1);
  editLayout->addWidget(new QLabel(tr("Flow control")),5, 2, Qt::AlignRight);
  editLayout->addWidget(_serialFlowControlComboBox,    5, 3);

  mainLayout->addLayout(editLayout);

  _buttonWhatsThis = new QPushButton(tr("Help=Shift+F1"), this);
  connect(_buttonWhatsThis, SIGNAL(clicked()), this, SLOT(slotWhatsThis()));
 
  _buttonCancel = new QPushButton(tr("Cancel"), this);
  connect(_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

  _buttonOK = new QPushButton(tr("OK"), this);
  connect(_buttonOK, SIGNAL(clicked()), this, SLOT(accept()));

  _buttonOK->setDefault(true);

  QHBoxLayout* buttonLayout = new QHBoxLayout;

  buttonLayout->addWidget(_buttonWhatsThis);
  buttonLayout->addStretch(1);
  buttonLayout->addWidget(_buttonCancel);
  buttonLayout->addWidget(_buttonOK);

  mainLayout->addLayout(buttonLayout);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSerialPort::~bncSerialPort() {
  delete _buttonCancel;
  delete _buttonOK;
  delete _buttonWhatsThis;
}

// Accept slot
////////////////////////////////////////////////////////////////////////////
void bncSerialPort::accept() {

  QStringList* mountPoints = new QStringList;

  QString _serialBaudRate    = _serialBaudRateComboBox->currentText(); 
  QString _serialFlowControl = _serialFlowControlComboBox->currentText(); 
  QString _serialDataBits    = _serialDataBitsComboBox->currentText(); 
  QString _serialParity      = _serialParityComboBox->currentText(); 
  QString _serialStopBits    = _serialStopBitsComboBox->currentText(); 

  if ( !_serialMountpointLineEdit->text().isEmpty() &&
       !_serialPortLineEdit->text().isEmpty() &&
       !_serialFormatLineEdit->text().isEmpty() &&
       !_serialCountryLineEdit->text().isEmpty() &&
       !_serialLatLineEdit->text().isEmpty() &&
       !_serialLonLineEdit->text().isEmpty() ) {
    mountPoints->push_back("//" 
      + _serialPortLineEdit->text().replace("/","-").replace(QRegExp("^[-]"), "") + "-"
      + _serialDataBits + "-"
      + _serialParity + "-"
      + _serialStopBits + "-"
      + _serialFlowControl + "-"
      + _serialBaudRate + "/"
      + _serialMountpointLineEdit->text() + " "
      + _serialFormatLineEdit->text() + " "
      + _serialCountryLineEdit->text() + " "
      + _serialLatLineEdit->text() + " "
      + _serialLonLineEdit->text() + " "
      + "no S");
  } else {
   QMessageBox::warning(this, tr("Warning"),
                               tr("Incomplete settings"),
                               QMessageBox::Ok);
  }

  emit newMountPoints(mountPoints);

  QDialog::accept();
}

// Whats This Help
void bncSerialPort::slotWhatsThis() {
QWhatsThis::enterWhatsThisMode();
}

