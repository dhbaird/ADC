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

    // enable DMA and interrupts
    adc->enableDMA();

    // ADC interrupt enabled isn't mandatory for DMA to work.
    adc->enableInterrupts(ADC_NUM::ADC_0);

    //dmaBuffer.start();
    dmaBuffer.start();

    // setup ADC internal settings
    adc->analogRead(readPin, ADC_NUM::ADC_0);
    adc->adc0->stopPDB();
    // At 2 Hz it's easy to see the buffer being filled.
    adc->adc0->startPDB(2);
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
            Serial.println(dmaBuffer.isFull());
            Serial.print("size(): ");
            Serial.println(dmaBuffer.size());
            printBuffer();
        } else if(c=='i') {
            dmaBuffer.add_interrupt(&dmaBuffer_isr);
        }
  }


    //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN));
    delay(100);
}

void dmaBuffer_isr() {
    //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN));
    Serial.println("dmaBuffer_isr");
    // update the internal buffer positions
    dmaBuffer.dmaChannel.clearInterrupt();
}


// it can be called everytime a new value is converted. The DMA isr is called first
void adc0_isr(void) {
    //int t = micros();
    Serial.println("ADC0_ISR"); //Serial.println(t);
    adc->adc0->readSingle(); // clear interrupt
}


void printBuffer() {
    Serial.println("Buffer: Address, Value");

    uint8_t i = 0;
    // we can get this info from the dmaBuffer object, even though we should have it already
    volatile int16_t* buffer = dmaBuffer.buffer();
    for (i = 0; i < buffer_size; i++) {
        Serial.print(uint32_t(&buffer[i]), HEX);
        Serial.print(", ");
        Serial.println(buffer[i]);
    }
//
//    Serial.print("Current pos: ");
//    Serial.println(uint32_t(dmaBuffer->dmaChannel->destinationAddress()), HEX);
//
//    Serial.print("p_elems: ");
//    Serial.println(uint32_t(dmaBuffer->p_elems), HEX);
//
//    Serial.print("b_start: ");
//    Serial.println(dmaBuffer->b_start);
//    Serial.print("b_end: ");
//    Serial.println(dmaBuffer->b_end);

}

