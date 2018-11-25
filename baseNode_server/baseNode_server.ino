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
                uint8_t smoke = payload.substring(0, payload.indexOf(" ")).toInt();
                //uint8_t temp = payload.substring(payload.indexOf(" ") + 1, payload.length()).toInt();
                uint8_t temp = map(payload.substring(payload.indexOf(" ") + 1, payload.length()).toInt(), 0, 1024, 0, 256);
		
                // Set value for output
		uint8_t pin;						
		uint8_t value = String(data).toInt() >= _threshold ? HIGH : LOW;

		switch (header.from_node) {			// Handle header and process data
			case 1:    				
				_smoke_sensor1 = smoke;		// Save value from smoke sensor 1
                                _temp_sensor1 = temp;		// Save value from temp sensor 1
				pin = _out1;			// Set output
		      		break;
		    	case 2:    				
				_smoke_sensor2 = smoke;		// Save value from smoke sensor 2
                                _temp_sensor2 = temp;		// Save value from temp sensor 2
		      		pin = _out2;			// Set output
		      		break;
		    	case 3:    				
				_smoke_sensor3 = smoke;		// Save value from smoke sensor 3
                                _temp_sensor3 = temp;		// Save value from temp sensor 3
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
						", \"alert\": " + String(true) + " }");
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
