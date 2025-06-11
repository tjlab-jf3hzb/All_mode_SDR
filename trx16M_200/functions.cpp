/* 
    File:   functions.cpp
    Author: JF3HZB / T. Uebo
 
    Created on July 1, 2023
*/


#include "functions.hpp"
#include "fft.h"

void LCD_setup(void)
{
  lcd.init();
  lcd.setColorDepth(16);
  //if (lcd.width() < lcd.height()) { lcd.setRotation(lcd.getRotation()^1); }
  //lcd.setRotation(2);

  for (int i = 0; i < 2; i++)
  {
    sprites[i].setColorDepth(lcd.getColorDepth());
    sprites[i].setFont(&fonts::Font2);
    sprites[i].setTextDatum(textdatum_t::top_right);
  }

  int div = 1;
  for (;;)
  {
    sprite_height = (lcd.height() + div - 1) / div;
    bool fail = false;
    for (int i = 0; !fail && i < 2; ++i)
    {
      fail = !sprites[i].createSprite(lcd.width(), sprite_height);
    }
    if (!fail) break;
    for (int i = 0; i < 2; ++i)
    {
      sprites[i].deleteSprite();
    }
    div++;
  }

  sp.setColorDepth(16);
  sp.setTextDatum(textdatum_t::middle_center);
  lcd.startWrite(); 
}



void METER::draw(float value, int yoff)
{

  // Background Image
  sprites[flip].pushImage(xposition, yposition -yoff, imgWidth, imgHeight, img);

  // Text
  //sprites[flip].setTextColor(cl_Text);
  //sprites[flip].setFont(&fonts::FreeMonoBold9pt7b);
  //sprites[flip].setFont(&fonts::FreeSansBold9pt7b);
  //sprites[flip].setFont(&fonts::FreeSerif9pt7b);
  //sprites[flip].setTextSize(0.9f);
  //sprites[flip].setCursor( 0.5f*(lcd.width()-lcd.textWidth(meter_text) ) + xposition, 65 + yposition - yoff );
  //sprites[flip].printf( meter_text );

  // Needle
  float zoom_x=2.5, zoom_y=80;
  float xc=0.5f* 107.0f;  //64;
  float yc=135 *68.0f/82.0f; //135
  float radius=100;
  float angle;

  angle = 60.0f*(value - 0.5f);

  sp.createSprite(1, 1);
  sp.clear(cl_Needle);
  sp.setPivot(0, -0.5f + radius/zoom_y );
  sp.pushRotateZoomWithAA(&sprites[flip], xc, yc + yposition - yoff, angle, zoom_x, zoom_y, 0);

  // 針のはみ出た部分を消す
  sprites[flip].fillRect(xposition, yposition+imgHeight -yoff, imgWidth, imgHeight-yposition, BGCol);
}
  





void GRAPH::plot(float *xdat, float *ydat, int N, uint32_t cl, int yoff)
{
  float xs,ys,xe,ye;
  float inv_xw = 1.0f/(x_max-x_min);
  float inv_yw = 1.0f/(y_max-y_min);

  for(int i=0; i<N-1; i++)
  {
    xs=xdat[i]; ys=ydat[i]; xe=xdat[i+1]; ye=ydat[i+1];
    if(  (xs>=x_min) && (xe<=x_max) ) 
    {
      if(ys<y_min) ys=y_min;
      if(ys>y_max) ys=y_max;
      if(ye<y_min) ye=y_min;
      if(ye>y_max) ye=y_max;

      xs = (float)((int)(width*(xs-x_min)*inv_xw + 0.5));
      xe = (float)((int)(width*(xe-x_min)*inv_xw + 0.5));
      ys = height*(ys-y_min)*inv_yw;
      ye = height*(ye-y_min)*inv_yw;
      sprites[flip].drawLine(
              xs+xposition, yposition + height - ys - yoff,
              xe+xposition, yposition + height - ye - yoff,
              cl);
    }
  }

}


void GRAPH::frame(uint32_t cl, int yoff)
{
  sprites[flip].drawRect(xposition, yposition- yoff, width+1, height+1, cl);
}


void GRAPH::axis(float dx, float dy,  uint32_t cl, uint32_t cl0, int yoff)
{
  float xs, ys, xe, ye;
  float inv_xw = 1.0f/(x_max-x_min);
  float inv_yw = 1.0f/(y_max-y_min);

  ys = height*(y_max-y_min)*inv_yw;
  ye = 0;

  for(int i = (int)(x_min/dx)-1; i<= (int)(x_max/dx)+1; i++)
  {
    uint32_t color=cl;
    if(i==0) color = cl0; else color=cl;
    xs=dx*i;
    if(xs>x_min && xs<x_max)
    {
      xe=xs;
      xs = (float)((int)(width*(xs-x_min)*inv_xw + 0.5));
      xe = (float)((int)(width*(xe-x_min)*inv_xw + 0.5));

      sprites[flip].drawLine(
        xs+xposition, yposition + height - ys - yoff,
        xe+xposition, yposition + height - ye - yoff,
      color);
    }
  }

}


void GRAPH::clear(uint32_t cl, int yoff)
{
   sprites[flip].fillRect(xposition, yposition- yoff, width, height, cl);
}


void GRAPH::wfall(float *xdat, float *ydat, int N, int yoff)
{ 
  float inv_xw = 1.0f/(x_max-x_min);

  // store data to wfall_buff
  for(int i=0; i<(int)(width+0.5); i++) wfall_buff[i+(int)(Tc*width+0.5)]=-1e32;
  for(int i=0; i<N; i++)
  {
    int x = (int)(width * (xdat[i]-x_min)*inv_xw + 0.5);
    if( x>=0 && x<(int)(width+0.5) )
    {
      int indx = (int)(Tc*width+0.5) + x;
      if(ydat[i] > wfall_buff[indx]) wfall_buff[indx] = ydat[i];
    }
  }
  // draw waterfall
  float cc = Tc;
  int yy, yyy;
  std::uint16_t* rawbuf = (std::uint16_t*)sprites[flip].getBuffer();
  for(int y=0; y<(int)(spheight+0.5); y++)
  {
    yy = (y+ (int)(yposition+height+1.0+0.5) -yoff) * lcd.width();
    yyy = (int)(cc+0.5) * (int)(width+0.5);
    for(int x=0; x<(int)(width+0.5); x++)
    {
      rawbuf[x + (int)(xposition+0.5) + yy]=cmap( wfall_buff[x+yyy] );
    }
    cc=cc - (Yh_sp/spheight) ; if(cc<0) cc = (int)(Yh_sp+0.5)-1;
  }

}


uint16_t GRAPH::cmap(float v){
  uint16_t cR, cG, cB, val;
  v-=wf_min;
  if(v<0) v=0;
  v*=( 1.0f/(wf_max - wf_min) );

  val=(uint16_t)( v*(6*256-101) +100 );

  if(val<256){
    cR=0;
    cG=0;
    cB=val;
  }
  else if(val>=256 && val<2*256){
    cR=0;
    cG=val-256;
    cB=255;      
  }
  else if(val>=2*256 && val<3*256){
    cR=0;
    cG=255;
    cB=3*256-1-val;
  }
  else if(val>=3*256 && val<4*256){
      cR=val-3*256;
      cG=255;
      cB=0;
  }
  else if(val>=4*256 && val<5*256){
      cR=255;
      cG=5*256-1-val;
      cB=0;
  }
  else if(val>=5*256 && val<6*256){
      cR=255;
      cG=0;
      cB=val-5*256;
  }
  else{
    cR=255;
    cG=0;
    cB=255;
  }
  return (lcd.swap565(cR,cG,cB));
}


void GRAPH::passband(float fL, float fH, float fN, bool Notch_ON , int RX_MODE, int yoff)
{ 
  if(RX_MODE != FM)
  {
    float inv_xw = 1.0f/(x_max-x_min);
    float inv_yw = 1.0f/(y_max-y_min);

    if(RX_MODE==CWU || RX_MODE==CWL){fL-=fo_CW; fH-=fo_CW; fN-=fo_CW; }
    if(RX_MODE==USB || RX_MODE==CWU){}
    else if(RX_MODE==LSB || RX_MODE==CWL){fL=-fL; fH=-fH, fN=-fN; }

    float xL = width*(fL-x_min)*inv_xw;
    float xH = width*(fH-x_min)*inv_xw;
    float xN = width*(fN-x_min)*inv_xw;
    float xN2 = 0;
    if(RX_MODE==AM) xN2 = width*(-fN-x_min)*inv_xw;

    if(Notch_ON == true)
    {
      sp.createSprite(1, 1);
      sp.clear(0xFF00FFU);
      sp.setPivot(0, -0.5);
      sp.pushRotateZoomWithAA(&sprites[flip], xN + xposition,  yposition - yoff, 0, 2.0f, height, 0);
      if(RX_MODE == AM)
      sp.pushRotateZoomWithAA(&sprites[flip], xN2 + xposition,  yposition - yoff, 0, 2.0f, height, 0);
    }

    float xzoom = xH-xL;
    if( fabs(xH-xL) < 2.0f )
    {
      xzoom = 2.0f*xzoom/fabs(xzoom);
    }
    sp.createSprite(1, 1);
    sp.clear(cl_PassBand);
    sp.setPivot(-0.5, -0.5);
    sp.pushRotateZoomWithAA(&sprites[flip], xL + xposition,  yposition - yoff, 0, xzoom, height*0.15f, 0);
  }
}


extern float *wf_HANN;
extern float *wf_HAMMING;
extern float *wf_BLACKMAN;
extern float *wf_BLACKMANHARRIS;
extern float *dat;
extern float *tmpRe;
extern float *tmpIm;
extern int f_MODE;
extern float CPLX_coeffs_Re[];
extern float CPLX_coeffs_Im[];

void calc_CMPLX_coeff(float *coeffRe, float *coeffIm, int N, float fL, float fH, float nL, float nH, bool Notch)
{
  float fs = fsample/DOWN_SAMPLE;
  int kL, kH;
  float g=1.0f/(float)N;
  float fnyq = 0.5*fs;
  float inv_df= (float)N/fs;
  float df= fs/(float)N;

  if(f_MODE == USB || f_MODE == LSB || f_MODE == AM)
  {
    kL = (int)( (fL + fnyq)*inv_df );
    kH = (int)( (fH + fnyq)*inv_df );
    for(int i=0; i<N; i++) tmpRe[i]=0;

    for(int i=kL+1; i<=kH; i++) tmpRe[i]=1;
    tmpRe[kL]= tmpRe[kL+1] * ( 1.0f-( (fL + fnyq)*inv_df - (float)kL ) );
    tmpRe[kH+1]= tmpRe[kH] * ( (fH + fnyq)*inv_df - (float)kH );
  }
  else if(f_MODE == CWU || f_MODE == CWL )
  {
    float inv_sgm = 1.0f/( 0.424661*(fH-fL) );
    float fo = 0.5f*(fH+fL);

    //σ=0.424661*Bw   exp(- 0.5 * ( (f-fo)/ σ)＾2 )
    for(int i=0; i<N; i++)
    {
      float f = (float)(i)*df - fnyq;
      tmpRe[i] = exp(-0.5*(f-fo)*(f-fo)*inv_sgm*inv_sgm );
    }

  }


  if(Notch == true)
  {
    kL = (int)( (nL + fnyq)*inv_df );
    kH = (int)( (nH + fnyq)*inv_df );
    for(int i=0; i<N; i++) tmpIm[i]=0;

    for(int i=kL+1; i<=kH; i++) tmpIm[i]=1;
    tmpIm[kL]= tmpIm[kL+1] * ( 1.0f-( (nL + fnyq)*inv_df - (float)kL ) );
    tmpIm[kH+1]= tmpIm[kH] * ( (nH + fnyq)*inv_df - (float)kH );

    for (int i = 0; i < N; i++)
    {
      tmpRe[i] -= tmpIm[i];
      if (tmpRe[i] < 0) tmpRe[i] = 0;
    }
  }

  for(int i=0; i<N; i++) tmpIm[i]=0;

  // IFFT -----------------------------------
  for(int i=0; i<(N>>1); i++)
  {
    dat[2*i]  = tmpRe[(N>>1) + i];
    dat[2*i+1]= tmpIm[(N>>1) + i];
    dat[2*( (N>>1) + i )    ] = tmpRe[i];
    dat[2*( (N>>1) + i ) + 1] = tmpIm[i];
  }

  ifft(dat, N);

  for(int i=0; i<(N>>1); i++)
  { 
    tmpRe[(N>>1) + i] = dat[2*i];
    tmpIm[(N>>1) + i] = dat[2*i+1];
    tmpRe[i] = dat[2*( (N>>1) + i )    ];
    tmpIm[i] = dat[2*( (N>>1) + i ) + 1];
  }
  //-------------------------------------------

  for(int i=0; i<N; i++)
  {
    ////coeffRe[i] = tmpRe[i] * wf_BLACKMANHARRIS[i*(NfftMAX/N)];
    ////coeffIm[i] = tmpIm[i] * wf_BLACKMANHARRIS[i*(NfftMAX/N)];
    coeffRe[i] = tmpRe[i] * wf_HAMMING[i*(NfftMAX/N)];
    coeffIm[i] = tmpIm[i] * wf_HAMMING[i*(NfftMAX/N)];
    //coeffRe[i] = CPLX_coeffs_Re[i];
    //coeffIm[i] = CPLX_coeffs_Im[i];
  }

}

