
#include <QtCore>

#include <newmatio.h>

#include "bnseph.h"
#include "bnsutils.h"

using namespace std;

// Main Program
/////////////////////////////////////////////////////////////////////////////
int main(int /* argc */, char** /* argv */) {

  QStringList lines;

  lines << " 4 08  5  9  0 15  0.0 0.512236729264E-04 0.909494701773E-12 0.780000000000E+03"
        << "   -0.540624853516E+04 0.384947776794E+00-0.186264514923E-08 0.000000000000E+00"
        << "   -0.231976684570E+05 0.119731616974E+01 0.000000000000E+00 0.600000000000E+01"
        << "   -0.928076904297E+04-0.325608444214E+01 0.279396772385E-08 0.100000000000E+01";

  t_ephGlo eph;
  
  eph.read(lines);

  ColumnVector xc(4);
  ColumnVector vv(3);

  int    GPSWeek;
  double GPSWeeks;
  QDateTime dateTime(QDate(2008, 5, 9), QTime(0, 0, 0, 0), Qt::UTC); 
  GPSweekFromDateAndTime(dateTime, GPSWeek, GPSWeeks);

  eph.position(GPSWeek, GPSWeeks, xc, vv);

  cout << setprecision(3) << xc.Rows(1,3);
  cout << setprecision(9) << xc(4) * 1e6 << endl;

  return 0;
} 

