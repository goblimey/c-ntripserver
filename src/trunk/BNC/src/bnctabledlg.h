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

#ifndef BNCTABLEDLG_H
#define BNCTABLEDLG_H

#include <QtCore>
#include <QComboBox>
#include <QDialog>
#include <QPushButton>
#include <QTableWidget>
#include <QWhatsThis>

#include "bncconst.h"

class bncCasterTableDlg : public QDialog {
  Q_OBJECT

  public:
    bncCasterTableDlg(const QString& ntripVersion, QWidget* parent);
    ~bncCasterTableDlg();

  signals:
    void newCaster(QString newCasterHost, QString newCasterPort);

  private slots:
    virtual void slotAcceptCasterTable();
    virtual void slotWhatsThis();

  private:
    QTableWidget* _casterTable;
    QPushButton*  _okButton;
    QPushButton*  _closeButton;
    QPushButton*  _whatsThisButton;
};

class bncTableDlg : public QDialog {
  Q_OBJECT

  public:
    bncTableDlg(QWidget* parent);
    ~bncTableDlg();
    static t_irc getFullTable(const QString& ntripVersion,
                              const QString& casterHost, int casterPort,
                              const QString& casterUser, const QString& casterPassword,
                              QStringList& allLines, bool alwaysRead = true);

  signals:
    void newMountPoints(QStringList* mountPoints);

  private slots:
    virtual void select();
    void slotGetTable();
    void slotShowMap();
    void slotSelectionChanged();
    void slotWhatsThis();
    void slotCasterTable();
    void slotNewCaster(QString newCasterHost, QString newCasterPort);
    void slotCasterHostChanged(const QString&);

  private:
    void addUrl(const QUrl& url);
    QComboBox*   _casterHostComboBox;
    QLineEdit*   _casterPortLineEdit;
    QLineEdit*   _casterUserLineEdit;
    QLineEdit*   _casterPasswordLineEdit;
    QComboBox*   _ntripVersionComboBox;

    QPushButton* _buttonGet;
    QPushButton* _buttonMap;
    QPushButton* _buttonClose;
    QPushButton* _buttonSelect;
    QPushButton* _buttonWhatsThis;
    QPushButton* _buttonCasterTable;

    QTableWidget* _table;
    QStringList   _allLines;
};

#endif
