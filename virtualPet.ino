#include <EEPROM.h>
#include <Adafruit_RGBLCDShield.h>

#define START -1
#define IDL 0
#define FEED 1
#define PLAY 2
#define GROW 3
#define MENU 4
#define GREEN 0x2
#define WHITE 0x7
#define RED 0x1

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//myPet variables
struct Age{
  int minutes;
  int seconds;
}; 

struct Pet{
  int development;
  int happiness;
  int fullness;
  Age petAge;
}; 

Pet myPet;
Pet loadPet;
int totalTime=0; //stores total Seconds

//animations variables

boolean displayRed= false;
int defRow = 1;
int c=3;
int ballColumn =6;

//timers variables
unsigned long time;
unsigned long defenderTime = 0;
unsigned long startAnimationPlay=0;
unsigned long startAnimation=0;
unsigned long startTime=0; //overall timer
unsigned long startTimeH = 5;
unsigned long startTimeF = 5;
unsigned long startTimeCLICK=0;
const int intervalHappiness = 7;
const int intervalFullness = 11;
const int intervalCLICK =750;
unsigned long colorDisplay=0; //maybe change to startDisplay
bool go=false;
unsigned long goalMessage;
boolean goal;
boolean checkGoal;

//checking for clicks
uint8_t buttons;
bool clickedRIGHT=false;
bool clickedLEFT=false;
bool clickedSELECT=false;
bool clickedDOWN=false;
bool clickedUP=false;

//switch cases
int IDLdisplay=0;
int STARTdisplay=0;
int MENUdisplay=0;
int state;//the overall program
int FEEDstate =1;
int PLAYstate = 1;

//custom characters
uint8_t downCHAR[] = {B00100,B00100,B00100,B00100,B10101,B01010,B00100,B00000};
uint8_t upCHAR[] = {B00000,B00100,B01010,B10101,B00100,B00100,B00100,B00000};
uint8_t leftCHAR[] = {B00000,B00010,B00100,B01000,B10111,B01000,B00100,B00010};
uint8_t rightCHAR[] = {B00000,B01000,B00100,B00010,B11101,B00010,B00100,B01000};
byte defender[] = {B00000,B10000,B10000,B10000,B10000,B10000,B10000,B00000};

byte goalDown[] = {
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B11111
};

byte goalUp[] = {
  B11111,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001,
  B00001
};


byte ball[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00110,
  B00110,
  B00000
};

byte egg[] = {
  B00100,
  B01010,
  B10001,
  B10101,
  B11011,
  B10001,
  B01010,
  B00100
};

byte adult[] = {
  B00011,
  B00110,
  B00111,
  B11110,
  B11110,
  B11110,
  B10100,
  B10100
};

byte young[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00110,
  B11111,
  B11100,
  B10100
};

byte banana[] = {
  B00000,
  B00000,
  B10000,
  B10000,
  B11000,
  B11100,
  B01110,
  B00111
};

uint8_t sadFace[] = {
  B00000,
  B00000,
  B00000,
  B01010,
  B00000,
  B01110,
  B10001,
  B00000
};

uint8_t contentFace[] = {
  B00000,
  B00000,
  B00000,
  B01010,
  B00000,
  B10001,
  B01110,
  B00000
};

uint8_t happyFace[] = {
  B00000,
  B00000,
  B00000,
  B01010,
  B00000,
  B11111,
  B01110,
  B00000
};

byte overfull[] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000
};

byte ravenous[] = {
  B00000,
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
  B00000
};

byte hungry[] = {
  B00000,
  B11111,
  B10001,
  B10001,
  B10001,
  B11111,
  B11111,
  B00000
};


byte peckish[] = {
  B00000,
  B11111,
  B10001,
  B10001,
  B11111,
  B11111,
  B11111,
  B00000
};

byte full[] = {
  B00000,
  B11111,
  B10001,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000
};


void setup() {
  
    Serial.begin(9600);
    lcd.begin(16,2);

    Serial.println("DOING SETUP--------------");


    //creating characters
    lcd.createChar(1, downCHAR);
    lcd.createChar(2, upCHAR);
    lcd.createChar(3, leftCHAR);
    lcd.createChar(4, rightCHAR);
    lcd.createChar(8,banana);

    //getting the pet from the EEPROM
    EEPROM.get(0,loadPet);
    myPet=loadPet;
    
    //checking if there is a saved pet by checking the age
    if (myPet.petAge.seconds !=0 || myPet.petAge.minutes!=0){
        state=START; //to ask if load or not
    } else{
        myPet={0,2,3,{0,0}};//creates new pet
        state=IDL;
        createCharDevelopment(myPet.development);
        createCharFullness(myPet.fullness);
        createCharHappiness(myPet.happiness);
    }

    Serial.println("STATS FROM EEPROM");
    showStats(); 
}

void loop() {
  
    time=millis();

    buttons = lcd.readButtons();

    //reading buttons
    checkSELECT();
    checkUpDown();
    checkLeftRight();

    //timer to keep track of the pet's age
    if ((unsigned long)time - startTime >= 1000 && (state!=MENU && state!=START && PLAYstate!=2 )){
        totalTime +=1;
        startTime=(time/1000)*1000;//flooring time
    }

    //timer in order to not let two clicks to be recorded when trying to press it just once
    if(startTimeCLICK+200<time){
        startTimeCLICK=time;
        go=true;
    } else {
        go=false;
    }

    //converting total time to minutes and seconds and assigning those to myPet
    myPet.petAge.seconds=(totalTime%60);
    myPet.petAge.minutes=(totalTime/60);

    //GENERAL CONDITIONS
    if (myPet.petAge.seconds>=5 || myPet.petAge.minutes>0){ //if pet reaches at least 5 seconds
        if (myPet.petAge.minutes==0 && myPet.development<1){
            myPet.development=1;//it will happen only the first time seconds is 5
            createCharDevelopment(myPet.development);
            Serial.println("--------GROWING AFTER FIRST 5 s");
            showStats();
        }

        //HAPPINESS TIMER EVERY 7 SECONDS, decreases happiness by one
        if (startTimeH+7<=totalTime){
            startTimeH=totalTime;
            if (myPet.happiness>0){
                myPet.happiness-=1;
                createCharHappiness(myPet.happiness);
                Serial.println("-----------7 SECONDS INTERVAL");
                showStats();
            }
        }

        //FULLNESS TIMER EVERY 11 SECONDS, decreases fullness by one
        if (startTimeF+11 <= totalTime){
            startTimeF=totalTime;
            if (myPet.fullness>0){
                myPet.fullness-=1;
                createCharFullness(myPet.fullness);
                Serial.println("----------11 SECONDS INTERVAL");
                showStats();
            }
            if (myPet.fullness==0 && myPet.happiness!=0){//setting happiness to 0 if fullness is 0
                myPet.happiness=0;
                createCharHappiness(myPet.happiness);
                Serial.println("---------FULLNESS IS 0");
                showStats();
            }
        }  
    }

    //changing main states with left and right clicks except for menu which needs to click leave menu
    if (clickedLEFT && go){ //LEFT CLICK
        setButtonsFalse();
        if (state==IDL) {
            Serial.println("---------GOING TO MENU");
            showStats();
            state=MENU;
            lcd.clear();
        } else if(state != MENU && state != START && PLAYstate!=2){
            state-=1;
            lcd.clear();
        }
    } else if (clickedRIGHT && go ){ //RIGHT CLICK
        setButtonsFalse();
        if(state!=MENU && state != START && PLAYstate!=2){
            state+=1;
            if (state==MENU) {

                Serial.println("---------GOING TO MENU");
                showStats();
            }
            lcd.clear(); 
        }
    }

    switch(state){

        case START: //It only gets accesed when ARDUINO is connected and there is a pet in EEPROM or if pet is saved in MENU
            lcd.setCursor(6,0);
            lcd.print("START");

            //changing inner display of START
            if (clickedDOWN && go){
                setButtonsFalse();
                if (STARTdisplay != 1){
                    STARTdisplay+=1;
                    lcd.clear();
                }
            } else if (clickedUP && go){
                setButtonsFalse();
                if (STARTdisplay != 0){
                    STARTdisplay-=1;
                    lcd.clear();
                }
            }

            //DISPLAY OF START (INNER STATE)
            switch (STARTdisplay){
                case 0: //LOAD PET
                    
                    //DISPLAY
                    lcd.setCursor(0,0);
                    lcd.write(1); //down arrow
                    lcd.setCursor(3,1);
                    lcd.print("<LOAD PET>");

                    //functionality for load pet
                    if (clickedSELECT && go){
                        setButtonsFalse();

                        //getting values from memory
                        EEPROM.get(0,loadPet); //gets pet from EEPROM
                        myPet=loadPet;
                        //updating variables
                        totalTime = (myPet.petAge.minutes *60) + myPet.petAge.seconds; //GETS new totalTime
                        startTimeH = totalTime;
                        startTimeF = totalTime;
                        createCharDevelopment(myPet.development);
                        createCharFullness(myPet.fullness);
                        createCharHappiness(myPet.happiness);

                        lcd.clear();
                        state+=1; //goes to IDL

                        time=millis();
                        startTime=time;
                        
                        Serial.println("----------LOADING SAVED PET");
                        showStats();
                    }
                break;

                case 1: //NEW PET
                
                    //DISPLAY
                    lcd.setCursor(15,0);
                    lcd.write(2); //UP arrow
                    lcd.setCursor(3,1);
                    lcd.print("<NEW PET>");

                    //functionality for new Pet  
                    if (clickedSELECT && go){
                        setButtonsFalse();

                        //reseting values
                        myPet={0,2,3,{0,0}}; //creates new PET with initial values
                        createCharDevelopment(myPet.development);
                        createCharFullness(myPet.fullness);
                        createCharHappiness(myPet.happiness);
                        startTimeH=5;
                        startTimeF=5;
                        totalTime=0;
                        
                        lcd.clear();
                        state+=1; //goes to IDL

                        time=millis();
                        startTime=time;
                        
                        Serial.println("-----------LOADING NEW PET"); 
                        showStats();
                    }
                break;
            }
        break;


        case IDL:

              //SHOWING ALL STATS
              lcd.setCursor(0,1);
              lcd.write(3);//arrow
              lcd.setCursor(15,1);
              lcd.write(4);//arrow
  
              lcd.setCursor(2,1);
              lcd.print("AGE:");
              
              if (myPet.petAge.minutes>=10){ //if simulation reaches 9:59 it stops displaying the time
                lcd.print("9");
                lcd.print(":");
                lcd.print("59");
              }else{
                lcd.print(myPet.petAge.minutes);
                lcd.print(":");
                displaySeconds(myPet.petAge.seconds);
              }
            
              lcd.setCursor(11,1);
              lcd.print("D:");
              lcd.write(7);
              lcd.setCursor(2,0);
              lcd.print("F:");
              lcd.write(6);
              lcd.setCursor(11,0);
              lcd.print("H:");
              lcd.write(5);
 

        break;

        case FEED:

            //INNER STATE FOR FEED, EITHER ANIMATION OR SHOWING STAT
            switch (FEEDstate){
                case 1: //showing stats

                    //GENERAL DISPLAY 
                    lcd.setCursor(8,1);
                    lcd.write(8);
                    lcd.setCursor(3,1);
                    lcd.write(7);
                    lcd.setCursor(0,1);
                    lcd.write(3); //arrow LEFT
                    lcd.setCursor(15,1);
                    lcd.write(4); //arrow RIGHT
                    lcd.setCursor(3,0);
                    lcd.print("FEED?");
                    lcd.setCursor(11,0);
                    lcd.write(6);
                    lcd.setCursor(13,0);
                    lcd.write(5);

                    //FUNCTIONALITY FEED

                    if (clickedSELECT && go){ //go is used as a delay so that software only register one click
                        setButtonsFalse();

                        //user is allowed to feed
                        if(myPet.development>0 && myPet.fullness<4){
                            if (myPet.fullness==3 && myPet.happiness!=0){//SETTING HAPPINESS TO 0 IF OVERFULL 
                                myPet.happiness = 0;
                                createCharHappiness(myPet.happiness);
                                Serial.println("--------------FULLNESS IS OVERFULL");
                                showStats();
                            }

                            myPet.fullness+=1;
                            createCharFullness(myPet.fullness);

                            //changing internal state to do animation
                            FEEDstate=2;
                            lcd.setBacklight(GREEN);

                        } else { //if development is 0 user is not allowed to feed
                            lcd.setBacklight(RED);
                            colorDisplay= millis();
                            displayRed= true;
                        }

                        Serial.println("---------DOING FEED");
                        showStats();
                    }

                    //creates a delay using millis so that the RED LIGHT appears as a blink if there is an error
                    if ( 300 + colorDisplay < millis() && displayRed){
                        lcd.setBacklight(WHITE);
                        displayRed=false;
                    }

                break;

                case 2: //animation
                    lcd.setCursor(11,0);
                    lcd.write(6);
                    lcd.setCursor(13,0);
                    lcd.write(5);
                    lcd.setCursor(3,0);
                    lcd.print("FEED");
                    lcd.setCursor(8,1);
                    lcd.write(8);

                    feedAnimation();
                break;
            }

        break;

        case PLAY:

            //INNER STATE EITHER PLAYING OR SHOWING STAT
            switch (PLAYstate){

                case 1: //normal state

                    //DISPLAY
                    lcd.setCursor(3,0);
                    lcd.print("PLAY?");
                    lcd.setCursor(0,1);
                    lcd.write(3);//arrow left
                    lcd.setCursor(15,1);
                    lcd.write(4);//arrowright
                    lcd.setCursor(3,1);
                    lcd.write(7);
                    lcd.setCursor(11,0);
                    lcd.write(6);
                    lcd.setCursor(13,0);
                    lcd.write(5);

                    //FUNCTIONALITY PLAY

                    if (clickedSELECT && go){
                        setButtonsFalse();

                        if (myPet.happiness<2 && myPet.fullness>=2){//user is allowed to play
                            myPet.happiness+=1;
                            createCharHappiness(myPet.happiness);
                            Serial.println(myPet.happiness);

                            //CHANGING INNER STATE IN ORDER TO PLAY GAME
                            PLAYstate=2;
                            checkGoal=true;
                            lcd.clear();
                            createCharGame();
                            c=3;
                            Serial.println("---------DOING PLAY");
                            showStats();
                        }else{ //user not allowed to play
                            lcd.setBacklight(RED);
                            colorDisplay= millis();
                            displayRed= true;
                        }
                    }

                    //creates a delay using millis so that the red light appears as a blink IF ERROR
                    if ( 300 + colorDisplay < millis() && displayRed==true){
                        lcd.setBacklight(WHITE);
                        displayRed=false;
                    }

                break;

                case 2://game state

                    //MOVING THE PET AROUND
                    if (buttons){
                        if ((buttons & BUTTON_RIGHT)){
                            setButtonsFalse();
                            if (c!=6) c+=1;
                            lcd.clear();
                            writeChars();
                        }else if ((buttons & BUTTON_LEFT) ){
                            setButtonsFalse();
                            if (c!=0) c-=1;
                            lcd.clear();
                            writeChars();
                        }
                    }

                    if (checkGoal){ //only check if it is goal once, once it is checked stops animations
                      
                        defenderAnimation(); //SETS THE BAR DEFINDING THE GOAL

                        if (c==6){ //when pet gets to the ball animation of ball begins
                            playAnimation();
                        }
    
                        if (ballColumn==14) { //game stops when the ball reaches the goal
    
                            goal = true;
                            checkGoal = false;
                            goalMessage = millis();
                                                        
                            if(defRow ==1){
                              lcd.setCursor(6,0);
                              lcd.print("NO GOAL");
                              lcd.setBacklight(RED);
                            } else {
                              lcd.setCursor(6,0);
                              lcd.print("GOAL");
                              lcd.setBacklight(GREEN);
                            }   
                        }
                    }

                    
                    //timer to stop the game and show goal message
                    if ( 1000 + goalMessage < millis() && goal){
                        goal=false;
                        lcd.clear();
                        //reseting characters
                        lcd.createChar(1, downCHAR);
                        lcd.createChar(2, upCHAR);
                        lcd.createChar(3, leftCHAR);
                        lcd.createChar(4, rightCHAR);

                        //reseting values
                        defRow=15;
                        ballColumn =6;
                        createCharHappiness(myPet.happiness);
                        c=3;

                        //changing state to normal state
                        PLAYstate=1;
                        time=millis();
                        startTime=time;
                        lcd.setBacklight(WHITE);
                    }
                break;
            }

        break;

        case GROW:

            //DISPLAY
            lcd.setCursor(3,0);
            lcd.print("GROW?");
            lcd.setCursor(0,1);
            lcd.write(3);//left arrow
            lcd.setCursor(15,1);
            lcd.write(4);//right arrow
            lcd.setCursor(8,1);
            lcd.write(7);
            lcd.setCursor(11,0);
            lcd.write(6);
            lcd.setCursor(13,0);
            lcd.write(5);

            //FUNCTIONALIY GROW

            if (clickedSELECT && go){ //grow pet
                setButtonsFalse();

                //growing from 1 to 2 if allowed
                if ((myPet.petAge.seconds>=35 || myPet.petAge.minutes>=1) && myPet.happiness>=1 && myPet.fullness>=3 && myPet.development!=2 ){ //user is allowed to grow
                    myPet.development=2;
                    createCharDevelopment(myPet.development);
                    lcd.setBacklight(GREEN);
                    colorDisplay= millis();
                }else{ //user not allowed to grow
                    lcd.setBacklight(RED);
                    colorDisplay= millis();
                }

                Serial.println("---------DOING GROW");
                showStats();
            }

            //creates a delay using millis so that the green or red light appears as a blink IF ERROR
            if ( 300 + colorDisplay < millis()){
                lcd.setBacklight(WHITE);
            }

        break;

        case MENU:

            lcd.setCursor(6,0);
            lcd.print("MENU");

            //changing inner state of menu
            if (clickedDOWN && go){
                setButtonsFalse();
                if (MENUdisplay != 3){
                    MENUdisplay+=1;
                    lcd.clear();
                }
            } else if (clickedUP && go){
                setButtonsFalse();
                if (MENUdisplay != 0){
                    MENUdisplay-=1;
                    lcd.clear();
                }
            }      

            //INNER STATE FOR DIPSLAY
            switch (MENUdisplay){
                case 0: //SAVE PET
                    //DISPLAY
                    lcd.setCursor(0,0);
                    lcd.write(1);//down arrow
                    lcd.setCursor(3,1);
                    lcd.print("<SAVE PET>");

                    if (clickedSELECT){ //user selects to save pet
                        setButtonsFalse();
                        EEPROM.put(0,myPet); //saves pet to EEPROM
                        lcd.clear();
                        state=START; //goes to START where user can choose between loading pet or starting new
                        Serial.println("---------SAVING PETS");
                        showStats();
                    }
                break;

                case 1: //DELETE PET does not start a new one, it carries on with the previous one

                    //DISPLAY
                    lcd.setCursor(0,0);
                    lcd.write(2);//up arrow
                    lcd.setCursor(15,0);
                    lcd.write(1);//down arrow
                    lcd.setCursor(3,1);
                    lcd.print("<DELETE PET>");

                    if (clickedSELECT){
                        setButtonsFalse();
                        Pet restartPet={0,2,3,{0,0}}; //gives myPet initial values
                        EEPROM.put(0,restartPet); //saves into EEPROM myPet with initial values so it deletes old one

                        lcd.clear();
                        state=IDL;

                        time=millis();
                        startTime=time;
                        
                        Serial.println("---------DELETING PETS");
                        showStats();
                    }
                break;

                case 2://START NEW
                    //this does not deletes previous pet so if it is not saved, the previous will be loaded next time, to save click on SAVE PET on menu
                    lcd.setCursor(0,0);
                    lcd.write(1);//down arrow
                    lcd.setCursor(15,0);
                    lcd.write(2); //UP arrow
                    lcd.setCursor(3,1);
                    lcd.print("<START NEW>");

                    if (clickedSELECT){
                        setButtonsFalse();
                        myPet={0,2,3,{0,0}}; //myPet with initial values
                        //RESTART VALUES
                        totalTime =0; 
                        createCharDevelopment(myPet.development);
                        createCharFullness(myPet.fullness);
                        createCharHappiness(myPet.happiness);
                        startTimeH=5;
                        startTimeF=5;
                        lcd.clear();
                        state=IDL; //GO TO IDL

                        time=millis();
                        startTime=time;
                        
                        Serial.println("---------NEW PET");
                        showStats();
                    }
                break;

                case 3://LEAVE MENU
                    lcd.setCursor(15,0);
                    lcd.write(2);//up arrow
                    lcd.setCursor(3,1);
                    lcd.print("<LEAVE MENU>");

                    if (clickedSELECT){
                      setButtonsFalse();
                      lcd.clear();
                      state=IDL; //goes to IDL

                      time=millis();
                      startTime=time;
                      
                      Serial.println("---------LEAVE MENU");
                      showStats();
                    }
                break;
            }
        break;
    }
}



//FUNCTIONS
void displaySeconds(int currentTime){
  //This function is used to modify the display of the seconds. When the seconds is a single digit then it 
  //adds a zero in front of it. If the numbr is two digits then it doesn't add a zero and displays it as it is
  int seconds;
  seconds = currentTime%60;
  if (seconds<10){
    lcd.print("0");
    lcd.print(seconds);
  } else{
    lcd.print(seconds);
  }  
}

void checkSELECT(){
  //This function checks if the SELECT button has been clicked. If it has been clicked then 
  //it assigns a value of true to the global variable clickedSELECT
  if (buttons){
    if (buttons & BUTTON_SELECT){
      clickedSELECT=true;
    }
  }
}

void checkLeftRight(){
  if (buttons){//&& (state!=START && state!=MENU)
    if (buttons & BUTTON_LEFT){
      clickedRIGHT=false;
      clickedLEFT=true;
    }
    if (buttons & BUTTON_RIGHT){
      clickedRIGHT=true;
      clickedLEFT=false; 
    }
  }
}

void checkUpDown(){
  if (buttons){
        if ((buttons & BUTTON_DOWN)){//&& *CASEdisplay<maxCASE
            clickedDOWN = true;
            clickedUP = false; 
        }else if ((buttons & BUTTON_UP) ){//&& *CASEdisplay>minCASE
            clickedDOWN = false;
            clickedUP = true;
        }
   }
}

void showStats(){
  // in order to test the program using the Serial monitor
  Serial.println("NEW STATS ARE");
  Serial.print("Development: ");
  Serial.println(myPet.development);
  Serial.print("Happiness: ");
  Serial.println(myPet.happiness);
  Serial.print("Fullness: ");
  Serial.println(myPet.fullness);
  Serial.print("Minutes: ");
  Serial.println(myPet.petAge.minutes);
  Serial.print("Seconds:  ");
  Serial.println(myPet.petAge.seconds);
}

void createCharHappiness(int happiness){  
  if (happiness==0){
    lcd.createChar(5, sadFace);
  }else if (happiness==1){
    lcd.createChar(5, contentFace);
  }else{
    lcd.createChar(5, happyFace);
  }
}

void createCharFullness(int fullness){
  if (fullness==0){
    lcd.createChar(6, ravenous);
  }else if (fullness==1){
    lcd.createChar(6, hungry);
  }else if (fullness == 2){
    lcd.createChar(6, peckish);
  }else if (fullness==3){
    lcd.createChar(6, full);
  }else{
    lcd.createChar(6, overfull);
  }
}

void createCharDevelopment(int development){
  if (development == 0){
    lcd.createChar(7, egg);
  } else if (development == 1){
    lcd.createChar(7, young);
  }else{
    lcd.createChar(7, adult);
  }
}


void feedAnimation(){
  
  if (startAnimation + 100 < time){
    
    lcd.clear();
    lcd.setCursor(c,1);
    lcd.write(7);//pet character
    c+=1;
    startAnimation = time;
  }

  if (c==9){//when it reaches the column 9 stop animation
    lcd.clear();
    lcd.setCursor(5,1);
    lcd.write(7);
    c=3;
    createCharFullness(myPet.fullness);
    FEEDstate=1;
    lcd.clear();
    lcd.setBacklight(WHITE);
    
  }
}

void createCharGame(){
  //creating the characters for the football mini game
  lcd.createChar(1,goalUp);
  lcd.createChar(2,goalDown);
  lcd.createChar(3,goalUp);
  lcd.createChar(4, defender);
  lcd.createChar(5, ball);
}

void writeChars(){
    //writing characters for mini game
    lcd.setCursor(15,0);
    lcd.write(1);
    lcd.setCursor(15,1);
    lcd.write(2);
    lcd.setCursor(6,1);
    lcd.write(5);
    lcd.setCursor(14, defRow);
    lcd.write(4);
    lcd.setCursor(c,1);
    lcd.write(7);
}

void defenderAnimation(){
  //moves the defender up and down every second
  if (defenderTime + 1000 <time){
    if (defRow == 1) defRow =0;
    else defRow =1;
    defenderTime = time;
    lcd.clear();
    writeChars();
  }
}

void playAnimation(){
  //plays the ball animation towards the goal
  if (startAnimationPlay + 200 < time){
    lcd.clear();
    lcd.setCursor(ballColumn,1);
    lcd.write(5);
    writeChars();
    ballColumn+=1;
    startAnimationPlay = time;
  } 
}

void setButtonsFalse(){
    //sets all buttons to false
    clickedRIGHT=false;
    clickedLEFT=false;
    clickedSELECT=false;
    clickedDOWN=false;
    clickedUP=false;
}
