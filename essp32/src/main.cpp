#include <Arduino.h>
#include <HardwareSerial.h>

#define BUZZER 23
#define SIM_TX 17
#define SIM_RX 16

HardwareSerial simSerial(2);

bool alert_active = false;

void sendAT(const char* cmd, unsigned long timeout);
void sendSMS(String text);
void makeCall(String number);
void resetSIM();

void setup() {
    pinMode(BUZZER, OUTPUT);
    Serial.begin(115200);
    simSerial.begin(115200, SERIAL_8N1, SIM_RX, SIM_TX);
    Serial.println("ESP32 Alert Ready!");

    delay(3000); 
    sendAT("AT", 1000);
    sendAT("AT+CSQ", 1000);  
    sendAT("AT+CMGF=1", 1000);  
    delay(1000);
}

void sendSMS(String text) {
    Serial.println(" BẮT ĐẦU GỬI SMS ");
    
    sendAT("AT+CMGF=1", 1000);  
    delay(500);
    
    String cmd = "AT+CMGS=\"0364312551\"";
    simSerial.println(cmd);
    Serial.print("Gửi lệnh: ");
    Serial.println(cmd);
    delay(2000);  

    simSerial.print(text);
    Serial.print("Nội dung: ");
    Serial.println(text);
    delay(1000);

    simSerial.write(26);
    Serial.println("Đã gửi Ctrl+Z");
    delay(5000);  

    while (simSerial.available()) {
        String response = simSerial.readString();
        Serial.print("Phản hồi SMS: ");
        Serial.println(response);
    }
    
    Serial.println(" KẾT THÚC GỬI SMS ");
    resetSIM(); 
}

void makeCall(String number) {
    Serial.println(" BẮT ĐẦU GỌI ĐIỆN ");
    
    String cmd = "ATD" + number + ";";
    simSerial.println(cmd);
    Serial.print("Gửi lệnh: ");
    Serial.println(cmd);
    
    delay(15000);  
    
    sendAT("ATH", 1000); 
    Serial.println("Đã kết thúc cuộc gọi!");
    
    Serial.println(" KẾT THÚC GỌI ĐIỆN ");
    resetSIM();  
}

void resetSIM() {
    Serial.println(" Reset trạng thái SIM ");
    delay(1000);

    while (simSerial.available()) {
        simSerial.read();
    }

    sendAT("AT", 1000);
    sendAT("AT+CMGF=1", 1000); 
    delay(1000);
    
    Serial.println(" Hoàn tất reset ");
}

void sendAT(const char* cmd, unsigned long timeout) {
    simSerial.println(cmd);
    Serial.print("Gửi lệnh: ");
    Serial.println(cmd);
    
    unsigned long start = millis();
    while (millis() - start < timeout) {
        if (simSerial.available()) {
            String response = simSerial.readString();
            Serial.print("Phản hồi: ");
            Serial.println(response);
        }
    }
}

void triggerAlert() {
    alert_active = true;

    digitalWrite(BUZZER, HIGH);
    Serial.println("Cảnh báo! Còi kêu...");
    delay(3000);
    digitalWrite(BUZZER, LOW);
    Serial.println("Còi tắt.");
    
    delay(2000);  

    Serial.println("\n>>> BƯỚC 1: GỬI SMS");
    sendSMS("Canh bao! Muc nuoc vuot nguong an toan.");
    
    delay(5000); 

    Serial.println("\n>>> BƯỚC 2: GỌI ĐIỆN");
    makeCall("0364312551");

    delay(10000);
    Serial.println("\nHoàn tất cảnh báo! \n");
    alert_active = false;
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        if (c == '1' && !alert_active) {
            triggerAlert();
        }
    }

    if (simSerial.available()) {
        String response = simSerial.readString();
        Serial.print("[SIM]: ");
        Serial.println(response);
    }
}