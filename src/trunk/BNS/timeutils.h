#ifndef TIMEUTILS_H
#define TIMEUTILS_H

double djul(long j1, long m1, double tt);
double gpjd(double second, int nweek) ;
void   jdgp(double tjul, double & second, long & nweek);
void   jmt (double djul, long& jj, long& mm, double& dd);

#endif
