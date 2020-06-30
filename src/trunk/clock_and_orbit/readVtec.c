#include "clock_orbit_rtcm.h"

#include <stdio.h>
#include <stdlib.h>

/* prototype, don't use any of this code in a final application */

void printVtec(struct VTEC vtec) {
	int i, j, k;
	for (i = 0; i < vtec.NumLayers; ++i) {
		printf("%d layer %d height %f degree %d order %d\n",
				vtec.EpochTime, i,
				vtec.Layers[i].Height,
				vtec.Layers[i].Degree,
				vtec.Layers[i].Order);
		for (j = 0; j <= vtec.Layers[i].Degree; ++j) {
			printf("Cn%d ", j);
			for (k = 0; k <= vtec.Layers[i].Order; ++k) {
				printf(" %7.3f", vtec.Layers[i].Cosinus[k][j]);
			}
			printf("\n");
		}
		for (j = 0; j <= vtec.Layers[i].Degree; ++j) {
			printf("Sn%d ", j);
			for (k = 0; k <= vtec.Layers[i].Order; ++k) {
				printf(" %7.3f", vtec.Layers[i].Sinus[k][j]);
			}
			printf("\n");
		}
	}
}



int main(void) {;
    char obuffer[CLOCKORBIT_BUFFERSIZE];
   	//////////////////////////
	// VTEC                 //
	//////////////////////////

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
		fprintf(stderr, "VTEC INPUT\n");
		printVtec(v);
		FILE *f;
		int l = MakeVTEC(&v, 0, obuffer, sizeof(obuffer));
		if (!l)
			fprintf(stderr, "BUILD ERROR\n");
		else {
			int nl, ns;
			struct VTEC vv;

			if ((f = fopen("outfile.raw", "ab+"))) {
				fwrite(obuffer, l, 1, f);
				fclose(f);
			} else
				fprintf(stderr, "SAVE ERROR\n");
		    //fwrite(obuffer, l, 1, stdout);
			memset(&vv, 0, sizeof(vv));
			nl = GetSSR(0, 0, &vv, 0, obuffer, l, &ns);
			if (nl < 0)
				fprintf(stderr, "EXTRACT ERROR %d\n", nl);
			else if (nl > 0)
				fprintf(stderr, "MULTIBLOCK UNSUPPORTED IN TEST\n");
			else if (ns != l)
				fprintf(stderr, "SIZE MISMATCH (%d/%d)\n", ns, l);
			else {
				fprintf(stderr, "VTEC OUTPUT\n");
				printVtec(vv);
			}
		}
	return 0;
}


