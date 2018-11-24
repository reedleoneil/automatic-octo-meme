#include <SPI.h>
#include <Ethernet.h>
#include <RF24.h>						// https://github.com/nRF24/RF24
#include <RF24Network.h>					// https://github.com/nRF24/RF24Network
#include <DigitalIO.h>						// https://github.com/greiman/DigitalIO

const uint8_t _out1 = 22;					// Led 1
const uint8_t _out2 = 24;					// Led 2
const uint8_t _out3 = 26;					// Led 3

uint8_t _sensor1 = 0;						// Value of sensor 1			
uint8_t _sensor2 = 0;						// Value of sensor 2
uint8_t _sensor3 = 0;						// Value of sensor 3

const uint8_t _threshold = 40;					// Threshold value for sensors

RF24 radio(31, 33); 				                // nRF24L01 (CE,CSN)
RF24Network network(radio); 					// Include the radio in the network

const uint16_t _node_address = 00; 				// Address of this node in Octal format ( 04,031, etc)
const uint8_t _channel = 90; 					// Radio frequency channel to operate

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };		// MAC Address
IPAddress ip(192, 168, 10, 177);					// IP Address
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
		unsigned long data;				// Data
		network.read(header, &data, sizeof(data)); 	// Read the incoming data
		
		uint8_t pin;					// Output pin	
		uint8_t value = data >= _threshold ? HIGH : LOW;// Value of output	

		switch (header.from_node) {			// Handle header and process data
			case 1:    				
				_sensor1 = data;		// Save value from sensor 1
				pin = _out1;			// Set output
		      		break;
		    	case 2:    				
				_sensor2 = data;		// Save value from sensor 2
		      		pin = _out2;			// Set output
		      		break;
		    	case 3:    				
		      		_sensor3 = data;		// Save value from sensor 3
		      		pin = _out3;			// Set output
				break;
		}

		digitalWrite(pin, value);			// Write value to output
	}     
}

void ethernet() {									// Ethernet
	EthernetClient client = server.available();					// Listen for incoming clients
	if (client) {									
		bool currentLineIsBlank = true;
		while (client.connected()) {
			if (client.available()) {
				char c = client.read();
				if (c == '\n' && currentLineIsBlank) {
					client.println("HTTP/1.1 200 OK");		// Send a standard http response header
					client.println("Content-Type: text/html");
                                        client.println("Access-Control-Allow-Origin: *");
					client.println("Connection: close");
					client.println();
	  				client.println(					// Payload in JSON format
						"{ \"sensor1\": " + String(_sensor1) + 	
						", \"sensor2\": " + String(_sensor2) + 
						", \"sensor3\": " + String(_sensor3) + " }");
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
