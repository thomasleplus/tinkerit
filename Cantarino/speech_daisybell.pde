#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

uint16_t pitchPhase, form1Phase,form2Phase,form3Phase;
uint16_t pitchPhaseInc,form1PhaseInc,form2PhaseInc,form3PhaseInc;
uint8_t form1Amp,form2Amp,form3Amp;
uint8_t noiseMod=10;

int8_t sinCalc[256] PROGMEM = {
  /* This table rolls a lot of functions together for speed.
     Extracting phase and amplitude from the nybble packed form
     Sine calculation
     Exponential amplitude mapping
     Scaling to appropriate range
     
     ROUND(
       FLOOR(a/16,1)
       *SIN(
         2
         * PI()
         * IF(
           MOD(a,16),
           EXP(0.18*MOD(a,16)),
           0
         ) /16
       )*127
     ,0)
  */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,2,2,3,3,4,5,6,7,8,10,12,14,17,20,24,
  0,4,4,5,6,7,9,11,13,15,18,22,26,31,37,45,
  0,5,6,7,8,10,12,14,17,20,24,28,34,41,49,58,
  0,5,6,7,9,10,12,15,18,21,26,31,37,44,53,63,
  0,5,6,7,8,10,12,14,17,20,24,28,34,41,49,58,
  0,4,4,5,6,7,9,11,13,15,18,22,26,31,37,45,
  0,2,2,3,3,4,5,6,7,8,10,12,14,17,20,24,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,-2,-2,-3,-3,-4,-5,-6,-7,-8,-10,-12,-14,-17,-20,-24,
  0,-4,-4,-5,-6,-7,-9,-11,-13,-15,-18,-22,-26,-31,-37,-45,
  0,-5,-6,-7,-8,-10,-12,-14,-17,-20,-24,-28,-34,-41,-49,-58,
  0,-5,-6,-7,-9,-10,-12,-15,-18,-21,-26,-31,-37,-44,-53,-63,
  0,-5,-6,-7,-8,-10,-12,-14,-17,-20,-24,-28,-34,-41,-49,-58,
  0,-4,-4,-5,-6,-7,-9,-11,-13,-15,-18,-22,-26,-31,-37,-45,
  0,-2,-2,-3,-3,-4,-5,-6,-7,-8,-10,-12,-14,-17,-20,-24
};

int8_t sqrCalc[256] PROGMEM ={
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,1,2,2,2,3,3,4,5,5,6,8,9,11,13,16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16,
  0,-1,-2,-2,-2,-3,-3,-4,-5,-5,-6,-8,-9,-11,-13,-16
};

// Changing these will also requires rewriting audioOn()

#if defined(__AVR_ATmega8__)
#define LED_PIN   13
#define LED_PORT  PORTB
#define LED_BIT   5
// On old ATmega8 boards, output is on pin 11
#define PWM_PIN       11
#define PWM_VALUE     OCR2
#define PWM_INTERRUPT TIMER2_OVF_vect
#elif defined(__AVR_ATmega1280__)
#define LED_PIN   13
#define LED_PORT  PORTB
#define LED_BIT   7
#define PWM_PIN       3
#define PWM_VALUE     OCR3C
#define PWM_INTERRUPT TIMER3_OVF_vect
#else
#define LED_PIN   13
#define LED_PORT  PORTB
#define LED_BIT   5
// For modern ATmega168 boards, output is on pin 3
#define PWM_PIN       3
#define PWM_VALUE     OCR2B
#define PWM_INTERRUPT TIMER2_OVF_vect
#endif

void audioOn() {
#if defined(__AVR_ATmega8__)
  // ATmega8 has different registers
  TCCR2 = _BV(WGM20) | _BV(COM21) | _BV(CS20);
  TIMSK = _BV(TOIE2);
#elif defined(__AVR_ATmega1280__)
  TCCR3A = _BV(COM3C1) | _BV(WGM30);
  TCCR3B = _BV(CS30);
  TIMSK3 = _BV(TOIE3);
#else
  // Set up PWM to 31.25kHz, phase accurate
  TCCR2A = _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = _BV(TOIE2);
#endif
}


void setup() {
  pinMode(PWM_PIN,OUTPUT);
  audioOn();
  pinMode(LED_PIN,OUTPUT);
}

#define FORMANT_SZ 7

enum {
  _SP,_DOT,_QM,_COM,_HYP,_IY,_IH,_EH,_AE,_AA,
  _AH,_AO,_UH,_AX,_IX,_ER,_UX,_OH,_RX,_LX,
  _WX,_YX,_WH,_R,_L,_W,_Y,_M,_N,_NX,
  _DX,_Q,_S,_SH,_F,_TH,__H,__X,_Z,_ZH,
  _V,_DH,_CHa,_CHb,_Ja,_Jb,_Jc,_Jd,_EY,_AY,
  _OY,_AW,_OW,_UW,_Ba,_Bb,_Bc,_Da,_Db,_Dc,
  _Ga,_Gb,_Gc,_GXa,_GXb,_GXc,_Pa,_Pb,_Pc,_Ta,
  _Tb,_Tc,_Ka,_Kb,_Kc,_KXa,_KXb,_KXc
};

uint8_t formantTable[] PROGMEM = {
   0x0, 0x0, 0x0,0x0,0x0,0x0,0x0,/*00 space*/ 0x13,0x43,0x5b,0x0,0x0,0x0,0x0,/*01 .*/
  0x13,0x43,0x5b,0x0,0x0,0x0,0x0,/*02 ?*/     0x13,0x43,0x5b,0x0,0x0,0x0,0x0,/*03 ,*/
  0x13,0x43,0x5b,0x0,0x0,0x0,0x0,/*04 -*/      0xa,0x54,0x6e,0xd,0xa,0x8,0x0,/*05 IY*/
   0xe,0x49,0x5d,0xd,0x8,0x7,0x0,/*06 IH*/    0x13,0x43,0x5b,0xe,0xd,0x8,0x0,/*07 EH*/
  0x18,0x3f,0x58,0xf,0xe,0x8,0x0,/*08 AE*/    0x1b,0x28,0x59,0xf,0xd,0x1,0x0,/*09 AA*/
  0x17,0x2c,0x57,0xf,0xc,0x1,0x0,/*10 AH*/    0x15,0x1f,0x58,0xf,0xc,0x0,0x0,/*11 AO*/
  0x10,0x25,0x52,0xf,0xb,0x1,0x0,/*12 UH*/    0x14,0x2c,0x57,0xe,0xb,0x0,0x0,/*13 AX*/
   0xe,0x49,0x5d,0xd,0xb,0x7,0x0,/*14 IX*/    0x12,0x31,0x3e,0xc,0xb,0x5,0x0,/*15 ER*/
   0xe,0x24,0x52,0xf,0xc,0x1,0x0,/*16 UX*/    0x12,0x1e,0x58,0xf,0xc,0x0,0x0,/*17 OH*/
  0x12,0x33,0x3e,0xd,0xc,0x6,0x0,/*18 RX*/    0x10,0x25,0x6e,0xd,0x8,0x1,0x0,/*19 LX*/
   0xd,0x1d,0x50,0xd,0x8,0x0,0x0,/*20 WX*/     0xf,0x45,0x5d,0xe,0xc,0x7,0x0,/*21 YX*/
   0xb,0x18,0x5a,0xd,0x8,0x0,0x0,/*22 WH*/    0x12,0x32,0x3c,0xc,0xa,0x5,0x0,/*23 R*/
   0xe,0x1e,0x6e,0xd,0x8,0x1,0x0,/*24 L*/      0xb,0x18,0x5a,0xd,0x8,0x0,0x0,/*25 W*/
   0x9,0x53,0x6e,0xd,0xa,0x8,0x0,/*26 Y*/      0x6,0x2e,0x51,0xc,0x3,0x0,0x0,/*27 M*/
   0x6,0x36,0x79,0x9,0x9,0x0,0x0,/*28 N*/      0x6,0x56,0x65,0x9,0x6,0x3,0x0,/*29 NX*/
   0x6,0x36,0x79,0x0,0x0,0x0,0x0,/*30 DX*/    0x11,0x43,0x5b,0x0,0x0,0x0,0x0,/*31 Q*/
   0x6,0x49,0x63,0x7,0xa,0xd,0xf,/*32 S*/      0x6,0x4f,0x6a,0x0,0x0,0x0,0x0,/*33 SH*/
   0x6,0x1a,0x51,0x3,0x3,0x3,0xf,/*34 F*/      0x6,0x42,0x79,0x0,0x0,0x0,0x0,/*35 TH*/
   0xe,0x49,0x5d,0x0,0x0,0x0,0x0,/*36 /H*/    0x10,0x25,0x52,0x0,0x0,0x0,0x0,/*37 /X*/
   0x9,0x33,0x5d,0xf,0x3,0x0,0x3,/*38 Z*/      0xa,0x42,0x67,0xb,0x5,0x1,0x0,/*39 ZH*/
   0x8,0x28,0x4c,0xb,0x3,0x0,0x0,/*40 V*/      0xa,0x2f,0x5d,0xb,0x4,0x0,0x0,/*41 DH*/
   0x6,0x4f,0x65,0x0,0x0,0x0,0x0,/*42 CHa*/    0x6,0x4f,0x65,0x0,0x0,0x0,0x0,/*43 CHb*/
   0x6,0x42,0x79,0x1,0x0,0x0,0x0,/*44 Ja*/     0x5,0x42,0x79,0x1,0x0,0x0,0x0,/*45 Jb*/
   0x6,0x6e,0x79,0x0,0xa,0xe,0x0,/*46 Jc*/     0x0, 0x0, 0x0,0x2,0x2,0x1,0x0,/*47 Jd*/
  0x13,0x48,0x5a,0xe,0xe,0x9,0x0,/*48 EY*/    0x1b,0x27,0x58,0xf,0xd,0x1,0x0,/*49 AY*/
  0x15,0x1f,0x58,0xf,0xc,0x0,0x0,/*50 OY*/    0x1b,0x2b,0x58,0xf,0xd,0x1,0x0,/*51 AW*/
  0x12,0x1e,0x58,0xf,0xc,0x0,0x0,/*52 OW*/     0xd,0x22,0x52,0xd,0x8,0x0,0x0,/*53 UW*/
   0x6,0x1a,0x51,0x2,0x0,0x0,0x0,/*54 Ba*/     0x6,0x1a,0x51,0x4,0x1,0x0,0xf,/*55 Bb*/
   0x6,0x1a,0x51,0x0,0x0,0x0,0x0,/*56 Bc*/     0x6,0x42,0x79,0x2,0x0,0x0,0x0,/*57 Da*/
   0x6,0x42,0x79,0x4,0x1,0x0,0xf,/*58 Db*/     0x6,0x42,0x79,0x0,0x0,0x0,0x0,/*59 Dc*/
   0x6,0x6e,0x70,0x1,0x0,0x0,0x0,/*60 Ga*/     0x6,0x6e,0x6e,0x4,0x1,0x0,0xf,/*61 Gb*/
   0x6,0x6e,0x6e,0x0,0x0,0x0,0x0,/*62 Gc*/     0x6,0x54,0x5e,0x1,0x0,0x0,0x0,/*63 GXa*/
   0x6,0x54,0x5e,0x4,0x1,0x0,0xf,/*64 GXb*/    0x6,0x54,0x5e,0x0,0x0,0x0,0x0,/*65 GXc*/
   0x6,0x1a,0x51,0x0,0x0,0x0,0x0,/*66 Pa*/     0x6,0x1a,0x51,0x0,0x0,0x0,0x0,/*67 Pb*/
   0x6,0x1a,0x51,0x0,0x0,0x0,0x0,/*68 Pc*/     0x6,0x42,0x79,0x0,0x0,0x0,0x0,/*69 Ta*/
   0x6,0x42,0x79,0x0,0x0,0x0,0x0,/*70 Tb*/     0x6,0x42,0x79,0x0,0x0,0x0,0x0,/*71 Tc*/
   0x6,0x6d,0x65,0x0,0x0,0x0,0x0,/*72 Ka*/     0xa,0x56,0x65,0xc,0xa,0x7,0x0,/*73 Kb*/
   0xa,0x6d,0x70,0x0,0x0,0x0,0x0,/*74 Kc*/     0x6,0x54,0x5e,0x0,0x0,0x0,0x0,/*75 KXa*/
   0x6,0x54,0x5e,0x0,0xa,0x5,0x0,/*76 KXb*/    0x6,0x54,0x5e,0x0,0x0,0x0,0x0 /*77 KXc*/
};

uint16_t pitchTable[64] = {
  // Covers A1 to C7
  58,61,65,69,73,77,82,86,92,97,
  103,109,115,122,129,137,145,154,163,173,
  183,194,206,218,231,244,259,274,291,308,
  326,346,366,388,411,435,461,489,518,549,
  581,616,652,691,732,776,822,871,923,978,
  1036,1097,1163,1232,1305,1383,1465,1552,1644,1742,
  1845,1955,2071,2195
};

uint8_t frameList[] PROGMEM = {
#if 1
  _Da,3,0,39,_Db,1,0,39,_Dc,1,3,39,_EY,8,6,39,_YX,20,3,39, // Dai..
  _Z,10,0,36,_IY,35,3,36, // ..sy
  _Da,3,0,32,_Db,1,0,32,_Dc,1,3,32,_EY,8,6,32,_YX,20,3,32, // Dai..
  _Z,10,0,27,_IY,35,3,27, // ..sy
  _Ga,2,0,29,_Gb,2,0,29,_Gc,2,0,29,_IH,10,3,29,_V,5,0,29, // Give
  _M,2,0,31,_IY,10,3,31, // me
  _YX,5,0,32,_AO,10,0,32,_RX,5,0,32, // your
  _AH,25,0,29,_NX,5,0,29, // an..
  _S,2,0,32,_ER,10,0,32,_RX,3,0,32, // ..swer
  _Da,3,0,27,_Db,1,0,27,_Dc,1,3,27,_UX,80,3,27,_WX,5,0,27, // do
  _AY,5,20,34,_YX,10,0,34,_M,8,0,34, // I'm
  __H,5,0,39,_AX,30,0,39,_F,10,0,39, // half
  _Ka,3,0,36,_Kb,3,0,36,_Kc,4,0,36,_R,5,0,36,_EY,30,0,36, // cra..
  _Z,5,0,32,_IY,40,0,32, // ..zy
  _AO,10,0,29,_LX,5,0,29, // all
  _F,5,0,31,_AO,10,0,31, // for
  _DH,5,0,32,_AH,10,0,32, // the
  _L,5,0,34,_AH,20,0,34,_V,5,0,34,// love
  _AA,10,0,36,_V,5,0,36,// of
  _Y,10,0,34,_UX,80,0,34, // you
  _IH,10,0,36,_Ta,2,0,36,_Tb,1,0,36,_Tc,2,0,36,// It
  _W,2,0,37,_OH,10,0,37,_N,1,0,37,_Ta,1,0,37,_Tb,1,0,37,_Tc,1,0,37,// won't
  _Ba,2,0,36,_Bb,1,0,36,_Bc,2,0,36,_IY,10,0,36,// be
  _AH,15,0,34,// a
  _S,2,0,39,_Ta,2,0,39,_Tb,2,0,39,_Tc,2,0,39,_AY,1,10,39,_YX,10,0,39,// sty..
  _L,3,0,36,_IH,10,0,36,_SH,2,0,36,// ..lish
  _M,5,0,34,_AE,10,0,34,// ma..
  _R,5,0,32,_IH,60,0,32,_Ja,2,0,32,_Jb,2,0,32,_Jc,2,0,32,// ..rriage
  _AY,5,10,34,_YX,5,0,34,// I
  _Ka,2,0,36,_Kb,2,0,36,_Kc,2,0,36,_AH,20,0,36,_N,2,0,36,_Ta,2,0,36,_Tb,2,0,26,_Tc,2,0,36,// can't
  _AX,15,0,32,// a..
  _F,5,0,29,_AO,20,0,29,_R,2,0,29,_Da,1,0,29,_Db,1,0,29,_Dc,1,0,29,// ..fford
  _AX,15,0,32,// a
  _Ka,1,0,29,_Kb,1,0,29,_Kc,1,0,29,_AE,12,0,29,// ca..
  _R,5,0,27,_IH,45,0,27,_Ja,2,0,27,_Jb,2,0,27,_Jc,2,0,27,// ..rriage
  _Ba,1,0,27,_Bb,1,0,27,_Bc,1,0,27,_AH,10,0,27,_Ta,1,0,27,_Tb,1,0,27,_Tc,1,0,27,// but
  _Y,5,0,32,_UH,10,10,32,_L,5,0,32,// you'll
  _L,3,0,36,_UH,10,0,36,_Ka,1,0,36,_Kb,1,0,36,_Kc,1,0,36,// look
  _S,2,0,34,_W,2,0,34,_IY,20,0,34,_Ta,2,0,34,_Tb,2,0,34,_Tc,2,0,34,// sweet
  _AX,15,0,27,// a..
  _Ka,2,0,32,_Kb,2,0,32,_Kc,2,0,32,_R,2,0,32,_AA,20,0,32,_S,5,0,32,// ..cross
  _DH,5,0,36,_AH,10,0,36,// the
  _S,2,0,34,_IY,10,0,34,_Ta,2,0,34,_Tb,2,0,34,_Tc,2,0,34,// seat
  _AA,10,0,36,_V,5,0,36,// of
  _AE,15,0,37,// a
  _Ba,2,0,39,_Bb,2,0,39,_Bc,2,0,39,_AY,5,5,39,_YX,5,0,39,// bi..
  _S,5,0,36,_IH,10,0,36,// ..cy..
  _Ka,2,0,32,_Kb,2,0,32,_Kc,2,0,32,_L,9,0,32,// ..cle
  _M,2,0,34,_EY,5,10,34,_YX,10,0,34,_Da,2,0,34,_Db,2,0,34,_Dc,2,0,34,// made
  _F,5,0,27,_OY,1,5,27,_RX,5,0,27,// for
  _Ta,2,0,32,_Tb,2,0,32,_Tc,2,0,32,_UX,50,0,32,// two
  #endif
  _Ta,0,0,61
};

int frameTime = 15; // ms
uint16_t basePitch;
int formantScale;

void loop() {
  formantScale = 54;//random(20,80);//54;
  uint8_t *framePos = frameList;
  while(1) {
    int n;
    uint8_t startFormant,staticFrames,tweenFrames;
    uint16_t startPitch,nextPitch;
    uint8_t nextFormant;
    int16_t startForm1PhaseInc,startForm2PhaseInc,startForm3PhaseInc;
    uint8_t startForm1Amp,startForm2Amp,startForm3Amp;
    uint8_t startMod;
    uint8_t *formantPos;

    // Read next framelist item
    startFormant = pgm_read_byte(framePos++);
    staticFrames = pgm_read_byte(framePos++);

    if (!staticFrames) break; // End of phrase

    tweenFrames = pgm_read_byte(framePos++);

    startPitch = pitchTable[pgm_read_byte(framePos++)];
    nextFormant = pgm_read_byte(framePos);
    nextPitch = pitchTable[pgm_read_byte(framePos+3)];
    pitchPhaseInc = startPitch;
    formantPos = formantTable + startFormant * FORMANT_SZ;
    form1PhaseInc = startForm1PhaseInc = pgm_read_byte(formantPos++)*formantScale;
    form2PhaseInc = startForm2PhaseInc = pgm_read_byte(formantPos++)*formantScale;
    form3PhaseInc = startForm3PhaseInc = pgm_read_byte(formantPos++)*formantScale;
    form1Amp = startForm1Amp = pgm_read_byte(formantPos++);
    form2Amp = startForm2Amp = pgm_read_byte(formantPos++);
    form3Amp = startForm3Amp = pgm_read_byte(formantPos++);
    noiseMod = startMod = pgm_read_byte(formantPos++);

    for (;staticFrames--;) delay(frameTime);
    if (tweenFrames) {
      uint8_t* formantPos;
      int16_t deltaForm1PhaseInc,deltaForm2PhaseInc,deltaForm3PhaseInc;
      int8_t deltaForm1Amp,deltaForm2Amp,deltaForm3Amp;
      int8_t deltaMod;
      uint8_t nextMod;
      int16_t deltaPitch;
      tweenFrames--;
      formantPos = formantTable + nextFormant * FORMANT_SZ;
      deltaForm1PhaseInc = pgm_read_byte(formantPos++)*formantScale - startForm1PhaseInc;
      deltaForm2PhaseInc = pgm_read_byte(formantPos++)*formantScale - startForm2PhaseInc;
      deltaForm3PhaseInc = pgm_read_byte(formantPos++)*formantScale - startForm3PhaseInc;
      deltaForm1Amp = pgm_read_byte(formantPos++) - startForm1Amp;
      deltaForm2Amp = pgm_read_byte(formantPos++) - startForm2Amp;
      deltaForm3Amp = pgm_read_byte(formantPos++) - startForm3Amp;
      deltaMod = pgm_read_byte(formantPos++) - startMod;
      deltaPitch = nextPitch - startPitch;
      deltaMod = nextMod - startMod;
      for (int i=1; i<=tweenFrames; i++) {
        form1PhaseInc = startForm1PhaseInc + (i*deltaForm1PhaseInc)/tweenFrames;
        form2PhaseInc = startForm2PhaseInc + (i*deltaForm2PhaseInc)/tweenFrames;
        form3PhaseInc = startForm3PhaseInc + (i*deltaForm3PhaseInc)/tweenFrames;
        form1Amp = startForm1Amp + (i*deltaForm1Amp)/tweenFrames;
        form2Amp = startForm2Amp + (i*deltaForm2Amp)/tweenFrames;
        form3Amp = startForm3Amp + (i*deltaForm3Amp)/tweenFrames;
        pitchPhaseInc = startPitch + (i*deltaPitch)/tweenFrames;
        noiseMod = startMod + (i*deltaMod)/tweenFrames;
        delay(frameTime);
      }
    }
  }
  delay(300);
}

SIGNAL(PWM_INTERRUPT)
{
  int8_t value;
  static int8_t noise;
  int16_t phaseNoise = noise * noiseMod;
  noise += noise<<2; noise++;  // noise' = 5*noise+1
  
  form1Phase += form1PhaseInc;
  value = pgm_read_byte(sinCalc+(((form1Phase>>8) & 0xf0) | form1Amp));
  form2Phase += form2PhaseInc;
  value += pgm_read_byte(sinCalc+(((form2Phase>>8) & 0xf0) | form2Amp));
  form3Phase += form3PhaseInc;
  value += pgm_read_byte(sqrCalc+(((form3Phase>>8) & 0xf0) | form3Amp));

  value = (value * (0xff^(pitchPhase>>8)))>>8;
  pitchPhase += pitchPhaseInc;
  if ((pitchPhase+phaseNoise) < pitchPhaseInc) {
    form1Phase = 0;
    form2Phase = 0;
    form3Phase = 0;
  }
  PWM_VALUE = value + 0x80;
}
