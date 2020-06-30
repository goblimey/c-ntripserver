#include "clock_orbit_rtcm.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

struct ClockOrbit coIn;
struct CodeBias cbIn;
double lasttow_co = -1.0;
double lasttow_cb = -1.0;

typedef enum {GPS, GLO, GAL, QZSS, SBAS, BDS} SATSYS;
int GPSUTCdiff = 16;
char satSYS[6] = { 'G', 'R', 'E', 'J', 'S', 'C' };
int messageTypeCo[6] = { 1060, 1066, 1243, 1249, 1255, 1261 };
int messageTypeCb[6] = { 1059, 1065, 1242, 1248, 1254, 1260 };

void printClockOrbit(const char* filename, struct ClockOrbit* clockOrb,
        const char* flag, int ind, char satSys, int offsetGnss);
void printClockOrbitDiff(const char* filename, struct ClockOrbit* clockOrb1,
        struct ClockOrbit* clockOrb2, const char* flag, int ind, char satSys, int offsetGnss);
void printCodeBias(const char* filename, struct CodeBias* codeBias,
        const char* flag,int ind, char satSys, int offsetGnss);
void printCodeBiasDiff(const char* filename, struct CodeBias* codeBias1,
        struct CodeBias* codeBias2, const char* flag, int ind, char satSys,
        int offsetGnss);

int main(void) {
    SATSYS sys;
    const char * inputFile = "ssr1_cocb_data/CLK801330.14C";
    for (sys = GPS; sys <= BDS; ++sys) {
        const char *outFilenameRaw, *outFilenameDbg;
        enum COR_SATSYSTEM CLOCKORBIT_SATGNSS = sys;
        enum COR_OFFSETS CLOCKORBIT_OFFSETGNSS;
        lasttow_co = -1.0;
        lasttow_cb = -1.0;
        switch (sys) {
        case GPS:
            outFilenameRaw = "ssr1_cocb_data/outfile_G.raw";
            outFilenameDbg = "ssr1_cocb_data/outfile_G.dbg";
            CLOCKORBIT_OFFSETGNSS = CLOCKORBIT_OFFSETGPS;
            break;
        case GLO:
            outFilenameRaw = "ssr1_cocb_data/outfile_R.raw";
            outFilenameDbg = "ssr1_cocb_data/outfile_R.dbg";
            CLOCKORBIT_OFFSETGNSS = CLOCKORBIT_OFFSETGLONASS;
            break;
        case GAL:
            outFilenameRaw = "ssr1_cocb_data/outfile_E.raw";
            outFilenameDbg = "ssr1_cocb_data/outfile_E.dbg";
            CLOCKORBIT_OFFSETGNSS = CLOCKORBIT_OFFSETGALILEO;
            break;
        case QZSS:
            outFilenameRaw = "ssr1_cocb_data/outfile_J.raw";
            outFilenameDbg = "ssr1_cocb_data/outfile_J.dbg";
            CLOCKORBIT_OFFSETGNSS = CLOCKORBIT_OFFSETQZSS;
            break;
        case SBAS:
            outFilenameRaw = "ssr1_cocb_data/outfile_S.raw";
            outFilenameDbg = "ssr1_cocb_data/outfile_S.dbg";
            CLOCKORBIT_OFFSETGNSS = CLOCKORBIT_OFFSETSBAS;
            break;
        case BDS:
            outFilenameRaw = "ssr1_cocb_data/outfile_C.raw";
            outFilenameDbg = "ssr1_cocb_data/outfile_C.dbg";
            CLOCKORBIT_OFFSETGNSS = CLOCKORBIT_OFFSETBDS;
            break;
        }
        unlink(outFilenameRaw);
        unlink(outFilenameDbg);
        FILE *asciiSsr, *f;
        size_t len = 0;
        char *buffer = 0, type = satSYS[sys];
        asciiSsr = fopen(inputFile, "r");
        if (asciiSsr == NULL) {
            fprintf(stderr, "ERROR: open file %s\n", inputFile);
            return 0;
        }
        while (getline(&buffer, &len, asciiSsr) > 0) { //fprintf(stderr, "line: %s", buffer);
            char coBuffer[CLOCKORBIT_BUFFERSIZE];
            char cbBuffer[CLOCKORBIT_BUFFERSIZE];
            int MT = 0, messageType = 0, ui = 0, week = 0, prn = 0, iode = 0, ncb = 0;
            double cbValue[3] = { 0.0 }, clock_a0 = 0.0, clock_a1 = 0.0, clock_a2 = 0.0, d_radial = 0.0;
            double d_along = 0.0, d_outofplane = 0.0, dd_radial = 0.0, dd_along = 0.0, dd_outofplane = 0.0;
            int cbType[3] = { 0 };
            static double tow_co, tow_cb;
            sscanf(buffer, "%d ", &messageType);
            if (messageType == messageTypeCo[GPS]) {
                sscanf(buffer,
                        "%d %d %d %lf %c%d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
                        &messageType, &ui, &week, &tow_co, &type, &prn, &iode,
                        &clock_a0, &d_radial, &d_along, &d_outofplane,
                        &clock_a1, &dd_radial, &dd_along, &dd_outofplane,
                        &clock_a2);
                type = satSYS[sys];
                MT = messageTypeCo[sys];
                switch (sys) {
                case GPS:case GAL:case SBAS:case BDS:
                    break;
                case GLO:
                    if (prn > 24)
                        continue;
                    break;
                case QZSS:
                    if (prn > 10)
                        continue;
                    break;
                }
                if ((lasttow_co != tow_co)) { // create block
                    int nl = 0, ns = 0, l = 0;
                    struct ClockOrbit coOut;
                    if (lasttow_co >= 0) {
                        if (sys == GLO) {
                            coIn.EpochTime[CLOCKORBIT_SATGLONASS] =
                                    (fmod((double)coIn.EpochTime[CLOCKORBIT_SATGLONASS]
                                                                 + (3 * 3600 - GPSUTCdiff), 86400.0));
                        }
                        printClockOrbit(outFilenameDbg, &coIn, "INPUT",
                                CLOCKORBIT_SATGNSS, type, CLOCKORBIT_OFFSETGNSS);
                        l = MakeClockOrbit(&coIn, COTYPE_AUTO, 0, coBuffer,
                                sizeof(coBuffer));
                        if (!l)
                            fprintf(stderr, "BUILD ERROR\n");
                        else {
                            if ((f = fopen(outFilenameRaw, "ab+"))) {
                                fwrite(coBuffer, l, 1, f);
                                fclose(f);
                            } else
                                fprintf(stderr, "SAVE ERROR %s\n",
                                        outFilenameRaw);
                        }
                        memset(&coOut, 0, sizeof(coOut));
                        nl = GetSSR(&coOut, 0, 0, 0, coBuffer, l, &ns);
                        if (nl < 0)
                            fprintf(stderr, "CLKORB EXTRACT ERROR %d\n", nl);
                        else if (nl > 0)
                            fprintf(stderr, "CLKORB MULTIBLOCK UNSUPPORTED IN TEST\n");
                        else if (ns != l)
                            fprintf(stderr, "CLKORB SIZE MISMATCH (%d/%d)\n", ns, l);
                        else {
                            printClockOrbit(outFilenameDbg, &coOut, "OUTPUT",
                                            CLOCKORBIT_SATGNSS, type, CLOCKORBIT_OFFSETGNSS);
                            printClockOrbitDiff(outFilenameDbg, &coIn, &coOut,
                                                "DIFF", CLOCKORBIT_SATGNSS, type, CLOCKORBIT_OFFSETGNSS);
                        }
                    }
                    memset(&coIn, 0, sizeof(coIn));
                    lasttow_co = tow_co;
                    coIn.messageType = MT;
                    coIn.EpochTime[CLOCKORBIT_SATGNSS] = (int) tow_co;
                    coIn.UpdateInterval = (int) ui;
                    coIn.Supplied[COBOFS_COMBINED] = 1;
                    coIn.SatRefDatum = DATUM_ITRF;
                }
                struct SatData *sd;
                sd = coIn.Sat
                        + CLOCKORBIT_OFFSETGNSS
                        + coIn.NumberOfSat[CLOCKORBIT_SATGNSS];
                sd->ID = prn;
                sd->IOD = iode;
                sd->Clock.DeltaA0 = clock_a0;
                sd->Clock.DeltaA1 = clock_a1;
                sd->Clock.DeltaA2 = clock_a2;
                sd->Orbit.DeltaRadial = d_radial;
                sd->Orbit.DeltaAlongTrack = d_along;
                sd->Orbit.DeltaCrossTrack = d_outofplane;
                sd->Orbit.DotDeltaRadial = dd_radial;
                sd->Orbit.DotDeltaAlongTrack = dd_along;
                sd->Orbit.DotDeltaCrossTrack = dd_outofplane;
                ++coIn.NumberOfSat[CLOCKORBIT_SATGNSS];
            }
            if (messageType == messageTypeCb[GPS]) {
                sscanf(buffer, "%d %d %d %lf %c%d %d %d %lf %d %lf %d %lf\n",
                        &messageType, &ui, &week, &tow_cb, &type, &prn, &ncb,
                        &cbType[0], &cbValue[0], &cbType[1], &cbValue[1],
                        &cbType[2], &cbValue[2]);
                type = satSYS[sys];
                MT = messageTypeCb[sys];
                switch (sys) {
                case GPS:
                    break;
                case GLO:
                    if (prn > 24)
                        continue;
                    cbType[0] = CODETYPEGLONASS_L1_CA;
                    cbType[1] = CODETYPEGLONASS_L1_P;
                    cbType[2] = CODETYPEGLONASS_L2_CA;
                    break;
                case GAL:
                    cbType[0] = CODETYPEGALILEO_E1_A;
                    cbType[1] = CODETYPEGALILEO_E5_I;
                    cbType[2] = CODETYPEGALILEO_E6_A;
                    break;
                case QZSS:
                    if (prn > 10)
                        continue;
                    cbType[0] = CODETYPEQZSS_L1_CA;
                    cbType[1] = CODETYPEQZSS_L2_CM;
                    cbType[2] = CODETYPEQZSS_L5_I;
                    break;
                case SBAS:
                    cbType[0] = CODETYPE_SBAS_L1_CA;
                    cbType[1] = CODETYPE_SBAS_L5_I;
                    cbType[2] = CODETYPE_SBAS_L5_Q;
                    break;
                case BDS:
                    cbType[0] = CODETYPE_BDS_B1_I;
                    cbType[1] = CODETYPE_BDS_B3_I;
                    cbType[2] = CODETYPE_BDS_B2_I;
                    break;
                }
                if (lasttow_cb != tow_cb) { // create block
                    int nl, ns, l;
                    struct CodeBias cbOut;
                    if (lasttow_cb >= 0) {
                        if (sys == GLO) {
                            cbIn.EpochTime[CLOCKORBIT_SATGLONASS] =
                                    (fmod((double)cbIn.EpochTime[CLOCKORBIT_SATGLONASS]
                                     + (3 * 3600 - GPSUTCdiff), 86400.0));
                        }
                        printCodeBias(outFilenameDbg, &cbIn, "INPUT",
                                CLOCKORBIT_SATGNSS, type, CLOCKORBIT_OFFSETGNSS);
                        l = MakeCodeBias(&cbIn, BTYPE_AUTO, 0, cbBuffer,
                                sizeof(cbBuffer));
                        if (!l)
                            fprintf(stderr, "BUILD ERROR\n");
                        else {
                            if ((f = fopen(outFilenameRaw, "ab+"))) {
                                fwrite(cbBuffer, l, 1, f);
                                fclose(f);
                            } else
                                fprintf(stderr, "SAVE ERROR %s\n",  outFilenameRaw);
                        }
                        memset(&cbOut, 0, sizeof(cbOut));
                        nl = GetSSR(NULL, &cbOut, NULL, 0, cbBuffer, l, &ns);
                        if (nl < 0)
                            fprintf(stderr, "CBIAS EXTRACT ERROR %d\n", nl);
                        else if (nl > 0)
                            fprintf(stderr, "CBIAS MULTIBLOCK UNSUPPORTED IN TEST\n");
                        else if (ns != l)
                            fprintf(stderr, "CBIAS SIZE MISMATCH (%d/%d)\n", ns, l);
                        else {
                            printCodeBias(outFilenameDbg, &cbOut, "OUTPUT",
                                    CLOCKORBIT_SATGNSS, type, CLOCKORBIT_OFFSETGNSS);
                            printCodeBiasDiff(outFilenameDbg, &cbIn, &cbOut,
                                    "DIFF", CLOCKORBIT_SATGNSS, type, CLOCKORBIT_OFFSETGNSS);
                        }
                    }
                    memset(&cbIn, 0, sizeof(cbIn));
                    lasttow_cb = tow_cb;
                    cbIn.messageType = MT;
                    cbIn.EpochTime[CLOCKORBIT_SATGNSS] = (int) tow_cb;
                    cbIn.UpdateInterval = ui;
                    coIn.Supplied[COBOFS_BIAS] = 1;
                }
                struct BiasSat *bs;
                bs = cbIn.Sat
                        + CLOCKORBIT_OFFSETGNSS
                        + cbIn.NumberOfSat[CLOCKORBIT_SATGNSS];
                bs->ID = prn;
                bs->NumberOfCodeBiases = ncb;
                int k = 0;
                for (k = 0; k < bs->NumberOfCodeBiases; k++) {
                    bs->Biases[k].Type = cbType[k];
                    bs->Biases[k].Bias = cbValue[k];
                }
                ++cbIn.NumberOfSat[CLOCKORBIT_SATGNSS];
            }
        }
        free(buffer);
        fclose(asciiSsr);
    }
    return 0;
}

void printClockOrbit(const char* filename, struct ClockOrbit* clockOrb,
        const char* flag, int ind, char satSys, int offsetGnss) {
    int i = 0;
    FILE *filestream = fopen(filename, "ab+");
    if (!clockOrb->NumberOfSat[ind])
        return;
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    fprintf(filestream, "CLKORB_%s\n", flag);
    for (i = offsetGnss; i < offsetGnss + clockOrb->NumberOfSat[ind]; ++i) {
        fprintf(filestream,
                "%10d %d %c%02d %5d %12.3f %12.3f %12.3f %12.3f %12.3f %12f %12f %12f %12.3f\n",
                clockOrb->EpochTime[ind], clockOrb->UpdateInterval, satSys,
                clockOrb->Sat[i].ID,
                clockOrb->Sat[i].IOD,
                clockOrb->Sat[i].Clock.DeltaA0,
                clockOrb->Sat[i].Orbit.DeltaRadial,
                clockOrb->Sat[i].Orbit.DeltaAlongTrack,
                clockOrb->Sat[i].Orbit.DeltaCrossTrack,
                clockOrb->Sat[i].Clock.DeltaA1,
                clockOrb->Sat[i].Orbit.DotDeltaRadial,
                clockOrb->Sat[i].Orbit.DotDeltaAlongTrack,
                clockOrb->Sat[i].Orbit.DotDeltaCrossTrack,
                clockOrb->Sat[i].Clock.DeltaA2);
    }
    fclose(filestream);
}

void printClockOrbitDiff(const char* filename, struct ClockOrbit* clockOrb1,
        struct ClockOrbit* clockOrb2, const char* flag, int ind, char satSys,
        int offsetGnss) {
    int i = 0;
    FILE *filestream = fopen(filename, "ab+");
    if (!clockOrb1->NumberOfSat[ind])
        return;
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    fprintf(filestream, "CLKORB_%s\n", flag);
    for (i = offsetGnss; i < offsetGnss + clockOrb1->NumberOfSat[ind]; ++i) {
        fprintf(filestream,
                "%10d %d %c%02d %5d %12.3f %12.3f %12.3f %12.3f %12.3f %12f %12f %12f %12.3f\n",
                clockOrb1->EpochTime[ind]
                        - clockOrb2->EpochTime[ind],
                clockOrb1->UpdateInterval
                        - clockOrb2->UpdateInterval, satSys,
                clockOrb1->Sat[i].ID
                        - clockOrb2->Sat[i].ID,
                clockOrb1->Sat[i].IOD - clockOrb2->Sat[i].IOD,
                clockOrb1->Sat[i].Clock.DeltaA0
                        - clockOrb2->Sat[i].Clock.DeltaA0,
                clockOrb1->Sat[i].Orbit.DeltaRadial
                        - clockOrb2->Sat[i].Orbit.DeltaRadial,
                clockOrb1->Sat[i].Orbit.DeltaAlongTrack
                        - clockOrb2->Sat[i].Orbit.DeltaAlongTrack,
                clockOrb1->Sat[i].Orbit.DeltaCrossTrack
                        - clockOrb2->Sat[i].Orbit.DeltaCrossTrack,
                clockOrb1->Sat[i].Clock.DeltaA1
                        - clockOrb2->Sat[i].Clock.DeltaA1,
                clockOrb1->Sat[i].Orbit.DotDeltaRadial
                        - clockOrb2->Sat[i].Orbit.DotDeltaRadial,
                clockOrb1->Sat[i].Orbit.DotDeltaAlongTrack
                        - clockOrb2->Sat[i].Orbit.DotDeltaAlongTrack,
                clockOrb1->Sat[i].Orbit.DotDeltaCrossTrack
                        - clockOrb2->Sat[i].Orbit.DotDeltaCrossTrack,
                clockOrb1->Sat[i].Clock.DeltaA2
                        - clockOrb2->Sat[i].Clock.DeltaA2);
    }
    fclose(filestream);
}

void printCodeBias(const char* filename, struct CodeBias* codeBias, const char* flag,
        int ind, char satSys, int offsetGnss) {
    int i = 0;
    FILE *filestream = fopen(filename, "ab+");
    if (!codeBias->NumberOfSat[ind])
        return;
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    fprintf(filestream, "CBIAS_%s\n", flag);
    for (i = offsetGnss; i < offsetGnss + codeBias->NumberOfSat[ind]; ++i) {
        fprintf(filestream, "%10d %d %c%02d %2d ",
                codeBias->EpochTime[ind], codeBias->UpdateInterval, satSys,
                codeBias->Sat[i].ID, codeBias->Sat[i].NumberOfCodeBiases);
        int j;
        for (j = 0; j < codeBias->Sat[i].NumberOfCodeBiases; j++) {
            fprintf(filestream, "%4d %12.3f", codeBias->Sat[i].Biases[j].Type,
                    codeBias->Sat[i].Biases[j].Bias);
        }
        fprintf(filestream, "\n");
    }
    fclose(filestream);
}

void printCodeBiasDiff(const char* filename, struct CodeBias* codeBias1,
        struct CodeBias* codeBias2, const char* flag, int ind, char satSys,
        int offsetGnss) {
    int i = 0;
    FILE *filestream = fopen(filename, "ab+");
    if (!codeBias1->NumberOfSat[ind])
        return;
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    fprintf(filestream, "CBIAS_%s\n", flag);
    for (i = offsetGnss; i < offsetGnss + codeBias1->NumberOfSat[ind]; ++i) {
        fprintf(filestream, "%10d %d %c%02d %2d ",
                codeBias1->EpochTime[ind]
                    - codeBias2->EpochTime[ind],
                codeBias1->UpdateInterval
                    - codeBias2->UpdateInterval, satSys,
                codeBias1->Sat[i].ID
                    - codeBias2->Sat[i].ID,
                codeBias1->Sat[i].NumberOfCodeBiases
                    - codeBias2->Sat[i].NumberOfCodeBiases);
        int j;
        for (j = 0; j < codeBias1->Sat[i].NumberOfCodeBiases; j++) {
            fprintf(filestream, "%4d %12.3f",
                    codeBias1->Sat[i].Biases[j].Type
                        - codeBias2->Sat[i].Biases[j].Type,
                    codeBias1->Sat[i].Biases[j].Bias
                        - codeBias2->Sat[i].Biases[j].Bias);
        }
        fprintf(filestream, "\n");
    }
    fclose(filestream);
}
