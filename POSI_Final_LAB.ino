#include "ThingSpeak.h"
#include <DHT.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char ssid[] = "Leptosoma";
const char password[] = "kotabaru";
const char server[] = "api.thingspeak.com";
unsigned long myChannelNumber = 2004514;
WiFiClient client;
const char *myWriteAPIKey = "[thingspeak API Key]";
void check_thresh();

// Pins
const int relay1 = 18; // Atomizer
const int relay2 = 19; // Fan
const int DHTPIN = 26; // DHT11

// Keypad BEGIN
const byte baris = 4;
const byte kolom = 3;
// declare value of each matrix
const char keys[baris][kolom] = {
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'*', '0', '#'}};
byte pinBaris[baris] = {13, 12, 14, 27};
byte pinKolom[kolom] = {25, 33, 32};
// Keypad Object
Keypad keypad = Keypad(makeKeymap(keys), pinBaris, pinKolom, baris, kolom);
// Keypad END

// DHT Start
float hum = 0.0;
float temp = 0.0;
DHT dht(DHTPIN, DHT11);
// Threshold Start
int t_hum = 30;
int t_temp = 50;

void update_dht() {
	hum = dht.readHumidity();
	temp = dht.readTemperature();
	//  hum = 1;
	//  temp = 1;
	Serial.print("[DATA] Current: ");
	Serial.print(hum);
	Serial.print(" ");
	Serial.println(temp);
	sendData(t_hum, hum, temp, t_temp);
}

// Menu Start
int current_menu = 1;
void next_menu() {
	if (current_menu == 3) {
		current_menu = 1;
	} else {
		current_menu++;
	}
}
void menu_1() {
	Serial.println("[MENU] Menu 1: Show Stats");
	update_dht();
	check_thresh();
	lcd.setCursor(0, 0);
	lcd.print("HUMIDITY  :");
	lcd.print(hum);
	lcd.print("%");
	lcd.setCursor(0, 1);
	lcd.print("TEMPERATURE:");
	lcd.print(temp);
	lcd.print("C");
}
void menu_2() {
	Serial.println("[MENU] Menu 2: Input Humidity");
	lcd.setCursor(0, 0);
	lcd.print("   INPUT");
	lcd.setCursor(0, 1);
	lcd.print("HUMIDITY   :");
}
void menu_3() {
	Serial.println("[MENU] Menu 2: Input Temperature");
	lcd.setCursor(0, 0);
	lcd.print("   INPUT");
	lcd.setCursor(0, 1);
	lcd.print("TEMPERATURE:");
}
void update_menu() {
	switch (current_menu) {
	case 1:
		menu_1();
		break;
	case 2:
		menu_2();
		break;
	case 3:
		menu_3();
		break;
	default:
		break;
	}
}

void update_hum(int humi) {
	Serial.print("[DATA] Hum threshold updated to: ");
	t_hum = humi;
	Serial.println(t_hum);
	sendData(t_hum, hum, temp, t_temp);
}
void update_temp(int tempi) {
	Serial.print("[DATA] Temp threshold updated to: ");
	t_temp = tempi;
	Serial.println(t_temp);
	sendData(t_hum, hum, temp, t_temp);
}

void sendData(float tarhum, float hum, float temp, float tartemp) {
	ThingSpeak.setField(1, tarhum);
	ThingSpeak.setField(2, hum);
	ThingSpeak.setField(3, temp);
	ThingSpeak.setField(4, tartemp);
	int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
	if (x == 200) {
		Serial.println("Channel update successful.");
	} else {
		Serial.println("Problem updating channel. HTTP error code " + String(x));
	}
	Serial.println("Waiting 20 secs");
}

void check_thresh() {
	Serial.print("[DATA] Threshold: ");
	Serial.print(t_hum);
	Serial.print(" ");
	Serial.println(t_temp);
	if (hum < t_hum) {
		digitalWrite(relay1, HIGH);
		Serial.println("[TRIG] Relay 1 (Atomizer) On");
	} else {
		digitalWrite(relay1, LOW);
		Serial.println("[TRIG] Relay 1 (Atomizer) Off");
	}
	if (temp > t_temp) {
		digitalWrite(relay2, HIGH);
		Serial.println("[TRIG] Relay 2 (Fan) On");
	} else {
		digitalWrite(relay2, LOW);
		Serial.println("[TRIG] Relay 2 (Fan) Off");
	}
}

void take_input() {
	Serial.print("[TIPS] Taking Input...");
	char key = keypad.getKey();
	char angka[10];
	int hum = t_hum;
	int temp = t_temp;
	int i = 0;
	while (1) {
		key = keypad.getKey();
		if (key && key != '*' && key != '#') {
			angka[i] = key;
			lcd.setCursor(12 + i, 1);
			lcd.print(key);
			Serial.print(angka[i]);
			i++;
		} else if (key == '*' || key == '#') {
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Input Complete!");
			Serial.println();
			Serial.print("[TIPS] Input Complete!");
			break;
		} else if (key) {
			Serial.println("[TIPS] Invalid Key!");
		}
	}
	Serial.println();
	switch (current_menu) {
	case 2:
		Serial.print("[DATA] Inputting Humidity: ");
		lcd.setCursor(0, 1);
		for (int j = 0; j < i; j++) {
			Serial.print(angka[j]);
			lcd.print(angka[j]);
			lcd.setCursor(j + 1, 1);
		}
		lcd.print("%");
		Serial.println("%");
		t_hum = atoi(angka);
		Serial.print("[DATA]: t_hum = ");
		Serial.println(t_hum);
		update_hum(t_hum); // TODO: Change
		break;
	case 3:
		Serial.print("[DATA] Inputting Temperature: ");
		lcd.setCursor(0, 1);
		for (int j = 0; j < i; j++) {
			Serial.print(angka[j]);
			lcd.print(angka[j]);
			lcd.setCursor(j + 1, 1);
		}
		lcd.print("C");
		Serial.println("C");
		t_temp = atoi(angka);
		Serial.print("[DATA]: t_temp = ");
		Serial.println(t_temp);
		update_temp(t_temp); // TODO: Change
		break;
	default:
		break;
	}
	current_menu = 1;
}

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	Serial.println("[BOOT] Hello, ESP32!");
	pinMode(relay1, OUTPUT);
	pinMode(relay2, OUTPUT);
	digitalWrite(relay1, LOW);
	digitalWrite(relay2, LOW);
	dht.begin();
	WiFi.begin(ssid, password);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");
	ThingSpeak.begin(client);

	// LCD Setup
	lcd.init();
	lcd.backlight();
	lcd.clear();
	update_menu();
	Serial.println("[BOOT] Boot Complete!");
}

void loop() {
	unsigned long multitas = millis();
	if (multitas % 300000 == 0 & multitas > 0) {
		update_dht();
		menu_1();
	}
	char key = keypad.getKey();
	if (key) {
		switch (key) {
		case '*':
			if (current_menu != 1) {
				take_input();
				delay(2000);
				lcd.clear();
				update_menu();
			} else {
				Serial.println("[TIPS] Cannot take input on menu1!");
			}
			break;
		case '#': // Menu Cycle
			next_menu();
			lcd.clear();
			update_menu();
			break;
		default:
			Serial.println("[TIPS] Press '*' key to input, '#' to Cycle Menu");
			break;
		}
	}
	delay(10);
}
