// including all the necessary libraries
#include <EasyTransfer.h>
#include <SoftwareSerial.h>
#include <RGBmatrixPanel.h>

// defining arduino ID number and pins we want to look at
#define NODEID 3
#define NUMNODES 4; 

#define CLK  8 
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2

// creating a Transfer object
EasyTransfer ET;

// creating a datat type for receiving/transmitting data
struct RECEIVE_DATA_STRUCTURE{
  int state;
  char from;
  char to;
  int c1;
  int c2;
  bool push;
};

// creating such data type
RECEIVE_DATA_STRUCTURE mydata;

// 0 -> choose warships, 1 -> the player 1 hits the opponent, 2 -> the player 2 hits the opponent, 3 -> end of the game
int state;

int grid[8][8]; //my grid, 0 -> sea, 1 -> ship, 2 -> miss, 3 -> hit 

// creating matrix for LED matrix panel
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

/* function for the 30 seconds at the beginning of the game to choose your ships
the time will be shown on the LED display by turning on pixels every second
*/
void timer() {
  for(uint8_t x = 0; x < 32; x++){
    for(uint8_t y = 0; y < 16; y++){
      matrix.drawPixel(x, y, matrix.Color333(7, 0, 0));
    }
    for (int j = 0; j < 100; j++) {
      multicom_update();
      delay(10);
    }
  }
}

// what the LED display shows when the player loses
void lose(){
  for (int i = 0; i < 16; i++){
    for (int j = 0; j < 16; j++){
      matrix.drawPixel(i, j, matrix.Color333(7, 0, 0));
    }
  }
}

// what the LED display shows when the player wins
void win(){
  for (int i = 0; i < 16; i++){
    for (int j = 0; j < 16; j++){
      matrix.drawPixel(i, j, matrix.Color333(0, 7, 0));
    }
  }
}

// for the game logic and drawing the ships on the LED display
void drawstate(){
  if (state==2){
    for (int i = 0; i < 16; i++){
      for (int j = 0; j < 16; j++){
        matrix.drawPixel(i, j, matrix.Color333(0, 0, 0));
      }
    }
    matrix.drawPixel(1, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(1, 6, matrix.Color333(7, 0, 0));
    matrix.drawPixel(1, 7, matrix.Color333(7, 0, 0));
    matrix.drawPixel(1, 8, matrix.Color333(7, 0, 0));
    matrix.drawPixel(1, 9, matrix.Color333(7, 0, 0));
    matrix.drawPixel(4, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(4, 6, matrix.Color333(7, 0, 0));
    matrix.drawPixel(4, 7, matrix.Color333(7, 0, 0));
    matrix.drawPixel(4, 8, matrix.Color333(7, 0, 0));
    matrix.drawPixel(4, 9, matrix.Color333(7, 0, 0));
    matrix.drawPixel(2, 7, matrix.Color333(7, 0, 0));
    matrix.drawPixel(3, 7, matrix.Color333(7, 0, 0));
    matrix.drawPixel(7, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(7, 6, matrix.Color333(7, 0, 0));
    matrix.drawPixel(7, 7, matrix.Color333(7, 0, 0));
    matrix.drawPixel(7, 8, matrix.Color333(7, 0, 0));
    matrix.drawPixel(7, 9, matrix.Color333(7, 0, 0));
    matrix.drawPixel(6, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(8, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(6, 9, matrix.Color333(7, 0, 0));
    matrix.drawPixel(8, 9, matrix.Color333(7, 0, 0));
    matrix.drawPixel(10, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(11, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(12, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(13, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(14, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(12, 5, matrix.Color333(7, 0, 0));
    matrix.drawPixel(12, 6, matrix.Color333(7, 0, 0));
    matrix.drawPixel(12, 7, matrix.Color333(7, 0, 0));
    matrix.drawPixel(12, 8, matrix.Color333(7, 0, 0));
    matrix.drawPixel(12, 9, matrix.Color333(7, 0, 0));
  }
  else if (state == 1){
    for (int i = 0; i < 16; i++){
      for (int j = 0; j < 16; j++){
        matrix.drawPixel(i, j, matrix.Color333(0, 0, 0));
      }
    } 
  }
}

// checks if ship alive or not
bool alive(){
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      if (grid[i][j] == 1)
        return true;
    }
  }
  return false;
}

// draws the grid of ships at the very beginning
void drawgrid() {
  for(uint8_t x = 16; x < 32; x++){
    for(uint8_t y = 0; y < 16; y++){
      if (grid[(x-16)/2][y/2] == 1){
        matrix.drawPixel(x, y, matrix.Color333(7, 7, 7));
      }
      else if (grid[(x-16)/2][y/2] == 0){
        matrix.drawPixel(x, y, matrix.Color333(0, 0, 7));        
      }
      else if (grid[(x-16)/2][y/2] == 2){
        matrix.drawPixel(x, y, matrix.Color333(0, 0, 0));        
      }
      else if (grid[(x-16)/2][y/2] == 3){
        matrix.drawPixel(x, y, matrix.Color333(7, 0, 0));        
      }
    }
  }
}

// things are initialized here
void setup() {
  state=0;
  Serial.begin(9600);
  matrix.begin();
  ET.begin(details(mydata), &Serial);
  Serial.println("starting Arduino 3 (display)");
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      grid[i][j]=0;
    }
  }
    for(uint8_t x = 0; x < 32; x++){
      for(uint8_t y = 0; y < 16; y++){
        matrix.drawPixel(x, y, matrix.Color333(7, 7, 7));
      }
  }
  timer ();
  for(uint8_t x = 0; x < 16; x++){
    for(uint8_t y = 0; y < 16; y++){
      matrix.drawPixel(x, y, matrix.Color333(0, 0, 0));
    }
  }
  drawgrid();
  state=1;
  drawstate();
}

// always updates the communication to see if any changes have been made
void loop() {
  multicom_update();
}

// update function, code for receiving data
void multicom_update()
{
 while(ET.receiveData())
  {
     if (mydata.to == NODEID) {
        Serial.println("Node 3 received info");
        multicom_receive();
     }
  }
}

/*
Code for sending data the data - we send from which arduino we are sending, 
to which arduino we are sending, the two coordinates we are sending and whether 
the button was pushed or not
*/
void multicom_send(int state, char to, int c1, int c2, bool push)
{
  mydata.state = state ;
  mydata.from = NODEID ;
  mydata.to = to ;
  mydata.c1 = c1 ; 
  mydata.c2 = c2 ;
  mydata.push = push ;
  ET.sendData() ; 
}

/*
Code for receiving the data - based on received info changing the 
pixels on the LED display
 */
void multicom_receive()
{
  delay(100);
  if (mydata.state != -1){
    Serial.println("State just changed");
    state=mydata.state;
    drawstate();
    if (state==3){
      win();
    }
  }
  else {
    if (state==0){
      if (mydata.push){
        grid[mydata.c2][mydata.c1] = 1;
      }
      else {
        grid[mydata.c2][mydata.c1] = 0;
      }
    }
    if (state==1){
      if (grid[mydata.c2][mydata.c1] == 1){
        grid[mydata.c2][mydata.c1] = 3;
        if (alive()){
        for (int i = 0; i < 10; i++){
          multicom_send(-1, 1, mydata.c1, mydata.c2, true);
          delay(10);
        }
        Serial.println("Sent to node 1 true");
        }
        else{
          lose();
            for (int i = 0; i < 10; i++){
                multicom_send(3, 1, 0, 0, false);
                delay(5);
            }
          for (int i = 0; i < 10; i++){
              multicom_send(3, 2, 0, 0, false);
              delay(5);
           }
           for (int i = 0; i < 10; i++){
              multicom_send(3, 4, 0, 0, false);
              delay(5);
            }
        }
      }
      else if (grid[mydata.c2][mydata.c1] == 0){
         for (int i = 0; i < 10; i++){
          multicom_send(-1, 1, mydata.c1, mydata.c2, false);
          delay(5);
        }
        grid[mydata.c2][mydata.c1] = 2;
      }
      else {
        Serial.println("wtf");
      }
      drawgrid();
    }
  }
}
