#include "clock_orbit_rtcm.h"
#include "clock_orbit_rtcm.c"

#include <stdio.h>
#include <stdlib.h>

/* prototype, don't use any of this code in a final application */

struct ClockOrbit co;
double lasttow = -1.0;

int main(void)
{
  char * buffer = 0;
  size_t len = 0;
  char obuffer[CLOCKORBIT_BUFFERSIZE];

  while(getline(&buffer, &len, stdin) > 0)
  {
    char type;
    int week, prn, iodc, iode;
    double tow, clock, radial, along, outofplane;
    int num = sscanf(buffer, "%d %lf %c%d %d %d %lf %lf %lf %lf\n", &week,
    &tow, &type, &prn, &iodc, &iode, &clock, &radial, &along, &outofplane);
    if(num == 10)
    {
      struct SatData *sd;
      if(lasttow != tow) /* create block */
      {
        if(lasttow >= 0)
        {
          FILE *f;
          int l = MakeClockOrbit(&co, COTYPE_AUTO, 0, obuffer, sizeof(obuffer));
          if(!l) fprintf(stderr, "BUILD ERROR\n");
          else
          {
            int nl, ns;
            struct ClockOrbit c;

            if((f = fopen("outfile.raw", "ab+")))
            {
              fwrite(obuffer, l, 1, f);
              fclose(f);
            }
            else fprintf(stderr, "SAVE ERROR\n");
            //fwrite(obuffer, l, 1, stdout);
            memset(&c, 0, sizeof(c));
            nl = GetSSR(&c, 0, 0, 0, obuffer, l, &ns);
            if(nl < 0) fprintf(stderr, "EXTRACT ERROR %d\n", nl);
            else if(nl > 0) fprintf(stderr, "MULTIBLOCK UNSUPPORTED IN TEST\n");
            else if(ns != l) fprintf(stderr, "SIZE MISMATCH (%d/%d)\n", ns,l);
            else
            {
              int i;
              for(i = 0; i < c.NumberOfSat[CLOCKORBIT_SATGPS]; ++i)
              {
                printf("%d G%02d %d %f %f %f %f\n", c.EpochTime[CLOCKORBIT_SATGPS],
                c.Sat[i].ID, co.Sat[i].IOD, c.Sat[i].Clock.DeltaA0,
                c.Sat[i].Orbit.DeltaRadial, c.Sat[i].Orbit.DeltaAlongTrack,
                c.Sat[i].Orbit.DeltaCrossTrack);
              }
            }
          }
        }
        memset(&co, 0, sizeof(co));
        lasttow = tow;
        co.EpochTime[CLOCKORBIT_SATGPS] = (int)tow;
        /* co.GLONASSEpochTime = 0; */
        co.Supplied[COBOFS_CLOCK] = 1;
        co.Supplied[COBOFS_ORBIT] = 1;
        co.SatRefDatum = DATUM_ITRF;
      }

      sd = co.Sat + co.NumberOfSat[CLOCKORBIT_SATGPS];
      sd->ID = prn;
      sd->IOD = iode;
      sd->Clock.DeltaA0 = clock;
      sd->Orbit.DeltaRadial = radial;
      sd->Orbit.DeltaAlongTrack = along;
      sd->Orbit.DeltaCrossTrack = outofplane;
      printf("%d %d/%f %c%02d %d %d %f %f %f %f\n",num, week, tow, type, prn,
      iodc, iode, clock, radial, along, outofplane);
      ++co.NumberOfSat[CLOCKORBIT_SATGPS];
    }
  }
  {
    struct VTEC v;
    memset(&v, 0, sizeof(v));
    v.EpochTime = 218764;
    v.NumLayers = 1;
    v.UpdateInterval = 1;
    v.Layers[0].Height = 450000;
    v.Layers[0].Degree = 4;
    v.Layers[0].Order = 4;
    v.Layers[0].Cosinus[0][0] = 24.435;
    v.Layers[0].Cosinus[1][0] = 4.495;
    v.Layers[0].Cosinus[2][0] = -7.805;
    v.Layers[0].Cosinus[3][0] = -1.900;
    v.Layers[0].Cosinus[4][0] = 2.045;
    v.Layers[0].Cosinus[1][1] = -9.960;
    v.Layers[0].Cosinus[2][1] = 0.845;
    v.Layers[0].Cosinus[3][1] = 2.260;
    v.Layers[0].Cosinus[4][1] = -0.315;
    v.Layers[0].Cosinus[2][2] = 3.730;
    v.Layers[0].Cosinus[3][2] = -0.780;
    v.Layers[0].Cosinus[4][2] = -0.055;
    v.Layers[0].Cosinus[3][3] = -0.020;
    v.Layers[0].Cosinus[4][3] = 1.150;
    v.Layers[0].Cosinus[4][4] = 0.990;

    v.Layers[0].Sinus[1][1] = -6.070;
    v.Layers[0].Sinus[2][1] = -3.105;
    v.Layers[0].Sinus[3][1] = 1.310;
    v.Layers[0].Sinus[4][1] = 1.545;
    v.Layers[0].Sinus[2][2] = 0.325;
    v.Layers[0].Sinus[3][2] = -1.050;
    v.Layers[0].Sinus[4][2] = -0.750;
    v.Layers[0].Sinus[3][3] = 0.170;
    v.Layers[0].Sinus[4][3] = -0.690;
    v.Layers[0].Sinus[4][4] = -0.705;
    FILE *f;
    int l = MakeVTEC(&v, 0, obuffer, sizeof(obuffer));
    if(!l) fprintf(stderr, "BUILD ERROR\n");
    else
    {
      int nl, ns;
      struct VTEC vv;

      if((f = fopen("outfile.raw", "ab+")))
      {
        fwrite(obuffer, l, 1, f);
        fclose(f);
      }
      else fprintf(stderr, "SAVE ERROR\n");
      //fwrite(obuffer, l, 1, stdout);
      memset(&vv, 0, sizeof(vv));
      nl = GetSSR(0, 0, &vv, 0, obuffer, l, &ns);
      if(nl < 0) fprintf(stderr, "EXTRACT ERROR %d\n", nl);
      else if(nl > 0) fprintf(stderr, "MULTIBLOCK UNSUPPORTED IN TEST\n");
      else if(ns != l) fprintf(stderr, "SIZE MISMATCH (%d/%d)\n", ns,l);
      else
      {
        int i, j, k;
        for(i = 0; i < vv.NumLayers; ++i)
        {
          printf("%d layer %d height %f degree %d order %d\n", vv.EpochTime,
          i, vv.Layers[i].Height, vv.Layers[i].Degree, vv.Layers[i].Order);
          for(j = 0; j <= vv.Layers[i].Degree; ++j)
          {
            printf("Cn%d ",j);
            for(k = 0; k <= vv.Layers[i].Order; ++k)
            {
              printf(" %7.3f",vv.Layers[i].Cosinus[k][j]);
            }
            printf("\n");
          }
          for(j = 0; j <= vv.Layers[i].Degree; ++j)
          {
            printf("Sn%d ",j);
            for(k = 0; k <= vv.Layers[i].Order; ++k)
            {
              printf(" %7.3f",vv.Layers[i].Sinus[k][j]);
            }
            printf("\n");
          }
        }
      }
    }
  }
  free(buffer);
  return 0;
}
