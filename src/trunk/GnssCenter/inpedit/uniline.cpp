
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_uniLine
 *
 * Purpose:    Universal-Line Widget (subclasses QTableWidget)
 *
 * Author:     L. Mervart
 *
 * Created:    08-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "uniline.h"

using namespace std;
using namespace GnssCenter;

static const char* plus_xpm[] = {
"16 16 2 1",
"  c #FFFFFFFFFFFF",
". c #00000000FFFF",
"                ",
"      ...       ",
"      ...       ",
"      ...       ",
"      ...       ",
"      ...       ",
" .............. ",
" .............. ",
" .............. ",
"      ...       ",
"      ...       ",
"      ...       ",
"      ...       ",
"      ...       ",
"                ",
"                "};

static const char* minus_xpm[] = {
"16 16 2 1",
"         c #FFFFFFFFFFFF",
".        c #00000000FFFF",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
" .............. ",
" .............. ",
" .............. ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

// Constructor
////////////////////////////////////////////////////////////////////////////////
t_uniLine::t_uniLine(const QString& fldMask, const QStringList& values,
                     QWidget* parent) : QTableWidget(parent) {

  static const QPixmap plusXPM(plus_xpm);
  static const QPixmap minusXPM(minus_xpm);
  static const QIcon   plusIcon(plusXPM);
  static const QIcon   minusIcon(minusXPM);

  setRowCount(values.size());
  
  QStringList labels = fldMask.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  if (labels.size() > 0) {
    setColumnCount(labels.size() + 2);
    labels << "" << "";
    setHorizontalHeaderLabels(labels);
  }

  for (int iRow = 0; iRow < values.size(); iRow++) {
    QStringList txt = values.at(iRow).split(QRegExp("\"\\s*\""));
    if (labels.size() == 0 && iRow == 0) {
      setColumnCount(txt.size() + 2);
    }
    for (int iCol = 0; iCol < txt.size(); iCol++) {    
      setItem(iRow, iCol, new QTableWidgetItem(txt.at(iCol).trimmed()));
    }
    for (int iCol = txt.size(); iCol < columnCount()-2; iCol++) {    
      setItem(iRow, iCol, new QTableWidgetItem(""));
    }
    setItem(iRow, columnCount()-2, new QTableWidgetItem(plusIcon, QString()));
    setItem(iRow, columnCount()-1, new QTableWidgetItem(minusIcon, QString()));
  }
  connect(this, SIGNAL(itemClicked(QTableWidgetItem*)),
          this, SLOT(slotItemClicked(QTableWidgetItem*)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////////
t_uniLine::~t_uniLine() {
}

// Add/Remove Line
////////////////////////////////////////////////////////////////////////////////
void t_uniLine::slotItemClicked(QTableWidgetItem* item) {

  static const QPixmap plusXPM(plus_xpm);
  static const QPixmap minusXPM(minus_xpm);
  static const QIcon   plusIcon(plusXPM);
  static const QIcon   minusIcon(minusXPM);

  int iCol = item->column();
  if      (iCol == columnCount()-2) {
    int iRow = item->row() + 1;
    insertRow(iRow);
    setItem(iRow, columnCount()-2, new QTableWidgetItem(plusIcon, QString()));
    setItem(iRow, columnCount()-1, new QTableWidgetItem(minusIcon, QString()));
  }
  else if (iCol == columnCount()-1) {
    removeRow(item->row());
  }
}

