%  This file name: "IIR_HPF_4th_design.m"
%-----------------------------------------------
% To generate "IIR_HPF.h",
%    execute this script on GNU Octave.
%
%      Jan. 5, 2023  by T. Uebo
%-----------------------------------------------
pkg load signal;
clear;
close all;
clc;

%-- Modify this section if you need ----
fs=20e3; %[Hz]
fc=8e3; %{Hz]
Rp=1;  %[dB]
Rs=70; %[dB]
%----------------------------------------

% 4th HPF
ord=4;

% Elliptic LPF
[b,a]=ellip(ord, Rp, Rs, 2*fc/fs, 'high');

% Chebyshev LPF
%[b,a]=cheby1(ord, Rp, 2*fc/fs);

figure(1);
freqz(b,a,2048,fs);

[sos, g]=tf2sos(b,a);
g=g.^(1/(ord/2));
B0=g.*sos(1,1:3);
A0=sos(1,5:6);
B1=g.*sos(2,1:3);
A1=sos(2,5:6);


FID=fopen("IIR_HPF.h", "w");
fprintf(FID,"// %dth IIR Elliptic HPF\n" ,ord);
fprintf(FID,"// fs=%d[Hz]\n" ,fs);
fprintf(FID,"// fc=%d[Hz]\n" ,fc);
fprintf(FID,"// Ripple=%d[dB]\n", Rp);
fprintf(FID,"// Att=%d[dB]\n", Rs);
fprintf(FID,"// \n");
fprintf(FID,"//      b0 + b1*Z^(-1) + b2*z^(-2)  \n");
fprintf(FID,"// H(z)=--------------------------- \n");
fprintf(FID,"//       1 + a1*z^(-1) + a2*z^(-2)  \n");
fprintf(FID,"// \n");
fprintf(FID,"// {b0, b1, b2, a1, a2}\n");

fprintf(FID,"float HPF4_biquad0[] =\n");
fprintf(FID,"{%e,%e,%e,%e,%e};\n",[B0 A0]);

fprintf(FID,"float HPF4_biquad1[] =\n");
fprintf(FID,"{%e,%e,%e,%e,%e};\n",[B1 A1]);


fclose(FID);
