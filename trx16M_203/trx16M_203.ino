/*
16MHz All mode TRX on ESP32-S3

 by JF3HZB/T.Uebo, Tj Lab
 
ver. 1.00  Feb. 25, 2024
ver. 1.01  Mar.  4, 2024
ver. 1.02  July  8, 2024  Gaussian filter for CW mode
ver. 1.03  July 11, 2024  RX gain for mode and bandwidth adjusted
ver. 1.04  Sep. 25, 2024  si5351.cpp modified
ver. 1.05  Sep. 26, 2024  Frequency shift method modified at CW RX mode
ver. 1.06  Oct.  4, 2024  Display color modified
ver. 1.07  Apr. 16, 2025  Bug fix. Thank you for JA3KAB's contribution.
ver. 2.00  June 10, 2025  AM squelch added. Thank you for JL1VNQ's contribution.
                          and Noise reduction added.
ver. 2.01  June 13, 2025  Slightly modified
ver. 2.02  June 15, 2025  Reduce popping noise when switching TX,RX, and MODE
ver. 2.03  July  4, 2025  S-meter image revised

Arduino IDE 2.3.2

Used processor
 ESP32-S3-WROOM-1-N16R8

Board manager
 ESP32 by Espressif Systems 2.0.9
 (Don't use 2.0.10, 11 ...)

Boad
 ESP32S3 Dev Module

Settings
  Flash size = 16MB
  Partition scheme = 16MB(3MB/APP 9.9MB/FATFS)
  PSRAM  = OPI PSRAM

Used Libraries
 LovyanGFX 1.2.7
 DFRobot_MCP4725 1.0.1

*/

#define NAME "All Mode SDR TRX"
#define VERSION "Ver. 2.03"
#define ID "JF3HZB/Tj Lab"

#include <driver/i2s.h>
#include "serialcommand.h"

#include "functions.hpp"
#include "si5351.h"

#include "Wire.h"
#include "DFRobot_MCP4725.h"
#define  REF_VOLTAGE  3300
#define  DAC_SDA 11
#define  DAC_SCL 10

DFRobot_MCP4725 DAC_S;
DFRobot_MCP4725 DAC_AGC;


LGFX lcd;
LGFX_Sprite sp;
LGFX_Sprite sprites[2];
bool flip;
int sprite_height;
METER S_meter;
GRAPH graph1;
GRAPH graph2;

// Pin assign
#define Shift 7
#define Width 15
#define Notch 16
#define SQL 6
#define MODE_SW 4
#define Rsv 5

#define Po_Meter 9

#define PTT 1
#define KEY 2

#define RX 47
#define TX 21
#define TEST 14
#define MUTE 0


// flags
int f_MODE = 0;
int o_MODE = -1;

bool f_TX = false;
bool o_TX = false;

bool TXEN = false;

bool Rev_SIDEBAND = false;

bool Power_ON = true;

bool f_SQLMUTE = false;
int MUTE_Timer = 400;
int o_MUTE_Timer = 400;

float G_adj = 10.0f;

//buffers
int rxbuf[BLOCK_SAMPLES*2], txbuf[BLOCK_SAMPLES*2];
float Lch_in[BLOCK_SAMPLES],  Rch_in[BLOCK_SAMPLES];
float Lch_out[BLOCK_SAMPLES], Rch_out[BLOCK_SAMPLES];
float RXsig[BLOCK_SAMPLES], I_RXsig[BLOCK_SAMPLES], Q_RXsig[BLOCK_SAMPLES]; 
float TXsig[BLOCK_SAMPLES], I_TXsig[BLOCK_SAMPLES], Q_TXsig[BLOCK_SAMPLES]; 
float I_RXsig0[BLOCK_SAMPLES/DOWN_SAMPLE],  I_RXsig1[BLOCK_SAMPLES/DOWN_SAMPLE];
float Q_RXsig0[BLOCK_SAMPLES/DOWN_SAMPLE],  Q_RXsig1[BLOCK_SAMPLES/DOWN_SAMPLE];
float Iu_TXsig[BLOCK_SAMPLES], Qu_TXsig[BLOCK_SAMPLES];
float HPFout[BLOCK_SAMPLES/DOWN_SAMPLE];
float zDC_AM[BLOCK_SAMPLES/DOWN_SAMPLE];

#define Ndelay BLOCK_SAMPLES
float Delay_sig[Ndelay];
int pt_DL = 0;
float FADE_IN[BLOCK_SAMPLES];
float FADE_OUT[BLOCK_SAMPLES];

#include "filter_coeffs/TX_DECIMATION_LPF.h"
#include "filter_coeffs/RX_DECIMATION_LPF.h"
// Ndec
float z_RXdecRe[Ndec], z_RXdecIm[Ndec], z_TXdec[Ndec];
fir_f32_t TXfir_dec, RXfir_decRe, RXfir_decIm;

#include "filter_coeffs/CPLX_BPF.h"
// Ncmplx
float RX_CPLX_coeffs_Re[Ncmplx];
float RX_CPLX_coeffs_Im[Ncmplx];

float z_firRe[Ncmplx], z_firIm[Ncmplx];
float z_firRe0[Ncmplx], z_firIm0[Ncmplx];
float z_firRe1[Ncmplx], z_firIm1[Ncmplx];
fir_f32_t TXfirRe, TXfirIm;
fir_f32_t RXfirRe, RXfirIm;
fir_f32_t AMfirRe0, AMfirIm0;
fir_f32_t AMfirRe1, AMfirIm1;


// #include "filter_coeffs/AM_LPF.h"
// // Nam
// fir_f32_t sfirAMRe, sfirAMIm;

#include "filter_coeffs/TX_IIR_LPF.h"
#include "filter_coeffs/RX_IIR_LPF.h"
float z_IIR0[2], z_IIR1[2], z_IIR2[2], z_IIR3[2];
float z_IIR4[2], z_IIR5[2], z_IIR6[2], z_IIR7[2];

#include "filter_coeffs/IIR_HPF.h"
float z_HPF0[2], z_HPF1[2];

float zRe=0.0f, zIm=0.0f, zDC=0.0f, zPH=0.0f;
float zMET=0.0f, zAGC=0.0f, zN=0.0f, zRX=0.0f;
float zDemod_AM = 1.0e7;

// for Short-time FFT & IFFT
float dat_stfft[2*Nstfft];
float sigf[Nframe][Nstfft];
float sigt[Nstfft];
float ejwt[2*Nstfft];
float wf_stfft[Nstfft];
int pt_w = 0;
int pt_frm = 0;


// fsample/4 Local OSC.
#define N_LO 4 
float I0[N_LO]={1.0f, 0, -1.0f, 0};
float Q0[N_LO]={0, -1.0f, 0, 1.0f};
int pt_LO = 0;


// Side tone
float TONE_I[Ntone];
float TONE_Q[Ntone];

// Lo. OSC
float LO_I[Ntone];
float LO_Q[Ntone];
float LO_CWU_I[Ntone];
float LO_CWU_Q[Ntone];
float LO_CWL_I[Ntone];
float LO_CWL_Q[Ntone];
int pt_st = 0;


//Ring Buffer
#define Ring_Buf_Size (Nfft+256)
volatile float Spectrum[Ring_Buf_Size];
volatile int pt_Sp = 0;
volatile float Level[Ring_Buf_Size];
volatile int pt_Lvl = 0;
volatile float Noise[Ring_Buf_Size];
volatile int pt_Noise =0;


//AGC
float g_lim=0;

//Noise reduction
float th_nr=0;
float inv_th=0;

// for FFT & IFFT
#include "fft.h"
float *wf_HANN;
float *wf_HAMMING;
float *wf_BLACKMAN;
float *wf_BLACKMANHARRIS;
float *wf_FLATTOP;

#define Analog_Buf_Size 8
volatile int Buf_Shift[Analog_Buf_Size];
volatile int Buf_Width[Analog_Buf_Size];
volatile int Buf_Notch[Analog_Buf_Size];
volatile int Buf_SQL[Analog_Buf_Size];
volatile int Buf_Rsv[Analog_Buf_Size];
volatile int pt_Analog = 0;

//work
float *tmpRe;
float *tmpIm;

TaskHandle_t H_Task[3];


/*-----------------------------------------------------------------------------------------------
  Setup
-------------------------------------------------------------------------------------------------*/
void setup(void) {

  float dt = 2.0f*M_PI/(float)Ntone;
  for(int i=0; i<Ntone; i++){
    TONE_I[i] = cos( (float)i* dt );
    TONE_Q[i] = sin( (float)i* dt );
  } 

  pt_LO = 0;
  for(int i=0; i<Ntone; i++){
    LO_I[i] = I0[pt_LO];
    LO_Q[i] = Q0[pt_LO];
    pt_LO++; if(pt_LO>=N_LO) pt_LO = 0; 
  }

  pt_LO = 0;
  for(int i=0; i<Ntone; i++){
    LO_CWU_I[i] = I0[pt_LO]*TONE_I[i] - Q0[pt_LO]*TONE_Q[i];
    LO_CWU_Q[i] = Q0[pt_LO]*TONE_I[i] + I0[pt_LO]*TONE_Q[i];
    pt_LO++; if(pt_LO>=N_LO) pt_LO = 0; 
  }

  pt_LO = 0;
  for(int i=0; i<Ntone; i++){
    LO_CWL_I[i] = I0[pt_LO]*TONE_I[i] + Q0[pt_LO]*TONE_Q[i];
    LO_CWL_Q[i] = Q0[pt_LO]*TONE_I[i] - I0[pt_LO]*TONE_Q[i];
    pt_LO++; if(pt_LO>=N_LO) pt_LO = 0; 
  }

  for(int i=0; i<BLOCK_SAMPLES; i++){
    FADE_IN[i]= (float)(i+1)/(float)BLOCK_SAMPLES;
    FADE_OUT[i]= (float)(BLOCK_SAMPLES-i-1)/(float)BLOCK_SAMPLES;
  } 

  pinMode(19, OUTPUT);
  pinMode(20, OUTPUT);
  pinMode(TEST, OUTPUT);


  pinMode(RX, OUTPUT);
  pinMode(TX, OUTPUT);
  pinMode(MUTE, OUTPUT);
  digitalWrite(TX, LOW);
  digitalWrite(RX, HIGH);
  digitalWrite(MUTE, HIGH);

  pinMode(PTT, INPUT);
  pinMode(KEY, INPUT);

  pinMode(Shift, ANALOG);
  pinMode(Width, ANALOG);
  pinMode(Notch, ANALOG);
  pinMode(SQL, ANALOG);
  pinMode(MODE_SW, ANALOG);
  pinMode(Rsv, ANALOG);
  pinMode(Po_Meter, ANALOG);

  analogSetAttenuation(ADC_11db);

  xTaskCreatePinnedToCore(alt_task, "alt_task", 4096, NULL, 3, &H_Task[0], 0); 
  xTaskCreatePinnedToCore(SerialCom, "SerialCom", 4096, NULL, 2, &H_Task[1], 0);

  // loop タスクの優先度を設定
  H_Task[2] = xTaskGetCurrentTaskHandle();
  vTaskPrioritySet(H_Task[2], 20);  //1-25


  // I2S setup (Lables are defined in i2s_types.h, esp_intr_alloc.h)----------------------------------------
  i2s_config_t i2s_config = {
    .mode =  (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX  | I2S_MODE_RX),
    .sample_rate = fsample,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LOWMED,
    .dma_buf_count = 16,
    .dma_buf_len = BLOCK_SAMPLES*2,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0,
    .mclk_multiple = I2S_MCLK_MULTIPLE_256,
  };
  i2s_driver_install( I2S_NUM_0, &i2s_config, 0, NULL);

  i2s_pin_config_t pin_config = {
        .mck_io_num = 42,
        .bck_io_num = 40,
        .ws_io_num = 38,
        .data_out_num = 39,
        .data_in_num = 41                                                       
    };
  i2s_set_pin( I2S_NUM_0, &pin_config);



  // set up FIR ------------------------------------------------------------------
  // Decimation  & FM LPF
  dsps_fird_init_f32(&TXfir_dec, TX_DEC_FIR_coeffs, z_TXdec, Ndec, DOWN_SAMPLE, 0);
  dsps_fird_init_f32(&RXfir_decRe, RX_DEC_FIR_coeffs, z_RXdecRe, Ndec, DOWN_SAMPLE, 0);
  dsps_fird_init_f32(&RXfir_decIm, RX_DEC_FIR_coeffs, z_RXdecIm, Ndec, DOWN_SAMPLE, 0);


  // Complex BPF for TX
  dsps_fir_init_f32(&TXfirRe, CPLX_coeffs_Re, z_firRe, Ncmplx);
  dsps_fir_init_f32(&TXfirIm, CPLX_coeffs_Im, z_firIm, Ncmplx);

  // Complex BPF for SSB/CW RX
  dsps_fir_init_f32(&RXfirRe, RX_CPLX_coeffs_Re, z_firRe, Ncmplx);
  dsps_fir_init_f32(&RXfirIm, RX_CPLX_coeffs_Im, z_firIm, Ncmplx);
  //dsps_fir_init_f32(&RXfirRe, CPLX_coeffs_Re, z_firRe, Ncmplx);
  //dsps_fir_init_f32(&RXfirIm, CPLX_coeffs_Im, z_firIm, Ncmplx);


  // Complex BPF for AM RX
  //dsps_fir_init_f32(&sfirAMRe, AM_FIR_coeffs, z_firRe, Nam);
  //dsps_fir_init_f32(&sfirAMIm, AM_FIR_coeffs, z_firIm, Nam);
  dsps_fir_init_f32(&AMfirRe0, RX_CPLX_coeffs_Re, z_firRe0, Ncmplx);
  dsps_fir_init_f32(&AMfirRe1, RX_CPLX_coeffs_Re, z_firRe1, Ncmplx);
  dsps_fir_init_f32(&AMfirIm0, RX_CPLX_coeffs_Im, z_firIm0, Ncmplx);
  dsps_fir_init_f32(&AMfirIm1, RX_CPLX_coeffs_Im, z_firIm1, Ncmplx);

  dsps_fft2r_init_fc32(NULL, Nstfft);

  // generate window function
  for(int i=0; i<=Nstfft; i++)
  {
    float t=(float)i/(float)Nstfft;
    //wf_stfft[i] = 0.54f - 0.46f*cos(2.0f*M_PI*t); //Hamming
    wf_stfft[i] = 0.5f - 0.5f*cos(2.0f*M_PI*t); //Hann
    //wf_stfft[i] = 0.35875f - 0.48828f*cos(2.0f*M_PI*t) + 0.14128f*cos(4.0f*M_PI*t) - 0.01168f*cos(6.0f*M_PI*t); //Balckman-Harris
    //wf_stfft[i] = 1 - 1.93f*cos(2.0f*M_PI*t) + 1.29f*cos(4.0f*M_PI*t) - 0.388f*cos(6.0f*M_PI*t) + 0.032f*cos(8.0*M_PI*t); //Flat-top
  } 
  #define ATT_Ratio sqrt(2.667f) //for Hann window

  LCD_setup();
  lcd.setTextColor(TFT_CYAN);
  lcd.setFont(&fonts::Font2);
  //lcd.setTextSize(1.0f*( lcd.height()/64.0f));
  lcd.setCursor( 0.5f*(lcd.width()-lcd.textWidth(NAME) ), 0.1f*lcd.height() );
  lcd.printf( NAME );
  //lcd.setTextSize(1.0f*( lcd.height()/64.0f));
  lcd.setCursor( 0.5f*(lcd.width()-lcd.textWidth(VERSION) ), 0.4f*lcd.height());
  lcd.printf(VERSION); 
  //lcd.setTextSize(1.0f*(lcd.height()/64.0f));
  lcd.setCursor( 0.5f*(lcd.width()-lcd.textWidth(ID) ), 0.7f*lcd.height());
  lcd.printf(ID);

  //sp.setFont(&fonts::Font6);
  sp.setFont(&fonts::FreeSansBold18pt7b);

  delay(1000);

}


/*-----------------------------------------------------------------------------------------------
  Signal Process Loop
-------------------------------------------------------------------------------------------------*/
void loop(void) {
  o_MUTE_Timer = MUTE_Timer;
  MUTE_Timer--;
  if(MUTE_Timer<0) MUTE_Timer = 0;

  size_t readsize = 0;

  //Input from I2S codec
  esp_err_t rxfb = i2s_read(I2S_NUM_0, &rxbuf[0], BLOCK_SAMPLES*2*4, &readsize, portMAX_DELAY);
  if (rxfb == ESP_OK && readsize==BLOCK_SAMPLES*2*4) 
  {
    digitalWrite(TEST, HIGH);
    int j;

    j=0;
    for (int i=0; i<BLOCK_SAMPLES; i++)
    {
      Lch_in[i] = (float) rxbuf[j];
      Rch_in[i] = (float) rxbuf[j+1];
      j+=2;
    }


    if(f_TX == false)
    {
      //------------------- RX -----------------------------------------------------------------------
      if( o_TX == true || MUTE_Timer>0) g_lim = 0;

      for (int i=0; i<BLOCK_SAMPLES; i++)
      {
        Spectrum[pt_Sp]= Lch_in[i];
        pt_Sp++; if( pt_Sp == Ring_Buf_Size ) pt_Sp=0;
      }

      // Down conversion  fsample/4 to DC(base band)
      if(f_MODE == CWU)
      {
        for(int i=0; i<BLOCK_SAMPLES; i++)
        {
          I_RXsig[i] = Lch_in[i]*LO_CWU_I[pt_LO];
          Q_RXsig[i] = Lch_in[i]*LO_CWU_Q[pt_LO];
          pt_LO++; if(pt_LO>=Ntone) pt_LO=0;
        }
      }
      else if(f_MODE == CWL)
      {
        for(int i=0; i<BLOCK_SAMPLES; i++)
        {
          I_RXsig[i] = Lch_in[i]*LO_CWL_I[pt_LO];
          Q_RXsig[i] = Lch_in[i]*LO_CWL_Q[pt_LO];
          pt_LO++; if(pt_LO>=Ntone) pt_LO=0;
        }
      }
      else
      {
        for(int i=0; i<BLOCK_SAMPLES; i++)
        {
          I_RXsig[i] = Lch_in[i]*LO_I[pt_LO];
          Q_RXsig[i] = Lch_in[i]*LO_Q[pt_LO];
          pt_LO++; if(pt_LO>=Ntone) pt_LO=0;
        }
      }
      

      // Decimation (Down sampling) 
      dsps_fird_f32(&RXfir_decRe, I_RXsig, I_RXsig, BLOCK_SAMPLES);
      dsps_fird_f32(&RXfir_decIm, Q_RXsig, Q_RXsig, BLOCK_SAMPLES);



      float Re, Im;
      // RX SSB/CW --------------------------------
      if(f_MODE == USB || f_MODE == CWU || f_MODE == LSB || f_MODE == CWL)
      {
        dsps_fir_f32(&RXfirRe, I_RXsig, I_RXsig, BLOCK_SAMPLES/DOWN_SAMPLE);
        dsps_fir_f32(&RXfirIm, Q_RXsig, Q_RXsig, BLOCK_SAMPLES/DOWN_SAMPLE);

        if(f_MODE == USB || f_MODE == CWU)
          for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++) RXsig[i] = (I_RXsig[i] - Q_RXsig[i])*G_adj;
        else
          for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++) RXsig[i] = (I_RXsig[i] + Q_RXsig[i])*G_adj;

        //for S-Meter
        for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++)
        {
          float s = RXsig[i]; 
          Level[pt_Lvl] = s*s;
          pt_Lvl++; if(pt_Lvl == Ring_Buf_Size) pt_Lvl = 0;
        }

        // Noise reduction by Spectral Subtraction 
        #include "nr_ss.h"

        // AGC
        float T_R_AGC;
        if(f_MODE == USB || f_MODE == LSB) T_R_AGC = T_slowAGC;
        else T_R_AGC = T_fastAGC;
        for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++)
        {
          RXsig[i] *=Gain_SSB_CW;
          float absl = fabs(RXsig[i])-lim_lvl;
          if( absl<0 ) absl = 0;
          if( absl>g_lim ) g_lim = g_lim*T_At_AGC + absl*(1.0f-T_At_AGC);
          else g_lim*=T_R_AGC;
          float xf = invsqrt(Comp_rate * g_lim+lim_lvl);
          RXsig[i] = RXsig[i] * lim_lvl*xf*xf;
        }
      }
      // RX AM ------------------------------------
      else if(f_MODE==AM)
      {
        dsps_fir_f32(&AMfirRe0, I_RXsig, I_RXsig0, BLOCK_SAMPLES/DOWN_SAMPLE);
        dsps_fir_f32(&AMfirRe1, Q_RXsig, Q_RXsig0, BLOCK_SAMPLES/DOWN_SAMPLE);
        // //dsps_fir_f32(&AMfirIm0, I_RXsig, I_RXsig1, BLOCK_SAMPLES/DOWN_SAMPLE);
        // //dsps_fir_f32(&AMfirIm1, Q_RXsig, Q_RXsig1, BLOCK_SAMPLES/DOWN_SAMPLE);


        float Demod_AM, xd, yd;
        for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++)
        {
          Re=I_RXsig0[i]*G_adj;
          Im=Q_RXsig0[i]*G_adj;
          
          // calc. absolute of complex -------------------
          //Demod_AM = sqrt(Re*Re+Im*Im);
          xd=fabs(Re);
          yd=fabs(Im);
          // // //8 -------
          // if(xd>yd) Demod_AM=(0.96043457f)*xd+(0.39782422f)*yd;
          // else      Demod_AM=(0.96043457f)*yd+(0.39782422f)*xd;
          // // //24 --------
          Demod_AM = 4.2978771f*(xd+yd);
          Demod_AM+= fabs( 0.70710678f*xd - 0.70710678f*yd);
          Demod_AM+= fabs( 0.96592583f*xd - 0.25881905f*yd);
          Demod_AM+= fabs( 0.25881905f*xd - 0.96592583f*yd);
          Demod_AM+= fabs( 0.86602540f*xd - 0.5f*yd);
          Demod_AM+= fabs( 0.5f*xd - 0.86602540f*yd);
          Demod_AM*= 0.13108693f;
          //-----------------------------------------------
          zDC = Demod_AM*0.01 + zDC*0.99; //LPF for getting DC component
          RXsig[i] = Demod_AM - zDC;
          //for S-Meter
          Level[pt_Lvl] = zDC*zDC;
          pt_Lvl++; if(pt_Lvl == Ring_Buf_Size) pt_Lvl = 0;

          RXsig[i] *=Gain_AM;
          zDC_AM[i] = zDC;
        }

        // Noise reduction by Spectral Subtraction
        #include "nr_ss.h"

        // AGC
        for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++)
        {
          float absl = fabs(zDC_AM[i]*Gain_AM)-lim_lvl;
          if( absl<0 ) absl = 0;
          if( absl>g_lim ) g_lim = g_lim*T_At_AGC + absl*(1.0f-T_At_AGC);
          else g_lim*=T_normAGC;
          float xf = invsqrt(Comp_rate * g_lim+lim_lvl);
          RXsig[i] = RXsig[i] * lim_lvl*xf*xf;
        }
      }
      // RX FM ---------------------------------
      else if(f_MODE==FM)
      {
        for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++)
        {
          Re = I_RXsig[i]*zRe + Q_RXsig[i]*zIm;
          Im = Q_RXsig[i]*zRe - I_RXsig[i]*zIm;
          zRe = I_RXsig[i];
          zIm = Q_RXsig[i];
          float Demod_FM = atan2(Im, Re) * (float)(fsample/DOWN_SAMPLE); 
          zDC = Demod_FM*0.003 + zDC*0.997; // LPF for getting DC component
          RXsig[i] = 5000.0f*(Demod_FM - zDC); // Reject DC
          //for S-Meter
          Re = I_RXsig[i]*G_adj;
          Im = Q_RXsig[i]*G_adj;
          Level[pt_Lvl] = Re*Re + Im*Im;
          pt_Lvl++; if(pt_Lvl == Ring_Buf_Size) pt_Lvl = 0;
          // Limitter
          RXsig[i] *=Gain_FM;
        }

        // Noise reduction by Spectral Subtraction
        #include "nr_ss.h"

        for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++)
        {
          float absl = fabs(RXsig[i])-lim_lvl;
          if( absl<0 ) absl = 0; 
          if( absl>g_lim ) g_lim = g_lim*T_At_AGC + absl*(1.0f-T_At_AGC);
          else g_lim*=0.999;
          float xf = invsqrt(Comp_rate * g_lim+lim_lvl);
          RXsig[i] = RXsig[i] * lim_lvl*xf*xf;
        }

        //for SQL
        dsps_biquad_f32(RXsig,  HPFout, BLOCK_SAMPLES/DOWN_SAMPLE, HPF4_biquad0, z_HPF0);
        dsps_biquad_f32(HPFout, HPFout, BLOCK_SAMPLES/DOWN_SAMPLE, HPF4_biquad1, z_HPF1);
        for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++)
        {
          Re = HPFout[i];
          Noise[pt_Noise] = Re*Re;
          pt_Noise++; if(pt_Noise == Ring_Buf_Size) pt_Noise = 0;
        }
      }
      // Demod. output is RXsig[]


      // Up sampLing (Interpolation)
      for(int i=BLOCK_SAMPLES/DOWN_SAMPLE-1; i>=0;  i--)
      {
        Rch_out[i*DOWN_SAMPLE]=DOWN_SAMPLE*RXsig[i];
        Lch_out[i*DOWN_SAMPLE]=0;
        for(int j=1; j<DOWN_SAMPLE; j++)
        {
          Rch_out[i*DOWN_SAMPLE+j]=0;
          Lch_out[i*DOWN_SAMPLE+j]=0;
        }
      }
      // Anti-aliasing Filter
      dsps_biquad_f32(Rch_out, Rch_out, BLOCK_SAMPLES, RX_biquad0, z_IIR0);
      dsps_biquad_f32(Rch_out, Rch_out, BLOCK_SAMPLES, RX_biquad1, z_IIR1);
      dsps_biquad_f32(Rch_out, Rch_out, BLOCK_SAMPLES, RX_biquad2, z_IIR2);
      dsps_biquad_f32(Rch_out, Rch_out, BLOCK_SAMPLES, RX_biquad3, z_IIR3);
    }
    else
    {
      //----------------- TX -------------------------------------------------------------------------
      dsps_fird_f32(&TXfir_dec, Rch_in, TXsig, BLOCK_SAMPLES);

      // Compressor
      for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++)
      {
        TXsig[i] *=Gain_MIC;
        float absl = fabs(TXsig[i])-lim_lvl;
        if( absl<0 ) absl = 0; 
        if(absl>g_lim)
          g_lim = g_lim*T_At_AGC + absl*(1.0f-T_At_AGC);
        else
          g_lim*=T_micAGC;

        float xf = invsqrt(1.0f * g_lim+lim_lvl);
        TXsig[i] = TXsig[i] * lim_lvl*xf*xf;
      }

      // TX SSB ---------------------------------
      if( (f_MODE == USB) || (f_MODE == LSB) )
      {
        for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++){
          TXsig[i] *= 6.0f;
        }
        dsps_fir_f32(&TXfirRe, TXsig, I_TXsig, BLOCK_SAMPLES/DOWN_SAMPLE);
        dsps_fir_f32(&TXfirIm, TXsig, Q_TXsig, BLOCK_SAMPLES/DOWN_SAMPLE);
        // Up sampLing (Interpolation)
        for(int i=BLOCK_SAMPLES/DOWN_SAMPLE-1; i>=0;  i--){
          Iu_TXsig[i*DOWN_SAMPLE]=DOWN_SAMPLE*I_TXsig[i];
          Qu_TXsig[i*DOWN_SAMPLE]=DOWN_SAMPLE*Q_TXsig[i];
          for(int j=1; j<DOWN_SAMPLE; j++){
            Iu_TXsig[i*DOWN_SAMPLE+j]=0;
            Qu_TXsig[i*DOWN_SAMPLE+j]=0;
          }
        }
        // Anti-aliasing Filter
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad0, z_IIR0);
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad1, z_IIR1);
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad2, z_IIR2);
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad3, z_IIR3);
        dsps_biquad_f32(Qu_TXsig, Qu_TXsig, BLOCK_SAMPLES, TX_biquad0, z_IIR4);
        dsps_biquad_f32(Qu_TXsig, Qu_TXsig, BLOCK_SAMPLES, TX_biquad1, z_IIR5);
        dsps_biquad_f32(Qu_TXsig, Qu_TXsig, BLOCK_SAMPLES, TX_biquad2, z_IIR6);
        dsps_biquad_f32(Qu_TXsig, Qu_TXsig, BLOCK_SAMPLES, TX_biquad3, z_IIR7);       
        // Up conversion
        if(f_MODE == LSB)
        {
          for(int i=0; i<BLOCK_SAMPLES; i++){
            Rch_out[i] = 0;
            Lch_out[i] = ( Iu_TXsig[i]*Q0[pt_LO] + Qu_TXsig[i]*I0[pt_LO] ) * OUTLEVEL_SSB;
            pt_LO++; if(pt_LO>=N_LO) pt_LO=0;
          }
        }
        else  //USB
        {
          for(int i=0; i<BLOCK_SAMPLES; i++){
            Rch_out[i] = 0;
            Lch_out[i] = ( Qu_TXsig[i]*Q0[pt_LO] + Iu_TXsig[i]*I0[pt_LO] ) * OUTLEVEL_SSB;
            pt_LO++; if(pt_LO>=N_LO) pt_LO=0;
          }        
        }
      }

      // TX AM -----------------------------
      else if(f_MODE == AM)
      {
        dsps_fir_f32(&TXfirRe, TXsig, TXsig, BLOCK_SAMPLES/DOWN_SAMPLE);
        for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++){
          TXsig[i] = 4.0f * (TXsig[i] + AM_Carrier_coeff);
        }
        // Up sampLing (Interpolation)
        for(int i=BLOCK_SAMPLES/DOWN_SAMPLE-1; i>=0;  i--){
          Iu_TXsig[i*DOWN_SAMPLE]=DOWN_SAMPLE*TXsig[i];
          Qu_TXsig[i*DOWN_SAMPLE]=0;
          for(int j=1; j<DOWN_SAMPLE; j++){
            Iu_TXsig[i*DOWN_SAMPLE+j]=0;
            Qu_TXsig[i*DOWN_SAMPLE+j]=0;
          }
        }
        // Anti-aliasing Filter
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad0, z_IIR0);
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad1, z_IIR1);
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad2, z_IIR2);
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad3, z_IIR3);
        // Up Conversion
        for(int i=0; i<BLOCK_SAMPLES; i++){
          Rch_out[i] = 0;
          Lch_out[i] = ( Iu_TXsig[i] * LO_Q[pt_LO] ) * OUTLEVEL_AM; 
          pt_LO++; if(pt_LO>=N_LO) pt_LO=0;
        }
      }

      // TX FM ---------------------
      else if(f_MODE == FM)
      {
        for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++){
          //Imperfect integrator(1st order LPF)
          // for converting the dimension of TXsig from frequency to phase
          zPH = (FM_deviation_coeff * TXsig[i]) * 0.9 + zPH * 0.1;

          I_TXsig[i] = 1.0e9*cos(zPH);
          Q_TXsig[i] = 1.0e9*sin(zPH);
        }
        // Up sampLing (Interpolation)
        for(int i=BLOCK_SAMPLES/DOWN_SAMPLE-1; i>=0;  i--){
          Iu_TXsig[i*DOWN_SAMPLE]=DOWN_SAMPLE*I_TXsig[i];
          Qu_TXsig[i*DOWN_SAMPLE]=DOWN_SAMPLE*Q_TXsig[i];
          for(int j=1; j<DOWN_SAMPLE; j++){
            Iu_TXsig[i*DOWN_SAMPLE+j]=0;
            Qu_TXsig[i*DOWN_SAMPLE+j]=0;
          }
        }
        // Anti-aliasing Filter
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad0, z_IIR0);
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad1, z_IIR1);
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad2, z_IIR2);
        dsps_biquad_f32(Iu_TXsig, Iu_TXsig, BLOCK_SAMPLES, TX_biquad3, z_IIR3);
        dsps_biquad_f32(Qu_TXsig, Qu_TXsig, BLOCK_SAMPLES, TX_biquad0, z_IIR4);
        dsps_biquad_f32(Qu_TXsig, Qu_TXsig, BLOCK_SAMPLES, TX_biquad1, z_IIR5);
        dsps_biquad_f32(Qu_TXsig, Qu_TXsig, BLOCK_SAMPLES, TX_biquad2, z_IIR6);
        dsps_biquad_f32(Qu_TXsig, Qu_TXsig, BLOCK_SAMPLES, TX_biquad3, z_IIR7);
        // Up conversion
        for(int i=0; i<BLOCK_SAMPLES; i++){
          Rch_out[i] = 0; 
          Lch_out[i] = ( Qu_TXsig[i]*Q0[pt_LO] + Iu_TXsig[i]*I0[pt_LO] ) * OUTLEVEL_FM;
          pt_LO++; if(pt_LO>=N_LO) pt_LO=0;
        }

      }
      // TX CW -----------------------
      else if(f_MODE == CWU || f_MODE == CWL)
      {
        if(digitalRead(KEY) == LOW)
        {
          if(o_MUTE_Timer>0 && MUTE_Timer==0){
            for(int i=0; i<BLOCK_SAMPLES; i++)
            {
              Rch_out[i] = SIDE_TONE_VOL*lim_lvl*TONE_I[pt_st]*FADE_IN[i];
              Lch_out[i] = ( OUTLEVEL_CW * 5.0e9/DOWN_SAMPLE) * Q0[pt_LO]*FADE_IN[i]; 
              pt_LO++; if(pt_LO >= N_LO) pt_LO=0;
              pt_st++; if(pt_st >= Ntone) pt_st = 0;
            }
          }
          else if(o_MUTE_Timer==0 && MUTE_Timer>0)
          {
            for(int i=0; i<BLOCK_SAMPLES; i++)
            {
              Rch_out[i] = SIDE_TONE_VOL*lim_lvl*TONE_I[pt_st]*FADE_OUT[i];
              Lch_out[i] = ( OUTLEVEL_CW * 5.0e9/DOWN_SAMPLE) * Q0[pt_LO]*FADE_OUT[i]; 
              pt_LO++; if(pt_LO >= N_LO) pt_LO=0;
              pt_st++; if(pt_st >= Ntone) pt_st = 0;
            }
          }
          else
          {
            for(int i=0; i<BLOCK_SAMPLES; i++)
            {
              Rch_out[i] = SIDE_TONE_VOL*lim_lvl*TONE_I[pt_st];
              Lch_out[i] = ( OUTLEVEL_CW * 5.0e9/DOWN_SAMPLE) * Q0[pt_LO]; 
              pt_LO++; if(pt_LO >= N_LO) pt_LO=0;
              pt_st++; if(pt_st >= Ntone) pt_st = 0;
            }
          }
        }
        else
        {
          for(int i=0; i<BLOCK_SAMPLES; i++)
          {
            Rch_out[i] = 0;
            Lch_out[i] = 0; 
          }
        }
      }

      // for TX spectrum
      for (int i=0; i<BLOCK_SAMPLES; i++)
      {
        Spectrum[pt_Sp]=Lch_out[i]*0.25f;
        pt_Sp++; if( pt_Sp == Ring_Buf_Size ) pt_Sp=0;
      }

    //-------- End of TX ----------------------------------------------------------
    }


    // // Delay RX signal
    // if( f_TX == false )
    //{
    //  for(int i=0; i<BLOCK_SAMPLES; i++)
    //  {
    //    float tmp_sig = Delay_sig[pt_DL];
    //    Delay_sig[pt_DL] = Rch_out[i];
    //   Rch_out[i] = Delay_sig[pt_DL];
    //    pt_DL++; if(pt_DL>=Ndelay) pt_DL = 0;
    //  }
    //}

     
    if(o_MUTE_Timer>0 && MUTE_Timer==0)
      for(int i=0; i<BLOCK_SAMPLES; i++) Rch_out[i]*=FADE_IN[i];
    else if(o_MUTE_Timer==0 && MUTE_Timer>0)
      for(int i=0; i<BLOCK_SAMPLES; i++) Rch_out[i]*=FADE_OUT[i];


    //Output to I2S codec
    j=0;
    if( (f_TX == false && f_SQLMUTE == true) || MUTE_Timer>0 )
    {
      for (int i=0; i<BLOCK_SAMPLES; i++) 
      {
        zRX = zRX*0.975f; //LPF for AF output
        txbuf[j]   = 0;
        txbuf[j+1] = (int) (zRX);
        j+=2;
      }
    }
    else
    {
      for (int i=0; i<BLOCK_SAMPLES; i++) 
      {
        zRX=Rch_out[i]*0.025f + zRX*0.975f; //LPF for AF output
        txbuf[j]   = (int) Lch_out[i];
        txbuf[j+1] = (int) (zRX);
        j+=2;
      }
    }

    i2s_write( I2S_NUM_0, &txbuf[0], BLOCK_SAMPLES*2*4, &readsize, portMAX_DELAY);
  }

  digitalWrite(TEST, LOW);
}






/*-----------------------------------------------------------------------------------------------

alt_task

-------------------------------------------------------------------------------------------------*/
float *X_dat;
float *Y_dat;
float *dat;
float *pow_sp;
float *wfall_buff;

void alt_task(void *args)
{
  wf_HANN = (float *)ps_malloc( NfftMAX*sizeof(float) );
  wf_HAMMING = (float *)ps_malloc( NfftMAX*sizeof(float) );
  wf_BLACKMAN = (float *)ps_malloc( NfftMAX*sizeof(float) );
  wf_BLACKMANHARRIS = (float *)ps_malloc( NfftMAX*sizeof(float) );
  wf_FLATTOP = (float *)ps_malloc( NfftMAX*sizeof(float) );

  dat = (float *)ps_malloc( NfftMAX*2*sizeof(float) );
 
  X_dat = (float *)ps_malloc( NfftMAX*sizeof(float) );
  Y_dat = (float *)ps_malloc( NfftMAX*sizeof(float) );

  tmpRe = (float *)ps_malloc( NfftMAX*sizeof(float) );
  tmpIm = (float *)ps_malloc( NfftMAX*sizeof(float) );

  pow_sp = (float *)ps_malloc( Nfft*sizeof(float) );
  wfall_buff = (float*)ps_malloc(Xw_graph*Yh_sp * sizeof(float) );
  //wfall_buff = (float*)ps_malloc(16384 * sizeof(float) );

  Wire.setPins(DAC_SDA, DAC_SCL);
  DAC_S.init(MCP4725A0_IIC_Address1, REF_VOLTAGE);
  DAC_AGC.init(MCP4725A0_IIC_Address0, REF_VOLTAGE);

  float fh = 0;
  float fl = 0;

  // generate window function
  for(int i=0; i<=NfftMAX; i++)
  {
    float t=(float)i/(float)NfftMAX;
    wf_HAMMING[i]        = 0.54f - 0.46f*cos(2.0f*M_PI*t);
    //wf_HANN[i]           = 0.5f - 0.5f*cos(2.0f*M_PI*t);
    //wf_BLACKMANHARRIS[i] = 0.35875f - 0.48828f*cos(2.0f*M_PI*t) + 0.14128f*cos(4.0f*M_PI*t) - 0.01168f*cos(6.0f*M_PI*t);
    //wf_FLATTOP[i]        = 1 - 1.93f*cos(2.0f*M_PI*t) + 1.29f*cos(4.0f*M_PI*t) - 0.388f*cos(6.0f*M_PI*t) + 0.032f*cos(8.0*M_PI*t);
  }

  graph1.x_min= -25e3 - 150.0f; //表示位置補正
  graph1.x_max=  25e3 - 150.0f; //表示位置補正
  graph2.xposition = Xo_graph2;
  graph2.yposition = Yo_graph2;
  graph2.width = Xw_graph2;
  graph2.height = Yh_graph2;
  graph2.x_min= -5e3;
  graph2.x_max= 5e3;
  graph2.y_min= graph1.y_min;
  graph2.y_max= graph1.y_max;

  si5351_init();
  set_freq(freq_RX);

  delay(2000);

  for(int i=0; i<Nfft/2; i++){
      X_dat[i]= (float)fsample * ( (float)i/(float)(Nfft) - 0.25f );
  }


  while(1)
  {
    int j;

    // switch TX/RX 
    o_TX = f_TX;
    if
    (   
      TXEN == true ||
      LOW == digitalRead(PTT) || 
      ( (f_MODE == CWU || f_MODE == CWL) && LOW == digitalRead(KEY) )
    )
      f_TX = true;
    else
      f_TX = false;

    if(f_TX == false && o_TX == true ){
      if(f_MODE == CWU || f_MODE == CWL) MUTE_Timer = 15;
      else MUTE_Timer = 15;
      digitalWrite(TX, LOW);
      digitalWrite(RX, HIGH);
      set_freq(freq_RX);
    }
    if(f_TX == true  && o_TX == false){
      if(f_MODE == CWU || f_MODE == CWL) MUTE_Timer = 2;
      else MUTE_Timer = 2;
      set_freq(freq_TX);
      digitalWrite(RX, LOW);
      digitalWrite(TX, HIGH);
    }

    
    // switch MODE
    o_MODE = f_MODE;
    int V_MODE = analogRead(MODE_SW);
    if(V_MODE>=0  && V_MODE<410)          f_MODE = FM;
    else if(V_MODE>= 410  && V_MODE<1229) f_MODE = AM;
    else if(V_MODE>=1229  && V_MODE<2048) f_MODE = LSB;
    else if(V_MODE>=2048  && V_MODE<2867) f_MODE = USB;
    else if(V_MODE>=2867  && V_MODE<3686) f_MODE = CWL;
    else if(V_MODE>=3686  && V_MODE<4096) f_MODE = CWU;


    if(f_MODE != o_MODE) 
    {
      if(MUTE_Timer<50) MUTE_Timer = 50;
    }


    Buf_Shift[pt_Analog] = 0;
    Buf_Width[pt_Analog] = 0;
    Buf_Notch[pt_Analog] = 0;
    Buf_SQL[pt_Analog] = 0;
    Buf_Rsv[pt_Analog] = 0;

    int Nav =32;
    for(int i=0; i<Nav; i++)
    {
    Buf_Shift[pt_Analog] += analogRead(Shift);
    Buf_Width[pt_Analog] += analogRead(Width);
    Buf_Notch[pt_Analog] += analogRead(Notch);
    Buf_SQL[pt_Analog] += analogRead(SQL);
    Buf_Rsv[pt_Analog] += analogRead(Rsv);
    }
    pt_Analog++; if(pt_Analog == Analog_Buf_Size) pt_Analog = 0;

    int V_Shift = 0;
    int V_Width = 0;
    int V_Notch = 0;
    int V_SQL = 0;
    int V_Rsv = 0;
    for(int i=0; i<Analog_Buf_Size; i++)
    {
      V_Shift += Buf_Shift[i];
      V_Width += Buf_Width[i];
      V_Notch += Buf_Notch[i];
      V_SQL += Buf_SQL[i];
      V_Rsv += Buf_Rsv[i];
    }


    V_Shift = (int)( (float)V_Shift/(float)Analog_Buf_Size/(float)Nav + 0.5 );
    V_Width = (int)( (float)V_Width/(float)Analog_Buf_Size/(float)Nav + 0.5 );
    V_Notch = (int)( (float)V_Notch/(float)Analog_Buf_Size/(float)Nav + 0.5 );
    V_Rsv = (int)( (float)V_Rsv/(float)Analog_Buf_Size/(float)Nav + 0.5 );


    float th_nr_val = (Noise_max - Noise_min)*V_Rsv/4096.0f + Noise_min;
    th_nr = pow( 10.0f, th_nr_val );
    inv_th = 1.0f/th_nr;

    //Serial.printf("%e\n", th_nr);

    float dfo = (float)(V_Shift-2048);
    float dfw = (float)(V_Width-4096);
    float fn  = (float)(V_Notch);


    if(f_MODE == USB || f_MODE == LSB)
    {
      dfo = 0.6f*dfo + fo_SSB;
      dfw = 0.6f*dfw + fw_SSB;
      if(dfw<300.0f) dfw=300.0f;
      fl = dfo - 0.5f*dfw;
      fh = dfo + 0.5f*dfw;
    }
    else if(f_MODE == CWU || f_MODE == CWL)
    {
      dfo = 0.4f *dfo + fo_CW;
      dfw = (fwmax_CW-fwmin_CW)*dfw/4096.0f + fwmax_CW;
      if(dfw<fwmin_CW) dfw=fwmin_CW;
      fl = dfo - 0.5f*dfw;
      fh = dfo + 0.5f*dfw;
    }
    else
    {
      dfo = 0.0f*dfo + fo_AM;
      dfw = 2.0f*dfw + fw_AM;
      if(dfw<1000.0f) dfw=1000.0f; 
      fl = dfo - 0.5f*dfw;
      fh = dfo + 0.5f*dfw;
    }

    // Adjust Gain
    if(f_MODE == FM){
      G_adj = 8.0f;
    }
    else if(f_MODE == AM){
      G_adj = 15.0f;
    }
    else{
      float gg= (BW-dfw)*(1.0f/BW);
      G_adj = 10.0f * ( 1.0f*pow(gg, 20) + 1.0f);
    }


    calc_CMPLX_coeff(
        RX_CPLX_coeffs_Re,
        RX_CPLX_coeffs_Im, 
        Ncmplx, 
        fl,
        fh,
        fn - 100.0f, 
        fn + 100.0f, 
        true
    );

    V_SQL = 200 - (int)( (70.0f/4096.0f) * (float)V_SQL/(float)Analog_Buf_Size/(float)Nav + 0.5 );



    int ptc;

    // S/Po Meter -------------------------------------------------
    float S_MET;
    if(f_TX == false && o_TX == false)
    {
      float Sig_power= 0;
      ptc = pt_Lvl;
      ptc -= Nfft; if(ptc < 0) ptc += Ring_Buf_Size;
      for(int i = 0; i<Nfft; i++)
      {
        if( Sig_power < Level[ptc] ) Sig_power = Level[ptc] ;
        ptc++; if(ptc == Ring_Buf_Size) ptc=0;
      }
      S_MET = 10.0f * log10(Sig_power);
      S_MET -= S1_Level; if(S_MET<=0) S_MET = 0;
      S_MET /= S_Range;
    }
    else
    {
      S_MET = (1.0f/4096.0f)*(float)analogRead(Po_Meter);
    }

    if(S_MET>=1.0f) S_MET = 1.0f;
    if(zMET>S_MET)
      zMET = S_MET * (1.0f - T_Meter) + zMET * T_Meter; 
    else
      zMET = S_MET * (1.0f - T_At_Meter) + zMET * T_At_Meter; 
    DAC_S.outputVoltage(zMET*REF_VOLTAGE);



    // SQL ------------------------------------------------------
    if(f_MODE == FM)
    {
      float Noise_power = 0;
      ptc = pt_Noise;
      ptc -= Nfft; if(ptc < 0) ptc += Ring_Buf_Size;
      for(int i = 0; i<Nfft; i++){
        Noise_power += Noise[ptc];
        ptc++; if(ptc == Ring_Buf_Size) ptc=0;
      }
      float NdB = 10*log10( Noise_power);
      if(NdB>0.0f) zN = NdB*0.5 + zN*0.5;
      //Serial.printf("%f, %f\n", zN, NdB);
      if(zN <V_SQL) f_SQLMUTE = false; else f_SQLMUTE = true;
    }
    else if(f_MODE == AM) // by JL1VNQ --------
    {
      int Sig_sq = 190 - (int)(S_MET * 100);
      if(Sig_sq < V_SQL) f_SQLMUTE = false;
      else f_SQLMUTE = true;
    } //----------------------------------------
    else
    {
      f_SQLMUTE = false;
    }





    // Get signal form ring buffer ----------------------------------------
    ptc = pt_Sp;
    ptc -= Nfft; if(ptc < 0) ptc += Ring_Buf_Size;
    j=0;
    float Sig_Mag = 0;
    for(int i = 0; i<Nfft; i++){
      dat[j]   = Spectrum[ptc];    // Re 
      dat[j+1] = 0; //Im

      float absdat = abs(dat[j]);
      if( absdat > Sig_Mag ) Sig_Mag = absdat; 

      ptc++; if(ptc == Ring_Buf_Size) ptc=0;
      j+=2;
    }

    // Output External AGC Voltage
    Sig_Mag -= Th_AGC;
    if( Sig_Mag<0 ) Sig_Mag = 0;
    Sig_Mag /= (Mag_max - Th_AGC);

    if(f_TX == false && MUTE_Timer == 0)
    {
      if(zAGC>Sig_Mag)
        zAGC = Sig_Mag*0.03 + zAGC*0.97; 
      else
        zAGC = Sig_Mag;
    }
    else
    {
      zAGC = 0;
    }

    DAC_AGC.outputVoltage(zAGC*REF_VOLTAGE);


    //------ Spectrum ------------------------------------------------------
    j=0;
    for(int i = 0; i<Nfft; i++){
      float wk = wf_HAMMING[i*(NfftMAX/Nfft)];
      dat[j]   *= wk;
      dat[j+1] *= wk;
      j+=2;
    }

    // FFT
    fft(dat, Nfft);
 
    // power spectrum
    float Re, Im;
    j=0;
    for(int i=0; i<Nfft/2; i++){
      Re=dat[j]; Im=dat[j+1];
      pow_sp[i] = Re*Re + Im*Im;
      Y_dat[i]= 10.0f*log10(pow_sp[i] + 1e-10);
      X_dat[i]= (float)fsample * ( (float)i/(float)(Nfft) - 0.25f );
      j+=2;
    }
    // power spectrum　pow_sp[i] (dB)



    //  Display process ---------------------------------------------------------------
    for (int yoff = 0; yoff < lcd.height(); yoff += sprite_height)
    {
      sprites[flip].clear(BGCol);

      // Draw Meter
      S_meter.draw(zMET, yoff);

      // Draw Graph1
      graph1.clear(0x000000U , yoff);

      graph1.axis(10e3, 0, 0x808080U, cl_CENTER, yoff);
      graph1.plot(X_dat, Y_dat, Nfft/2, 0x00FF00U , yoff);
      graph1.frame(0xFFFFFFU , yoff);
      graph1.wfall(X_dat, Y_dat, Nfft/2, yoff);


      // Draw Graph2
      graph2.clear(0x000000U , yoff);

      graph2.axis(2e3, 0, 0x808080U, cl_CENTER, yoff);
      graph2.plot(X_dat, Y_dat, Nfft/2, 0x00FF00U , yoff);


      if(f_TX == false)
      graph2.passband(fl, fh, fn, true, f_MODE, yoff);

      graph2.frame(0xFFFFFFU , yoff);



      // Display "MODE"
      int xp_mode=108, yp_mode=3;

      sprites[flip].fillRoundRect(xp_mode, yp_mode-yoff, 52, 25, 3, TFT_ORANGE);     
      sprites[flip].setFont(&fonts::FreeSansBold9pt7b);
      sprites[flip].setTextSize(1.0, 1.0);
      sprites[flip].setTextColor(TFT_BLACK);
      
      if(f_MODE == USB)
      {
        sprites[flip].setCursor(xp_mode+6, yp_mode+5 -yoff);
        sprites[flip].print("USB");
      }
      else if(f_MODE == LSB)
      {
        sprites[flip].setCursor(xp_mode+8, yp_mode+5-yoff);
        sprites[flip].print("LSB");
      }
      else if(f_MODE == AM)
      {
        sprites[flip].setCursor(xp_mode+11, yp_mode+5-yoff);
        sprites[flip].print("AM");
      }
      else if(f_MODE == FM)
      {
        sprites[flip].setCursor(xp_mode+13, yp_mode+5-yoff);
        sprites[flip].print("FM");
      }
      else if(f_MODE == CWU)
      {
        sprites[flip].setCursor(xp_mode+5, yp_mode+5-yoff);
        sprites[flip].print("CWU");
      }
      else if(f_MODE == CWL)
      {
        sprites[flip].setCursor(xp_mode+5, yp_mode+5-yoff);
        sprites[flip].print("CWL");
      }

      sprites[flip].pushSprite(&lcd, 0, yoff);
      flip = !flip;
    }
    // End of Display process -------------------------------------------------------------


    graph1.Tc++; if( graph1.Tc== Yh_sp ) graph1.Tc=0;

    delay(5);
  }

}




/*----------------------------------------------------------------------
   Serial Com
------------------------------------------------------------------------*/
#define com_len 128
unsigned char com_buf[com_len];
int cp = 0;
void SerialCom(void *args)
{
  Serial.begin(115200);
  delay(50);

  while (1)
  {

    while (Serial.available()) {
      char tmp[4];
      char c = Serial.read();  // get a character
      // echo back
      if (c == 0x08)  //BS
      {
        tmp[0] = c;
        tmp[1] = 0x20;
        tmp[2] = 0x08;
        tmp[3] = '\0';
        Serial.print(tmp);
      } else {
        tmp[0] = c;
        tmp[1] = '\0';
        Serial.print(tmp);
      }

      //
      if (c == 0x08)  //BS
      {
        if (cp > 0) cp--;
        com_buf[cp] = '\0';
      }
      else if ((c == 0x0d) || (c == 0x0a))
      {
        com_buf[cp] = '\0';
        exe_com(com_buf);
        cp = 0;
      } else {
        com_buf[cp] = c;
        cp++;
        if (cp >= com_len) cp = 0;
      }
    }

    delay(10);   
  }
}
