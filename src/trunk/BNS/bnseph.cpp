/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnseph
 *
 * Purpose:    Retrieve broadcast ephemeris from BNC
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include "bnseph.h" 
#include "bnssettings.h" 
#include "bnctime.h" 
extern "C" {
#include "rtcm3torinex.h"
}

#define PI          3.1415926535898
const static double c_vel = 299792458.0;

using namespace std;
using namespace BNS;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::t_bnseph(QObject* parent) : QThread(parent) {

  this->setTerminationEnabled(true);

  _socket = 0;

  bnsSettings settings;

  QIODevice::OpenMode oMode;
  if (Qt::CheckState(settings.value("fileAppend").toInt()) == Qt::Checked) {
    oMode = QIODevice::WriteOnly | QIODevice::Unbuffered | QIODevice::Append;
  }
  else {
    oMode = QIODevice::WriteOnly | QIODevice::Unbuffered;
  }

  // Echo ephemeris into a file
  // ---------------------------
  QString echoFileName = settings.value("ephEcho").toString();
  if (echoFileName.isEmpty()) {
    _echoFile   = 0;
    _echoStream = 0;
  }
  else {
    _echoFile = new QFile(echoFileName);
    if (_echoFile->open(oMode)) {
      _echoStream = new QTextStream(_echoFile);
    }
    else {
      _echoStream = 0;
    }
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bnseph::~t_bnseph() {
  delete _socket;
  delete _echoStream;
  delete _echoFile;
}

// Connect the Socket
////////////////////////////////////////////////////////////////////////////
void t_bnseph::reconnect() {

  delete _socket;

  bnsSettings settings;
  QString host = settings.value("ephHost").toString();
  if (host.isEmpty()) {
    host = "localhost";
  }
  int     port = settings.value("ephPort").toInt();

  _socket = new QTcpSocket();
  _socket->connectToHost(host, port);

  const int timeOut = 30*1000;  // 30 seconds
  if (!_socket->waitForConnected(timeOut)) {
//  emit(newMessage("bnseph::run Connect Timeout"));
    emit(newMessage("Ephemeris server: Connection timeout")); // weber
    msleep(1000);
  }
}

// Start 
////////////////////////////////////////////////////////////////////////////
void t_bnseph::run() {

//emit(newMessage("bnseph::run Start"));
  emit(newMessage("Ephemeris server: Connection opened")); // weber

  while (true) {
    if ( _socket == 0 ||
         (_socket->state() != QAbstractSocket::ConnectingState &&
          _socket->state() != QAbstractSocket::ConnectedState) ) {
      reconnect();
    }
    if (_socket && _socket->state() == QAbstractSocket::ConnectedState) {
      readEph();
    }
    else {
      msleep(10);
    }
  }
}

// Read One Ephemeris 
////////////////////////////////////////////////////////////////////////////
void t_bnseph::readEph() {

  int nBytes = 0;
  t_eph* eph = 0;

  QByteArray line = waitForLine(_socket);

  if (line.isEmpty()) {
    return;
  }

  if (_echoStream) {
    *_echoStream << line;
    _echoStream->flush();
  }

  nBytes += line.length();

  QTextStream in(line);
  QString     prn;
   
  in >> prn;

  int numlines = 0;    
  if (prn.indexOf('R') != -1) {
    eph = new t_ephGlo();
    numlines = 4;
  }
  else if (prn.indexOf('G') != -1) {
    eph = new t_ephGPS();
    numlines = 8;
  }
  else if (prn.indexOf('E') != -1) {
    eph = new t_ephGal();
    numlines = 8;
  }
  else {
    emit(newMessage(QString("Ephemeris server: line not recognized: %1")
                    .arg(line.data()).toAscii().data()));
    return;
  }

  QStringList lines;
  lines << line;

  for (int ii = 2; ii <= numlines; ii++) {
    QByteArray line = waitForLine(_socket);
    if (line.isEmpty()) {
      delete eph;
      return;
    }

    if (_echoStream) {
      *_echoStream << line;
      _echoStream->flush();
    }

    nBytes += line.length();
    lines << line;
  }
   
  if (eph->read(lines) == success) {
    eph->setRecepDateTime(QDateTime::currentDateTime());
    emit(newEph(eph, nBytes));
  } 
  else {
    emit(newMessage(QString("Broadcast message too new, excluded.\n%1")
                    .arg(lines.join("")).toAscii().data()));
    delete eph;
    return;
  } 
}

// Compare Time
////////////////////////////////////////////////////////////////////////////
bool t_eph::isNewerThan(const t_eph* eph) const {
  if (_GPSweek >  eph->_GPSweek ||
      (_GPSweek == eph->_GPSweek && _GPSweeks > eph->_GPSweeks)) {
    return true;
  }
  else {
    return false;
  }
}

// Read GPS Ephemeris
////////////////////////////////////////////////////////////////////////////
t_irc t_ephGPS::read(const QStringList& lines) {

  for (int ii = 1; ii <= lines.size(); ii++) {
    QTextStream in(lines.at(ii-1).toAscii());

    if (ii == 1) {
      double  year, month, day, hour, minute, second;
      in >> _prn >> year >> month >> day >> hour >> minute >> second
         >> _clock_bias >> _clock_drift >> _clock_driftrate;
      
      if (year < 100) year += 2000;
      
      QDateTime* dateTime = new QDateTime(QDate(int(year), int(month), int(day)), 
                                          QTime(int(hour), int(minute), int(second)), Qt::UTC);

      // do not allow too new message (actually 12h) ! /JD
      QDateTime currTime = QDateTime::currentDateTime();
      if( dateTime->secsTo(currTime) < -3600*12 ){
         delete dateTime;
         return failure; 
      }
       
      GPSweekFromDateAndTime(*dateTime, _GPSweek, _GPSweeks); 

      delete dateTime;

      _TOC = _GPSweeks;
    }
    else if (ii == 2) {
      in >> _IODE >> _Crs >> _Delta_n >> _M0;
    }  
    else if (ii == 3) {
      in >> _Cuc >> _e >> _Cus >> _sqrt_A;
    }
    else if (ii == 4) {
      in >> _TOE >> _Cic >> _OMEGA0 >> _Cis;
    }  
    else if (ii == 5) {
      in >> _i0 >> _Crc >> _omega >> _OMEGADOT;
    }
    else if (ii == 6) {
      double GPSweek;
      in >>  _IDOT >> _L2Codes >> GPSweek >> _L2PFlag;
    }
    else if (ii == 7) {
      in >> _ura >> _health >> _TGD >> _IODC;
    }
    else if (ii == 8) {
      in >> _TOW;
    }
  }
  return success;
}

// Returns nearest integer value
////////////////////////////////////////////////////////////////////////////
static double NearestInt(double fl, double * remain)
{
  bool isneg = fl < 0.0;
  double intval;
  if(isneg) fl *= -1.0;
  intval = (double)((unsigned long)(fl+0.5));
  if(isneg) {fl *= -1.0; intval *= -1.0;}
  if(remain)
    *remain = fl-intval;
  return intval;
} /* NearestInt() */

// Returns CRC24
////////////////////////////////////////////////////////////////////////////
static unsigned long CRC24(long size, const unsigned char *buf)
{
  unsigned long crc = 0;
  int i;

  while(size--)
  {
    crc ^= (*buf++) << (16);
    for(i = 0; i < 8; i++)
    {
      crc <<= 1;
      if(crc & 0x1000000)
        crc ^= 0x01864cfb;
    }
  }
  return crc;
} /* CRC24 */

// build up RTCM3 for GPS
////////////////////////////////////////////////////////////////////////////

#define GPSTOINT(type, value) static_cast<type>(NearestInt(value,0))

#define GPSADDBITS(a, b) {bitbuffer = (bitbuffer<<(a)) \
                       |(GPSTOINT(long long,b)&((1ULL<<a)-1)); \
                       numbits += (a); \
                       while(numbits >= 8) { \
                       buffer[size++] = bitbuffer>>(numbits-8);numbits -= 8;}}
#define GPSADDBITSFLOAT(a,b,c) {long long i = GPSTOINT(long long,(b)/(c)); \
                             GPSADDBITS(a,i)};

int t_ephGPS::RTCM3(unsigned char *buffer)
{

  unsigned char *startbuffer = buffer;
  buffer= buffer+3;
  int size = 0;
  int numbits = 0;
  unsigned long long bitbuffer = 0;
  if (_ura <= 2.40){
    _ura = 0;
  }
  else if (_ura <= 3.40){
    _ura = 1;
  }
  else if (_ura <= 6.85){
    _ura = 2;
  }
  else if (_ura <= 9.65){
    _ura = 3;
  }
  else if (_ura <= 13.65){
    _ura = 4;
  }
  else if (_ura <= 24.00){
    _ura = 5;
  }
  else if (_ura <= 48.00){
    _ura = 6;
  }
  else if (_ura <= 96.00){
    _ura = 7;
  }
  else if (_ura <= 192.00){
    _ura = 8;
  }
  else if (_ura <= 384.00){
    _ura = 9;
  }
  else if (_ura <= 768.00){
    _ura = 10;
  }
  else if (_ura <= 1536.00){
    _ura = 11;
  }
  else if (_ura <= 1536.00){
    _ura = 12;
  }
  else if (_ura <= 2072.00){
    _ura = 13;
  }
  else if (_ura <= 6144.00){
    _ura = 14;
  }
  else{
    _ura = 15;
  }

  GPSADDBITS(12, 1019)
  GPSADDBITS(6,_prn.right((_prn.length()-1)).toInt())
  GPSADDBITS(10, _GPSweek)
  GPSADDBITS(4, _ura)
  GPSADDBITS(2,_L2Codes)
  GPSADDBITSFLOAT(14, _IDOT, PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GPSADDBITS(8, _IODE)
  GPSADDBITS(16, static_cast<int>(_TOC)>>4)
  GPSADDBITSFLOAT(8, _clock_driftrate, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<25))
  GPSADDBITSFLOAT(16, _clock_drift, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GPSADDBITSFLOAT(22, _clock_bias, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<1))
  GPSADDBITS(10, _IODC)
  GPSADDBITSFLOAT(16, _Crs, 1.0/static_cast<double>(1<<5))
  GPSADDBITSFLOAT(16, _Delta_n, PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GPSADDBITSFLOAT(32, _M0, PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  GPSADDBITSFLOAT(16, _Cuc, 1.0/static_cast<double>(1<<29))
  GPSADDBITSFLOAT(32, _e, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<3))
  GPSADDBITSFLOAT(16, _Cus, 1.0/static_cast<double>(1<<29))
  GPSADDBITSFLOAT(32, _sqrt_A, 1.0/static_cast<double>(1<<19))
  GPSADDBITS(16, static_cast<int>(_TOE)>>4)
  GPSADDBITSFLOAT(16, _Cic, 1.0/static_cast<double>(1<<29))
  GPSADDBITSFLOAT(32, _OMEGA0, PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<1))
  GPSADDBITSFLOAT(16, _Cis, 1.0/static_cast<double>(1<<29))
  GPSADDBITSFLOAT(32, _i0, PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  GPSADDBITSFLOAT(16, _Crc, 1.0/static_cast<double>(1<<5))
  GPSADDBITSFLOAT(32, _omega, PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<1))
  GPSADDBITSFLOAT(24, _OMEGADOT, PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GPSADDBITSFLOAT(8, _TGD, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  GPSADDBITS(6, _health) 
  GPSADDBITS(1, _L2PFlag)
  GPSADDBITS(1, 0) /* GPS fit interval */

  startbuffer[0]=0xD3;
  startbuffer[1]=(size >> 8);
  startbuffer[2]=size;
  unsigned long  i = CRC24(size+3, startbuffer);
  buffer[size++] = i >> 16;
  buffer[size++] = i >> 8;
  buffer[size++] = i;
  size += 3;
  return size;
}

// Compute GPS Satellite Position
////////////////////////////////////////////////////////////////////////////
void t_ephGPS::position(int GPSweek, double GPSweeks, ColumnVector& xc,
                        ColumnVector& vv) const {

  static const double secPerWeek = 7 * 86400.0;
  static const double omegaEarth = 7292115.1467e-11;
  static const double gmWGS      = 398.6005e12;

  if (xc.Nrows() < 4) {
    xc.ReSize(4);
  }
  xc = 0.0;

  if (vv.Nrows() < 3) {
    vv.ReSize(3);
  }
  vv = 0.0;

  double a0 = _sqrt_A * _sqrt_A;
  if (a0 == 0) {
    return;
  }

  double n0 = sqrt(gmWGS/(a0*a0*a0));
  double tk = GPSweeks - _TOE;
  if (GPSweek != _GPSweek) {  
    tk += (GPSweek - _GPSweek) * secPerWeek;
  }
  double n  = n0 + _Delta_n;
  double M  = _M0 + n*tk;
  double E  = M;
  double E_last;
  do {
    E_last = E;
    E = M + _e*sin(E);
  } while ( fabs(E-E_last)*a0 > 0.001 );
  double v      = 2.0*atan( sqrt( (1.0 + _e)/(1.0 - _e) )*tan( E/2 ) );
  double u0     = v + _omega;
  double sin2u0 = sin(2*u0);
  double cos2u0 = cos(2*u0);
  double r      = a0*(1 - _e*cos(E)) + _Crc*cos2u0 + _Crs*sin2u0;
  double i      = _i0 + _IDOT*tk + _Cic*cos2u0 + _Cis*sin2u0;
  double u      = u0 + _Cuc*cos2u0 + _Cus*sin2u0;
  double xp     = r*cos(u);
  double yp     = r*sin(u);
  double OM     = _OMEGA0 + (_OMEGADOT - omegaEarth)*tk - 
                   omegaEarth*_TOE;
  
  double sinom = sin(OM);
  double cosom = cos(OM);
  double sini  = sin(i);
  double cosi  = cos(i);
  xc(1) = xp*cosom - yp*cosi*sinom;
  xc(2) = xp*sinom + yp*cosi*cosom;
  xc(3) = yp*sini;                 
  
  double tc = GPSweeks - _TOC;
  if (GPSweek != _GPSweek) {  
    tc += (GPSweek - _GPSweek) * secPerWeek;
  }
  xc(4) = _clock_bias + _clock_drift*tc + _clock_driftrate*tc*tc;

  // Velocity
  // --------
  double tanv2 = tan(v/2);
  double dEdM  = 1 / (1 - _e*cos(E));
  double dotv  = sqrt((1.0 + _e)/(1.0 - _e)) / cos(E/2)/cos(E/2) / (1 + tanv2*tanv2) 
               * dEdM * n;
  double dotu  = dotv + (-_Cuc*sin2u0 + _Cus*cos2u0)*2*dotv;
  double dotom = _OMEGADOT - omegaEarth;
  double doti  = _IDOT + (-_Cic*sin2u0 + _Cis*cos2u0)*2*dotv;
  double dotr  = a0 * _e*sin(E) * dEdM * n 
                + (-_Crc*sin2u0 + _Crs*cos2u0)*2*dotv;
  double dotx  = dotr*cos(u) - r*sin(u)*dotu;
  double doty  = dotr*sin(u) + r*cos(u)*dotu;

  vv(1)  = cosom   *dotx  - cosi*sinom   *doty      // dX / dr
           - xp*sinom*dotom - yp*cosi*cosom*dotom   // dX / dOMEGA
                       + yp*sini*sinom*doti;        // dX / di

  vv(2)  = sinom   *dotx  + cosi*cosom   *doty
           + xp*cosom*dotom - yp*cosi*sinom*dotom
                          - yp*sini*cosom*doti;

  vv(3)  = sini    *doty  + yp*cosi      *doti;

  // Relativistic Correction
  // -----------------------
  //  xc(4) -= 4.442807633e-10 * _e * sqrt(a0) *sin(E);
  xc(4) -= 2.0 * DotProduct(xc.Rows(1,3),vv) / c_vel / c_vel;
}

// Read Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
t_irc t_ephGlo::read(const QStringList& lines) {

  static const double secPerWeek = 7 * 86400.0;

  for (int ii = 1; ii <= lines.size(); ii++) {
    QTextStream in(lines.at(ii-1).toAscii());

    if (ii == 1) {
      double  year, month, day, hour, minute, second;
      in >> _prn >> year >> month >> day >> hour >> minute >> second
         >> _tau >> _gamma >> _tki;

      _tau = -_tau;
      
      if (year < 100) year += 2000;

      // do not allow too new message (actually 24h) ! /JD
      QDateTime mesgTime = QDateTime::fromString(QString("%1-%2-%3 %4").arg(year).arg(month).arg(day).arg(hour), "yyyy-MM-dd hh");
      QDateTime currTime = QDateTime::currentDateTime();
      if (mesgTime.secsTo(currTime) < -3600*24) {
        return failure;
      }
       
      bncTime tHlp; 
      tHlp.set(int(year), int(month), int(day), 
               int(hour), int(minute), second);
      
      _GPSweek  = tHlp.gpsw();
      _GPSweeks = tHlp.gpssec();

      // Correct UTC -> GPS;
      // -------------------
      _gps_utc = gnumleap(int(year), int(month), int(day));
      _GPSweeks += _gps_utc;
      if (_GPSweeks >= secPerWeek) {
        _GPSweek  += 1;
        _GPSweeks -= secPerWeek;
      }
    }
    else if (ii == 2) {
      in >>_x_pos >> _x_velocity >> _x_acceleration >> _health;
    }
    else if (ii == 3) {
      in >>_y_pos >> _y_velocity >> _y_acceleration >> _frequency_number;
    }
    else if (ii == 4) {
      in >>_z_pos >> _z_velocity >> _z_acceleration >> _E;
    }
  }

  // Initialize status vector
  // ------------------------
  _tt = _GPSweeks;

  _xv(1) = _x_pos * 1.e3; 
  _xv(2) = _y_pos * 1.e3; 
  _xv(3) = _z_pos * 1.e3; 
  _xv(4) = _x_velocity * 1.e3; 
  _xv(5) = _y_velocity * 1.e3; 
  _xv(6) = _z_velocity * 1.e3; 

  return success;
}


// build up RTCM3 for GLONASS
////////////////////////////////////////////////////////////////////////////
#define GLONASSTOINT(type, value) static_cast<type>(NearestInt(value,0))

#define GLONASSADDBITS(a, b) {bitbuffer = (bitbuffer<<(a)) \
                       |(GLONASSTOINT(long long,b)&((1ULL<<(a))-1)); \
                       numbits += (a); \
                       while(numbits >= 8) { \
                       buffer[size++] = bitbuffer>>(numbits-8);numbits -= 8;}}
#define GLONASSADDBITSFLOATM(a,b,c) {int s; long long i; \
                       if(b < 0.0) \
                       { \
                         s = 1; \
                         i = GLONASSTOINT(long long,(-b)/(c)); \
                         if(!i) s = 0; \
                       } \
                       else \
                       { \
                         s = 0; \
                         i = GLONASSTOINT(long long,(b)/(c)); \
                       } \
                       GLONASSADDBITS(1,s) \
                       GLONASSADDBITS(a-1,i)}

int t_ephGlo::RTCM3(unsigned char *buffer)
{

  int size = 0;
  int numbits = 0;
  long long bitbuffer = 0;
  unsigned char *startbuffer = buffer;
  buffer= buffer+3;

  GLONASSADDBITS(12, 1020)
  GLONASSADDBITS(6, _prn.right((_prn.length()-1)).toInt())
  GLONASSADDBITS(5, 7+_frequency_number)
  GLONASSADDBITS(1, 0)
  GLONASSADDBITS(1, 0)
  GLONASSADDBITS(2, 0)
  _tki=_tki+3*60*60;
  GLONASSADDBITS(5, static_cast<int>(_tki)/(60*60))
  GLONASSADDBITS(6, (static_cast<int>(_tki)/60)%60)
  GLONASSADDBITS(1, (static_cast<int>(_tki)/30)%30)
  GLONASSADDBITS(1, _health) 
  GLONASSADDBITS(1, 0)
  unsigned long long timeofday = (static_cast<int>(_tt+3*60*60-_gps_utc)%86400);
  GLONASSADDBITS(7, timeofday/60/15)
  GLONASSADDBITSFLOATM(24, _x_velocity*1000, 1000.0/static_cast<double>(1<<20))
  GLONASSADDBITSFLOATM(27, _x_pos*1000, 1000.0/static_cast<double>(1<<11))
  GLONASSADDBITSFLOATM(5, _x_acceleration*1000, 1000.0/static_cast<double>(1<<30))
  GLONASSADDBITSFLOATM(24, _y_velocity*1000, 1000.0/static_cast<double>(1<<20))
  GLONASSADDBITSFLOATM(27, _y_pos*1000, 1000.0/static_cast<double>(1<<11))
  GLONASSADDBITSFLOATM(5, _y_acceleration*1000, 1000.0/static_cast<double>(1<<30))
  GLONASSADDBITSFLOATM(24, _z_velocity*1000, 1000.0/static_cast<double>(1<<20))
  GLONASSADDBITSFLOATM(27,_z_pos*1000, 1000.0/static_cast<double>(1<<11))
  GLONASSADDBITSFLOATM(5, _z_acceleration*1000, 1000.0/static_cast<double>(1<<30))
  GLONASSADDBITS(1, 0)
  GLONASSADDBITSFLOATM(11, _gamma, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<10))
  GLONASSADDBITS(2, 0) /* GLONASS-M P */
  GLONASSADDBITS(1, 0) /* GLONASS-M ln(3) */
  GLONASSADDBITSFLOATM(22, _tau, 1.0/static_cast<double>(1<<30))
  GLONASSADDBITS(5, 0) /* GLONASS-M delta tau */
  GLONASSADDBITS(5, _E)
  GLONASSADDBITS(1, 0) /* GLONASS-M P4 */
  GLONASSADDBITS(4, 0) /* GLONASS-M FT */
  GLONASSADDBITS(11, 0) /* GLONASS-M NT */
  GLONASSADDBITS(2, 0) /* GLONASS-M active? */
  GLONASSADDBITS(1, 0) /* GLONASS additional data */
  GLONASSADDBITS(11, 0) /* GLONASS NA */
  GLONASSADDBITS(32, 0) /* GLONASS tau C */
  GLONASSADDBITS(5, 0) /* GLONASS-M N4 */
  GLONASSADDBITS(22, 0) /* GLONASS-M tau GPS */
  GLONASSADDBITS(1, 0) /* GLONASS-M ln(5) */
  GLONASSADDBITS(7, 0) /* Reserved */

  startbuffer[0]=0xD3;
  startbuffer[1]=(size >> 8);
  startbuffer[2]=size;
  unsigned long i = CRC24(size+3, startbuffer);
  buffer[size++] = i >> 16;
  buffer[size++] = i >> 8;
  buffer[size++] = i;
  size += 3;
  return size;
}

// Derivative of the state vector using a simple force model (static)
////////////////////////////////////////////////////////////////////////////
ColumnVector t_ephGlo::glo_deriv(double /* tt */, const ColumnVector& xv,
                                 double* acc) {

  // State vector components
  // -----------------------
  ColumnVector rr = xv.rows(1,3);
  ColumnVector vv = xv.rows(4,6);

  // Acceleration 
  // ------------
  static const double GM    = 398.60044e12;
  static const double AE    = 6378136.0;
  static const double OMEGA = 7292115.e-11;
  static const double C20   = -1082.6257e-6;

  double rho = rr.norm_Frobenius();
  double t1  = -GM/(rho*rho*rho);
  double t2  = 3.0/2.0 * C20 * (GM*AE*AE) / (rho*rho*rho*rho*rho);
  double t3  = OMEGA * OMEGA;
  double t4  = 2.0 * OMEGA;
  double z2  = rr(3) * rr(3);

  // Vector of derivatives
  // ---------------------
  ColumnVector va(6);
  va(1) = vv(1);
  va(2) = vv(2);
  va(3) = vv(3);
  va(4) = (t1 + t2*(1.0-5.0*z2/(rho*rho)) + t3) * rr(1) + t4*vv(2) + acc[0]; 
  va(5) = (t1 + t2*(1.0-5.0*z2/(rho*rho)) + t3) * rr(2) - t4*vv(1) + acc[1]; 
  va(6) = (t1 + t2*(3.0-5.0*z2/(rho*rho))     ) * rr(3)            + acc[2];

  return va;
}

// Compute Glonass Satellite Position
////////////////////////////////////////////////////////////////////////////
void t_ephGlo::position(int GPSweek, double GPSweeks, ColumnVector& xc,
                        ColumnVector& vv) const {

  static const double secPerWeek  = 7 * 86400.0;
  static const double nominalStep = 10.0;

  double dtPos = GPSweeks - _tt;
  if (GPSweek != _GPSweek) {  
    dtPos += (GPSweek - _GPSweek) * secPerWeek;
  }

  int nSteps  = int(fabs(dtPos) / nominalStep) + 1;
  double step = dtPos / nSteps;

  double acc[3];
  acc[0] = _x_acceleration * 1.e3;
  acc[1] = _y_acceleration * 1.e3;
  acc[2] = _z_acceleration * 1.e3;
  for (int ii = 1; ii <= nSteps; ii++) { 
    _xv = rungeKutta4(_tt, _xv, step, acc, glo_deriv);
    _tt += step;
  }

  // Position and Velocity
  // ---------------------
  xc(1) = _xv(1);
  xc(2) = _xv(2);
  xc(3) = _xv(3);

  vv(1) = _xv(4);
  vv(2) = _xv(5);
  vv(3) = _xv(6);

  // Clock Correction
  // ----------------
  double dtClk = GPSweeks - _GPSweeks;
  if (GPSweek != _GPSweek) {  
    dtClk += (GPSweek - _GPSweek) * secPerWeek;
  }
  xc(4) = -_tau + _gamma * dtClk;
}

// Glonass IOD
////////////////////////////////////////////////////////////////////////////
int t_ephGlo::IOD() const {

  bool old = false;

  if (old) { // 5 LSBs of iod are equal to 5 LSBs of tb
    unsigned int tb  = int(fmod(_GPSweeks,86400.0)); //sec of day
    const int shift = sizeof(tb) * 8 - 5;
    unsigned int iod = tb << shift;
    return (iod >> shift);
  }
  else     {  
    bncTime tGPS(_GPSweek, _GPSweeks);
    int hlpWeek = _GPSweek;
    int hlpSec  = int(_GPSweeks);
    int hlpMsec = int(_GPSweeks * 1000);
    updatetime(&hlpWeek, &hlpSec, hlpMsec, 0);
    bncTime tHlp(hlpWeek, hlpSec);
    double diffSec = tGPS - tHlp;
    bncTime tMoscow = tGPS + diffSec;
    return int(tMoscow.daysec() / 900);
  }
}

// Compute Galileo Satellite Position
////////////////////////////////////////////////////////////////////////////
void t_ephGal::position(int GPSweek, double GPSweeks, ColumnVector& xc,
                        ColumnVector& vv) const {

  static const double secPerWeek = 7 * 86400.0;
  static const double omegaEarth = 7292115.1467e-11;
  static const double gmWGS      = 398.6005e12;

  xc.ReSize(4); xc = 0.0;
  vv.ReSize(3); vv = 0.0;

  double a0 = _sqrt_A * _sqrt_A;
  if (a0 == 0) {
    return;
  }

  double n0 = sqrt(gmWGS/(a0*a0*a0));
  double tk = GPSweeks - _TOE;
  if (GPSweek != _GPSweek) {  
    tk += (GPSweek - _GPSweek) * secPerWeek;
  }
  double n  = n0 + _Delta_n;
  double M  = _M0 + n*tk;
  double E  = M;
  double E_last;
  do {
    E_last = E;
    E = M + _e*sin(E);
  } while ( fabs(E-E_last)*a0 > 0.001 );
  double v      = 2.0*atan( sqrt( (1.0 + _e)/(1.0 - _e) )*tan( E/2 ) );
  double u0     = v + _omega;
  double sin2u0 = sin(2*u0);
  double cos2u0 = cos(2*u0);
  double r      = a0*(1 - _e*cos(E)) + _Crc*cos2u0 + _Crs*sin2u0;
  double i      = _i0 + _IDOT*tk + _Cic*cos2u0 + _Cis*sin2u0;
  double u      = u0 + _Cuc*cos2u0 + _Cus*sin2u0;
  double xp     = r*cos(u);
  double yp     = r*sin(u);
  double OM     = _OMEGA0 + (_OMEGADOT - omegaEarth)*tk - 
                   omegaEarth*_TOE;
  
  double sinom = sin(OM);
  double cosom = cos(OM);
  double sini  = sin(i);
  double cosi  = cos(i);
  xc[0] = xp*cosom - yp*cosi*sinom;
  xc[1] = xp*sinom + yp*cosi*cosom;
  xc[2] = yp*sini;                 
  
  double tc = GPSweeks - _TOC;
  if (GPSweek != _GPSweek) {  
    tc += (GPSweek - _GPSweek) * secPerWeek;
  }
  xc[3] = _clock_bias + _clock_drift*tc + _clock_driftrate*tc*tc;

  // Velocity
  // --------
  double tanv2 = tan(v/2);
  double dEdM  = 1 / (1 - _e*cos(E));
  double dotv  = sqrt((1.0 + _e)/(1.0 - _e)) / cos(E/2)/cos(E/2) / (1 + tanv2*tanv2) 
               * dEdM * n;
  double dotu  = dotv + (-_Cuc*sin2u0 + _Cus*cos2u0)*2*dotv;
  double dotom = _OMEGADOT - omegaEarth;
  double doti  = _IDOT + (-_Cic*sin2u0 + _Cis*cos2u0)*2*dotv;
  double dotr  = a0 * _e*sin(E) * dEdM * n 
                + (-_Crc*sin2u0 + _Crs*cos2u0)*2*dotv;
  double dotx  = dotr*cos(u) - r*sin(u)*dotu;
  double doty  = dotr*sin(u) + r*cos(u)*dotu;

  vv[0]  = cosom   *dotx  - cosi*sinom   *doty      // dX / dr
           - xp*sinom*dotom - yp*cosi*cosom*dotom   // dX / dOMEGA
                       + yp*sini*sinom*doti;        // dX / di

  vv[1]  = sinom   *dotx  + cosi*cosom   *doty
           + xp*cosom*dotom - yp*cosi*sinom*dotom
                          - yp*sini*cosom*doti;

  vv[2]  = sini    *doty  + yp*cosi      *doti;

  // Relativistic Correction
  // -----------------------
  //  xc(4) -= 4.442807633e-10 * _e * sqrt(a0) *sin(E);
  xc[3] -= 2.0 * (xc[0]*vv[0] + xc[1]*vv[1] + xc[2]*vv[2]) / c_vel / c_vel;
}

// Read Galileo Ephemeris
////////////////////////////////////////////////////////////////////////////
t_irc t_ephGal::read(const QStringList& lines) {

  for (int ii = 1; ii <= lines.size(); ii++) {
    QTextStream in(lines.at(ii-1).toAscii());

    if (ii == 1) {
      double  year, month, day, hour, minute, second;
      in >> _prn >> year >> month >> day >> hour >> minute >> second
         >> _clock_bias >> _clock_drift >> _clock_driftrate;
      
      if (year < 100) year += 2000;
      
      QDateTime* dateTime = new QDateTime(QDate(int(year), int(month), int(day)), 
                                          QTime(int(hour), int(minute), int(second)), Qt::UTC);

      // do not allow too new message (actually 12h) ! /JD
      QDateTime currTime = QDateTime::currentDateTime();
      if( dateTime->secsTo(currTime) < -3600*12 ){
         delete dateTime;
         return failure; 
      }
       
      GPSweekFromDateAndTime(*dateTime, _GPSweek, _GPSweeks); 

      delete dateTime;

      _TOC = _GPSweeks;
    }
    else if (ii == 2) {
      in >> _IODnav >> _Crs >> _Delta_n >> _M0;
    }  
    else if (ii == 3) {
      in >> _Cuc >> _e >> _Cus >> _sqrt_A;
    }
    else if (ii == 4) {
      in >> _TOE >> _Cic >> _OMEGA0 >> _Cis;
    }  
    else if (ii == 5) {
      in >> _i0 >> _Crc >> _omega >> _OMEGADOT;
    }
    else if (ii == 6) {
      double GPSweek, dummy;
      in >>  _IDOT >> dummy >> GPSweek;
    }
    else if (ii == 7) {
      in >> _SISA >> _E5aHS >> _BGD_1_5A >> _BGD_1_5B;
    }
    else if (ii == 8) {
      double TOW;
      in >> TOW;
    }
  }
  return success;
}

#define GALILEOTOINT(type, value) static_cast<type>(NearestInt(value, 0))

#define GALILEOADDBITS(a, b) {bitbuffer = (bitbuffer<<(a)) \
                       |(GALILEOTOINT(long long,b)&((1LL<<a)-1)); \
                       numbits += (a); \
                       while(numbits >= 8) { \
                       buffer[size++] = bitbuffer>>(numbits-8);numbits -= 8;}}
#define GALILEOADDBITSFLOAT(a,b,c) {long long i = GALILEOTOINT(long long,(b)/(c)); \
                             GALILEOADDBITS(a,i)};
// build up RTCM3 for Galileo
////////////////////////////////////////////////////////////////////////////
int t_ephGal::RTCM3(unsigned char *buffer) {
  int size = 0;
  int numbits = 0;
  long long bitbuffer = 0;
  unsigned char *startbuffer = buffer;
  buffer= buffer+3;

  GALILEOADDBITS(12, /*inav ? 1046 :*/ 1045)
  GALILEOADDBITS(6, _prn.right((_prn.length()-1)).toInt())
  GALILEOADDBITS(12, _GPSweek)
  GALILEOADDBITS(10, _IODnav)
  GALILEOADDBITS(8, _SISA)
  GALILEOADDBITSFLOAT(14, _IDOT, PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GALILEOADDBITS(14, _TOC/60)
  GALILEOADDBITSFLOAT(6, _clock_driftrate, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<29))
  GALILEOADDBITSFLOAT(21, _clock_drift, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<16))
  GALILEOADDBITSFLOAT(31, _clock_bias, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<4))
  GALILEOADDBITSFLOAT(16, _Crs, 1.0/static_cast<double>(1<<5))
  GALILEOADDBITSFLOAT(16, _Delta_n, PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GALILEOADDBITSFLOAT(32, _M0, PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  GALILEOADDBITSFLOAT(16, _Cuc, 1.0/static_cast<double>(1<<29))
  GALILEOADDBITSFLOAT(32, _e, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<3))
  GALILEOADDBITSFLOAT(16, _Cus, 1.0/static_cast<double>(1<<29))
  GALILEOADDBITSFLOAT(32, _sqrt_A, 1.0/static_cast<double>(1<<19))
  GALILEOADDBITS(14, _TOE/60)
  GALILEOADDBITSFLOAT(16, _Cic, 1.0/static_cast<double>(1<<29))
  GALILEOADDBITSFLOAT(32, _OMEGA0, PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<1))
  GALILEOADDBITSFLOAT(16, _Cis, 1.0/static_cast<double>(1<<29))
  GALILEOADDBITSFLOAT(32, _i0, PI/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  GALILEOADDBITSFLOAT(16, _Crc, 1.0/static_cast<double>(1<<5))
  GALILEOADDBITSFLOAT(32, _omega, PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<1))
  GALILEOADDBITSFLOAT(24, _OMEGADOT, PI/static_cast<double>(1<<30)
  /static_cast<double>(1<<13))
  GALILEOADDBITSFLOAT(10, _BGD_1_5A, 1.0/static_cast<double>(1<<30)
  /static_cast<double>(1<<2))
  /*if(inav)
  {
    GALILEOADDBITSFLOAT(10, _BGD_1_5B, 1.0/static_cast<double>(1<<30)
    /static_cast<double>(1<<2))
    GALILEOADDBITS(2, _E5bHS)
    GALILEOADDBITS(1, flags & MNFGALEPHF_E5BDINVALID)
  }
  else*/
  {
    GALILEOADDBITS(2, _E5aHS)
    GALILEOADDBITS(1, /*flags & MNFGALEPHF_E5ADINVALID*/0)
  }
  _TOE = 0.9999E9;
  GALILEOADDBITS(20, _TOE)

  GALILEOADDBITS(/*inav ? 1 :*/ 3, 0) /* fill up */

  startbuffer[0]=0xD3;
  startbuffer[1]=(size >> 8);
  startbuffer[2]=size;
  unsigned long i = CRC24(size+3, startbuffer);
  buffer[size++] = i >> 16;
  buffer[size++] = i >> 8;
  buffer[size++] = i;
  size += 3;
  return size;
}
