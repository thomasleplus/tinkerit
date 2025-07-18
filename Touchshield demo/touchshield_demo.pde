// touchshield_demo.pde
//
// Demo for the Liquidware Touchshield
//
// Compile using the Arduino environment.
//
// https://github.com/thomasleplus/tinkerit
//
// By Peter Knight, Tinker.it

#define NUM_BALLS 11
int ballPosnX[NUM_BALLS] = {-2, -1,-1,-1,-1,  0,0,  2, 2,2,2};
int ballPosnY[NUM_BALLS] = {-1, -2,-1, 0, 1, -1,2, -2,-1,0,2};

signed char cosTable[360] = {
  127,127,127,127,127,127,126,126,126,125,125,125,124,124,123,123,122,121,121,120,
  119,119,118,117,116,115,114,113,112,111,110,109,108,107,105,104,103,101,100,99,
  97,96,94,93,91,90,88,87,85,83,82,80,78,76,75,73,71,69,67,65,
  64,62,60,58,56,54,52,50,48,46,43,41,39,37,35,33,31,29,26,24,
  22,20,18,15,13,11,9,7,4,2,0,-2,-4,-7,-9,-11,-13,-15,-18,-20,
  -22,-24,-26,-29,-31,-33,-35,-37,-39,-41,-43,-46,-48,-50,-52,-54,-56,-58,-60,-62,
  -63,-65,-67,-69,-71,-73,-75,-76,-78,-80,-82,-83,-85,-87,-88,-90,-91,-93,-94,-96,
  -97,-99,-100,-101,-103,-104,-105,-107,-108,-109,-110,-111,-112,-113,-114,-115,-116,-117,-118,-119,
  -119,-120,-121,-121,-122,-123,-123,-124,-124,-125,-125,-125,-126,-126,-126,-127,-127,-127,-127,-127,
  -127,-127,-127,-127,-127,-127,-126,-126,-126,-125,-125,-125,-124,-124,-123,-123,-122,-121,-121,-120,
  -119,-119,-118,-117,-116,-115,-114,-113,-112,-111,-110,-109,-108,-107,-105,-104,-103,-101,-100,-99,
  -97,-96,-94,-93,-91,-90,-88,-87,-85,-83,-82,-80,-78,-76,-75,-73,-71,-69,-67,-65,
  -64,-62,-60,-58,-56,-54,-52,-50,-48,-46,-43,-41,-39,-37,-35,-33,-31,-29,-26,-24,
  -22,-20,-18,-15,-13,-11,-9,-7,-4,-2,0,2,4,7,9,11,13,15,18,20,
  22,24,26,29,31,33,35,37,39,41,43,46,48,50,52,54,56,58,60,62,
  64,65,67,69,71,73,75,76,78,80,82,83,85,87,88,90,91,93,94,96,
  97,99,100,101,103,104,105,107,108,109,110,111,112,113,114,115,116,117,118,119,
  119,120,121,121,122,123,123,124,124,125,125,125,126,126,126,127,127,127,127,127
};

float fcos(int angle)
{
  return cosTable[angle] / 127.;
}
float fsin(int angle)
{
  angle = (angle + 270);
  if (angle >= 360) angle-=360;
  return cosTable[angle] / 127.;
}

COLOR red = {255,0,0};
COLOR white = {255,255,255};
COLOR pink = {255,128,128};
COLOR black = {0,0,0};

void drawBall(int x,int y,int r) {
  COLOR red = {196+r*4,128-r*8,128-r*8};
  lcd_circle(x,y,r+1,black,black);
  lcd_circle(x,y,r,black,red);
  if (r>=4) lcd_circle(x-r/2,y-r/2,r/4,pink,white);
}

void renderBalls() {
  float ballX[NUM_BALLS];
  float ballY[NUM_BALLS];
  float ballZ[NUM_BALLS];
  static float angx = 0;
  static float angy = 0;
  static float angz = 0;
  
  for (int n=0; n<NUM_BALLS; n++) {
    int x = ballPosnX[n];
    int y = ballPosnY[n];
    int z = 0;

    // Rotate about x axis
    float x2 = x;
    float y2 = y * fcos(angx) - z * fsin(angx);
    float z2 = y * fsin(angx) + z * fcos(angx);

    // Rotate about y axis
    float x3 = x2 * fcos(angy) + z2 * fsin(angy);
    float y3 = y2;
    float z3 = -x2 * fsin(angy) + z2 * fcos(angy);

    // Rotate about z axis
    float x4 = x3 * fcos(angz) - y3 * fsin(angz);
    float y4 = x3 * fsin(angz) + y3 * fcos(angz);
    float z4 = z3;
    
    ballX[n] = x4;
    ballY[n] = y4;
    ballZ[n] = z4;
  }
  
  // Depth sort
  int increment = NUM_BALLS / 2;
  while (increment > 0) {
    for (int i = increment; i < NUM_BALLS; i++) {
      int j = i;
      float tempX = ballX[i];
      float tempY = ballY[i];
      float tempZ = ballZ[i];
      while ((j >= increment) && (ballZ[j-increment] > tempZ)) {
        ballX[j] = ballX[j - increment];
        ballY[j] = ballY[j - increment];
        ballZ[j] = ballZ[j - increment];
        j = j - increment;
      }
      ballX[j] = tempX;
      ballY[j] = tempY;
      ballZ[j] = tempZ;
    }
 
    if (increment == 2)
       increment = 1;
    else 
       increment = (int) (increment / 2.2);
  }

  // Render with perspective
  for (int i=0; i<NUM_BALLS; i++) {
    float scale = (ballZ[i] + 10);
    float x = ballX[i] * 2 * scale + 64;
    float y = ballY[i] * 2 * scale + 64;
    float r = scale;
    drawBall(x,y,r);
  }
  
  // Rotate camera
  angx+=1.;
  if (angx >= 360) angx -=360;
  angy+=0.3;
  if (angy >= 360) angy -=360;
  angz+=0.1;
  if (angz >= 360) angz -=360;

}

void setup()
{
  lcd_rectangle(0,0,127,127,black,black);
}

char randomNum() {
  static int seed=1;
  seed = (seed * 5)+1;
  return seed;
}

#define flip(a) (0x7f ^ a)

void loop()
{
  char x1=randomNum() & 0x7f;
  char x2=randomNum() & 0x7f;
  char y1=randomNum() & 0x7f;
  char y2=randomNum() & 0x7f;
  char x1f = flip(x1);
  char x2f = flip(x2);
  char y1f = flip(y1);
  char y2f = flip(y2);
  
  COLOR randCol = {0,randomNum() * 0x3f,randomNum()};
  lcd_line(x1,y1,x2,y1,randCol);
  renderBalls();
}
