%--------------------------------------------------------------------
% To generate "AM_LPF.h",
%    execute this script on GNU Octave.
%
%      July 19, 2018  by T. Uebo
%--------------------------------------------------------------------

pkg load signal
clear;
close all;
clc;

%--- Modify this section if you need --------
% Define LPF cutoff: fc(Hz)
fc=6000;

%Sampling frequency
fs=20e3;
%Number of Taps
Ndat=256;


%--Set a Window function---
%wf=ones(Ndat,1);
%wf=hanning(Ndat);
%wf=hamming(Ndat);
wf=blackman(Ndat);
%wf=blackmanharris(Ndat);
%--------------------------------------------


%--- Calc. complex FIR coeff. ---------------
kc=round( fc/(fs/Ndat) );

fres=[ ones(1,kc) zeros(1,Ndat/2-kc)];
fres=[fres fres(end:-1:1)];

imp_res=ifft(fres);

imp_res=[ imp_res(Ndat/2+1:end) imp_res(1:Ndat/2) ];
imp_res = real(imp_res).*wf.';

%----Generate coeff. file ----------------------------
FID=fopen('AM_LPF.h', 'w');

fprintf(FID, '#define Nam %d\n' , Ndat);
fprintf(FID, 'float AM_FIR_coeffs[%d]={\n' , Ndat);
for p=[1:Ndat-1]
fprintf(FID,'%e,\n', imp_res(p));
end;
fprintf(FID,'%e\n', imp_res(Ndat) );
fprintf(FID,'};\n');
fclose(FID);

%---- Display results ------------------------
figure(1); plot(imp_res);
ylabel('FIR coeff. ');

figure(2);
[hr,fr]=freqz(imp_res,[1], Ndat*16,'whole',fs);
fr=fr-fs/2;
hr=[hr(Ndat*8+1:end); hr(1:Ndat*8)];
plot(fr, 20.*log10( abs(hr)) );
xlabel('Frequency(Hz)');
ylabel('Response(dB)');
grid on;


