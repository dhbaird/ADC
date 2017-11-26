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

#ifndef RINGBUFFERDMA_H
#define RINGBUFFERDMA_H

#include "DMAChannel.h"
#include "ADC_Module.h"


/** Class RingBufferDMA implements a DMA ping-pong buffer of fixed size
*/
template<uint32_t max_capacity>
class RingBufferDMA
{
    public:
        //! Constructor, buffer stores the conversions of ADC number ADC_num
        /** There are no interrupts when the buffer is full.
        */
        RingBufferDMA(volatile int16_t* const elems, ADC_NUM ADC_num) : RingBufferDMA(elems, ADC_num, &do_nothing_isr) {
            dmaChannel.clearInterrupt();
            dmaChannel.detachInterrupt();
        }
        //! Constructor, buffer stores the conversions of ADC number ADC_num
        /**
        *   \param dma_isr is a function pointer than will be called when the buffer is full.
        */
        RingBufferDMA(volatile int16_t* const elems, ADC_NUM ADC_num, void (*dma_isr)(void)) :
            ADC_RA(ADC_num==ADC_NUM::ADC_0 ? ADC0_RA : ADC1_RA),
            elems(elems)
        {
            static_assert(max_capacity%2==0, "RingBufferDMA's max_capacity must be a power of two.");

            // set up a DMA channel to store the ADC data
            // The idea is to have ADC_RA as a source,
            // each ADC conversion triggers a DMA transfer of size 2 bytes
            // when it's full it will overwrite the old elements.
            dmaChannel.source(ADC_RA);
            dmaChannel.destinationBuffer(elems, 2*max_capacity); // 2 bytes per element
            dmaChannel.transferSize(2); // both SRC and DST size
            dmaChannel.interruptAtCompletion(); //interruptAtHalf or interruptAtCompletion

            uint8_t DMAMUX_SOURCE_ADC = DMAMUX_SOURCE_ADC0;
            #if ADC_NUM_ADCS>1
            if(ADC_num==ADC_NUM::ADC_1){
                DMAMUX_SOURCE_ADC = DMAMUX_SOURCE_ADC1;
            }
            #endif // ADC_NUM_ADCS

            dmaChannel.triggerAtHardwareEvent(DMAMUX_SOURCE_ADC); // start DMA transfer when ADC finishes a conversion
            dmaChannel.attachInterrupt(dma_isr);
        }

        //! Destructor
        ~RingBufferDMA() {
            dmaChannel.detachInterrupt();
            dmaChannel.disable();
        }

        //! Returns true if the buffer is full
        bool isFull() {
            return dmaChannel.complete();
        }

        //! Returns true if the buffer is empty
        bool isEmpty() {
            return false;
        }

        //! Start DMA operation
        void start() {
            // read ADC_RA to clear any DMA trigger
            volatile uint16_t val __attribute__((unused)) = ADC_RA;
            dmaChannel.enable();
        }

        //! Position DMA will write to next
        uint32_t write_pos() {
            //return (reinterpret_cast<uint32_t>(dmaChannel.destinationAddress()) - reinterpret_cast<uint32_t>(elems))/2;
            return (int16_t*)dmaChannel.destinationAddress() - elems;
        }

        //! Pointer to the data
        volatile int16_t* const buffer() {return elems;}

        //! DMAChannel to handle all low level DMA code.
        DMAChannel dmaChannel{};


    protected:
    private:

        //! Size of buffer
        const uint32_t b_size = max_capacity;

        //! ADC module of the instance
        uint8_t ADC_number;

        volatile uint32_t& ADC_RA;

        //! Pointer to the elements of the buffer
        volatile int16_t* const elems;

        static void do_nothing_isr() {}

};


#endif // RINGBUFFERDMA_H
