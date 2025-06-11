#include <Arduino.h>
#include "functions.hpp"
#include "fft.h"

extern int f_MODE;
extern bool  TXEN;

extern float *ejwt;
extern float *inv_ejwt;

//-----------------------------------------------------------------------
// コマンド解析と実行
//-----------------------------------------------------------------------
void exe_com(unsigned char *buf)
{
#define Narg 15   // 引数の最大個数（コマンド自体を含む）
#define Nstr 16  // 各引数の最大文字数
  int pt = 0, p_arg = 0, c_arg = 0;
  char arg[Narg][Nstr];

  // buf　を　null('\0') まで読み込みながら引数を分離
  while (1)
  {
    if (buf[pt] == '\0')
    {
      arg[p_arg][c_arg] = '\0';
      p_arg++;
      if (p_arg > Narg) p_arg = Narg;
      break;
    }

    if (buf[pt] == ' ')
    {
      arg[p_arg][c_arg] = '\0';
      p_arg++;
      if (p_arg >= Narg) p_arg = Narg - 1;
      c_arg = 0;
    }
    else
    {
      arg[p_arg][c_arg] = buf[pt];
      c_arg++;
      if (c_arg >= Nstr) c_arg = Nstr - 1;
    }
    pt++;
  }

  // p_arg ：引数の数（コマンド自体を含む）
  //コマンド処理
  //----------------------------------------
  if (strcmp("i", &arg[0][0]) == 0)
  {
    Serial.print("JF3HZB");
    Serial.print("\n\r");
  }
  //----------------------------------------
  else if (strcmp("x", &arg[0][0]) == 0)
  {
    esp_restart();
  }
  //-----------------------------------------
  else if (strcmp("test", &arg[0][0]) == 0)
  {
    uint32_t gain;
    uint32_t ch;
    if(p_arg == 3)
    {
      sscanf(&arg[1][0], "%d", &gain);
      sscanf(&arg[2][0], "%d", &ch);
      Serial.printf("arg=%d, %d, %d\n\r", p_arg, gain, ch);
      Serial.print("Done.\n\r");
    }else
    {
      Serial.print("Invalid!\n\r");
    }
  }
  else if (strcmp("calc", &arg[0][0]) == 0)
  {
    float x;
    sscanf(&arg[1][0], "%f", &x);
    float y=invsqrt(x);
    Serial.printf("%e\n\r", y*y);
  }
  //--------------------------------------------
  else if (strcmp("usb", &arg[0][0]) == 0)
  {
    f_MODE=0;
    Serial.print("USB\n\r");
  }
  else if (strcmp("lsb", &arg[0][0]) == 0)
  {
    f_MODE=1;
    Serial.print("LSB\n\r");
  }
  else if (strcmp("am", &arg[0][0]) == 0)
  {
    f_MODE=2;
    Serial.print("AM\n\r");
  }
  else if (strcmp("fm", &arg[0][0]) == 0)
  {
    f_MODE=3;
    Serial.print("FM\n\r");
  }
  else if (strcmp("t", &arg[0][0]) == 0)
  {
    TXEN=true;
    Serial.print("TX\n\r");
  }
  else if (strcmp("r", &arg[0][0]) == 0)
  {
    TXEN=false;
    Serial.print("RX\n\r");
  }


}