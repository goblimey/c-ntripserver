
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_selWin
 *
 * Purpose:    Widget for File/Directory Selection
 *
 * Author:     L. Mervart
 *
 * Created:    08-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "selwin.h"

using namespace std;
using namespace GnssCenter;

// Constructor
////////////////////////////////////////////////////////////////////////////////
t_selWin::t_selWin(t_selWin::Mode mode, QWidget* parent) : QWidget(parent) {

  _mode = mode;

  QHBoxLayout* layout = new QHBoxLayout( this );
  layout->setMargin(0);

  _lineEdit = new QLineEdit(this);
  layout->addWidget(_lineEdit);

  connect(_lineEdit, SIGNAL(textEdited(const QString &)),
          this, SLOT(slotTextEdited()));

  _button = new QPushButton("...", this);
  _button->setFixedWidth(_button->fontMetrics().width(" ... "));
  layout->addWidget(_button);

  connect(_button, SIGNAL(clicked()), this, SLOT(slotChooseFile()));
  setFocusProxy(_lineEdit);
}

// Destructor
////////////////////////////////////////////////////////////////////////////////
t_selWin::~t_selWin() {
}

// 
////////////////////////////////////////////////////////////////////////////////
void t_selWin::setFileNames(const QStringList& fileNames) {
  _fileNames = fileNames;
  setLineEditText();
}

// 
////////////////////////////////////////////////////////////////////////////////
void t_selWin::setLineEditText() {
  if      (_fileNames.size() == 0) {
    _lineEdit->setText("");
  }
  else if (_fileNames.size() == 1) {
    _lineEdit->setText(_fileNames[0]);
  }
  else if (_fileNames.size() > 1) {
    _lineEdit->setText(QString("SELECTED (%1 FILES)").arg(_fileNames.size()));
  }
}

// 
////////////////////////////////////////////////////////////////////////////////
const QStringList& t_selWin::fileNames() const {
  return _fileNames;
}

// 
////////////////////////////////////////////////////////////////////////////////
void t_selWin::slotTextEdited() {
  _fileNames.clear();
  if (!_lineEdit->text().isEmpty()) {
    _fileNames << _lineEdit->text();
  }
  emit fileNamesChanged();
}

// 
////////////////////////////////////////////////////////////////////////////////
void t_selWin::slotChooseFile() {
  if      (mode() == File) {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
      _fileNames.clear();
      _fileNames << fileName;
      emit fileNamesChanged();
   }
  }
  else if (mode() == Files) {
    QStringList fileNames = QFileDialog::getOpenFileNames(this);
    if (fileNames.size()) {
      _fileNames = fileNames;
      emit fileNamesChanged();
    }
  }
  else {
   QString dirName = QFileDialog::getExistingDirectory(this);
    if (!dirName.isEmpty()) {
      _fileNames.clear();
      _fileNames << dirName;
      emit fileNamesChanged();
    }
  }
  setLineEditText();
}

