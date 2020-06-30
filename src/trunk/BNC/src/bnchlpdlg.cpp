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
 * Class:      bncHlpDlg
 *
 * Purpose:    Displays the help
 *
 * Author:     L. Mervart
 *
 * Created:    24-Sep-2006
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "bnchlpdlg.h"
#include "bnchtml.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncHlpDlg::bncHlpDlg(QWidget* parent, const QUrl& url) :
                    QDialog(parent) {

  const int ww = QFontMetrics(font()).width('w');

  bncHtml* _tb = new bncHtml;
  setWindowTitle("Help Contents");
  _tb->setSource(url);
  _tb->setReadOnly(true);
  connect(_tb, SIGNAL(backwardAvailable(bool)),
          this, SLOT(backwardAvailable(bool)));
  connect(_tb, SIGNAL(forwardAvailable(bool)),
          this, SLOT(forwardAvailable(bool)));

  QVBoxLayout* dlgLayout = new QVBoxLayout;
  dlgLayout->addWidget(_tb);

  QHBoxLayout* butLayout = new QHBoxLayout;

  _backButton = new QPushButton("Backward");
  _backButton->setMaximumWidth(10*ww);
  _backButton->setEnabled(false);
  connect(_backButton, SIGNAL(clicked()), _tb, SLOT(backward()));
  butLayout->addWidget(_backButton);

  _forwButton = new QPushButton("Forward");
  _forwButton->setMaximumWidth(10*ww);
  _forwButton->setEnabled(false);
  connect(_forwButton, SIGNAL(clicked()), _tb, SLOT(forward()));
  butLayout->addWidget(_forwButton);

  _closeButton = new QPushButton("Close");
  _closeButton->setMaximumWidth(10*ww);
  butLayout->addWidget(_closeButton);
  connect(_closeButton, SIGNAL(clicked()), this, SLOT(close()));

  dlgLayout->addLayout(butLayout);

  setLayout(dlgLayout);
  resize(110*ww, 100*ww);
  show();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncHlpDlg::~bncHlpDlg() {
  delete _tb;
  delete _backButton;
  delete _forwButton;
  delete _closeButton;
}

// Slots
////////////////////////////////////////////////////////////////////////////
void bncHlpDlg::backwardAvailable(bool avail) {
  _backButton->setEnabled(avail);
}

void bncHlpDlg::forwardAvailable(bool avail) {
  _forwButton->setEnabled(avail);
}
