/* Teensy 3.x, LC ADC library
 * https://github.com/pedvide/ADC
 * Copyright (c) 2017 Pedro Villanueva
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* ADC.cpp: Implements the control of one or more ADC modules of Teensy 3.x, LC
 *
 */

#include "ADC.h"



// translate pin number to SC1A nomenclature and viceversa
// we need to create this static const arrays so that we can assign the "normal arrays" to the correct one
// depending on which ADC module we will be.
/* channel2sc1aADCx converts a pin number to their value for the SC1A register, for the ADC0 and ADC1
*  numbers with +ADC_SC1A_PIN_MUX (128) means those pins use mux a, the rest use mux b.
*  numbers with +ADC_SC1A_PIN_DIFF (64) means it's also a differential pin (treated also in the channel2sc1a_diff_ADCx)
*  For diff_table_ADCx, +ADC_SC1A_PIN_PGA means the pin can use PGA on that ADC
*/


constexpr const uint8_t ADC::channel2sc1aADC0[];
#if ADC_NUM_ADCS>1
constexpr const uint8_t ADC::channel2sc1aADC1[];
#endif

constexpr const uint8_t ADC::sc1a2channelADC0[];
#if ADC_NUM_ADCS>1
constexpr const uint8_t ADC::sc1a2channelADC1[];
#endif

constexpr const ADC_Module::ADC_NLIST ADC::diff_table_ADC0[];
#if ADC_NUM_ADCS>1
constexpr const ADC_Module::ADC_NLIST ADC::diff_table_ADC1[];
#endif


ADC_Module ADC::adc0_obj = ADC_Module(0, ADC::channel2sc1aADC0, ADC::diff_table_ADC0);
#if ADC_NUM_ADCS>1
ADC_Module ADC::adc1_obj = ADC_Module(1, ADC::channel2sc1aADC1, ADC::diff_table_ADC1);
#endif

//////////////// SYNCHRONIZED BLOCKING METHODS //////////////////
///// ONLY FOR BOARDS WITH MORE THAN ONE ADC /////
/////////////////////////////////////////////////////////////////

#if ADC_NUM_ADCS>1

/*Returns the analog values of both pins, measured at the same time by the two ADC modules.
* It waits until the value is read and then returns the result as a struct Sync_result,
* use Sync_result.result_adc0 and Sync_result.result_adc1.
* If a comparison has been set up and fails, it will return ADC_ERROR_VALUE in both fields of the struct.
*/
ADC::Sync_result ADC::analogSynchronizedRead(uint8_t pin0, uint8_t pin1) {

    Sync_result res = {ADC_ERROR_VALUE, ADC_ERROR_VALUE};

    // check pins
    if ( !adc0->checkPin(pin0) ) {
        adc0->fail_flag |= ADC_ERROR::WRONG_PIN;
        return res;
    }
    if ( !adc1->checkPin(pin1) ) {
        adc1->fail_flag |= ADC_ERROR::WRONG_PIN;
        return res;
    }


    // check if we are interrupting a measurement, store setting if so.
    // vars to save the current state of the ADC in case it's in use
    ADC_Module::ADC_Config old_adc0_config = {0};
    uint8_t wasADC0InUse = adc0->isConverting(); // is the ADC running now?
    if(wasADC0InUse) { // this means we're interrupting a conversion
        // save the current conversion config, the adc isr will restore the adc
        __disable_irq();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc0->saveConfig(&old_adc0_config);
        __enable_irq();
    }
    ADC_Module::ADC_Config old_adc1_config = {0};
    uint8_t wasADC1InUse = adc1->isConverting(); // is the ADC running now?
    if(wasADC1InUse) { // this means we're interrupting a conversion
        // save the current conversion config, the adc isr will restore the adc
        __disable_irq();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc1->saveConfig(&old_adc1_config);
        __enable_irq();
    }

    // no continuous mode
    adc0->singleMode();
    adc1->singleMode();

    // start both measurements
    adc0->startReadFast(pin0);
    adc1->startReadFast(pin1);

    // wait for both ADCs to finish
    while( (adc0->isConverting()) || (adc1->isConverting()) ) { // wait for both to finish
        yield();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
    }


    __disable_irq(); // make sure nothing interrupts this part
    if ( adc0->isComplete() ) { // conversion succeded
        res.result_adc0 = adc0->readSingle();
    } else { // comparison was false
        adc0->fail_flag |= ADC_ERROR::COMPARISON;
    }
    if ( adc1->isComplete() ) { // conversion succeded
        res.result_adc1 = adc1->readSingle();
    } else { // comparison was false
        adc1->fail_flag |= ADC_ERROR::COMPARISON;
    }
    __enable_irq();


    // if we interrupted a conversion, set it again
    if (wasADC0InUse) {
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc0->loadConfig(&old_adc0_config);
    }
    if (wasADC1InUse) {
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc1->loadConfig(&old_adc1_config);
    }

    //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );

    return res;
}

/*Returns the diff analog values of both sets of pins, measured at the same time by the two ADC modules.
* It waits until the value is read and then returns the result as a struct Sync_result,
* use Sync_result.result_adc0 and Sync_result.result_adc1.
* If a comparison has been set up and fails, it will return ADC_ERROR_VALUE in both fields of the struct.
*/
ADC::Sync_result ADC::analogSynchronizedReadDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N) {

    Sync_result res = {ADC_ERROR_VALUE, ADC_ERROR_VALUE};;

    // check pins
    if(!adc0->checkDifferentialPins(pin0P, pin0N)) {
        adc0->fail_flag |= ADC_ERROR::WRONG_PIN;
        return res;   // all others are invalid
    }
    if(!adc1->checkDifferentialPins(pin1P, pin1N)) {
        adc1->fail_flag |= ADC_ERROR::WRONG_PIN;
        return res;   // all others are invalid
    }

    uint8_t resolution0 = adc0->getResolution();
    uint8_t resolution1 = adc1->getResolution();

    // check if we are interrupting a measurement, store setting if so.
    // vars to save the current state of the ADC in case it's in use
    ADC_Module::ADC_Config old_adc0_config = {0};
    uint8_t wasADC0InUse = adc0->isConverting(); // is the ADC running now?
    if(wasADC0InUse) { // this means we're interrupting a conversion
        // save the current conversion config, the adc isr will restore the adc
        __disable_irq();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc0->saveConfig(&old_adc0_config);
        __enable_irq();
    }
    ADC_Module::ADC_Config old_adc1_config = {0};
    uint8_t wasADC1InUse = adc1->isConverting(); // is the ADC running now?
    if(wasADC1InUse) { // this means we're interrupting a conversion
        // save the current conversion config, the adc isr will restore the adc
        __disable_irq();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc1->saveConfig(&old_adc1_config);
        __enable_irq();
    }

    // no continuous mode
    adc0->singleMode();
    adc1->singleMode();

    // start both measurements
    adc0->startDifferentialFast(pin0P, pin0N);
    adc1->startDifferentialFast(pin1P, pin1N);

    // wait for both ADCs to finish
    while( (adc0->isConverting()) || (adc1->isConverting()) ) {
        yield();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
    }
    __disable_irq(); // make sure nothing interrupts this part
    if (adc0->isComplete()) { // conversion succeded
        res.result_adc0 = adc0->readSingle();
        if(resolution0==16) { // 16 bit differential is actually 15 bit + 1 bit sign
            res.result_adc0 *= 2; // multiply by 2 as if it were really 16 bits, so that getMaxValue gives a correct value.
        }
    } else { // comparison was false
        adc0->fail_flag |= ADC_ERROR::COMPARISON;
    }
    if (adc1->isComplete()) { // conversion succeded
        res.result_adc1 = adc1->readSingle();
        if(resolution1==16) { // 16 bit differential is actually 15 bit + 1 bit sign
            res.result_adc1 *= 2; // multiply by 2 as if it were really 16 bits, so that getMaxValue gives a correct value.
        }
    } else { // comparison was false
        adc1->fail_flag |= ADC_ERROR::COMPARISON;
    }
    __enable_irq();


    // if we interrupted a conversion, set it again
    if (wasADC0InUse) {
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc0->loadConfig(&old_adc0_config);
    }
    if (wasADC1InUse) {
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc1->loadConfig(&old_adc1_config);
    }

    //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );

    return res;
}

/////////////// SYNCHRONIZED NON-BLOCKING METHODS //////////////

// Starts an analog measurement at the same time on the two ADC modules
/* It returns inmediately, get value with readSynchronizedSingle().
*   If the pin is incorrect it returns false
*   If this function interrupts a measurement, it stores the settings in adc_config
*/
bool ADC::startSynchronizedSingleRead(uint8_t pin0, uint8_t pin1) {

    // check pins
    if ( !adc0->checkPin(pin0) ) {
        adc0->fail_flag |= ADC_ERROR::WRONG_PIN;
        return false;
    }
    if ( !adc1->checkPin(pin1) ) {
        adc1->fail_flag |= ADC_ERROR::WRONG_PIN;
        return false;
    }

    // check if we are interrupting a measurement, store setting if so.
    adc0->adcWasInUse = adc0->isConverting(); // is the ADC running now?
    if(adc0->adcWasInUse) { // this means we're interrupting a conversion
        // save the current conversion config, the adc isr will restore the adc
        __disable_irq();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc0->saveConfig(&adc0->adc_config);
        __enable_irq();
    }
    adc1->adcWasInUse = adc1->isConverting(); // is the ADC running now?
    if(adc1->adcWasInUse) { // this means we're interrupting a conversion
        // save the current conversion config, the adc isr will restore the adc
        __disable_irq();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc1->saveConfig(&adc1->adc_config);
        __enable_irq();
    }

    // no continuous mode
    adc0->singleMode();
    adc1->singleMode();

    // start both measurements
    adc0->startReadFast(pin0);
    adc1->startReadFast(pin1);


    //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
    return true;

}

// Start a differential conversion between two pins (pin0P - pin0N) and (pin1P - pin1N)
/* It returns inmediately, get value with readSynchronizedSingle().
*   \param pinP must be A10 or A12.
*   \param pinN must be A11 (if pinP=A10) or A13 (if pinP=A12).
*   Other pins will return false.
*   If this function interrupts a measurement, it stores the settings in adc_config
*/
bool ADC::startSynchronizedSingleDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N) {

    // check pins
    if(!adc0->checkDifferentialPins(pin0P, pin0N)) {
        adc0->fail_flag |= ADC_ERROR::WRONG_PIN;
        return false;   // all others are invalid
    }
    if(!adc1->checkDifferentialPins(pin1P, pin1N)) {
        adc1->fail_flag |= ADC_ERROR::WRONG_PIN;
        return false;   // all others are invalid
    }

    // check if we are interrupting a measurement, store setting if so.
    adc0->adcWasInUse = adc0->isConverting(); // is the ADC running now?
    if(adc0->adcWasInUse) { // this means we're interrupting a conversion
        // save the current conversion config, the adc isr will restore the adc
        __disable_irq();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc0->saveConfig(&adc0->adc_config);
        __enable_irq();
    }
    adc1->adcWasInUse = adc1->isConverting(); // is the ADC running now?
    if(adc1->adcWasInUse) { // this means we're interrupting a conversion
        // save the current conversion config, the adc isr will restore the adc
        __disable_irq();
        //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );
        adc1->saveConfig(&adc1->adc_config);
        __enable_irq();
    }

    // no continuous mode
    adc0->singleMode();
    adc1->singleMode();

    // start both measurements
    adc0->startDifferentialFast(pin0P, pin0N);
    adc1->startDifferentialFast(pin1P, pin1N);

    //digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN) );

    return true;
}

// Reads the analog value of a single conversion.
/*
*   \return the converted value.
*/
ADC::Sync_result ADC::readSynchronizedSingle() {
    ADC::Sync_result res;

    res.result_adc0 = adc0->readSingle();
    res.result_adc1 = adc1->readSingle();

    return res;
}


///////////// SYNCHRONIZED CONTINUOUS CONVERSION METHODS ////////////

//! Starts a continuous conversion in both ADCs simultaneously
/** Use readSynchronizedContinuous to get the values
*
*/
bool ADC::startSynchronizedContinuous(uint8_t pin0, uint8_t pin1) {

    // check pins
    if ( !adc0->checkPin(pin0) ) {
        adc0->fail_flag |= ADC_ERROR::WRONG_PIN;
        return false;
    }
    if ( !adc1->checkPin(pin1) ) {
        adc1->fail_flag |= ADC_ERROR::WRONG_PIN;
        return false;
    }

    adc0->startContinuous(pin0);
    adc1->startContinuous(pin1);

    // setup the conversions the usual way, but to make sure that they are
    // as synchronized as possible we stop and restart them one after the other.
    const uint32_t temp_ADC0_SC1A = ADC0_SC1A; ADC0_SC1A = 0x1F;
    const uint32_t temp_ADC1_SC1A = ADC1_SC1A; ADC1_SC1A = 0x1F;

    __disable_irq(); // both measurements should have a maximum delay of an instruction time
    ADC0_SC1A = temp_ADC0_SC1A;
    ADC1_SC1A = temp_ADC1_SC1A;
    __enable_irq();

    return true;
}

//! Starts a continuous differential conversion in both ADCs simultaneously
/** Use readSynchronizedContinuous to get the values
*
*/
bool ADC::startSynchronizedContinuousDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N) {

    // check pins
    if(!adc0->checkDifferentialPins(pin0P, pin0N)) {
        adc0->fail_flag |= ADC_ERROR::WRONG_PIN;
        return false;   // all others are invalid
    }
    if(!adc1->checkDifferentialPins(pin1P, pin1N)) {
        adc1->fail_flag |= ADC_ERROR::WRONG_PIN;
        return false;   // all others are invalid
    }

    adc0->startContinuousDifferential(pin0P, pin0N);
    adc1->startContinuousDifferential(pin1P, pin1N);

    // setup the conversions the usual way, but to make sure that they are
    // as synchronized as possible we stop and restart them one after the other.
    const uint32_t temp_ADC0_SC1A = ADC0_SC1A; ADC0_SC1A = 0x1F;
    const uint32_t temp_ADC1_SC1A = ADC1_SC1A; ADC1_SC1A = 0x1F;

    __disable_irq();
    ADC0_SC1A = temp_ADC0_SC1A;
    ADC1_SC1A = temp_ADC1_SC1A;
    __enable_irq();


    return true;
}

//! Returns the values of both ADCs.
ADC::Sync_result ADC::readSynchronizedContinuous() {
    ADC::Sync_result res;

    res.result_adc0 = adc0->analogReadContinuous();
    res.result_adc1 = adc1->analogReadContinuous();

    return res;
}

//! Stops synchronous continuous conversion
void ADC::stopSynchronizedContinuous() {

    adc0->stopContinuous();
    adc1->stopContinuous();
}

#endif
