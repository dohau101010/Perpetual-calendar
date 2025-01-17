//dinh nghia dia chi ghi va dia chi doc ds1307
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                    
#define ds13_addr_wr 0xd0
#define ds13_addr_rd 0xd1

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  
//khai bao 2 mang: 
//mang ghi: giay-phut-gio-thu-ngay-thang-nam-madk_msds  
//mang doc: giay-phut-gio-thu-ngay-thang-nam-madk_msds  
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx      
unsigned int8 time_w[9]={0x50,0x58,0x14,0x08,0x05,0x06,0x22,0x90,0x95};  
unsigned int8 time_r[9]={0x30,0x30,0x10,0x08,0x20,0x02,0x20,0x90,0x96};
#define giay_ds  time_r[0]              
#define phut_ds  time_r[1]                  
#define gio_ds   time_r[2]
#define thu_ds   time_r[3]
#define ngay_ds  time_r[4]
#define thang_ds time_r[5]                  
#define nam_ds   time_r[6]

#define ma_qd   time_w[8] 
#define ma_ds   time_r[8]                                       
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  
//cap nhat time: giay-phut-gio-thu-ngay-thang-nam-madk_msds  
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
void ds1307_cap_nhat_time(int1 sel)                 
{         
   unsigned int8 i;   
   i2c_start();
   i2c_write(ds13_addr_wr);        
   i2c_write(0x00);    
   for(i=0;i<9;i++)                          
   {  
      if(sel)  i2c_write(time_w[i]);
      else     i2c_write(time_r[i]);
   }                                 
   i2c_stop();        
}                      
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  
//doc thoi gian   
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                             
void ds1307_thoi_gian(unsigned int8 j)     
{   
   unsigned int8 i; 
   i2c_start();
   i2c_write(ds13_addr_wr);
   i2c_write(0x00);                     
   i2c_start();                
                
   i2c_write(ds13_addr_rd);
   for(i=0;i<j;i++) time_r[i]=i2c_read();  
                                 
   i = i2c_read(0); //not ack, don't care i  
   i2c_stop();
}                                                                       
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx  
//kiem tra ma da luu: sai thi cap nhat, dung thi thoat
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx           
void ds1307_setup()                               
{                                              
   ds1307_thoi_gian(9);          
   if(ma_qd!=ma_ds)   
   {
      ds1307_cap_nhat_time(1);
   }
}
void ds1307_getTime()     
{   
   unsigned int8 i; 
   i2c_start();
   i2c_write(ds13_addr_wr);
   i2c_write(0x00);                     
   i2c_start();                
                
   i2c_write(ds13_addr_rd);
   for(i=0;i<8;i++) time_r[i]=i2c_read();  
                                 
   i = i2c_read(0); //not ack, don't care i  
   i2c_stop();
}











signed int8 gt_mod=0;
//signed int8  gt_mod=0, tg_chinh=0;
#define tang 0
#define giam 1

/*Ham co chuc nang nhan dia chi va du lieu de luu vao dia chi tuong ung roi thuc hien*/
void ds1307_luu_vao_ds(unsigned int8 dcdt,dt)
{
   i2c_start();
   i2c_write(ds13_addr_wr);
   i2c_write(dcdt); //dia chi doi tuong
   i2c_write(dt); //goi doi tuong (giay_ds) ra ds1307
   i2c_stop();
}

unsigned int8 tang_or_giam_so_bcd(unsigned int8 x, int1 sel)
{
   unsigned int8 y,dv,ch,v;
   ch=(x>>4);  dv=x&0x0f;
   y=ch*10+dv;
   if(sel==0)  y++;
   else        y--;
   ch=y/10; dv=y%10;
   v=(ch<<4)|dv;
   return(v);
}

void tang_giay()
{
   if(giay_ds==0x59) giay_ds=0;
   else giay_ds=tang_or_giam_so_bcd(giay_ds,tang);
   ds1307_luu_vao_ds(0,giay_ds); //0-la dia chi cua giay; tham so giay_ds moi vua tang xong
   //trao 2 tham so: ... thong so 2 la gia tri cua giay can luu tru vao o nho co dia chi 0
}
void tang_phut()
{
   if(phut_ds==0x59) phut_ds=0;
   else phut_ds=tang_or_giam_so_bcd(phut_ds,tang);
   ds1307_luu_vao_ds(1,phut_ds); //1-dia chi cua phut
}
void tang_gio()
{
   if(gio_ds==0x23) gio_ds=0;
   else gio_ds=tang_or_giam_so_bcd(gio_ds,tang);
   ds1307_luu_vao_ds(2,gio_ds); //2-dia chi cua gio
}
void chinh_tang_gpg()
{
   switch(gt_mod)
   {
      case 3: tang_phut();
              break;
      case 2: tang_gio();
              break;
      default:break;
   }
}

void tang_nam()
{
   if(nam_ds==0x99) nam_ds=0;
   else nam_ds=tang_or_giam_so_bcd(nam_ds,tang);
   ds1307_luu_vao_ds(6,nam_ds); 
}
void tang_thang()
{
   if(thang_ds==0x12) thang_ds=1;
   else thang_ds=tang_or_giam_so_bcd(thang_ds,tang);
   ds1307_luu_vao_ds(5,thang_ds); 
}

void tang_ngay()
{
   if(ngay_ds==0x31) ngay_ds=1;
   else ngay_ds=tang_or_giam_so_bcd(ngay_ds,tang);
   ds1307_luu_vao_ds(4,ngay_ds); 
}


void chinh_tang_thu()
{
   if(thu_ds==8)  thu_ds=2;
   else           thu_ds++;
   ds1307_luu_vao_ds(3,thu_ds);
}


void giam_giay()
{
   if(giay_ds==0) giay_ds=0x59;
   else giay_ds=tang_or_giam_so_bcd(giay_ds,giam); //tra ve so BCD bang giay_ds(bCD) tang len 1
   ds1307_luu_vao_ds(0,giay_ds); //0-la dia chi giay; tham so giay_ds moi vua tang xong
}

void giam_phut()
{
   if(phut_ds==0) phut_ds=0x59;
   else phut_ds=tang_or_giam_so_bcd(phut_ds,giam);
   ds1307_luu_vao_ds(1,phut_ds); //1-la dia chi giay; tham so giay_ds moi vua tang xong
}

void giam_gio()
{
   if(gio_ds==0) gio_ds=0x23;
   else gio_ds=tang_or_giam_so_bcd(gio_ds,giam);
   ds1307_luu_vao_ds(2,gio_ds); //2-la dia chi giay; tham so giay_ds moi vua tang xong
}

void giam_nam()
{
   if(nam_ds==0) nam_ds=0x99;
   else nam_ds=tang_or_giam_so_bcd(nam_ds,giam);
   ds1307_luu_vao_ds(6,nam_ds); 
}
void giam_thang()
{
   if(thang_ds==1) thang_ds=0x12;
   else thang_ds=tang_or_giam_so_bcd(thang_ds,giam);
   ds1307_luu_vao_ds(5,thang_ds); 
}

void giam_ngay()
{
   if(ngay_ds==1) ngay_ds=0x31;
   else ngay_ds=tang_or_giam_so_bcd(ngay_ds,giam);
   ds1307_luu_vao_ds(4,ngay_ds); 
}

void chinh_giam_thu()
{
   if(thu_ds==2)  thu_ds=8;
   else           thu_ds--;
   ds1307_luu_vao_ds(3,thu_ds);
}
