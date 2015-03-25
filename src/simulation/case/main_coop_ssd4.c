#include "plus/pie_stdinc.h"
#include "plus/pie_ini_file.h"
#include "plus/pie_alloc.h"
#include "plus/pie_random.h"

#include "simulation/pie_numeric.h"
#include "simulation/pie_channel.h"
#include "simulation/pie_equalizer.h"
#include "simulation/pie_modem_4d.h"
#include "simulation/pie_modem_2d.h"
#include "simulation/pie_sim_misc.h"
#include "simulation/pie_ssd_interleaver.h"
#include "simulation/pie_trellis.h"

#include "stdio.h"

#define MODEM_SECTION "Modem4D"
#define MODEM_SECTION2 "Modem2D"
#define CODEC_SECTION "TrellisCodec"
#define CASE_SECTION "Simulation"

// #define NO_COOP

typedef struct Relay {
        PieComplex fading;
        PieComplex chan[3000];
        PieComplex equ[3000];
        PieBoolean done;
        PieBoolean source;
} Relay;

static int compFlag[3][2] = {
        {PIE_MODEM_4D_A, PIE_MODEM_4D_B},
        {(PIE_MODEM_4D_A | PIE_MODEM_4D_C), (PIE_MODEM_4D_B | PIE_MODEM_4D_D)},
        {(PIE_MODEM_4D_A | PIE_MODEM_4D_C | PIE_MODEM_4D_B), 
         (PIE_MODEM_4D_B | PIE_MODEM_4D_D | PIE_MODEM_4D_A)},
};

int main_coop_ssd4()
{
        // init/*{{{*/
        PieIniFile ini;
        if (!PieIniFile_init(&ini, "resource/config.ini")) {
                printf("config file init error\n");
                return 0;
        }

        // init components

        PieModem4D modem;
        if (!PieModem4D_init(&modem, &ini, MODEM_SECTION)) {
                printf("init modem error\n");
                return 0;
        }

        PieTrellis codec;
        if (!PieTrellis_init(&codec, &ini, CODEC_SECTION)) {
                printf("init codec error\n");
                return 0;
        }

        // init simulation config

        char outputFile[100];
        if (!PieIniFile_getProfileString(&ini,
                        CASE_SECTION, "ssc4", 100, outputFile)) {
                printf("output path error\n");
                return 0;
        }

        double snr[100];
        int snrNum = PieIniFile_getProfileVectorDouble(&ini, 
                        CASE_SECTION, "SNR", 100, snr);
        if (snrNum == 0) {
                printf("snr number is 0\n");
                return 0;
        }

        int pnum[100];
        int tmp = PieIniFile_getProfileVectorInt(&ini,
                        CASE_SECTION, "NUM", 100, pnum);
        if (snrNum != tmp) {
                printf("snr number inconsistent\n");
                return 0;
        }

        int len = PieIniFile_getProfileInt(&ini, CASE_SECTION, "frameLen", 0);
        if (len == 0) {
                printf("frame len error\n");
                return 0;
        }

        int user = PieIniFile_getProfileInt(&ini, CASE_SECTION, "user", 0);
        if (user <= 0) {
                printf("user number error\n");
                return 0;
        }

        int seed = PieIniFile_getProfileInt(&ini, CASE_SECTION, "seed", 0);
        Pie_randomSeed((PieUint64)seed);

        double rlsnr[100];
        int rlnum = PieIniFile_getProfileVectorDouble(&ini,
                        CASE_SECTION, "RL", 100, rlsnr);
        if (rlnum != snrNum) {
                printf("rl snr number error\n");
                return 0;
        }

        double ber[100];
        double fer[100];

        int transferNumber = PieTrellis_getTransferNumber(&codec, 
                        len, PIE_TRUE);
        int encodedNumber = codec.parityNumber * transferNumber;
        int encodedNumber2 = encodedNumber / 2;
        int modulatedNumber = encodedNumber / modem.order;
        int slotSize = modulatedNumber / 4;
        int slotSize2 = modulatedNumber / 2;

/*}}}*/

        // buffer alloc/*{{{*/
        PieUint32 *source = Pie_calloc(len, sizeof *source);
        PieUint32 *system = Pie_calloc(transferNumber * 5, sizeof *system);
        PieUint32 *system2 = system + transferNumber;
        PieUint32 *parity1 = system2 + transferNumber;
        PieUint32 *parity2 = parity1 + transferNumber;
        PieUint32 *parity3 = parity2 + transferNumber;
        PieUint32 *encoded = Pie_calloc(encodedNumber, sizeof *encoded);

        PieComplex *modulated = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *itlvQ = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *chan = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *symb = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *echan = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *equ = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *equQ = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *deitlv2 = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *demodIn = Pie_calloc(modulatedNumber, sizeof *modulated);

        double *demodOut = Pie_calloc(encodedNumber, sizeof *demodOut);
        double *decodeIn = Pie_calloc(encodedNumber, sizeof *decodeIn);
        double *parity1r = decodeIn;
        double *parity2r = parity1r + transferNumber;
        double *parity3r = parity2r + transferNumber;
        double *llr = Pie_calloc(len, sizeof *llr);
        double *systemLLR = Pie_calloc(transferNumber * 5, sizeof *systemLLR);
        double *systemLLR2 = systemLLR + transferNumber;
        double *parityLLR1 = systemLLR2 + transferNumber;
        double *parityLLR2 = parityLLR1 + transferNumber;
        double *parityLLR3 = parityLLR2 + transferNumber;
        double *zero = Pie_calloc(3000, sizeof *zero);
        for (int i = 0; i < 3000; i++)
                zero[i] = 0;

        Relay *rr = Pie_calloc(user, sizeof *rr);

/*}}}*/

        printf("simulation on process\n");
        for(int i = 0; i < snrNum; i++) {
                int errB = 0;
                int errF = 0;
                int full[4] = {0, 0, 0, 0};
                for(int j = 0; j < pnum[i]; j++) {
                        printf("snr: %.2f, rl: %.2f err: %d, prg: %d/%d\r", 
                                        snr[i], rlsnr[i], errF, j, pnum[i]);
// ----------------------------------------------------------------------------
                        // data src/*{{{*/
                        // source message
                        if (!PieSim_randomVectorInt(len, source)) {
                                printf("generate source error\n");
                                return 0;
                        }

                        // convolutional code
                        if (!PieTrellis_encode(&codec, 
                                               len, 
                                               source, 
                                               0, 
                                               transferNumber, 
                                               PIE_TRELLIS_TAIL,
                                               0, 
                                               system, 
                                               system2,
                                               parity1, 
                                               parity2, 
                                               parity3)) {
                                printf("encode error\n");
                                return 0;
                        }

                        // bit reassigned
                        PieUint32 *tmp = encoded;
                        for (int i = 0; i < transferNumber; i++) {
                                tmp[0] = parity1[i];
                                tmp[1] = parity2[i];
                                tmp[2] = parity3[i];
                                tmp += codec.parityNumber;
                        }

                        // modulate
                        if (!PieModem4D_modulate(&modem, 
                                                 encodedNumber, 
                                                 encoded, 
                                                 modulatedNumber, 
                                                 modulated)) {
                                printf("modulate error\n");
                                return 0;
                        }

                        // component interleave
                        if (!PieSSDInterleaver_splitQ(modulatedNumber, 
                                                      modulated,
                                                      itlvQ)) {
                                printf("q interleave error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_split2(slotSize2,
                                                      itlvQ,
                                                      symb)) {
                                printf("2 interleave error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_split2(slotSize2,
                                                      itlvQ + slotSize2,
                                                      symb + slotSize2)) {
                                printf("2 interleave error\n");
                                return 0;
                        }
/*}}}*/

                        // relay logic/*{{{*/
                        int relay[4];
                        relay[0] = 0;
                        relay[1] = 0;
                        relay[2] = 0;
                        relay[3] = 0;
                        for (int u = 0; u < user; u++) {
                                rr[u].fading.real = 0;
                                rr[u].fading.imag = 0;
                                rr[u].done = PIE_FALSE;
                                rr[u].source = PIE_FALSE;
                        }
                        for (int t = 0; t < 3; t++) {
                                int xx = t * slotSize;
                                PieBoolean claim = PIE_FALSE;
                                for (int u = 0; u < user; u++) {
                                        if (rr[u].source) {
                                                if (!claim && !rr[u].done) {
                                                        claim = PIE_TRUE;
                                                        rr[u].done = PIE_TRUE;
                                                }
                                                continue;
                                        }

                        // relay channel/*{{{*/
                        PieSim_copyVectorComplex(slotSize,
                                                 symb + xx,
                                                 rr[u].chan + xx);
                        if (!PieChannel_iFading(slotSize,
                                                rr[u].chan + xx,
                                                &(rr[u].fading),
                                                modem.order,
                                                rr[u].equ + xx)) {
                                printf("fading error\n");
                                return 0;
                        } 
                        if (!PieChannel_AWGN(slotSize, 
                                             rr[u].chan + xx, 
                                             rlsnr[i], 
                                             PIE_CHANNEL_SNR_MODE, 
                                             modem.order)) {
                                printf("awgn error\n");
                                return 0;
                        }
/*}}}*/

                        // relay receiver /*{{{*/
                        // equlize
                        if (!PieEqualizer_equalize(modulatedNumber,
                                                   rr[u].chan,
                                                   rr[u].equ,
                                                   modem.order,
                                                   modulatedNumber,
                                                   echan)) {
                                printf("equlize error\n");
                                return 0;
                        }

                        // component deinterleave/*{{{*/
                        if (!PieSSDInterleaver_combine2(slotSize2,
                                                        echan,
                                                        deitlv2)) {
                                printf("combine 2 error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combine2(slotSize2,
                                                        echan + slotSize2,
                                                        deitlv2 + slotSize2)) {
                                printf("combine 2 error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combineQ(modulatedNumber,
                                                        deitlv2,
                                                        demodIn)) {
                                printf("combine Q error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combine2(slotSize2,
                                                        rr[u].equ,
                                                        deitlv2)) {
                                printf("combine 2 error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combine2(slotSize2,
                                                        rr[u].equ + slotSize2,
                                                        deitlv2 + slotSize2)) {
                                printf("combine 2 error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combineEqu(modulatedNumber,
                                                          deitlv2,
                                                          equQ)) {
                                printf("combine equ error\n");
                                return 0;
                        }
/*}}}*/

                        // demodulate
                        PieModem4D_enableComponents(&modem, compFlag[t][0]);
                        if (!PieModem4D_demodulate(&modem,
                                                   slotSize2,
                                                   demodIn,
                                                   deitlv2,
                                                   equQ,
                                                   rlsnr[i],
                                                   PIE_MODEM_4D_SNR,
                                                   PIE_NULL,
                                                   encodedNumber2,
                                                   demodOut)) {
                                printf("demod error\n");
                                return 0;
                        }
                        PieModem4D_enableComponents(&modem, compFlag[t][1]);
                        if (!PieModem4D_demodulate(&modem,
                                                   slotSize2,
                                                   demodIn + slotSize2,
                                                   deitlv2 + slotSize2,
                                                   equQ + slotSize2,
                                                   rlsnr[i],
                                                   PIE_MODEM_4D_SNR,
                                                   PIE_NULL,
                                                   encodedNumber2,
                                                   demodOut + encodedNumber2)) {
                                printf("demod error\n");
                                return 0;
                        }

                        // reassign bit
                        double *tmpp = demodOut;
                        for (int i = 0; i < transferNumber; i++) {
                                parity1r[i] = tmpp[0];
                                parity2r[i] = tmpp[1];
                                parity3r[i] = tmpp[2];
                                tmpp += codec.parityNumber;
                        }

                        // decode
                        if (!PieTrellis_upperDecode(&codec,
                                                    transferNumber,
                                                    PIE_TRELLIS_TAIL,
                                                    0, 0,
                                                    zero,
                                                    zero + 300,
                                                    parity1r,
                                                    parity2r,
                                                    parity3r,
                                                    zero + 600,
                                                    zero + 900,
                                                    systemLLR,
                                                    systemLLR2,
                                                    parityLLR1,
                                                    parityLLR2,
                                                    parityLLR3)) {
                                printf("decode error\n");
                        }
                        for (int n = 0; n < len / 2; n++) {
                                llr[n * 2] = systemLLR[n];
                                llr[n * 2 + 1] = systemLLR2[n];
                        }
                        int z = PieSim_decideCompare(len, source, llr);
/*}}}*/

                                        if (z == 0) {
                                                rr[u].source = PIE_TRUE;
                                                if (!claim && !rr[u].done) {
                                                        claim = PIE_TRUE;
                                                        rr[u].done = PIE_TRUE;
                                                }
                                        }
                                }
                                // logic
                                if (claim) {
                                        for (int u = 0; u < user; u++) {
                                                rr[u].fading.real = 0;
                                                rr[u].fading.imag = 0;
                                        }
                                        relay[t + 1] = relay[t] + 1;
                                } else
                                        relay[t + 1] = relay[t];
                        }
                        if (relay[3] == 1)
                                relay[3] = 0;
                        int mmm = 0;
                        for (int z = 0; z < 4; z++) {
                                if (relay[z] > mmm)
                                        mmm = relay[z];
                        }
                        full[mmm]++;

/*}}}*/

                        // channel/*{{{*/
                        PieSim_copyVectorComplex(modulatedNumber,
                                                 symb, chan);
                        PieComplex *tmp1 = chan;
                        PieComplex *tmp2 = equ;
                        PieComplex fading[4];
                        for (int k = 0; k < 4; k++) {
                                fading[k].real = 0;
                                fading[k].imag = 0;
                        }
                        for (int k = 0; k < 4; k++) {
                                if (!PieChannel_iFading(slotSize,
                                                        tmp1,
                                                        fading + relay[k],
                                                        modem.order,
                                                        tmp2)) {
                                        printf("fading error\n");
                                        return 0;
                                }
                                tmp1 += slotSize;
                                tmp2 += slotSize;
                        }
                        if (!PieChannel_AWGN(modulatedNumber, 
                                             chan, 
                                             snr[i], 
                                             PIE_CHANNEL_SNR_MODE, 
                                             modem.order)) {
                                printf("awgn error\n");
                                return 0;
                        }
/*}}}*/

                        // data dst/*{{{*/
                        // equlize
                        if (!PieEqualizer_equalize(modulatedNumber,
                                                   chan,
                                                   equ,
                                                   modem.order,
                                                   modulatedNumber,
                                                   echan)) {
                                printf("equlize error\n");
                                return 0;
                        }

                        // component deinterleave/*{{{*/
                        if (!PieSSDInterleaver_combine2(slotSize2,
                                                        echan,
                                                        deitlv2)) {
                                printf("combine 2 error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combine2(slotSize2,
                                                        echan + slotSize2,
                                                        deitlv2 + slotSize2)) {
                                printf("combine 2 error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combineQ(modulatedNumber,
                                                        deitlv2,
                                                        demodIn)) {
                                printf("combine Q error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combine2(slotSize2,
                                                        equ,
                                                        deitlv2)) {
                                printf("combine 2 error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combine2(slotSize2,
                                                        equ + slotSize2,
                                                        deitlv2 + slotSize2)) {
                                printf("combine 2 error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combineEqu(modulatedNumber,
                                                          deitlv2,
                                                          equQ)) {
                                printf("combine equ error\n");
                                return 0;
                        }
/*}}}*/

                        // demodulate
                        PieModem4D_enableComponents(&modem, PIE_MODEM_4D_ALL);
                        if (!PieModem4D_demodulate(&modem,
                                                   modulatedNumber,
                                                   demodIn,
                                                   deitlv2,
                                                   equQ,
                                                   snr[i],
                                                   PIE_MODEM_4D_SNR,
                                                   PIE_NULL,
                                                   encodedNumber,
                                                   demodOut)) {
                                printf("demod error\n");
                                return 0;
                        }

                        // reassign bit
                        double *tmpp = demodOut;
                        for (int i = 0; i < transferNumber; i++) {
                                parity1r[i] = tmpp[0];
                                parity2r[i] = tmpp[1];
                                parity3r[i] = tmpp[2];
                                tmpp += codec.parityNumber;
                        }

                        // decode
                        if (!PieTrellis_upperDecode(&codec,
                                                    transferNumber,
                                                    PIE_TRELLIS_TAIL,
                                                    0, 0,
                                                    zero,
                                                    zero,
                                                    parity1r,
                                                    parity2r,
                                                    parity3r,
                                                    zero,
                                                    zero,
                                                    systemLLR,
                                                    systemLLR2,
                                                    parityLLR1,
                                                    parityLLR2,
                                                    parityLLR3)) {
                                printf("decode error\n");
                                return 0;
                        }
                        for (int n = 0; n < len / 2; n++) {
                                llr[n * 2] = systemLLR[n];
                                llr[n * 2 + 1] = systemLLR2[n];
                        }
/*}}}*/


                        int z = PieSim_decideCompare(len, source, llr);
                        errB += z;
                        if (z)
                                errF++;

// ----------------------------------------------------------------------------
                }
                ber[i] = 1.0 * errB / (pnum[i] * len);
                fer[i] = 1.0 * errF / (pnum[i]);
                printf("\rsnr: %.2f, rl: %.2f, err: %d/%d, ber: %E, fer: %E\n",
                                snr[i], rlsnr[i], errF, pnum[i], ber[i], fer[i]);
                printf("----------------------handsome dividing line-----------------------\n");

                FILE *erOut = fopen(outputFile, "a");
                fprintf(erOut, "snr: %.2f, rl: %.2f, err: %d/%d, ber: %E, fer: %E\n",
                                snr[i], rlsnr[i], errF, pnum[i], ber[i], fer[i]);
                fprintf(erOut, "full %d %d %d %d\n", full[0], full[1], full[2], full[3]);
                fclose(erOut);
        }

        FILE *erOut = fopen(outputFile, "a");
	fprintf(erOut, "\nDL_SNR\t");
	for (int i = 0; i < snrNum; i++)
		fprintf(erOut, "%.2f\t\t", snr[i]);
	fprintf(erOut, "\nRL_SNR\t");
	for (int i = 0; i < snrNum; i++)
		fprintf(erOut, "%.2f\t\t", rlsnr[i]);
	fprintf(erOut, "\nPT_NUM\t");
	for (int i = 0; i < snrNum; i++)
		fprintf(erOut, "%d\t\t", pnum[i]);
	fprintf(erOut, "\nBER");
	for (int i = 0; i < snrNum; i++)
		fprintf(erOut, "%E\t", ber[i]);
	fprintf(erOut, "\nFER");
	for (int i = 0; i < snrNum; i++)
		fprintf(erOut, "%E\t", fer[i]);
	fprintf(erOut, "\n\n");

	fclose(erOut);

        getchar();
        getchar();
        return 1;
}

int main_coop_ssd2()
{
        // init/*{{{*/
        PieIniFile ini;
        if (!PieIniFile_init(&ini, "resource/config.ini")) {
                printf("config file init error\n");
                return 0;
        }

        // init components

        PieModem2D modem;
        if (!PieModem2D_init(&modem, &ini, MODEM_SECTION2)) {
                printf("init modem error\n");
                return 0;
        }

        PieTrellis codec;
        if (!PieTrellis_init(&codec, &ini, CODEC_SECTION)) {
                printf("init codec error\n");
                return 0;
        }

        // init simulation config

        char outputFile[100];
        if (!PieIniFile_getProfileString(&ini,
                        CASE_SECTION, "ssc2", 100, outputFile)) {
                printf("output path error\n");
                return 0;
        }

        double snr[100];
        int snrNum = PieIniFile_getProfileVectorDouble(&ini, 
                        CASE_SECTION, "SNR", 100, snr);
        if (snrNum == 0) {
                printf("snr number is 0\n");
                return 0;
        }

        int pnum[100];
        int tmp = PieIniFile_getProfileVectorInt(&ini,
                        CASE_SECTION, "NUM", 100, pnum);
        if (snrNum != tmp) {
                printf("snr number inconsistent\n");
                return 0;
        }

        int len = PieIniFile_getProfileInt(&ini, CASE_SECTION, "frameLen", 0);
        if (len == 0) {
                printf("frame len error\n");
                return 0;
        }

        int user = PieIniFile_getProfileInt(&ini, CASE_SECTION, "user", 0);
        if (user <= 0) {
                printf("user number error\n");
                return 0;
        }

        int seed = PieIniFile_getProfileInt(&ini, CASE_SECTION, "seed", 0);
        Pie_randomSeed((PieUint64)seed);

        double rlsnr[100];
        int rlnum = PieIniFile_getProfileVectorDouble(&ini,
                        CASE_SECTION, "RL", 100, rlsnr);
        if (rlnum != snrNum) {
                printf("rl snr number error\n");
                return 0;
        }

        double ber[100];
        double fer[100];

        int transferNumber = PieTrellis_getTransferNumber(&codec, 
                        len, PIE_TRUE);
        int encodedNumber = codec.parityNumber * transferNumber;
        int encodedNumber2 = encodedNumber / 2;
        int modulatedNumber = encodedNumber / modem.order;
        int slotSize = modulatedNumber / 2;

/*}}}*/

        // buffer alloc/*{{{*/
        PieUint32 *source = Pie_calloc(len, sizeof *source);
        PieUint32 *system = Pie_calloc(transferNumber * 5, sizeof *system);
        PieUint32 *system2 = system + transferNumber;
        PieUint32 *parity1 = system2 + transferNumber;
        PieUint32 *parity2 = parity1 + transferNumber;
        PieUint32 *parity3 = parity2 + transferNumber;
        PieUint32 *encoded = Pie_calloc(encodedNumber, sizeof *encoded);

        PieComplex *modulated = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *chan = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *symb = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *echan = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *equ = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *equQ = Pie_calloc(modulatedNumber, sizeof *modulated);
        PieComplex *demodIn = Pie_calloc(modulatedNumber, sizeof *modulated);

        double *demodOut = Pie_calloc(encodedNumber, sizeof *demodOut);
        double *decodeIn = Pie_calloc(encodedNumber, sizeof *decodeIn);
        double *parity1r = decodeIn;
        double *parity2r = parity1r + transferNumber;
        double *parity3r = parity2r + transferNumber;
        double *llr = Pie_calloc(len, sizeof *llr);
        double *systemLLR = Pie_calloc(transferNumber * 5, sizeof *systemLLR);
        double *systemLLR2 = systemLLR + transferNumber;
        double *parityLLR1 = systemLLR2 + transferNumber;
        double *parityLLR2 = parityLLR1 + transferNumber;
        double *parityLLR3 = parityLLR2 + transferNumber;
        double *zero = Pie_calloc(3000, sizeof *zero);
        for (int i = 0; i < 3000; i++)
                zero[i] = 0;

        Relay *rr = Pie_calloc(user, sizeof *rr);

/*}}}*/

        printf("simulation on process\n");
        for(int i = 0; i < snrNum; i++) {
                int errB = 0;
                int errF = 0;
                int full[2] = {0, 0};
                for(int j = 0; j < pnum[i]; j++) {
                        printf("snr: %.2f, rl: %.2f err: %d, prg: %d/%d\r", 
                                        snr[i], rlsnr[i], errF, j, pnum[i]);
// ----------------------------------------------------------------------------
                        // data src/*{{{*/
                        // source message
                        if (!PieSim_randomVectorInt(len, source)) {
                                printf("generate source error\n");
                                return 0;
                        }

                        // convolutional code
                        if (!PieTrellis_encode(&codec, 
                                               len, 
                                               source, 
                                               0, 
                                               transferNumber, 
                                               PIE_TRELLIS_TAIL,
                                               0, 
                                               system, 
                                               system2,
                                               parity1, 
                                               parity2, 
                                               parity3)) {
                                printf("encode error\n");
                                return 0;
                        }

                        // bit reassigned
                        PieUint32 *tmp = encoded;
                        for (int i = 0; i < transferNumber; i++) {
                                tmp[0] = parity1[i];
                                tmp[1] = parity2[i];
                                tmp[2] = parity3[i];
                                tmp += codec.parityNumber;
                        }

                        // modulate
                        if (!PieModem2D_modulate(&modem, 
                                                 encodedNumber, 
                                                 encoded, 
                                                 modulatedNumber, 
                                                 modulated)) {
                                printf("modulate error\n");
                                return 0;
                        }

                        // component interleave
                        if (!PieSSDInterleaver_splitQ(modulatedNumber, 
                                                      modulated,
                                                      symb)) {
                                printf("q interleave error\n");
                                return 0;
                        }
/*}}}*/

                        // relay logic/*{{{*/
                        int relay[2];
                        relay[0] = 0;
                        relay[1] = 0;
#ifndef NO_COOP
                        for (int u = 0; u < user; u++) {
                                rr[u].fading.real = 0;
                                rr[u].fading.imag = 0;
                                rr[u].done = PIE_FALSE;
                                rr[u].source = PIE_FALSE;
                        }
                        for (int t = 0; t < 1; t++) {
                                int xx = t * slotSize;
                                PieBoolean claim = PIE_FALSE;
                                for (int u = 0; u < user; u++) {
                                        if (rr[u].source) {
                                                if (!claim && !rr[u].done) {
                                                        claim = PIE_TRUE;
                                                        rr[u].done = PIE_TRUE;
                                                }
                                                continue;
                                        }

                        // relay channel/*{{{*/
                        PieSim_copyVectorComplex(slotSize,
                                                 symb + xx,
                                                 rr[u].chan + xx);
                        if (!PieChannel_iFading(slotSize,
                                                rr[u].chan + xx,
                                                &(rr[u].fading),
                                                modem.order,
                                                rr[u].equ + xx)) {
                                printf("fading error\n");
                                return 0;
                        } 
                        if (!PieChannel_AWGN(slotSize, 
                                             rr[u].chan + xx, 
                                             rlsnr[i], 
                                             PIE_CHANNEL_SNR_MODE, 
                                             modem.order)) {
                                printf("awgn error\n");
                                return 0;
                        }
/*}}}*/

                        // relay receiver /*{{{*/
                        // equlize
                        if (!PieEqualizer_equalize(modulatedNumber,
                                                   rr[u].chan,
                                                   rr[u].equ,
                                                   modem.order,
                                                   modulatedNumber,
                                                   echan)) {
                                printf("equlize error\n");
                                return 0;
                        }

                        // component deinterleave/*{{{*/
                        if (!PieSSDInterleaver_combineQ(modulatedNumber,
                                                        echan,
                                                        demodIn)) {
                                printf("combine Q error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combineEqu(modulatedNumber,
                                                          rr[u].equ,
                                                          equQ)) {
                                printf("combine equ error\n");
                                return 0;
                        }
/*}}}*/

                        // demodulate
                        PieModem2D_setMode(&modem, PIE_MODEM_2D_MODE_ONLY_I);
                        if (!PieModem2D_demodulate(&modem,
                                                   slotSize,
                                                   demodIn,
                                                   rr[u].equ,
                                                   equQ,
                                                   rlsnr[i],
                                                   0,
                                                   PIE_NULL,
                                                   encodedNumber2,
                                                   demodOut)) {
                                printf("demod error\n");
                                return 0;
                        }
                        PieModem2D_setMode(&modem, PIE_MODEM_2D_MODE_ONLY_Q);
                        if (!PieModem2D_demodulate(&modem,
                                                   slotSize,
                                                   demodIn + slotSize,
                                                   rr[u].equ + slotSize,
                                                   equQ + slotSize,
                                                   rlsnr[i],
                                                   0,
                                                   PIE_NULL,
                                                   encodedNumber2,
                                                   demodOut + encodedNumber2)) {
                                printf("demod error\n");
                                return 0;
                        }

                        // reassign bit
                        double *tmpp = demodOut;
                        for (int i = 0; i < transferNumber; i++) {
                                parity1r[i] = tmpp[0];
                                parity2r[i] = tmpp[1];
                                parity3r[i] = tmpp[2];
                                tmpp += codec.parityNumber;
                        }

                        // decode
                        if (!PieTrellis_upperDecode(&codec,
                                                    transferNumber,
                                                    PIE_TRELLIS_TAIL,
                                                    0, 0,
                                                    zero,
                                                    zero + 300,
                                                    parity1r,
                                                    parity2r,
                                                    parity3r,
                                                    zero + 600,
                                                    zero + 900,
                                                    systemLLR,
                                                    systemLLR2,
                                                    parityLLR1,
                                                    parityLLR2,
                                                    parityLLR3)) {
                                printf("decode error\n");
                        }
                        for (int n = 0; n < len / 2; n++) {
                                llr[n * 2] = systemLLR[n];
                                llr[n * 2 + 1] = systemLLR2[n];
                        }
                        int z = PieSim_decideCompare(len, source, llr);
/*}}}*/

                                        if (z == 0) {
                                                rr[u].source = PIE_TRUE;
                                                if (!claim && !rr[u].done) {
                                                        claim = PIE_TRUE;
                                                        rr[u].done = PIE_TRUE;
                                                }
                                        }
                                }
                                // logic
                                if (claim) {
                                        for (int u = 0; u < user; u++) {
                                                rr[u].fading.real = 0;
                                                rr[u].fading.imag = 0;
                                        }
                                        relay[t + 1] = relay[t] + 1;
                                } else
                                        relay[t + 1] = relay[t];
                        }
                        int mmm = 0;
                        for (int z = 0; z < 2; z++) {
                                if (relay[z] > mmm)
                                        mmm = relay[z];
                        }
                        full[mmm]++;
#endif

/*}}}*/

                        // channel/*{{{*/
                        PieSim_copyVectorComplex(modulatedNumber,
                                                 symb, chan);
                        PieComplex *tmp1 = chan;
                        PieComplex *tmp2 = equ;
                        PieComplex fading[2];
                        for (int k = 0; k < 2; k++) {
                                fading[k].real = 0;
                                fading[k].imag = 0;
                        }
                        for (int k = 0; k < 2; k++) {
                                if (!PieChannel_iFading(slotSize,
                                                        tmp1,
                                                        fading + relay[k],
                                                        modem.order,
                                                        tmp2)) {
                                        printf("fading error\n");
                                        return 0;
                                }
                                tmp1 += slotSize;
                                tmp2 += slotSize;
                        }
                        if (!PieChannel_AWGN(modulatedNumber, 
                                             chan, 
                                             snr[i], 
                                             PIE_CHANNEL_SNR_MODE, 
                                             modem.order)) {
                                printf("awgn error\n");
                                return 0;
                        }
/*}}}*/

                        // data dst/*{{{*/
                        // equlize
                        if (!PieEqualizer_equalize(modulatedNumber,
                                                   chan,
                                                   equ,
                                                   modem.order,
                                                   modulatedNumber,
                                                   echan)) {
                                printf("equlize error\n");
                                return 0;
                        }

                        // component deinterleave/*{{{*/
                        if (!PieSSDInterleaver_combineQ(modulatedNumber,
                                                        echan,
                                                        demodIn)) {
                                printf("combine Q error\n");
                                return 0;
                        }
                        if (!PieSSDInterleaver_combineEqu(modulatedNumber,
                                                          equ,
                                                          equQ)) {
                                printf("combine equ error\n");
                                return 0;
                        }
/*}}}*/

                        // demodulate
                        PieModem2D_setMode(&modem, PIE_MODEM_2D_MODE_NORMAL);
                        if (!PieModem2D_demodulate(&modem,
                                                   modulatedNumber,
                                                   demodIn,
                                                   equ,
                                                   equQ,
                                                   snr[i],
                                                   0,
                                                   PIE_NULL,
                                                   encodedNumber,
                                                   demodOut)) {
                                printf("demod error\n");
                                return 0;
                        }

                        // reassign bit
                        double *tmpp = demodOut;
                        for (int i = 0; i < transferNumber; i++) {
                                parity1r[i] = tmpp[0];
                                parity2r[i] = tmpp[1];
                                parity3r[i] = tmpp[2];
                                tmpp += codec.parityNumber;
                        }

                        // decode
                        if (!PieTrellis_upperDecode(&codec,
                                                    transferNumber,
                                                    PIE_TRELLIS_TAIL,
                                                    0, 0,
                                                    zero,
                                                    zero,
                                                    parity1r,
                                                    parity2r,
                                                    parity3r,
                                                    zero,
                                                    zero,
                                                    systemLLR,
                                                    systemLLR2,
                                                    parityLLR1,
                                                    parityLLR2,
                                                    parityLLR3)) {
                                printf("decode error\n");
                                return 0;
                        }
                        for (int n = 0; n < len / 2; n++) {
                                llr[n * 2] = systemLLR[n];
                                llr[n * 2 + 1] = systemLLR2[n];
                        }
/*}}}*/


                        int z = PieSim_decideCompare(len, source, llr);
                        errB += z;
                        if (z)
                                errF++;

// ----------------------------------------------------------------------------
                }
                ber[i] = 1.0 * errB / (pnum[i] * len);
                fer[i] = 1.0 * errF / (pnum[i]);
                printf("\rsnr: %.2f, rl: %.2f, err: %d/%d, ber: %E, fer: %E\n",
                                snr[i], rlsnr[i], errF, pnum[i], ber[i], fer[i]);
                printf("----------------------handsome dividing line-----------------------\n");
                /* printf("full %d / %d\n", full, pnum[i]); */

                FILE *erOut = fopen(outputFile, "a");
                fprintf(erOut, "snr: %.2f, rl: %.2f, err: %d/%d, ber: %E, fer: %E\n",
                                snr[i], rlsnr[i], errF, pnum[i], ber[i], fer[i]);
                fprintf(erOut, "full %d %d\n", full[0], full[1]);
                fclose(erOut);
        }

        FILE *erOut = fopen(outputFile, "a");
	fprintf(erOut, "\nDL_SNR\t");
	for (int i = 0; i < snrNum; i++)
		fprintf(erOut, "%.2f\t\t", snr[i]);
	fprintf(erOut, "\nRL_SNR\t");
	for (int i = 0; i < snrNum; i++)
		fprintf(erOut, "%.2f\t\t", rlsnr[i]);
	fprintf(erOut, "\nPT_NUM\t");
	for (int i = 0; i < snrNum; i++)
		fprintf(erOut, "%d\t\t", pnum[i]);
	fprintf(erOut, "\nBER");
	for (int i = 0; i < snrNum; i++)
		fprintf(erOut, "%E\t", ber[i]);
	fprintf(erOut, "\nFER");
	for (int i = 0; i < snrNum; i++)
		fprintf(erOut, "%E\t", fer[i]);
	fprintf(erOut, "\n\n");

	fclose(erOut);

        getchar();
        getchar();
        return 1;
}
