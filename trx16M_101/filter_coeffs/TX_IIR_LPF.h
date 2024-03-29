// 8th IIR Elliptic LPF
// fs=120000[Hz]
// fc=7900[Hz]
// Ripple=1[dB]
// Att=80[dB]
// 
//      b0 + b1*Z^(-1) + b2*z^(-2)  
// H(z)=--------------------------- 
//       1 + a1*z^(-1) + a2*z^(-2)  
// 
// {b0, b1, b2, a1, a2}
float TX_biquad0[] =
{1.229237e-01,2.404400e-02,1.229237e-01,-1.807344e+00,9.421196e-01};
float TX_biquad1[] =
{1.229237e-01,-1.739427e-01,1.229237e-01,-1.810567e+00,8.857124e-01};
float TX_biquad2[] =
{1.229237e-01,-2.049068e-01,1.229237e-01,-1.816329e+00,9.831364e-01};
float TX_biquad3[] =
{1.229237e-01,-2.123802e-01,1.229237e-01,-1.817139e+00,8.347993e-01};
