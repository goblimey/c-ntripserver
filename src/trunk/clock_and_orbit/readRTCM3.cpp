/* Programheader
 *
        Name:           readRTCM.cpp
        Project:        SSR test
        Version:        $Id: readRTCM3.cpp 6208 2014-09-25 14:39:00Z stuerze $
        Description:    reads, decodes and writes mixed RTCM SSR data sets
                        supported messages:
                        - System:          part of 1013,
                        - Ephemeris Data:  1019, 1020,
                        - Orbit and Clock: 1057, 1058, 1061,1063, 1064, 1067
                        - Code Biases:     1059, 1065,
                        - Phase Biases:    1265, 1266,
                        - VTEC:            1264
*/

extern "C" {
#include "clock_orbit_rtcm.h"
#include "rtcm3torinex.h"
}

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
using namespace std;

typedef enum {
    GPS, GLO, GAL, QZSS, SBAS, BDS
} SATSYS;
int GPSUTCdiff = 16;
/* prototype, don't use any of this code in a final application */
void printClock(const char* filename, struct ClockOrbit* clockOrb, SATSYS sys,
        char satSys, int offsetGnss);

void printOrbit(const char* filename, struct ClockOrbit* clockOrb, SATSYS sys,
        char satSys, int offsetGnss);

void printUra(const char* filename, struct ClockOrbit* clockOrb, SATSYS sys,
        char satSys, int offsetGnss);

void printVtec(const char* filename, struct VTEC* vtec);

void printCodeBias(const char* filename, struct CodeBias* codeBias, SATSYS sys,
        char satSys, int offsetGnss);

void printPhaseBias(const char* filename, struct PhaseBias* phaseBias, SATSYS sys,
        char satSys, int offsetGnss);

void printSystemParameters(const char* filename,
        struct RTCM3ParserData* parser);

void printTextMessageType(const char* filename, int messageType);

void printGpsEph(const char* filename, struct gpsephemeris* ephemerisGPS);

void printGlonassEph(const char* filename,
        struct glonassephemeris* ephemerisGLO);

void decodeFileContent(char* inputFile, char* outputFile);

int main(void) {
    char inFilename[124]  = "ssr2_svpb_data/svpb224m.rtc";
    char outFilename[124] = "ssr2_svpb_data/svpb224m_bkg.txt";
    unlink(outFilename);
    // Decode and save rtcm & ssr messages
    decodeFileContent(inFilename, outFilename);
    return 0;
}

void decodeFileContent(char* inputFile, char* outputFile) {
    char obuffer[CLOCKORBIT_BUFFERSIZE];
    long int offset = 0;
    FILE * pFile = fopen(inputFile, "rb");
    if (pFile == NULL) {
        fputs("File error", stderr);
        return;
    }
    while (1) {
        RTCM3ParserData rtcm3Out; memset(&rtcm3Out, 0, sizeof(rtcm3Out));
        ClockOrbit coOut; memset(&coOut, 0, sizeof(coOut));
        CodeBias cbOut; memset(&cbOut, 0, sizeof(cbOut));
        PhaseBias pbOut; memset(&pbOut, 0, sizeof(pbOut));
        VTEC vtecOut; memset(&vtecOut, 0, sizeof(vtecOut));
        fseek(pFile, offset, SEEK_SET);
        int l = fread(obuffer, 1, CLOCKORBIT_BUFFERSIZE, pFile);
        int bytesUsed = 0;
        GCOB_RETURN nl = GetSSR(&coOut, &cbOut, &vtecOut, &pbOut, obuffer, l, &bytesUsed);
        offset += bytesUsed;
        if (nl <= -30) {
            fprintf(stderr, "Not enough data. Error code: %d .\n", nl);
            break;
        } else if (nl < 0) {
            fprintf(stderr, "GCOB_RETURN %d\nTry now RTCM3TORINEX\n", nl);
            int z;
            for (z = 0; z < bytesUsed; ++z) {
                HandleByte(&rtcm3Out, (unsigned int) obuffer[z]);
            }
            fprintf(stderr, "RTCM message type: %d\n",rtcm3Out.blocktype);
            switch (rtcm3Out.blocktype) {
                case 1013:
                    printSystemParameters(outputFile, &rtcm3Out);
                    break;
                case 1019:
                    printGpsEph(outputFile, &rtcm3Out.ephemerisGPS);
                    break;
                case 1020:
                    printGlonassEph(outputFile, &rtcm3Out.ephemerisGLONASS);
                    break;
                case 1029:// rtcm3torinex is unable to decode this type of RTCM message
                	printTextMessageType(outputFile, rtcm3Out.blocktype);
                    break;
                case 1230:// rtcm3torinex is unable to decode this type of RTCM message
                	printTextMessageType(outputFile, rtcm3Out.blocktype);
            }
        } else if (nl == GCOBR_OK) {//fprintf(stderr, "GCOBR_OK\n");
            switch (coOut.messageType) {
                case 1057:
                    printOrbit(outputFile, &coOut, GPS, 'G',
                            CLOCKORBIT_OFFSETGPS);
                    break;
                case 1058:
                    printClock(outputFile, &coOut, GPS, 'G',
                            CLOCKORBIT_OFFSETGPS);
                    break;
                case 1061:
                    printUra(outputFile, &coOut, GPS, 'G',
                            CLOCKORBIT_OFFSETGPS);
                    break;
                case 1063:
                    printOrbit(outputFile, &coOut, GLO, 'R',
                            CLOCKORBIT_OFFSETGLONASS);
                    break;
                case 1064:
                    printClock(outputFile, &coOut, GLO, 'R',
                            CLOCKORBIT_OFFSETGLONASS);
                    break;
                case 1067:
                    printUra(outputFile, &coOut, GLO, 'R',
                            CLOCKORBIT_OFFSETGLONASS);
                    break;
            }
            switch (cbOut.messageType) {
                case 1059:
                    printCodeBias(outputFile, &cbOut, GPS, 'G',
                            CLOCKORBIT_OFFSETGPS);
                    break;
                case 1065:
                    printCodeBias(outputFile, &cbOut, GLO, 'R',
                            CLOCKORBIT_OFFSETGLONASS);
                    break;
            }
            switch (pbOut.messageType) {
                case 1265:
                    printPhaseBias(outputFile, &pbOut, GPS, 'G',
                            CLOCKORBIT_OFFSETGPS);
                    break;
                case 1266:
                    printPhaseBias(outputFile, &pbOut, GLO, 'R',
                            CLOCKORBIT_OFFSETGLONASS);
                    break;
            }
            if (vtecOut.EpochTime) {
                printVtec(outputFile, &vtecOut);
            }
        } else if (nl == GCOBR_MESSAGEFOLLOWS) {
            fprintf(stderr, "GCOBR_MESSAGEFOLLOWS\n");
        }
    }
}

void printClock(const char* filename, struct ClockOrbit* clockOrb, SATSYS sys,
        char satSys, int offsetGnss) {
    int timeOffset = 0;
    string datum[2] = { "ITRF", "LOCAL" };
    FILE *filestream = fopen(filename, "ab+");
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    switch (sys) {
        case GPS:
            fprintf(filestream,
                    "**RTCM 3 - SSR GPS Clock Message <%d>\nSystem:\t\t%10d (GPS)\nTime of week [s]:",
                    clockOrb->messageType, sys);
            timeOffset = 0;
            break;
        case GLO:
            fprintf(filestream,
                    "**RTCM 3 - SSR GLONASS Clock Message <%d>\nSystem:\t\t%10d (GLONASS)\nTime of day [s]:",
                    clockOrb->messageType, sys);
            timeOffset = (GPSUTCdiff - 3 * 3600);
            break;
    }
    fprintf(filestream,
            "\t%10d\nUpdate Interval:\t%10d\nIOD SSR:\t\t%10d\n"
                    "ProviderId:\t\t%10d\nSolutionId:\t\t%10d\nNumber of Satellites:\t%10d\n"
                    "PRN     A0[m]     A1[mm/s]  A2[mm/s^2]\n",
            clockOrb->EpochTime[sys] + timeOffset, clockOrb->UpdateInterval,
            clockOrb->SSRIOD, clockOrb->SSRProviderID, clockOrb->SSRSolutionID,
            clockOrb->NumberOfSat[sys]);
    unsigned int i = 0;
    for (i = offsetGnss; i < offsetGnss + clockOrb->NumberOfSat[sys]; ++i) {
        fprintf(filestream, "%c%02d %10.4f%10.3f%10.5f\n", satSys,
                clockOrb->Sat[i].ID, clockOrb->Sat[i].Clock.DeltaA0,
                clockOrb->Sat[i].Clock.DeltaA1 * 1.0E3,
                clockOrb->Sat[i].Clock.DeltaA2 * 1.0E3);
    }
    fclose(filestream);
    return;
}

void printOrbit(const char* filename, struct ClockOrbit* clockOrb, SATSYS sys,
        char satSys, int offsetGnss) {
    int timeOffset = 0;
    string datum[2] = { "ITRF", "LOCAL" };
    FILE *filestream = fopen(filename, "ab+");
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    switch (sys) {
        case GPS:
            fprintf(filestream,
                    "**RTCM 3 - SSR GPS Orbit Message <%d>\nSystem:\t\t%10d (GPS)\nTime of week [s]:",
                    clockOrb->messageType, sys);
            timeOffset = 0;
            break;
        case GLO:
            fprintf(filestream,
                    "**RTCM 3 - SSR GLONASS Orbit Message <%d>\nSystem:\t\t%10d (GLONASS)\nTime of day [s]:",
                    clockOrb->messageType, sys);
            timeOffset = (GPSUTCdiff - 3 * 3600);
            break;
    }
    fprintf(filestream,
            "\t%10d\nUpdate Interval:\t%10d\nIOD SSR:\t\t%10d\n"
                    "ProviderId:\t\t%10d\nSolutionId:\t\t%10d\nReference Datum:\t\t%s\nNumber of Satellites:\t%10d\n"
                    "PRN   IOD    Radial    AlongTrk  CrossTrk[m]    DotDelta[mm/s]\n",
            clockOrb->EpochTime[sys] + timeOffset, clockOrb->UpdateInterval,
            clockOrb->SSRIOD, clockOrb->SSRProviderID, clockOrb->SSRSolutionID,
            (datum[clockOrb->SatRefDatum]).c_str(), clockOrb->NumberOfSat[sys]);
    unsigned int i = 0;
    for (i = offsetGnss; i < offsetGnss + clockOrb->NumberOfSat[sys]; ++i) {
        fprintf(filestream, "%c%02d %5d %10.4f%10.4f%10.4f%10.3f%10.3f%10.3f\n",
                satSys, clockOrb->Sat[i].ID, clockOrb->Sat[i].IOD,
                clockOrb->Sat[i].Orbit.DeltaRadial,
                clockOrb->Sat[i].Orbit.DeltaAlongTrack,
                clockOrb->Sat[i].Orbit.DeltaCrossTrack,
                clockOrb->Sat[i].Orbit.DotDeltaRadial * 1.0E3,
                clockOrb->Sat[i].Orbit.DotDeltaAlongTrack * 1.0E3,
                clockOrb->Sat[i].Orbit.DotDeltaAlongTrack * 1.0E3);
    }
    fclose(filestream);
    return;
}

void printUra(const char* filename, struct ClockOrbit* clockOrb, SATSYS sys,
        char satSys, int offsetGnss) {
    int timeOffset = 0;
    FILE *filestream = fopen(filename, "ab+");
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    switch (sys) {
        case GPS:
            fprintf(filestream,
                    "**RTCM 3 - SSR GPS URA Message <%d>\nSystem:\t\t%10d (GPS)\nTime of week [s]:",
                    clockOrb->messageType, sys);
            timeOffset = 0;
            break;
        case GLO:
            fprintf(filestream,
                    "**RTCM 3 - SSR GLONASS URA Message <%d>\nSystem:\t\t%10d (GLONASS)\nTime of day [s]:",
                    clockOrb->messageType, sys);
            timeOffset = (GPSUTCdiff - 3 * 3600);
            break;
    }
    fprintf(filestream,
            "\t%10d\nUpdate Interval:\t%10d\nIOD SSR:\t\t%10d\n"
                    "ProviderId:\t\t%10d\nSolutionId:\t\t%10d\nNumber of Satellites:\t%10d\n"
                    "PRN     URA[m]\n", clockOrb->EpochTime[sys] + timeOffset,
            clockOrb->UpdateInterval, clockOrb->SSRIOD, clockOrb->SSRProviderID,
            clockOrb->SSRSolutionID, clockOrb->NumberOfSat[sys]);
    unsigned int i = 0;
    for (i = offsetGnss; i < offsetGnss + clockOrb->NumberOfSat[sys]; ++i) {
        fprintf(filestream, "%c%02d %10.4f\n", satSys, clockOrb->Sat[i].ID,
                clockOrb->Sat[i].UserRangeAccuracy);
    }
    fclose(filestream);
    return;
}

void printCodeBias(const char* filename, struct CodeBias* codeBias, SATSYS sys,
        char satSys, int offsetGnss) {
    int timeOffset = 0;
    FILE *filestream = fopen(filename, "ab+");
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    switch (sys) {
        case GPS:
            fprintf(filestream,
                    "**RTCM 3 - SSR GPS Code Bias Message <%d>\nSystem:\t\t%10d (GPS)\nTime of week [s]:",
                    codeBias->messageType, sys);
            timeOffset = 0;
            break;
        case GLO:
            fprintf(filestream,
                    "**RTCM 3 - SSR GLONASS Code Bias Message <%d>\nSystem:\t\t%10d (GLONASS)\nTime of day [s]:",
                    codeBias->messageType, sys);
            timeOffset = (GPSUTCdiff - 3 * 3600);
            break;
    }
    fprintf(filestream,
            "\t%10d\nUpdate Interval:\t%10d\nIOD SSR:\t\t%10d\n"
                    "ProviderId:\t\t%10d\nSolutionId:\t\t%10d\nNumber of Satellites:\t%10d\n"
                    "PRN  Number      Type      Bias[m]     Type     Bias[m]\n",
            codeBias->EpochTime[sys] + timeOffset, codeBias->UpdateInterval,
            codeBias->SSRIOD, codeBias->SSRProviderID, codeBias->SSRSolutionID,
            codeBias->NumberOfSat[sys]);
    unsigned int i = 0;
    for (i = offsetGnss; i < offsetGnss + codeBias->NumberOfSat[sys]; ++i) {
        fprintf(filestream, "%c%02d %5d ", satSys, codeBias->Sat[i].ID,
                codeBias->Sat[i].NumberOfCodeBiases);
        int j;
        for (j = 0; j < codeBias->Sat[i].NumberOfCodeBiases; j++) {
            fprintf(filestream, "%10d %12.2f", codeBias->Sat[i].Biases[j].Type,
                    codeBias->Sat[i].Biases[j].Bias);
        }
        fprintf(filestream, "\n");
    }
    fclose(filestream);
    return;
}

void printPhaseBias(const char* filename, struct PhaseBias* phaseBias, SATSYS sys,
        char satSys, int offsetGnss) {
    int timeOffset = 0;
    FILE *filestream = fopen(filename, "ab+");
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    switch (sys) {
        case GPS:
            fprintf(filestream,
                    "**RTCM 3 - SSR GPS Phase Bias Message <%d>\nSystem:\t\t%10d (GPS)\nTime of week [s]:",
                    phaseBias->messageType, sys);
            timeOffset = 0;
            break;
        case GLO:
            fprintf(filestream,
                    "**RTCM 3 - SSR GLONASS Phase Bias Message <%d>\nSystem:\t\t%10d (GLONASS)\nTime of day [s]:",
                    phaseBias->messageType, sys);
            timeOffset = (GPSUTCdiff - 3 * 3600);
            break;
    }
    fprintf(filestream,
            "\t%10d\nUpdate Interval:\t\t%10d\nIOD SSR:\t\t\t%10d\n"
                    "ProviderId:\t\t\t%10d\nSolutionId:\t\t\t%10d\n"
    		        "DispersiveBiasConsistencyIndicator\t%2d\nMWConsistencyIndicator\t\t%10d\nNumber of Satellites:\t\t%10d\n"
                    "PRN  Yaw[deg] YawRate[deg/sec] Number Type INT-ind  WL-ind  DISC-count Bias[m] Type INT-ind  WL-ind  DISC-count Bias[m]\n",
                    phaseBias->EpochTime[sys] + timeOffset,
                    phaseBias->UpdateInterval,
                    phaseBias->SSRIOD, phaseBias->SSRProviderID, phaseBias->SSRSolutionID,
                    phaseBias->DispersiveBiasConsistencyIndicator ,
                    phaseBias->MWConsistencyIndicator,
                    phaseBias->NumberOfSat[sys]);
    unsigned int i = 0;
    for (i = offsetGnss; i < offsetGnss + phaseBias->NumberOfSat[sys]; ++i) {
        fprintf(filestream, "%c%02d %8.3f %8.4f %12d",
        		satSys,phaseBias->Sat[i].ID,
        		phaseBias->Sat[i].YawAngle,
        		phaseBias->Sat[i].YawRate,
                phaseBias->Sat[i].NumberOfPhaseBiases);
        int j;
        for (j = 0; j < phaseBias->Sat[i].NumberOfPhaseBiases; j++) {
            fprintf(filestream, "%6d %6d %6d %10d %10.2f",
            		phaseBias->Sat[i].Biases[j].Type,
            		phaseBias->Sat[i].Biases[j].SignalIntegerIndicator,
            		phaseBias->Sat[i].Biases[j].SignalsWideLaneIntegerIndicator,
            		phaseBias->Sat[i].Biases[j].SignalDiscontinuityCounter,
                    phaseBias->Sat[i].Biases[j].Bias);
        }
        fprintf(filestream, "\n");
    }
    fclose(filestream);
    return;
}

void printVtec(const char* filename, struct VTEC* vtec) {
    FILE *filestream = fopen(filename, "ab+");
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    fprintf(filestream,
            "**RTCM 3 - SSR VTEC Message <1264>\n"
                    "Time of week [s]:\t%10d\nUpdate Interval:\t%10d\nIOD SSR:\t\t%10d\n"
                    "ProviderId:\t\t%10d\nSolutionId:\t\t%10d\nNumber of Layers:\t%10d\n"
                    "Quality:\t\t%13.3f\n", vtec->EpochTime,
            vtec->UpdateInterval, vtec->SSRIOD, vtec->SSRProviderID,
            vtec->SSRSolutionID, vtec->NumLayers, vtec->Quality);

    unsigned int i, j, k;
    for (i = 0; i < vtec->NumLayers; ++i) {
        fprintf(filestream,
                "Layer: %2d Height [m]: %f Degree n:%2d Order m:%2d\n", i,
                vtec->Layers[i].Height, vtec->Layers[i].Degree,
                vtec->Layers[i].Order);
        for (j = 0; j <= vtec->Layers[i].Degree; ++j) {
            fprintf(filestream, "Cn%d [TECU]: ", j);
            for (k = 0; k <= vtec->Layers[i].Order; ++k) {
                fprintf(filestream, " %7.3f", vtec->Layers[i].Cosinus[k][j]);
            }
            fprintf(filestream, "\n");
        }
        for (j = 1; j <= vtec->Layers[i].Degree; ++j) {
            fprintf(filestream, "Sn%d [TECU]: ", j);
            for (k = 0; k <= vtec->Layers[i].Order; ++k) {
                fprintf(filestream, " %7.3f", vtec->Layers[i].Sinus[k][j]);
            }
            fprintf(filestream, "\n");
        }
    }
    fclose(filestream);
    return;
}

void printSystemParameters(const char* filename,
        struct RTCM3ParserData* parser) {
    FILE *filestream = fopen(filename, "ab+");
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    fprintf(filestream,
            "**RTCM 3 - System parameters <1013>\n"
                    "Modified Julian Day:\t %d\nSeconds of Day:\t\t%2d\nLeapSeconds:\t\t%2d\n",
            parser->modjulday, parser->secofday, parser->leapsec);
    fclose(filestream);
    return;
}

void printTextMessageType(const char* filename, int messageType) {
    FILE *filestream = fopen(filename, "ab+");
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    fprintf(filestream,
            "**RTCM 3 - Text Message <%d>\n", messageType);
    fclose(filestream);
    return;
}
void printGpsEph(const char* filename, struct gpsephemeris* ephemerisGPS) {
    FILE *filestream = fopen(filename, "ab+");
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    fprintf(filestream, "**RTCM 3 - GPS Ephemeris <1019>\n"
            "PRN                    %13d\n"
            "IODE                   %13d\n"
            "URAindex [1..15]       %13d\n"
            "SVhealth               %13d\n"
            "GPSweek                %13d\n"
            "IODC                   %13d\n"
            "TOW [s]                %13d\n"
            "TOC [s]                %13d\n"
            "TOE [s]                %13d\n"
            "clock_bias [s]         %13e\n"
            "clock_drift[s/s]       %13e\n"
            "clock_driftrate [s/s^2]%13e\n"
            "Crs [m]                %13e\n"
            "Delta_n [rad/s]        %13e\n"
            "M0 [rad]               %13e\n"
            "Cuc [rad]              %13e\n"
            "e                      %13e\n"
            "Cus [rad]              %13e\n"
            "sqrt_A [m^0.5]         %13e\n"
            "Cic [rad]              %13e\n"
            "OMEGA0 [rad]           %13e\n"
            "Cis [rad]              %13e\n"
            "i0 [rad]               %13e\n"
            "Crc [m]                %13e\n"
            "omega [rad]            %13e\n"
            "OMEGADOT [rad/s]       %13e\n"
            "IDOT [rad/s]           %13e\n"
            "TGD [s]                %13e\n", ephemerisGPS->satellite,
            ephemerisGPS->IODE, ephemerisGPS->URAindex, ephemerisGPS->SVhealth,
            ephemerisGPS->GPSweek, ephemerisGPS->IODC, ephemerisGPS->TOW,
            ephemerisGPS->TOC, ephemerisGPS->TOE, ephemerisGPS->clock_bias,
            ephemerisGPS->clock_drift, ephemerisGPS->clock_driftrate,
            ephemerisGPS->Crs, ephemerisGPS->Delta_n, ephemerisGPS->M0,
            ephemerisGPS->Cuc, ephemerisGPS->e, ephemerisGPS->Cus,
            ephemerisGPS->sqrt_A, ephemerisGPS->Cic, ephemerisGPS->OMEGA0,
            ephemerisGPS->Cis, ephemerisGPS->i0, ephemerisGPS->Crc,
            ephemerisGPS->omega, ephemerisGPS->OMEGADOT, ephemerisGPS->IDOT,
            ephemerisGPS->TGD);
    fclose(filestream);
    return;
    return;
}

void printGlonassEph(const char* filename,
        struct glonassephemeris* ephemerisGLO) {
    FILE *filestream = fopen(filename, "ab+");
    if (filestream == NULL) {
        fprintf(stderr, "ERROR: open file %s\n", filename);
        return;
    }
    fprintf(filestream, "**RTCM 3 - GLONASS Ephemeris <1020>\n"
            "SV                      %10d\n"
            "frequency_number        %10d\n"
            "tb [s]                  %10d\n"
            "tk [s]                  %10d\n"
            "E  [days]               %10d\n"
            "tau [s]                 %13e\n"
            "gamma                   %13e\n"
            "x_pos [km]              %13e\n"
            "x_velocity [km/s]       %13e\n"
            "x_acceleration [km/s^2] %13e\n"
            "y_pos [km]              %13e\n"
            "y_velocity [km/s]       %13e\n"
            "y_acceleration [km/s^2] %13e\n"
            "z_pos [km]              %13e\n"
            "z_velocity [km/s]       %13e\n"
            "z_acceleration [km/s^2] %13e\n", ephemerisGLO->almanac_number,
            ephemerisGLO->frequency_number, ephemerisGLO->tb, ephemerisGLO->tk,
            ephemerisGLO->E, ephemerisGLO->tau, ephemerisGLO->gamma,
            ephemerisGLO->x_pos, ephemerisGLO->x_velocity,
            ephemerisGLO->x_acceleration, ephemerisGLO->y_pos,
            ephemerisGLO->y_velocity, ephemerisGLO->y_acceleration,
            ephemerisGLO->z_pos, ephemerisGLO->z_velocity,
            ephemerisGLO->z_acceleration);
    fclose(filestream);
    return;
}

