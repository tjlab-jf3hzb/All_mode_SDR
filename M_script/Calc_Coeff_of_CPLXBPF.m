%--------------------------------------------------------------------
% To generate "CPLX_BPF.h",
%    execute this script on GNU Octave.
%
%      July 19, 2018  by T. Uebo
%--------------------------------------------------------------------

pkg load signal
clear;
close all;
clc;

%--- Modify this section if you need --------
% Define Pass Band: f1(Hz) to f2(Hz)
f1=150;
f2=2920;

%Sampling frequency
fs=20e3;
%Number of Taps
Ndat=512;


%--Set a Window function---
%wf=ones(Ndat,1);
%wf=hanning(Ndat);
%wf=hamming(Ndat);
wf=blackman(Ndat);
%wf=blackmanharris(Ndat);
%--------------------------------------------


%--- Calc. complex FIR coeff. ---------------
if (f1<f2)
  fa=f1+fs/2;
  fb=f2+fs/2;
else
  fa=f2+fs/2;
  fb=f1+fs/2;
end

ka=round( fa/(fs/Ndat) );
kb=round( fb/(fs/Ndat) );
fres=[ zeros(1,ka-1) ones(1,kb+1-ka) zeros(1,Ndat-kb)];
fres=[fres(Ndat/2+1:end) fres(1:Ndat/2)];

imp_res=ifft(fres);

Ich=[real(imp_res(Ndat/2+1:end)) real(imp_res(1:Ndat/2))];
Qch=[imag(imp_res(Ndat/2+1:end)) imag(imp_res(1:Ndat/2))];
Ich=Ich.*wf.'; Qch=Qch.*wf.';


%----Generate coeff. file ----------------------------
FID=fopen('CPLX_BPF.h', 'w');

fprintf(FID, '#define Ncmplx %d\n' , Ndat);

fprintf(FID, 'float CPLX_coeffs_Re[%d]={\n' , Ndat);

for p=[1:Ndat-1]
fprintf(FID,'%e,\n', Ich(p));
end;
fprintf(FID,'%e\n', Ich(Ndat) );
fprintf(FID,'};\n');

fprintf(FID, 'float CPLX_coeffs_Im[%d]={\n' , Ndat);
for p=[1:Ndat-1]
fprintf(FID,'%e,\n', Qch(p) );
end;
fprintf(FID,'%e\n',  Qch(Ndat) );
fprintf(FID,'};\n');

fclose(FID);



%---- Display results ------------------------
figure(1); plot(Ich);
ylabel('FIR coeff. (Real Part)');
figure(2); plot(Qch);
ylabel('FIR coeff. (Imaginary Part)');
figure(3);
[hr,fr]=freqz(Ich+j*Qch,[1], Ndat*16,'whole',fs);
fr=fr-fs/2;
hr=[hr(Ndat*8+1:end); hr(1:Ndat*8)];
plot(fr, 20.*log10( abs(hr)) );
xlabel('Frequency(Hz)');
ylabel('Response(dB)');
grid on;

figure(4)
subplot(1,2,1)
plot(fr, 20.*log10( abs(hr)) );
xlabel('Frequency(Hz)');
ylabel('Response(dB)');
grid on;
axis([f1-500, f1+500, -80 10]);

subplot(1,2,2)
plot(fr, 20.*log10( abs(hr)) );
xlabel('Frequency(Hz)');
ylabel('Response(dB)');
grid on;
axis([f2-500, f2+500, -80 10]);

