/*
  Etch-a-Selfie
  Copyright (c) 2019 Patrick Knöbel
*/

#include <math.h>
#include <stddef.h>
#include <stdlib.h>

#include <esp_attr.h>

#include "convert.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct 
{
   uint16_t xs;
   uint16_t ys;
   uint16_t xe;
   uint16_t ye;
} s_line;

inline static float tanhlookup(float v)
{
  static const DRAM_ATTR uint16_t tanhlookuptable[]={0,127,254,381,508,635,762,889,1016,1143,1270,1397,1524,1651,1778,1904,2031,2158,2285,2412,2539,2666,2792,2919,3046,3173,3299,3426,3553,3679,3806,3932,4059,4185,4312,4438,4565,4691,4817,4944,5070,5196,5322,5448,5574,5700,5826,5952,6078,6204,6330,6456,6581,6707,6833,6958,7084,7209,7335,7460,7585,7711,7836,7961,8086,8211,8336,8461,8586,8710,8835,8960,9084,9209,9333,9458,9582,9706,9830,9954,10078,10202,10326,10450,10573,10697,10821,10944,11067,11191,11314,11437,11560,11683,11806,11929,12051,12174,12296,12419,12541,12663,12786,12908,13029,13151,13273,13395,13516,13638,13759,13880,14002,14123,14244,14364,14485,14606,14726,14847,14967,15087,15208,15328,15447,15567,15687,15806,15926,16045,16164,16283,16402,16521,16640,16759,16877,16996,17114,17232,17350,17468,17586,17703,17821,17938,18056,18173,18290,18407,18523,18640,18756,18873,18989,19105,19221,19337,19453,19568,19684,19799,19914,20029,20144,20259,20373,20488,20602,20716,20830,20944,21058,21172,21285,21398,21512,21625,21738,21850,21963,22075,22188,22300,22412,22524,22635,22747,22858,22969,23080,23191,23302,23413,23523,23634,23744,23854,23964,24073,24183,24292,24401,24510,24619,24728,24836,24945,25053,25161,25269,25377,25484,25592,25699,25806,25913,26020,26126,26233,26339,26445,26551,26657,26762,26868,26973,27078,27183,27288,27392,27497,27601,27705,27809,27912,28016,28119,28222,28325,28428,28531,28633,28736,28838,28940,29041,29143,29244,29346,29447,29548,29648,29749,29849,29949,30049,30149,30249,30348,30447,30546,30645,30744,30842,30941,31039,31137,31235,31332,31430,31527,31624,31721,31818,31914,32010,32107,32202,32298,32394,32489,32584,32679,32774,32869,32963,33058,33152,33246,33339,33433,33526,33619,33712,33805,33898,33990,34082,34174,34266,34358,34449,34540,34631,34722,34813,34904,34994,35084,35174,35264,35353,35443,35532,35621,35710,35798,35887,35975,36063,36151,36238,36326,36413,36500,36587,36674,36760,36847,36933,37019,37104,37190,37275,37360,37445,37530,37615,37699,37783,37867,37951,38035,38118,38202,38285,38368,38450,38533,38615,38697,38779,38861,38942,39024,39105,39186,39267,39347,39428,39508,39588,39668,39747,39827,39906,39985,40064,40143,40221,40300,40378,40456,40533,40611,40688,40766,40843,40919,40996,41072,41149,41225,41301,41376,41452,41527,41602,41677,41752,41826,41901,41975,42049,42123,42196,42270,42343,42416,42489,42562,42634,42706,42778,42850,42922,42994,43065,43136,43207,43278,43349,43419,43490,43560,43630,43699,43769,43838,43907,43976,44045,44114,44182,44251,44319,44387,44454,44522,44589,44657,44724,44790,44857,44924,44990,45056,45122,45188,45253,45319,45384,45449,45514,45578,45643,45707,45771,45835,45899,45963,46026,46090,46153,46216,46278,46341,46403,46466,46528,46590,46651,46713,46774,46835,46897,46957,47018,47079,47139,47199,47259,47319,47379,47438,47497,47557,47616,47674,47733,47791,47850,47908,47966,48024,48081,48139,48196,48253,48310,48367,48424,48480,48537,48593,48649,48705,48760,48816,48871,48926,48981,49036,49091,49145,49200,49254,49308,49362,49416,49469,49523,49576,49629,49682,49735,49787,49840,49892,49944,49996,50048,50100,50151,50203,50254,50305,50356,50407,50457,50508,50558,50608,50658,50708,50758,50807,50857,50906,50955,51004,51053,51101,51150,51198,51246,51294,51342,51390,51438,51485,51532,51580,51627,51674,51720,51767,51813,51860,51906,51952,51998,52043,52089,52134,52180,52225,52270,52315,52359,52404,52448,52493,52537,52581,52625,52669,52712,52756,52799,52842,52885,52928,52971,53014,53056,53099,53141,53183,53225,53267,53308,53350,53391,53433,53474,53515,53556,53597,53637,53678,53718,53758,53799,53839,53879,53918,53958,53997,54037,54076,54115,54154,54193,54232,54270,54309,54347,54385,54423,54461,54499,54537,54575,54612,54649,54687,54724,54761,54798,54834,54871,54907,54944,54980,55016,55052,55088,55124,55160,55195,55231,55266,55301,55336,55371,55406,55441,55475,55510,55544,55579,55613,55647,55681,55715,55748,55782,55815,55849,55882,55915,55948,55981,56014,56047,56079,56112,56144,56176,56209,56241,56272,56304,56336,56368,56399,56431,56462,56493,56524,56555,56586,56617,56647,56678,56709,56739,56769,56799,56829,56859,56889,56919,56948,56978,57007,57037,57066,57095,57124,57153,57182,57211,57239,57268,57296,57325,57353,57381,57409,57437,57465,57493,57520,57548,57575,57603,57630,57657,57684,57711,57738,57765,57792,57819,57845,57872,57898,57924,57950,57976,58002,58028,58054,58080,58106,58131,58157,58182,58207,58232,58257,58282,58307,58332,58357,58382,58406,58431,58455,58480,58504,58528,58552,58576,58600,58624,58647,58671,58695,58718,58741,58765,58788,58811,58834,58857,58880,58903,58926,58948,58971,58994,59016,59038,59061,59083,59105,59127,59149,59171,59193,59214,59236,59257,59279,59300,59322,59343,59364,59385,59406,59427,59448,59469,59490,59510,59531,59551,59572,59592,59613,59633,59653,59673,59693,59713,59733,59753,59772,59792,59812,59831,59851,59870,59889,59909,59928,59947,59966,59985,60004,60022,60041,60060,60079,60097,60116,60134,60152,60171,60189,60207,60225,60243,60261,60279,60297,60315,60332,60350,60367,60385,60402,60420,60437,60454,60472,60489,60506,60523,60540,60557,60573,60590,60607,60624,60640,60657,60673,60690,60706,60722,60738,60755,60771,60787,60803,60819,60834,60850,60866,60882,60897,60913,60929,60944,60959,60975,60990,61005,61020,61036,61051,61066,61081,61096,61110,61125,61140,61155,61169,61184,61198,61213,61227,61242,61256,61270,61285,61299,61313,61327,61341,61355,61369,61383,61396,61410,61424,61438,61451,61465,61478,61492,61505,61518,61532,61545,61558,61571,61584,61597,61610,61623,61636,61649,61662,61675,61688,61700,61713,61725,61738,61750,61763,61775,61788,61800,61812,61824,61837,61849,61861,61873,61885,61897,61909,61921,61932,61944,61956,61968,61979,61991,62002,62014,62025,62037,62048,62060,62071,62082,62093,62104,62116,62127,62138,62149,62160,62171,62182,62192,62203,62214,62225,62235,62246,62257,62267,62278,62288,62299,62309,62319,62330,62340,62350,62360,62371,62381,62391,62401,62411,62421,62431,62441,62451,62461,62470,62480,62490,62500,62509,62519,62528,62538,62547,62557,62566,62576,62585,62594,62604,62613,62622,62631,62641,62650,62659,62668,62677};
  if (v>=2)
    return 1;    
  if (v<=-2)
    return -1;
  if (v<0)
    return (0-tanhlookuptable[(uint16_t)((0-v)*512)])/(255*255.0);    
  return (tanhlookuptable[(uint16_t)(v*512)])/(255*255.0);
}

static float scalelin(float scale, float min, float max)
{
  return min+(max-min)*scale;
}

static float scalelog(float scale, float min, float max)
{
  return exp(log(min) + (log(max)-log(min))*scale);
}

inline static uint8_t bw_set(uint8_t *bw, uint32_t pos, uint8_t v)
{
  uint8_t shift=(pos%4)*2;
  pos=pos/4;
  bw[pos]=(bw[pos] & (~(0b11 << shift))) | ((v & 0b11) << shift);
}
inline static uint8_t bw_get(uint8_t *bw, uint32_t pos)
{ 
  return (bw[pos/4] >> ((pos%4)*2)) & 0b11;
}

inline static uint8_t done_set(uint8_t *done, uint32_t pos, uint8_t v)
{
  uint8_t shift=pos%8;
  pos=pos/8;
  done[pos]=(done[pos] & (~(0b1 << shift))) | ((v & 0b1) << shift);
}
inline static uint8_t done_get(uint8_t *done, uint32_t pos)
{ 
  return (done[pos/8] >> (pos%8)) & 0b1;
}


//https://lodev.org/cgtutor/floodfill.html
//The scanline floodfill algorithm using stack instead of recursion, more robust
static void floodFillScanlineStack(uint16_t w, uint16_t h, uint8_t *bw, uint32_t *stack, uint16_t x, uint16_t y)
{  
  uint32_t pointer =0;
  stack[pointer++] = x+y*w;

  while(pointer>0)
  {
    uint32_t c= stack[--pointer];
    uint16_t x=c%w;
    uint16_t y=(c/w)|0;
    int16_t x1 = x;
    while(x1 >= 0 && bw_get(bw,x1+y*w) == 1) x1--;
    //8 Way:
    if(x1 > 0 && y > 0 && bw_get(bw,x1+(y-1)*w) == 1)
    {
      stack[pointer++] =x1+(y - 1)*w;
    }
    if(x1 > 0 && y < h-1 && bw_get(bw,x1+(y+1)*w) == 1)
    {
      stack[pointer++]=x1+(y + 1)*w;
    }
    x1++;
    uint8_t spanAbove = 0;
    uint8_t spanBelow = 0;
    while(x1 < w && bw_get(bw,x1+y*w) == 1)
    {
      bw_set(bw,x1+y*w,2);
      if(!spanAbove && y > 0 && bw_get(bw,x1+(y-1)*w) == 1)
      {
        stack[pointer++]=x1+(y - 1)*w;
        spanAbove = 1;
      }
      else if(spanAbove && y > 0 &&  bw_get(bw,x1+(y-1)*w) != 1)
      {
        spanAbove = 0;
      }
      if(!spanBelow && y < h - 1 &&  bw_get(bw,x1+(y+1)*w) == 1 )
      {
        stack[pointer++]=x1+(y + 1)*w;
        spanBelow = 1;
      }
      else if(spanBelow && y < h - 1 && bw_get(bw,x1+(y+1)*w) != 1)
      {
        spanBelow = 0;
      }
      x1++;
    }
    //8Way:
    if(x1 < w && y > 0 && bw_get(bw,x1+(y-1)*w) == 1)
    {
      stack[pointer++]=x1+(y - 1)*w;
    }
    if(x1 < w && y < h-1 && bw_get(bw,x1+(y+1)*w) == 1)
    {
      stack[pointer++]=x1+(y + 1)*w;
    } 
  }
}

static s_line searchAndConnect(uint16_t w, uint16_t h, uint8_t *bw)
{
  s_line line; 
  
  //marke new boardes
  for(uint16_t y=1;y<h-1;y++)
  {
    for(uint16_t x=1;x<w-1;x++)
    {
      if(bw_get(bw,x+y*w)==0)
      {
        if( bw_get(bw,(x+1)+y*w)==2 || bw_get(bw,x+(y+1)*w)==2 || 
            bw_get(bw,(x-1)+y*w)==2 || bw_get(bw,x+(y-1)*w)==2    )
        {
          bw_set(bw,x+y*w,3);
        }
      }
    }
  }

  for (uint16_t r=0;;r++)
  {
    for(uint16_t y=0;y<h;y++)
    {
      for(uint16_t x=0;x<w;x++)
      {
        if(bw_get(bw,x+y*w)==3)
        {   
          int16_t rx = r;
          int16_t ry = 0;
          int16_t rd = 1 - r;
          do 
          {
            if(rx<w && ry<h)
            {
              for(uint8_t c=0;c<8;c++)
              {
                int16_t dx, dy;
                if(c%2)
                {
                  dx=rx;
                  dy=ry;
                }
                else
                {
                  dx=ry;
                  dy=rx;                  
                }
                if((c/2)%2)
                {
                  dx=0-dx;
                }
                if((c/4)%2)
                {
                  dy=0-dy;
                }
                if((x+dx)>=0 && (x+dx)<w && 
                   (y+dy)>=0 && (y+dy)<h && 
                    bw_get(bw,x+dx+(y+dy)*w)==1 )
                {
                  line.xs=x;
                  line.ys=y;
                  line.xe=x+dx;
                  line.ye=y+dy;
                  return line;
                }                     
              }
            }
            ry = ry + 1;
            if (rd < 0)
            {
                rd = rd + 2*ry + 1;          
            }
            else
            {
                rx = rx - 1;
                rd = rd + 2*(ry-rx) + 1;
            }
          } while(rx > ry);          
        }
      }
    }
  }
}

static void drawLine(uint16_t w, uint16_t h, uint8_t *bw, s_line line)
{
  uint16_t x0=line.xs;
  uint16_t y0=line.ys;
  uint16_t x1=line.xe;
  uint16_t y1=line.ye;
  int16_t dx =  abs(x1-x0);
  int8_t sx = x0<x1 ? 1 : -1;
  int16_t dy = -abs(y1-y0);
  int8_t sy = y0<y1 ? 1 : -1;
  int16_t err = dx+dy;  /* error value e_xy */
  for(;;)
  {   /* loop */
    bw_set(bw,x0+y0*w,2);
    if (x0==x1 && y0==y1) 
    {
      break;       
    }
    int16_t e2 = 2*err;
    if (e2 >= dy) 
    {
      err += dy; /* e_xy+e_x > 0 */
      x0 += sx;
    }
    if (e2 <= dx)
    { /* e_xy+e_y < 0 */
      err += dx;
      y0 += sy;
    }
  }
}

void convert_RGB2Grayscale(uint16_t rgb_w, uint16_t rgb_h, uint8_t *rgb, uint16_t gray_w, uint16_t gray_h, uint8_t *gray)
{
  uint16_t offsetx = (rgb_w -gray_w)/2;
  uint16_t offsety = (rgb_h -gray_h)/2;
  
  for (uint16_t x=0; x<gray_w;x++)
  {    
    for (uint16_t y=0; y<gray_h;y++)
    {
      uint32_t i=x+offsetx+(y+offsety)*rgb_w;
      gray[x+y*gray_w]=(uint8_t)(rgb[i*3+0]*0.299+rgb[i*3+1]*0.587+rgb[i*3+2]*0.114);
    }  
  }
}

void convert_XDOG_init(uint16_t w, uint16_t h, convert_statuscallback_t statuscb, uint8_t *gray, uint16_t *buffer1, uint16_t *buffer2)
{
  static const DRAM_ATTR float kernel[2][21] = 
  {  // 0         1        2        3        4        5       6         7       8         9        10        11        12        13       14     15       16       17       18       19      20   
    {0.000001,0.000009,0.000075,0.000476,0.002359,0.009135,0.027646,0.065407,0.120983,0.174973,0.197872,0.174973,0.120983,0.065407,0.027646,0.009135,0.002359,0.000476,0.000075,0.000009,0.000001},    
    {0.000959,0.002419,0.005532,0.011479,0.02161,0.036908 ,0.05719 , 0.0804 ,0.102547,0.118665,0.124582,0.118665,0.102547,0.0804  ,0.05719 ,0.036908,0.02161 ,0.011479,0.005532,0.002419,0.000959}
  };
  
  const uint8_t kernel_size=21;
  const uint8_t kernel_half=10;        
  
  uint16_t * gaus[2] ={buffer1,buffer2};
  
  uint16_t convbuf[MAX(w,h)+kernel_half*2]; //~2kB should be ok to place on stack 

  statuscb("filter",0);  

  for(uint8_t g=0;g<2;g++)
  {
    for(uint32_t ls=0;ls<w*h;ls+=w)
    {
      //fill buffer            
      uint16_t v=gray[ls]*256;
      for(uint16_t b=0;b<kernel_half;b++){
        convbuf[b]=v;
      }            
      for(uint16_t b=0;b<w;b++){
        convbuf[kernel_half+b]=gray[ls+b]*256;
      }
      v=gray[ls+w-1];
      for(uint16_t b=0;b<kernel_half;b++){
        convbuf[kernel_half+w+b]=v;
      }
      //convolution
      for(uint16_t p=0;p<w;p++){
        float sum=0;
        for(uint16_t k=0;k<kernel_size;k++){
          sum+=convbuf[p+k]*kernel[g][k];
        }
        gaus[g][ls+p]=sum;
      } 
      if((ls/w)%(h/20)==0) statuscb("filter",25*ls/(w*h)+50*g);
    }
    
    for(uint16_t rs=0;rs<w;rs+=1)
    {
      //fill buffer
      uint16_t v=gaus[g][rs];
      for(uint16_t b=0;b<kernel_half;b++){
        convbuf[b]=v;
      }
      for(uint16_t b=0;b<h;b++){
        convbuf[kernel_half+b]=gaus[g][rs+b*w];
      }
      v=gaus[g][rs+(h-1)*w];
      for(uint16_t b=0;b<kernel_half;b++){
        convbuf[kernel_half+h+b]=v;
      }
      //convolution
      for(uint16_t p=0;p<h;p++){
        float sum=0;
        for(uint16_t k=0;k<kernel_size;k++){
          sum+=convbuf[p+k]*kernel[g][k];
        }
        gaus[g][rs+p*w]=sum;
      }          
      if(rs%(w/20)==0) statuscb("filter",25*rs/w+25+50*g);            
    }
  }
  statuscb("filter",100);
}

void convert_XDOG(uint16_t w, uint16_t h, float gamma, float phi, uint16_t *buffer1, uint16_t *buffer2, uint8_t *bw)
{
  uint16_t * gaus[2] ={buffer1,buffer2};
  gamma=scalelog(gamma,0.5,0.99);
  phi=scalelog(phi,500,1);
  
  //calc mean:
  float mean=0;
  for (uint32_t i=0; i<w*h;i++){
    float v = (gaus[0][i]-(gamma*gaus[1][i]))/(256*256.0);
    if(v < -0.1){
      v=1.0;
    }
    else{
      v=1.0+tanhlookup(phi*v);
    }
    mean+=v;
  }        
  mean=mean/(w*h);

  for (uint32_t i=0; i<w*h;i++){
    float v = (gaus[0][i]-(gamma*gaus[1][i]))/(256*256.0);
    if(v < -0.1){
      v=1.0;
    }
    else{
      v=1.0+tanhlookup(phi*v);
    }
    if(v < mean){
      bw_set(bw,i,1);
    }
    else{
      bw_set(bw,i,0);
    }
    
  }   
}

void convert_connect(uint16_t w, uint16_t h, convert_statuscallback_t statuscb, uint8_t *bw, uint32_t *stack)
{
  statuscb("connecting",0);
  
  //we need a Line to start from:
  for(uint16_t i=0;i<w;i++)
  {
    bw_set(bw,i,1);
  }
  for(uint16_t i=0;i<w;i++)
  {
    bw_set(bw,i+(h-1)*w,1);
  }
  for(uint16_t i=0;i<h;i++)
  {
    bw_set(bw,i*w,1);
  }
  for(uint16_t i=0;i<h;i++)
  {
    bw_set(bw,w-1+i*w,1);
  }
  
  floodFillScanlineStack(w,h,bw,stack,0,h-1);   
            
  uint32_t totalBlackPixel=0;
  for(uint32_t i=0;i<w*h;i++)
  {
    if(bw_get(bw,i)==1)
    {
      totalBlackPixel++;
    }
  }
  
  for(;;)
  {
    //test if there is stell something left:
    uint32_t blackPixel=0;
    for(uint32_t i=0;i<w*h;i++)
    {
      if(bw_get(bw,i)==1)
      {
        blackPixel++;
      }
    }
    if(blackPixel==0)
    {
      //Done
      break;
    } 
    statuscb("connecting",pow(101,(totalBlackPixel-blackPixel)/(float)totalBlackPixel)-1);
    
    s_line line=searchAndConnect(w,h,bw); 

    drawLine(w,h,bw,line);
    floodFillScanlineStack(w,h,bw,stack,line.xe,line.ye);  
  
  }
  
  //remove boarder
  for(uint32_t i=0;i<w*h;i++)
  {
    if(bw_get(bw,i)==3)
    {
        bw_set(bw,i,0);
    }
  }
  
  statuscb("connecting",100);  
}


void convert_etch(uint16_t w, uint16_t h, convert_statuscallback_t statuscb, convert_gcodecallback_t gcodecb, uint8_t *bw, uint8_t *done, uint16_t * cost, uint32_t *discoverd, uint32_t discoverd_length)
{
  char gcodebuffer[100];
  statuscb("etching",0);
  uint32_t totalRedPixel=0;
  for(uint32_t i=0;i<w*h;i++)
  {
    if(bw_get(bw,i)==2)
    {
      totalRedPixel++;
    }
  }
  //start point
  uint16_t cx=0;
  uint16_t cy=h-1;
  snprintf(gcodebuffer,100,"G1 X%d Y%d",cx,cy);
  gcodecb(gcodebuffer);
  for(;;)
  {
    //search for the longest line:
    uint16_t ndx=1;
    uint16_t ndy=0;
    uint16_t nlength=0;
    for (uint8_t dir=0;dir<4;dir++)
    {
      uint16_t dx,dy;
      switch(dir)
      {
        case 0:
          dx=1;
          dy=0;
          break;
        case 1:
          dx=0;
          dy=1;
          break;
        case 2:
          dx=-1;
          dy=0;
          break;
        case 3:
          dx=0;
          dy=-1;
          break;
      }
      uint16_t tx=cx;
      uint16_t ty=cy;
      uint16_t length=0;
      while(tx>=0 && ty>=0 && tx<w && ty<h && bw_get(bw,tx+ty*w)==2)
      {
        length++;
        tx+=dx;
        ty+=dy;
      }
      if(length>nlength)
      {
        ndx=dx;
        ndy=dy;
        nlength=length;
      }                
    }
    
    //marke line done!
    for(uint16_t i=0;i<nlength;i++)
    {
      bw_set(bw,cx+cy*w,3);
      if(i<nlength-1)
      {
        cx+=ndx;
        cy+=ndy;
      }
    }

    //generate G code:
    snprintf(gcodebuffer,100,"G1 X%d Y%d",cx,cy);
    gcodecb(gcodebuffer);
    
    //find next empty spot => breadth first search
    uint32_t readpointer=0;
    uint32_t writepointer=0;            
    uint16_t ax=cx;
    uint16_t ay=cy;
    uint32_t current=ax+ay*w; 
    for (uint32_t i=0;i<w*h;i++)
    {
      done_set(done,i,0); // need to be cleared!      
    }
    discoverd[writepointer++]=current;
    cost[current]=0;
    done_set(done,current,1);
    while (readpointer!=writepointer)
    {
      //remove first element:
      current=discoverd[readpointer];
      ax= current%w; 
      ay= (current/w)|0;

      if(bw_get(bw,current)==2)
      {
        //found next spot to echt
        break;
      }
      readpointer=(readpointer+1)%discoverd_length;    
      
      //process
      uint16_t cmin=cost[current];
      for(uint8_t n=0;n<9;n++)
      {
        int8_t dx=n%3==0?1:((n%3==1)?0:-1);
        int8_t dy=(n/3)%3==0?1:(((n/3)%3==1)?0:-1);
        uint32_t next=ax+dx +(ay+dy)*w;
        if (ax+dx>=0 && ay+dy>=0 && ax+dx<w && ay+dy<h && done_get(done,next))
        {
          uint16_t c=cost[next]+((dx!=0 && dy!=0)?14:10);
          if(cmin>c)
            cmin=c;
        }
      }
      cost[current]=cmin;
      
      for(uint8_t n=0;n<9;n++)
      {
        int8_t dx=n%3==0?1:((n%3==1)?0:-1);
        int8_t dy=(n/3)%3==0?1:(((n/3)%3==1)?0:-1);
        uint32_t next=ax+dx+(ay+dy)*w;
        if (ax+dx>=0 && ay+dy>=0 && ax+dx<w && ay+dy<h && 
            done_get(done,next)==0 && bw_get(bw,next)!=0)
        {
          cost[next]=cost[current]+100; //Higher cost not visited yet!
          done_set(done,next,1);
          discoverd[writepointer]=next;                  
          writepointer=(writepointer+1)%discoverd_length;          
        }
      }
    }
    if(readpointer==writepointer)
    {
      //done! all is etched
      break;
    }
    
    //Trace back;
    discoverd[0]=current;
    int16_t i=0;
    for(;discoverd[i]!=cx+cy*w;i++)
    {
      uint16_t ax=discoverd[i]%w; 
      uint16_t ay=discoverd[i]/w;
if(discoverd[i]>w*h){
snprintf(gcodebuffer,100,"OOR i %d discoverd[i] %d",i,discoverd[i]);
statuscb(gcodebuffer,0);
        
} 
      uint16_t c=cost[discoverd[i]];
      uint8_t found=0;
      for(uint8_t n=0;n<9;n++)
      {
        int8_t dx=n%3==0?1:((n%3==1)?0:-1);
        int8_t dy=(n/3)%3==0?1:(((n/3)%3==1)?0:-1);
        uint32_t next=ax+dx+(ay+dy)*w;
        if ((dx!=0 || dy!=0) && ax+dx>=0 && ay+dy>=0 && 
                                ax+dx<w  && ay+dy<h  && 
            done_get(done,next)                         )
        {
          if(c>cost[next])
          {
            c=cost[next];
            discoverd[i+1]=next;
            found=1;
          }
        }
      }
if(found==0){
snprintf(gcodebuffer,100,"NF current %d cx %d cy %d discoverd[i] %d i %d",current,cx,cy, discoverd[i], i);
statuscb(gcodebuffer,0);
        
        
}
    }

    int8_t ldx=0;
    int8_t ldy=0;
    for(;i>=0;i--)
    {
      uint16_t x=discoverd[i]%w;
      uint16_t y=discoverd[i]/w;
      uint16_t nx=discoverd[i+1]%w;
      uint16_t ny=discoverd[i+1]/w;
      int8_t dx=x-nx;
      int8_t dy=y-ny;
      if(ldx!=dx || ldy!=dy)
      {
        //generate G code:
        snprintf(gcodebuffer,100,"G1 X%d Y%d",x,y);
        gcodecb(gcodebuffer);
        ldx=dx;
        ldy=dy;
      }
    }  
    
    cx=discoverd[0]%w;
    cy=discoverd[0]/w;
    snprintf(gcodebuffer,100,"G1 X%d Y%d",cx,cy);
    gcodecb(gcodebuffer);
    
    uint32_t greenPixel=0;
    for(uint32_t i=0;i<w*h;i++)
    {
      if(bw_get(bw,i)==3)
      {
        greenPixel++;
      }
    }
    statuscb("etching",99*greenPixel/totalRedPixel);
    
  }
  
  statuscb("etching",100);

  
}


