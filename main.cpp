#include "mbed.h"
#include "TextLCD.h"
#include "DisplayBase.h"
// the pins for the LCD
TextLCD lcd(D3, D4, D5, D6, D7, D9); // RS, E, D4, D5, D6, D7

AnalogIn lm35_sensor(A0); // Connects LM35 OUT to A0

float temp_voltage; // Stores LM35 voltage   
float temperature; // temperature stored in Celsius
int temp_int, temp_frac; // Stores the integer and fractional parts of the temperature for LCD 

// Defines pins for buttons and LEDs with internal pull-up resistors
DigitalIn button1(D10, PullUp); // Button 1 RED
DigitalIn button2(D12, PullUp); // Button 2 GREEN
DigitalIn button3(D14, PullUp); // Button 3 BLUE

DigitalOut led1(D11); // LED for button 1
DigitalOut led2(D13); // LED for button 2
DigitalOut led3(D15); // LED for button 3

BufferedSerial dfplayer(D8, D2, 9600); //assigns TX, RX, Baud rate to df player
// Function created to send a command to DFPlayer Mini
void DFPlayer_SendCommand(uint8_t command, uint8_t param1, uint8_t param2) {
    uint16_t checksum=0xFFFF-(0xFF+0x06+command+0x00+param1+param2)+1;
     // Calculates checksum value by subtracting the sum of fixed bytes and parameters from 0xFFFF and then adding 1

    uint8_t buffer[10]={ //UART for DF Player
        0x7e,// start byte
        0xFF,//version
        0x06,//the number of bytes after 'LEN'
        command,//commands
        0x00,//command feedback
        param1,//parameter 1
        param2,//parameter 2
        static_cast<uint8_t>((checksum>>8) & 0xFF),//checksum casts to integer
        static_cast<uint8_t>(checksum & 0xFF),//checksum casts to integer
        0xEF//end byte
    };
    

    // Sends data via BufferedSerial
    dfplayer.write(buffer, sizeof(buffer)); // Send 7 bytes
}

// Function used to play a song
void DFPlayer_Play(uint8_t song_num) {
    DFPlayer_SendCommand(0x03, 0x00, song_num); // Plays song by number
}

// Function created to stop playing
void DFPlayer_Stop() {
    DFPlayer_SendCommand(0x16, 0x00, 0x00); // Stops playback
}
// All menu states 
enum MenuState { MAIN_MENU, HOTDOG_MENU, DRINKS_MENU, CONFIRM_ORDER };
MenuState currentMenu = MAIN_MENU;

int hotdog_count = 0; // Hotdog counter
int drink_count = 0;  // Drink counter

const float hotdog_price = 5.00; // Price per hotdog
const float drink_price = 2.00;  // Price per drink 

const int MAX_ITEMS = 9; // Max limit for items

Timer inactivityTimer; // Timer used to track inactivity 
const int inactivityTimeout = 20; // inactivity causes timer (seconds) to start 

void displayMainMenu() {
    lcd.cls(); // the LCD is cleared
    lcd.locate(0, 0); // goes to the first coloumn and first row of LCD
    lcd.printf("Hello! T=");
    temp_voltage = lm35_sensor.read() * 5.0f; // Converts  analog input value to voltage (since VCC = 5V)
    // As LM35 outputs 10 mV per degree Celsius, so we can multiply by 100 to get temperature in Celsius
    temperature = (temp_voltage * 100); 

    // Separates the integer and fractional value of temperature for LCD display
    temp_int = (int)temperature; // Integer value
    temp_frac = (int)((temperature - temp_int) * 100); // Fractional value (2 decimal places)
    // Checks whether the inactivity timeout has been reached
    lcd.printf("%d.%02d%cC",temp_int,temp_frac,223);// prints temperature reading (deg Celcius)
    lcd.printf("        ");//stops overwriting on the lcd
    lcd.cls();
    lcd.locate(0, 1);// sets writing to the second Row on lcd
    lcd.printf("FOOD DRINK ORDER");//prints menu options
    lcd.cls(); 
}

void displayHotdogMenu() {
    lcd.cls(); 
    lcd.locate(0, 0);
    lcd.printf("Hotdog T=");
    temp_voltage = lm35_sensor.read() * 5.0f; // Convert the analog input to voltage (since VCC = 5V)
    // LM35 gives 10 mV per degree Celsius, so we multiply by 100 to get temperature in Celsius
    temperature = (temp_voltage * 100); 


    // Separates the integer and fractional value of temperature for LCD display
    temp_int = (int)temperature; // Integer value
    temp_frac = (int)((temperature - temp_int) * 100); // Fractional value (2 decimal places)
    // Checks whether the inactivity timeout has been reached
    lcd.printf("%d.%02d%cC",temp_int,temp_frac,223);
    lcd.printf("              ");
    lcd.cls();
    lcd.locate(0, 1);
    lcd.printf("1:+ 2:- 3:ADD %d", hotdog_count );
    lcd.printf("            ");
    lcd.cls(); // Clears the LCD
}

void displayDrinksMenu() {
    lcd.cls(); 
    lcd.locate(0, 0);
    lcd.printf("Drink T=");
    temp_voltage = lm35_sensor.read() * 5.0f; // Convert the analog input to voltage (since VCC = 5V)
    // LM35 gives 10 mV per degree Celsius, so we multiply by 100 to get temperature in Celsius
    temperature = (temp_voltage * 100); 


    // Separates the integer and fractional value of temperature for LCD display
    temp_int = (int)temperature; // Integer value
    temp_frac = (int)((temperature - temp_int) * 100); // Fractional value (2 decimal places)
    // Checks whether the inactivity timeout has been reached
    lcd.printf("%d.%02d%cC",temp_int,temp_frac,223);
    lcd.printf("              ");
    lcd.cls();
    lcd.locate(0, 1);
    lcd.printf("1:+ 2:- 3:ADD %d ", drink_count );
    lcd.printf("         ");
    lcd.cls(); 
}

void displayConfirmOrder() {
    lcd.cls(); // Clear the LCD at the start of the confirm screen
    lcd.locate(0, 0);
    lcd.printf("Confirm T=");
    temp_voltage = lm35_sensor.read() * 5.0f; // Convert the analog input to voltage (since VCC = 5V)
    // LM35 gives 10 mV per degree Celsius, so we multiply by 100 to get temperature in Celsius
    temperature = (temp_voltage * 100); 


    // Separates the integer and fractional value of temperature for LCD display
    temp_int = (int)temperature; // Integer value
    temp_frac = (int)((temperature - temp_int) * 100); // Fractional value (2 decimal places)
    // Checks whether the inactivity timeout has been reached
    lcd.printf("%d.%02d%cC",temp_int,temp_frac,223);
    

    lcd.printf("              ");
    lcd.cls();
    lcd.locate(0, 1);
    lcd.printf("HD:%d", hotdog_count );
    lcd.printf(" D:%d", drink_count );
    int total_price = (hotdog_count * hotdog_price) + (drink_count * drink_price);
    
    //lcd.printf(" $%d", total_price ); // Display total price
    lcd.printf(" P=$"); 
    lcd.printf("%d",total_price); // write the total price }
    lcd.printf("   ");
    lcd.cls(); // Clears the LCD
    
  
   
    
}

int main() {


    dfplayer.set_format(8, SerialBase::None, 1);//format df player

    // Initializes the LCD
    lcd.cls(); 
    ThisThread::sleep_for(2000ms); // Wait for 2 seconds
  
    inactivityTimer.start(); // Start the inactivity timer
    DFPlayer_Play(1);
    while (1) {
        
        if (inactivityTimer.read() > inactivityTimeout) {
            hotdog_count=0;
            drink_count=0;
            currentMenu = MAIN_MENU; // Go to Main Menu after timeout
            inactivityTimer.reset(); // Resets the timer
            DFPlayer_Stop();
            ThisThread::sleep_for(200ms);
            DFPlayer_Play(1);
        }

        switch (currentMenu) {
            case MAIN_MENU:
                lcd.cls();
                displayMainMenu();
                // Main menu logic
                if (!button1) { // Button 1 - Go to Hotdog Menu
                    DFPlayer_Stop();
                    ThisThread::sleep_for(200ms);
                    DFPlayer_Play(2);
                    led1=1;
                    currentMenu = HOTDOG_MENU;
                    inactivityTimer.reset(); // Reset inactivity timer on button press
                    ThisThread::sleep_for(200ms); // Debounce
                    led1=0;
                } else if (!button2) { // Button 2 - Go to Drinks Menu
                    DFPlayer_Stop();
                    ThisThread::sleep_for(200ms);
                    DFPlayer_Play(3);
                    led2=1;
                    currentMenu = DRINKS_MENU;
                    inactivityTimer.reset(); // Reset inactivity timer on button press
                    ThisThread::sleep_for(200ms); // Debounce
                    led2 = 0;
                } else if (!button3) { // Button 3 - Go to Confirm Order
                    DFPlayer_Stop();
                    ThisThread::sleep_for(200ms);
                    DFPlayer_Play(4);
                    led3 = 1;//turns led 3 on
                    currentMenu = CONFIRM_ORDER;
                    inactivityTimer.reset(); // Reset inactivity timer on button press
                    ThisThread::sleep_for(200ms); // Debounce
                    led3 = 0;//Turns led 3 off
                }
                break;

            case HOTDOG_MENU:
                displayHotdogMenu();
                // Hotdog menu logic
                if (!button1) { // Button 1 - Adds 1 to hotdog count
                    if (hotdog_count < MAX_ITEMS) {
                        
                    
                        
                        led1 = 1; // Turns led 1 on
                        hotdog_count++;
                    } else {
                        lcd.cls();
                        lcd.locate(0, 0);
                        lcd.printf("Max ");
                        lcd.printf("T=");
                        temp_voltage = lm35_sensor.read() * 5.0f; // Convert the analog input to voltage (since VCC = 5V)
                        // LM35 gives 10 mV per degree Celsius, so we multiply by 100 to get temperature in Celsius
                        temperature = (temp_voltage * 100); 

           
                        // Separates the integer and fractional value of temperature for LCD display
                        temp_int = (int)temperature; // Integer value
                        temp_frac = (int)((temperature - temp_int) * 100); // Fractional value (2 decimal places)
                        // Checks whether the inactivity timeout has been reached
                        lcd.printf("%d.%02d%cC",temp_int,temp_frac,223);
                        lcd.printf("        ");
                        ThisThread::sleep_for(1000ms); // Show message for 1 second
                    }
                    inactivityTimer.reset(); // Reset inactivity timer on button press
                    ThisThread::sleep_for(200ms); // Debounce
                } else {
                    led1 = 0; // Turn off led1
                }
                
                if (!button2) { // Button 2 - Subtract 1 from hotdog count
                    if (hotdog_count > 0) {
                        hotdog_count--;
                    }
                    
                    led2 = 1; // Turn on LED2
                    inactivityTimer.reset(); // Reset inactivity timer on button press
                    ThisThread::sleep_for(200ms); // Debounce
                } else {
                    led2 = 0; // Turn off led2
                }

                if (!button3) { // Button 3 - Add to order and return to main menu
                    
                    led3 = 1;  // turns led 3 on
                    currentMenu = MAIN_MENU;
                    inactivityTimer.reset(); // inactivity timer resets on button press
                    ThisThread::sleep_for(200ms); // Debounce
                    DFPlayer_Stop();
                    ThisThread::sleep_for(200ms);
                    DFPlayer_Play(1);
                    
                }
                
                    
                led3 = 0;  // Turns led 3 off
                break;

            case DRINKS_MENU:
                displayDrinksMenu();// Drinks menu 
                if (!button1) { // Button 1 - Adds 1 to drink count
                    if (drink_count < MAX_ITEMS) {
                        drink_count++;
                    } else {
                        lcd.cls();
                        lcd.locate(0, 0);
                        lcd.printf("Max");
                        lcd.printf(" T=");
                        temp_voltage = lm35_sensor.read() * 5.0f; // Convert the analog input to voltage (since VCC = 5V)
                        // LM35 gives 10 mV per degree Celsius, so we multiply by 100 to get temperature in Celsius
                        temperature = (temp_voltage * 100); 

                        
                        // Separates the integer and fractional value of temperature for LCD display
                        temp_int = (int)temperature; // Integer value
                        temp_frac = (int)((temperature - temp_int) * 100); // Fractional value (2 decimal places)
                        // Checks whether the inactivity timeout has been reached
                        lcd.printf("%d.%02d%cC",temp_int,temp_frac,223);
                        lcd.printf("        ");
                        ThisThread::sleep_for(1000ms); //  Displays message for 1 second
                    }
                    
                    led1 = 1; // Turns led1 on
                    inactivityTimer.reset(); // Reset inactivity timer on button press
                    ThisThread::sleep_for(200ms); // Debounce
                } else {
                    led1 = 0; // Turn off LED1
                }

                if (!button2) { // Button 2 - Subtract 1 from drink count
                    if (drink_count > 0) {
                        drink_count--;
                    }
                    
                    led2 = 1; // Turns led2 on
                    inactivityTimer.reset(); // Reset inactivity timer using button 
                    ThisThread::sleep_for(200ms); // delay for 0.2 seconds
                } else {
                    led2 = 0; // Turns led 2 off
                }

                if (!button3) { // Button 3 - Add to order and return to main menu
                   
                    led3 = 1; // Turn on LED3
                    currentMenu = MAIN_MENU;
                    inactivityTimer.reset(); // Reset inactivity timer using button 
                    ThisThread::sleep_for(200ms);
                    DFPlayer_Stop();
                    ThisThread::sleep_for(200ms);
                    DFPlayer_Play(1);
                }
                
                    
                led3 = 0; // Turns led 3 off
                break;

            case CONFIRM_ORDER:
                
                displayConfirmOrder();// order food or drink function
                // Confirm order logic
                if (!button1) { // Button 1 - Confirm order
                    led1 = 1;
                    lcd.cls(); // Clear screen after confirmation
                    lcd.locate(0, 0);
                    lcd.printf("Confirmed! T=");
                    temp_voltage = lm35_sensor.read() * 5.0f; // Convert the analog input to voltage (since VCC = 5V)
                     // LM35 gives 10 mV per degree Celsius, so we multiply by 100 to get temperature in Celsius
                    temperature = (temp_voltage * 100); 

              
                    // Separates the integer and fractional value of temperature for LCD display
                    temp_int = (int)temperature; // Integer value
                    temp_frac = (int)((temperature - temp_int) * 100); // Fractional value (2 decimal places)
                    // Checks whether the inactivity timeout has been reached
                    lcd.printf("%d.%02d%cC",temp_int,temp_frac,223);
                    ThisThread::sleep_for(2000ms); 
                    led1 = 0;
                    hotdog_count=0; 
                    drink_count=0;
                    currentMenu = MAIN_MENU; // Return to main menu after confirmation
                    inactivityTimer.reset(); // Reset inactivity timer
                    DFPlayer_Stop();// turns music player off
                    ThisThread::sleep_for(200ms);// 0.2 second delay
                    DFPlayer_Play(1);//turns the music on
                       
                } else if (!button2) { // Button 2 - Returns to main menu
                    led2 = 1;// turns led 2 on
                    currentMenu = MAIN_MENU;// returns to main menu
                    inactivityTimer.reset(); // Reset inactivity timer
                    ThisThread::sleep_for(200ms); // Debounce
                    led2 = 0;//turns led2 off
                    DFPlayer_Stop();// turns music
                    ThisThread::sleep_for(200ms);// delay of 0.2 seconds
                    DFPlayer_Play(1);// goes back to first song
                                

                
                } else if (!button3) { // Button 2 - Return to main menu
                    led3 = 1; // turns led3 one
                    currentMenu = MAIN_MENU;//rests backs to main menu
                    inactivityTimer.reset(); // Reset back to inactivity timer
                    ThisThread::sleep_for(200ms); // waits for 0.2 seconds
                    led3 = 0; // turns led3 off
                    DFPlayer_Stop();// stops music
                    ThisThread::sleep_for(200ms);// delay
                    DFPlayer_Play(1);// goes back to first song
                                

                }
       

                break;
        }


    }
}
