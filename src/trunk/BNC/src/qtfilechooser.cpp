/* -------------------------------------------------------------------------
 * Generally useful widget
 * -------------------------------------------------------------------------
 *
 * Class:      qtFileChooser
 *
 * Purpose:    Widget for selecting a file or directory
 *
 * Author:     L. Mervart
 *
 * Created:    10-May-2011
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "qtfilechooser.h"

// Constructor
////////////////////////////////////////////////////////////////////////////////
qtFileChooser::qtFileChooser(QWidget* parent, qtFileChooser::Mode mode) : 
  QWidget(parent), _mode(mode) {

  QHBoxLayout* layout = new QHBoxLayout( this );
  layout->setMargin(0);

  _lineEdit = new QLineEdit(this);
  layout->addWidget(_lineEdit);

  connect(_lineEdit, SIGNAL(textChanged(const QString &)),
          this, SIGNAL(fileNameChanged(const QString &)));

  _button = new QPushButton("...", this);
  _button->setFixedWidth(_button->fontMetrics().width(" ... "));
  layout->addWidget(_button);

  connect(_button, SIGNAL(clicked()), this, SLOT(chooseFile()));
  setFocusProxy(_lineEdit);
}

// Destructor
////////////////////////////////////////////////////////////////////////////////
qtFileChooser::~qtFileChooser() {
}

// 
////////////////////////////////////////////////////////////////////////////////
void qtFileChooser::setFileName(const QString& fileName) {
  _lineEdit->setText(fileName);
}

// 
////////////////////////////////////////////////////////////////////////////////
QString qtFileChooser::fileName() const {
  return _lineEdit->text();
}

// 
////////////////////////////////////////////////////////////////////////////////
void qtFileChooser::chooseFile() {
  QString fileName;
  if      (mode() == File) {
    fileName = QFileDialog::getOpenFileName(this);
  }
  else if (mode() == Files) {
    QStringList fileNames = QFileDialog::getOpenFileNames(this);
    fileName = fileNames.join(",");
  }
  else {
    fileName = QFileDialog::getExistingDirectory(this);
  }

  if (!fileName.isEmpty()) {
    _lineEdit->setText(fileName);
    emit fileNameChanged(fileName);
  }
}
