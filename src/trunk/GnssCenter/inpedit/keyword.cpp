
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_keyword
 *
 * Purpose:    Keyword in RTNet Input File
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "keyword.h"
#include "lineedit.h"
#include "selwin.h"
#include "uniline.h"

using namespace std;
using namespace GnssCenter;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_keyword::t_keyword(QString line, QTextStream& inStream, QStringList& staticLines) {

  _ok     = false;
  _widget = 0;  // do not delete (it is owned by layout)

  int numVal = 0;
  QTextStream in(line.toAscii(), QIODevice::ReadOnly);
  in >> _name >> numVal;

  staticLines << _name;

  if (!_name.isEmpty()) {
    _ok = true;

    if      (numVal == 1) {
      _origValues.append(in.readLine().trimmed());
    }
    else if (numVal > 1) {
      for (int ii = 0; ii < numVal; ii++) {
        _origValues.append(inStream.readLine().trimmed());
      }
    }

    while (inStream.status() == QTextStream::Ok && !inStream.atEnd()) {
      line = inStream.readLine();
      staticLines << line;
      line = line.trimmed();
      if      (line.isEmpty()) {
        break;
      }
      else if (line[0] == '#') {
        int pos = 0;
        while (true) {
          QRegExp rx("([^#;]*)\\s+=\\s+([^#;]*)");
          pos = rx.indexIn(line, pos);
          if (pos == -1) {
            break;
          }
          else {
            pos += rx.matchedLength();
          }
          QString descKey = rx.cap(1).trimmed();
          QString descVal = rx.cap(2).trimmed();
          _desc[descKey]  = descVal;
        }
      }
    }

    // Remove leading and trailing double-quotes
    // -----------------------------------------
    _origValues.replaceInStrings(QRegExp("^\\s*\"|\"\\s*$"), QString());
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_keyword::~t_keyword() {
}

// Create Widget (it will be owned by layout)
////////////////////////////////////////////////////////////////////////////
QWidget* t_keyword::createWidget(const QString& fldMask) {

  if (_widget != 0) {
    // TODO: exception
  }

  QString widgetType = _desc.value("widget");

  if      (widgetType == "checkbox") {
    QCheckBox* chBox = new QCheckBox(); 
    if (_origValues.size() && _origValues[0] == "1") {
      chBox->setChecked(true);
    }
    _widget = chBox;
  }
  else if (widgetType == "combobox") {
    QComboBox* cmbBox = new QComboBox();
    cmbBox->addItems(_desc.value("cards").split(QRegExp("\\s"), QString::SkipEmptyParts));
    if (_origValues.size()) {
      int index = cmbBox->findText(_origValues[0]);
      if (index != -1) {
        cmbBox->setCurrentIndex(index);
      }
    }
    _widget = cmbBox;
  }
  else if (widgetType == "lineedit") {
    t_lineEdit* lineEdit = new t_lineEdit();
    if (_origValues.size()) {
      lineEdit->setText(_origValues[0]);
    }
    _widget = lineEdit;
  }
  else if (widgetType == "radiobutton") {
    QRadioButton* radButt = new QRadioButton();
    if (_origValues.size() && _origValues[0] == "1") {
      radButt->setChecked(true);
    }
    _widget = radButt;
  }
  else if (widgetType == "selwin") {
    t_selWin::Mode mode = t_selWin::File;
    if      (_desc.value("seldir") == "true") {
      mode = t_selWin::Directory;
    }
    else if (_desc.value("maxfiles").toInt() > 1) {
      mode = t_selWin::Files;
    }
    t_selWin* selWin = new t_selWin(mode);
    selWin->setFileNames(_origValues);
    _widget = selWin;
  }
  else if (widgetType == "spinbox") {
    QSpinBox* spinBox = new QSpinBox();
    QStringList rangeStr = _desc.value("range").split(QRegExp("\\s"), QString::SkipEmptyParts);
    if (rangeStr.size() >= 1) spinBox->setMinimum(rangeStr[0].toInt());
    if (rangeStr.size() >= 2) spinBox->setMaximum(rangeStr[1].toInt());
    if (rangeStr.size() >= 3) spinBox->setSingleStep(rangeStr[2].toInt());
    if (_origValues.size()) {
      spinBox->setValue(_origValues[0].toInt());
    }
    _widget = spinBox;
  }
  else if (widgetType == "uniline") {
    _widget = new t_uniLine(fldMask, _origValues);
  }

  return _widget;
}

// 
////////////////////////////////////////////////////////////////////////////
QStringList t_keyword::values() const {

  if (_widget == 0) {
    return _origValues;
  }

  QStringList values;

  QString widgetType = _desc.value("widget");

  if      (widgetType == "checkbox") {
    QCheckBox* chBox = static_cast<QCheckBox*>(_widget); 
    if (chBox->isChecked()) {
      values << "1";
    }
    else {
      values << "0";
    }
  }
  else if (widgetType == "combobox") {
    QComboBox* cmbBox = static_cast<QComboBox*>(_widget);
    values << cmbBox->currentText();
  }
  else if (widgetType == "lineedit") {
    t_lineEdit* lineEdit = static_cast<t_lineEdit*>(_widget);
    if (!lineEdit->text().isEmpty()) {
      values << lineEdit->text();
    }
  }
  else if (widgetType == "radiobutton") {
    QRadioButton* radButt = static_cast<QRadioButton*>(_widget);
    if (radButt->isChecked()) {
      values << "1";
    }
    else {
      values << "0";
    }
  }
  else if (widgetType == "selwin") {
    t_selWin* selWin = static_cast<t_selWin*>(_widget);
    values << selWin->fileNames();
  }
  else if (widgetType == "spinbox") {
    QSpinBox* spinBox = static_cast<QSpinBox*>(_widget);
    values << QString("%1").arg(spinBox->value());
  }
  else if (widgetType == "uniline") {
    t_uniLine* uniLine = static_cast<t_uniLine*>(_widget);
    for (int iRow = 0; iRow < uniLine->rowCount(); iRow++) {
      QString rowStr;
      for (int iCol = 0; iCol < uniLine->columnCount()-2; iCol++) {
        QTableWidgetItem* item = uniLine->item(iRow, iCol);
        if (item) {
          if (iCol != 0) rowStr += " \"";
          rowStr += item->text().trimmed();
          if (iCol != uniLine->columnCount()-3) rowStr += '\"';
        }
      }
      values << rowStr;
    }
  }

  return values;
}

