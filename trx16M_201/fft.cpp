/*
  FFT, IFFT, 1/sqrt(x)

    File:   fft.cpp
    Author: JF3HZB / T. Uebo
 
    Created on June 10, 2025
*/
#include <Arduino.h>
#include "fft.h"
#include "ejwt.h"

int bit_reverse(int x, int num_bits) {
    int result = 0;
    for (int i = 0; i < num_bits; i++) {
        result = (result << 1) | (x & 1);
        x >>= 1;
    }
    return result;
}

//********************************************************************
void fft(float *a, int Len)
//********************************************************************
{
  int tLen = Len;
  int Nb_fft = 0;
  while(tLen>1){
    tLen>>=1; Nb_fft++;
  }

  int Wstep=Nb_max - Nb_fft;

  // Bit reverse order ------------------------------------
  for (int i = 0; i < Len; i++) {
      int j = bit_reverse(i, Nb_fft);
      if (j > i) {
          // swap a[i] and a[j]
          float xr = a[2*i];
          float xi = a[2*i+1];
          a[2*i] = a[2*j];
          a[2*j] = xr;
          a[2*i+1] = a[2*j+1];
          a[2*j+1] = xi;
      }
  }
    
  // Butterfly -----------------------------------------
  int p = 1<<( Nb_fft - 1);  //  p = Len/2
  int q = 1;
  for (int i=0; i< Nb_fft ; i++){
    int u=0;
    int qq=q<<1;
    for (int j=0; j<p; j++){
      int m=0;
      for (int k=0; k<q; k++){
        int IndxA= u + k;
        int IndxB= IndxA + q;
        int h=m<<Wstep;
        //--- x:=a[IndxB] * W[h] ;
        float xr = a[2*IndxB] * wr[h];
        xr -= a[2*IndxB+1] * wi[h];
        float xi = a[2*IndxB] * wi[h];
        xi += a[2*IndxB+1] * wr[h];
        //--- a[IndxB]:=a[IndxA] - x --------
        a[2*IndxB]= a[2*IndxA] - xr;
        a[2*IndxB+1]= a[2*IndxA+1] - xi;
        //--- a[IndxA]:=a[IndxA] + x --------
        a[2*IndxA] += xr;
        a[2*IndxA+1] += xi;
        m+=p;
      }
      u+=qq;
    }
    p>>=1;
    q<<=1;
  }
  //-------------------------------------------------
  // for(int i=0; i<2*Len; i++) a[i]*=(1.0f/(float)Len);
}
//********************************************************************
//   End of fft
//********************************************************************



//********************************************************************
void ifft(float *a, int Len)
//********************************************************************
{ 
  int tLen = Len;
  int Nb_fft = 0;
  while(tLen>1){
    tLen>>=1; Nb_fft++;
  }

  int Wstep=Nb_max - Nb_fft;

  // Bit reverse order ------------------------------------
  for (int i = 0; i < Len; i++) {
      int j = bit_reverse(i, Nb_fft);
      if (j > i) {
          // swap a[i] and a[j]
          float xr = a[2*i];
          float xi = a[2*i+1];
          a[2*i] = a[2*j];
          a[2*j] = xr;
          a[2*i+1] = a[2*j+1];
          a[2*j+1] = xi;
      }
  }
    
  // Butterfly -----------------------------------------
  int p = 1<<( Nb_fft - 1);  //  p = Len/2
  int q = 1;
  for (int i=0; i< Nb_fft ; i++){
    int u=0;
    int qq=q<<1;
    for (int j=0; j<p; j++){
      int m=0;
      for (int k=0; k<q; k++){
        int IndxA= u + k;
        int IndxB= IndxA + q;
        int h=m<<Wstep;
        //--- x:=a[IndxB] * W[h] ;
        float xr = a[2*IndxB] * wr[h];
        xr -= a[2*IndxB+1] * (-wi[h]);
        float xi = a[2*IndxB] * (-wi[h]);
        xi += a[2*IndxB+1] * wr[h];
        //--- a[IndxB]:=a[IndxA] - x --------
        a[2*IndxB]= a[2*IndxA] - xr;
        a[2*IndxB+1]= a[2*IndxA+1] - xi;
        //--- a[IndxA]:=a[IndxA] + x --------
        a[2*IndxA] += xr;
        a[2*IndxA+1] += xi;
        m+=p;
      }
      u+=qq;
    }
    p>>=1;
    q<<=1;
  }
  //-------------------------------------------------
  for(int i=0; i<2*Len; i++) a[i]*=(1.0f/(float)Len);
}
//********************************************************************
//   End of ifft
//********************************************************************



// https://en.wikipedia.org/wiki/Fast_inverse_square_root
float invsqrt(float a)
{
    int32_t i;
    float y;
    const float threehalfs = 1.5f;
    y = a;
    i = *( int32_t *)&y;
    i = 0x5f3759df - (i >> 1); 
    y = *( float *)&i;
    // Newton's method
    float x2 = 0.5f*a;
    y = y * (threehalfs - (x2 * y * y));   // 1st iteration
    // y = y * (threehalfs - (x2 * y * y));   // 2nd iteration
    return y;
}
