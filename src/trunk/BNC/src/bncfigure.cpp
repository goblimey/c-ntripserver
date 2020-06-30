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
 * Class:      bncFigure
 *
 * Purpose:
 *
 * Author:     Perlt, Mervart
 *
 * Created:    11-Nov-2009
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include <QMapIterator>
#include <QPainter>
#include <QTimer>
#include <QUrl>
#include <QVariant>

#include "bncfigure.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncFigure::bncFigure(QWidget *parent) : QWidget(parent) {
  updateMountPoints();
  slotNextAnimationFrame();
  for (int ii = 0; ii <= 1000; ii++) {
    _ran[0][ii] = qrand() % 255;
    _ran[1][ii] = qrand() % 255;
    _ran[2][ii] = qrand() % 255;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncFigure::~bncFigure() {
  QMapIterator<QByteArray, sumAndMean*> it(_bytes);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncFigure::updateMountPoints() {
  QMutexLocker locker(&_mutex);

  _counter = 0;
  _maxRate = 0;

  QMapIterator<QByteArray, sumAndMean*> it1(_bytes);
  while (it1.hasNext()) {
    it1.next();
    delete it1.value();
  }
  _bytes.clear();

  bncSettings settings;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    QStringList hlp   = it.next().split(" ");
    if (hlp.size() <= 1) continue;
    QUrl        url(hlp[0]);
    QByteArray  staID = url.path().mid(1).toLatin1();
    _bytes[staID] = new sumAndMean();
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncFigure::slotNewData(const QByteArray staID, double nbyte) {
  QMutexLocker locker(&_mutex);
  QMap<QByteArray, sumAndMean*>::const_iterator it = _bytes.find(staID);
  if (it != _bytes.end()) {
    it.value()->_sum += nbyte*8.;
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncFigure::slotNextAnimationFrame() {
  QMutexLocker locker(&_mutex);

  const static int MAXCOUNTER = 10;

  ++_counter;

  // If counter reaches its maximal value, compute the mean rate
  // -----------------------------------------------------------
  if (_counter == MAXCOUNTER) {
    _maxRate = 0.0;
    QMapIterator<QByteArray, sumAndMean*> it(_bytes);
    while (it.hasNext()) {
      it.next();
      it.value()->_mean = it.value()->_sum / _counter;
      it.value()->_sum  = 0.0;
      if (it.value()->_mean > _maxRate) {
        _maxRate = it.value()->_mean;
      }
    }
    _counter = 0;
  }

  update();

  QTimer::singleShot(1000, this, SLOT(slotNextAnimationFrame()));
}

//
////////////////////////////////////////////////////////////////////////////
void bncFigure::paintEvent(QPaintEvent *) {

  int xMin =   0;
  int xMax = 640;
  int yMin =   0;
  int yMax = 140;
  float xLine = .60;

  QPainter painter(this);

  QFont font;
  font.setPointSize(int(font.QFont::pointSize()*0.8));
  painter.setFont(font);

  // y-axis
  // ------
  int yLength = int((yMax-yMin)*xLine) - (yMin+10);
  painter.drawLine(xMin+60, int((yMax-yMin)*xLine), xMin+60, yMin+10);

  double maxRateRounded;
  QString maxRateStr;
  if (_maxRate < 1e3) {
    maxRateRounded = int(_maxRate/200)*200 + 300;
    maxRateStr = QString("%1 bps  ").arg(int(maxRateRounded/200)*200);
    painter.drawText(0, int((yMax-yMin)*xLine)-5, xMin+60,15,Qt::AlignRight,tr("0 bps  "));
  }
  else if (_maxRate < 1e6) {
    maxRateRounded = int(_maxRate/1.e3)*1.e3 + 1500;
    maxRateStr = QString("%1 kbps  ").arg(int(maxRateRounded/1.e3));
    painter.drawText(0, int((yMax-yMin)*xLine)-5, xMin+60,15,Qt::AlignRight,tr("0 kbps  "));
  }
  else {
    maxRateRounded = int(_maxRate/1.e6)*1.e6 + 1500000;
    maxRateStr = QString("%1 Mbps  ").arg(int(maxRateRounded/1.e6));
    painter.drawText(0, int((yMax-yMin)*xLine)-5, xMin+60,15,Qt::AlignRight,tr("0 Mbps  "));
  }

  if(_maxRate > 0.0) {
    painter.drawText(0, yMin+20-5, xMin+60,15,Qt::AlignRight,maxRateStr);
  }

  // x-axis
  // ------
  painter.drawLine(xMin+60, int((yMax-yMin)*xLine), xMax*3, int((yMax-yMin)*xLine));

  int anchor = 0;
  QMapIterator<QByteArray, sumAndMean*> it(_bytes);
  while (it.hasNext()) {
    it.next();
    QByteArray staID = it.key();

    int xx = xMin+80+anchor*12;

    if(_maxRate > 0.0) {
      int yy = int(yLength * (it.value()->_mean / maxRateRounded));
      QColor color = QColor::fromRgb(_ran[0][anchor],_ran[1][anchor],_ran[2][anchor],150);
      painter.fillRect(xx-13, int((yMax-yMin)*xLine)-yy, 9, yy,
                       QBrush(color,Qt::SolidPattern));
      painter.setPen(Qt::black);
      if(it.value()->_mean<=0) {
        painter.setPen(Qt::red);
      }
    }

    painter.save();
    painter.translate(xx-13, int(yMax-yMin)*xLine+65);
    painter.rotate(-90);
    painter.drawText(0,0,65,50,Qt::AlignRight,staID.left(5) + "   ");
    painter.restore();

    anchor++;
  }
}

