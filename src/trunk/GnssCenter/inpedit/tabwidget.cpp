
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_tabWidget
 *
 * Purpose:    RTNet Input File
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "tabwidget.h" 
#include "keyword.h" 
#include "panel.h" 

using namespace std;
using namespace GnssCenter;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_tabWidget::t_tabWidget() : QTabWidget() {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_tabWidget::~t_tabWidget() {
  QMapIterator<QString, t_keyword*> it(_keywords); 
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_tabWidget::readInputFile(const QString& fileName) {

  _fileName = fileName;

  _staticLines.clear();

  QFile file(_fileName);
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextStream inStream(&file);

  int iPanel = 0;

  while (inStream.status() == QTextStream::Ok && !inStream.atEnd()) {
    QString line = inStream.readLine().trimmed();

    // Skip Comments and empty Lines
    // -----------------------------
    if      (line.isEmpty() || line[0] == '!') {
      _staticLines << line;
      continue;
    }

    // Read Panels
    // -----------
    else if (line[0] == '#' && line.indexOf("BEGIN_PANEL") != -1) {
      t_panel* panel = new t_panel(line, inStream, &_keywords, _staticLines);
      if (panel->ok()) {
        ++iPanel;
        addTab(panel, QString("Panel %1").arg(iPanel));
      }
      else {
        delete panel;
      }
    }

    // Read Keywords
    // -------------
    else {
      t_keyword* keyword = new t_keyword(line, inStream, _staticLines);
      if (keyword->ok()) {
        _keywords[keyword->name()] = keyword;
      }
      else {
        delete keyword;
      }
    }
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_tabWidget::writeInputFile(const QString& fileName) {
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return;
  }
  QTextStream out(&file);
  for (int ii = 0; ii < _staticLines.size(); ii++) {
    QString tLine = _staticLines[ii].trimmed();
    if (tLine.isEmpty() || tLine[0] == '!' || tLine[0] == '#') {
      out << _staticLines[ii] << '\n';
    }
    else {
      if (_keywords.contains(tLine)) {
        const t_keyword* keyword = _keywords[tLine];
        if (keyword) {
          const QStringList& values = keyword->values();
          out << tLine << ' ' << values.size();
          if (values.size() != 1) {
            out << '\n';
          }
          for (int ii = 0; ii < values.size(); ii++) {
            out << "  \"" << values[ii] << "\"\n"; 
          }
        }
      }
    }
  }
}
