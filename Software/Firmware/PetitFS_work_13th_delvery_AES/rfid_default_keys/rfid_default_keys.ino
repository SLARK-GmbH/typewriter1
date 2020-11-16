#include <DES_config.h>
#include <printf.h>
#include <DES.h>

/*
   ----------------------------------------------------------------------------
   This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
   for further details and other examples.

   NOTE: The library file MFRC522.h has a lot of useful info. Please read it.

   Released into the public domain.
   ----------------------------------------------------------------------------
   Example sketch/program which will try the most used default keys listed in
   https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys to dump the
   block 0 of a MIFARE RFID card using a RFID-RC522 reader.

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

*/

#include <SPI.h>
#include <MFRC522.h>

#define SERIAL_DEBUG_MFRC522
//#define TEST_IMITATE_CARD_RESPONSE

#define RST_PIN         9 //0           // Configurable, see typical pin layout above
#define SS_PIN          10  //1          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.


MFRC522::MIFARE_Key key; // RFID Schlüssel default HEx 00-00-00-00-00-00
MFRC522::StatusCode status;

DES des;


// Number of known default keys (hard-coded)
// NOTE: Synchronize the NR_KNOWN_KEYS define with the defaultKeys[] array
#define NR_KNOWN_KEYS   9
// Known keys, see: https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys
byte knownKeys[NR_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] =  {
  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF = factory default
  {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
  {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
  {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
  {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
  {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
  {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
  {0xca, 0xfe, 0xaf, 0xfe, 0xef, 0xf1}, // CA FE AF FE EF F1
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 00 00 00 00 00 00
};

#define NOT_CONNECTED_ANALOG_PIN_4_RANDOM_SEED 6
#define CRYPT_KEY_SIZE 32

/*
   Initialize.
*/

uint8_t rfid_buffer[CRYPT_KEY_SIZE];
uint8_t bsize = sizeof (rfid_buffer);
uint8_t crypt_key[CRYPT_KEY_SIZE];
uint8_t page_no;

uint32_t counter = 0L;

void setup() {
  Serial.begin(115200);         // Initialize serial communications with the PC
  while (!Serial);            // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();                // Init SPI bus
  mfrc522.PCD_Init();         // Init MFRC522 card
  Serial.println(F("Try the most used default keys to print block 0 of a MIFARE PICC."));
  Serial.println();
  Serial.println(F("Finishing setup."));
  Serial.println();
}

/*
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/*
   Try using the PICC (the tag/card) with the given key to access block 0.
   On success, it will show the key details, and dump the block data on Serial.

   @return true when the given key worked, false otherwise.
*/
bool try_key(MFRC522::MIFARE_Key *key)
{
  bool result = false;
  byte buffer[18];
  byte block = 0;
  MFRC522::StatusCode status;

  // http://arduino.stackexchange.com/a/14316
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return false;
  if ( ! mfrc522.PICC_ReadCardSerial())
    return false;
  // Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    // Serial.print(F("PCD_Authenticate() failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  // Read block
  byte byteCount = sizeof(buffer);
  status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
  if (status != MFRC522::STATUS_OK) {
    // Serial.print(F("MIFARE_Read() failed: "));
    // Serial.println(mfrc522.GetStatusCodeName(status));
  }
  else {
    // Successful read
    result = true;
    Serial.print(F("Success with key:"));
    dump_byte_array((*key).keyByte, MFRC522::MF_KEY_SIZE);
    Serial.println();
    // Dump block data
    Serial.print(F("Block ")); Serial.print(block); Serial.print(F(":"));
    dump_byte_array(buffer, 16);
    Serial.println();
  }
  Serial.println();

  mfrc522.PICC_HaltA();       // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
  return result;
}

/*
   Main loop.
*/
void loop() {
  Serial.println(F("Starting loop."));
  Serial.println(counter++);

  MFRC522::StatusCode status;

  byte buffer[512];
  byte bufferSize;

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {

#ifdef SERIAL_DEBUG_MFRC522
    Serial.print(F("3368 MIFARE_Write() sak: "));
    Serial.println(mfrc522.uid.sak);
#endif
  }

/*
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;
*/
  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  if (false ) {

    status = mfrc522.MIFARE_Read(0, buffer, &bufferSize);

    if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.print(F("179 MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      Serial.println(F("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?"));
#endif

      //while (true); // error
    } else {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.println(F("187 MIFARE_Read() success: "));
      Serial.print(F("Page: "));
      Serial.println(bufferSize);
      dump_byte_array(buffer, 18);
      Serial.println();
#endif
    }

    page_no = 0x2a;

    status = mfrc522.MIFARE_Read(page_no, buffer, &bufferSize);

    if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.print(F("201 MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      Serial.println(F("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?"));
#endif

      //while (true); // error
    } else {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.println(F("209 MIFARE_Read() success: "));
      Serial.print(F("Page: "));
      Serial.println(bufferSize);
      dump_byte_array(buffer, 18);
      Serial.println();
#endif
    }

    Serial.print(F("page_no:"));

    for (uint8_t k = 0; k < 16; k++) rfid_buffer[k] = 0;
    rfid_buffer[0] = 0x2c;

    Serial.print(F("222 MIFARE_Write() page_no - rfid_buffer: "));
    Serial.print(page_no);
    dump_byte_array(rfid_buffer, 16);
    Serial.println();

    // Write block
    status = mfrc522.MIFARE_Write(page_no, rfid_buffer, 16);

    Serial.print(F("230 MIFARE_Write() result: "));
    Serial.println(mfrc522.GetStatusCodeName(status));

    if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.print(F("235 MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif

      while (true); // error
    } else {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.println(F("242 MIFARE_Write() success: "));
#endif

      status = mfrc522.MIFARE_Read(page_no, rfid_buffer, &bsize);

      if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
        Serial.print(F("249 MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        Serial.println(F("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?"));
#endif

        //while (true); // error
      } else {
#ifdef SERIAL_DEBUG_MFRC522
        Serial.println(F("257 MIFARE_Read() success: "));
        Serial.print(F("Page: "));
        Serial.println(page_no);
        dump_byte_array(rfid_buffer, 18);
        Serial.println();
#endif
      }
    }

    //while (1);
  }

  //Serial.print(F("270 PCD_PerformSelfTest() failed: "));
  //Serial.println(mfrc522.PCD_PerformSelfTest());

  for (uint8_t k = 0; k < 32; k++) buffer[k] = 0;

  // Build command buffer
  //buffer[1] = 0x00;   // 0x1A;
  //buffer[0] = 0x24;   // 0x00;
  //buffer[0] = 0x1A;
  //buffer[1] = 0x00; 00 00 FF 04 FC D4 42 1A 00 D0 00

  buffer[0] = 0x00;
  buffer[1] = 0x00;
  buffer[2] = 0xFF;
  buffer[3] = 0x04;
  buffer[4] = 0xfc;
  buffer[5] = 0xd4;
  buffer[6] = 0x42;
  buffer[7] = 0x1a;
  buffer[8] = 0x00;
  buffer[9] = 0xd0;
  buffer[10] = 0x00;
  buffer[11] = 0x00;

  //status = mfrc522.PCD_CalculateCRC(&buffer[0], 2, &buffer[2]);
  //iso14443a_crc(&buffer[0], 10, &buffer[10]);

  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("280 PCD_CalculateCRC() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  byte validBits = 7;

  Serial.print(F("273 command 1A:"));
  dump_byte_array(buffer, 20);
  Serial.println();

  //status = mfrc522.PCD_TransceiveData(&buffer[0], (byte)4, buffer, &bufferSize, &validBits, (byte)0, true);
  //status = mfrc522.PCD_TransceiveData(&buffer[0], (byte)4, buffer, &bufferSize, &validBits, (byte)0, true);
  status = mfrc522.PCD_TransceiveData(&buffer[0], (byte)4, buffer, &bufferSize, nullptr, (byte)0, true);

  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("280 PCD_TransceiveData() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  Serial.print(F("bufferSize:"));
  Serial.println(bufferSize);
  Serial.print(F("validBits:"));
  Serial.println(validBits);

  Serial.print(F("287 response of PCD_TransceiveData:"));
  dump_byte_array(buffer, bufferSize);
  Serial.println();

  //while (1);


  mfrc522.PCD_DumpVersionToSerial();
  mfrc522.PICC_DumpMifareUltralightToSerial();

  Serial.print(F("297 after of PICC_DumpMifareUltralightToSerial:"));
  Serial.println();

  while (1);

  generate_and_store_crypt_key();

  if (false) {
    // Try the known default keys
    MFRC522::MIFARE_Key key;
    for (byte k = 0; k < NR_KNOWN_KEYS; k++) {
      // Copy the known key into the MIFARE_Key structure
      for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
        key.keyByte[i] = knownKeys[k][i];
      }
      // Try the key
      if (try_key(&key)) {
        // Found and reported on the key and block,
        // no need to try other keys for this PICC
        break;
      }
    }
    Serial.println();
    Serial.println();
  }

  mfrc522.PICC_DumpMifareUltralightToSerial();

  Serial.println();
  Serial.println();

  delay(5000);
}

#define STARTING_PAGE_ULLC 4
#define BYTES_IN_PAGE_ULLC 4

void generate_and_store_crypt_key() {

  randomSeed(analogRead(NOT_CONNECTED_ANALOG_PIN_4_RANDOM_SEED));

  for (uint8_t i = 0; i < 32; i++) {
    rfid_buffer[i] = random(256);

    crypt_key[i] = rfid_buffer[i];
  }

  Serial.print(F("crypt_key:"));
  dump_byte_array(crypt_key, CRYPT_KEY_SIZE);
  Serial.println();

  MFRC522::StatusCode status;

  // write random (256 bit) crypt key in 2 blocks on the rfid chip
  for (uint8_t j = 0; j < 32; j += 4) {
    page_no = j / 4 + 4;

    Serial.print(F("page_no:"));

    for (uint8_t k = 0; k < 16; k++) rfid_buffer[k] = 0;
    for (uint8_t k = 0; k < 4; k++) rfid_buffer[k] = crypt_key[j + k];

    Serial.print(F("359 MIFARE_Write() page_no - rfid_buffer: "));
    Serial.print(page_no);
    dump_byte_array(rfid_buffer, 16);
    Serial.println();

    // Write block
    status = mfrc522.MIFARE_Write(page_no, rfid_buffer, 16);

    Serial.print(F("367 MIFARE_Write() result: "));
    Serial.println(mfrc522.GetStatusCodeName(status));

    if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.print(F("372 MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif

      while (true); // error
    } else {
#ifdef SERIAL_DEBUG_MFRC522
      Serial.println(F("379 MIFARE_Write() success: "));
#endif

      status = mfrc522.MIFARE_Read(page_no, rfid_buffer, &bsize);

      if (status != MFRC522::STATUS_OK) {
#ifdef SERIAL_DEBUG_MFRC522
        Serial.print(F("386 MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        Serial.println(F("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?"));
#endif

        //while (true); // error
      } else {
#ifdef SERIAL_DEBUG_MFRC522
        Serial.println(F("394 MIFARE_Read() success: "));
        Serial.print(F("Page: "));
        Serial.println(page_no);
        dump_byte_array(rfid_buffer, 18);
        Serial.println();
#endif
      }
    }
  }
}

/*

  https://stackoverflow.com/questions/26815834/pn532-mifare-ultralight-c-mutual-authentication

  Yes, it is possible to perform mutual authentication with Mifare Ultralight C. Use the following procedure:

  STEP 1: Send start authentication command. (1A 00) to card

  STEP 2: Card generates an 8 byte random number RndB. This random number is DES/3DES enciphered with the diversified key, denoted by ek(RndB), and is then transmitted to the terminal.

  STEP 3 The terminal runs a DES/3DES deciphering operation on the received ek(RndB) and thus retrieves RndB. RndB is then rotated left by 8 bits (first byte is moved to the end of RndB), yielding RndB’. Now the terminal itself generates an 8 byte random number RndA. This RndA is concatenated with RndB’ and deciphered using DES/3DES (The decryption of the two blocks is chained using the Cipher Block Chaining (CBC) send mode). This token dk(RndA + RndB’) is sent to the card.

  STEP 4: The card runs an DES/3DES encipherment on the received token and thus gains RndA + RndB’. The card can now verify the sent RndB’ by comparing it with the RndB’ obtained by rotating the original RndB left by 8 bits internally. A successful verification proves to the card that the card and the terminal possess the same secret (key). If the verification fails, the card stops the authentication procedure and returns an error message. As the card also received the random number RndA, generated by the terminal, it can perform a rotate left operation by 8 bits on RndA to gain RndA’, which is enciphered again, resulting in ek(RndA’). This token is sent to the terminal.

  STEP 5: The terminal runs a DES/3DES decipherment on the received ek(RndA’) and thus gains RndA’ for comparison with the terminal-internally rotated RndA’. If the comparison fails, the terminal exits the procedure and may halt the card.

  STEP 6: The card sets the authentication state as ‘Authenticated’


  Note: You should have your own implementation of send_command() (depending on your card reader), des_ISO_decrypt() and des_ISO_encrypt() (depending on your DES library).
*/

int mutual_authentication( const byte *diversifiedKey )
{
  byte cmd[256], response[256];
  int cmdLen, responseLen;

  //Send 1A00
  cmd[0] = 0x1A;
  cmd[1] = 0x00;
  cmdLen = 2;
  //int ret = send_command( cmd, cmdLen, response, &responseLen );
  //    StatusCode PCD_TransceiveData(
  //                  byte *sendData,
  //                  byte sendLen,
  //                  byte *backData,
  //                  byte *backLen,
  //                  byte *validBits = nullptr,
  //                  byte rxAlign = 0,
  //                  bool checkCRC = false);

#ifdef TEST_IMITATE_CARD_RESPONSE
  randomSeed(analogRead(NOT_CONNECTED_ANALOG_PIN_4_RANDOM_SEED));

  for (byte i = 0; i < 8; i++) response[i] = random(256);

  des.tripleEncrypt(response, response, diversifiedKey);

  responseLen = 8;
#else
  status = mfrc522.PCD_TransceiveData(cmd, cmdLen, response, responseLen, nullptr, 0, true);
#endif

  //Get ekRndB
  byte ekRndB[8], rndB[8];

  memcpy(ekRndB, response, 8);
  //Decrypt ekRndB with diversifiedKey
  //des_ISO_decrypt( diversifiedKey, ekRndB, rndB, 8 );
  //des.tripleDecrypt(out, in, key);
  des.tripleDecrypt(rndB, ekRndB, diversifiedKey);

  //PCD Generates RndA
  byte rndA[8];
  for (byte i = 0; i < 8; i++) rndA[i] = random(256);
  byte rndARndBComplement[16], ekRndARndBComplement[16];

  // Append RndA and RndB' ( RndB' is generated by rotating RndB one byte to the left )
  // after the status byte.
  memcpy(&rndARndBComplement[0], rndA, 8);
  memcpy(&rndARndBComplement[8], &rndB[1], 7);  // bytes 1 to 7
  rndARndBComplement[15] = rndB[0];           // byte 0

  // Apply the DES send operation to the 16 argument bytes before sending the second frame to the PICC
  //des_ISO_encrypt(diversifiedKey, rndARndBComplement, ekRndARndBComplement, 16);
  //des.tripleEncrypt(out, in, key);
  des.tripleEncrypt(ekRndARndBComplement, rndARndBComplement, diversifiedKey);

  cmd[0] = 0xAF;
  memcpy(&cmd[1], ekRndARndBComplement, 16);
  cmdLen = 17;

#ifdef TEST_IMITATE_CARD_RESPONSE
  des.tripleDecrypt(response, ekRndARndBComplement, diversifiedKey);

  memcpy(&rndARndBComplement[0], &response[1], 7);  // bytes 1 to 7
  rndARndBComplement[7] = response[0];           // byte 0

  des.tripleEncrypt(response, rndARndBComplement, diversifiedKey);

  responseLen = 8;
#else
  //ret = send_command( cmd, cmdLen, response, &responseLen );
  status = mfrc522.PCD_TransceiveData(cmd, cmdLen, response, responseLen, nullptr, 0, true);
#endif

  byte ekRndAComplement[8], rndAComplement[8], finalOutput[8];

  //des_ISO_decrypt(diversifiedKey, ekRndAComplement, rndAComplement, 8);
  //des.tripleDecrypt(out, in, key);
  des.tripleDecrypt(rndAComplement, response, diversifiedKey);

  //memcpy(&ekRndAComplement[0], &response[1], 7);
  //ekRndAComplement[7] = response[0];

  memcpy(&finalOutput[1], &rndAComplement[0], 7);
  finalOutput[0] = rndAComplement[7];

  Serial.print("finalOutput: ");
  dump_byte_array(finalOutput, 8);
  Serial.println();

  Serial.print("rndA: ");
  dump_byte_array(rndA, 8);
  Serial.println();

  //compare the received RndA with the one we originally had
  return memcmp(&finalOutput[0], &rndA[0], 8);
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

/*
   https://stackoverflow.com/questions/40869293/iso-iec-14443a-crc-calcuation
*/

void iso14443a_crc(uint8_t *pbtData, size_t szLen, uint8_t *pbtCrc) {
  uint32_t wCrc = 0x6363;

  do {
    uint8_t  bt;
    bt = *pbtData++;
    bt = (bt ^ (uint8_t)(wCrc & 0x00FF));
    bt = (bt ^ (bt << 4));
    wCrc = (wCrc >> 8) ^ ((uint32_t) bt << 8) ^ ((uint32_t) bt << 3) ^ ((uint32_t) bt >> 4);
  } while (--szLen);

  *pbtCrc++ = (uint8_t)(wCrc & 0xFF);
  *pbtCrc = (uint8_t)((wCrc >> 8) & 0xFF);
}
