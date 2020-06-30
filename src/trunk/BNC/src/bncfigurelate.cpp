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
 * Class:      bncFigureLate
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
#include <math.h>

#include <QMapIterator>
#include <QPainter>
#include <QTimer>
#include <QUrl>
#include <QVariant>

#include "bncfigurelate.h" 
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncFigureLate::bncFigureLate(QWidget *parent) : QWidget(parent) {
  updateMountPoints();
  slotNextAnimationFrame();
  for (int ii = 0; ii <= 1000; ii++) {
    _ran[0][ii] = qrand() % 255;
    _ran[1][ii] = qrand() % 255;
    _ran[2][ii] = qrand() % 100;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncFigureLate::~bncFigureLate() { 
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigureLate::updateMountPoints() {
  QMutexLocker locker(&_mutex);

  _latency.clear();

  bncSettings settings;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    QStringList hlp   = it.next().split(" ");
    if (hlp.size() <= 1) continue;
    QUrl        url(hlp[0]);
    QByteArray  staID = url.path().mid(1).toLatin1();
    _latency[staID] = 0.0;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigureLate::slotNewLatency(const QByteArray staID, double clate) {
  QMutexLocker locker(&_mutex);
  if (_latency.find(staID) != _latency.end()) {
    _latency[staID] = fabs(clate)*1000.0;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigureLate::slotNextAnimationFrame() {
  QMutexLocker locker(&_mutex);
  update();
  QTimer::singleShot(1000, this, SLOT(slotNextAnimationFrame()));
}

// 
////////////////////////////////////////////////////////////////////////////
void bncFigureLate::paintEvent(QPaintEvent *) {

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

  double maxLate = 0.0;
  QMapIterator<QByteArray, double> it1(_latency);
  while (it1.hasNext()) {
    it1.next();
    if (it1.value() > maxLate) {
      maxLate = it1.value();
    }
  }

  double maxLateRounded;
  QString maxLateStr;
  if(maxLate < 1e3) {
    maxLateRounded = int(maxLate/200)*200 + 300;
    maxLateStr = QString("%1 ms  ").arg(int(maxLateRounded/200)*200);
    painter.drawText(0, int((yMax-yMin)*xLine)-5, xMin+60,15,Qt::AlignRight,tr("0 ms  "));
  }
  else if (maxLate < 6e4) {
    maxLateRounded = int(maxLate/1.e3)*1.e3 + 1500;
    maxLateStr = QString("%1 sec  ").arg(int(maxLateRounded/1.e3));
    painter.drawText(0, int((yMax-yMin)*xLine)-5, xMin+60,15,Qt::AlignRight,tr("0 sec  "));
  }
  else {
    maxLateRounded = int(maxLate / 6.e4)*6.e4 + 90000;
    maxLateStr = QString("%1 min  ").arg(int(maxLateRounded/6.e4));
    painter.drawText(0, int((yMax-yMin)*xLine)-5, xMin+60,15,Qt::AlignRight,tr("0 min  "));
  }

  if(maxLate > 0.0) {
    painter.drawText(0, yMin+20-5, xMin+60,15,Qt::AlignRight,maxLateStr);
  }

  // x-axis
  // ------
  painter.drawLine(xMin+60, int((yMax-yMin)*xLine), xMax*3, int((yMax-yMin)*xLine));

  int anchor = 0;
  QMapIterator<QByteArray, double> it(_latency);
  while (it.hasNext()) {
    it.next();
    QByteArray staID = it.key();

    int xx = xMin+80+anchor*12;

    if(maxLate > 0.0) {
      int yy = int(yLength * (it.value() / maxLateRounded));
      QColor color = QColor::fromHsv(180,200,120+_ran[2][anchor]);
      painter.fillRect(xx-13, int((yMax-yMin)*xLine)-yy, 9, yy, 
                       QBrush(color,Qt::SolidPattern));
      painter.setPen(Qt::black);
      if(it.value()<=0) {
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

