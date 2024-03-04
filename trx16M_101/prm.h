#ifndef PRM_H
#define	PRM_H

#include "image/met_image107_3.h"

#include "ST7735S_128x160.hpp"
//#include "ST7735S_128x160_2.hpp"

//----------- Need to edit values -----------------------------//
#define fxtal   25000000   // Ref. frequency of Si5351A [Hz]
#define fcf     16930000  // Filter center frequency [Hz]
#define fadj_TX    0       // Adjust vale for TX frequency [Hz]


//--------- if you need, edit values --------------------------//
// S meter, Band scope
#define S1_Level_adj 0.0f     //[dB]
#define S_Range  3.0f*12.0f   //[dB]
#define sp_Reflevel_adj 0.0f  //[dB]
#define sp_Range 80.0f        //[dB]

// Time constant for meter
#define T_Meter 0.97f
#define T_At_Meter 0.7f


// PBT settings
#define fo_SSB 1400.0f   //[Hz]
#define fw_SSB 2800.0f   //[Hz]
#define fo_AM  0.0f      //[Hz]
#define fw_AM  8000.0f   //[Hz]
#define CW_TONE 700      //[Hz]
#define fo_CW   CW_TONE  //[Hz]
//#define fw_CW   500.0f   //[Hz]
#define fw_CW  1000.0f   //[Hz]   ver. 1.01


// Time constant for AGC
#define T_slowAGC 0.9998f
#define T_normAGC 0.9994f
#define T_fastAGC 0.9990f
#define Comp_rate 0.5f


// Mic. comp. (Limitter)
#define T_micAGC 0.9990f
#define T_At_AGC 0.9f



// RX gain
#define Gain_SSB_CW 50.0f
#define Gain_AM 50.0f
#define Gain_FM 10.0f


// TX para.
#define Gain_MIC 5.0f
#define SIDE_TONE_VOL 0.5f

#define OUTLEVEL_SSB 1.0f
#define OUTLEVEL_AM 1.0f
#define OUTLEVEL_CW 1.0f
#define OUTLEVEL_FM 1.0f

#define AM_Carrier_coeff 0.18e9
#define FM_deviation_coeff 0.002e-5
//------------------------------------------------------//




#define fsx4     480768
#define fs_real (fsx4 / 4)
#define fcar    (fsx4 / 16)

// Output frequency of Si5351A
#define freq_RX (fcf + fcar)
#define freq_TX ( freq_RX - (fsx4) + fadj_TX )

#endif	/* PRM_H */