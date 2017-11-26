#include "ADC.h"
#include "RingBuffer.h"

const uint8_t readPin = A9;

ADC *adc = new ADC(); // adc object

const uint32_t buffer_size = 8;
RingBuffer<buffer_size> *buffer = new RingBuffer<buffer_size>;


void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
}

int16_t value = 0;
char c=0;

void loop() {

    value = adc->analogRead(readPin);

    if (Serial.available()) {
        c = Serial.read();
        if(c=='r') { // read buffer
            Serial.print("Read: ");
            Serial.println(buffer->read());
        } else if(c=='f') { // full?
            Serial.print("isFull(): ");
            Serial.println(buffer->isFull());
        } else if(c=='e') { // empty?
            Serial.print("isEmpty(): ");
            Serial.println(buffer->isEmpty());
        } else if(c=='p'){
            for(uint32_t i = 0; i<buffer_size; i++) {
                Serial.print(i); Serial.print(": "); Serial.println(buffer->elems[i]);
            }
        } else if(c=='w') { // empty?
            Serial.println("Write value.");
            buffer->write(value);
        }
    }

    delay(100);
}

