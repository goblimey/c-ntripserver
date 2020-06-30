/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_settings
 *
 * Purpose:    Subclasses the QSettings
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <QSettings>

#include "settings.h"
#include "app.h"

using namespace GnssCenter;

QMutex t_settings::_mutex;  // static mutex 

// Constructor
////////////////////////////////////////////////////////////////////////////
t_settings::t_settings(const QString& groupName) {
  QMutexLocker locker(&_mutex);

  _groupName = groupName;
  _app       = static_cast<t_app*>(qApp);

  // First fill the options
  // ---------------------- 
  if (_app->_settings.size() == 0) {
    reRead();
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_settings::~t_settings() {
}

// (Re-)read the Options from File or Set the Defaults
////////////////////////////////////////////////////////////////////////////
void t_settings::reRead() {

  _app->_settings.clear();

  QSettings settings(_app->confFileName(), QSettings::IniFormat);

  // Read from File
  // --------------
  if (settings.allKeys().size() > 0) {
    QStringListIterator it(settings.allKeys());
    while (it.hasNext()) {
      QString key = it.next();
      _app->_settings[key] = settings.value(key);
    }
  }

  // Set Defaults
  // ------------
  else {

  }
}

// 
////////////////////////////////////////////////////////////////////////////
QVariant t_settings::value(const QString& key,
                            const QVariant& defaultValue) const {
  QMutexLocker locker(&_mutex);
  QString fullKey = _groupName.isEmpty() ? key : _groupName + '/' + key;
  if (_app->_settings.contains(fullKey)) {
    return _app->_settings[fullKey];
  }
  else {
    return defaultValue;
  }
}

// 
////////////////////////////////////////////////////////////////////////////
void t_settings::setValue(const QString &key, const QVariant& value) {
  QMutexLocker locker(&_mutex);
  QString fullKey = _groupName.isEmpty() ? key : _groupName + '/' + key;
  _app->_settings[fullKey] = value;
}

// 
////////////////////////////////////////////////////////////////////////////
void t_settings::remove(const QString& key ) {
  QMutexLocker locker(&_mutex);
  QString fullKey = _groupName.isEmpty() ? key : _groupName + '/' + key;
  _app->_settings.remove(fullKey);
}

// 
////////////////////////////////////////////////////////////////////////////
void t_settings::sync() {
  QMutexLocker locker(&_mutex);
  QSettings settings(_app->confFileName(), QSettings::IniFormat);
  settings.clear();
  QMapIterator<QString, QVariant> it(_app->_settings);
  while (it.hasNext()) {
    it.next();
    settings.setValue(it.key(), it.value());
  }
  settings.sync();
}
