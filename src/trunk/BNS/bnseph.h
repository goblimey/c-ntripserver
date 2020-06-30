#ifndef BNSEPH_H
#define BNSEPH_H

#include <math.h>
#include <newmat.h>

#include <QtCore>
#include <QThread>
#include <QtNetwork>

#include "bnsutils.h" 

class t_eph {
 public:
  virtual ~t_eph() {};

  bool    isNewerThan(const t_eph* eph) const;
  QString prn() const {return _prn;}
  void    setRecepDateTime(const QDateTime& dateTime) {
    _receptDateTime = dateTime;
  }
  const QDateTime& receptDateTime() const {return _receptDateTime;}

  virtual void position(int GPSweek, double GPSweeks, ColumnVector& xc,
                        ColumnVector& vv) const = 0;
  virtual BNS::t_irc read(const QStringList& lines) = 0;
  virtual int  IOD() const = 0;
  virtual int  RTCM3(unsigned char *) = 0;

 protected:  
  QString   _prn;
  int       _GPSweek;
  double    _GPSweeks;
  QDateTime _receptDateTime;
};

class t_ephGlo : public t_eph {
 public:
  t_ephGlo() {
    _gps_utc = 0.0;
    _xv.ReSize(6); 
  };
  virtual ~t_ephGlo() {};
  virtual BNS::t_irc  read(const QStringList& lines);
  virtual void position(int GPSweek, double GPSweeks, ColumnVector& xc,
                        ColumnVector& vv) const;
  virtual int  IOD() const;
  virtual int  RTCM3(unsigned char *);
 private:
  static ColumnVector glo_deriv(double /* tt */, const ColumnVector& xv,
                                double* acc);
  mutable double       _tt;  // time in seconds of GPSweek
  mutable ColumnVector _xv;  // status vector (position, velocity) at time _tt
  double  _gps_utc;          // GPS - UTC in seconds      

  double _E;                  // [days]   
  double _tau;                // [s]      
  double _gamma;              //          
  double _x_pos;              // [km]     
  double _x_velocity;         // [km/s]   
  double _x_acceleration;     // [km/s^2] 
  double _y_pos;              // [km]     
  double _y_velocity;         // [km/s]   
  double _y_acceleration;     // [km/s^2] 
  double _z_pos;              // [km]     
  double _z_velocity;         // [km/s]   
  double _z_acceleration;     // [km/s^2] 
  double _health;             // 0 = O.K. 
  double _frequency_number;   // ICD-GLONASS data position 
  double _tki;                // message frame time
};


class t_ephGPS : public t_eph {
 public:
  t_ephGPS() {};
  virtual ~t_ephGPS() {};
  virtual BNS::t_irc  read(const QStringList& lines);
  virtual void position(int GPSweek, double GPSweeks, ColumnVector& xc,
                        ColumnVector& vv) const;
  virtual int  IOD() const {return int(_IODE);}
  virtual int  RTCM3(unsigned char *);
 private:
  double  _TOW;              //  [s]    
  double  _TOC;              //  [s]    
  double  _TOE;              //  [s]    
  double  _IODE;             
  double  _IODC;             

  double  _clock_bias;       //  [s]    
  double  _clock_drift;      //  [s/s]  
  double  _clock_driftrate;  //  [s/s^2]

  double  _Crs;              //  [m]    
  double  _Delta_n;          //  [rad/s]
  double  _M0;               //  [rad]  
  double  _Cuc;              //  [rad]  
  double  _e;                //         
  double  _Cus;              //  [rad]  
  double  _sqrt_A;           //  [m^0.5]
  double  _Cic;              //  [rad]  
  double  _OMEGA0;           //  [rad]  
  double  _Cis;              //  [rad]  
  double  _i0;               //  [rad]  
  double  _Crc;              //  [m]    
  double  _omega;            //  [rad]  
  double  _OMEGADOT;         //  [rad/s]
  double  _IDOT;             //  [rad/s]

  double  _TGD;              //  [s]    
  double _health;            //  SV health
  double _ura;               //  SV accuracy
  double _L2PFlag;           //  L2 P data flag
  double _L2Codes;           //  Codes on L2 channel 
};

class t_ephGal : public t_eph {
 public:
  t_ephGal() { }
  virtual ~t_ephGal() {}
  virtual BNS::t_irc read(const QStringList& lines);
  virtual void position(int GPSweek, double GPSweeks, ColumnVector& xc,
                        ColumnVector& vv) const;
  virtual int  IOD() const { return static_cast<int>(_IODnav); }
  virtual int  RTCM3(unsigned char *);

 private:
  double  _IODnav;             
  double  _TOC;              //  [s]    
  double  _TOE;              //  [s]    
  double  _clock_bias;       //  [s]    
  double  _clock_drift;      //  [s/s]  
  double  _clock_driftrate;  //  [s/s^2]
  double  _Crs;              //  [m]    
  double  _Delta_n;          //  [rad/s]
  double  _M0;               //  [rad]  
  double  _Cuc;              //  [rad]  
  double  _e;                //         
  double  _Cus;              //  [rad]  
  double  _sqrt_A;           //  [m^0.5]
  double  _Cic;              //  [rad]  
  double  _OMEGA0;           //  [rad]  
  double  _Cis;              //  [rad]  
  double  _i0;               //  [rad]  
  double  _Crc;              //  [m]    
  double  _omega;            //  [rad]  
  double  _OMEGADOT;         //  [rad/s]
  double  _IDOT;             //  [rad/s]
  double  _BGD_1_5A;         //  group delay [s] 
  double  _BGD_1_5B;         //  group delay [s] 
  int     _SISA;             //  Signal In Space Accuracy
  int     _E5aHS;            //  E5a Health Status

};

class t_bnseph : public QThread {
 Q_OBJECT
 public:
  t_bnseph(QObject* parent = 0);
  virtual ~t_bnseph();  
  virtual void run();  

 signals:
  void newEph(t_eph* eph, int nBytes);
  void newMessage(const QByteArray msg);
  void error(const QByteArray msg);
 
 private:
  void reconnect();
  void readEph();
  QTcpSocket* _socket;
  QFile*                    _echoFile;
  QTextStream*              _echoStream;
};
#endif
