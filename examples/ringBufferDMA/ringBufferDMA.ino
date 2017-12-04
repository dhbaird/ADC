/*
*   It doesn't work for Teensy LC yet!
*/

#include "ADC.h"
#include "RingBufferDMA.h"

const int readPin = A9;
ADC *adc = new ADC(); // adc object

const uint8_t buffer_size = 8;
// use dma with ADC0
RingBufferDMA<buffer_size> dmaBuffer(ADC_NUM::ADC_0);


void setup() {

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(readPin, INPUT); //pin 23 single ended

    Serial.begin(9600);

    adc->setAveraging(ADC_AVERAGES::_8); // set number of averages
    adc->setResolution(ADC_RESOLUTION::_12); // set bits of resolution

    // enable DMA
    adc->enableDMA();

    // Start the buffer and wait for the conversions
    dmaBuffer.start();

    #if ADC_USE_PDB
    // setup ADC internal settings
    adc->analogRead(readPin, ADC_NUM::ADC_0);
    adc->adc0->stopPDB();
    // At 2 Hz it's easy to see the buffer being filled.
    adc->adc0->startPDB(2);
    #endif // ADC_USE_PDB
}

char c=0;



void loop() {

    if (Serial.available()) {
        c = Serial.read();
        if(c=='c') { // start conversion
            Serial.println("Conversion.");
            adc->analogRead(readPin, ADC_NUM::ADC_0);
        } else if(c=='p') { // print buffer
            Serial.print("Current destination address: ");
            Serial.println(reinterpret_cast<uint32_t>(dmaBuffer.dmaChannel.destinationAddress()), HEX);
            #if defined(KINETISL)
            Serial.print("DSR: ");
            Serial.println(dmaBuffer.dmaChannel.CFG->DSR_BCR>>24, BIN);
            Serial.print("BCR: ");
            Serial.println(dmaBuffer.dmaChannel.CFG->DSR_BCR&0x00FFFFFF);
            Serial.print("DCR: ");
            Serial.println(dmaBuffer.dmaChannel.CFG->DCR, HEX);
            #endif
            printBuffer();
        } else if(c=='i') {
            dmaBuffer.attachInterrupt(&dmaBuffer_isr);
        }
    }


    digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN));
    delay(100);
}

// This isr will be called if you set it up with attachInterrupt for Teensy 3.x (not mandatory)
// For Teensy LC it is NECESSARY for the isr to clear the interrupt and reset the number of bytes to transfer
// The example below shows how to do it
void dmaBuffer_isr() {
    // clear the interrupt
    // FOR TEENSY LC IT RESETS THE NUMBER OF TRANSFERS (IN BYTES), OTHERWISE THE BUFFER WON'T CONTINUE WORKING WHEN IT'S FULL
    // YOU MUST CALL THIS (OR INLINE THE CODE DIRECTLY).
    dmaBuffer.clearInterrupt();

    // You can do some other stuff afterwards
    Serial.println("dmaBuffer_isr");
}


void printBuffer() {
    Serial.println("Buffer: Address, Value");
    volatile uint16_t* buffer = dmaBuffer.buffer();
    for (uint32_t i = 0; i < buffer_size; i++) {
        Serial.print(uint32_t(&buffer[i]), HEX);
        Serial.print(", ");
        Serial.println(buffer[i]);
    }
}

