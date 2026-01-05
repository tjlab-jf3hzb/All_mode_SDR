/* 
    File:   functions.hpp
    Author: JF3HZB / T. Uebo
 
    Created on July 1, 2023
*/
#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#include <esp_dsp.h>
#include "prm.h"

#define lim_lvl 0.25e9
#define Th_AGC 0.536871e9
#define Mag_max 2.14758e9
#define S1_Level (142.0f + S1_Level_adj)

#define USB 0
#define LSB 1
#define AM 2
#define FM 3
#define CWU 4
#define CWL 5

#define fsample 120000
#define DOWN_SAMPLE 6
#define BLOCK_SAMPLES (DOWN_SAMPLE*64)

#define BW 8000.0f
#define G_fm 0.4f

#define NfftMAX 4096
#define Nfft 1024

#define Nframe 4
#define Nstfft (64*Nframe) //(Nframe*BLOCK_SAMPLES/DOWN_SAMPLE)
#define Base_NR 1e3

#define Xo_graph 0
#define Yo_graph 68
#define Xw_graph 159
#define Yh_graph 30
#define Yh_sp 29

#define Xo_graph2 107
#define Yo_graph2 (68-35)
#define Xw_graph2 (159-107)
#define Yh_graph2 30

#define BGCol       TFT_BLACK //0x80FF80U
#define TFT_BLACK2  0x0020  //opaque black


extern LGFX lcd;
extern LGFX_Sprite sp;
extern LGFX_Sprite sprites[2];
extern bool flip;
extern int sprite_height;
extern float *wfall_buff; 


void LCD_setup(void);

class METER
{
  private:
  public:
  uint32_t cl_Text = 0xFFFFFFU;  //0x000000U;
  uint32_t cl_Needle = 0xFF4000U;
  char meter_text[10]  = "S/Po";

  float xposition = 0;
  float yposition = 0;

  METER(){}
  void draw(float value, int yoff);  
};



class GRAPH
{
  private:
  uint16_t cmap(float v);

  //uint32_t cl_PassBand = 0x808080U;
  uint32_t cl_PassBand = 0xFFFF00U;

  public:
  float xposition = Xo_graph;
  float yposition = Yo_graph;

  float width = Xw_graph;
  float height = Yh_graph;

  float spheight = Yh_sp*1;

  float x_min= -25e3;
  float x_max= 25e3;
  float y_min= (230 + sp_Reflevel_adj - sp_Range);
  float y_max= (230 + sp_Reflevel_adj);
  float wf_min = y_min; //waterfall
  float wf_max = y_max; //waterfall
  int Tc = 0;

  GRAPH(){
    //
  }

  void clear(uint32_t cl, int yoff);
  void frame(uint32_t cl, int yoff);
  void axis(float dx, float dy,  uint32_t cl, uint32_t cl0, int yoff);
  void plot(float *xdat, float *ydat, int N, uint32_t cl, int yoff);
  void wfall(float *xdat, float *ydat, int N, int yoff); //waterfall graph 
  void passband(float fL, float fH, float fN, bool Notch_ON , int RX_MODE, int yoff);
};

void calc_CMPLX_coeff(float *Re, float *Im, int N, float fL, float fH, float nL, float nH, bool Notch);


#endif	/* FUNCTIONS_H */