/* 
 * File:   si5351.cpp
 * Author: JF3HZB / T.UEBO
 *
 * Created on 2020/08/20, 23:07
 * 
 * Get IQ local signal down to 100kHz
 * 
 */



#include <arduino.h>
#include "si5351.h"

#define SDA    48
#define SCL    45

#define pi 3.14159265359
#define PH (pi/2) // PH=90deg
#define df 4      // (Hz)

#define tw 1

uint32_t oMf=-1; 
uint32_t oMc=-1; 
uint32_t oFL=-1;

void wr_I2C(unsigned char d){
    int k;
    for(k=0;k<8;k++){
        if(d & 0x80) digitalWrite(SDA, HIGH); else digitalWrite(SDA, LOW);
        
        delayMicroseconds(tw);
        digitalWrite(SCL, HIGH);
        delayMicroseconds(tw);
        digitalWrite(SCL, LOW);
        delayMicroseconds(tw);
        digitalWrite(SDA, LOW);
        d <<= 1;
    }
    digitalWrite(SCL, HIGH);
    delayMicroseconds(tw);    
    digitalWrite(SCL, LOW);       
}


void cmd_si5351(unsigned char reg_No, unsigned char d){
    digitalWrite(SDA, LOW);  // start condition
    delayMicroseconds(tw);
    digitalWrite(SCL, LOW);  //
    delayMicroseconds(tw);
    
    wr_I2C(0xC0);
    wr_I2C(reg_No);
    wr_I2C(d);
    
    delayMicroseconds(tw);    
    digitalWrite(SCL, HIGH);  // stop condition
    delayMicroseconds(tw);
    digitalWrite(SDA, HIGH);  //
    delayMicroseconds(10+tw);
}



void cmd_si5351_block(unsigned char reg_No, unsigned char *d){
    digitalWrite(SDA, LOW);  // start condition
    delayMicroseconds(tw);
    digitalWrite(SCL, LOW);  //
    delayMicroseconds(tw);
    
    wr_I2C(0xC0);
    wr_I2C(reg_No);
    
    int i;
    for (i=0; i<8; i++) {
      wr_I2C(d[i]);
    }
       
    delayMicroseconds(tw);    
    digitalWrite(SCL, HIGH);  // stop condition
    delayMicroseconds(tw);
    digitalWrite(SDA, HIGH);  //
    delayMicroseconds(10+tw);
}








void set_freq(unsigned long freq){
//    freq [Hz]
//
//    fvco= fxtal*(a+b/c)  ( a:15 -- 90,   b:0 -- 1048575, c:1 -- 1048575 )
//    freq= fvco /(a+b/c)  ( a:4, 6--1800, b:0 -- 1048575, c:1 -- 1048575 )
//
//    P1= 128*a +   floor(128*b/c) - 512
//    P2= 128*b - c*floor(128*b/c)
//    P3= c
//
	
  int k;
  uint32_t M;
  uint32_t R;
  uint32_t FL;

  unsigned long c;  
  unsigned long a;
  unsigned long b;
  unsigned long dd;
  unsigned long P1;
  unsigned long P2;
  unsigned long P3;
  unsigned char dat[8];
      
  if(freq<1500) freq=1500; else if(freq>280000000) freq=280000000;
  
  

  if(freq>=3000000){ //-------------------freq>=3MHz--------------------------------------------------
    FL=0;
    
    if(FL!=oFL){
      cmd_si5351(16,0x4C);      // Enable CLK0 (MS0=Integer Mode, Source=PLL_A) 
      cmd_si5351(18,0x4C);      // Enable CLK2 (MS2=Integer Mode, Source=PLL_A)
    }
     
  	if(    freq> 125000000){M=4; R=0;}
  	else if(freq>=87600000){M=6; R=0;}
    else if(freq>=68000000){M=8; R=0;}
    else if(freq>=55600000){M=10; R=0;}
  	else if(freq>=47100000){M=12; R=0;}
  	else if(freq>=40800000){M=14; R=0;}
  	else if(freq>=36000000){M=16; R=0;}
  	else if(freq>=32300000){M=18; R=0;}
  	else if(freq>=29200000){M=20; R=0;}
  	else if(freq>=26700000){M=22; R=0;}
  	else if(freq>=24600000){M=24; R=0;}
  	else if(freq>=22700000){M=26; R=0;}
  	else if(freq>=21200000){M=28; R=0;}
  	else if(freq>=19700000){M=30; R=0;}
    else if(freq>=18600000){M=32; R=0;}
    else if(freq>=17600000){M=34; R=0;}
  	else if(freq>=16300000){M=36; R=0;}
  	else if(freq>=14700000){M=40; R=0;}
  	else if(freq>=13100000){M=44; R=0;}
  	else if(freq>=11600000){M=50; R=0;}
  	else if(freq>=10100000){M=56; R=0;}
  	else if(freq>= 8520000){M=66; R=0;}
  	else if(freq>= 6970000){M=78; R=0;}
  	else if(freq>= 5430000){M=98; R=0;}
  	else                  {M=127; R=0;}
    
    freq*=M;
  	freq<<=R;
  	
  	c=0xFFFFF;
  	a=freq/fxtal;
  	b=(unsigned long)((double)(freq-a*fxtal)*(double)c/(double)fxtal);
  	dd=(128*b)/c;
  	P1=128*a+dd-512;
  	P2=128*b-c*dd;
  	P3=c;
  	  	
  	//Set fvco of PLL_A
    cmd_si5351(26,(P3>>8)&0xFF);        //MSNA_P3[15:8]
    cmd_si5351(27,P3&0xFF);             //MSNA_P3[7:0]
    cmd_si5351(28,(P1>>16)&0x03);       //MSNA_P1[17:16]
    cmd_si5351(29,(P1>>8)&0xFF);        //MSNA_P1[15:8]
    cmd_si5351(30,P1&0xFF);              //MSNA_P1[7:0]
    cmd_si5351(31,(P3>>12)&0xF0|(P2>>16)&0x0F);//MSNA_P3[19:16], MSNA_P2[19:16]
    cmd_si5351(32,(P2>>8)&0xFF);        //MSNA_P2[15:8]
    cmd_si5351(33,P2&0xFF);             //MSNA_P2[7:0]

    if( (oMf!=M)||(FL!=oFL) ){
    	// Set MS0, MS1
    	// a=M, b=0, c=1 ---> P1=128*M-512, P2=0, P3=1	
    	if(M==4){
    		P1=0;
    		cmd_si5351(42,0);                   //MS0_P3[15:8]
    		cmd_si5351(43,1);                   //MS0_P3[7:0]
    		cmd_si5351(44,0b00001100);  		//0,R0_DIV[2:0],MS0_DIVBY4[1:0],MS0_P1[17:16]
    		cmd_si5351(45,0);        			//MS0_P1[15:8]
    		cmd_si5351(46,0);              		//MS0_P1[7:0]
    		cmd_si5351(47,0);                   //MS0_P3[19:16], MS0_P2[19:16]
    		cmd_si5351(48,0);                   //MS0_P2[15:8]
    		cmd_si5351(49,0);                   //MS0_P2[7:0]
    		
    		cmd_si5351(58,0);                   //MS2_P3[15:8]
    		cmd_si5351(59,1);                   //MS2_P3[7:0]
    		cmd_si5351(60,0b00001100);  		//0,R1_DIV[2:0],MS2_DIVBY4[1:0],MS2_P1[17:16]
    		cmd_si5351(61,0);        			//MS2_P1[15:8]
    		cmd_si5351(62,0);              		//MS2_P1[7:0]
    		cmd_si5351(63,0);                   //MS2_P3[19:16], MS2_P2[19:16]
    		cmd_si5351(64,0);                   //MS2_P2[15:8]
    		cmd_si5351(65,0);                   //MS2_P2[7:0]	
    	}else{
    		P1=128*M-512;
    		cmd_si5351(42,0);                    //MS0_P3[15:8]
    		cmd_si5351(43,1);                    //MS0_P3[7:0]
    		cmd_si5351(44,(R<<4)&0x70|(P1>>16)&0x03);//0,R0_DIV[2:0],MS0_DIVBY4[1:0],MS0_P1[17:16]
    		cmd_si5351(45,(P1>>8)&0xFF);        //MS0_P1[15:8]
    		cmd_si5351(46,P1&0xFF);              //MS0_P1[7:0]
    		cmd_si5351(47,0);                    //MS0_P3[19:16], MS0_P2[19:16]
    		cmd_si5351(48,0);                    //MS0_P2[15:8]
    		cmd_si5351(49,0);                    //MS0_P2[7:0]
    		
    		cmd_si5351(58,0);                    //MS2_P3[15:8]
    		cmd_si5351(59,1);                    //MS2_P3[7:0]
    		cmd_si5351(60,(R<<4)&0x70|(P1>>16)&0x03);//0,R1_DIV[2:0],MS2_DIVBY4[1:0],MS2_P1[17:16]
    		cmd_si5351(61,(P1>>8)&0xFF);        //MS2_P1[15:8]
    		cmd_si5351(62,P1&0xFF);              //MS2_P1[7:0]
    		cmd_si5351(63,0);                    //MS2_P3[19:16], MS2_P2[19:16]
    		cmd_si5351(64,0);                    //MS2_P2[15:8]
    		cmd_si5351(65,0);                    //MS2_P2[7:0]
    	}

    	cmd_si5351(165,0);
    	cmd_si5351(167,0);
    	cmd_si5351(177,0x20);   // Reset PLLA
    }
    
    oMf=M;
  }//------------------- End of set freq>=3MHz----------------------------------------------
  
  else { //-------------------freq<3MHz-----------------------------------------------------
   
    long fvco;

    if     (freq>=1500000){ FL=1; fvco=freq*300;}
    else if(freq>= 500000){ FL=2; fvco=freq*600;}
    else                  { FL=3; fvco=freq*1800;}
    R=0;
    
    if(FL!=oFL){
      double Td=1e6*PH/(2*pi*df);  //convert 90deg to Td(us) 
           
      cmd_si5351(16,0x0C);      // Enable CLK0 (MS0=Fractional Mode, Source=PLL_A) 
      cmd_si5351(18,0x0C);      // Enable CLK2 (MS2=Fractional Mode, Source=PLL_A)

      c=0xFFFFF; 
      a=fvco/fxtal;
      b=(unsigned long)((double)(fvco-a*fxtal)*(double)c/(double)fxtal);
      dd=(128*b)/c;
      P1=128*a+dd-512;
      P2=128*b-c*dd;
      P3=c;
                        
      //Set fvco of PLL_A
      cmd_si5351(26,(P3>>8)&0xFF);        //MSNA_P3[15:8]
      cmd_si5351(27,P3&0xFF);             //MSNA_P3[7:0]
      cmd_si5351(28,(P1>>16)&0x03);       //MSNA_P1[17:16]
      cmd_si5351(29,(P1>>8)&0xFF);        //MSNA_P1[15:8]
      cmd_si5351(30,P1&0xFF);             //MSNA_P1[7:0]
      cmd_si5351(31,(P3>>12)&0xF0|(P2>>16)&0x0F);//MSNA_P3[19:16], MSNA_P2[19:16]
      cmd_si5351(32,(P2>>8)&0xFF);        //MSNA_P2[15:8]
      cmd_si5351(33,P2&0xFF);             //MSNA_P2[7:0] 


      //-------------- shift 90 deg --------------------------------
      c=0xFFFFF; 
      a=fvco/(freq-df );
      b=(long)( (double)(fvco-a*(freq-df))*(double)c/(double)(freq-df) );
      dd=(128*b)/c;
      P1=128*a+dd-512;
      P2=128*b-c*dd;
      P3=c;

      dat[0]=(P3>>8)&0xFF;
      dat[1]=P3&0xFF;
      dat[2]=(R<<4)&0x70|(P1>>16)&0x03;
      dat[3]=(P1>>8)&0xFF;
      dat[4]=P1&0xFF;
      dat[5]=(P3>>12)&0xF0|(P2>>16)&0x0F;
      dat[6]=(P2>>8)&0xFF;
      dat[7]=P2&0xFF;           
      cmd_si5351_block(42,dat);
      cmd_si5351_block(58,dat);
      cmd_si5351(165,0);
      cmd_si5351(167,0);
               
      cmd_si5351(177,0x20);   // Reset PLLA 

      a=fvco/( freq );
      b=(unsigned long)( (double)(fvco-a*freq )*(double)c/(double)(freq) );
      dd=(128*b)/c;
      P1=128*a+dd-512;
      P2=128*b-c*dd;
      P3=c;

      dat[0]=(P3>>8)&0xFF;
      dat[1]=P3&0xFF;
      dat[2]=(R<<4)&0x70|(P1>>16)&0x03;
      dat[3]=(P1>>8)&0xFF;
      dat[4]=P1&0xFF;
      dat[5]=(P3>>12)&0xF0|(P2>>16)&0x0F;
      dat[6]=(P2>>8)&0xFF;
      dat[7]=P2&0xFF;          
      cmd_si5351_block(42, dat);
      delayMicroseconds((int)Td);
      cmd_si5351_block(58, dat);      
      //------------------------------------------------------
    }

    a=fvco/fxtal;
    b=(unsigned long)((double)(fvco-a*fxtal)*(double)c/(double)fxtal);
    dd=(128*b)/c;
    P1=128*a+dd-512;
    P2=128*b-c*dd;
    P3=c;
                      
    //Set freq of PLL_A
    cmd_si5351(26,(P3>>8)&0xFF);        //MSNA_P3[15:8]
    cmd_si5351(27,P3&0xFF);             //MSNA_P3[7:0]
    cmd_si5351(28,(P1>>16)&0x03);       //MSNA_P1[17:16]
    cmd_si5351(29,(P1>>8)&0xFF);        //MSNA_P1[15:8]
    cmd_si5351(30,P1&0xFF);             //MSNA_P1[7:0]
    cmd_si5351(31,(P3>>12)&0xF0|(P2>>16)&0x0F);//MSNA_P3[19:16], MSNA_P2[19:16]
    cmd_si5351(32,(P2>>8)&0xFF);        //MSNA_P2[15:8]
    cmd_si5351(33,P2&0xFF);             //MSNA_P2[7:0]

  } //-------------------------- End of set freq<3MHz---------------------------------------------

  
  oFL=FL;      
}













void set_car_freq(unsigned long freq, unsigned char EN, unsigned char RST){
//    freq [Hz]
//
//    fvco= fxtal*(a+b/c)  ( a:15 -- 90,   b:0 -- 1048575, c:1 -- 1048575 )
//    freq= fvco /(a+b/c)  ( a:4, 6--1800, b:0 -- 1048575, c:1 -- 1048575 )
//
//    P1= 128*a +   floor(128*b/c) - 512
//    P2= 128*b - c*floor(128*b/c)
//    P3= c
//
  
  int k;
  uint32_t M;
  uint32_t R;
  
    if(EN==1){
      cmd_si5351(17,0x6C);   //CLK1

      if(freq<1500) freq=1500; else if(freq>280000000) freq=280000000;

	  if(    freq> 125000000){M=4; R=0;}
      else if(freq>=87600000){M=6; R=0;}
      else if(freq>=68000000){M=8; R=0;}
      else if(freq>=55600000){M=10; R=0;}
      else if(freq>=47100000){M=12; R=0;}
      else if(freq>=40800000){M=14; R=0;}
      else if(freq>=36000000){M=16; R=0;}
      else if(freq>=32300000){M=18; R=0;}
      else if(freq>=29200000){M=20; R=0;}
      else if(freq>=26700000){M=22; R=0;}
      else if(freq>=24600000){M=24; R=0;}
      else if(freq>=22700000){M=26; R=0;}
      else if(freq>=21200000){M=28; R=0;}
      else if(freq>=19700000){M=30; R=0;}
      else if(freq>=18600000){M=32; R=0;}
      else if(freq>=17600000){M=34; R=0;}
      else if(freq>=16300000){M=36; R=0;}
      else if(freq>=14700000){M=40; R=0;}
      else if(freq>=13100000){M=44; R=0;}
      else if(freq>=11600000){M=50; R=0;}
      else if(freq>=10100000){M=56; R=0;}
      else if(freq>= 8520000){M=66; R=0;}
      else if(freq>= 6970000){M=78; R=0;}
      else if(freq>= 5430000){M=98; R=0;}
      else if(freq>= 3880000){M=128; R=0;}
      else if(freq>= 2340000){M=190; R=0;}
      else if(freq>=  797000){M=360; R=0;}
      else if(freq>=  500000){M=800; R=0;}
      else if(freq>=  330000){M=1280; R=0;}
      else if(freq>=  150000){M=1300; R=1;}
      else if(freq>=   67000){M=1500; R=2;}
      else if(freq>=   30300){M=1600; R=3;} 
      else if(freq>=   14000){M=1800; R=4;}
      else if(freq>=    7000){M=1800; R=5;} 
      else if(freq>=    3500){M=1800; R=6;} 
      else{M=1800; R=7;}
      
      freq*=M;
      freq<<=R;
      
      unsigned long c=0xFFFFF;  
      unsigned long a=freq/fxtal;
      unsigned long b=(unsigned long)((double)(freq-a*fxtal)*(double)c/(double)fxtal);
      unsigned long dd=(128*b)/c;
      unsigned long P1=128*a+dd-512;
      unsigned long P2=128*b-c*dd;
      unsigned long P3=c;
      
      
      //Set fvco of PLL_B
        cmd_si5351(34,(P3>>8)&0xFF);        //MSNB_P3[15:8]
        cmd_si5351(35,P3&0xFF);             //MSNB_P3[7:0]
        cmd_si5351(36,(P1>>16)&0x03);       //MSNB_P1[17:16]
        cmd_si5351(37,(P1>>8)&0xFF);        //MSNB_P1[15:8]
        cmd_si5351(38,P1&0xFF);              //MSNB_P1[7:0]
        cmd_si5351(39,(P3>>12)&0xF0|(P2>>16)&0x0F);//MSNB_P3[19:16], MSNB_P2[19:16]
        cmd_si5351(40,(P2>>8)&0xFF);        //MSNB_P2[15:8]
        cmd_si5351(41,P2&0xFF);             //MSNB_P2[7:0]
      
      // Set MS2
      // a=M, b=0, c=1 ---> P1=128*M-512, P2=0, P3=1  
      if(M==4){
        P1=0;
        cmd_si5351(50,0);             //MS1_P3[15:8]
        cmd_si5351(51,1);             //MS1_P3[7:0]
        cmd_si5351(52,0b00001100);    //0,R0_DIV[2:0],MS1_DIVBY4[1:0],MS1_P1[17:16]
        cmd_si5351(53,0);             //MS1_P1[15:8]
        cmd_si5351(54,0);             //MS1_P1[7:0]
        cmd_si5351(55,0);             //MS1_P3[19:16], MS1_P2[19:16]
        cmd_si5351(56,0);             //MS1_P2[15:8]
        cmd_si5351(57,0);             //MS1_P2[7:0]
      }else{
        P1=128*M-512;
        cmd_si5351(50,0);                   //MS1_P3[15:8]
        cmd_si5351(51,1);                   //MS1_P3[7:0]
        cmd_si5351(52,(R<<4)&0x70|(P1>>16)&0x03);//0,R0_DIV[2:0],MS1_DIVBY4[1:0],MS1_P1[17:16]
        cmd_si5351(53,(P1>>8)&0xFF);        //MS1_P1[15:8]
        cmd_si5351(54,P1&0xFF);             //MS1_P1[7:0]
        cmd_si5351(55,0);                   //MS1_P3[19:16], MS1_P2[19:16]
        cmd_si5351(56,0);                   //MS1_P2[15:8]
        cmd_si5351(57,0);                   //MS1_P2[7:0]

      }
          
      if( (oMc!=M)||(RST==1) ){
        //cmd_si5351(177,0x80);   // Reset PLLB
      }
      oMc=M;
       
    }else{
      cmd_si5351(17,0x80);     
    }     
}



void si5351_init(void){
    pinMode(SDA, OUTPUT);
    pinMode(SCL, OUTPUT);
    digitalWrite(SDA, HIGH);
    digitalWrite(SCL, HIGH);
    delay(10);
	  cmd_si5351(183,0b10010010); // CL=8pF
	  cmd_si5351(16,0x80);     	// Disable CLK0
    cmd_si5351(17,0x80);     	// Disable CLK1
    cmd_si5351(18,0x80);      // Disable CLK2
    cmd_si5351(177,0xA0);    	// Reset PLL_A and B	 
    cmd_si5351(16,0x4C);     	// Enable CLK0 (MS0=Integer Mode, Source=PLL_A)	
    cmd_si5351(17,0x80);     	// Disable CLK1 (MS2=Integer Mode, Source=PLL_B)
    cmd_si5351(18,0x4C);      // Enable CLK2 (MS1=Integer Mode, Source=PLL_A)
    cmd_si5351(177,0xA0);     // Reset PLL_A and B
    delay(10);
    set_freq(10000000);
    cmd_si5351(177,0x20);   // Reset PLLA
    delay(10);    
}
