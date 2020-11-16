/*
   mod13.12 - do NOT upgrade !!! library MFRC522 version 1.4.3 - 1.4.5 and 1.4.6 have problems to read some RFID chips
   mod13.11 - https://github.com/arduino/Arduino/issues/6387 - modifications to remove COM port support

              diff --git a/hardware/arduino/avr/cores/arduino/USBCore.cpp b/hardware/arduino/avr/cores/arduino/USBCore.cpp
              index e00fb028e..efbee7be5 100644
              --- a/hardware/arduino/avr/cores/arduino/USBCore.cpp
              +++ b/hardware/arduino/avr/cores/arduino/USBCore.cpp
              @@ -376,8 +376,8 @@ bool ClassInterfaceRequest(USBSetup& setup)
               {
                      u8 i = setup.wIndex;
               
              -       if (CDC_ACM_INTERFACE == i)
              -               return CDC_Setup(setup);
              +       //if (CDC_ACM_INTERFACE == iv
              +       //      return CDC_Setup(setup);
               
               #ifdef PLUGGABLE_USB_ENABLED
                      return PluggableUSB().setup(setup);
              @@ -467,7 +467,7 @@ static u8 SendInterfaces()
               {
                      u8 interfaces = 0;
               
              -       CDC_GetInterface(&interfaces);
              +       //CDC_GetInterface(&interfaces);
               
               #ifdef PLUGGABLE_USB_ENABLED
                      PluggableUSB().getInterface(&interfaces);
   
   mod12.10 - center some big letters output
              - to get place for additional calls code was cleaned from obsolete calls

   mod11.09 - font smoothed using gray colors to cover steps in round parts

   mod10.08 - drawPixel has to be used to display pixels
              fastPixel/fastPixel2 insert some additional pixels as it seems in random locations
              drawPixel has some more calls of >>> spiWait17(); <<<

   mod09.07 - AES encryption is not symetric; an additional parameter is used to specify if encrxption or decryption
              has to be performed
   mod08.07 - grey pixel are used to smooth the fonts; both type of fonts can be used, simple and smoothed
   mod07.06 - delete a pixel output on display shown for unknown reason
   mod06.06 - delay after each char while "printing" PWD - mobile phone
   mod05.06 - extensions for paging while selecting a PWD - hold button to quick next page
   mod04.05 - some modifications to accelerate display of visible PWDs to select from
   mod04.04 - it seems that PetitFS do not return '.' (dot) of hidden files therefore filtering of '._' from mac
              must be done by checking for '_' (underline) character
   mod04.03 - special handling for >>>@<<< - using Keyboard.press to imitate entry with more than one key pressed
              Alt Gr + Q for german keyboard
   mod04.02 - skip hidden "." and some system files "._" (mac)
   mod04.01 - apparently the mount call has to retry as often until 2 consecutive responses are same

  == below the commands to download the build:=====================================================================

    /home/me/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino14/bin/avrdude -C/home/me/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino14/etc/avrdude.conf -v -patmega32u4 -cusbtiny -Uflash:w:/tmp/arduino_build_6710/PetitFS_work_8th_delvery_AES.ino.hex:i -B1

    !!! remember to modify the number in >>> arduino_build_6710 <<< - it changes each time Arduino IDE is loaded !!!

    and to set apprioprate values for fuses:

    avrdude -v -patmega32u4 -cusbtiny -Uhfuse:w:0xD9:m -Ulfuse:w:0xFF:m -Uefuse:w:0xF3:m


  == below sentence makes an update of Serial.print line numbering =================================================================

   simple assign code as "c" like c=''' ... '''

   import re
   NL=chr(0xa)

   print(NL.join([(line,re.sub('\d{3,}','%d' % (n+1), line, 1))['Serial.print' in line and bool(re.search('"\d{3,}',line))] for n, line in enumerate(c.split(NL))]))


  ===============================================================================================================
*/


//----------------------------Libraries---------------------------------

#include "CONFIG_PIN_CONFIG.h"                                      //Pin Konfiguration etc

#include "USBCore.h"                // use modificated to save space skipping USB port support

//=============================================================================================================

//=============================================================================================================

//Text IDs zu Werten

#include <AES.h>         // Verschlüsselungslibrary AES
#include <SpritzCipher.h> // Verschlüsselungslibrary Spritz
#include <SPI.h>         // SPI Treiber
#include "OneButton.h"   // Library für Button handling
#include <MFRC522.h>     // RFID Chip Treiber
#include <SPIFlash.h>    // Treiber für FLASH Memory
#include <TFT_ILI9163.h> // Treiber für Display incl Fonts

#include <Keyboard.h>    // Keyboard USB Emulator

#include <PetitFS.h>




/*
 * Remark
   Modify USB profile to remove serial port - just want keyboard
   in cloud of above #include <Keyboard.h>
   as described in https://github.com/arduino/Arduino/issues/6387
   some modification (2)- comments out on 2018-09-03

   in /home/me/.arduino15/packages/arduino/hardware/avr/1.6.23/cores/arduino/USBCore.cpp:60
   60 -- //#  define USB_MANUFACTURER "SparkFun"
   61 -- #  define USB_MANUFACTURER "SLARK GmbH"

   in /home/me/.arduino15/packages/SparkFun/hardware/avr/1.1.12/boards.txt:76
   76 -- promicro.name=SparkFun Pro Micro
   77 -- #promicro.name=SLARK GmbH
*/

//--------------------------Lokale Defines--------------------------------

#define USE_GRAY_COLORS_4_SMALL_FONTS

// comment QUIET to show observed errors and progress
#define QUIET

#define SHOW_READ_PROGRESS


//#define SERIAL_DEBUG

//#define SERIAL_DEBUG_MFRC522
//#define SERIAL_DEBUG_select_pwd     // to see if decrypt works
//#define SERIAL_DEBUG_select_pwd_1   // to see display_visible_pwds
//#define SERIAL_DEBUG_import_dir
//#define SERIAL_DEBUG_read_sd
//#define SERIAL_DEBUG_SD_inserted

//#define SERIAL_DEBUG_read_sd_font
//#define SERIAL_DEBUG_dump_pwds

//#define SERIAL_DEBUG_dump_pwd_to_pass

//#define SERIAL_DEBUG_dump_fonts

//#define SERIAL_DEBUG_show_text

//#define SERIAL_DEBUG_displayString
//#define SERIAL_DEBUG_displayString_o

//#define SERIAL_DEBUG_displayString_x
//#define SERIAL_DEBUG_displayString_xx
//#define SERIAL_DEBUG_displayString_y
//#define SERIAL_DEBUG_displayString_z

//#define SERIAL_DEBUG_select_pwd_2

//#define ERASE_ALL_PWD_FLASH
//#define TIME_4_READ_IN_MEASUREMENT

//#define SERIAL_DEBUG_createPWD

#define FONT_SAVE_PROGRESS

// The SD chip select pin is currently defined as 10
// in pffArduino.h.  Edit pffArduino.h to change the CS pin.

FATFS fs;     /* File system object */
DIR dj;

TFT_ILI9163 tft = TFT_ILI9163(); // library instanziieren, Pins definiert in User_Setup.h

SPIFlash flash(FLASH_SS);

bool button_pressed_while_power_on = false;

uint8_t select_pwd_ptr_len;

//#define RFID_READ_RESET

void(* resetFunc) (void) = 0;

//-----------------------------Tastatur-----------------------

void hold_by_power_on() {

  /*
    OneButton button_c_s(SWITCH_C, true); // Instanz Schalter auf Wippe

    button_c_s.tick();

    button_pressed_while_power_on = button_c_s.iSERIAL_DEBUG_displayStringsHoldOnStart();
  */

  pinMode( SWITCH_C, INPUT_PULLUP );

  button_pressed_while_power_on = digitalRead(SWITCH_C) == LOW;

}


byte smallbig = 2; // size of fonts to be used; 1 for PWD names, 2 for PIN and some displayed textes

uint16_t background_color = TFT_BLACK ^ TFT_INVERT_MASK;


#ifdef USE_GRAY_COLORS_4_SMALL_FONTS
uint16_t font_colors_on_black[6] = {0xffff, 0xbdd7, 0xa514, 0x630c, 0x2945, 0x0841};
uint16_t font_colors_on_teal[6]  = {0xffff, 0xded7, 0xce74, 0xad6c, 0x9485, 0x8401};
//uint16_t font_colors_on_green[6] = {0xffff, 0xbff7, 0xa7f4, 0x67ec, 0x2fe5, 0x0fe1};
//uint16_t font_colors_on_ornge[6] = {0xffff, 0xbf3f, 0xa6ff, 0x663f, 0x2d9f, 0x0d3f};
//uint16_t font_colors_on_red[6]   = {0xffff, 0xbddf, 0xa51f, 0x631f, 0x295f, 0x085f};

//uint16_t font_colors[5][6] = {(0xffff, 0xbdd7, 0xa514, 0x630c, 0x2945, 0x0841),
//                              (0xffff, 0xded7, 0xce74, 0xad6c, 0x9485, 0x8401),
//                              (0xffff, 0xbff7, 0xa7f4, 0x67ec, 0x2fe5, 0x0fe1),
//                              (0xffff, 0xbf3f, 0xa6ff, 0x663f, 0x2d9f, 0x0d3f),
//                              (0xffff, 0xbddf, 0xa51f, 0x631f, 0x295f, 0x085f)};

uint16_t * font_color_ptr = & font_colors_on_black[0];
#endif

bool SD_Card_inserted = false;
short Systemzustand = PIN_EINGABE; // Variable für Systemzustand Defines in CONFIG_SYS_STATES.h

int anzahl_pwd = 0;
int not_saved_anzahl_pwd;
int f_number = 0;
int count_font_files = 0;
int ignore_number = 0;



int current_pwd = 0;
int marked_pwd;

bool lockup_background_called = false;

byte pin_digits[PIN_length] = {0, 0, 0, 0};

long start_time_for_pin_input;

int pin = 0;
int old_pin = 0;
byte current_input = 0;
byte old_input = 0;
byte current_digit = 0;

int pwd_first_visible = 0;
bool any_button_active;

unsigned long targetTime = 0;

//------------------------------------------------------------------------------
void setup() {

  SPI.begin(); // Init SPI bus

  flash.begin(SPI_FLASH_SIZE);

#ifdef SERIAL_DEBUG_dump_fonts

  Serial.begin(9600);

  while (!Serial);

  uint8_t buf[32];

  if (false) {
    Serial.println(F("230 loop entered: "));
    Serial.print(F("Dump small fonts: "));
    Serial.println();

    for (uint32_t j = FD_Start; j < FD_Start_1; j += 32) {

      flash.readCharArray(j, buf, 32);

      for (int i = 0; i < 32; i++) {
        Serial.print(buf[i] < 0x10 ? " 0" : " ");
        Serial.print(buf[i], HEX);
      }
      Serial.println();
    }
  }

  if (false) {
    Serial.println(F("247 loop entered: "));
    Serial.print(F("Dump big fonts: "));
    Serial.println();

    for (uint32_t j = FD_Start_1; j < SPI_FLASH_SIZE; j += 32) {

      flash.readCharArray(j, buf, 32);

      for (int i = 0; i < 32; i++) {
        Serial.print(buf[i] < 0x10 ? " 0" : " ");
        Serial.print(buf[i], HEX);
      }
      Serial.println();
    }
  }

  if (true) {
    Serial.println(F("264 loop entered: "));
    Serial.print(F("Dump gray fonts: "));
    Serial.println();

    Serial.print(F("FD_Start_small_gray: "));
    Serial.print(FD_Start_small_gray, HEX);
    Serial.print(F(" - "));
    Serial.println(FD_Start_small_gray);

    Serial.print(F("FD_Start_small_gray_size: "));
    Serial.print(FD_Start_small_gray_size, HEX);
    Serial.print(F(" - "));
    Serial.println(FD_Start_small_gray_size);

    for (uint32_t j = (uint32_t) FD_Start_small_gray; j < ((uint32_t) FD_Start_small_gray + (uint32_t) FD_Start_small_gray_size); j += 32) {

      flash.readCharArray(j, buf, 32);

      for (int i = 0; i < 32; i++) {
        Serial.print(buf[i] < 0x10 ? " 0" : " ");
        Serial.print(buf[i], HEX);
      }
      Serial.println();
    }
  }

  if (true) {
    Serial.println(F("291 loop entered: "));
    Serial.print(F("Dump gray fonts: "));
    Serial.println();

    Serial.print(F("FD_Start_big_gray: "));
    Serial.print((uint32_t) FD_Start_big_gray, HEX);
    Serial.print(F(" - "));
    Serial.println((uint32_t) FD_Start_big_gray);

    Serial.print(F("FD_BIG_SIZE_gray: "));
    Serial.print((uint32_t) FD_BIG_SIZE_gray, HEX);
    Serial.print(F(" - "));
    Serial.println((uint32_t) FD_BIG_SIZE_gray);

    for (uint32_t j = (uint32_t) FD_Start_big_gray; j < ((uint32_t) FD_Start_big_gray + (uint32_t) FD_BIG_SIZE_gray); j += 32) {

      flash.readCharArray(j, buf, 32);

      for (int i = 0; i < 32; i++) {
        Serial.print(buf[i] < 0x10 ? " 0" : " ");
        Serial.print(buf[i], HEX);
      }
      Serial.println();
    }
  }

  while (true);
#endif

#ifdef SERIAL_DEBUG_dump_pwds

  Serial.begin(9600);

  while (!Serial);

  uint8_t aux_buf[32];

  Serial.println(F("328 loop entered: "));
  Serial.print(F("Dump passwords names: "));
  Serial.println();

  for (uint32_t j = PWD_Index; j < PWD_Start; j += 32) {

    flash.readCharArray(j, aux_buf, 32);

    for (int i = 0; i < 32; i++) {
      Serial.print(aux_buf[i] < 0x10 ? " 0" : " ");
      Serial.print(aux_buf[i], HEX);
    }
    Serial.println();
  }

  Serial.println(F("343 loop entered: "));
  Serial.print(F("Dump passwords: "));
  Serial.print(F(" PWD_Start: "));
  Serial.print(PWD_Start, HEX);
  Serial.print(F(" FD_Start: "));
  Serial.print(FD_Start, HEX);
  Serial.println();

  for (uint32_t j = PWD_Start; j < FD_Start; j += 32) {

    flash.readCharArray(j, aux_buf, 32);

    for (int i = 0; i < 32; i++) {
      Serial.print(aux_buf[i] < 0x10 ? " 0" : " ");
      Serial.print(aux_buf[i], HEX);
    }
    Serial.println();
  }

  while (true);
#endif

  //-------------------------------------DISPLAY --------------------------------

  tft.init();
  tft.setRotation(2);

#ifdef SERIAL_DEBUG_displayString
  Serial.begin(9600);
  while (!Serial);

  Serial.println(F("728 displayString started"));
#endif


#ifdef SERIAL_DEBUG_show_text

  long start_time_for_letter;
  long start_time_for_letter_t;
  long end_time_for_letter;
  bool many = true;

label_abc:

  tft.fillScreen(background_color);

  smallbig = 1;

  start_time_for_letter = micros();
  //displayString_o("THE QUICK BROWN", 1, 128 - 16, TFT_BLACK + 1);
  if (many) displayString("ABCDEFGHIJKLMN", 1, 128 - 16, TFT_BLACK + 1);
  else      displayString("I", 1, 128 - 16, TFT_BLACK + 1);
  end_time_for_letter = micros() - start_time_for_letter;

  char current_string[10];

  for (byte ap = 0; ap < 10; ap++) {
    current_string[8 - ap] = (end_time_for_letter % 10) + '0';
    end_time_for_letter = end_time_for_letter / 10;
  }

  current_string[9] = CHAR_ZERO;

  displayString(current_string, 1, 128 - 32, TFT_BLACK + 1);

  //--------------------------------------------------------------------------------
  if (false) {
    start_time_for_letter = micros();
    if (many) displayString("OPQRSTUVWXYZ", 1, 128 - 48, TFT_BLACK + 1);
    else      displayString("W", 1, 128 - 48, TFT_BLACK + 1);
    end_time_for_letter = micros() - start_time_for_letter;

    for (byte ap = 0; ap < 10; ap++) {
      current_string[8 - ap] = (end_time_for_letter % 10) + '0';
      end_time_for_letter = end_time_for_letter / 10;
    }

    current_string[9] = CHAR_ZERO;

    displayString(current_string, 1, 128 - 64, TFT_BLACK + 1);

    //-------------------------------------------------------------------------

    start_time_for_letter = micros();
    //displayString_o("!\"§$%&/()=?+*'#", 1, 128 - 80, TFT_BLACK + 1);
    //displayString_o("!\"§$%&/()=?+", 1, 128 - 80, TFT_BLACK + 1);
    if (many) displayString("abcdefghijklmnopq", 1, 128 - 80, TFT_BLACK + 1);
    else      displayString("i", 1, 128 - 80, TFT_BLACK + 1);
    end_time_for_letter = micros() - start_time_for_letter;

    for (byte ap = 0; ap < 10; ap++) {
      current_string[8 - ap] = (end_time_for_letter % 10) + '0';
      end_time_for_letter = end_time_for_letter / 10;
    }

    current_string[9] = CHAR_ZERO;

    displayString(current_string, 1, 128 - 96, TFT_BLACK + 1);
    //--------------------------------------------------------------------------------

    start_time_for_letter = micros();
    smallbig = 1;
    //displayString("rsz12345", 1, 128 - 112, TFT_BLACK + 1);
    //displayString("AaTtZz_01", 1, 128 - 112, TFT_BLACK + 1);
    if (many) displayString("!\"§$%&/()=?+*'#@", 1, 128 - 112, TFT_BLACK + 1);
    else      displayString("w", 1, 128 - 112, TFT_BLACK + 1);

    end_time_for_letter = micros() - start_time_for_letter;

    for (byte ap = 0; ap < 10; ap++) {
      current_string[8 - ap] = (end_time_for_letter % 10) + '0';
      end_time_for_letter = end_time_for_letter / 10;
    }

    current_string[9] = CHAR_ZERO;

    smallbig = 1;
    displayString(current_string, 1, 128 - 128, TFT_BLACK + 1);
  } else {
    smallbig = 2;

    start_time_for_letter = micros();
    //displayString_o("THE QUICK BROWN", 1, 128 - 16, TFT_BLACK + 1);
    displayString("CW_wz#$@", 1, 128 - 64, TFT_BLACK + 1);
    end_time_for_letter = micros() - start_time_for_letter;

    char current_string[10];

    for (byte ap = 0; ap < 8; ap++) {
      current_string[6 - ap] = (end_time_for_letter % 10) + '0';
      end_time_for_letter = end_time_for_letter / 10;
    }

    current_string[7] = CHAR_ZERO;

    displayString(current_string, 1, 128 - 96, TFT_BLACK + 1);

    smallbig = 1;

    start_time_for_letter = micros();
    //displayString_o("!\"§$%&/()=?+*'#", 1, 128 - 80, TFT_BLACK + 1);
    //displayString_o("!\"§$%&/()=?+", 1, 128 - 80, TFT_BLACK + 1);
    if (many) displayString("!\",$%&/()=?+*'#", 1, 128 - 112, TFT_BLACK + 1);
    else      displayString("i", 1, 128 - 112, TFT_BLACK + 1);
    end_time_for_letter = micros() - start_time_for_letter;

    for (byte ap = 0; ap < 10; ap++) {
      current_string[8 - ap] = (end_time_for_letter % 10) + '0';
      end_time_for_letter = end_time_for_letter / 10;
    }

    current_string[9] = CHAR_ZERO;

    displayString(current_string, 1, 128 - 128, TFT_BLACK + 1);
  }

  //--------------------------------------------------------------------------

  //  displayString("FOX JUMPS OVER", 1, 128 - 48, TFT_BLACK + 1);
  //  displayString("THE LAZY DOG", 1, 128 - 64, TFT_BLACK + 1);
  //  displayString("the quick brown", 1, 128 - 48, TFT_BLACK + 1);
  //  displayString("fox jumps over", 1, 128 - 64, TFT_BLACK + 1);
  //  displayString("the lazy dog", 1, 128 - 80, TFT_BLACK + 1);
  //  displayString("1234567890", 1, 128 - 96, TFT_BLACK + 1);
  //  displayString("!\"§$%&/()=?+*'#", 1, 128 - 112, TFT_BLACK + 1);
  //  displayString(";:_,.-<>@{|}[]^`\\~", 1, 128 - 128, TFT_BLACK + 1);


  //  1234567890\n!\"§$%&/()=?+*'#;:_,.-<>@{|}[]^`\\~
  //
  //  displayString("ABCDEFGH", 10, 100, TFT_BLACK+1);
  //  delay(3000);
  //  displayString("IJKLMNOP", 10, 80, TFT_BLACK+1);
  //  delay(3000);
  //  displayString("QRSTUVWXYZ", 10, 60, TFT_BLACK+1);
  //  delay(3000);
  //  displayString("abcdefghijklmno", 10, 40, TFT_BLACK + 1);
  //  delay(3000);
  //  displayString("pqrstuvwxyz", 10, 20, TFT_BLACK + 1);
  //  delay(3000);
  //  displayString("1234567890", 10, 20, TFT_BLACK + 1);
  //  delay(3000);


  //goto label_abc;

  while (true);
#endif

#ifdef SERIAL_DEBUG_displayString_xx
  smallbig = 2;
  //strcpy(display_string, "sort PWDs");
  select_ss(TFT_SS);
  displayString("Sort PWDs", 10, 70, TFT_SLARK_GREEN ^ TFT_INVERT_MASK);

  while (true);
#endif

  //hold_by_power_on();

  FRESULT pfs_result;
  FRESULT prev_pfs_result = FR_OK;

#ifdef SERIAL_DEBUG_SD_inserted
  Serial.begin(9600);
  //aes.do_aes_encrypt(plain,sizeof(plain),cipher,key,bits,iv);
  while (!Serial);

  Serial.println(F("429 loop entered: "));
  Serial.print(F("SD_Card_inserted: "));
  Serial.println(SD_Card_inserted);
#endif

  select_ss(SD_SS);

  for (byte m = 0; m < 10; m++) {

    pfs_result = pf_mount(&fs);

#ifdef SERIAL_DEBUG_SD_inserted
    Serial.println(F("441 pfs_result: "));
    Serial.println(pfs_result);
#endif

    if (pfs_result == FR_OK && pfs_result == prev_pfs_result) {
      SD_Card_inserted = true;
      break;
    }

    if (pfs_result == RES_NOTRDY && pfs_result == prev_pfs_result) {
      SD_Card_inserted = false;
      break;
    }

    prev_pfs_result = pfs_result;
  }

  //  if (pfs_result == FR_OK) {
  //    SD_Card_inserted = true;
  //  } else {
  //    SD_Card_inserted = false;
  //  }

#ifdef SERIAL_DEBUG_SD_inserted
  Serial.println(F("465 pfs_result: "));
  Serial.println(SD_Card_inserted);
#endif

  //select_ss(TFT_SS);
  //tft.fillScreen(SD_Card_inserted ? TFT_SLARK_SKYBLUE : TFT_BLACK);
  background_color = SD_Card_inserted ? TFT_SLARK_SKYBLUE : TFT_BLACK;
  tftFillScreen(background_color);

  for (byte jk = 0; jk < 4; jk++) displayletter('0', 40 + jk * 20, 60, TFT_WHITE);

  //read_sd();

}

char display_string[16];

void loop() {

#ifdef STACK_CANARY_VAL
  while (p <= SP) *p++ = STACK_CANARY_VAL;
#endif

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  //char current_name[5];
  //FATFS pfs_fs;
  FRESULT pfs_result;


#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  //aes.do_aes_encrypt(plain,sizeof(plain),cipher,key,bits,iv);
  while (!Serial);

  //SD_Card_inserted = true;
  //button_pressed_while_power_on = true;

  Serial.println(F("502 loop entered: "));
  Serial.print(F("SD_Card_inserted: "));
  Serial.println(SD_Card_inserted);
#endif

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //button_pressed_while_power_on = true;

  //if (button_pressed_while_power_on) SD_Card_inserted = (bool) SD.begin(SD_SS);
  //if (button_pressed_while_power_on) {
  if (false) {

    //disk_initialize();

    smallbig = 1;

    //init_spi();

    select_ss(SD_SS);

    for (byte m = 0; m < 10; m++) {
      //pfs_result = pf_mount(&fs);
      //if (pfs_result == FR_OK) break;
      if (pf_mount(&fs)) break;
    }

#ifdef SERIAL_DEBUG_pfs_fs
    Serial.print(F("529a pf_mount called: "));

    Serial.print(F("pfs_result: "));
    Serial.println(pfs_result);

    Serial.println(F("pfs_fs.fs_type, pfs_fs.flag, pfs_fs.csize, pfs_fs.pad1, pfs_fs.n_rootdir, pfs_fs.n_fatent, pfs_fs.org_clust, pfs_fs.curr_clust"));
    Serial.println(pfs_fs.fs_type);
    Serial.println(pfs_fs.flag);
    Serial.println(pfs_fs.csize);
    Serial.println(pfs_fs.pad1);
    Serial.println(pfs_fs.n_rootdir);
    Serial.println(pfs_fs.n_fatent);
    Serial.println(pfs_fs.fatbase);
    Serial.println(pfs_fs.dirbase);
    Serial.println(pfs_fs.database);
    Serial.println(pfs_fs.fptr);
    Serial.println(pfs_fs.fsize);
    Serial.println(pfs_fs.org_clust);
    Serial.println(pfs_fs.curr_clust);
#endif

    smallbig = 2;

    if (pfs_result == FR_OK) {
      SD_Card_inserted = true;
      background_color = 1;
    } else  {
      SD_Card_inserted = false;
      background_color = 0;

#ifdef SERIAL_DEBUG
      Serial.print(F("pfs_result: "));
      Serial.println(pfs_result);
#endif
      strcpy(display_string, "Check SD");

      select_ss(TFT_SS);

      //displayString("Check SD", 15, 50, TFT_SLARK_RED ^ TFT_INVERT_MASK);
      displayString(display_string, 15, 50, TFT_SLARK_RED ^ TFT_INVERT_MASK);
    }

    //display_number(2, SD_Card_inserted, 0);
  }

#ifdef SERIAL_DEBUG_dump_pwd_to_pass

  Serial.begin(9600);

  while (!Serial);
#endif

#ifdef PERFORM_PIN_CHECK
  if (Systemzustand == PIN_EINGABE) {

    current_input = 0;
    pin_input();
  }
#else
  Systemzustand = PIN_KORREKT;
#endif

#ifdef STACK_CANARY_VAL
  display_number(8, 1000000 + StackMarginWaterMark(), 0);
  display_number(8, 1000000 + freeRam(), 0);
#endif

#ifdef PERFORM_RFID_CHECK
  if (Systemzustand == PIN_KORREKT) {

    Systemzustand = RFID_LESEN;

    smallbig = 2;

    //displayString("RFID", 30, 50, TFT_BLACK ^ TFT_INVERT_MASK);
    //select_ss(TFT_SS);
    strcpy(display_string, "RFID");
    //
    displayString(display_string, 40, 60, SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK);
    //displayString(display_string, 160 - get_str_width(display_string)*2, 50, SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK);
    //delay(2000);

    //select_ss(RFID_SS);
    read_chip_key();

    //strcpy(display_string, "RFID");
    //select_ss(TFT_SS);
    //tft.fillScreen(TFT_SLARK_GREEN ^ TFT_INVERT_MASK);

    //tftFillScreen(TFT_SLARK_GREEN ^ TFT_INVERT_MASK);
    background_color = TFT_SLARK_GREEN ^ TFT_INVERT_MASK;
    tftFillScreen(background_color);

    displayString(display_string, 40, 60, TFT_SLARK_GREEN ^ TFT_INVERT_MASK);

    delay(500);

  }
#else
  Systemzustand = CODE_IM_GERAET;
#endif

#ifdef STACK_CANARY_VAL
  display_number(8, 2000000 + StackMarginWaterMark(), 0);
  display_number(8, 2000000 + freeRam(), 0);
#endif

#ifdef PERFORM_SD_CHECK
  if (Systemzustand == CODE_IM_GERAET) {
    if (SD_Card_inserted) {
      strcpy(display_string, "Read SD");
      displayString(display_string, 10, 50, TFT_SLARK_GREEN ^ TFT_INVERT_MASK);

      read_sd(&fs);

      anzahl_pwd = f_number;

      smallbig = 2;

      int an = anzahl_pwd;
      int nn = 1;

      while (an /= 10) nn++;

      display_number(nn, anzahl_pwd + not_saved_anzahl_pwd, PWD_Index_Size);

      sort_pwds();
    }

    Systemzustand = PWD_AUSWAHL;
  }
#else
  Systemzustand = PWD_AUSWAHL;
#endif

#ifdef STACK_CANARY_VAL
  display_number(8, 3000000 + StackMarginWaterMark(), 0);
  display_number(8, 3000000 + freeRam(), 0);
#endif

#ifdef PERFORM_PWD_SELECT

  //select_ss(TFT_SS);
  //tft.fillScreen(TFT_BLACK ^ TFT_INVERT_MASK);
  background_color = TFT_BLACK ^ TFT_INVERT_MASK;
  tftFillScreen(background_color);
  //tftFillScreen(TFT_BLACK ^ TFT_INVERT_MASK);


  while (Systemzustand == PWD_AUSWAHL) {

    smallbig = 1;

    select_pwd();
  }
#else
  while (true); // endless loop to prevent one from loop "void loop()" as defined above - it would endless check one or all of PIN, RFID and SD
#endif

}


void sort_pwds() {

  long msecs = millis();
  long p_msecs = msecs;

  bool repeat = true;
  char sorted_idx[anzahl_pwd];
  char s1[PWD_Index_Text_size], s2[PWD_Index_Text_size], aux;

  for (int i = 0; i < anzahl_pwd; i++) sorted_idx[i] = (char) i;

  long flash_address_1;
  long flash_address_2;

  uint16_t bar = 0;

  strcpy(display_string, "Sorting");

  displayString(display_string, 10, 70, TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK);

  select_ss(FLASH_SS);

  while (repeat) {
    repeat = false;

    for (int i = 0; i < anzahl_pwd - 1; i++) {

      if ((millis() - msecs) > 1000) {
        select_ss(TFT_SS);

        tft.fillRect(50, 20 + 2 * bar++, 2, 2, TFT_WHITE);

        msecs = millis();

        select_ss(FLASH_SS);
      }

      flash_address_1 = PWD_Index + (PWD_Index_Text_size + PWD_Index_Header) * ((uint8_t) sorted_idx[i]);
      flash_address_2 = PWD_Index + (PWD_Index_Text_size + PWD_Index_Header) * ((uint8_t) sorted_idx[i + 1]);

      for (byte j = 0; j < PWD_Index_Text_size; j++) {

        s1[j] = flash.readChar(flash_address_1 + j);
        if (!s1[j]) break;

        s2[j] = flash.readChar(flash_address_2 + j);
        if (!s2[j]) break;

        if (toupper(s1[j]) > toupper(s2[j])) {

          aux = sorted_idx[i];
          sorted_idx[i] = sorted_idx[i + 1];
          sorted_idx[i + 1] = aux;

          repeat = true;

          break;

        } else {
          if (toupper(s1[j]) < toupper(s2[j])) {
            break;
          }
        }
      }
    }
  }

  //flash_erase_2_write(PWD_Abc_Sorted_Index, 1);
  flash.eraseSector(PWD_Abc_Sorted_Index);

  for (int i = 0; i < anzahl_pwd; i++) flash.writeChar(PWD_Abc_Sorted_Index + i, sorted_idx[i], true);


  // use sorted_idx to store the number of PWDs and as actuall value of PIN to save memory using
  // one call of writeCharArray on place of 4 calls of writeChar

  // !!! PWD_Index_Count and PIN_valid_value have to occupy 4 consecutive bytes starting at address given by PWD_Index_Count !!!

  sorted_idx[0] = (char) (anzahl_pwd / 0x100);
  sorted_idx[1] = (char) (anzahl_pwd % 0x100);

  sorted_idx[2] = (char) (pin / 0x100);
  sorted_idx[3] = (char) (pin % 0x100);

  //flash.writeCharArray(PWD_Index_Count, sorted_idx[0], 4, false);
  for (int i = 0; i < 4; i++) flash.writeChar(PWD_Index_Count + i, sorted_idx[i], true);
}



//------------------------------------------------------------------------------
void errorHalt(char* msg) {
#ifdef SERIAL_DEBUG
  Serial.print("Error: ");
  Serial.println(msg);
#endif
  while (1);
}


#ifdef USE_CRYPT
uint8_t crypt_key[CRYPT_KEY_SIZE] = {0};
#endif

void test_generate_and_store_crypt_key() {

  randomSeed(analogRead(NOT_CONNECTED_ANALOG_PIN_4_RANDOM_SEED));

  for (byte i = 0; i < 32; i++) {
    crypt_key[i] = random(256);
  }
}


//void encrypt_pwd(byte plain[PWD_Size + 1], byte cipher[PWD_Size + 1]) {
void encrypt_pwd(uint8_t plain[PWD_Size], uint8_t cipher[PWD_Size], bool do_encrypt) {

  //cipher = plain;

  //return;

#ifdef FOLLOW_USE_CRYPT
  for (int i = 0; i < 32; i++) {
    Serial.print(crypt_key[i] < 0x10 ? " 0" : " ");
    Serial.print(crypt_key[i], HEX);
  }
  Serial.println();
#endif

#ifdef USE_CRYPT

#ifdef SERIAL_DEBUG_dump_pwd_to_pass
  Serial.println(F("812 MIFARE_Read() success: "));
  Serial.print(F("crypt_key: "));
  dump_byte_array(crypt_key, CRYPT_KEY_SIZE);
  Serial.println();

  Serial.println(F("817 MIFARE_Read() success: "));
  Serial.print(F("plain: "));
  dump_byte_array(plain, CRYPT_KEY_SIZE);
  Serial.println();
#endif

#ifdef USE_SPRITZ
  spritz_ctx s_ctx;

  //byte buf[PWD_Size + 1]; /* Output buffer */
  //unsigned int i;

  spritz_setup(&s_ctx, crypt_key, sizeof (crypt_key));
  //spritz_crypt(&s_ctx, plain, PWD_Size + 1, cipher);
  spritz_crypt(&s_ctx, plain, PWD_Size, cipher);

#else

  AES aes; // Instanz Verschlüsselung

  byte succ = aes.set_key(crypt_key, 256);

  if (do_encrypt) succ = aes.encrypt(plain, cipher);
  else succ = aes.decrypt(cipher, plain);

#endif

#ifdef SERIAL_DEBUG_dump_pwd_to_pass
  Serial.println(F("845 MIFARE_Read() success: "));
  Serial.print(F("cipher: "));
  dump_byte_array(cipher, CRYPT_KEY_SIZE);
  Serial.println();
#endif
#endif
}


//------------------------------------------------------------------------------

void print_dj(DIR dj) {
  if (false) {
    Serial.print("858 dj: ");
    dump_byte_array((byte *) &dj, sizeof(dj));
    Serial.println();
  } else {
    Serial.print("dj.index: ");
    Serial.println(dj.index);
    Serial.print("dj.fn: ");
    Serial.println((long) dj.fn);
    Serial.print("dj.sclust: ");
    Serial.println(dj.sclust);
    Serial.print("dj.clust: ");
    Serial.println(dj.clust);
    Serial.print("dj.sect: ");
    Serial.println(dj.sect);
  }
}

//------------------------------------------------------------------------------

int read_sd(FATFS *fs) {

#ifdef USE_CRYPT
#ifndef PERFORM_RFID_CHECK
  test_generate_and_store_crypt_key();
#endif
#endif


  FILINFO fno;
  FRESULT pfs_result;
  PFS_UINT nr;


#ifdef SERIAL_DEBUG_read_sd
  Serial.begin(9600);
  //aes.do_aes_encrypt(plain,sizeof(plain),cipher,key,bits,iv);
  while (!Serial);

  Serial.println(F("896 Serial display started"));
#endif

  //  select_ss(FLASH_SS);
  //
  //  flash.eraseBlock32K(PWD_Index);
  //  flash.eraseBlock32K(PWD_Start);
  //  flash.eraseBlock32K(PWD_Start + 0x8000);

  //select_ss(TFT_SS);
  //tft.fillScreen(SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK);

  background_color = SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK;
  tftFillScreen(background_color);

  //tftFillScreen(SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK);

  smallbig = 1;


  // Initialize SD and file system.
  //  pfs_result = pf_mount(&fs);

  select_ss(SD_SS);

  for (byte m = 0; m < 3; m++) {
    //delay(3000);


    pfs_result = pf_mount(fs);

    if (pfs_result == FR_OK) {

#ifdef SERIAL_DEBUG_read_sd_x
      Serial.print("926 mount pfs_result: ");
      Serial.print(pfs_result);
      Serial.print(" - ");
      Serial.println(m);

      Serial.print(F("931a pf_mount called: "));

      Serial.print(F("pfs_result: "));
      Serial.println(pfs_result);

      Serial.println(F("pfs_fs.fs_type, pfs_fs.flag, pfs_fs.csize, pfs_fs.pad1, pfs_fs.n_rootdir, pfs_fs.n_fatent, pfs_fs.org_clust, pfs_fs.curr_clust"));
      Serial.println(fs->fs_type);
      Serial.println(fs->flag);
      Serial.println(fs->csize);
      Serial.println(fs->pad1);
      Serial.println(fs->n_rootdir);
      Serial.println();
      Serial.println(fs->n_fatent);
      Serial.println(fs->fatbase);
      Serial.println(fs->dirbase);
      Serial.println(fs->database);
      Serial.println();
      Serial.println(fs->fptr);
      Serial.println(fs->fsize);
      Serial.println(fs->org_clust);
      Serial.println(fs->curr_clust);
      Serial.println(fs->dsect);
      Serial.println();
#endif

      break;
    } else {
#ifdef SERIAL_DEBUG_read_sd
      Serial.print("959 mount pfs_result: ");
      Serial.println(pfs_result, m);
#endif
    }
  }

  if (pfs_result) {
#ifdef SERIAL_DEBUG_read_sd
    Serial.print("967 mount pfs_result: ");
    Serial.println(pfs_result);
#endif
  }

  if (pfs_result) {
    strcpy(display_string, "pf_mount");
    errorHalt(display_string);
  }

  select_ss(SD_SS);

  pfs_result = pf_opendir((&dj), "/");

  if (false) {
    if (pfs_result == FR_OK) {
      for (;;) {
        pfs_result = pf_readdir(&dj, &fno);
        if (pfs_result != FR_OK || fno.fname[0] == 0) break;
        Serial.print("986 opendir pfs_result: ");
        Serial.print(fno.fname);
        Serial.print(" - ");
        Serial.print(fno.fsize);
        Serial.println();
      }
      while (true);
    }
  }

  if (pfs_result) {
#ifdef SERIAL_DEBUG_read_sd
    Serial.print("998 opendir pfs_result: ");
    Serial.println(pfs_result);
#endif
    strcpy(display_string, "pf_opendir");
    errorHalt(display_string);
  } else {
#ifdef SERIAL_DEBUG_read_sd
    print_dj(dj);
#endif
  }

  //FRESULT fr;
  char save_fname[13];
  uint16_t save_dj_index;

  long msecs = millis();
  long p_msecs = msecs;
  long delta_msecs;

  char eol[2] {CHAR_NL, CHAR_ZERO};

  char buf[32];
  char pwd_buf[PWD_Size];

  char *ptr_pwd_buf = &pwd_buf[0];

  bool flash_prepared_2_save_PWDs = false;

  byte name_length;
  bool written;
  int len_1st, len_2nd;

  long flash_addr;

  select_ss(SD_SS);

  //pfs_result = pf_readdir(&dj, &fno);

  do {

    pfs_result = pf_readdir(&dj, &fno);

  } while (!fno.fsize || fno.fname[0] == '.' || fno.fname[0] == '_');      // mod 4.2 - skip hidden files
  // mod04.04 - consider '_' given by PetitFS as 1st char in name


#ifdef SERIAL_DEBUG_read_sd_x
  Serial.println(F("1045 fname: "));
  Serial.print(fno.fname);
  Serial.print(" - ");
  Serial.print(pfs_result);
  Serial.println();
  print_dj(dj);
#endif


  do {

#ifdef SERIAL_DEBUG_read_sd
    Serial.print("1057 readdir pfs_result: ");
    Serial.print(pfs_result);
    Serial.print(" - ");
    Serial.print(fno.fname);
    Serial.print(" - ");
    Serial.println(fno.fsize);
#endif

    len_1st = 0;
    len_2nd = 0;

#ifdef SERIAL_DEBUG_read_sd_x
    Serial.print("1069 no of all files: ");
    Serial.println(f_number + count_font_files + not_saved_anzahl_pwd);
#endif

    // readdir must be called before clear screen if all VISIBLE names have been printed
    // otherwise readdir returns a garbagge as the file name -> therefore call it at the bottom of
    // this sequence and if no new screen is needed call it just here !!!
    //    if ((f_number + count_font_files + not_saved_anzahl_pwd) != 0 && (f_number + count_font_files + not_saved_anzahl_pwd) % VISIBLE_PASSWORDS == 0) {
    //      len_1st = 0;
    //    } else {
    //      pfs_result = pf_readdir(&dj, &fno);
    //
    //#ifdef SERIAL_DEBUG_read_sd
    //      print_dj();
    //#endif
    //
    //
    //    }
    //
    //    if (pfs_result) {
    //#ifdef SERIAL_DEBUG_read_sd
    //      Serial.print("1090 readdir pfs_result: ");
    //      Serial.println(pfs_result);
    //#endif
    //      strcpy(display_string, "pf_readdir");
    //      errorHalt(display_string);
    //    } else {
    //#ifdef SERIAL_DEBUG_read_sd
    //      Serial.print("1097 pf_open - fno.fname: ");
    //      Serial.print(" - ");
    //      Serial.print(fno.fname);
    //      //    Serial.print(" - ");
    //      //    dump_byte_array(fno.fname, strlen(fno.fname));
    //      Serial.println();
    //#endif
    //    }

    //if (f_number > PWD_Index_Size - 1) break;

    //if (!dj.sect) break;
    //if (!fno.fname) break;

#ifdef SERIAL_DEBUG_read_sd
    Serial.print("1112 no of all files: ");
    Serial.println(fno.fname);
#endif

    displayString(fno.fname, 10, 112 - 16 * ((f_number + count_font_files + not_saved_anzahl_pwd + ignore_number) % VISIBLE_PASSWORDS), (TFT_BLACK ^ TFT_INVERT_MASK) + 1);

    open_file(fno.fname);

    if (is_typefile(fno.fname, ".bik")) {

      select_ss(FLASH_SS);

      flash.eraseSector((uint32_t) Keyboard_Layout_Start);

      len_1st = 0;

      select_ss(SD_SS);

      for (byte ef = 0; ef < 3; ef++) {

        pfs_result = pf_read(pwd_buf, 256, &nr);

#ifdef SERIAL_DEBUG_read_sd
        if (pfs_result) {
          Serial.print("1136 open pfs_result: ");
          Serial.println(pfs_result);
        } else {
          Serial.print("1139 open pfs_result: ");
          Serial.println(pfs_result);
          break;
        }
#endif
      }

      if (pfs_result) {
#ifdef SERIAL_DEBUG_read_sd_x
        Serial.print("1148 BIK not data read: ");
        Serial.print(len_1st);
        dump_byte_array(pwd_buf, 256);
        Serial.println();
#endif
        strcpy(display_string, "pf_read");
        errorHalt(display_string);
      } else {
#ifdef SERIAL_DEBUG_read_sd_x
        Serial.print("1157 BIK data read from SD: ");
        Serial.print(len_1st);
        dump_byte_array(pwd_buf, 256);
        Serial.println();
#endif
      }

      flash_addr = Keyboard_Layout_Start;

      Serial.println("1166 write bik data");

      select_ss(FLASH_SS);

      //-------------------------------------------------------------------------------------------------------------------
      for (int ii = 0; ii < 256; ii++) flash.writeChar(flash_addr + ii, ptr_pwd_buf[ii], true);
      //-------------------------------------------------------------------------------------------------------------------

#ifdef SERIAL_DEBUG_read_sd_x
      if (written) {

        Serial.println("1177 BIK name written ");

        flash_addr = Keyboard_Layout_Start;

        select_ss(FLASH_SS);

        flash_addr = Keyboard_Layout_Start;

        written = flash.readCharArray(flash_addr, ptr_pwd_buf, len_1st);

        if (written) {
          Serial.println("1188 BIK data read from Flash");
        } else {
          Serial.println("1190 BIK data not read ");
        }

        dump_byte_array(ptr_pwd_buf, 256);
        Serial.println();
      } else {
        Serial.println("1196 BIK name not written ");

        select_ss(FLASH_SS);

        written = flash.readCharArray(flash_addr, ptr_pwd_buf, len_1st, true);

        if (written) {
          Serial.println(" BIK data read ");
        } else {
          Serial.println(" BIK data not read ");
        }

        dump_byte_array(pwd_buf, 256);

        Serial.println();
      }
#endif

      select_ss(SD_SS);

      count_font_files++;

    } else {

      if (is_typefile(fno.fname, ".bin")) {
        //open_file(fno.fname);

#ifdef USE_GRAY_COLORS_4_SMALL_FONTS
        written = save_fonts_data(fno.fsize, (uint32_t) FD_Start_small_gray, (uint32_t) FD_SMALL_STEP_gray, (uint32_t) FD_SMALL_SIZE_gray, ptr_pwd_buf);
#else
        written = save_fonts_data(fno.fsize, (uint32_t) FD_Start, (uint32_t) FD_SMALL_STEP, (uint32_t) FD_SMALL_SIZE, ptr_pwd_buf);
#endif
        select_ss(SD_SS);

        count_font_files++;

      } else {

        if (is_typefile(fno.fname, ".bip")) {
          //open_file(fno.fname);

          written = save_fonts_data(fno.fsize, (uint32_t) FD_Start_1, (uint32_t) FD_BIG_STEP, (uint32_t) FD_BIG_SIZE, ptr_pwd_buf);

          select_ss(SD_SS);

          count_font_files++;

        } else {

          if (is_typefile(fno.fname, ".txt")) {

#ifdef SERIAL_DEBUG_read_sd
            Serial.print("1248 fname: ");
            Serial.println(fno.fname);
#endif

            if (!flash_prepared_2_save_PWDs) {
              select_ss(FLASH_SS);

              flash.eraseBlock32K(PWD_Index);
              flash.eraseBlock32K(PWD_Start);
              flash.eraseBlock32K(PWD_Start + 0x8000);

              flash_prepared_2_save_PWDs = true;

            }

            if (f_number < PWD_Index_Size) {

              //---------------------------------------------------------------------------------------------------------------
              uint16_t p[4] = { 0, 0, 0, 0 };
              uint16_t j = 0;

              for (uint16_t j = 0; j < sizeof(buf); j++) buf[j] = CHAR_ZERO;
              for (uint16_t j = 0; j < sizeof(pwd_buf); j++) pwd_buf[j] = CHAR_ZERO;

              select_ss(SD_SS);

              pfs_result = pf_read(pwd_buf, sizeof(pwd_buf), &nr);

              while (j < fno.fsize && j < nr && (pwd_buf[j] == CHAR_NL || pwd_buf[j] == CHAR_CR)) {
                p[0]++;
                j++;
              }

              p[1] = p[0];

              while (j < fno.fsize && j < nr && pwd_buf[j] != CHAR_NL && pwd_buf[j] != CHAR_CR) {
                p[1]++;
                j++;
              }

              p[2] = p[1];

              while (j < fno.fsize && j < nr && (pwd_buf[j] == CHAR_NL || pwd_buf[j] == CHAR_CR)) {
                p[2]++;
                j++;
              }

              p[3] = p[2];

              while (j < fno.fsize && j < nr && pwd_buf[j] != CHAR_NL && pwd_buf[j] != CHAR_CR) {
                p[3]++;
                j++;
              }

              if (p[3] < PWD_Size) {
                if (p[0] > 0) {
                  for (uint8_t i = 0; i < p[1] - p[0]; i++) pwd_buf[i] = pwd_buf[p[0] + i];

                  for (uint8_t i = 0; i < p[3] - p[2]; i++) buf[i] = pwd_buf[p[2] + i];

                  pwd_buf[p[1] - p[0]] = CHAR_ZERO;
                  buf[p[3] - p[2]] = CHAR_ZERO;
                } else {
                  for (uint8_t i = 0; i < p[3] - p[2]; i++) buf[i] = pwd_buf[p[2] + i];

                  pwd_buf[p[1]] = CHAR_ZERO;
                  buf[p[3] - p[2]] = CHAR_ZERO;
                }

              } else {

                select_ss(SD_SS);

                pfs_result = pf_lseek(p[2]);

                pfs_result = pf_read(buf, sizeof(buf), &nr);

                j = 0;

                if (nr < PWD_Index_Text_size) {
                  p[0] = 0;

                  while (j < nr && (buf[j] == CHAR_NL || buf[j] == CHAR_CR)) {
                    p[0]++;
                    j++;
                  }

                  p[1] = p[0];

                  while (j < nr & buf[j] != CHAR_NL && buf[j] != CHAR_CR) {
                    p[1]++;
                    j++;
                  }

                  if (p[0] > 0) {
                    for (uint8_t i = 0; i < p[1] - p[0]; i++) buf[i] = buf[p[0] + i];

                    buf[p[1] - p[0]] = CHAR_ZERO;
                  } else {
                    buf[p[1]] = CHAR_ZERO;
                  }
                }
              }

              if (false && createPWD(pwd_buf)) {
                pfs_result = pf_lseek(0);

                pfs_result = pf_write(pwd_buf, strlen(pwd_buf), &nr);
                pfs_result = pf_write(eol, strlen(eol), &nr);
                pfs_result = pf_write(buf, strlen(buf), &nr);
                pfs_result = pf_write(eol, strlen(eol), &nr);
                for (uint8_t j = 0; j < fno.fsize - (strlen(pwd_buf) + strlen(buf) + 2 * strlen(eol)); j++)
                  pfs_result = pf_write(CHAR_NL, 1, &nr);
                pfs_result = pf_write(0, 0, &nr);       // needed to finalize the write operation
              }

              flash_addr = PWD_Start + (uint32_t) PWD_Size * f_number;

              encrypt_pwd(pwd_buf, pwd_buf, true);

              select_ss(FLASH_SS);

              for (byte ef = 0; ef < 3; ef++) {

                written = flash.writeCharArray(flash_addr, ptr_pwd_buf, 256, false);

                if (written) {
                  break;
                }
              }

              uint16_t name_length;

              if (buf[0] == CHAR_ZERO) {
                name_length = strlen(fno.fname);
                for (uint16_t i = 0; i < name_length - 4; i++) buf[i] = fno.fname[i];
                buf[name_length - 4] = 0;
                buf[30] = name_length - 4;
              } else {
                name_length = strlen(buf);
                if (name_length < 30) {
                  buf[name_length] = 0;
                  buf[30] = name_length;
                } else {
                  buf[29] = 0;
                  buf[30] = 29;
                }
              }

              buf[31] = f_number;

              flash_addr = PWD_Index + (PWD_Index_Text_size + PWD_Index_Header) * f_number;

              select_ss(FLASH_SS);

              for (byte ef = 0; ef < 3; ef++) {
                for (uint8_t ii = 0; ii < 32; ii++) {
                  written = flash.writeChar(flash_addr + ii, buf[ii], true);
                  if (!written) break;
                }

                if (written) break;
              }

              //---------------------------------------------------------------------------------------------------------------

              f_number++;

            } else {

              not_saved_anzahl_pwd++;

            }
          } else {
            ignore_number++;
          }
        }
      }
    }

    delta_msecs = millis() - p_msecs;
    p_msecs = millis();

#ifdef SERIAL_DEBUG_read_sd
    Serial.print("Time to read this file: ");
    Serial.println(delta_msecs);

    //Serial.println();
    //Serial.println();
#endif

    strcpy(save_fname, fno.fname);
    save_dj_index = dj.index;

#ifdef SERIAL_DEBUG_read_sd_x
    Serial.println(F("1443 fname: "));
    Serial.print(fno.fname);
    Serial.print(" - ");
    Serial.print(pfs_result);
    Serial.println();
    //print_dj(dj);
#endif

    select_ss(SD_SS);

    do {

      pfs_result = pf_readdir(&dj, &fno);

#ifdef SERIAL_DEBUG_read_sd
      Serial.println(F("1458 fname: "));
      Serial.print(fno.fname);
      Serial.print(" - ");
      Serial.print(pfs_result);
      Serial.println();
      //print_dj(dj);
#endif

    } while ((!fno.fsize || fno.fname[0] == '.' || fno.fname[0] == '_' || non_ascii_in_string(fno.fname)) && dj.sect);    // mod 4.2 - skip hidden files
    // mod04.04 - consider '_' given by PetitFS as 1st char in name

    /* the following lines below were inserted to compensate not understanded behaviour:
       after reading a long file (byte by byte) it was observed that data are not correctly
       returned therefore the dir will be rewind and files already processed will be skipped
       without any processing and on this way - without longer delay - the next file will
       be requested anew to continue further processing - it works for now but has to be replaced
       by corresponding modification after the true reason was found and corrected !!!
    */

    if ((!strlen(fno.fname) || non_ascii_in_string(fno.fname)) && (is_typefile(save_fname, ".bin") || is_typefile(save_fname, ".bip") || is_typefile(save_fname, ".bik"))) {
      dir_rewind(&dj);
      while (dj.index != save_dj_index) {
        pfs_result = pf_readdir(&dj, &fno);
      }

      // pfs_result = pf_readdir(&dj, &fno);      // mod 4.2 - skip hidden files

      do {

        pfs_result = pf_readdir(&dj, &fno);

      } while (!fno.fsize || fno.fname[0] == '.' || fno.fname[0] == '_');     // mod04.04 - consider '_' given by PetitFS as 1st char in name
    }


#ifdef SERIAL_DEBUG_read_sd
    Serial.println(F("1494 fname: "));
    Serial.print(fno.fname);
    Serial.print(" - ");
    Serial.print(pfs_result);
    Serial.println();
    //print_dj(dj);
#endif

    if ((f_number + count_font_files + not_saved_anzahl_pwd + ignore_number) % VISIBLE_PASSWORDS == 0) {
      //select_ss(TFT_SS);

      if (f_number < PWD_Index_Size) {
        //tft.fillScreen(SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK);
        //tftFillScreen(SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK);
        background_color = SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK;
        tftFillScreen(background_color);
      }
      else {
        //tft.fillScreen(TFT_SLARK_ORANGE ^ TFT_INVERT_MASK);
        //tftFillScreen(TFT_SLARK_ORANGE ^ TFT_INVERT_MASK);
        background_color = TFT_SLARK_ORANGE ^ TFT_INVERT_MASK;
        tftFillScreen(background_color);
      }

      select_ss(SD_SS);

    }
  } while (dj.sect);

#ifdef SERIAL_DEBUG_read_sd
  //  Serial.print(F("Number of found files: "));
  //  Serial.println(f_number);

  Serial.print("Time to read all files: ");
  Serial.println(millis() - msecs);

  Serial.println();
  Serial.println();
#endif

  return f_number;
}


bool non_ascii_in_string(char *name_string) {
  while (name_string[0]) {
#ifdef SERIAL_DEBUG_read_sd_non_ascii
    Serial.print("1535 char: ");
    Serial.println(name_string[0]);
#endif
    if (name_string[0] < ' ' || name_string[0] > '~') return true;
    name_string++;
  }

  return false;
}


void save_pwd_data(long flen, char *filename) {

  char buf[32];
  char pwd_buf[PWD_Size];
  char *ptr_pwd_buf = &pwd_buf[0];
  long flash_addr;
  int f_number;
  bool written;

  uint16_t p[4] = { 0, 0, 0, 0 };

  uint16_t j = 0;

  PFS_UINT nr;
  FRESULT pfs_result;

  for (uint16_t j = 0; j < sizeof(buf); j++) buf[j] = CHAR_ZERO;
  for (uint16_t j = 0; j < sizeof(pwd_buf); j++) pwd_buf[j] = CHAR_ZERO;

  select_ss(SD_SS);

  pfs_result = pf_read(pwd_buf, sizeof(pwd_buf), &nr);

  while (j < flen && j < nr & (pwd_buf[j] == CHAR_NL || pwd_buf[j] == CHAR_CR)) {
    p[0]++;
    j++;
  }

  p[1] = p[0];

  while (j < flen && j < nr & pwd_buf[j] != CHAR_NL && pwd_buf[j] != CHAR_CR) {
    p[1]++;
    j++;
  }

  p[2] = p[1];

  while (j < flen && j < nr && (pwd_buf[j] == CHAR_NL || pwd_buf[j] == CHAR_CR)) {
    p[2]++;
    j++;
  }

  p[3] = p[2];

  while (j < flen && j < nr && pwd_buf[j] != CHAR_NL && pwd_buf[j] != CHAR_CR) {
    p[3]++;
    j++;
  }

  if (p[3] < PWD_Size) {
    if (p[0] > 0) {
      for (uint8_t i = 0; i < p[1] - p[0]; i++) pwd_buf[i] = pwd_buf[p[0] + i];

      for (uint8_t i = 0; i < p[3] - p[2]; i++) buf[i] = pwd_buf[p[2] + i];

      pwd_buf[p[1] - p[0]] = CHAR_ZERO;
      buf[p[3] - p[2]] = CHAR_ZERO;
    } else {
      for (uint8_t i = 0; i < p[3] - p[2]; i++) buf[i] = pwd_buf[p[2] + i];

      pwd_buf[p[1]] = CHAR_ZERO;
      buf[p[3] - p[2]] = CHAR_ZERO;
    }

  } else {

    select_ss(SD_SS);

    pfs_result = pf_lseek(p[2]);

    pfs_result = pf_read(buf, sizeof(buf), &nr);

    j = 0;

    if (nr < PWD_Index_Text_size) {
      p[0] = 0;

      while (j < nr && (buf[j] == CHAR_NL || buf[j] == CHAR_CR)) {
        p[0]++;
        j++;
      }

      p[1] = p[0];

      while (j < nr & buf[j] != CHAR_NL && buf[j] != CHAR_CR) {
        p[1]++;
        j++;
      }

      if (p[0] > 0) {
        for (uint8_t i = 0; i < p[1] - p[0]; i++) buf[i] = buf[p[0] + i];

        buf[p[1] - p[0]] = CHAR_ZERO;
      } else {
        buf[p[1]] = CHAR_ZERO;
      }
    }
  }

  flash_addr = PWD_Start + (uint32_t) PWD_Size * f_number;

  encrypt_pwd(pwd_buf, pwd_buf, true);

  select_ss(FLASH_SS);

  for (byte ef = 0; ef < 3; ef++) {

    written = flash.writeCharArray(flash_addr, ptr_pwd_buf, 256, false);

    if (written) {
      break;
    }
  }

  int name_length;

  if (buf[0] == CHAR_ZERO) {
    name_length = strlen(filename);
    for (int i = 0; i < name_length - 4; i++) buf[i] = filename[i];
  } else {
    name_length = strlen(buf);
  }

  buf[name_length - 4] = 0;
  buf[30] = name_length - 4;
  buf[31] = f_number;

  flash_addr = PWD_Index + (PWD_Index_Text_size + PWD_Index_Header) * f_number;

  select_ss(FLASH_SS);

  for (byte ef = 0; ef < 3; ef++) {
    for (uint8_t ii = 0; ii < 32; ii++) {
      written = flash.writeChar(flash_addr + ii, buf[ii], true);
      if (!written) break;
    }

    if (written) break;
  }
}


void test_select_this_pwd(int f_number) {

  char current_pwd_name[32] = {0};
  char current_pwd_data[PWD_Size + 1] = {0};
  long pwd_addr;

  select_ss(FLASH_SS);

  for (int pidx = 0; pidx < f_number; pidx++) {
    pwd_addr = PWD_Index + (uint32_t) 32 * (uint32_t) pidx;

    for (int i = 0; i < 32; i++) current_pwd_name[i] = flash.readChar(pwd_addr + i);

#if (defined SERIAL_DEBUG_read_sd) || defined (SERIAL_DEBUG_select_pwd)
    Serial.println(F("1702 current_pwd_name read: "));
    Serial.print(pidx);
    Serial.print(" - ");
    Serial.print(pwd_addr);
    Serial.print(" - ");
    Serial.print("(");
    Serial.print(pwd_addr, HEX);
    Serial.print(F(") PWD-N: "));
    Serial.print(current_pwd_name);
    Serial.println();
    Serial.print("current_pwd_name: ");
    for (int i = 0; i < 32; i++) {
      Serial.print((uint8_t) current_pwd_name[i] < 0x10 ? " 0" : " ");
      Serial.print((uint8_t) current_pwd_name[i], HEX);
    }
    Serial.println();
#endif

    pwd_addr = PWD_Start + (uint32_t) PWD_Size * (uint8_t) pidx;
#ifdef SERIAL_DEBUG_read_sd
    Serial.println(F("1722 pwd_addr read: "));
    Serial.print(pwd_addr);
    Serial.println();
#endif
    for (int i = 0; i < PWD_Size; i++) current_pwd_data[i] = flash.readChar(pwd_addr + i);

#ifdef SERIAL_DEBUG_read_sd
    Serial.println(F("1729 current_pwd_data read: "));
    Serial.print(F("PWD-D: "));
    //Serial.print(current_pwd_data);
    //Serial.println();
    Serial.print("current_pwd_data: ");
    for (int i = 0; i < PWD_Size; i++) {
      Serial.print((uint8_t) current_pwd_data[i] < 0x10 ? " 0" : " ");
      Serial.print((uint8_t) current_pwd_data[i], HEX);
    }
    Serial.println();
#endif

#ifdef USE_CRYPT
#ifdef SERIAL_DEBUG_read_sd
    Serial.println(F("1743 current_pwd_data encrypted: "));
    Serial.print(F("PWD-E: "));
    for (int i = 0; i < PWD_Size; i++) {
      Serial.print((uint8_t) current_pwd_data[i] < 0x10 ? " 0" : " ");
      Serial.print((uint8_t) current_pwd_data[i], HEX);
    }
    Serial.println();
#endif

    encrypt_pwd(current_pwd_data, current_pwd_data, false);

#ifdef SERIAL_DEBUG_read_sd
    Serial.println(F("1755 current_pwd_data plain: "));
    Serial.print(F("PWD-Ph: "));
    for (int i = 0; i < PWD_Size; i++) {
      Serial.print((uint8_t) current_pwd_data[i] < 0x10 ? " 0" : " ");
      Serial.print((uint8_t) current_pwd_data[i], HEX);
    }
    Serial.println();

    Serial.println(F("1763 current_pwd_data read: "));
    Serial.print(F("PWD-Pp: "));
    Serial.print(current_pwd_data);
    Serial.println();
#endif
#endif

#ifdef USE_KBRD
    //for (int i = 0; i < PWD_Size; i++) current_pwd_data[i] = usToDE[current_pwd_data[i]];
    for (int i = 0; i < PWD_Size; i++) current_pwd_data[i] = flash.readChar(Keyboard_Layout_Start + current_pwd_data[i]);

#ifdef SERIAL_DEBUG_read_sd
    Serial.println(F("1775-c current_pwd_data mapped: "));
    Serial.print(F("PWD-K: "));
    for (int i = 0; i < PWD_Size; i++) {
      Serial.print((uint8_t) current_pwd_data[i] < 0x10 ? " 0" : " ");
      Serial.print((uint8_t) current_pwd_data[i], HEX);
    }
    Serial.println();
    Serial.println();
    Serial.println();
#endif
#endif

#ifdef SERIAL_DEBUG_read_sd
    Serial.println();
    Serial.println("Done");
#endif
  }
}


void open_file(char *filename) {
  FRESULT pfs_result;

  //select_ss(SD_SS);

  for (byte ef = 0; ef < 3; ef++) {

    select_ss(SD_SS);

    pfs_result = pf_open(filename);

    if (pfs_result) {
#ifdef SERIAL_DEBUG_read_sd
      Serial.print("1808 open pfs_result: ");
      Serial.println(pfs_result);
#endif
    } else {
#ifdef SERIAL_DEBUG_read_sd
      Serial.print("1813 open pfs_result: ");
      Serial.println(pfs_result);
#endif
      break;
    }
  }
}

bool is_typefile(char *filename, char *file_ext) {
  return strstr(strlwr(filename + (strlen(filename) - 4)), file_ext);
}

/*
  void fill_rect(uint8_t x) {
  if (x)  tft.fillRect(118 - 16 * ((f_number + count_font_files + not_saved_anzahl_pwd + ignore_number) % VISIBLE_PASSWORDS), 112 + x, 2, 2, TFT_WHITE);
  else  tft.fillRect(118 - 16 * ((f_number + count_font_files + not_saved_anzahl_pwd + ignore_number) % VISIBLE_PASSWORDS), 112, 2, 40, TFT_SLARK_SKYBLUE);
  }
*/

bool save_fonts_data(uint32_t file_size, uint32_t font_flash_addr, uint32_t font_step, uint32_t reserved_size, char * ptr_pwd_buf) {

  bool saving_done = true;
  int i, ii;
  uint8_t font_step_size = 0;
  PFS_UINT nr;
  FRESULT pfs_result;
  char rotor[4] = {0x2d, 0x5c, 0x7c, 0x2f};
  uint32_t ir = 0;


#ifdef SERIAL_DEBUG_read_sd0xa0xa
  Serial.begin(9600);
  while (!Serial);

  Serial.println(F("1847 Serial save_fonts_data display started"));
#endif

  select_ss(FLASH_SS);

  for (i = 0; i < reserved_size; i++) {
    flash.eraseSector(font_flash_addr + i * 4096);
  }

#ifdef SERIAL_DEBUG_read_sd_font
  Serial.print("1857 font_step written: ");
  Serial.print(font_flash_addr);
  Serial.print(" # ");
  Serial.print(font_step);
  Serial.print(" # ");
  Serial.print(i);
  Serial.print(" # ");
  Serial.print(flash.readChar(font_step));
  Serial.print(" # ");
  Serial.print(flash.readChar(font_step + 1));
  Serial.print(" # ");
  Serial.print(flash.readChar(font_step + 2));
  Serial.print(" # ");
  Serial.print(flash.readChar(font_step + 3));
  Serial.println();
#endif

  for (i = 0; i < file_size; i += 256) {

    select_ss(SD_SS);

    pfs_result = pf_read(ptr_pwd_buf, 256, &nr);

    if (font_step_size) {

    } else {
      for (ii = 1; ii < 256; ii++) {
        if (ptr_pwd_buf[ii]) {

          font_step_size = ii;

          select_ss(FLASH_SS);

          flash.writeChar(font_step, font_step_size, true);
          break;
        }
      }
    }

#ifdef FONT_SAVE_PROGRESS
    if ((i % 0x100) == 0 && i) {
      select_ss(TFT_SS);

      tft.fillRect(
        118 - 16 * ((f_number + count_font_files + not_saved_anzahl_pwd + ignore_number) % VISIBLE_PASSWORDS),
        0,
        8,
        8,
        (ir % 2) ? TFT_SLARK_SKYBLUE : TFT_WHITE);

      ir++;
    }

#endif

    select_ss(FLASH_SS);

    if (flash.writeCharArray(font_flash_addr, ptr_pwd_buf, nr, false)) {
      font_flash_addr += nr;
    }
  }

#ifdef SERIAL_DEBUG_read_sd_font
  Serial.print("1920 data written: ");
  Serial.print(saving_done);
  Serial.print(" # ");
  Serial.print(file_size);
  Serial.print(" # ");
  Serial.print(font_flash_addr);
  Serial.print(" # ");
  Serial.print(font_step);

  select_ss(FLASH_SS);

  Serial.print(" # ");
  Serial.print((uint8_t) flash.readChar(font_step++));
  Serial.print(" # ");
  Serial.print((uint8_t) flash.readChar(font_step++));
  Serial.print(" # ");
  Serial.print((uint8_t) flash.readChar(font_step++));
  Serial.print(" # ");
  Serial.print((uint8_t) flash.readChar(font_step++));
  Serial.println();
#endif

  return saving_done;
}

bool save_fonts_data_blocked(long file_size, long font_flash_addr, long font_step, byte reserved_size, char *pwd_buf) {

  bool saving_done = true;
  byte skip_1st_non_zero_byte = 0;
  char aux_byte;
  char * ptr_aux_byte = &aux_byte;
  int i;
  uint8_t font_step_size;
  //char current_string[8];
  PFS_UINT nr;
  FRESULT pfs_result;


#ifdef SERIAL_DEBUG_read_sd
  Serial.begin(9600);
  //aes.do_aes_encrypt(plain,sizeof(plain),cipher,key,bits,iv);
  while (!Serial);

  Serial.println(F("1963 Serial save_fonts_data display started"));
#endif

  select_ss(FLASH_SS);

  for (i = 0; i < reserved_size; i++) {
    flash.eraseSector(font_flash_addr + i * 4096);
  }

#ifdef SERIAL_DEBUG_read_sd_font
  Serial.print("1973 font_step written: ");
  Serial.print(font_flash_addr);
  Serial.print(" # ");
  Serial.print(font_step);
  Serial.print(" # ");
  Serial.print(i);
  Serial.print(" # ");
  Serial.print(flash.readChar(font_step));
  Serial.print(" # ");
  Serial.print(flash.readChar(font_step + 1));
  Serial.print(" # ");
  Serial.print(flash.readChar(font_step + 2));
  Serial.print(" # ");
  Serial.print(flash.readChar(font_step + 3));
  Serial.println();
#endif

  for (i = 0; i < file_size; i++) {

    select_ss(SD_SS);

    pfs_result = pf_read(ptr_aux_byte, 1, &nr);

    if (pfs_result) {
      strcpy(display_string, "pf_read");
      errorHalt(display_string);
    }

    //    if (skip_1st_non_zero_byte == 0 && aux_byte != CHAR_ZERO) {
    if (skip_1st_non_zero_byte == 0 && aux_byte) {
      skip_1st_non_zero_byte = 1;
    } else {
      //      if (skip_1st_non_zero_byte == 1 && aux_byte != CHAR_ZERO) {
      if (skip_1st_non_zero_byte == 1 && aux_byte) {

        font_step_size = i;

        select_ss(FLASH_SS);

        //flash.writeChar(font_step, (char) i, true);
        flash.writeChar(font_step, font_step_size, true);

#ifdef SERIAL_DEBUG_read_sd_font
        Serial.print("2016 font_step written: ");
        Serial.print(font_flash_addr);
        Serial.print(" # ");
        Serial.print(font_step);
        Serial.print(" # ");
        Serial.print(font_step_size);
        Serial.println();
#endif
        skip_1st_non_zero_byte = 2;
      }
    }

#ifdef FONT_SAVE_PROGRESS
    if ((i % 0x200) == 0 && i) {
      select_ss(TFT_SS);
      tft.fillRect(118 - 16 * ((f_number + count_font_files + not_saved_anzahl_pwd + ignore_number) % VISIBLE_PASSWORDS), 112 + (uint16_t)(i / 0x100), 2, 2, TFT_WHITE);
    }
#endif

    select_ss(FLASH_SS);

    if (flash.writeChar(font_flash_addr, aux_byte, true)) {
      font_flash_addr++;
    } else {
      saving_done = false;
    }
  }

#ifdef SERIAL_DEBUG_read_sd_font
  Serial.print("2045 data written: ");
  Serial.print(saving_done);
  Serial.print(" # ");
  Serial.print(file_size);
  Serial.print(" # ");
  Serial.print(font_flash_addr);
  Serial.print(" # ");
  Serial.print(font_step);

  select_ss(FLASH_SS);

  Serial.print(" # ");
  Serial.print((uint8_t) flash.readChar(font_step++));
  Serial.print(" # ");
  Serial.print((uint8_t) flash.readChar(font_step++));
  Serial.print(" # ");
  Serial.print((uint8_t) flash.readChar(font_step++));
  Serial.print(" # ");
  Serial.print((uint8_t) flash.readChar(font_step++));
  Serial.println();
#endif

  return saving_done;
}


bool save_fonts_data_256(long file_size, long font_flash_addr, long font_step, byte reserved_size, char *pwd_buf) {

  bool saving_done = true;
  byte skip_1st_non_zero_byte = 0;
  char aux_byte;
  char * ptr_aux_byte = &aux_byte;
  int i;
  uint8_t ii;

  PFS_UINT nr;
  FRESULT pfs_result;

  select_ss(FLASH_SS);

  for (i = 0; i < reserved_size; i++) {
    flash.eraseSector(font_flash_addr + i * 4096);
  }

  for (i = 0; i < file_size; i += 256) {

    select_ss(SD_SS);

    //pfs_result = pf_read(ptr_aux_byte, 256, &nr);
    pfs_result = pf_read(pwd_buf, 256, &nr);
    Serial.println(nr);

    if (pfs_result) {
      strcpy(display_string, "pf_read");
      errorHalt(display_string);
    }

    if (i == 0) {
      for (uint8_t ij; ij < 256; ij++) {
        if (skip_1st_non_zero_byte == 0 && aux_byte != CHAR_ZERO) {
          skip_1st_non_zero_byte = 1;
        } else {
          if (skip_1st_non_zero_byte == 1 && aux_byte != CHAR_ZERO) {

            select_ss(FLASH_SS);
            Serial.println(i);

            //flash.writeChar(font_step, (char) i, true);
            flash.writeChar(font_step, (char) ij, true);
            Serial.println(i);

            break;
          }
        }
      }
      continue;
    }

    select_ss(FLASH_SS);

    if (flash.writeCharArray(font_flash_addr, pwd_buf, nr, true)) {
      font_flash_addr += 256;
      Serial.print("2127 writeCharArray");
      Serial.print(i);
      Serial.print(" - ");
      Serial.print(font_flash_addr);
      Serial.println();
    } else {
      Serial.print("2133 writeCharArray");
      saving_done = false;
    }
  }

  select_ss(FLASH_SS);

  return saving_done;
}

void dump_small_fonts_2_serial() {

  uint8_t buf[32];

  for (uint32_t j = FD_Start; j < FD_Start_1 - FD_Start; j += 32) {

    flash.readCharArray(j, buf, 32);

    dump_byte_array(buf, 32);
  }
}

void dump_big_fonts_2_serial() {

  uint8_t buf[32];

  for (uint32_t j = FD_Start; j < SPI_FLASH_SIZE - FD_Start_1; j += 32) {

    flash.readCharArray(j, buf, 32);

    dump_byte_array(buf, 32);
  }
}


void dump_byte_array(byte * buffer, int bufferSize) {
  Serial.println();
  Serial.print("bufferSize: "); Serial.println(bufferSize);
  Serial.print("dump_byte_array: ");
  for (int i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  Serial.println();
}

void tftFillScreen(uint16_t fillcolor) {
  select_ss(TFT_SS);
  tft.fillScreen(fillcolor);
}



void displayString(char *charString, uint16_t cursor_x, uint16_t cursor_y, uint16_t textcolor) {

#ifdef SERIAL_DEBUG_displayString_o
  long start_time_for_letter;
  long end_time_for_letter;
#endif

  if (textcolor != (TFT_BLACK + 1)) {
    //tft.fillScreen(textcolor);
    background_color = textcolor;
    tftFillScreen(background_color);
  }

#ifdef SERIAL_DEBUG_displayString_o
  start_time_for_letter = micros();
#endif

  int i = 0;
  int xpos = cursor_x;
  int ypos = cursor_y;

  while (charString[i] && xpos < (160 - 10)) {
    xpos += displayletter(charString[i++], xpos, ypos, TFT_WHITE);
  }

#ifdef SERIAL_DEBUG_displayString_o
  end_time_for_letter = micros() - start_time_for_letter;
#endif

}

void xchange_background(uint8_t *pixel_color, uint16_t textcolor, uint16_t pc_size) {
  uint8_t hByte = (background_color & 0xff00) / 0x100;
  uint8_t lByte = background_color & 0xff;
  for (uint16_t ip = 0; ip < pc_size; ip += 2) {
    if (pixel_color[ip] == 0 && pixel_color[ip + 1] == 0) {
      pixel_color[ip] = hByte;
      pixel_color[ip + 1] = lByte;
    }
    //    else {
    //      pixel_color[ip] = TFT_SLARK_RED  ^ TFT_INVERT_MASK;
    //    }
  }
}

int8_t displayletter(char letter, uint16_t cursor_y, uint16_t cursor_x, uint16_t textcolor) {
  long addr2read;
  uint8_t readword[2];
  uint8_t res_read;

  select_ss(FLASH_SS);

  addr2read = (uint32_t) (smallbig == 1 ? FD_Start_small_gray : FD_Start_big_gray) + (letter - FIRST_CHAR_IN_FONT_DATA) * 2;

  if (flash.readCharArray(addr2read, readword, 2)) {
    //addr2read = (uint32_t) FD_Start_small_gray + readword[0] * 0x100 + readword[1];
    uint32_t delta = (uint32_t)readword[0] * 0x100 + (uint32_t) readword[1];
    addr2read = (uint32_t) (smallbig == 1 ? FD_Start_small_gray : FD_Start_big_gray) + delta;
    if (smallbig == 2 && letter > 'Z' && delta < 0x10000)
    {
      addr2read += (uint32_t) 0x10000;
    }
  }

  int8_t no_of_lines = flash.readByte(addr2read++);

  if (false) {
    uint8_t pixel_color[smallbig * 2 * 16 * (no_of_lines + 1)] = {0};     // to assure reading in case of odd number of lines

    flash.readCharArray(addr2read, pixel_color, smallbig * 2 * 16 * (no_of_lines + 1));

    select_ss(TFT_SS);        // mod04.05 - SPI select once for a row - do it before the loop

    tft.setAddrWindow(cursor_x, cursor_y, cursor_x + smallbig * 16 - 1, cursor_y + no_of_lines - 1);

    tft.pushColors(pixel_color, smallbig * 16 * no_of_lines);

    select_ss(FLASH_SS);
  } else {
    uint8_t mSize = (no_of_lines + 1) / 2;
    uint8_t pixel_color[smallbig * 32 * mSize] = {0}; // to assure reading in case of odd number of lines

    for (uint8_t m = 0; m < 2; m++) {
      flash.readCharArray(addr2read, pixel_color, smallbig * 32 * mSize);

      //if (textcolor != TFT_BLACK)
      xchange_background(pixel_color, textcolor, smallbig * 32 * mSize);

      select_ss(TFT_SS);        // mod04.05 - SPI select once for a row - do it before the loop

      tft.setAddrWindow(cursor_x, cursor_y + mSize * m, cursor_x + smallbig * 16 - 1, cursor_y + mSize + (mSize - 1) * m - 1);

      tft.pushColors(pixel_color, smallbig * 16 * (mSize - m));

      addr2read += (uint32_t) (smallbig * 2 * 16 * mSize);

      select_ss(FLASH_SS);
    }
  }
  return no_of_lines + 1;         // lines are here the colons of char bitmap
}


int8_t debug_displayletter(char letter, uint16_t cursor_y, uint16_t cursor_x, uint16_t textcolor) {
  uint16_t cxs = cursor_x;
  uint16_t cys = cursor_y;
  long addr2read;
  uint32_t line[6] = {0L};
  long addr_delta_gray;
  uint8_t font_step, grey_ext;

  uint8_t readword[2];

#ifdef SERIAL_DEBUG_displayString_y
  long start_time_for_letter;
  long end_time_for_letter;
#endif

  int color = TFT_WHITE;
  int colorg = TFT_GRAY;

  if (smallbig == 1) {

#ifdef USE_GRAY_COLORS_4_SMALL_FONTS


    if (flash.readCharArray(FD_SMALL_STEP_gray, readword, 2)) {
      font_step = readword[0];
      grey_ext  = readword[1];
    }
#else
    if (flash.readCharArray(FD_SMALL_STEP, readword, 2)) {
      font_step = readword[0];
      grey_ext  = readword[1];
    }
#endif

#ifdef USE_GRAY_COLORS_4_SMALL_FONTS
    addr2read = FD_Start_small_gray + (letter - FIRST_CHAR_IN_FONT_DATA) * font_step;

#ifdef SERIAL_DEBUG_displayString_x
    Serial.println();
    Serial.print(F("2338 no_of_lines: "));
    Serial.print(FD_Start_small_gray, HEX);
    Serial.print(" - ");
    Serial.print(font_step, HEX);
    Serial.print(" - ");
    Serial.println(addr2read, HEX);
#endif

#else

    addr2read = FD_Start + (letter - FIRST_CHAR_IN_FONT_DATA) * font_step;

#ifdef SERIAL_DEBUG_displayString_x
    Serial.println();
    Serial.print(F("2352 no_of_lines: "));
    Serial.print(FD_Start, HEX);
    Serial.print(" - ");
    Serial.print(font_step, HEX);
    Serial.print(" - ");
    Serial.println(addr2read, HEX);
#endif

#endif

  } else {
    if (flash.readCharArray(FD_BIG_STEP, readword, 2)) {
      font_step = readword[0];
      grey_ext  = readword[1];
    }

    addr2read = (uint32_t) FD_Start_1 + (letter - FIRST_CHAR_IN_FONT_DATA) * font_step;

#ifdef SERIAL_DEBUG_displayString_x
    Serial.println();
    Serial.print(F("2372 no_of_lines: "));
    Serial.print(FD_Start_1, HEX);
    Serial.print(" - ");
    Serial.print(font_step, HEX);
    Serial.print(" - ");
    Serial.print(addr2read, HEX);
    Serial.print(" - ");
    Serial.println(letter);
#endif
  }

  addr_delta_gray = 96 * font_step;

  select_ss(FLASH_SS);

  int8_t no_of_lines = flash.readByte(addr2read++);

#ifdef SERIAL_DEBUG_displayString_xx
  Serial.println();
  Serial.print(F("2391 no_of_lines: "));
  Serial.print(letter);
  Serial.print(" - ");
  Serial.print(no_of_lines);
  Serial.print(" - ");
  Serial.println(addr2read, HEX);
#endif

  for (int8_t r = 0; r < smallbig; r++) {
    cursor_y = cys;

    for (int8_t i = 0; i < no_of_lines; i++) {

      if (i) cursor_y++;

#ifdef SERIAL_DEBUG_displayString_y
      start_time_for_letter =       micros();
#endif

      if (smallbig == 1) {
#ifdef SERIAL_DEBUG_displayString_xx
        Serial.print(F("2412 line: "));
        Serial.print(" small line addr 1:");
        Serial.println(addr2read, HEX);
#endif

#ifdef USE_GRAY_COLORS_4_SMALL_FONTS
        for (uint8_t l = 0; l < 6; l++) {
          if (flash.readCharArray(addr2read + addr_delta_gray * l + 2 * i, readword, 2)) {
            line[l] = readword[0] + readword[1] * 0x100;
          }
        }
#else
        for (uint8_t l = 0; l < 2; l++) {
          if (!grey_ext && flash.readCharArray(addr2read + addr_delta_gray * l + 2 * i, readword, 2)) {
            line[l] = readword[0] + readword[1] * 0x100;
          }
        }
#endif

#ifdef SERIAL_DEBUG_displayString_xx
        Serial.print(F("2432 line: "));
        Serial.print(" small line 2:");
        Serial.print(line_gl[0], HEX);
        Serial.print(" - ");
        Serial.print(line_gl[1], HEX);
        Serial.print(" - ");
        Serial.print(line_gl[2], HEX);
        Serial.print(" - ");
        Serial.print(line_gl[3], HEX);
        Serial.print(" - ");
        Serial.print(line_gl[4], HEX);
        Serial.print(" - ");
        Serial.println(line_gl[5], HEX);
#endif

      } else {
        if ( false) {
          if (flash.readCharArray(addr2read + 4 * i + r * 2, readword, 2)) {
            line[0] = readword[0] + readword[1] * 0x100;
          }
        }

        for (uint8_t l = 0; l < 6; l++) {
          if (flash.readCharArray(addr2read + addr_delta_gray * l + 4 * i + r * 2, readword, 2)) {
            line[l] = readword[0] + readword[1] * 0x100;
          }
        }

#ifdef SERIAL_DEBUG_displayString_xx
        Serial.print(F("2461 line: "));
        Serial.print(" small line 1:");
        Serial.println(line, HEX);
#endif
        if (false) {
          if (!grey_ext && flash.readCharArray(addr2read + 4 * i + r * 2, readword, 2)) {
            line[0] = readword[0] + readword[1] * 0x100;
          }
        }

#ifdef SERIAL_DEBUG_displayString_xx
        Serial.print(F("2472 line: "));
        Serial.print(" small line 2:");
        Serial.println(lineg, HEX);
#endif
      }

#ifdef SERIAL_DEBUG_displayString_y
      end_time_for_letter = micros() - start_time_for_letter;
      Serial.println();
      Serial.print(F("2481 line: "));
      Serial.print("flash.readByte");
      Serial.print(" - ");
      Serial.println(end_time_for_letter);
      Serial.println(); cursor_ycursor_y
#endif

#ifdef SERIAL_DEBUG_displayString_xx
      Serial.print(F("2489 line: "));
      Serial.print(i);
      Serial.print(" - ");
      Serial.print(line_gl[0], HEX);
      Serial.print(" - ");
      Serial.print(line_gl[1], HEX);
      Serial.print(" - ");
      Serial.print(line_gl[2], HEX);
      Serial.print(" - ");
      Serial.print(line_gl[3], HEX);
      Serial.print(" - ");
      Serial.print(line_gl[4], HEX);
      Serial.print(" - ");
      Serial.println(line_gl[5], HEX);
#endif

#ifdef SERIAL_DEBUG_displayString_y
      start_time_for_letter = micros();
#endif

      select_ss(TFT_SS);        // mod04.05 - SPI select once for a row - do it before the loop

      long linemask = 0x8000;

      cursor_x = cxs - r * 16;

      for (int8_t ix = 0; ix < 16; ix++) {
        if (smallbig == 1) {

#ifdef USE_GRAY_COLORS_4_SMALL_FONTS
          for (uint8_t l = 0; l < 6; l++) {
#ifdef SERIAL_DEBUG_displayString_xx
            Serial.print(F("2521 line: "));
            Serial.print(i);
            Serial.print(" - ");
            Serial.print(line_gl[0], HEX);
            Serial.print(" - ");
            Serial.print(l);
            Serial.print(" - ");
            Serial.println(line_gl[l], HEX);
#endif
            if (line[l] & linemask) tft.drawPixel(cursor_x, cursor_y, background_color ? font_colors_on_black[l] : font_colors_on_teal[l]);
          }
#else
          // mod04.08 - drawPixel has to be used; fastPixel/fastPixel2 insert some additional pixels
          //            as it seems in random locations -  drawPixel has some more calls of >>> spiWait17(); <<<
          for (uint8_t l = 0; l < 2; l++) {
            if (line[l] & linemask) tft.drawPixel(cursor_x, cursor_y, color);
            //else if (lineg & linemask) tft.drawPixel(cursor_x, cursor_y, colorg);
          }
#endif
        } else {
          for (uint8_t l = 0; l < 2; l++) {
            if (line[l] & linemask) tft.drawPixel(cursor_x, cursor_y, color);
            //else if (lineg & linemask) tft.drawPixel(cursor_x, cursor_y, colorg);
          }
        }
        cursor_x++;
        linemask >>= 1;
      }

      select_ss(RFID_SS);


#ifdef SERIAL_DEBUG_displayString_y
      end_time_for_letter = micros() - start_time_for_letter;
      Serial.println();
      Serial.print(F("2556 line: "));
      Serial.print("tft.fastPixel2");
      Serial.print(" - ");
      Serial.println(end_time_for_letter);
      Serial.println();
#endif
    }
  }

  select_ss(FLASH_SS);

  return no_of_lines + 1;
}


void display_number(byte number_of_digits, long number, long limit) {
  bool overlimit = limit > 0 && number >= (limit - 1);

  smallbig = 2;

  if (overlimit) {

    long an = limit;
    int nn = 1;

    while (an /= 10) nn++;

    char current_string[nn + 3];

    for (byte ap = 0; ap < number_of_digits; ap++) {
      current_string[(number_of_digits - 1) - ap] = (limit % 10) + '0';
      limit = limit / 10;
    }

    current_string[number_of_digits] = CHAR_ZERO;

    displayString(current_string, 10, 90, TFT_SLARK_ORANGE ^ TFT_INVERT_MASK);

    smallbig = 1;

    current_string[0] = '(';

    // set true below to see the end address of font in memory
    for (byte ap = 0; ap < nn; ap++) {
      current_string[(1 + nn - 1) - ap] = (number % 10) + '0';
      number = number / 10;
    }

    current_string[nn + 1] = ')';
    current_string[nn + 2] = CHAR_ZERO;

    displayString(current_string, 10, 60, (TFT_BLACK ^ TFT_INVERT_MASK) + 1);

  } else {
    char current_string[number_of_digits + 1];

    for (byte ap = 0; ap < number_of_digits; ap++) {
      current_string[(number_of_digits - 1) - ap] = (number % 10) + '0';
      number = number / 10;
    }

    current_string[number_of_digits] = CHAR_ZERO;

    displayString(current_string, 10, 90, TFT_DARKGREEN ^ TFT_INVERT_MASK);
  }

  smallbig = 2;

  delay(2000);
}


void display_number_new(byte number_of_digits, long number, long limit) {
  bool overlimit = (limit > 0) && (number >= (limit - 1));

  smallbig = 2;

  if (overlimit) {
    char current_string[2 * number_of_digits + 1];

    // set true below to see the end address of font in memory
    for (byte ap = 0; ap < number_of_digits; ap++) {
      current_string[(number_of_digits - 1) - ap] = (limit % 10) + '0';
      limit = limit / 10;
    }

    current_string[number_of_digits] = ' ';
    current_string[number_of_digits] = '(';

    // set true below to see the end address of font in memory
    for (byte ap = 0; ap < number_of_digits; ap++) {
      current_string[(number_of_digits - 1) - ap] = (number % 10) + '0';
      number = number / 10;
    }

    current_string[number_of_digits] = ')';
    current_string[number_of_digits] = CHAR_ZERO;

    displayString(current_string, 10, 90, TFT_SLARK_ORANGE ^ TFT_INVERT_MASK);
  } else {
    char current_string[number_of_digits + 1];

    // set true below to see the end address of font in memory
    for (byte ap = 0; ap < number_of_digits; ap++) {
      current_string[(number_of_digits - 1) - ap] = (number % 10) + '0';
      number = number / 10;
    }
    current_string[number_of_digits] = CHAR_ZERO;

    displayString(current_string, 10, 90, TFT_DARKGREEN ^ TFT_INVERT_MASK);
  }

  delay(2000);
}



SPISettings settingsA(4000000, MSBFIRST, SPI_MODE0);

void select_ss(uint8_t ss_select) {
  digitalWrite(SD_SS,    HIGH);
  digitalWrite(FLASH_SS, HIGH);
  digitalWrite(TFT_SS,   HIGH);
  digitalWrite(RFID_SS,  HIGH);

  SPI.endTransaction();

  switch (ss_select) {
    case SD_SS:     SPI.beginTransaction(settingsA); digitalWrite(SD_SS,    LOW); break;
    case FLASH_SS:  SPI.beginTransaction(settingsA); digitalWrite(FLASH_SS, LOW); break;
    case TFT_SS:    SPI.beginTransaction(settingsA); digitalWrite(TFT_SS,   LOW); break;
    case RFID_SS:   SPI.beginTransaction(settingsA); digitalWrite(RFID_SS,  LOW); break;
  }
}


/**
   Helper routine to display pin to show its actual value
*/
void write_pin_display() {

  select_ss(TFT_SS);
  //tft.fillRect(40, 40 + current_digit * 20, 40, 20, SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK);

  background_color = SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK;
  //tft.fillRect(52, 40 + current_digit * 20, 40, 20, SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK);
  tft.fillRect(52, 40 + current_digit * 20, 40, 20, background_color);

  displayletter('0' + pin_digits[current_digit], 40 + current_digit * 20, 60, TFT_WHITE ^ TFT_INVERT_MASK);

  old_pin = pin;
}

void pin_input() {
  lockup_background_called = false;
  smallbig = 2;




  //--------------------------------BUTTONS----------------------------------------------------------
  // Buttons nur nutzbar, wenn wir in einem Eingabemodus sind

  OneButton button_r_1(SWITCH_R_1, true); // Instanz Wippe rechts innen
  OneButton button_c_s(SWITCH_C, true); // Instanz Schalter auf Wippe
  OneButton button_l_1(SWITCH_L_1, true); // Instanz Wippe links innen

  button_r_1.attachClick(click_r_1);
  button_l_1.attachClick(click_l_1);
  button_c_s.attachClick(check_pin);

  //    for (byte jk = 0; jk < 4; jk++) displayletter('0', 40 + jk * 20, 60, TFT_WHITE ^ TFT_INVERT_MASK);

  while (Systemzustand == PIN_EINGABE) {

    button_r_1.tick();
    button_c_s.tick();
    button_l_1.tick();

    if (pin != old_pin) {
      if (lockup_background_called) {
        lockup_background_called = false;

        //select_ss(TFT_SS);
        //tft.fillScreen(SD_Card_inserted ? 0xee70 : TFT_BLACK ^ TFT_INVERT_MASK);
        //background_color = SD_Card_inserted ? 0xee70 : TFT_BLACK ^ TFT_INVERT_MASK;
        background_color = SD_Card_inserted ? TFT_SLARK_SKYBLUE ^ TFT_INVERT_MASK : TFT_BLACK ^ TFT_INVERT_MASK;
        tftFillScreen(background_color);
        //tftFillScreen(SD_Card_inserted ? 0xee70 : TFT_BLACK ^ TFT_INVERT_MASK);

        for (byte jk = 0; jk < 4; jk++) displayletter('0' + pin_digits[jk], 40 + jk * 20, 60, TFT_WHITE ^ TFT_INVERT_MASK);
      }

      write_pin_display();

      start_time_for_pin_input = millis();
      targetTime = 0;

    } else {
      if (millis() - start_time_for_pin_input > 20000) {
        lockup_background_called = true;
        lockup_background();
      }
    }
  }
}


//----Funktionen für Buttons --------------

void click_r_1() { //Die aktuelle Stelle der Pin hochzählen
  current_input = 1;
  pin_change();
}

void click_l_1() { //Die aktuelle Stelle der Pin hochzählen
  current_input = 2;
  pin_change();
}

void check_pin() {
  select_ss(FLASH_SS);

  byte b1 = flash.readByte(PIN_valid_value);
  byte b2 = flash.readByte(PIN_valid_value + 1);

  int saved_pin = b1 * 0x100 + b2;

#ifdef SERIAL_DEBUG
  Serial.println(F("2778 inside 0f check pin: "));
  Serial.print(F("pin: "));
  Serial.print(pin);
  Serial.print(F(" - saved_pin: "));
  Serial.print(saved_pin);
#endif

  //--------------------------PIN Prüfung!-----------------------------
  if (SD_Card_inserted || pin == saved_pin) {
    Systemzustand = PIN_KORREKT;
  }
}

void pin_change() {
  if (old_input == 0) {
    old_input = current_input;
  }

  if (current_input != old_input) {
    current_digit++;
    old_input = current_input;
  }

  if (current_digit >= PIN_length) current_digit = 0;

  pin_digits[current_digit] = pin_digits[current_digit] + 1;

  if (pin_digits[current_digit] > 9) pin_digits[current_digit] = 0;

  pin = 0;

  for (byte i = 0; i < PIN_length; i++) {
    pin = pin * 10 + pin_digits[i];
  }
}


//--------------------------------RFID Leser----------------------------------------------------------

MFRC522::MIFARE_Key key; // RFID Schlüssel default HEx 00-00-00-00-00-00
MFRC522::StatusCode status;
MFRC522 mfrc522(RFID_RST, RFID_SS); // Instanz MFRC522

byte new_key[6] = RFID_Own_Key;

byte rfid_buffer[CRYPT_KEY_SIZE];
byte bsize = sizeof (rfid_buffer);

#define MFRC522_WITH_DEBUG

#ifdef MFRC522_WITH_DEBUG

void read_chip_key() {

  select_ss(RFID_SS);

#ifdef SERIAL_DEBUG_MFRC522
  Serial.begin(9600);
  while (!Serial);

  Serial.println(F("3339 (Re)Customise started"));
#endif

#ifdef SERIAL_DEBUG_MFRC522
  Serial.println(F("3343 read_chip_key"));
  Serial.println(F("3344 entered"));
#endif

  //    void activateRec(MFRC522 mfrc522);
  //    void clearInt(MFRC522 mfrc522);
  //    byte readCardData[7];

  // assign values for a desired key
  for (byte i = 0; i < 6; i++) key.keyByte[i] = new_key[i];

  mfrc522.PCD_Init(); // Init MFRC522

#ifdef SERIAL_DEBUG_MFRC522
  Serial.print(F("3357 PCD_Init passed: "));
  Serial.println();
#endif

  while (Systemzustand == RFID_LESEN) {

    select_ss(RFID_SS);

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {

#ifdef SERIAL_DEBUG_MFRC522
      Serial.print(F("3368 MIFARE_Write() sak: "));
      Serial.println(mfrc522.uid.sak);
#endif
      /*
         Ultralight MFRC has no protection to read data and to read Ultralight C one needs another reader PN532
         with MFRC522 it doeas not work
         Hence the version below is only for Ultralight and NOT for Ultralight C !!! both identified by sak = 0
      */
      if (mfrc522.uid.sak == 0) {
        byte blockAddr = 6;

        status = mfrc522.MIFARE_Read(blockAddr, rfid_buffer, &bsize);

        for (byte i = 0; i < 6; i++) {
          if (rfid_buffer[i] != new_key[i]) {
            for (byte i = 0; i < 4; i++) rfid_buffer[i] = new_key[i];

            // Write block
            status = mfrc522.MIFARE_Write(blockAddr++, rfid_buffer, 16);

            if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
              Serial.print(F("MIFARE_Write() failed: "));
              Serial.println(mfrc522.GetStatusCodeName(status));
#endif
            }

            for (byte i = 4; i < 6; i++) rfid_buffer[i - 4] = new_key[i];

            for (byte i = 6; i < 8; i++) rfid_buffer[i - 4] = 0;

            // Write block
            status = mfrc522.MIFARE_Write(blockAddr++, rfid_buffer, 16);

            if (status != MFRC522::STATUS_OK) {

#ifdef SERIAL_DEBUG_MFRC522
              Serial.print(F("MIFARE_Write() failed: "));
              Serial.println(mfrc522.GetStatusCodeName(status));
#endif
            }

            randomSeed(analogRead(NOT_CONNECTED_ANALOG_PIN_4_RANDOM_SEED));

            for (byte p = 0; p < 8; p++) {
              for (byte i = 0; i < 4; i++) rfid_buffer[i] = random(256);

              status = mfrc522.MIFARE_Write(blockAddr++, rfid_buffer, 16);

              if (status != MFRC522::STATUS_OK) {

#ifdef SERIAL_DEBUG_MFRC522
                Serial.print(F("MIFARE_Write() failed: "));
                Serial.println(mfrc522.GetStatusCodeName(status));
#endif
              }
            }

            break;
          }
        }
        read_known_rfid_chip();
      } else {  // RFID classic

#ifdef SERIAL_DEBUG_MFRC522
        Serial.println(F("3433 PCD_Authenticate: "));
        Serial.print(F("Key: "));
        dump_byte_array(key.keyByte, 6);
        Serial.println();
        //if (i == 2) mfrc522.PICC_DumpToSerial(&(mfrc522.uid));  // this line cannot be executed because of changed A_Key
#endif

        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 0, &key, &(mfrc522.uid));

        if (status != MFRC522::STATUS_OK) {

          if (SD_Card_inserted) { // customize new rfid chip or recustomize one already known
            customise_new_rfid_chip();
          } else {
#ifndef QUIET
            displayString("RFID 1", 30, 50, TFT_SLARK_RED);
            display_number(1, 1, 1);
#endif
            //while (true);
#ifdef RFID_READ_RESET
            resetFunc();
#else
            return;
#endif
          }
        } else {

#ifdef SERIAL_DEBUG_MFRC522
          Serial.print(F("2936 PCD_Authenticate passed: "));
          Serial.println(mfrc522.GetStatusCodeName(status));
#endif

          read_known_rfid_chip();
        }
      }
    }
  }
}

void read_known_rfid_chip() {
  byte crypt_key_part[2];

  if (mfrc522.uid.sak != 0) {
    byte crypt_key_part[2] = {1, 2};
  } else {
    byte crypt_key_part[2] = {8, 12};
  }

  for (byte i = 1; i <= 2; i++) {

#ifdef SERIAL_DEBUG_MFRC522
    Serial.println(F("2959 before MIFARE_Read(): "));
    Serial.print(F("i: "));
    Serial.print(i);
    Serial.println();
    //if (i == 2) mfrc522.PICC_DumpToSerial(&(mfrc522.uid));  // this line cannot be executed because of changed A_Key
#endif

    status = mfrc522.MIFARE_Read(i, rfid_buffer, &bsize);

    if (status != MFRC522::STATUS_OK) {

#ifdef SERIAL_DEBUG_MFRC522
      Serial.print(F("2971 MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      Serial.println(F("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?"));
#endif

#ifndef QUIET
      displayString("RFID 3", 30, 50, TFT_SLARK_RED);
      display_number(1, 2, 2);
#endif

      //while (true);
#ifdef RFID_READ_RESET
      resetFunc();
#else
      return;
#endif
    } else {

#ifdef SERIAL_DEBUG_MFRC522
      Serial.println(F("2987 MIFARE_Read() success: "));
      Serial.print(F("Block: "));
      Serial.println(i);
      dump_byte_array(rfid_buffer, 18);
      Serial.println();
      //if (i == 2) mfrc522.PICC_DumpToSerial(&(mfrc522.uid));  // this line cannot be executed because of changed A_Key
#endif

#ifdef USE_CRYPT
#ifdef SERIAL_DEBUG_MFRC522
      Serial.println(F("2997 MIFARE_Read() success: "));
      Serial.print(F("i: "));
      Serial.print(i);
      Serial.println();
      //if (i == 2) mfrc522.PICC_DumpToSerial(&(mfrc522.uid));  // this line cannot be executed because of changed A_Key
#endif

      for (byte j = 0; j < 16; j++) crypt_key[j + (i - 1) * 16] = rfid_buffer[j];

#ifdef SERIAL_DEBUG_MFRC522
      Serial.println(F("3007 MIFARE_Read() success: "));
      Serial.print(F("crypt_key: "));
      dump_byte_array(crypt_key, 16);
      Serial.println();
      //if (i == 2) mfrc522.PICC_DumpToSerial(&(mfrc522.uid));  // this line cannot be executed because of changed A_Key
#endif

#endif

    }
  }
#ifdef USE_CRYPT
#ifdef SERIAL_DEBUG_MFRC522
  Serial.println(F("3020 MIFARE_Read() success: "));
  Serial.print(F("crypt_key: "));
  dump_byte_array(crypt_key, CRYPT_KEY_SIZE);
  Serial.println();
  //if (i == 2) mfrc522.PICC_DumpToSerial(&(mfrc522.uid));  // this line cannot be executed because of changed A_Key
#endif
#endif

  if (SD_Card_inserted) Systemzustand = CODE_IM_GERAET;
  else Systemzustand = PWD_AUSWAHL;

#ifndef QUIET
  //select_ss(TFT_SS);
  displayString("RFID 2", 30, 50, TFT_SLARK_GREEN);
  delay(500);
#endif

}

//-------------------------------------------------------------------------

void customise_new_rfid_chip() {

  strcpy(display_string, "RFID new");
  displayString(display_string, 30, 50, TFT_SLARK_SKYBLUE);
  //delay(2000);


#ifdef SERIAL_DEBUG_MFRC522
  Serial.begin(9600);
  //aes.do_aes_encrypt(plain,sizeof(plain),cipher,key,bits,iv);
  while (!Serial);

  Serial.println(F("3053 (Re)Customise started"));
#endif

  // assign values for a manufacturer default key
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xff;

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 0, &key, &(mfrc522.uid));
  }

  if (status != MFRC522::STATUS_OK) {

#ifdef SERIAL_DEBUG_MFRC522
    Serial.print(F("3066 MIFARE_Write() twice failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    Serial.println(F("Not recognized chip - neither new nor previously used one"));
#endif

#ifndef QUIET
    displayString("RFID 5", 30, 50, TFT_SLARK_RED);
    display_number(1, 3, 3);

#endif
    //while (true); // error
#ifdef RFID_READ_RESET
    resetFunc();
#else
    return;
#endif

  } else {

    // for a virgin chip change the key to desired one and store a new generated encryption key

#ifdef SERIAL_DEBUG_MFRC522
    Serial.println(F("3084 PCD_Authenticate() success: "));

    Serial.println(F("Authenticated using key: "));
    dump_byte_array(key.keyByte, 6);
    Serial.println();
    Serial.println();
#endif

    status = mfrc522.MIFARE_Read(0, rfid_buffer, &bsize);

    if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.print(F("3096 MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      Serial.println(F("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?"));
#endif

#ifndef QUIET
      displayString("RFID 6", 30, 50, TFT_SLARK_RED);
      display_number(1, 4, 4);
#endif

      //while (true); // error
#ifdef RFID_READ_RESET
      resetFunc();
#else
      return;
#endif

    } else {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.println(F("3111 MIFARE_Read() success: "));
      Serial.print(F("Block: "));
      Serial.println("3113");
      dump_byte_array(rfid_buffer, 18);
      Serial.println();
#endif
    }

    generate_and_store_crypt_key();

    write_key_A_onto_rfid_chip();

  }
}

void generate_and_store_crypt_key() {

  randomSeed(analogRead(NOT_CONNECTED_ANALOG_PIN_4_RANDOM_SEED));

  // write random (256 bit) crypt key in 2 blocks on the rfid chip
  for (byte j = 1; j <= 2; j++) {

    for (byte i = 0; i < 16; i++) {
      rfid_buffer[i] = random(256);

#ifdef USE_CRYPT
      crypt_key[i + (j - 1) * 16] = rfid_buffer[i];
#endif
    }

#ifdef SERIAL_DEBUG_MFRC522
    Serial.println();
    Serial.println(F("3143 Prepared random key; first/second 16 bytes: "));
    Serial.print(F("Key_part to be stored in block:"));
    Serial.println(j);
    dump_byte_array(rfid_buffer, 16);
    Serial.println();
#endif

    // Write block
    status = mfrc522.MIFARE_Write(j, rfid_buffer, 16);

    if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.print(F("3155 MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif

#ifndef QUIET
      displayString("RFID 7", 30, 50, TFT_SLARK_RED);
      display_number(1, 5, 5);
#endif
      //while (true); // error
#ifdef RFID_READ_RESET
      resetFunc();
#else
      return;
#endif
    } else {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.println(F("3168 MIFARE_Write() success: "));
#endif

      status = mfrc522.MIFARE_Read(j, rfid_buffer, &bsize);

      if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
        Serial.print(F("3175 MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        Serial.println(F("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?"));
#endif

#ifndef QUIET
        displayString("RFID 8", 30, 50, TFT_SLARK_RED);
        display_number(1, 6, 6);
#endif
        //while (true); // error
#ifdef RFID_READ_RESET
        resetFunc();
#else
        return;
#endif
      } else {
#ifdef SERIAL_DEBUG_MFRC522
        Serial.println(F("3189 MIFARE_Read() success: "));
        Serial.print(F("Block: "));
        Serial.println(j);
        dump_byte_array(rfid_buffer, 18);
        Serial.println();
#endif
      }
    }
  }
}

void write_key_A_onto_rfid_chip() {
  status = mfrc522.MIFARE_Read(3, rfid_buffer, &bsize);

  if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
    Serial.print(F("3205 MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    Serial.println(F("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?"));
#endif

#ifndef QUIET
    displayString("RFID 9", 30, 50, TFT_SLARK_RED);
    display_number(1, 7, 7);
#endif

    //while (true); // error
#ifdef RFID_READ_RESET
    resetFunc();
#else
    return;
#endif
  } else {
#ifdef SERIAL_DEBUG_MFRC522
    Serial.println(F("3220 MIFARE_Read() success: "));
    Serial.print(F("Block: "));
    Serial.println(3);
    dump_byte_array(rfid_buffer, 18);
    Serial.println();
#endif
  }

  // write new desired key into first six bytes or read block
  for (byte i = 0; i < 6; i++) rfid_buffer[i] = new_key[i];

#ifdef SERIAL_DEBUG_MFRC522
  Serial.println();
  Serial.println(F("3233 Prepared block 3 to replace the existing one: "));
  Serial.println(3);
  dump_byte_array(rfid_buffer, 16);
  Serial.println();
#endif


  // Write block
  status = mfrc522.MIFARE_Write(3, rfid_buffer, 16);

  if (status != MFRC522::STATUS_OK) {
#ifndef QUIET
    displayString("RFID 10", 30, 50, TFT_SLARK_RED);
    display_number(1, 8, 8);
#endif
    //while (true);
#ifdef RFID_READ_RESET
    resetFunc();
#else
    return;
#endif

  } else {
#ifdef SERIAL_DEBUG_MFRC522
    Serial.println(F("3253 MIFARE_Write() success: "));
#endif

    status = mfrc522.MIFARE_Read(3, rfid_buffer, &bsize);

    if (status != MFRC522::STATUS_OK) {

#ifdef SERIAL_DEBUG_MFRC522
      Serial.print(F("3261 MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      Serial.println(F("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?"));
#endif

#ifndef QUIET
      displayString("RFID 11", 30, 50, TFT_SLARK_RED);
      display_number(1, 9, 9);
#endif
      //while (true);
#ifdef RFID_READ_RESET
      resetFunc();
#else
      return;
#endif

    } else {

#ifdef SERIAL_DEBUG_MFRC522
      Serial.println(F("3276 MIFARE_Read() success: "));
      Serial.print(F("Block: "));
      Serial.println(3);
      dump_byte_array(rfid_buffer, 18);
      Serial.println();
#endif
      Systemzustand = CODE_IM_GERAET;

      return;
    }
  }
}

#else

void read_chip_key() {

  select_ss(RFID_SS);

  // assign values for a desired key
  for (byte i = 0; i < 6; i++) key.keyByte[i] = new_key[i];

  mfrc522.PCD_Init(); // Init MFRC522

  while (Systemzustand == RFID_LESEN) {

    select_ss(RFID_SS);

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      if (mfrc522.uid.sak == 0) {
        byte blockAddr = 6;

        status = mfrc522.MIFARE_Read(blockAddr, rfid_buffer, &bsize);

        for (byte i = 0; i < 6; i++) {
          if (rfid_buffer[i] != new_key[i]) {
            for (byte i = 0; i < 4; i++) rfid_buffer[i] = new_key[i];

            // Write block
            if (mfrc522.MIFARE_Write(blockAddr++, rfid_buffer, 16) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
              resetFunc();
#else
              return;
#endif
            for (byte i = 4; i < 6; i++) rfid_buffer[i - 4] = new_key[i];
            for (byte i = 6; i < 8; i++) rfid_buffer[i - 4] = 0;

            // Write block
            if (mfrc522.MIFARE_Write(blockAddr++, rfid_buffer, 16) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
              resetFunc();
#else
              return;
#endif

            randomSeed(analogRead(NOT_CONNECTED_ANALOG_PIN_4_RANDOM_SEED));

            for (byte p = 0; p < 8; p++) {
              for (byte i = 0; i < 4; i++) rfid_buffer[i] = random(256);

              if (mfrc522.MIFARE_Write(blockAddr++, rfid_buffer, 16) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
                resetFunc();
#else
                return;
#endif
            }

            break;
          }
        }
        read_known_rfid_chip();

      } else {  // RFID classic

        if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 0, &key, &(mfrc522.uid)) != MFRC522::STATUS_OK) {
          if (SD_Card_inserted) { // customize new rfid chip or recustomize one already known
            customise_new_rfid_chip();
          } else {
#ifdef RFID_READ_RESET
            resetFunc();
#else
            return;
#endif          }
          } else {
            read_known_rfid_chip();
          }
        }
      }
    }
  }

  void read_known_rfid_chip() {
    byte crypt_key_part[2];

    if (mfrc522.uid.sak != 0) {
      byte crypt_key_part[2] = {1, 2};
    } else {
      byte crypt_key_part[2] = {8, 12};
    }

    for (byte i = 1; i <= 2; i++) {
      if (mfrc522.MIFARE_Read(i, rfid_buffer, &bsize) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
        resetFunc();
#else
        return;
#endif
      else for (byte j = 0; j < 16; j++) crypt_key[j + (i - 1) * 16] = rfid_buffer[j];
    }

    if (SD_Card_inserted) Systemzustand = CODE_IM_GERAET;
    else Systemzustand = PWD_AUSWAHL;
  }

  //-------------------------------------------------------------------------

  void customise_new_rfid_chip() {

    strcpy(display_string, "RFID new");
    displayString(display_string, 30, 50, TFT_SLARK_SKYBLUE);
    //delay(2000);

    // assign values for a manufacturer default key
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xff;

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 0, &key, &(mfrc522.uid)) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
        resetFunc();
#else
        return;
#endif

      if (mfrc522.MIFARE_Read(0, rfid_buffer, &bsize) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
        resetFunc();
#else
        return;
#endif

      generate_and_store_crypt_key();

      write_key_A_onto_rfid_chip();
    }
  }

  void generate_and_store_crypt_key() {

    randomSeed(analogRead(NOT_CONNECTED_ANALOG_PIN_4_RANDOM_SEED));

    for (byte j = 1; j <= 2; j++) {   // write random (256 bit) crypt key in 2 blocks on the rfid chip

      for (byte i = 0; i < 16; i++) {
        rfid_buffer[i] = random(256);
        crypt_key[i + (j - 1) * 16] = rfid_buffer[i];
      }

      if (mfrc522.MIFARE_Write(j, rfid_buffer, 16) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
        resetFunc();
#else
        return;
#endif

      if (mfrc522.MIFARE_Read(j, rfid_buffer, &bsize) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
        resetFunc();
#else
        return;
#endif  }
    }

    void write_key_A_onto_rfid_chip() {
      if (mfrc522.MIFARE_Read(3, rfid_buffer, &bsize) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
        resetFunc();
#else
        return;
#endif

      for (byte i = 0; i < 6; i++) rfid_buffer[i] = new_key[i];

      if (mfrc522.MIFARE_Write(3, rfid_buffer, 16) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
        resetFunc();
#else
        return;
#endif
      if (mfrc522.MIFARE_Read(3, rfid_buffer, &bsize) != MFRC522::STATUS_OK)
#ifdef RFID_READ_RESET
        resetFunc();
#else
        return;
#endif

      Systemzustand = CODE_IM_GERAET;
    }

#endif

//-------------------------------------------------------------------------

//#define TRACE_display_visible_pwds

#define idx_of_1st_pwd_char 3

void display_visible_pwds() {

#ifdef SERIAL_DEBUG_displayString
  long start_time_for_letter;
  long end_time_for_letter;
#endif


#ifdef SERIAL_DEBUG_displayString
  start_time_for_letter = micros();
#endif

#ifdef TRACE_display_visible_pwds
  char current_string[8];
  long addr_2_show;
#endif

  char current_name[36];
  //uint8_t act_len;
  long addr_2_read_from;

#ifdef SERIAL_DEBUG_select_pwd_2
  long start_time_for_letter;

  Serial.begin(9600);
  //aes.do_aes_encrypt(plain,sizeof(plain),cipher,key,bits,iv);
  while (!Serial);

  Serial.print(F("3324b pwd_first_visible: "));
  Serial.println(pwd_first_visible);

  start_time_for_letter = micros();

#endif

  //int sorted_idx;

  current_name[0] = ' ';
  current_name[1] = ' ';
  current_name[2] = ' ';
  current_name[3] = ' ';

#ifdef TRACE_display_visible_pwds
  //select_ss(TFT_SS);

  if (anzahl_pwd == 0) {
    displayString("SET_PWD", 30, 50, TFT_YELLOW ^ TFT_INVERT_MASK);
    delay(2000);
  } else {
    displayString("SET_PWD", 30, 50, TFT_ORANGE ^ TFT_INVERT_MASK);
    delay(2000);
  }
#endif

  for (int j = pwd_first_visible; j < (pwd_first_visible + VISIBLE_PASSWORDS) && j < anzahl_pwd; j++) {

    select_ss(FLASH_SS);

    //sorted_idx = (uint8_t) flash.readChar(PWD_Abc_Sorted_Index + j);
    addr_2_read_from = PWD_Index + (PWD_Index_Text_size + PWD_Index_Header) * (uint8_t) flash.readChar(PWD_Abc_Sorted_Index + j);

#ifdef TRACE_display_visible_pwds
    addr_2_show = addr_2_read_from;
    for (byte ap = 0; ap < 7; ap++) {
      current_string[6 - ap] = (addr_2_show % 10) + '0';
      addr_2_show = addr_2_show / 10;
    }
    current_string[7] = CHAR_ZERO;

    //select_ss(TFT_SS);

    displayString(current_string, 10, 90, 0x9cdf); // 0xFF9999 - pink

    delay(5000);
    //for (int i = idx_of_1st_pwd_char; i < 20; i++) current_name[i] = CHAR_ZERO;

    select_ss(FLASH_SS);

#endif

    if (false) {
      for (int i = 0; i < PWD_Index_Text_size + PWD_Index_Header && i < 16; i++) {

        current_name[i + idx_of_1st_pwd_char] = flash.readChar(addr_2_read_from + i);

        if (current_name[i + idx_of_1st_pwd_char] == CHAR_ZERO) {
          for (int k = i + idx_of_1st_pwd_char; k < 20; k++) current_name[k] = CHAR_ZERO;
          break;
        }
      }
    } else {
      //act_len = flash.readChar(addr_2_read_from + 30);
      for (int i = 0; i < PWD_Index_Text_size; i++) {

        current_name[i + idx_of_1st_pwd_char] = flash.readChar(addr_2_read_from + i);

        if (current_name[i + idx_of_1st_pwd_char] == CHAR_ZERO) {
          //for (int k = i + idx_of_1st_pwd_char; k < PWD_Index_Text_size + 4; k++) current_name[k] = CHAR_ZERO;
          break;
        }
      }
    }

#ifdef SERIAL_DEBUG_select_pwd_1
    Serial.print(F("3400b current_name: "));
    Serial.println(current_name);
#endif


    //select_ss(TFT_SS);

    // 160x128 - for portrait orientation 32 pixel must be considered for not visible part of logical screen (2 lines by 16 pixel font height)
    // - hence 3 in the line below
    displayString(current_name, 0, 160 - 16 * (j - pwd_first_visible + 3), (TFT_BLACK ^ TFT_INVERT_MASK) + 1); // 4 screen rotation = 0

    select_ss(FLASH_SS);

  }


#ifdef SERIAL_DEBUG_select_pwd_2
  Serial.println();
  Serial.print(F("3418 line: "));
  Serial.print("time");
  Serial.print(" - ");
  Serial.println(micros() - start_time_for_letter);
  Serial.println();
#endif

#ifdef SERIAL_DEBUG_displayString
  end_time_for_letter = micros() - start_time_for_letter;
  Serial.println();
  Serial.print(F("3428 line: "));
  Serial.print("display_visible_pwds");
  Serial.print(" - ");
  Serial.println(end_time_for_letter);
  Serial.println();
#endif


}

uint8_t get_str_width(char * str_array) {
  uint8_t width = 0;
  uint8_t font_step;

  if (smallbig == 1) {
    font_step = flash.readByte(FD_SMALL_STEP);
  } else {
    font_step = flash.readByte(FD_BIG_STEP);
  }

  uint8_t i = 0;

  while (str_array[i]) {

#ifdef USE_GRAY_COLORS_4_SMALL_FONTS
    width += flash.readByte(FD_Start_small_gray + (str_array[i++] - FIRST_CHAR_IN_FONT_DATA) * font_step);
#else
    width += flash.readByte(FD_Start + (str_array[i++] - FIRST_CHAR_IN_FONT_DATA) * font_step);
#endif
  }

  return width;
}

//------------------------------------Select PWD---------------------
bool page_up_or_down = false;

void select_pwd() {

#ifdef SERIAL_DEBUG_select_pwd_2
  long start_time_for_letter;
#endif

  char current_name[4];

  // smallbig = 2;

  //  select_ss(TFT_SS);
  //
  //  //displayString("SEL_PWD", 30, 50, TFT_BLACK ^ TFT_INVERT_MASK);
  //  //displayString("Sel. PWD", 10, 50, (TFT_SLARK_GREEN ^ TFT_INVERT_MASK));
  //  displayString("Sel. PWD", 10, 50, (TFT_DARKGREEN ^ TFT_INVERT_MASK));

  smallbig = 1;

  lockup_background_called = false;
  int previous_pwd = PWD_Index_Size + 1;

  current_name[0] = '=';
  current_name[1] = '>';
  current_name[2] = ' ';
  current_name[3] = CHAR_ZERO;

  select_pwd_ptr_len = get_str_width(current_name);

  OneButton button_r_2(SWITCH_R_2, true); // Instanz Wippe rechts aussen
  //OneButton button_r_1(SWITCH_R_1, true); // Instanz Wippe rechts innen
  OneButton button_c_s(SWITCH_C, true); // Instanz Schalter auf Wippe
  //OneButton button_l_1(SWITCH_L_1, true); // Instanz Wippe links innen
  OneButton button_l_2(SWITCH_L_2, true); // Instanz Wippe links aussen

  //  button_r_2.setPressTicks(1800);
  button_r_2.setPressTicks(800);
  button_r_2.attachClick(select_up);
  //button_r_2.attachClick(select_p_up);
  //button_r_2.attachLongPressStart(select_p_up);
  button_r_2.attachDuringLongPress(select_p_up);

  //  button_r_1.setPressTicks(800);
  //  button_r_1.attachClick(select_up);
  //  button_r_1.attachLongPressStart(select_up);
  //  button_r_1.attachDuringLongPress(select_up);

  button_c_s.attachClick(select_this_pwd);

  //  button_l_1.setPressTicks(800);
  //  button_l_1.attachClick(select_down);
  //  button_l_1.attachLongPressStart(select_down);
  //  button_l_1.attachDuringLongPress(select_down);

  button_l_2.setPressTicks(800);
  //button_l_2.attachClick(select_p_down);
  button_l_2.attachClick(select_down);
  //button_l_2.attachLongPressStart(select_p_down);
  button_l_2.attachDuringLongPress(select_p_down);

  select_ss(FLASH_SS);

  byte b1 = flash.readByte(PWD_Index_Count);
  byte b2 = flash.readByte(PWD_Index_Count + 1);

  anzahl_pwd = b1 * 0x100 + b2;


#ifdef SERIAL_DEBUG_select_pwd
  Serial.begin(9600);

  while (!Serial);

  Serial.println(F("3537 (Re)Customise started"));

  test_select_this_pwd(anzahl_pwd);
#endif

  current_pwd = 0;

  //select_ss(TFT_SS);
  //tft.fillScreen(TFT_BLACK ^ TFT_INVERT_MASK);
  background_color = TFT_BLACK ^ TFT_INVERT_MASK;
  tftFillScreen(background_color);
  //tftFillScreen(TFT_BLACK ^ TFT_INVERT_MASK);

  display_visible_pwds();

  while (Systemzustand == PWD_AUSWAHL) {

    if (previous_pwd != current_pwd) {
      previous_pwd = current_pwd;

      if (current_pwd < pwd_first_visible) {
        if (current_pwd < VISIBLE_PASSWORDS) {
          pwd_first_visible = 0;
        } else {
          pwd_first_visible = pwd_first_visible - VISIBLE_PASSWORDS;
        }

        //select_ss(TFT_SS);
        //tft.fillScreen(TFT_BLACK ^ TFT_INVERT_MASK);
        background_color = TFT_BLACK ^ TFT_INVERT_MASK;
        tftFillScreen(background_color);
        //tftFillScreen(TFT_BLACK ^ TFT_INVERT_MASK);

        display_visible_pwds();

      } else {
        if (current_pwd >= pwd_first_visible + VISIBLE_PASSWORDS) {
          pwd_first_visible = current_pwd;

          if (pwd_first_visible >= anzahl_pwd - VISIBLE_PASSWORDS) {
            pwd_first_visible = anzahl_pwd - VISIBLE_PASSWORDS;
          }

          //select_ss(TFT_SS);
          //tft.fillScreen(TFT_BLACK ^ TFT_INVERT_MASK);
          background_color = TFT_BLACK ^ TFT_INVERT_MASK;
          tftFillScreen(background_color);
          //tftFillScreen(TFT_BLACK ^ TFT_INVERT_MASK);

          display_visible_pwds();
        }
      }

      if (lockup_background_called) {
        lockup_background_called = false;

        //select_ss(TFT_SS);
        //tft.fillScreen(TFT_BLACK ^ TFT_INVERT_MASK);
        background_color = TFT_BLACK ^ TFT_INVERT_MASK;
        tftFillScreen(background_color);
        //tftFillScreen(TFT_BLACK ^ TFT_INVERT_MASK);

        display_visible_pwds();
      }

      select_ss(TFT_SS);

      //tft.fillRect(0, 0, 160, 21, TFT_BLACK ^ TFT_INVERT_MASK);
      tft.fillRect(0, 0, 128, 21, TFT_BLACK ^ TFT_INVERT_MASK);         // mod07.06 - the undesired pixel is moved to last row of display

      //tft.fillRect(0, 0, 128, select_pwd_ptr_len, TFT_BLACK ^ TFT_INVERT_MASK);         // mod07.06 - the undesired pixel is moved to last row of display


      current_name[0] = '=';
      current_name[1] = '>';
      current_name[2] = ' ';
      current_name[3] = CHAR_ZERO;

#ifdef SERIAL_DEBUG_select_pwd
      Serial.print(F("3608a anzahl_pwd: "));
      Serial.println(anzahl_pwd);
      Serial.print(F("3610a current_pwd: "));
      Serial.println(current_pwd);
      Serial.print(F("3612a pwd_first_visible: "));
      Serial.println(pwd_first_visible);
      Serial.print(F("3614a FD_SMALL_STEP: "));
      Serial.println(flash.readByte(FD_SMALL_STEP));
      Serial.print(F("3616a FD_BIG_STEP: "));
      Serial.println(flash.readByte(FD_BIG_STEP));
#endif

      select_ss(TFT_SS);

      displayString(current_name, 0, 160 - 16 * (((current_pwd - pwd_first_visible) % VISIBLE_PASSWORDS) + 3), (TFT_BLACK ^ TFT_INVERT_MASK) + 1); // 4 screen rotation = 2

      tft.fillRect(0, 0, 12, 2, TFT_BLACK ^ TFT_INVERT_MASK);       // mod07.06 -  and here deleted explicitly - it's only way I found that seems to work

      start_time_for_pin_input = millis();

      targetTime = 0;

    } else {
      if (millis() - start_time_for_pin_input > 20000) {
        lockup_background_called = true;
        lockup_background();
      }
    }

    if (lockup_background_called && any_button_active) {
      lockup_background_called = false;

      //select_ss(TFT_SS);
      //tft.fillScreen(TFT_BLACK);
      background_color = TFT_BLACK ^ TFT_INVERT_MASK;
      tftFillScreen(background_color);
      //tftFillScreen(TFT_BLACK);

      display_visible_pwds();

      start_time_for_pin_input = millis();
    }

    any_button_active = false;

    if (!any_button_active) button_r_2.tick();
    if (!any_button_active) button_l_2.tick();

    //if (!any_button_active) button_r_1.tick();
    //if (!any_button_active) button_l_1.tick();

    button_c_s.tick();
  }
}

void select_up() {
  //if (any_button_active) return;

  if (false && page_up_or_down) {
    page_up_or_down = false;
    return;
  }

  if (current_pwd < (anzahl_pwd - 1)) current_pwd++;

  //if (current_pwd >= (anzahl_pwd - 1)) current_pwd = anzahl_pwd - 1;

  any_button_active = true;
}

void select_down() {
  //if (any_button_active) return;

  if (false && page_up_or_down) {
    page_up_or_down = false;
    return;
  }

  if (current_pwd) current_pwd--;

  any_button_active = true;
}

void select_p_up() {
  //if (any_button_active) return;

  if (anzahl_pwd < VISIBLE_PASSWORDS) {
    current_pwd = anzahl_pwd - 1;
  } else {
    if (current_pwd > (anzahl_pwd - VISIBLE_PASSWORDS)) {
      current_pwd = anzahl_pwd - 1; //anzahl_pwd - VISIBLE_PASSWORDS;
    } else {

      current_pwd = current_pwd + VISIBLE_PASSWORDS;

      //if (current_pwd > anzahl_pwd - 1) current_pwd = anzahl_pwd - 1;
    }
  }

  any_button_active = true;

  page_up_or_down = true;
}

void select_p_down() {
  //if (any_button_active) return;

  if (current_pwd < VISIBLE_PASSWORDS) {
    current_pwd = 0;
  } else {
    current_pwd = current_pwd - VISIBLE_PASSWORDS;
    //if (current_pwd < 0) current_pwd = 0;
  }

  any_button_active = true;

  page_up_or_down = true;
}

void select_this_pwd() {

  // do not issue PWD in save screen mode
  if (lockup_background_called) {
    any_button_active = true;
    return;
  }

#ifndef SERIAL_DEBUG_select_pwd
  Keyboard.begin();
#endif

  char current_pwd_data[PWD_Size + 1] = {0};
  long pwd_addr;

  select_ss(FLASH_SS);

  pwd_addr = PWD_Start + (uint32_t) PWD_Size * (uint8_t) flash.readChar(PWD_Abc_Sorted_Index + current_pwd);

  for (int i = 0; i < PWD_Size; i++) current_pwd_data[i] = flash.readChar(pwd_addr + i);

#ifdef SERIAL_DEBUG_dump_pwd_to_pass
  Serial.println(F("3747 loop entered: "));
  Serial.print(F("Dump pwd_addr: "));
  Serial.println(pwd_addr);

  uint8_t buf[32];

  Serial.println(F("3753 loop entered: "));
  Serial.print(F("Dump password string: "));
  Serial.println();

  memcpy(buf, current_pwd_data, 32);

  for (int i = 0; i < 32; i++) {
    Serial.print(buf[i] < 0x10 ? " 0" : " ");
    Serial.print(buf[i], HEX);
  }
  Serial.println();
#endif

  select_ss(TFT_SS);

#ifdef SERIAL_DEBUG_select_pwd
  Serial.println(F("3769-a current_pwd_data read: "));
  Serial.print(F("PWD-C: "));
  //    dump_byte_array(current_pwd_data, PWD_Size);
  for (int i = 0; i < PWD_Size; i++) {
    Serial.print((uint8_t) current_pwd_data[i] < 0x10 ? " 0" : " ");
    Serial.print((uint8_t) current_pwd_data[i], HEX);
  }
  Serial.println();
#endif

#ifdef USE_CRYPT
  encrypt_pwd(current_pwd_data, current_pwd_data, false);
#endif

#ifdef SERIAL_DEBUG_select_pwd
  Serial.println(F("3784-b current_pwd_data encrypted: "));
  Serial.print(F("PWD-E: "));
  //    dump_byte_array(current_pwd_data, PWD_Size);
  for (int i = 0; i < PWD_Size; i++) {
    Serial.print((uint8_t) current_pwd_data[i] < 0x10 ? " 0" : " ");
    Serial.print((uint8_t) current_pwd_data[i], HEX);
  }
  Serial.println();
#endif

  select_ss(FLASH_SS);

  //for (int i = 0; i < PWD_Size; i++) current_pwd_data[i] = usToDE[current_pwd_data[i]];
  for (int i = 0; i < PWD_Size; i++) 
    current_pwd_data[i] = (uint8_t) flash.readChar(Keyboard_Layout_Start + (uint8_t) current_pwd_data[i]);

#ifdef SERIAL_DEBUG_dump_pwd_to_pass
  Serial.println(F("3800 loop entered: "));
  Serial.print(F("Dump password string: "));
  Serial.println();

  memcpy(buf, current_pwd_data, 32);

  for (int i = 0; i < 32; i++) {
    Serial.print(buf[i] < 0x10 ? " 0" : " ");
    Serial.print(buf[i], HEX);
  }
  Serial.println();
#endif


#ifdef SERIAL_DEBUG_select_pwd
  Serial.println(F("3815-c current_pwd_data mapped: "));
  Serial.print(F("PWD-K: "));
  //    dump_byte_array(current_pwd_data, PWD_Size);
  for (int i = 0; i < PWD_Size; i++) {
    Serial.print((uint8_t) current_pwd_data[i] < 0x10 ? " 0" : " ");
    Serial.print((uint8_t) current_pwd_data[i], HEX);
  }
  Serial.println();

#else

  // mod04.03
  // special handling for special characters
  // triggered by @

  int i;

  while (current_pwd_data[i]) {
    if (current_pwd_data[i] == 0x40) {      // @ - 0x40
      // with en as language it delivers @ - with de "
      //Keyboard.press(KEY_LEFT_SHIFT);
      //Keyboard.write('2');

      Keyboard.press(KEY_RIGHT_ALT);

      Keyboard.write('q');

      Keyboard.releaseAll();

    } else {

      Keyboard.print(current_pwd_data[i]);

    }

    i++;

    delay(10);

  }

#endif
}

byte triangle_number = 0;

// below background color of SLARK got using GIMP from a photo posted in Slack
//int SLARK_background   = rgb3x8_2_565_xrb(0x8b6953);
//int SLARK_background = 0x5351;

void lockup_background() {

  int slark_colors[TFT_SLARK_COLORS] = SLARK_GmbH_colors;


  if (targetTime == 0) {
    targetTime = start_time_for_pin_input + 25000;
  }

  if (targetTime < millis()) {
    targetTime += 300;

    triangle_number++;

    if (triangle_number > 30) {
      triangle_number = 0;

      //select_ss(TFT_SS);
      //tft.fillScreen(slark_colors[TFT_SLARK_COLORS - 1] ^ TFT_INVERT_MASK);
      tftFillScreen(slark_colors[TFT_SLARK_COLORS - 1] ^ TFT_INVERT_MASK);

    } else {
      tft.fillTriangle(
        random(128), random(160),
        random(128), random(160),
        random(128), random(160),
        slark_colors[random(TFT_SLARK_COLORS - 1)] ^ TFT_INVERT_MASK);
    }
  }
}


/*
   https://docs.microsoft.com/en-us/windows/security/threat-protection/security-policy-settings/password-must-meet-complexity-requirements

   below some relevat excerpts:

   ... minimum password length of 8 ...

   The password contains characters from three of the following categories:

    - Uppercase letters of European languages (A through Z, with diacritic marks, Greek and Cyrillic characters)
    - Lowercase letters of European languages (a through z, sharp-s, with diacritic marks, Greek and Cyrillic characters)
    - Base 10 digits (0 through 9)
    - Non-alphanumeric characters (special characters): (~!@#$%^&*_-+=`|\(){}[]:;"'<>,.?/)
      [Consider implementing a requirement in your organization to use ALT characters in the range from 0128 through 0159
      as part of all administrator passwords. (ALT characters outside of this range can represent standard alphanumeric
      characters that do not add additional complexity to the password.)]
      Currency symbols such as the Euro or British Pound are not counted as special characters for this policy setting.
    - Any Unicode character that is categorized as an alphabetic character but is not uppercase or lowercase.
      This includes Unicode characters from Asian languages.

      input for this method has to follow regex syntax:

      new[ssssss]{min_length,max_length}
      adm[ssssss]{min_length,max_length}
      upd[ssssss]{min_length,max_length}

      where "s" means any of the special characters as shown above, that must not occure in PWD to be generated

      !!! Because the write operation cannot increase the file size it's necessary to create the file being large enough
          to assure space for both PWD and eventually presen 2nd line that identify the PWD on place of the file name

          Probably the simplest possibilty it to create a file using following pattern:

  new[$%&]{13,157}
  Gen 2nd line
  1234567890
  2234567890
  3234567890
  4234567890
  5234567890
  6234567890
  7234567890
  8234567890
  9234567890
  0234567890
  1234567890
  2234567890
  3234567890
  4234567890
  5234567890
  6234567890
  7234567890
  8234567890
  9234567890
  0234567890
  1234567890
  2234567890
  3234567890
  4234567890
  5234567890
  6234567890
  7234567890
  8234567890

          It reserves about 300 byte more than 256 of maximal PWD size and 16 byte of 2nd line description
*/

bool createPWD(char *pwd) {

  uint8_t pwd_len = strlen(pwd);

#ifdef SERIAL_DEBUG_createPWD

  Serial.begin(9600);

  while (!Serial);

  Serial.println(F("3974 createPWD entered: "));
  Serial.print(F("PWD from file: "));
  Serial.print(pwd);
  Serial.print(" - ");
  dump_byte_array(pwd, pwd_len);
  Serial.println();

  Serial.print(F("3981 keyword in pwd: "));
  Serial.println(pwd[0] == 'n');
#endif

  //if (strstr(pwd, 'new') != NULL || strstr(pwd, 'upd') != NULL || strstr(pwd, 'adm') != NULL) {
  if (pwd[0] == 'n' || pwd[0] == 'u' || pwd[0] == 'a') {

    uint8_t s_pos = 0;    // = strchr(pwd, ']');
    while (pwd[s_pos] != '[' && s_pos < pwd_len) s_pos++;
    uint8_t e_pos = pwd_len;    // = strchr(pwd, '{') - 1;
    while (pwd[e_pos] != ']' && e_pos > s_pos) e_pos--;

#ifdef SERIAL_DEBUG_createPWD
    Serial.print(F("3994 s_pos - e_pos: "));
    Serial.print(s_pos);
    Serial.print(" - ");
    Serial.print(e_pos);
    Serial.println();
#endif

    char no_go_char[e_pos - s_pos];

    //memcpy(no_go_char, pwd[s_pos + 1], e_pos - 2 - s_pos - 1);
    for (uint8_t i = 0; i < e_pos - s_pos - 1; i++) no_go_char[i] = pwd[s_pos + i + 1];
    no_go_char[e_pos - s_pos - 1] = 0;

    char r_char;
    uint8_t r_max;
    uint8_t n = 0;
    uint8_t min_length = 0;
    uint8_t max_length = 0;
    uint8_t fix_length = 0;

    //min_length = strtol(pwd[e_pos], NULL, 10);
    //max_length = strtol(strchr(pwd[e_pos], ','), NULL, 10);

    s_pos = e_pos + 1;
    while (pwd[s_pos] != '{') s_pos++;
    s_pos++;
    while (pwd[s_pos] != '}' && pwd[s_pos] != ','  && s_pos < pwd_len) {
      min_length = min_length * 10 + (pwd[s_pos] - '0');
      s_pos++;
    }

    if (min_length < 8) min_length = 8;

    if (pwd[s_pos] == '}') fix_length = min_length;
    else {
      s_pos++;
      while (pwd[s_pos] != '}' && s_pos < pwd_len) {
        max_length = max_length * 10 + pwd[s_pos] - '0';
        s_pos++;
      }
      if (max_length > 256) max_length = 256;
    }

#ifdef SERIAL_DEBUG_createPWD
    Serial.print(F("4038 min_length - max_length - fix_length: "));
    Serial.print(min_length);
    Serial.print(" - ");
    Serial.print(max_length);
    Serial.print(" - ");
    Serial.print(fix_length);
    Serial.println();
#endif

    if (pwd[0] == 'a') r_max = 0x9f;
    else r_max = 0x7f;

#ifdef SERIAL_DEBUG_createPWD
    Serial.println(F("4051 createPWD values: "));
    Serial.print(F("s_pos - e_pos - min_length - max_length - r_max - no_go_char: "));
    Serial.println();
    Serial.print(s_pos);
    Serial.print(" - ");
    Serial.print(e_pos);
    Serial.print(" - ");
    Serial.print(min_length);
    Serial.print(" - ");
    Serial.print(max_length);
    Serial.print(" - ");
    Serial.print(r_max);
    Serial.print(" - ");
    Serial.print(no_go_char);
    Serial.print(" - ");
    dump_byte_array(no_go_char, strlen(no_go_char));
    Serial.println();
#endif

    randomSeed(analogRead(NOT_CONNECTED_ANALOG_PIN_4_RANDOM_SEED));

    //    while (n < random(max_length - min_length) + min_length) {
    while (n < max_length) {
      r_char = random(r_max - 0x20) + 0x20;
      if (r_char > 0x20 && strchr(no_go_char, r_char) == NULL) {
        pwd[n] = r_char;
        n++;
      }
      pwd[n] = 0;
    }
#ifdef SERIAL_DEBUG_createPWD
    Serial.println(F("4082 createPWD values: "));
    Serial.print(pwd);
    Serial.print(" - ");
    dump_byte_array(pwd, strlen(pwd));
    Serial.println();
#endif

    return true;
  } else {
    return false;
  }
}
