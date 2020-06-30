
#ifndef BNSUTILS_H
#define BNSUTILS_H

#include <newmat.h>

#include <QString>
#include <QDateTime>
#include <QTcpSocket>

class gpsEph;

namespace BNS {
  enum t_irc {failure = -1, success}; // return code
  
  void expandEnvVar(QString& str);
  
  QDateTime dateAndTimeFromGPSweek(int GPSWeek, double GPSWeeks);
  
  void GPSweekFromDateAndTime(const QDateTime& dateTime, 
                              int& GPSWeek, double& GPSWeeks);
  
  void GPSweekFromYMDhms(int year, int month, int day, int hour, int min,
                         double sec, int& GPSWeek, double& GPSWeeks);
  
  void mjdFromDateAndTime(const QDateTime& dateTime, int& mjd, double& dayfrac);
  
  void currentGPSWeeks(int& week, double& sec);
  
  void XYZ_to_RSW(const ColumnVector& rr, const ColumnVector& vv,
                  const ColumnVector& xyz, ColumnVector& rsw);
  
  ColumnVector rungeKutta4(double xi, const ColumnVector& yi, double dx,
                           double* acc,
  	   ColumnVector (*der)(double x, const ColumnVector& y, double* acc));
  
  QByteArray waitForLine(QTcpSocket* socket);
}
#endif
