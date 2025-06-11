/*
  Noise reduction by Spectral Subtraction

    File:   nr_ss.h
    Author: JF3HZB / T. Uebo
 
    Created on June 10, 2025
*/

    if( th_nr>(Base_NR*1.01f) )
    {
      for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++) 
      {
        sigt[pt_w]=RXsig[i];
        pt_w++; if(pt_w==Nstfft) pt_w=0;
      }

      for(int i=0; i<Nstfft; i++) 
      {
        dat_stfft[2*i    ]=sigt[pt_w]*wf_stfft[i];
        dat_stfft[2*i + 1]=0;
        pt_w++; if(pt_w==Nstfft) pt_w = 0;
      }

      // fft
      dsps_fft2r_fc32(dat_stfft, Nstfft);
      dsps_bit_rev2r_fc32(dat_stfft, Nstfft);

      //--- Spectral Subtraction -----------------------------------------
      for(int i=0; i<Nstfft; i++)
      {
        float xabsr = fabs(dat_stfft[2*i  ]);
        float xabsi = fabs(dat_stfft[2*i+1]);
        float amp;
        if(xabsr>xabsi) amp = (0.96043457f)*xabsr + (0.39782422f)*xabsi;
        else            amp = (0.96043457f)*xabsi + (0.39782422f)*xabsr;
        amp *= ( 1.0f/(float)Nstfft );

        float g_reduce = amp*inv_th;
        if(g_reduce>1.0f) g_reduce = 1.0f;      
        dat_stfft[2*i  ] *= g_reduce;
        dat_stfft[2*i+1] *= g_reduce;
      }

      // ifft
      for(int i=0; i<Nstfft; i++) dat_stfft[2*i+1]=-dat_stfft[2*i+1];
      dsps_fft2r_fc32(dat_stfft, Nstfft);
      dsps_bit_rev2r_fc32(dat_stfft, Nstfft);
      for(int i=0; i<Nstfft; i++) dat_stfft[2*i+1]=-dat_stfft[2*i+1];

      for(int i=0; i<Nstfft; i++)
        sigf[pt_frm][i]=dat_stfft[2*i]*(1.0f/(float)Nstfft); //*wf_stfft[i];        
      pt_frm++; if(pt_frm==Nframe) pt_frm=0;

      // overlap-add
      int i_frm[Nframe];
      for(int i=0; i<Nframe; i++)
      {
        int k=pt_frm+i;
        if(k>=Nframe) k-=Nframe;
        i_frm[i]=k;
      }
      for(int i=0; i<BLOCK_SAMPLES/DOWN_SAMPLE; i++)
      {
        #if Nframe == 4
        RXsig[i]=sigf[i_frm[0]][i+3*(Nstfft/Nframe)]+ 
                  sigf[i_frm[1]][i+2*(Nstfft/Nframe)]+ 
                  sigf[i_frm[2]][i+1*(Nstfft/Nframe)]+ 
                  sigf[i_frm[3]][i+0*(Nstfft/Nframe)];
        RXsig[i] *= ATT_Ratio*(1.0f/(float)Nframe);
        #elif Nframe == 2
        RXsig[i]=sigf[i_frm[0]][i+1*(Nstfft/Nframe)]+ 
                  sigf[i_frm[1]][i+0*(Nstfft/Nframe)];
        RXsig[i] *= ATT_Ratio*(1.0f/(float)Nframe);
        #endif
      }
    }