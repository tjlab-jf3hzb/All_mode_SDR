%----------------------------------------------------------
%  Generator for Octave
%
% Sep. 20, 2019  by T.Uebo
%------------------------------------------------------------

clear;


%-- Color ---------------------------
%1
##cl_bg=[0 0 0];
##cl_scale=[0 1 1];
##cl_over=[0 1 1];
##cl_num=[1 1 0.7];
##cl_text=[0 1 0.5];



%2
##cl_bg=[0 0 0];
##cl_scale=[1 1 1];
##cl_over=[0.6 0 0];
##cl_num=[1 1 1];
##cl_text=[0 0 1];


%%3
##cl_bg=[0 0 0];
##cl_scale=[0 1 0.2];
##cl_over=[0.6 0 0];
##cl_num=[1 1 0.7];
##cl_text=[0 1 0.5];


%4
##cl_bg=[1 1 0.6];
##cl_scale=[0 0 0];
##cl_over=[0.6 0 0];
##cl_num=[0 0 0];
##cl_text=[0 0 0.5];


%5
##cl_bg=[1 1 1];
##cl_scale=[0 0 0];
##cl_over=[0.6 0 0];
##cl_num=[0 0 0];
##cl_text=[0 0 0.5];


%6
cl_bg=[0.4 1 0.4];
cl_scale=[0 0 0];
cl_over=[0.6 0 0];
cl_num=[0 0 0];
cl_text=[0 0 0.5];



%------------------------------------
R_Low=-30/180*pi;
R_High=30/180*pi;
R_Int=0.2;

tick_pitch=0.162;
Zero=0.065;

r=20;
rs=18.7;


figure(1);

set(gcf, 'InvertHardCopy', 'off');
whitebg(cl_bg);

axis off;
%axis([-5 5 r-7 r+3]);
axis([-15 15 5 35]);
pbaspect([1 1 1]);

hold on;


for k=[1:10]

% Main Scale
% Draw Arc ------------------------------------
th=[R_Low:0.001:R_Int];
x=r.*sin(th);
y=r.*cos(th);
plot(x,y,'color',cl_scale, 'linewidth', 10);


th=[R_Int:0.001:R_High];
x=r.*sin(th);
y=r.*cos(th);
plot(x,y,'color',cl_over, 'linewidth', 10);

x=(r+0.4).*sin(th);
y=(r+0.4).*cos(th);
plot(x,y,'color',cl_over, 'linewidth', 10);


% Draw Tick -------------------------------------
rg=[r:0.1:r+2.1];
for th=[R_Low+Zero:tick_pitch:R_Int]
x=rg.*sin(th);
y=rg.*cos(th);
plot(x,y,'color',cl_scale, 'linewidth', 5);
end


rg=[r:0.1:r+1.3];
for th=[ R_Low+Zero + tick_pitch/2 :tick_pitch : R_Int]
x=rg.*sin(th);
y=rg.*cos(th);
plot(x,y,'color',cl_scale, 'linewidth', 3);
end


% Draw Numeral -------------------------------------
rn=r+3.7;
num=1;
for th=[R_Low+Zero:tick_pitch:R_Int]
x=rn.*sin(th);
y=rn.*cos(th);
text(x-0.5,y,num2str(num) , 'color',cl_num, 'fontsize',27, 'FontWeight','bold');
num=num+2;
end



% Sub Scale
% Draw Arc ------------------------------------
th=[R_Low:0.001:R_High];
x=rs.*sin(th);
y=rs.*cos(th);
plot(x,y,'color',cl_scale, 'linewidth', 8);


% Draw Tick -------------------------------------
rg=[rs:-0.1:rs-1.1];
subpitch=(R_High-R_Low)./10.2;
for th=[R_Low+0.01:subpitch:R_High]
x=rg.*sin(th);
y=rg.*cos(th);
plot(x,y,'color',cl_scale, 'linewidth', 3);
end


% Draw Numeral -------------------------------------
rn=rs-2.4;
num=0;
for th=[R_Low+0.01:subpitch.*2:R_High]
x=rn.*sin(th);
y=rn.*cos(th);
text(x-0.7,y,num2str(num) , 'color',cl_num, 'fontsize',25, 'FontWeight','bold');
num=num+2;
end


% Draw Text ------------------------------------------------
%text(-4,r-7.3,'S / RF','color',cl_text, 'fontsize',30, 'FontWeight','bold');


end

figure(1);
hold off;

saveas(gcf,'tmp.bmp');



%----  Clip the image ---------------------------------------------------------
a=imread('tmp.bmp');

xc=245;
yc=200;

b=a(yc+40:end-yc, xc+30:end-xc, :);
figure(2);
imshow(b);
imwrite(b,'met_image.bmp');


%{
-------------------------------------------
Convert bmp to hedder file
https://lang-ship.com/tools/image2data/
-------------------------------------------
%}
