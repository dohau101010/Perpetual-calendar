#include <16f887.h>
#device adc=10
#fuses hs
#use delay(clock=20m)
#use i2c(master,slow,sda=pin_c4,scl=pin_c3)

#include <ds1307.c>

#define ssdo   pin_c6
#define ssck   pin_c5
#define rck    pin_c7
#define up     pin_b1
#define dw     pin_b2
#define mod    pin_b0

#define warning_Led     pin_e1

unsigned int8  bdn1, bdn2, bdn3, i;
unsigned int8 leg_7seg[4] = {0xff, 0xff, 0xff, 0xff};
unsigned int8 scan_code[5] = {0x08, 0x04, 0x02, 0x01}; 
unsigned int16 tempt, temSet;
int1 statusLedWarn = false;

const unsigned char ma7doan[16]= {
   0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,        
   0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e};

#int_timer1
void itr_timer1()
{
   bdn1++;
   bdn2++;
   bdn3++;
   set_timer1(3036);
}



void put_1byte(unsigned int8 x)
{
   unsigned int8   sb;   
   #bit msb  = x.7
   for (sb=0; sb<8;sb++)
   {                                                 
      output_bit(ssdo,msb);    
      output_low(ssck); 
      output_high(ssck);
      x= x<<1;         
   }
}

void led7seg_display()
{                                                                  
   for(i = 0; i < 4; i++)
   {
      output_low(rck);
      put_1byte(leg_7seg[i]);   
      put_1byte(scan_code[i]);
      output_high(rck);
      
      delay_ms(1);  
      
      output_low(rck);
      put_1byte(0xff);   
      put_1byte(0);
      output_high(rck);
   }
}  

void displayTempt()
{
   tempt = read_adc()/2.046;
   leg_7seg[3] = 0xff -1 -8 -16 -32;
   leg_7seg[2] = 0xff-1 - 2 -32 -64; 
   leg_7seg[1] = ma7doan[tempt%10%10];
   leg_7seg[0] = ma7doan[tempt/10%10];
}
void displayTemptSet()
{
   leg_7seg[3] = 0xff - 64;
   leg_7seg[2] = ma7doan[temSet%10];
   leg_7seg[1] = ma7doan[temSet/10];
   leg_7seg[0] = 0xff - 64;
}
void displayDate()
{
    leg_7seg[3] = ma7doan[thang_ds%16];
    leg_7seg[2] = ma7doan[thang_ds/16]; 
    leg_7seg[1] = ma7doan[ngay_ds%16]&0x7f;
    leg_7seg[0] = ma7doan[ngay_ds/16];
}
void displayTime()
{
    leg_7seg[3] = ma7doan[phut_ds%16];
    leg_7seg[2] = ma7doan[phut_ds/16]; 
    leg_7seg[1] = ma7doan[gio_ds%16]&0x7f;
    leg_7seg[0] = ma7doan[gio_ds/16];
}

void normalDisplay()
{
   if(bdn1 < 10)      displayTempt();
   else if(bdn1 < 20) displayDate();
   else if(bdn1 < 30) displayTime();
   else bdn1 = bdn1 - 30;
}


int1 phim_cd(int1 pin)   
{                    
   if (!input(pin))   
   {        
      delay_ms(20);                                     
      if (!input(pin))
      {
         while(!input(pin)) { led7seg_display();
}
         return 1;      
      }                      
      return 0;   
   }         
  return 0;  
} 

void phim_mod()
{
   if(phim_cd(mod))
   {
      gt_mod++;
      if(gt_mod>5) gt_mod=0;
   }
}
void setButtonAlls()
{
   if(gt_mod == 1)
   {
      if(phim_cd(up) && temSet < 99) temSet++;
      if(phim_cd(dw) && temSet > 0)  temSet--;
      displayTemptSet();
   }
   else if(gt_mod == 2)
   {
      if(phim_cd(up)) tang_ngay();
      if(phim_cd(dw)) giam_ngay();
      displayDate();
   } 
   else if(gt_mod == 3)
   {
      if(phim_cd(up)) tang_thang();
      if(phim_cd(dw)) giam_thang();
      displayDate();
   }
    else if(gt_mod == 4)
   {
      if(phim_cd(up)) tang_gio();
      if(phim_cd(dw)) giam_gio();
      displayTime();
   }
    else if(gt_mod == 5)
   {
      if(phim_cd(up)) tang_phut();
      if(phim_cd(dw)) giam_phut();
      displayTime();
   }
}

void offSetButton()
{
   if(gt_mod == 1)
   {
    leg_7seg[2] = 0xff;
    leg_7seg[1] = 0xff;
   }
   else if(gt_mod == 2)
   {
     leg_7seg[1] = 0xff;
     leg_7seg[0] = 0xff;
   } 
   else if(gt_mod == 3)
   {
     leg_7seg[3] = 0xff;
     leg_7seg[2] = 0xff;
   }
    else if(gt_mod == 4)
   {
     leg_7seg[1] = 0xff;
     leg_7seg[0] = 0xff;
   }
    else if(gt_mod == 5)
   {
      leg_7seg[3] = 0xff;
      leg_7seg[2] = 0xff;
   }
}

void displayBlinkSetButtons()
{
   if(bdn2 < 5)         setButtonAlls();
   else if(bdn2 < 10)   offSetButton();
   else bdn2 = bdn2 - 10;
}

void alarmHighTempt()
{
   if(tempt > temSet) output_bit(warning_Led, statusLedWarn);
   else               output_bit(warning_Led, 0);
}
void main()
{
   set_tris_c(0xff);
   set_tris_e(0x01);
   setup_adc(adc_clock_div_32);        
   setup_adc_ports(san5|vss_vdd); 
   set_adc_channel(5);
   tempt = 0; temSet = 30;
   
   ds1307_setup();
   
   setup_timer_1(t1_internal|t1_div_by_8);
   set_timer1(3036);
   enable_interrupts(global);
   enable_interrupts(int_timer1);
   ds1307_setup();
   ds1307_getTime();
   while(true)
   {
       phim_mod();
       if(gt_mod == 0) 
       {
            if(bdn3 >= 10) 
         {
            bdn3 = bdn3 - 10;
            ds1307_getTime();
            statusLedWarn = ~statusLedWarn;
         }
         alarmHighTempt();
         normalDisplay();
       }
       else            displayBlinkSetButtons();
       led7seg_display();

   }
}
