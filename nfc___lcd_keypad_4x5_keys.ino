/*Author  : Bijendra Bishow Maskey
 * Date   : 04.04.2016
 * modified date: 23.06.2016
 * Title  : monopoly and nfc
 */

/*LCD circuit*/
/*
 * LCD RS = A0 / 14
 * LCD En = A1 / 15
 * 
 * LCD D4 = A2 / 16
 * LCD D5 = A3 / 17
 * LCD D6 = D4 / 1
 * LCD D7 = D0 / 0
 * LCD R/W ; VSS = GND
 * 
 */
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <math.h>
#include <stdlib.h>
#include "pitches.h"

#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

#define SALARY 2000000
#define DECIMAL_DIGIT_LIMIT 4
#define INTEGER_DIGIT_LIMIT 3
#define MELODYPIN 4

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);         //declare variable of type Adafruit_PN532
#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
// also change #define in Adafruit_PN532.cpp library file
   #define Serial SerialUSB
#endif

LiquidCrystal lcd(14, 15, 16, 17, 1, 0); // LCD pins A0,A1,  A2,A3,A4,A5
const byte ROWS = 5;    //five rows
const byte COLS = 4;    //four columns
char keys[ROWS][COLS] = {
  {'M','S','K','X'},    //X & Y are dummy characters that can be used later as per requirement
  {'7','8','9','+'},
  {'4','5','6','T'},
  {'1','2','3','-'},
  {'C','0','.','Y'},
};                      //two dimesional array to store character equivalent of the keys
byte rowPins[ROWS] = {5, 6, 7, 8, 9};         //connect to the row pinouts of the keypad
byte colPins[COLS] = {10, 11, 12, 13};        //connect to the column pinouts of the keypad
//double total_actual_money = 0;
uint8_t state = 0;                            //different states allows user to enter only defined characters
uint32_t money_integer = 0;                   //integer part of number from pressing of keys
uint32_t money_decimal = 0;                   //decimal part of number from pressing of keys
uint8_t decimal_count = 0;                    //to indicate how many decimal digit entered by user
uint8_t integer_count = 0;                    //to indicate how many integer digit entered by user
uint32_t actual_money = 0;                    //money converted from million/thousands won to actual won
//uint32_t total_money = 0;                     //total money of users up to now.
//uint8_t total_actual_money_user_ptr = 0;
uint8_t end_loop_flag = 0;                    //to indicate to forcefully get out from end loop, 0=> stay in loop, 1 => get out from loop.

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


//int ledPin =13;
/*##########################################################################
monopoly funcitons:

##########################################################################*/
typedef struct{
  uint8_t uid;
  uint32_t balance;
}INFORMATION;

INFORMATION user[8];

/*
 * LCD and KEYS variables
 */

void user_information_init(INFORMATION *user, uint8_t uid)
{
  user->uid = uid;                //as per database storage order give uid.
  user->balance = 15000000;       //initially load 15million won as balance
}

void display_balance_of_all_users()
{
  Serial.print(" user_1 balance = "); Serial.print(user[0].balance, DEC); Serial.println("won");
  Serial.print(" user_2 balance = "); Serial.print(user[1].balance, DEC); Serial.println("won");
  Serial.print(" user_3 balance = "); Serial.print(user[2].balance, DEC); Serial.println("won");
  Serial.print(" user_4 balance = "); Serial.print(user[3].balance, DEC); Serial.println("won");
  Serial.print(" user_5 balance = "); Serial.print(user[4].balance, DEC); Serial.println("won");
  Serial.print(" user_6 balance = "); Serial.print(user[5].balance, DEC); Serial.println("won");
  Serial.print(" user_7 balance = "); Serial.print(user[6].balance, DEC); Serial.println("won");
  Serial.print(" user_8 balance = "); Serial.print(user[7].balance, DEC); Serial.println("won");

}
/*###########################################################################################################
monopoly funcitons end
###########################################################################################################*/

void setup(void) {

//  pinMode(ledPin, OUTPUT);
//  Serial.begin(115200);                                                 //serial communication to display on monitor begin
  lcd.begin(16, 2);                     // start lcd 16X2 type
  Serial.println("\n\nCreated by: Bijendra Bishow Maskey");               // pin 0 (RX) has been used for LCD due to lack of pins so lcd or serial output only one can be used at a time
  Serial.println("Program started......");
  nfc.begin();
//  uint32_t versiondata = nfc.getFirmwareVersion();
//  if (! versiondata) {
//    Serial.print("_PN532_ chip not found");
//    while (1);      // halt/stop execution
//  }
//  
//  // Got PN532 chip firmware version data and display it
//  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
//  Serial.print("Firmware version. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
//  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  // configure board to read RFID tags
  nfc.SAMConfig();
  Serial.println("Waiting for an ISO14443A Card(nfc tag) ...");
  for(int i = 0; i<8; i++)
  {
    user_information_init(&user[i], i+1);
  }
//  display_balance_of_all_users();
  lcd.setCursor(1,0);
  lcd.print("Monopoly Game");
  lcd.setCursor(2,1);
  lcd.print("Printed IC lab");
  melody_mario();
}
/**
  *@ param  ->  block_no = block entered by the user
  *@ return ->  0 if the block_no is a sector trailer
                1 if the block_no isn't the sector trailer
*/
bool PN532_trailer_block_check(uint8_t block_no)
{
  if((block_no+1)%4 == 0){
    return 0;
  }
  else{
    return 1;
  }
}

void PN532_print_card_id(uint8_t uid_flag)
{
  switch(uid_flag){
      case 1:
        Serial.println("<user name: mifare classic 1k card>");
        break;
      case 2:
        Serial.println("<user name: student id card of Bijendra Maskey>");
        break;
       case 3:
        Serial.println("<user name: POP card of Bijendra Maskey>");
        break;
        case 4:
        Serial.println("<user name: student id card of Yu Hwa kyeong>");
        break;
      default:
        Serial.println("<user name: unrecognized This Mifare nfc tag is not in directory>");
    }
}

uint8_t PN532_compare_uid_database(uint8_t *uid)
{
  uint8_t uid_database[8][4] = {
    {0xE1, 0xCF, 0xF0, 0xCF},             //card 1 = 0000
    {0xB2, 0xF0, 0x55, 0x93},             //card 2 = 0001
    {0x82, 0x8A, 0x58, 0x93},             //card 3 = 0010
    {0xE2, 0xCB, 0x85, 0x70},             //card 4 = 0011
    {0xB2, 0x10, 0x4E, 0x93},             //card 5 = 0101
    {0xF2, 0x40, 0x56, 0x93},             //card 6 = 0110
    {0xF2, 0xFB, 0x60, 0x93},             //card 7 = 0111    
    {0x02, 0x1B, 0x4C, 0x93}              //card 8 = 1000    
  };
  uint8_t uid_flag = 0;
  
  for(int i=0; i<8; i++)
    {
      if(!memcmp(uid, uid_database[i], 4))
      {
        uid_flag = i+1;
        i = 8;
      }
    }
  return uid_flag;
}

/*
#####################################################################################################################
                                                    LCD FUNCTIONS BEGIN
#####################################################################################################################
*/
/*
 * function: display the amount in million or thousands unit
 */
void display_amount_lcd(float amount)
{
//  char buff[7];
  lcd_clear_line(0);
  lcd.setCursor(0,0);
  lcd.print("total");
  lcd.setCursor(6,0);
  if(amount >= 1000000)
  {
    amount = amount/1000000.0;
    lcd.print(amount,4);
//    sprintf(buff,"%7f", amount);      //for right alignment but showing error 
//    lcd.print(buff);
    lcd.setCursor(15,0);
    lcd.print('M');
  }
  else if(amount >= 1000)
  {
    amount = amount/1000.0;
    lcd.print(amount,4);
//    sprintf(buff,"%7f", amount);    //for right alignment but showing error
//    lcd.print(buff);
    lcd.setCursor(15,0);
    lcd.print('K');
  }
  else
  {
  }
}
/*
 * function: clear the the numebers and other keys entered along with the count
 */
 void keys_clear(uint32_t *integer_part, uint32_t *decimal_part,uint32_t *actual_money)
 {
  *integer_part = 0;                                                         //clear integer part to 0 for next run
  *decimal_part = 0;                                                         //clear decimal part to 0 for next run
  *actual_money = 0;                                                         //amount added and converted to real/actual money is cleared to 0 for next run
  integer_count = 0;                                                         //clear integer counter to 0 for next run
  decimal_count = 0;                                                         //clear decmial counter to 0 for next run
 }
/*
 * if wrong key pressed beep sound or red led or others
 * indiacating error sign
 */
void keys_wrongkey()
{
  //indicate error eg turn off green light and show red light
  lcd_clear_line(0);
  lcd.setCursor(4,0);
  lcd.print("wrong key");
  melody_wrong_key();
}

/*
 * clear 1 line of lcd screen by printing blank space
 */
void lcd_clear_line(uint8_t row_no)
{
  lcd.setCursor(0,row_no);
  for(uint8_t i =0; i<16; i++)
  {
    lcd.print(' ');
  }
}
/*
 * right justify uint32_t value 
 * converts into string and fromat it
 * @param: value: right justifies it and prints it out
 *         digit_length:  reserves 'digit_length' space and prints it with space padding in fornt
 *                        eg: for digit_length = 4 it prints '23' as '  23'
 */
void lcd_right_justify(uint32_t value, uint8_t digit_length)
{
  char buff[digit_length];
  sprintf(buff,"%3d", value);
  lcd.print(buff);
}

/*##############################################################################
 * function:  add 2million won to the variable entered
 * param   :  double* type 
 */
void keys_salary_increase(uint32_t *money)
{
  *money = *money + SALARY;
  lcd.setCursor(0,1);
  lcd.print('+');
  lcd.setCursor(5,1);
  lcd.print((SALARY/1000000.0),4);
  lcd.setCursor(14,1);
  lcd.print(" M");
}

/*###############################################################################
 * function : convert incoming character from keys one by one into integer form
 */
void keys_numbers(uint32_t *money, char *key_value)
{
  *money = *money*10 + (*key_value - '0');
}

/*
 * function: multiply by 1M
 */
void key_million_times(uint32_t *integer_part,uint32_t *decimal_part, uint32_t *actual_money)
{
  float temp_money = 0;
  if((*integer_part != 0)||(*decimal_part != 0))
  {
   temp_money = *integer_part + *decimal_part/(pow(10.0,decimal_count));         //integrate integer and decimal part to temp_money for decimal part divide by 10^no of decimal digits entered
   *integer_part = 0;                                                         //clear integer part to 0 for next run
   *decimal_part = 0;                                                         //clear decimal part to 0 for next run
   integer_count = 0;                                                         //clear integer counter to 0 for next run
   decimal_count = 0;                                                         //clear decmial counter to 0 for next run
   lcd.clear();
   lcd.setCursor(5,1);
   lcd.print(temp_money,4);
   lcd.setCursor(14,1);
   lcd.print(" M");
   *actual_money = temp_money*1000000;
/*   displays amount in won    */
//   lcd.setCursor(1,0);                  
//   lcd.print(*actual_money);
//   lcd.setCursor(13,0);
//   lcd.print("Won");
  }
}

/*
 * function: multiply by 1K
 */
void key_thousand_times(uint32_t *integer_part,uint32_t *decimal_part, uint32_t *actual_money)
{
  float temp_money = 0;
  if((*integer_part != 0)||(*decimal_part != 0))
  {
   temp_money = *integer_part + *decimal_part/(pow(10,decimal_count));         //integrate integer and decimal part to actual_money for decimal part divide by 10^no of decimal digits entered
   *integer_part = 0;                                                         //clear integer part to 0 for next run
   *decimal_part = 0;                                                         //clear decimal part to 0 for next run
   integer_count = 0;                                                         //clear integer counter to 0 for next run
   decimal_count = 0;                                                         //clear decmial counter to 0 for next run
   lcd.clear();
   lcd.setCursor(5,1);
   lcd.print(temp_money,4);
   lcd.setCursor(14,1);
   lcd.print(" K");
   *actual_money = temp_money*1000;
  }
}

/*
 *function: adds the just now entered amount to total actual money 
 */
void keys_add(uint32_t *actual_money, uint32_t *total_money)
{
  lcd.setCursor(0,1);
  lcd.print('+');
  *total_money = *total_money + *actual_money;
  display_amount_lcd(*total_money);
}

/*
 *function: subtracts the just now entered amount from total actual money 
 */
void keys_subtract(uint32_t *actual_money, uint32_t *total_money)
{
  lcd.setCursor(0,1);
  lcd.print('-');
  if(*total_money > *actual_money)
  {
    *total_money = *total_money - *actual_money;
  }
  else
  {
    delay(200);             //this delay is so that -amount can be seen on LCD before the insufficient notice appears
    lcd_clear_line(1);
    lcd.setCursor(0,1);
    lcd.print("insufficient bal");
    //error
  }  
  display_amount_lcd(*total_money);
}

/*
 *function: transfers the just now entered amount from one user to another
 */
void keys_transfer(uint32_t *actual_money, uint32_t *total_money)
{
  lcd.setCursor(0,1);
  lcd.print("<-");
  *total_money = *total_money - *actual_money;
//  lcd.setCursor(1,0);
//  lcd.print(*total_money);
//  lcd.setCursor(13,0);
//  lcd.print("Won");
  display_amount_lcd(*total_money);
}
/*
#####################################################################################################################
                                                    LCD FUNCTIONS END
##################################################################################################################### 
*/

/*
#####################################################################################################################
                                                    SWITCH CASES BEGIN
##################################################################################################################### 
*/
void switch_states(uint8_t *monopoly_user_no, uint8_t *switch_state)
{
//  uint8_t state = *switch_state;
  uint8_t user_ptr = *monopoly_user_no - 1;
  char key_value = keypad.getKey();
  if (key_value != NO_KEY)
  {
    switch(state)
    {
      case 0:                                            // 'S'-> raise 2M; '0'...'9' ; '.' only accepted
       if(key_value == 'S')
       {
        lcd.clear();
        keys_salary_increase(&user[user_ptr].balance);       //if key_value = 'S'
        display_amount_lcd(user[user_ptr].balance);           //display money in K or M form.
        melody_finish();
       }

       else if((key_value == '0')||(key_value == '1')||(key_value == '2')||(key_value == '3')||(key_value == '4')||(key_value == '5')||(key_value == '6')||(key_value == '7')||(key_value == '8')||(key_value == '9'))
       {
        lcd.clear();
        if(integer_count < INTEGER_DIGIT_LIMIT)
        {
          keys_numbers(&money_integer, &key_value);
          if(key_value != '0'){                     //prevent from incrementing of integer_count when first cosecutive 
            integer_count++;                        //increment it each time integer key is hit before '.'
            state = 1;
          }
          lcd.setCursor(5,1);
          lcd_right_justify(money_integer, INTEGER_DIGIT_LIMIT);
//          lcd.print(money_integer);
        }
        else{
          lcd.clear();
          lcd_clear_line(0);
          //keys_wrongkey();
         lcd.setCursor(6,0);
         lcd.print("digit limit");
        }
       }

       else if(key_value == '.')
       {
        lcd.clear();
        state = 2;
        lcd.setCursor(8,1);
        lcd.print('.');
       }

       else
       {
        lcd.clear();
        //error / wrong keys pressed
        keys_wrongkey();
       }
      break;

      case 1:
        if((key_value == '0')||(key_value == '1')||(key_value == '2')||(key_value == '3')||(key_value == '4')||(key_value == '5')||(key_value == '6')||(key_value == '7')||(key_value == '8')||(key_value == '9'))
       {
        if(integer_count < INTEGER_DIGIT_LIMIT)
        {
          keys_numbers(&money_integer, &key_value);
          state = 1;
          integer_count++;
          lcd.setCursor(5,1);
          lcd_right_justify(money_integer, INTEGER_DIGIT_LIMIT);
//          lcd.print(money_integer);
        }
        else{
         lcd.setCursor(6,0);
         lcd.print("digit limit");
         melody_wrong_key();
        } 
       }

       else if(key_value == '.')
       {
        state = 2;
        lcd.setCursor(8,1);
        lcd.print('.');
       }

       else if(key_value == 'M')
       {
        key_million_times(&money_integer,&money_decimal,&actual_money);        //decimal integer = 0 as not pressed yet
        state = 3;
       }

       else if(key_value == 'K')
       {
        key_thousand_times(&money_integer,&money_decimal,&actual_money);      //decimal integer = 0 as not pressed yet
        state = 3;
       }
       else if(key_value == 'C')
       {
        keys_clear(&money_integer,&money_decimal,&actual_money);    //reset all the integers,decimal and amount
        state = 0;
        lcd.clear();
        lcd.setCursor(4,0);
        lcd.print("cleared");
       }

       else
       {
        //error / wrong keys pressed
        keys_wrongkey();
       }

      break;

      case 2:                                      //after decimal '.' has been pressed
       if((key_value == '0')||(key_value == '1')||(key_value == '2')||(key_value == '3')||(key_value == '4')||(key_value == '5')||(key_value == '6')||(key_value == '7')||(key_value == '8')||(key_value == '9'))
       {
        if(decimal_count < DECIMAL_DIGIT_LIMIT)
        {
          keys_numbers(&money_decimal, &key_value);
          decimal_count++;
          lcd.setCursor(9,1);
          lcd.print(money_decimal);
        }
        else{
         lcd.setCursor(6,0);
         lcd.print("digit limit");
         melody_wrong_key();       
        }
       }
       
       else if(key_value == 'M')
       {
        key_million_times(&money_integer,&money_decimal,&actual_money);
        state = 3;
       }

       else if(key_value == 'K')
       {
        key_thousand_times(&money_integer,&money_decimal,&actual_money);
        state = 3;
       }
       
       else if(key_value == 'C')
       {
        keys_clear(&money_integer,&money_decimal,&actual_money);    //reset all the integers,decimal and amount
        state = 0;
        lcd.clear();
        lcd.setCursor(4,0);
        lcd.print("cleared");
       }

       else
       {
        //error / wrong keys pressed
        keys_wrongkey();
       }
      break;

      case 3:
       if(key_value == '+')
       {
        keys_add(&actual_money, &user[user_ptr].balance);
        state = 0;
        melody_finish();
       }
       
       else if(key_value == '-')
       {
        keys_subtract(&actual_money, &user[user_ptr].balance);
        state = 0;
        melody_finish2();
       }

       else if(key_value == 'T')
       {
        keys_subtract(&actual_money, &user[user_ptr].balance);
        *switch_state = 4;              //to execute the switch_state which is actually in main
        melody_finish2();
       }

       else if(key_value == 'C')
       {
        keys_clear(&money_integer,&money_decimal,&actual_money);    //reset all the integers,decimal and amount
        state = 0;
        lcd.clear();
        lcd.setCursor(4,0);
        lcd.print("cleared");
       }

       else
       {
        //error / wrong keys pressed
        keys_wrongkey();
       }
      break;
      
      
      case 4:
        if(key_value == 'C')
        {
          keys_clear(&money_integer,&money_decimal,&actual_money);    //reset all the integers,decimal and amount
          end_loop_flag = 1;
          state = 0;
          lcd.clear();
          lcd.setCursor(4,0);
          lcd.print("cleared");
        }
        else
        {
          keys_wrongkey();
        }
      break;
      
      case 5:
        if(key_value == '+')
        {
          keys_add(&actual_money, &user[user_ptr].balance);           //add transferred amount to new user
          end_loop_flag = 1;
          state = 0;
          keys_clear(&money_integer,&money_decimal,&actual_money);    //reset all the integers,decimal and amount
          melody_finish();
        }
        else if(key_value == 'C')
        {
          keys_clear(&money_integer,&money_decimal,&actual_money);    //reset all the integers,decimal and amount
          end_loop_flag = 1;
          state = 0;
          lcd.clear();
          lcd.setCursor(4,0);
          lcd.print("cleared");
        }
        else
        {
          keys_wrongkey();
        }    
      break;

      default:
       lcd_clear_line(0);
       lcd.setCursor(3,0);
       lcd.print("default");
    } 
  }
}

/*
#####################################################################################################################
                                                    SWITCH CASES END
##################################################################################################################### 
*/
/*
 * Function: readUID of nfc tag and return userno after comparing UID read with the database
 * parameters: 
 * return: user_no
 */
uint8_t PN532_find_user_no(uint8_t *uid, uint8_t *uidLength)
{
  uint8_t monopoly_user_no = 0;
  uint8_t success = 0;

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, uidLength);
  if(success)
  {
    monopoly_user_no = PN532_compare_uid_database(uid);         //compare the uid of nfc tag with the database and return the position with which it matches   (1,2,3......) like 1 for user 1 and so on
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, *uidLength);
    Serial.println("");
    PN532_print_card_id(monopoly_user_no);                             //display the information about whose card it is comparing with the uid_database 
    return monopoly_user_no ;
  }
  else
  {
    return 0;
  }
}

/*
#####################################################################################################################
                                                    MAIN FUNCTION
##################################################################################################################### 
*/
#if 0
void loop(void)
{
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t monopoly_user_no  = 1;            // user id/no of monopoly game
  uint8_t switch_state = 0;                 //its value defines which state to go in switch case, left at it is by default ot let the change occur inside switch case itself.
  
//  monopoly_user_no = PN532_find_user_no(uid, &uidLength);         //compare with database and get user no based on 4byte UID of nfc tag.
  if(monopoly_user_no)                                            //to check if the card is present, returns user_no else return 0
  {
    display_amount_lcd(user[monopoly_user_no].balance);                   //display money in K or M form.
    lcd.setCursor(0,1);
    lcd.print("User: ");
    lcd.print(monopoly_user_no);
    do
    {
      switch_states(&monopoly_user_no, &switch_state);
      /*This code part occurs only when 'T'(i.e. transfer key) is pressed*/
      if(switch_state == 4)
      {
        uint8_t temp_user_no = PN532_find_user_no(uid, &uidLength);         //get the current user no from the nfc tag in contact with reader
        while((temp_user_no == monopoly_user_no)||(temp_user_no == 0))      //stay in this loop untill new user shows the tag
        {
          state = 4;
          switch_states(&monopoly_user_no, &switch_state);                  //wait for 'C' to exit from while loop(goes to state 4) //make monopoly_user_no a ptr and change it to high value like 255 or 129 above no of mzxm users
          temp_user_no = PN532_find_user_no(uid, &uidLength);               //get the current user no from the nfc tag in contact with reader
        }
        //comes to this part if new user shows the tag
        if(monopoly_user_no != END_LOOP_FLAG)
        {
          state = 5;                                                        //to go to state 5 to add transferred amount to the new user.
          switch_states(&monopoly_user_no, &switch_state);                  //temp_user_no => new user no; waits for '+' key to add the transferred amount to new user.
        }  
      }
    }while(1);  //stay in do while loop unitll same user nfc tag is in contact with the reader. //END_LOOP
    state = 0;
    
    delay(2000);
  }
}

#endif

#if 1       // use this part when nfc tag can be interfaced i.e nfc reader is interfaced to arduino otherwise will stuck in loop till it detect the nfc tag

void loop(void)
{
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t monopoly_user_no  = 0;            // user id/no of monopoly game
  uint8_t switch_state = 0;                 //its value defines which state to go in switch case, left at it is by default ot let the change occur inside switch case itself.
  uint8_t temp_user_no  = 0;
 
  monopoly_user_no = PN532_find_user_no(uid, &uidLength);         //compare with database and get user no based on 4byte UID of nfc tag.
  if(monopoly_user_no)                                            //to check if the card is present, returns user_no else return 0
  {
    lcd.clear();
    display_amount_lcd(user[monopoly_user_no-1].balance);                     //display money in K or M form. as monopoly user no-1 = user ptr
    lcd.setCursor(0,1);
    lcd.print("User: ");
    lcd.print(monopoly_user_no);
    while((monopoly_user_no == PN532_find_user_no(uid, &uidLength))&&(end_loop_flag == 0))          //stay in do while loop unitll same user nfc tag is in contact with the reader. //END_LOOP
    {
      switch_states(&monopoly_user_no, &switch_state);
      
      /*This code part occurs only when 'T'(i.e. transfer key) is pressed*/
      if(switch_state == 4)
      {
        uint8_t temp_user_no = PN532_find_user_no(uid, &uidLength);         //get the current user no from the nfc tag in contact with reader
        while(((temp_user_no == monopoly_user_no)||(temp_user_no == 0))&&(end_loop_flag == 0))      //stay in this loop untill new user shows the tag
        {
//          lcd.setCursor(4,1);
//          lcd.print("state4 loop");
////          delay(1500);
          state = 4;
          switch_states(&monopoly_user_no, &switch_state);                  //wait for 'C' to exit from while loop(goes to state 4) //make monopoly_user_no a ptr and change it to high value like 255 or 129 above no of mzxm users
          temp_user_no = PN532_find_user_no(uid, &uidLength);               //get the current user no from the nfc tag in contact with reader
        }
//        lcd.setCursor(2,1);
//        lcd.print("state4 loopout");
//        delay(1500);
        //comes to this part if new user shows the tag

        if(end_loop_flag == 0)
        {
          state = 5;
          monopoly_user_no = temp_user_no;
          lcd.clear();
          display_amount_lcd(user[monopoly_user_no-1].balance);                     //display money in K or M form. as monopoly user no-1 = user ptr
          lcd.setCursor(0,1);
          lcd.print("User: ");
          lcd.print(monopoly_user_no);
          temp_user_no = PN532_find_user_no(uid, &uidLength);         //get the current user no from the nfc tag in contact with reader
          while(end_loop_flag == 0)                                           //stay in loop untill '+' or 'C' key is pressed
          {
  //          lcd.clear();
  //          lcd.setCursor(2,1);
  //          lcd.print("state5 loop");
  ////          delay(1500);
            switch_states(&monopoly_user_no, &switch_state);                  //wait for 'C' to exit from while loop(goes to state 4) //make monopoly_user_no a ptr and change it to high value like 255 or 129 above no of mzxm users
          }
  //        lcd.clear();
  //        lcd.setCursor(2,1);
  //        lcd.print("state5 loopout");
  //        delay(1500);
        }
      }
    }
    
    state = 0;                                                  //set back to default initial state 0 when another user tag is detected
    end_loop_flag = 0;                                          //to set back the end_loop_flag to default value for next run so that it wouldn't get out of loop 
    keys_clear(&money_integer,&money_decimal,&actual_money);    //reset all the integers,decimal and amount
    delay(250);
//    lcd.clear();
//    lcd.setCursor(5,0);
//    lcd.print("out of loop");
//    delay(1500);
  }
}
#endif


void melody_mario()
{
  int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
};

int tempo[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
};

 int size = sizeof(melody) / sizeof(int);
   for (int thisNote = 0; thisNote < size; thisNote++)
   {
 
     // to calculate the note duration, take one second
     // divided by the note type.
     //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
     int noteDuration = 1000 / tempo[thisNote];
 
     tone(MELODYPIN, melody[thisNote], noteDuration);
 
     // to distinguish the notes, set a minimum time between them.
     // the note's duration + 30% seems to work well:
     int pauseBetweenNotes = noteDuration * 1.30;
     delay(pauseBetweenNotes); 
     // stop the tone playing:
     tone(MELODYPIN, 0, noteDuration);
    }  
}

void melody_wrong_key()
{
  tone(MELODYPIN, NOTE_G3, 250);
  delay(250);
  // stop the tone playing:
  noTone(MELODYPIN);
}

void melody_right_key()
{
  tone(MELODYPIN, NOTE_GS5, 200);
  delay(200);
  // stop the tone playing:
  noTone(MELODYPIN);
}


void melody_finish()
{
  tone(MELODYPIN, NOTE_GS6, 100);
  delay(100);
  tone(MELODYPIN, NOTE_GS6, 100);
  delay(100);
  tone(MELODYPIN, NOTE_GS5, 150);
  delay(150);
  // stop the tone playing:
  noTone(MELODYPIN);
}

void melody_finish2()
{
  tone(MELODYPIN, NOTE_DS6, 100);
  delay(100);
  tone(MELODYPIN, NOTE_DS6, 100);
  delay(100);
  tone(MELODYPIN, NOTE_DS5, 150);
  delay(150);
  // stop the tone playing:
  noTone(MELODYPIN);
}



/*
#################################
for debugging lcd display format
#################################

  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("out of loop");
#################################
*/
