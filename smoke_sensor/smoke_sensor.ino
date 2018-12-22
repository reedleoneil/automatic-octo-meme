#include <SPI.h>
#include <RF24.h>								// https://github.com/nRF24/RF24
#include <RF24Network.h>							// https://github.com/nRF24/RF24Network
#include <OneWire.h>                                                            // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h>                                                  // https://github.com/milesburton/Arduino-Temperature-Control-Library

RF24 radio(10, 9); 							        // nRF24L01 (CE,CSN)
RF24Network network(radio); 							// Include the radio in the network

const uint16_t _node_address = 01; 						// Address of this node in Octal format ( 04,031, etc)
const uint8_t _channel = 90; 							// Radio frequency channel to operate

const uint16_t _parent_address = 00;						// Address of parent node in Octal format ( 04,031, etc)

const unsigned long interval = 1000; //ms 					// How often to send data to the other unit
unsigned long last_sent; 							// When did we last send?

OneWire oneWire(2);
DallasTemperature sensors(&oneWire);
float _celcius = 0;

void setup() {
        //Serial.begin(9600);
	radio.begin();								// Begin operation of the nRF24L01
        radio.setDataRate(RF24_2MBPS);                                          // Set data rate
	network.begin(_channel, _node_address); 				// Begin network (channel, node address)
        sensors.begin();
}

void loop() {
	network.update();							// Pump the network regularly
	unsigned long now = millis();
	if (now - last_sent >= interval) { 					// If it's time to send a data, send it!
		last_sent = now;
		unsigned long smoke_sensor = analogRead(A0);		        // Read value from sensor
                unsigned long temp_sensor = analogRead(A1);			// Read value from sensor
                sensors.requestTemperatures(); 
                temp_sensor=sensors.getTempCByIndex(0);				
		char data[8];							// Data
		/* Construct payload */
                String payload = String(map(smoke_sensor, 0, 1023, 0, 255))     // Map sensor value from 1024 to 256 for smoke sensor value
                                 + " " + 
                                 String(temp_sensor);	                        // Temp sensor reading
                payload.toCharArray(data, 8);					// Convert payload to char array                        
		RF24NetworkHeader header1(_parent_address);			// Construct a header with node address of recipient 		
		network.write(header1, &data, sizeof(data)); 			// Send the data
                Serial.println(data);
	}
}
