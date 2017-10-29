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

/* ADC.h: Control for one (Teensy 3.0, LC) or two ADC modules (Teensy 3.1).
 *
 */

/* TODO
* - Function to measure more that 1 pin consecutively (stream?)
*
* bugs:
* - comparison values in 16 bit differential mode are twice what they should be
*/

#ifndef ADC_H
#define ADC_H

// include ADC module class
#include "ADC_Module.h"


/** Class ADC: Controls the Teensy 3.x ADC
*
*/
class ADC
{
    public:
        #if defined(ADC_TEENSY_3_0)
        //! Translate pin number to SC1A nomenclature
        static constexpr const uint8_t channel2sc1aADC0[ADC_MAX_PIN+1]= { // new version, gives directly the sc1a number. 0x1F=31 deactivates the ADC.
            5, 14, 8, 9, 13, 12, 6, 7, 15, 4, 0, 19, 3, 21, // 0-13, we treat them as A0-A13
            5, 14, 8, 9, 13, 12, 6, 7, 15, 4, // 14-23 (A0-A9)
            31, 31, 31, 31, 31, 31, 31, 31, 31, 31, // 24-33
            0+ADC_SC1A_PIN_DIFF, 19+ADC_SC1A_PIN_DIFF, 3+ADC_SC1A_PIN_DIFF, 21+ADC_SC1A_PIN_DIFF, // 34-37 (A10-A13)
            26, 22, 23, 27, 29, 30 // 38-43: temp. sensor, VREF_OUT, A14, bandgap, VREFH, VREFL. A14 isn't connected to anything in Teensy 3.0.
        };
        #elif defined(ADC_TEENSY_3_1) // the only difference with 3.0 is that A13 is not connected to ADC0 and that T3.1 has PGA.
        //! Translate pin number to SC1A nomenclature
        static constexpr const uint8_t channel2sc1aADC0[ADC_MAX_PIN+1]= { // new version, gives directly the sc1a number. 0x1F=31 deactivates the ADC.
            5, 14, 8, 9, 13, 12, 6, 7, 15, 4, 0, 19, 3, 31, // 0-13, we treat them as A0-A13
            5, 14, 8, 9, 13, 12, 6, 7, 15, 4, // 14-23 (A0-A9)
            31, 31, 31, 31, 31, 31, 31, 31, 31, 31, // 24-33
            0+ADC_SC1A_PIN_DIFF, 19+ADC_SC1A_PIN_DIFF, 3+ADC_SC1A_PIN_DIFF, 31+ADC_SC1A_PIN_DIFF, // 34-37 (A10-A13)
            26, 22, 23, 27, 29, 30 // 38-43: temp. sensor, VREF_OUT, A14, bandgap, VREFH, VREFL. A14 isn't connected to anything in Teensy 3.0.
        };
        #elif defined(ADC_TEENSY_LC)
        //! Translate pin number to SC1A nomenclature
        static constexpr const uint8_t channel2sc1aADC0[ADC_MAX_PIN+1]= { // new version, gives directly the sc1a number. 0x1F=31 deactivates the ADC.
            5, 14, 8, 9, 13, 12, 6, 7, 15, 11, 0, 4+ADC_SC1A_PIN_MUX, 23, 31, // 0-13, we treat them as A0-A12 + A13= doesn't exist
            5, 14, 8, 9, 13, 12, 6, 7, 15, 11, // 14-23 (A0-A9)
            0+ADC_SC1A_PIN_DIFF, 4+ADC_SC1A_PIN_MUX+ADC_SC1A_PIN_DIFF, 23, 31, 31, 31, 31, 31, 31, 31, // 24-33 ((A10-A12) + nothing), A11 uses mux a
            31, 31, 31, 31, // 34-37 nothing
            26, 27, 31, 27, 29, 30 // 38-43: temp. sensor, , , bandgap, VREFH, VREFL.
        };
        #elif defined(ADC_TEENSY_3_5)
        //! Translate pin number to SC1A nomenclature
        static constexpr const uint8_t channel2sc1aADC0[ADC_MAX_PIN+1]= { // new version, gives directly the sc1a number. 0x1F=31 deactivates the ADC.
            5, 14, 8, 9, 13, 12, 6, 7, 15, 4, 3, 31, 31, 31, // 0-13, we treat them as A0-A13
            5, 14, 8, 9, 13, 12, 6, 7, 15, 4, // 14-23 (A0-A9)
            26, 27, 29, 30, 31, 31, 31, // 24-30: Temp_Sensor, bandgap, VREFH, VREFL.
            31, 31, 17, 18,// 31-34 A12(ADC1), A13(ADC1), A14, A15
            31, 31, 31, 31, 31, 31, 31, 31, 31, // 35-43
            31, 31, 31, 31, 31, 31, 31, 31, 31, // 44-52
            31, 31, 31, 31, 31, 31, 31, 31, 31, // 53-61
            31, 31, 3+ADC_SC1A_PIN_DIFF, 31+ADC_SC1A_PIN_DIFF, 23, 31, 1, 31 // 62-69 64: A10, 65: A11 (NOT CONNECTED), 66: A21, 68: A25 (no diff)
        };
        #elif defined(ADC_TEENSY_3_6)
        //! Translate pin number to SC1A nomenclature
        static constexpr const uint8_t channel2sc1aADC0[ADC_MAX_PIN+1]= { // new version, gives directly the sc1a number. 0x1F=31 deactivates the ADC.
            5, 14, 8, 9, 13, 12, 6, 7, 15, 4, 3, 31, 31, 31, // 0-13, we treat them as A0-A13
            5, 14, 8, 9, 13, 12, 6, 7, 15, 4, // 14-23 (A0-A9)
            26, 27, 29, 30, 31, 31, 31, // 24-30: Temp_Sensor, bandgap, VREFH, VREFL.
            31, 31, 17, 18,// 31-34 A12(ADC1), A13(ADC1), A14, A15
            31, 31, 31, 31, 31, 31, 31, 31, 31, // 35-43
            31, 31, 31, 31, 31, 31, 31, 31, 31, // 44-52
            31, 31, 31, 31, 31, 31, 31, 31, 31, // 53-61
            31, 31, 3+ADC_SC1A_PIN_DIFF, 31+ADC_SC1A_PIN_DIFF, 23, 31 // 62-67 64: A10, 65: A11 (NOT CONNECTED), 66: A21, 67: A22(ADC1)
        };
        #endif // defined


        #if ADC_NUM_ADCS>1
        #if defined(ADC_TEENSY_3_1)
        //! Translate pin number to SC1A nomenclature
        static constexpr const uint8_t channel2sc1aADC1[ADC_MAX_PIN+1]= { // new version, gives directly the sc1a number. 0x1F=31 deactivates the ADC.
            31, 31, 8, 9, 31, 31, 31, 31, 31, 31, 3, 31, 0, 19, // 0-13, we treat them as A0-A13
            31, 31, 8, 9, 31, 31, 31, 31, 31, 31, // 14-23 (A0-A9)
            31, 31,  // 24,25 are digital only pins
            5+ADC_SC1A_PIN_MUX, 5, 4, 6, 7, 4+ADC_SC1A_PIN_MUX, 31, 31, // 26-33 26=5a, 27=5b, 28=4b, 29=6b, 30=7b, 31=4a, 32,33 are digital only
            3+ADC_SC1A_PIN_DIFF, 31+ADC_SC1A_PIN_DIFF, 0+ADC_SC1A_PIN_DIFF, 19+ADC_SC1A_PIN_DIFF, // 34-37 (A10-A13) A11 isn't connected.
            26, 18, 31, 27, 29, 30 // 38-43: temp. sensor, VREF_OUT, A14 (not connected), bandgap, VREFH, VREFL.
        };
        #elif defined(ADC_TEENSY_3_5)
        //! Translate pin number to SC1A nomenclature
        static constexpr const uint8_t channel2sc1aADC1[ADC_MAX_PIN+1]= { // new version, gives directly the sc1a number. 0x1F=31 deactivates the ADC.
            31, 31, 8, 9, 31, 31, 31, 31, 31, 31, 31, 19, 14, 15, // 0-13, we treat them as A0-A13
            31, 31, 8, 9, 31, 31, 31, 31, 31, 31, // 14-23 (A0-A9)
            26, 27, 29, 30, 18, 31, 31,  // 24-30: Temp_Sensor, bandgap, VREFH, VREFL, VREF_OUT
            14, 15, 31, 31, 4, 5, 6, 7, 17, // 31-39 A12-A20
            31, 31, 31, 31, // 40-43
            31, 31, 31, 31, 31, 10, 11, 31, 31, // 44-52, 49: A23, 50: A24
            31, 31, 31, 31, 31, 31, 31, 31, 31, // 53-61
            31, 31, 0+ADC_SC1A_PIN_DIFF, 19+ADC_SC1A_PIN_DIFF, 31, 23, 31, 1 // 62-69 64: A10, 65: A11, 67: A22, 69: A26 (not diff)
        };
        #elif defined(ADC_TEENSY_3_6)
        //! Translate pin number to SC1A nomenclature
        static constexpr const uint8_t channel2sc1aADC1[ADC_MAX_PIN+1]= { // new version, gives directly the sc1a number. 0x1F=31 deactivates the ADC.
            31, 31, 8, 9, 31, 31, 31, 31, 31, 31, 31, 19, 14, 15, // 0-13, we treat them as A0-A13
            31, 31, 8, 9, 31, 31, 31, 31, 31, 31, // 14-23 (A0-A9)
            26, 27, 29, 30, 18, 31, 31,  // 24-30: Temp_Sensor, bandgap, VREFH, VREFL, VREF_OUT
            14, 15, 31, 31, 4, 5, 6, 7, 17, // 31-39 A12-A20
            31, 31, 31, 23, // 40-43: A10(ADC0), A11(ADC0), A21, A22
            31, 31, 31, 31, 31, 10, 11, 31, 31, // 44-52, 49: A23, 50: A24
            31, 31, 31, 31, 31, 31, 31, 31, 31, // 53-61
            31, 31, 0+ADC_SC1A_PIN_DIFF, 19+ADC_SC1A_PIN_DIFF, 31, 23 // 61-67 64: A10, 65: A11, 66: A21(ADC0), 67: A22
        };
        #endif
        #endif // ADC_NUM_ADCS > 1

        #if defined(ADC_TEENSY_3_0) || defined(ADC_TEENSY_3_1)
        //! Translate pin number to SC1A nomenclature for differential pins
        static constexpr const uint8_t sc1a2channelADC0[ADC_MAX_PIN+1]= { // new version, gives directly the pin number
            34, 0, 0, 36, 23, 14, 20, 21, 16, 17, 0, 0, 19, 18, // 0-13
            15, 22, 23, 0, 0, 35, 0, 37, // 14-21
            39, 40, 0, 0, 38, 41, 42, 43, // VREF_OUT, A14, temp. sensor, bandgap, VREFH, VREFL.
            0 // 31 means disabled, but just in case
        };
        #elif defined(ADC_TEENSY_LC)
        //! Translate pin number to SC1A nomenclature for differential pins
        static constexpr const uint8_t sc1a2channelADC0[ADC_MAX_PIN+1]= { // new version, gives directly the pin number
            24, 0, 0, 0, 25, 14, 20, 21, 16, 17, 0, 23, 19, 18, // 0-13
            15, 22, 23, 0, 0, 0, 0, 0, // 14-21
            26, 0, 0, 0, 38, 41, 0, 42, 43, // A12, temp. sensor, bandgap, VREFH, VREFL.
            0 // 31 means disabled, but just in case
        };
        #elif defined(ADC_TEENSY_3_5) || defined(ADC_TEENSY_3_6)
        //! Translate pin number to SC1A nomenclature for differential pins
        static constexpr const uint8_t sc1a2channelADC0[ADC_MAX_PIN+1]= { // new version, gives directly the pin number
            0, 68, 0, 64, 23, 14, 20, 21, 16, 17, 0, 0, 19, 18, // 0-13
            15, 22, 0, 33, 34, 0, 0, 0, // 14-21
            0, 66, 0, 0, 70, 0, 0, 0, // 22-29
            0 // 31 means disabled, but just in case
        };
        #endif // defined

        #if ADC_NUM_ADCS>1
        #if defined(ADC_TEENSY_3_1)
        //! Translate pin number to SC1A nomenclature for differential pins
        static constexpr const uint8_t sc1a2channelADC1[ADC_MAX_PIN+1]= { // new version, gives directly the pin number
            36, 0, 0, 34, 28, 26, 29, 30, 16, 17, 0, 0, 0, 0, // 0-13. 5a=26, 5b=27, 4b=28, 4a=31
            0, 0, 0, 0, 39, 37, 0, 0, // 14-21
            0, 0, 0, 0, 38, 41, 0, 42, // 22-29. VREF_OUT, A14, temp. sensor, bandgap, VREFH, VREFL.
            43
        };
        #elif defined(ADC_TEENSY_3_5) || defined(ADC_TEENSY_3_6)
        //! Translate pin number to SC1A nomenclature for differential pins
        static constexpr const uint8_t sc1a2channelADC1[ADC_MAX_PIN+1]= { // new version, gives directly the pin number
            0, 69, 0, 0, 35, 36, 37, 38, 0, 0, 49, 50, 0, 0, // 0-13.
            31, 32, 0, 39, 71, 65, 0, 0, // 14-21
            0, 67, 0, 0, 0, 0, 0, 0, // 22-29.
            0
        };
        #endif
        #endif // ADC_NUM_ADCS>1


        #if defined(ADC_TEENSY_3_1)
        //! Translate differential pin number to SC1A nomenclature
        static constexpr const ADC_Module::ADC_NLIST diff_table_ADC0[ADC_DIFF_PAIRS]= {
            {A10, 0+ADC_SC1A_PIN_PGA}, {A12, 3}
        };
        //! Translate differential pin number to SC1A nomenclature
        static constexpr const ADC_Module::ADC_NLIST diff_table_ADC1[ADC_DIFF_PAIRS]= {
            {A10, 3}, {A12, 0+ADC_SC1A_PIN_PGA}
        };
        #elif defined(ADC_TEENSY_3_0)
        //! Translate differential pin number to SC1A nomenclature
        static constexpr const ADC_Module::ADC_NLIST diff_table_ADC0[ADC_DIFF_PAIRS]= {
            {A10, 0}, {A12, 3}
        };
        #elif defined(ADC_TEENSY_LC)
        //! Translate differential pin number to SC1A nomenclature
        static constexpr const ADC_Module::ADC_NLIST diff_table_ADC0[ADC_DIFF_PAIRS]= {
            {A10, 0}
        };
        #elif defined(ADC_TEENSY_3_5) || defined(ADC_TEENSY_3_6)
        //! Translate differential pin number to SC1A nomenclature
        static constexpr const ADC_Module::ADC_NLIST diff_table_ADC0[ADC_DIFF_PAIRS]= {
            {A10, 3}
        };
        //! Translate differential pin number to SC1A nomenclature
        static constexpr const ADC_Module::ADC_NLIST diff_table_ADC1[ADC_DIFF_PAIRS]= {
            {A10, 0}
        };
        #endif


        // ADCs objects
        static ADC_Module adc0_obj;
        #if ADC_NUM_ADCS>1
        static ADC_Module adc1_obj;
        #endif

    private:

        // Number of ADC objects
        const uint8_t num_ADCs = ADC_NUM_ADCS;

        // Workload-based dispatch policy:
        // Dispatch conversion to the selected ADC. If no specific ADC is selected (ADC_NUM::ANY):
        // Check which ADC can handle the pin, if both:
        // Use the ADC with lesser workload
        // If only one ADC can measure pin, use it.
        template<typename ret_type, typename... Args>
        ret_type workload_dispatch_policy(bool (ADC_Module::*check_fun)(Args... args),
                                          ret_type (ADC_Module::*conversion_fun)(Args... args), ADC_NUM adc_num, Args... args) {
            #if ADC_NUM_ADCS==1
            return (adc0->*conversion_fun)(args...); // use ADC0
            #else
            if(adc_num==ADC_NUM::ANY) { // use no ADC in particular
                // check which ADC can read the pin
                bool adc0Pin = (adc0->*check_fun)(args...);
                bool adc1Pin = (adc1->*check_fun)(args...);

                if(adc0Pin && adc1Pin)  { // Both ADCs
                    if( (adc0->num_measurements) > (adc1->num_measurements)) { // use the ADC with less workload
                        return (adc1->*conversion_fun)(args...);
                    } else {
                        return (adc0->*conversion_fun)(args...);
                    }
                } else if(adc0Pin) { // ADC0
                    return (adc0->*conversion_fun)(args...);
                } else if(adc1Pin) { // ADC1
                    return (adc1->*conversion_fun)(args...);
                } else { // pin not valid in any ADC
                    adc0->fail_flag |= ADC_ERROR::WRONG_PIN;
                    adc1->fail_flag |= ADC_ERROR::WRONG_PIN;
                    return ADC_ERROR_VALUE;   // all others are invalid
                }
            } else { // Use a specific ADC
                return (adc[static_cast<uint8_t>(adc_num)]->*conversion_fun)(args...);
            }
            #endif // ADC_NUM_ADCS
        }

        // Simple dispatch policy:
        // Dispatch conversion to the selected ADC. If no specific ADC is selected (ADC_NUM::ANY):
        // Check if ADC0 can handle it, if so, use it.
        // If not, check ADC1, if so, use it.
        template<typename ret_type, typename... Args>
        ret_type simple_dispatch_policy(bool (ADC_Module::*check_fun)(Args... args),
                                        ret_type (ADC_Module::*conversion_fun)(Args... args), ADC_NUM adc_num, Args... args) {
            #if ADC_NUM_ADCS==1
            return (adc0->*conversion_fun)(args...); // use ADC0
            #else
            if(adc_num==ADC_NUM::ANY) { // use no ADC in particular
                // check which ADC can read the pin
                bool adc0Pin = (adc0->*check_fun)(args...);
                if(adc0Pin) { // ADC0
                    return (adc0->*conversion_fun)(args...);
                }

                bool adc1Pin = (adc1->*check_fun)(args...);
                if(adc1Pin) { // ADC1
                    return (adc1->*conversion_fun)(args...);
                }

                // Not valid for any ADC
                adc0->fail_flag |= ADC_ERROR::WRONG_PIN;
                adc1->fail_flag |= ADC_ERROR::WRONG_PIN;
                return ADC_ERROR_VALUE;   // all others are invalid
            } else { // Use a specific ADC
                return (adc[static_cast<uint8_t>(adc_num)]->*conversion_fun)(args...);
            }
            #endif // ADC_NUM_ADCS
        }

        // Change the return function to the policy that you want: workload_dispatch_policy or simple_dispatch_policy.
        template<typename ret_type, typename... Args>
        __attribute__((always_inline)) ret_type dispatch_policy(bool (ADC_Module::*check_fun)(Args... args),
                                 ret_type (ADC_Module::*conversion_fun)(Args... args),
                                 ADC_NUM adc_num, Args... args) {
            return workload_dispatch_policy(check_fun, conversion_fun, adc_num, args...);
            //return simple_dispatch_policy(check_fun, conversion_fun, adc_num, args...);
        }


    public:

        /** Default constructor */
        constexpr ADC() {
            //ctor

            //digitalWriteFast(LED_BUILTIN, HIGH);

            // make sure the clocks to the ADC are on
            atomic::setBitFlag(SIM_SCGC6, SIM_SCGC6_ADC0);
            #if ADC_NUM_ADCS>1
            atomic::setBitFlag(SIM_SCGC3, SIM_SCGC3_ADC1);
            #endif

        }

        // create both adc objects

        //! Object to control the ADC0
        ADC_Module *const adc0 = &adc0_obj; // adc object pointer
        #if ADC_NUM_ADCS>1
        //! Object to control the ADC1
        ADC_Module *const adc1 = &adc1_obj; // adc object pointer
        #endif

        #if ADC_NUM_ADCS==1
        //! Array of all ADC modules
        ADC_Module *const adc[ADC_NUM_ADCS] = {adc0};
        #else
        //! Array of all ADC modules
        ADC_Module *const adc[ADC_NUM_ADCS] = {adc0, adc1};
        #endif


        /////////////// METHODS TO SET/GET SETTINGS OF THE ADC ////////////////////

        //! Set the voltage reference you prefer, default is vcc
        /*! It recalibrates at the end.
        *   \param type can be ADC_REFERENCE::REF_3V3, ADC_REFERENCE::REF_1V2 (not for Teensy LC) or ADC_REFERENCE::REF_EXT
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void setReference(ADC_REFERENCE type, ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->setReference(type);
            return;
        }


        //! Change the resolution of the measurement.
        /*!
        *  \param bits is the number of bits of resolution.
        *  For single-ended measurements: 8, 10, 12 or 16 bits.
        *  For differential measurements: 9, 11, 13 or 16 bits.
        *  If you want something in between (11 bits single-ended for example) select the immediate higher
        *  and shift the result one to the right.
        *  Whenever you change the resolution, change also the comparison values (if you use them).
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void setResolution(uint8_t bits, ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->setResolution(bits);
            return;
        }

        //! Returns the resolution of the ADC_Module.
        /**
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use..
        *   \return the resolution of adc_num ADC.
        */
        uint8_t getResolution(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            return adc[static_cast<uint8_t>(adc_num)]->getResolution();
        }

        //! Returns the maximum value for a measurement: 2^res-1.
        /**
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use..
        *   \return the maximum value of adc_num ADC.
        */
        uint32_t getMaxValue(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            return adc[static_cast<uint8_t>(adc_num)]->getMaxValue();
        }


        //! Sets the conversion speed (changes the ADC clock, ADCK)
        /**
        * \param speed can be any from the ADC_CONVERSION_SPEED enum: VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED_16BITS, HIGH_SPEED, VERY_HIGH_SPEED,
        *       ADACK_2_4, ADACK_4_0, ADACK_5_2 or ADACK_6_2.
        *
        * VERY_LOW_SPEED is guaranteed to be the lowest possible speed within specs for resolutions less than 16 bits (higher than 1 MHz),
        * it's different from LOW_SPEED only for 24, 4 or 2 MHz bus frequency.
        * LOW_SPEED is guaranteed to be the lowest possible speed within specs for all resolutions (higher than 2 MHz).
        * MED_SPEED is always >= LOW_SPEED and <= HIGH_SPEED.
        * HIGH_SPEED_16BITS is guaranteed to be the highest possible speed within specs for all resolutions (lower or eq than 12 MHz).
        * HIGH_SPEED is guaranteed to be the highest possible speed within specs for resolutions less than 16 bits (lower or eq than 18 MHz).
        * VERY_HIGH_SPEED may be out of specs, it's different from HIGH_SPEED only for 48, 40 or 24 MHz bus frequency.
        *
        * Additionally the conversion speed can also be ADACK_2_4, ADACK_4_0, ADACK_5_2 and ADACK_6_2,
        * where the numbers are the frequency of the ADC clock (ADCK) in MHz and are independent on the bus speed.
        * This is useful if you are using the Teensy at a very low clock frequency but want faster conversions,
        * but if F_BUS<F_ADCK, you can't use VERY_HIGH_SPEED for sampling speed.
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void setConversionSpeed(ADC_CONVERSION_SPEED speed, ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->setConversionSpeed(speed);}


        //! Sets the sampling speed
        /** Increase the sampling speed for low impedance sources, decrease it for higher impedance ones.
        * \param speed can be any of the ADC_SAMPLING_SPEED enum: VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED or VERY_HIGH_SPEED.
        *
        * VERY_LOW_SPEED is the lowest possible sampling speed (+24 ADCK).
        * LOW_SPEED adds +16 ADCK.
        * MED_SPEED adds +10 ADCK.
        * HIGH_SPEED adds +6 ADCK.
        * VERY_HIGH_SPEED is the highest possible sampling speed (0 ADCK added).
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void setSamplingSpeed(ADC_SAMPLING_SPEED speed, ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->setSamplingSpeed(speed);
        }


        //! Set the number of averages
        /*!
        * \param num can be 0, 4, 8, 16 or 32.
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void setAveraging(uint8_t num, ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->setAveraging(num);
        }


        //! Enable interrupts
        /** An IRQ_ADCx Interrupt will be raised when the conversion is completed
        *  (including hardware averages and if the comparison (if any) is true).
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void enableInterrupts(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->enableInterrupts();
        }

        //! Disable interrupts
        /**
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void disableInterrupts(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->disableInterrupts();
        }


        //! Enable DMA request
        /** An ADC DMA request will be raised when the conversion is completed
        *  (including hardware averages and if the comparison (if any) is true).
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void enableDMA(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->enableDMA();
        }

        //! Disable ADC DMA request
        /**
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void disableDMA(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->disableDMA();
        }


        //! Enable the compare function to a single value
        /** A conversion will be completed only when the ADC value
        *  is >= compValue (greaterThan=1) or < compValue (greaterThan=0)
        *  Call it after changing the resolution
        *  Use with interrupts or poll conversion completion with isComplete()
        *   \param compValue value to compare
        *   \param greaterThan true or false
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void enableCompare(int16_t compValue, bool greaterThan, ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->enableCompare(compValue, greaterThan);
        }

        //! Enable the compare function to a range
        /** A conversion will be completed only when the ADC value is inside (insideRange=1) or outside (=0)
        *  the range given by (lowerLimit, upperLimit),including (inclusive=1) the limits or not (inclusive=0).
        *  See Table 31-78, p. 617 of the freescale manual.
        *  Call it after changing the resolution
        *  Use with interrupts or poll conversion completion with isComplete()
        *   \param lowerLimit lower value to compare
        *   \param upperLimit upper value to compare
        *   \param insideRange true or false
        *   \param inclusive true or false
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void enableCompareRange(int16_t lowerLimit, int16_t upperLimit, bool insideRange, bool inclusive, ADC_NUM adc_num = ADC_NUM::ADC_0)  {
            adc[static_cast<uint8_t>(adc_num)]->enableCompareRange(lowerLimit, upperLimit, insideRange, inclusive);
        }

        //! Disable the compare function
        /**
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void disableCompare(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->disableCompare();
        }


        //! Enable and set PGA
        /** Enables the PGA and sets the gain
        *   Use only for signals lower than 1.2 V and only in differential mode
        *   \param gain can be 1, 2, 4, 8, 16, 32 or 64
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void enablePGA(uint8_t gain, ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->enablePGA(gain);
        }

        //! Returns the PGA level
        /** PGA level = from 1 to 64
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return PGA level = from 1 to 64
        */
        uint8_t getPGA(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            return adc[static_cast<uint8_t>(adc_num)]->getPGA();
        }

        //! Disable PGA
        /**
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void disablePGA(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->disablePGA();
        }


        ////////////// INFORMATION ABOUT THE STATE OF THE ADC /////////////////

        //! Is the ADC converting at the moment?
        /**
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return true if yes, false if not.
        */
        bool isConverting(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            return adc[static_cast<uint8_t>(adc_num)]->isConverting();
        }

        //! Is an ADC conversion ready?
        /** When a value is read this function returns 0 until a new value exists
        *   So it only makes sense to call it with continuous or non-blocking methods
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return true if yes, false if not.
        */
        bool isComplete(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            return adc[static_cast<uint8_t>(adc_num)]->isComplete();
        }

        //! Is the ADC in differential mode?
        /**
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return true or false
        */
        bool isDifferential(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            return adc[static_cast<uint8_t>(adc_num)]->isDifferential();
        }

        //! Is the ADC in continuous mode?
        /**
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return true or false
        */
        bool isContinuous(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            return adc[static_cast<uint8_t>(adc_num)]->isContinuous();
        }



        //////////////// BLOCKING CONVERSION METHODS //////////////////

        //! Returns the analog value of the pin.
        /** It waits until the value is read and then returns the result.
        * If a comparison has been set up and fails, it will return ADC_ERROR_VALUE.
        * This function is interrupt safe, so it will restore the adc to the state it was before being called
        * If more than one ADC exists, it will select the module with less workload, you can force a selection using
        * adc_num.
        *   \param pin can be any of the analog pins
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return the value of the pin.
        */
        int analogRead(uint8_t pin, ADC_NUM adc_num = ADC_NUM::ANY) __attribute__((always_inline)) {
            // dispatch to the right ADC module depending on the policy that we want.
            return dispatch_policy(&ADC_Module::checkPin, &ADC_Module::analogRead, adc_num, pin);
        }


        //! Returns the analog value of the special internal source, such as the temperature sensor.
        /** It calls analogRead(uint8_t pin) internally, with the correct value for the pin for all boards.
        *   Possible values:
        *   TEMP_SENSOR,  Temperature sensor.
        *   VREF_OUT,  1.2 V reference (switch on first using VREF.h).
        *   BANDGAP, BANDGAP (switch on first using VREF.h).
        *   VREFH, High VREF.
        *   VREFL, Low VREF.
        *   \param pin ADC_INTERNAL_SOURCE to read.
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return the value of the pin.
        */
        int analogRead(ADC_INTERNAL_SOURCE pin, ADC_NUM adc_num = ADC_NUM::ANY) __attribute__((always_inline)) {
            return analogRead(static_cast<uint8_t>(pin), adc_num);
        }

        //! Reads the differential analog value of two pins (pinP - pinN).
        /** It waits until the value is read and then returns the result.
        * This function is interrupt safe, so it will restore the adc to the state it was before being called
        * If more than one ADC exists, it will select the module with less workload, you can force a selection using
        * adc_num
        *   \param pinP must be A10 or A12.
        *   \param pinN must be A11 (if pinP=A10) or A13 (if pinP=A12).
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return the differential value of the pins, invalid pins return ADC_ERROR_VALUE.
        *   If a comparison has been set up and fails, it will return ADC_ERROR_VALUE.
        */
        int analogReadDifferential(uint8_t pinP, uint8_t pinN, ADC_NUM adc_num = ADC_NUM::ANY) __attribute__((always_inline)) {
            return dispatch_policy(&ADC_Module::checkDifferentialPins, &ADC_Module::analogReadDifferential, adc_num, pinP, pinN);
        }


        /////////////// NON-BLOCKING CONVERSION METHODS //////////////

        //! Starts an analog measurement on the pin and enables interrupts.
        /** It returns immediately, get value with readSingle().
        *   If this function interrupts a measurement, it stores the settings in adc_config
        *   \param pin can be any of the analog pins
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return true if the pin is valid, false otherwise.
        */
        bool startSingleRead(uint8_t pin, ADC_NUM adc_num = ADC_NUM::ANY) __attribute__((always_inline)) {
            return dispatch_policy(&ADC_Module::checkPin, &ADC_Module::startSingleRead, adc_num, pin);
        }

        //! Start a differential conversion between two pins (pinP - pinN) and enables interrupts.
        /** It returns immediately, get value with readSingle().
        *   If this function interrupts a measurement, it stores the settings in adc_config
        *   \param pinP must be A10 or A12.
        *   \param pinN must be A11 (if pinP=A10) or A13 (if pinP=A12).
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return true if the pins are valid, false otherwise.
        */
        bool startSingleDifferential(uint8_t pinP, uint8_t pinN, ADC_NUM adc_num = ADC_NUM::ANY) __attribute__((always_inline)) {
            return dispatch_policy(&ADC_Module::checkDifferentialPins, &ADC_Module::startSingleDifferential, adc_num, pinP, pinN);
        }

        //! Reads the analog value of a single conversion.
        /** Set the conversion with with startSingleRead(pin) or startSingleDifferential(pinP, pinN).
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return the converted value.
        */
        int readSingle(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            return adc[static_cast<uint8_t>(adc_num)]->readSingle();
        }



        ///////////// CONTINUOUS CONVERSION METHODS ////////////

        //! Starts continuous conversion on the pin.
        /** It returns as soon as the ADC is set, use analogReadContinuous() to read the value.
        *   \param pin can be any of the analog pins
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return true if the pin is valid, false otherwise.
        */
        bool startContinuous(uint8_t pin, ADC_NUM adc_num = ADC_NUM::ANY) __attribute__((always_inline)) {
            return dispatch_policy(&ADC_Module::checkPin, &ADC_Module::startContinuous, adc_num, pin);
        }

        //! Starts continuous conversion between the pins (pinP-pinN).
        /** It returns as soon as the ADC is set, use analogReadContinuous() to read the value.
        *   \param pinP must be A10 or A12.
        *   \param pinN must be A11 (if pinP=A10) or A13 (if pinP=A12).
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return true if the pins are valid, false otherwise.
        */
        bool startContinuousDifferential(uint8_t pinP, uint8_t pinN, ADC_NUM adc_num = ADC_NUM::ANY) __attribute__((always_inline)) {
            return dispatch_policy(&ADC_Module::checkDifferentialPins, &ADC_Module::startContinuousDifferential, adc_num, pinP, pinN);
        }

        //! Reads the analog value of a continuous conversion.
        /** Set the continuous conversion with with analogStartContinuous(pin) or startContinuousDifferential(pinP, pinN).
        *   If single-ended and 16 bits it's necessary to typecast it to an unsigned type (like uint16_t),
        *   otherwise values larger than 3.3/2 V are interpreted as negative!
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        *   \return the last converted value.
        */
        int analogReadContinuous(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            return adc[static_cast<uint8_t>(adc_num)]->analogReadContinuous();
        }

        //! Stops continuous conversion
        /**
        *   \param adc_num ADC_NUM enum member. Selects the ADC module to use.
        */
        void stopContinuous(ADC_NUM adc_num = ADC_NUM::ADC_0) __attribute__((always_inline)) {
            adc[static_cast<uint8_t>(adc_num)]->stopContinuous();
        }



        /////////// SYNCHRONIZED METHODS ///////////////
        ///// ONLY FOR BOARDS WITH MORE THAN ONE ADC /////
        #if ADC_NUM_ADCS>1

        //! Struct for synchronous measurements
        /** result_adc0 has the result from ADC0 and result_adc1 from ADC1.
        */
        struct Sync_result{
            int32_t result_adc0; /*!< Result of ADC0 */
            int32_t result_adc1; /*!< Result of ADC1 */
        };

        //////////////// SYNCHRONIZED BLOCKING METHODS //////////////////

        //! Returns the analog values of both pins, measured at the same time by the two ADC modules.
        /** It waits until the values are read and then returns the result as a struct Sync_result,
        *   use Sync_result.result_adc0 and Sync_result.result_adc1.
        *   If a comparison has been set up and fails, it will return ADC_ERROR_VALUE in both fields of the struct.
        *   This function is interrupt safe, so it will restore the adc to the state it was before being called
        *   \param pin0 pin in ADC0
        *   \param pin1 pin in ADC1
        *   \return a Sync_result struct with the result of each ADC value.
        */
        Sync_result analogSynchronizedRead(uint8_t pin0, uint8_t pin1);

        //! Same as analogSynchronizedRead
        /**
        *   \param pin0 pin in ADC0
        *   \param pin1 pin in ADC1
        *   \return a Sync_result struct with the result of each ADC value.
        */
        Sync_result analogSyncRead(uint8_t pin0, uint8_t pin1) __attribute__((always_inline)) {
            return analogSynchronizedRead(pin0, pin1);
        }

        //! Returns the differential analog values of both sets of pins, measured at the same time by the two ADC modules.
        /** It waits until the values are read and then returns the result as a struct Sync_result,
        *   use Sync_result.result_adc0 and Sync_result.result_adc1.
        *   If a comparison has been set up and fails, it will return ADC_ERROR_VALUE in both fields of the struct.
        *   This function is interrupt safe, so it will restore the adc to the state it was before being called
        *   \param pin0P positive pin in ADC0
        *   \param pin0N negative pin in ADC0
        *   \param pin1P positive pin in ADC1
        *   \param pin1N negative pin in ADC1
        *   \return a Sync_result struct with the result of each differential ADC value.
        */
        Sync_result analogSynchronizedReadDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N);

        //! Same as analogSynchronizedReadDifferential
        /**
        *   \param pin0P positive pin in ADC0
        *   \param pin0N negative pin in ADC0
        *   \param pin1P positive pin in ADC1
        *   \param pin1N negative pin in ADC1
        *   \return a Sync_result struct with the result of each differential ADC value.
        */
        Sync_result analogSyncReadDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N) __attribute__((always_inline)) {
            return analogSynchronizedReadDifferential(pin0P, pin0N, pin1P, pin1N);
        }

        /////////////// SYNCHRONIZED NON-BLOCKING METHODS //////////////

        //! Starts an analog measurement at the same time on the two ADC modules
        /** It returns immediately, get value with readSynchronizedSingle().
        *   If this function interrupts a measurement, it stores the settings in adc_config
        *   \param pin0 pin in ADC0
        *   \param pin1 pin in ADC1
        *   \return true if the pins are valid, false otherwise.
        */
        bool startSynchronizedSingleRead(uint8_t pin0, uint8_t pin1);

        //! Start a differential conversion between two pins (pin0P - pin0N) and (pin1P - pin1N)
        /** It returns immediately, get value with readSynchronizedSingle().
        *   If this function interrupts a measurement, it stores the settings in adc_config
        *   \param pin0P positive pin in ADC0
        *   \param pin0N negative pin in ADC0
        *   \param pin1P positive pin in ADC1
        *   \param pin1N negative pin in ADC1
        *   \return true if the pins are valid, false otherwise.
        */
        bool startSynchronizedSingleDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N);

        //! Reads the analog value of a single conversion.
        /**
        *   \return the converted value.
        */
        Sync_result readSynchronizedSingle();


        ///////////// SYNCHRONIZED CONTINUOUS CONVERSION METHODS ////////////

        //! Starts a continuous conversion in both ADCs simultaneously
        /** Use readSynchronizedContinuous to get the values
        *   \param pin0 pin in ADC0
        *   \param pin1 pin in ADC1
        *   \return true if the pins are valid, false otherwise.
        */
        bool startSynchronizedContinuous(uint8_t pin0, uint8_t pin1);

        //! Starts a continuous differential conversion in both ADCs simultaneously
        /** Use readSynchronizedContinuous to get the values
        *   \param pin0P positive pin in ADC0
        *   \param pin0N negative pin in ADC0
        *   \param pin1P positive pin in ADC1
        *   \param pin1N negative pin in ADC1
        *   \return true if the pins are valid, false otherwise.
        */
        bool startSynchronizedContinuousDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N);

        //! Returns the values of both ADCs.
        /**
        *   \return the converted value.
        */
        Sync_result readSynchronizedContinuous();

        //! Stops synchronous continuous conversion
        void stopSynchronizedContinuous();

        #endif


        //////////// ERROR PRINTING /////
        //! Prints the human-readable error from all ADC, if any.
        void printError() {
            for(int i=0; i< ADC_NUM_ADCS; i++) {
                adc[i]->printError();
            }
        }

        //! Resets all errors from all ADCs, if any.
        void resetError() {
            for(int i=0; i< ADC_NUM_ADCS; i++) {
                adc[i]->resetError();
            }
        }

};


#endif // ADC_H
