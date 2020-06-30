/* -----------------------------------------------------------------------------
 *
 * Function   :  djul
 *
 * Purpose    :  computes the modified julian date (mjd) from 
 *               year, month and day
 *
 * Author     :  Z. Lukes
 *
 * Created    :  13-OCT-2001
 *
 * Changes    :
 *
 * ---------------------------------------------------------------------------*/

#include <math.h>

#ifndef NO_CVS_HEADER
static const char *const cvsid = "$Header: /usr/local/cvsroot/BNC/RTCM3/timeutils.cpp,v 1.1 2008/08/07 14:05:11 zdenek Exp $";
#endif

double djul(long jj, long mm, double tt) {
  long    ii, kk;
  double  djul ;

  if( mm <= 2 ) {
    jj = jj - 1;
    mm = mm + 12;
  }  
  
  ii   = jj/100;
  kk   = 2 - ii + ii/4;
  djul = (365.25*jj - fmod( 365.25*jj, 1.0 )) - 679006.0;
  djul = djul + floor( 30.6001*(mm + 1) ) + tt + kk;
  return djul;
} 

/* -----------------------------------------------------------------------------
 *
 * Function   :  gpjd
 *
 * Purpose    :  computes the modified julian date (mjd) from 
 *               gpsweek number and number of seconds past last
 *               saturday/sunday midnight
 *
 * Author     :  Z. Lukes
 *
 * Created    :  13-OCT-2001
 *
 * Changes    :
 *
 * ---------------------------------------------------------------------------*/

double gpjd(double second, int nweek) {
  double deltat;

  // days since starting epoch of gps weeks (sunday 06-jan-80)
  
  deltat = nweek*7.0 + second/86400.0 ;

  // mod. julian date
  
  return( 44244.0 + deltat) ;
} 

/* -----------------------------------------------------------------------------
 *
 * Function   :  jdgp
 *
 * Purpose    :  compute number of seconds past midnight of last 
 *               saturday/sunday and gps week number of current  
 *		 date given in modified julian date
 *
 * Author     :  Z. Lukes
 *
 * Created    :  13-OCT-2001
 *
 * Changes    :
 *
 * ---------------------------------------------------------------------------*/

void jdgp(double tjul, double & second, long & nweek) {
  double      deltat;

  deltat = tjul - 44244.0 ;

  // current gps week

  nweek = (long) floor(deltat/7.0);

  // seconds past midnight of last weekend

  second = (deltat - (nweek)*7.0)*86400.0;

}

/* -----------------------------------------------------------------------------
 *
 * Function   :  djul
 *
 * Purpose    :  compute year,month,day of month from          
 *		 modified julian date (mjd=jul. date-2400000.5)
 *
 * Author     :  Z. Lukes
 *
 * Created    :  13-OCT-2001
 *
 * Changes    :
 *
 * ---------------------------------------------------------------------------*/

void jmt(double djul, long& jj, long& mm, double& dd) {
  long   ih, ih1, ih2 ;
  double t1, t2,  t3, t4;

  t1  = 1.0 + djul - fmod( djul, 1.0 ) + 2400000.0;
  t4  = fmod( djul, 1.0 );
  ih  = long( (t1 - 1867216.25)/36524.25 );
  t2  = t1 + 1 + ih - ih/4;
  t3  = t2 - 1720995.0;
  ih1 = long( (t3 - 122.1)/365.25 );
  t1  = 365.25*ih1 - fmod( 365.25*ih1, 1.0 );
  ih2 = long( (t3 - t1)/30.6001 );
  dd  = t3 - t1 - (int)( 30.6001*ih2 ) + t4;
  mm  = ih2 - 1;
  
  if ( ih2 > 13 ) mm = ih2 - 13;
  
  jj  = ih1;
  
  if ( mm <= 2 ) jj = jj + 1;
  
} 
