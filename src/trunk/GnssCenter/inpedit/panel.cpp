
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_panel
 *
 * Purpose:    Panel in RTNet Input File
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "panel.h"
#include "keyword.h"
#include "uniline.h"

using namespace std;
using namespace GnssCenter;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_panel::t_panel(const QString& line, QTextStream& inStream,
                 QMap<QString, t_keyword*>* keywords,
                 QStringList& staticLines) : QScrollArea(0) {

  _keywords = keywords;

  _ok = true;

  _layout = new QGridLayout();
  _layout->setSpacing(0);
  _layout->setSizeConstraint(QLayout::SetFixedSize);
  
  _page = new QWidget();
  _page->setLayout(_layout);  

  QFont font("Courier");
  _page->setFont(font);

  this->setWidget(_page);

  read(line, inStream, staticLines);

  setWidgetResizable(true);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_panel::~t_panel() {
}

// Read Panel
////////////////////////////////////////////////////////////////////////////
void t_panel::read(QString line, QTextStream& inStream, QStringList& staticLines) {
  staticLines << line;
  int iRow = -1;
  while (inStream.status() == QTextStream::Ok && !inStream.atEnd()) {
    line = inStream.readLine().trimmed();
    staticLines << line;
    if  (line.isEmpty() || line.indexOf("END_PANEL") != -1) {
      break;
    }
    else {
      ++iRow;

      QStringList keyNames;
      int lastInd = line.lastIndexOf('#');
      if (lastInd != -1) {
        keyNames = line.mid(lastInd+1).split(QRegExp("\\s+"),
                                             QString::SkipEmptyParts);
        line = line.left(lastInd).replace('#', " ");
      }
      
      // Empty Line
      // ----------
      if (line.trimmed().isEmpty()) {
        QLabel* label = new QLabel(line, this);
        addWidget(label, iRow, 0, 1, line.length());
        continue;
      }

      // Non-Editable Text
      // -----------------
      QStringListIterator it(
            line.split(QRegExp(">[^<]+<|\\s{2,}"),QString::SkipEmptyParts) );
      int icLast = -1;
      while (it.hasNext()) {
        QString txt = it.next().trimmed();
        if (!txt.isEmpty()) {
          QLabel* label = new QLabel();
          label->setScaledContents(true);
          label->setAlignment(Qt::AlignLeft);
          int ic = line.indexOf(txt, icLast+1);
          icLast = ic + txt.length();
          txt.replace(QRegExp("@")," ");
          label->setText(txt);
          addWidget(label, iRow, ic, 1, txt.length());
          if (iRow == 0) {
            QFont font = label->font();
            font.setBold(true);
            label->setFont(font);
          }
        }
      }

      // Editable Field
      // --------------
      int fld = -1;
      int pos = 0;
      while (true) {
        QRegExp fldRx(">([^<]+)<");
        pos = fldRx.indexIn(line, pos);
        if (pos != -1) {
          fld += 1;
          int len = fldRx.matchedLength();
          if (_keywords->contains(keyNames[fld])) {
            t_keyword* keyword = (*_keywords)[ keyNames[fld] ];
            QWidget* widget = keyword->createWidget(fldRx.cap(1).trimmed());
            if (widget) {
              addWidget(widget, iRow, pos, 1, len);
            }
          }
          pos += len;
        }
        else {
          break;
        }
      }
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_panel::addWidget(QWidget* widget, int row, int col,
                        int rSpan, int cSpan, const QString& toolTip) {

  const int fontW = QFontMetrics(_page->font()).width('W');
  const int fontH = QFontMetrics(_page->font()).height();

  for (int ii = row; ii < row+rSpan; ii++) {
    _layout->setRowMinimumHeight(ii, fontH+6);
  }
  for (int ii = col; ii < col+cSpan; ii++) {
    _layout->setColumnMinimumWidth(ii, fontW+1);
  }

  t_uniLine* uniline = dynamic_cast<t_uniLine*>(widget);
  if (uniline) {
    _layout->addWidget(widget, row, col, rSpan, cSpan);
  }
  else {
    QSize size(cSpan*fontW+6, rSpan*fontH+4);
    widget->setMinimumSize(size);
    widget->setMaximumSize(size);
    _layout->addWidget(widget, row, col, rSpan, cSpan,
                       Qt::AlignLeft | Qt::AlignTop);
  }

  // Tool Tip (keyword name)
  // -----------------------
  if (!toolTip.isEmpty()) {
    widget->setToolTip(toolTip);
  }
}
