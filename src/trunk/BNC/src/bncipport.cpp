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
 * Class:      bncIpPort
 *
 * Purpose:    Select host for stream retrieval without NTRIP
 *
 * Author:     G. Weber
 *
 * Created:    18-Feb-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "bncipport.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncIpPort::bncIpPort(QWidget* parent) : QDialog(parent) {

  setMinimumSize(400,150);
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  QGridLayout* editLayout = new QGridLayout;

  setWindowTitle(tr("Add Stream from TCP/IP Port"));

  _ipHostLineEdit = new QLineEdit();
  _ipPortLineEdit = new QLineEdit();
  _ipMountLineEdit = new QLineEdit();
  _ipFormatLineEdit = new QLineEdit();
  _ipLatLineEdit = new QLineEdit();
  _ipLonLineEdit = new QLineEdit();
  _ipCountryLineEdit = new QLineEdit();

  int ww = QFontMetrics(font()).width('w');
  _ipPortLineEdit->setMaximumWidth(9*ww);
  _ipMountLineEdit->setMaximumWidth(9*ww);
  _ipFormatLineEdit->setMaximumWidth(9*ww);
  _ipLatLineEdit->setMaximumWidth(9*ww);
  _ipLonLineEdit->setMaximumWidth(9*ww);
  _ipCountryLineEdit->setMaximumWidth(9*ww);

  // WhatsThis, Add Stream from TCP/IP Port
  // --------------------------------------
  _ipHostLineEdit->setWhatsThis(tr("<p>If no proxy server is involved in the communication, BNC allows to retrieve streams via TCP directly from an IP address without using the Ntrip transport protocol.</p><p>Enter the IP address of the stream providing host.</p>"));
  _ipPortLineEdit->setWhatsThis(tr("<p>Enter the IP port number of the stream providing host.</p>"));
  _ipMountLineEdit->setWhatsThis(tr("<p>Specify a mountpoint.</p><p>Recommended is a 4-character station ID.<br>Example: FFMJ</p>"));
  _ipFormatLineEdit->setWhatsThis(tr("<p>Specify the stream format.</p><p>Available options are 'RTCM_2', 'RTCM_3', RTNET, and 'ZERO'.</p>"));
  _ipLatLineEdit->setWhatsThis(tr("<p>Enter the approximate latitude of the stream providing receiver in degrees.<p></p>Example: 45.32</p>"));
  _ipLonLineEdit->setWhatsThis(tr("<p>Enter the approximate longitude of the stream providing receiver in degrees.<p></p>Example: -15.20</p>"));
  _ipCountryLineEdit->setWhatsThis(tr("<p>Specify the country code.</p><p>Follow the ISO 3166-1 alpha-3a code.<br>Example, Germany: DEU</p>"));

  editLayout->addWidget(new QLabel(tr("Host")),      0, 0, Qt::AlignRight);
  editLayout->addWidget(_ipHostLineEdit,             0, 1);
  editLayout->addWidget(new QLabel(tr("Port")),      0, 2, Qt::AlignRight);
  editLayout->addWidget(_ipPortLineEdit,             0, 3);
  editLayout->addWidget(new QLabel(tr("Mountpoint")),1, 0, Qt::AlignRight);
  editLayout->addWidget(_ipMountLineEdit,            1, 1);
  editLayout->addWidget(new QLabel(tr("Format")),    1, 2, Qt::AlignRight);
  editLayout->addWidget(_ipFormatLineEdit,           1, 3);
  editLayout->addWidget(new QLabel(tr("Latitude")),  2, 0, Qt::AlignRight);
  editLayout->addWidget(_ipLatLineEdit,              2, 1);
  editLayout->addWidget(new QLabel(tr("Longitude")), 2, 2, Qt::AlignRight);
  editLayout->addWidget(_ipLonLineEdit,              2, 3);
  editLayout->addWidget(new QLabel(tr("Country")),   3, 0, Qt::AlignRight);
  editLayout->addWidget(_ipCountryLineEdit,          3, 1);

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
bncIpPort::~bncIpPort() {
  delete _buttonCancel;
  delete _buttonOK;
  delete _buttonWhatsThis;
}

// Accept slot
////////////////////////////////////////////////////////////////////////////
void bncIpPort::accept() {

  QStringList* mountPoints = new QStringList;

  if ( !_ipHostLineEdit->text().isEmpty()   &&
       !_ipPortLineEdit->text().isEmpty()   &&
       !_ipMountLineEdit->text().isEmpty()  &&
       !_ipFormatLineEdit->text().isEmpty() &&
       !_ipCountryLineEdit->text().isEmpty() &&
       !_ipLatLineEdit->text().isEmpty()    &&
       !_ipLonLineEdit->text().isEmpty() ) {

    mountPoints->push_back("//" + _ipHostLineEdit->text() + ":" 
                                + _ipPortLineEdit->text() + "/" 
                                + _ipMountLineEdit->text() + " "
                                + _ipFormatLineEdit->text() + " "
                                + _ipCountryLineEdit->text() + " "
                                + _ipLatLineEdit->text() + " "
                                + _ipLonLineEdit->text() + " "
                                + "no N");
  } else {
   QMessageBox::warning(this, tr("Warning"),
                               tr("Incomplete settings"),
                               QMessageBox::Ok);
  }

  emit newMountPoints(mountPoints);

  QDialog::accept();
}

// Whats This Help
void bncIpPort::slotWhatsThis() {
QWhatsThis::enterWhatsThisMode();
}

