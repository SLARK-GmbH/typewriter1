#include <User_Setup.h>

//#define USE_NOINLINE

#ifdef USE_NOINLINE

void __attribute__ ((noinline)) display_number(byte number_of_digits, long number, long limit);
void __attribute__ ((noinline)) flash_erase_2_write(long start_address, byte area_size);
bool __attribute__ ((noinline))  is_typefile(char *filename, char *file_ext);
void __attribute__ ((noinline))  displayString(char *charString, uint16_t cursor_x, uint16_t cursor_y, uint16_t textcolor);
int8_t __attribute__ ((noinline))  displayletter(char letter, uint16_t cursor_y, uint16_t cursor_x, uint16_t textcolor);

#endif


//----------------------------PIN Mappings---------------------------------

#define READ_FILE_REPETITIONS 10    // number of tries to open a file

#define GET_DATA_FROM_SD      // for test only - normally all 4 parts have to be executed


#ifdef GET_DATA_FROM_SD

#define PERFORM_PIN_CHECK
#define PERFORM_RFID_CHECK
#define PERFORM_SD_CHECK
#define PERFORM_PWD_SELECT

#else

/*
   to use 32768 bytes to upload code with programmer - normally 28672
   the line: promicro.upload.maximum_size=32768
   in the file /home/me/.arduino15/packages/SparkFun/hardware/avr/1.1.11/boards.txt
   or ./snap/arduino/32/.arduino15/packages/SparkFun/hardware/avr/1.1.13/boards.txt
   has to be modified as above for the board: promicro.name=SparkFun Pro Micro
*/

#define PERFORM_PIN_CHECK
#define PERFORM_RFID_CHECK
#define PERFORM_SD_CHECK
#define PERFORM_PWD_SELECT

#endif

//-----RFID----

#define RFID_RST 1
#define RFID_SS  0
#define RFID_IRQ 2

//-----Crypt

#define USE_CRYPT

// if USE_SPRITZ is commented out AES will be used
//#define USE_SPRITZ

#define CRYPT_KEY_SIZE   32
//#define FOLLOW_USE_CRYPT    // if set one cann see the first few bytes of key to verify
// the same one is used while storing PWDs and while passing to the host

#define NOT_CONNECTED_ANALOG_PIN_4_RANDOM_SEED 6

//-----SD-Karte----

#define SD_SS 3

//-----SPI-Flash----

#define FLASH_SS 6

//-----Display----

#define TFT_SS   8
#define TFT_AD   9
#define TFT_RST 10

//-----Taster----


#define SWITCH_L_1   18
#define SWITCH_L_2   19
#define SWITCH_C     20
#define SWITCH_R_2   21
#define SWITCH_R_1   22

//-----Zeug zur Bildschirmfüllung-------

#define MAX_LINES  14

#define CHAR_ZERO '\0'
#define CHAR_NL   '\n'
#define CHAR_CR   '\r'



#define FIRST_CHAR_IN_FONT_DATA 0x20 // usually a space

//-----Adressen in SPI Flash-----

#define SPI_FLASH_SIZE    262144  // 256kB; 256 x 1024


#ifdef FONT_DATA_IN_SPI_MEMORY

// !!! inroducing fonts made smooth using gray colors - shift PWDs at the end of available memory

#define Shift_PWDs_up_by_32k  0x8000

#define Keyboard_Layout_Active 7  //Hier steht die Nummer des aktuellen Keyboard Layouts drin
#define Keyboard_Layouts_Count 20 //Adresse, wo die Anzahl steht, NICHT die Anzahl
#define Keyboard_Layout_Names 128  //Startadresse, 8 Byte pro Namen des Layouts
#define Keyboard_Layout_Size  256  //Größe der Keyboard Tabellen 128 reicht, aber wer weiß... 
#define Keyboard_Layout_Start 3072 //Startadresse des ersten Layouts - 0xc00

#define Texte_Start           9216 //Adresse der Textetabelle - 0x2400

#define FD_SMALL_SIZE         7     // 24k for gray small fonts; previous: 2     // 8k - FD_Size/4
#define FD_BIG_SIZE           26    // 104k for gray big fonts;   previous  6    // 24k - FD_Size - FD_Size/4

#define FD_Start_small_gray_size    FD_SMALL_SIZE * 4 * 1024      // 0x6000
#define FD_Start_big_gray_size      FD_BIG_SIZE * 4 * 1024        // 0x1A000

#define FD_SMALL_SIZE_gray    FD_SMALL_SIZE     // 32k - FD_Size/4

//Fontdaten abgelegt im SPI Speicher um Programmspeicher zu enlasten
// 8ung - es braucht Speicher und muss bedacht werden, dass Abspeichern von Passwörter nicht 4 sonder nur 2 mal möglich ist !!!

#define FD_Start_small_gray   12288L                                              // 0x4000
//------------------------------------------------------------------------------------------

#define FD_small_gray_size_end  FD_Start_small_gray + FD_Start_small_gray_size    // 0xA000

#define FD_SMALL_STEP_gray    FD_small_gray_size_end - 4

#define FD_Start_big_gray     FD_small_gray_size_end                              // 0xA000
//------------------------------------------------------------------------------------------

#define FD_Start              0x38000   // Shift_PWDs_up_by_32k + FD_Start_small_gray + FD_SMALL_SIZE_gray
// 229376 //Startadresse für kleine Fontdaten
#define FD_Size               4 * 32768         // 0x20000 - previous: 32768 (0x8000) //32kB für diese Daten

#define FD_Start_1            FD_Start_big_gray // FD_Start + 0x8000   // 8 * 1024   //Startadresse für große Fontdaten

#define FD_SMALL_STEP         FD_Start_1 - 4
#define FD_BIG_STEP           (uint32_t) FD_Start_1 + (uint32_t) FD_BIG_SIZE * 0x1000 - 4   // previous: FD_Start_1 + SPI_FLASH_SIZE - 4

#define FD_SMALL_GREY_EXTENSION   (uint32_t) FD_SMALL_STEP + 1     // !!! it has to be in the next byte to the STEP
#define FD_BIG_GREY_EXTENSION     (uint32_t) FD_BIG_STEP + 1       // as it's read using by one call of readCharArray method

#endif



#define FD_BIG_SIZE_gray    FD_BIG_SIZE * 4 * 1024     // 32k - FD_Size/4

#define FD_big_gray_size_end  FD_Start_big_gray + FD_Start_big_gray_size

#define FD_BIG_STEP_gray    FD_big_gray_size_end - 4


#define Texte_Size            32   //32 Zeichen Pro Text 

#define VISIBLE_PASSWORDS     8    //Anzahl der Paswörter auf dem Display

#define PWD_Block_of_64k_1    64 * 1024 - 1
//#define PWD_Index             Shift_PWDs_up_by_32k + 131072    // 128k - Adresse des Passwortindex - 0x28000
#define PWD_Index 0x28000
//------------------------------------------------------------------------------------------------------------
#define PWD_Index_Size        256    //Anzahl möglicher Passwörter 
#define PWD_Abc_Sorted_Index  PWD_Start - 4096
#define PWD_Index_Text_size   30     //16384 Bytes für die Namen der Passwörter
#define PWD_Index_Header      2      //31. Byte der Namen ist Header; gelöscht, aktiv, ... , 32. Byte Adresse des Passworts

#define PWD_Index_Count       PWD_Start - 4      //counter value to be writable after password import - flash must be erased to be changable
// address where actual number of PWDs is stored
#define PWD_Index_Count_Size  2

#define PIN_length            4                 // number of digits in PIN

#define PIN_valid_value       PWD_Start - 2  // actual valid PIN value
#define PIN_valid_value_size  2


//Passwörter werden 4 mal gespeichert und beim einlesen verglichen!
//#define PWD_Start             Shift_PWDs_up_by_32k + 163840 // 160k - Startadresse für Passwörter
// 0x8000 + 0x28000 = 0x30000
#define PWD_Start 0x30000
//-------------------------------------------------------------------------------------------------------------
#define PWD_Size              256    //256 Zeichen Pro Passwort reserviert

#define RFID_Key_Size         6      //Standardgröße für RFID Key
#define RFID_Key_Address      128    //Adresse des Keys

// 0x534c41524b47 -> ":".join("{:02x}".format(ord(c)) for c in 'SLARKG') <- python to get the hex number

#define RFID_Own_Magic {0x53, 0x4c, 0x41, 0x52, 0x4b, 0x47}
#define RFID_Own_Key   {0xAF, 0xFE, 0xCA, 0xFE, 0xEF, 0xF1}
#define RFID_Default_Key   {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}


//-----Systemzust�nde----

#define ANGESCHALTET               0
#define SPI_FLASH_LEER             1

#define RFID_LESEN                 4

#define PWD_AUSWAHL               17

#define PWD_SELECTED              19

#define CODE_IM_GERAET            10
#define MENU_TEXTE_VORHANDEN      20
#define SPRACHE_FESTGELEGT        30
#define MAPPINGTABELLEN_VORHANDEN 40
#define KEYMAPPING_AUSGEWAEHLT    50

#define PW_IMPORT_ERFOLGREICH     60

#define PASSWORTFILE_VORHANDEN    70

#define SD_GESTECKT               80

#define RFID_GELESEN              90

#define PIN_EINGABE              100

#define PIN_KORREKT              200
#define PIN_FALSCH               300


/*
   rgb3x8 -> rgb565 can be calculated using python lines:
   rgb3x8=0xfedcba
   and
   '%04x' % (((int(rgb3x8 % 0x100) & 0b11111000)<<8) + ((int((rgb3x8 / 0x100) % 0x100) & 0b11111100)<<3)+(int(rgb3x8 / 0x10000)>>3))
*/

#define TFT_SLARK_RED         0x001f
#define TFT_SLARK_GREEN       0x07e0

#define TFT_SLARK_YELLOW      0x07ff
#define TFT_SLARK_ORANGE      0x053f    // 0xFFA500
#define TFT_SLARK_SKYBLUE     0x8400  //0xfb2c
#define TFT_SLARK_WHITE       0xffff
#define TFT_SLARK_BACKGROUND  0x5351
#define TFT_SLARK_PINK        0x9cdf    // 0xFF9999 - pink

#define TFT_TEAL              0x8400    // 0x008080 - teal

/*
   https://www.rapidtables.com/web/color/gray-color.html

    gainsboro               #DCDCDC   rgb(220,220,220)  0xdefb
    lightgray / lightgrey   #D3D3D3   rgb(211,211,211)  0xd69a
    silver                  #C0C0C0   rgb(192,192,192)  0xc618
    darkgray / darkgrey     #A9A9A9   rgb(169,169,169)  0xad55
    gray / grey             #808080   rgb(128,128,128)  0x8410
*/

#define TFT_GAINSBORO   0xdefb
#define TFT_LIGHTGRAY   0xd69a
#define TFT_SILVER      0xc618
#define TFT_DARKGRAY    0xad55
#define TFT_GRAY        0x8410

#define rgb_colors_of_gray_on_black    {0x000000, 0x080808, 0x282828, 0x606060, 0xa0a0a0, 0xb8b8b8, 0xffffff}
#define rgb_colors_of_gray_on_teal     {0x008080, 0x080808, 0x282828, 0x606060, 0xa0a0a0, 0xb8b8b8, 0xffffff}

#define rgb565_colors_of_gray {0x0000, 0x0841, 0x2945, 0x630c, 0xa514, 0xbdd7, 0xffff}

// yellow, orange, skyblue, white and SLARK_background
#define SLARK_GmbH_colors_old     { TFT_SLARK_YELLOW, TFT_SLARK_ORANGE, TFT_SLARK_SKYBLUE, TFT_SLARK_WHITE, TFT_SLARK_RED, TFT_SLARK_GREEN, TFT_DARKGREEN, TFT_SLARK_BACKGROUND }
#define SLARK_GmbH_colors { 0x2cde, 0x359e, 0x473f, 0x93c3, 0xd4a4 }
// colors above as on the picture: brands SLARK - orange, mustard, yellow, petrol, blue


#define TFT_SLARK_COLORS_old 8
#define TFT_SLARK_COLORS 5

//#define TFT_INVERT_COLORS

#ifdef TFT_INVERT_COLORS
#define TFT_INVERT_MASK  0xffff
#else
#define TFT_INVERT_MASK  0x0
#endif


/*

  +----------------------+
  |                      | \
  |                      | | Keyboard_Layout_Active 7
  |                      | |
  |                      | |  Keyboard_Layouts_Count 20
  |                      | |
  |                      | |  Keyboard_Layout_Names 128
  |                      | > 144k
  |                      | |  Keyboard_Layout_Start 1024 (1k)
  |                      | |
  |                      | |
  |                      | |  Texte_Start           9216 (9k)
  |                      | |
  |                      | |  FD_Start_gray         16384
  |                      | |
  |                      | |  FD_Start_gray_size    32768
  |                      | |  FD_SMALL_STEP_gray    FD_Start_gray + FD_Start_gray_size - 4
  |                      | |  FD_Start_gray_size_end  FD_Start_gray + FD_Start_gray_size
  |                      | |
  |                      | /
  +======128k============+
  |   32768 (16384)      | \  PWD_Index             131072 (114688)  flash.eraseSection(PWD_Index, PWD_Start - PWD_Index);
  |      32k (16k)       | |
  |                      | |
  |                      | |
  |                      | > 32k
  |                      | |
  +-------156k-----------+ |
  | PWD_Abc_Sorted_Index | | \   PWD_Start - 4k
  |     256 (128)        | | |
  +----------------------+ | > 4k
  |           2          | | |   PWD_Index_Count       PWD_Start - 4  \   !!! caution both PWD_Index_Count and PIN_valid_value
  +----------------------+ | |                                         >      must occupy 4 cosecutive bytes to make possible
  |           2          | / /   PIN_valid_value       PWD_Start - 2  /       writing them with one call of writeCharArray !!!
  +=======160k===========+
  |       65536          | \  PWD_Start             163840  flash.eraseSection(PWD_Start, FD_Start - PWD_Start);
  |        64k           | |
  |                      | > 64k
  |                      | |
  |                      | /
  +=======224k===========+
  |        8192          | \  FD_Start              229376  flash.eraseSection(font_flash_addr, reserved_size);
  |         8k           | |                FD_Start  8k
  |                      | >  8k
  +----------------------+ |
  |           4          | /  FD_SMALL_STEP         FD_Start_1 - 4 / FD_SMALL_GREY_EXTENSION FD_Start_1 - 2
  +=======232k===========+
  |       24576          | \  FD_Start_1            FD_Start + FD_SMALL_SIZE
  |        24k           | |          flash.eraseSection(font_flash_addr, reserved_size);
  |                      | |                    FD_Start_1  24k
  |                      | > 24k
  |                      | |
  +----------------------+ |
  |           4          | /  FD_BIG_STEP           SPI_FLASH_SIZE - 4 / FD_BIG_GREY_EXTENSION  SPI_FLASH_SIZE - 2
  +-------256k-----------+  SPI_FLASH_SIZE        262144

*/
