#include <IRremote.h>

#define UP 0x511DBB
#define RIGHT 0x20FE4DBB
#define LEFT 0x52A3D41F
#define DOWN 0xA3C8EDDB

// 2-dimensional array of row pin numbers:
const int row[8] = {
  15, 7, 19, 5, 12, 18, 13, 16
};

// 2-dimensional array of column pin numbers:
const int col[8] = {
  6, 11, 10, 3, 17, 4, 8, 9
};

// 2-dimensional array of pixels:
int pixels[8][8];

typedef struct Snake Snake;
struct Snake{
  int head[2];     // the (row, column) of the snake head
  int body[40][2]; //An array that contains the (row, column) coordinates
  int len;         //The length of the snake 
  int dir[2];      //A direction to move the snake along
};

//Define The Apple as a Struct
typedef struct Apple Apple;
struct Apple{
  int rPos; //The row index of the apple
  int cPos; //The column index of the apple
};

//Variables To Handle The Game Time
float oldTime = 0;
float timer = 0;
float updateRate = 3;

Snake snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};//Initialize a snake object
Apple apple = {(int)random(0,8),(int)random(0,8)};//Initialize an apple object
byte pic[8] = {0,0,0,0,0,0,0,0};//The 8 rows of the LED Matrix

int RECV_PIN = 2; // define input pin on Arduino 
IRrecv irrecv(RECV_PIN); 
decode_results results; // decode_results class is defined in IRremote.h
int i,j;//Counters

void setup() { 
  // initialize the I/O pins as outputs iterate over the pins:
  for (int thisPin = 0; thisPin < 8; thisPin++) {
    // initialize the output pins:
    pinMode(col[thisPin], OUTPUT);
    pinMode(row[thisPin], OUTPUT);
    // take the col pins (i.e. the cathodes) high to ensure that the LEDS are off:
    digitalWrite(col[thisPin], HIGH);
  }
  // initialize the pixel matrix:
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      pixels[x][y] = HIGH;
    }
  }
	Serial.begin(9600); 
	irrecv.enableIRIn(); // Start the receiver 
} 

void loop() { 
  float deltaTime = calculateDeltaTime();
  timer += deltaTime;
	if (irrecv.decode(&results)) {
    switch(results.value){
      case UP: Serial.println("Up");
        if(snake.dir[1] == 0){
          snake.dir[0] = 0;
          snake.dir[1] = 1;
        }
        break;
      case RIGHT: Serial.println("Right");
          if(snake.dir[0] == 0){
          snake.dir[0] = 1;
          snake.dir[1] = 0;
        }
        break;
      case LEFT: Serial.println("Left");
        if(snake.dir[0] == 0){
          snake.dir[0] = -1;
          snake.dir[1] = 0;
        }
        break;
      case DOWN: Serial.println("Down");
          if(snake.dir[1] == 0){
          snake.dir[0] = 0;
          snake.dir[1] = -1;
        }

        break;
    }
		irrecv.resume(); // Receive the next value 
	}
  //Update
  if(timer > 1000/updateRate){
    timer = 0;
    Update();
  }
  
  //Render
  displayLedPattern();
	//delay (100); // small delay to prevent reading errors
}

float calculateDeltaTime(){
  float currentTime = millis();
  float dt = currentTime - oldTime;
  oldTime = currentTime;
  return dt;
}

void displayLedPattern(){
  for (int thisRow = 0; thisRow < 8; thisRow++) {
    digitalWrite(row[thisRow], HIGH);
    for (int thisCol = 0; thisCol < 8; thisCol++) {
      // get the state of the current pixel;
      int thisPixel = pic[thisRow] & (128 >> thisCol);
      if (thisPixel != 0)
        thisPixel = LOW;
      else
        thisPixel = HIGH;
      // when the row is HIGH and the col is LOW,
      // the LED where they meet turns on:
      digitalWrite(col[thisCol], thisPixel);
      // turn the pixel off:
      if (thisPixel == LOW) {
        digitalWrite(col[thisCol], HIGH);
      }
    }
    // take the row pin low to turn off the whole row:
    digitalWrite(row[thisRow], LOW);
  }
}

void reset(){
  for(int j=0;j<8;j++){
    pic[j] = 0;
  }
}

void Update(){
  reset();//Reset (Clear) the 8x8 LED matrix
  
  int newHead[2] = {snake.head[0]+snake.dir[0], snake.head[1]+snake.dir[1]};

  //Handle Borders
  if(newHead[0]==8){
    newHead[0]=0;
  }else if(newHead[0]==-1){
    newHead[0] = 7;
  }else if(newHead[1]==8){
    newHead[1]=0;
  }else if(newHead[1]==-1){
    newHead[1]=7;
  }
  
  //Check If The Snake hits itself
   for(j=0;j<snake.len;j++){
    if(snake.body[j][0] == newHead[0] && snake.body[j][1] == newHead[1]){
      //Pause the game for 1 sec then Reset it
      delay(1000);
      snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};//Reinitialize the snake object
      apple = {(int)random(0,8),(int)random(0,8)};//Reinitialize an apple object
      return;
    }
  }

  //Check if The snake ate the apple
  if(newHead[0] == apple.rPos && newHead[1] ==apple.cPos){
    snake.len = snake.len+1;
    apple.rPos = (int)random(0,8);
    apple.cPos = (int)random(0,8);
  }else{
    removeFirst();//Shifting the array to the left
  }
  
  snake.body[snake.len-1][0]= newHead[0];
  snake.body[snake.len-1][1]= newHead[1];
  
  snake.head[0] = newHead[0];
  snake.head[1] = newHead[1];
  
  //Update the pic Array to Display(snake and apple)
  for(j=0;j<snake.len;j++){
    pic[snake.body[j][0]] |= 128 >> snake.body[j][1];
  }
  pic[apple.rPos] |= 128 >> apple.cPos;
  /*
  for(j=0;j<snake.len;j++){
    for(i = 0; i < 8; i++)
      pixels[snake.body[j][0]][i] = HIGH;
    pixels[snake.body[j][0]][snake.body[j][1]] = LOW;
  }
  for(i = 0; i < 8; i++)
    pixels[apple.rPos][i] = HIGH;
  pixels[apple.rPos][apple.cPos] = LOW;
  */
}

void removeFirst(){
  for(j=1;j<snake.len;j++){
    snake.body[j-1][0] = snake.body[j][0];
    snake.body[j-1][1] = snake.body[j][1];
  }
}
