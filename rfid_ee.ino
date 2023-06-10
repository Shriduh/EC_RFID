#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);

struct Student {
  String cardId;
  bool isInside;
  unsigned long enterTime;
  unsigned long totalDuration;
};

const int MAX_STUDENTS = 10;
Student students[MAX_STUDENTS];
int numStudents = 0;

String getCardId() {
  String cardId = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardId += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    cardId += String(mfrc522.uid.uidByte[i], HEX);
  }
  cardId.toUpperCase();
  return cardId;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("RFID Attendance System");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String cardId = getCardId();

    bool isRegistered = false;
    int studentIndex = -1;
    for (int i = 0; i < numStudents; i++) {
      if (cardId == students[i].cardId) {
        isRegistered = true;
        studentIndex = i;
        break;
      }
    }

    if (isRegistered) {
      if (students[studentIndex].isInside) {
        Serial.println("Student left the class. Card ID: " + cardId);
        students[studentIndex].isInside = false;

        unsigned long leaveTime = millis() / 1000; // Leave time in seconds
        unsigned long duration = leaveTime - students[studentIndex].enterTime;
        students[studentIndex].totalDuration += duration;
        sendAttendanceData(cardId, "Left", students[studentIndex].enterTime, leaveTime, students[studentIndex].totalDuration);
      } else {
        Serial.println("Student entered the class. Card ID: " + cardId);
        students[studentIndex].isInside = true;
        students[studentIndex].enterTime = millis() / 1000; // Enter time in seconds
        sendAttendanceData(cardId, "Entered", students[studentIndex].enterTime, 0, students[studentIndex].totalDuration);
      }
    } else {
      if (numStudents < MAX_STUDENTS) {
        students[numStudents].cardId = cardId;
        students[numStudents].isInside = true;
        students[numStudents].enterTime = millis() / 1000; // Enter time in seconds
        students[numStudents].totalDuration = 0;
        Serial.println("Student entered the class. Card ID: " + cardId);
        sendAttendanceData(cardId, "Entered", students[numStudents].enterTime, 0, students[numStudents].totalDuration);
        numStudents++;
      } else {
        Serial.println("Maximum number of students reached. Cannot register new student.");
      }
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
}

void sendAttendanceData(String cardId, String status, unsigned long enterTime, unsigned long leaveTime, unsigned long totalDuration) {
  Serial.print(cardId + "," + status + "," + String(enterTime) + "," + String(leaveTime) + "," + String(totalDuration) + "\n");
}
