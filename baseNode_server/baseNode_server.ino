#include <SPI.h>
#include <Ethernet.h>
#include <RF24.h>						// https://github.com/nRF24/RF24
#include <RF24Network.h>					// https://github.com/nRF24/RF24Network
#include <DigitalIO.h>						// https://github.com/greiman/DigitalIO

const uint8_t _out1 = 22;					// Led 1
const uint8_t _out2 = 24;					// Led 2
const uint8_t _out3 = 26;					// Led 3

uint8_t _smoke_sensor1 = 0;					// Value of smoke sensor 1			
uint8_t _smoke_sensor2 = 0;					// Value of smoke sensor 2
uint8_t _smoke_sensor3 = 0;					// Value of smoke sensor 3

uint8_t _temp_sensor1 = 0;					// Value of temp sensor 1			
uint8_t _temp_sensor2 = 0;					// Value of temp sensor 2
uint8_t _temp_sensor3 = 0;					// Value of temp sensor 3

const uint8_t _threshold = 40;					// Threshold value for smoke sensors
const unsigned long _interval = 5000; //ms 			// Time to detect smoke

unsigned long _s1_last_detected_normal;				// Smoke sensor 1 time last detected normal
unsigned long _s2_last_detected_normal;				// Smoke sensor 2 time last detected normal
unsigned long _s3_last_detected_normal;				// Smoke sensor 3 time last detected normal

unsigned long _s1_last_detected_threshold;			// Smoke sensor 1 time last detected threshold
unsigned long _s2_last_detected_threshold;			// Smoke sensor 2 time last detected threshold
unsigned long _s3_last_detected_threshold;			// Smoke sensor 3 time last detected threshold

bool _s1_triggered;						// Sensor 1: is smoke detected?
bool _s2_triggered;						// Sensor 2: is smoke detected?
bool _s3_triggered;						// Sensor 3: is smoke detected?

RF24 radio(31, 33); 				                // nRF24L01 (CE,CSN)
RF24Network network(radio); 					// Include the radio in the network

const uint16_t _node_address = 00; 				// Address of this node in Octal format ( 04,031, etc)
const uint8_t _channel = 90; 					// Radio frequency channel to operate

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };		// MAC Address
IPAddress ip(192, 168, 10, 177);				// IP Address
EthernetServer server(80);					// Initialize the Ethernet on port 80 

void setup() 							// Setup
{
  	Serial.begin(9600);					// Open serial communication (baud rate)
	radio.begin();						// Begin operation of the nRF24L01
        radio.setDataRate(RF24_2MBPS);                          // Set data rate
	network.begin(_channel, _node_address); 		// Begin network (channel, node address)
	Ethernet.begin(mac, ip);				// Start ethernet connection
	server.begin();						// Start the server

	pinMode(_out1, OUTPUT);					// Set pin mode of out 1
	pinMode(_out2, OUTPUT);					// Set pin mode of out 2
	pinMode(_out3, OUTPUT);					// Set pin mode of out 3
}

void loop() {							// Loop
	rf24();							// RF24
	ethernet();						// Ethernet
}

void rf24() {							// RF24
	network.update();					// Pump the network regularly
	while ( network.available() ) { 			// Is there any incoming data?
		RF24NetworkHeader header;			// Construct a blank header
		char data[8];				        // Data
		network.read(header, &data, sizeof(data)); 	// Read the incoming data

                // Parse data
                String payload(data);
                uint8_t smoke = payload.substring(0, payload.indexOf(" ")).toInt();
                //uint8_t temp = payload.substring(payload.indexOf(" ") + 1, payload.length()).toInt();
                uint8_t temp = map(payload.substring(payload.indexOf(" ") + 1, payload.length()).toInt(), 0, 1024, 0, 256);
	
		unsigned long now = millis();			

		switch (header.from_node) {			// Handle header and process data
			case 1:    				
				_smoke_sensor1 = smoke;		// Save value from smoke sensor 1
                                _temp_sensor1 = temp;		// Save value from temp sensor 1
				if (_smoke_sensor1 >= _threshold) {
                			_s1_last_detected_threshold = now;
                			if (now - _s1_last_detected_normal >= _interval) {
                				_s1_triggered = true;
                				digitalWrite(_out1, HIGH);			// Write value to output
                			}
                		} else {
                			_s1_last_detected_normal = now;
                			if (now - _s1_last_detected_threshold >= _interval) {
                				_s1_triggered = false;
                				digitalWrite(_out1, LOW);			// Write value to output
                			}
                		}
		      		break;
		    	case 2:    				
				_smoke_sensor2 = smoke;		// Save value from smoke sensor 2
                                _temp_sensor2 = temp;		// Save value from temp sensor 2
				if (_smoke_sensor2 >= _threshold) {
                			_s2_last_detected_threshold = now;
                			if (now - _s2_last_detected_normal >= _interval) {
                				_s2_triggered = true;
                				digitalWrite(_out2, HIGH);			// Write value to output
                			}
                		} else {
                			_s2_last_detected_normal = now;
                			if (now - _s2_last_detected_threshold >= _interval) {
                				_s2_triggered = false;
                				digitalWrite(_out2, LOW);			// Write value to output
                			}
                		}
		      		break;
		    	case 3:    				
				_smoke_sensor3 = smoke;		// Save value from smoke sensor 3
                                _temp_sensor3 = temp;		// Save value from temp sensor 3
				if (_smoke_sensor3 >= _threshold) {
                			_s3_last_detected_threshold = now;
                			if (now - _s3_last_detected_normal >= _interval) {
                				_s3_triggered = true;
                				digitalWrite(_out3, HIGH);			// Write value to output
                			}
                		} else {
                			_s3_last_detected_normal = now;
                			if (now - _s3_last_detected_threshold >= _interval) {
                				_s3_triggered = false;
                				digitalWrite(_out3, LOW);			// Write value to output
                			}
                		}
				break;
		}
	}     
}

void ethernet() {									// Ethernet
	EthernetClient client = server.available();					// Listen for incoming clients
	if (client) {									
		bool currentLineIsBlank = true;
		while (client.connected()) {
			if (client.available()) {
				char c = client.read();
                                //Serial.print(c);
				if (c == '\n' && currentLineIsBlank) {
					client.println("HTTP/1.1 200 OK");		// Send a standard http response header
					client.println("Content-Type: text/html");
                                        client.println("Access-Control-Allow-Origin: *");
					client.println("Connection: close");
					client.println();
	  				client.println(					// Payload in JSON format
						"{ \"smoke_sensor1\": " + String(_smoke_sensor1) + 	
						", \"smoke_sensor2\": " + String(_smoke_sensor2) + 
                                                ", \"smoke_sensor3\": " + String(_smoke_sensor3) + 
                                                ", \"temp_sensor1\": " + String(_temp_sensor1) + 
                                                ", \"temp_sensor2\": " + String(_temp_sensor2) + 
                                                ", \"temp_sensor3\": " + String(_temp_sensor3) + 
                                                ", \"s1_triggered\": " + String(_s1_triggered) + 
                                                ", \"s2_triggered\": " + String(_s2_triggered) + 
						", \"s3_triggered\": " + String(_s3_triggered) + " }");
				  	break;
				}
				if (c == '\n') {
					currentLineIsBlank = true;
				} else if (c != '\r') {
					currentLineIsBlank = false;
				}
      			}
	    	}
	    	delay(1);
	    	client.stop();
	}
}
