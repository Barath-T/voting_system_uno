#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <Keypad.h>

#define F_TX 2
#define F_RX 3
#define UN_TX 4
#define UN_RX 5

uint8_t getFingerprintEnroll(uint8_t id);
int8_t getFingerprintID();

SoftwareSerial mySerial(F_RX, F_TX);
SoftwareSerial uno_node_serial(UN_RX, UN_TX);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t keypad_pins[2][4] = {{9, 8, 7, 6}, {13, 12, 11, 10}};
uint8_t keypad_matrix[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

Keypad keypad = Keypad(makeKeymap(keypad_matrix), keypad_pins[1], keypad_pins[0], 4, 4);

uint8_t STATE = 0; // 0 - normal, 1 - admin
uint8_t id = 2;

void setup()
{
  Serial.begin(9600);
  uno_node_serial.begin(9600);
  finger.begin(57600);

  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
  }
  else
  {
    Serial.println("Did not find fingerprint sensor");
  }

  // keypad
  for (int i = 0; i < 4; i++)
  {
    pinMode(keypad_pins[0][i], INPUT);
    pinMode(keypad_pins[1][i], OUTPUT);
  }
}

void loop()
{
  // if (uno_node_serial.available() > 0)
  // {
  //   Serial.println("starting");
  //   String response = uno_node_serial.readStringUntil('\n');
  //   Serial.println(response);
  // }
  char key = keypad.getKey();
  if (key != NO_KEY)
  {
    switch (key)
    {
    case '1':
    case '2':
    case '3':
      if (STATE == 1)
      {
        switch (key)
        {
        case '1':
          Serial.println("waiting for admin's fingerprint..");
          if (getFingerprintID() == 1)
          {
            delay(2000);
            Serial.println("waiting for new fingerprint..");
            getFingerprintEnroll(id++);
          }
          else
          {
            Serial.println("wrong admin fingerprint!");
          }
          break;
        case '2':
          finger.emptyDatabase();
          Serial.println("emptied database");
          break;
        }
      }
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '0':
      if (STATE == 0)
      {
        int8_t curr_fingerprint = getFingerprintID();
        Serial.println(curr_fingerprint);
        if (curr_fingerprint != -1)
        {
          char string_to_send[50];
          int str_len = sprintf(string_to_send, "vote,%c,%d\n", key, curr_fingerprint);
          if (str_len != -1)
          {
            uno_node_serial.write(string_to_send);
          }
          while (1)
          {
            delay(5000);
            if (uno_node_serial.available() > 0)
            {
              Serial.println("eyererere");
              String response = uno_node_serial.readStringUntil('\n');
              if (response == "201")
              {
                Serial.print("voted for ");
                Serial.println(key);
              }
              else
              {
                Serial.println("vote not valid");
              }
              break;
            }
          }
          // Serial.print("voted for ");
          // Serial.println(key);
        }
      }
      break;
    case 'A':
      Serial.println("waiting for admin's fingerprint..");
      if (getFingerprintID() == 1)
      {
        STATE = !STATE;
        Serial.print("current state: ");
        Serial.print(STATE);
        Serial.println();
      }
      else
      {
        Serial.println("wrong admin fingerprint!");
      }

      break;
    default:
      getFingerprintEnroll(1);
      break;
    }
  }
}
uint8_t deleteFingerprint(uint8_t id)
{
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK)
  {
    Serial.println("Deleted!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not delete in that location");
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
  }
  else
  {
    Serial.print("Unknown error: 0x");
    Serial.println(p, HEX);
  }

  return p;
}
uint8_t getFingerprintEnroll(uint8_t id)
{

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Stored!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}
int8_t getFingerprintID()
{
  int8_t p = -1;
  // while (p != FINGERPRINT_OK)
  //{
  p = finger.getImage();
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image taken");
    break;
  case FINGERPRINT_NOFINGER:
    Serial.println("No finger detected");
    return -1;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return -1;
  case FINGERPRINT_IMAGEFAIL:
    Serial.println("Imaging error");
    return -1;
  default:
    Serial.println("Unknown error");
    return -1;
  }
  //}

  // OK success!

  p = finger.image2Tz();
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return -1;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return -1;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return -1;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return -1;
  default:
    Serial.println("Unknown error");
    return -1;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Found a print match!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return -1;
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    Serial.println("Did not find a match");
    return -1;
  }
  else
  {
    Serial.println("Unknown error");
    return -1;
  }

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);

  return finger.fingerID;
}