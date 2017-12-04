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

    #if defined(KINETISL)
    // This interrupt is necessary for Teensy LC
    dmaBuffer.attachInterrupt(&dmaBuffer_isr);
    #endif // only for Teensy LC

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
            Serial.print("isFull(): ");
            Serial.println(dmaBuffer.isFull() ? "True" : "False");
            Serial.print("num_elems: ");
            Serial.println(dmaBuffer.num_elems());
            Serial.print("destinationAddress: ");
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


    //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN));
    delay(100);
}

void dmaBuffer_isr() {
    Serial.println("dmaBuffer_isr");
    // clear the interrupt
    dmaBuffer.dmaChannel.clearInterrupt();

    // RESET NUMBER OF TRANSFERS (IN BYTES) FOR TEENSY LC, OTHERWISE THE BUFFER WON'T CONTINUE WORKING WHEN IT'S FULL
    #if defined(KINETISL)
    dmaBuffer.dmaChannel.CFG->DSR_BCR = buffer_size*sizeof(uint16_t);
    #endif // defined

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

