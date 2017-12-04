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


/** Class RingBufferDMA implements a circular DMA buffer of fixed size
*/
template<uint32_t max_capacity>
class RingBufferDMA
{
    public:
        //! Constructor, stores the conversions of ADC number ADC_num
        RingBufferDMA(ADC_NUM ADC_num) : ADC_RA(ADC_num==ADC_NUM::ADC_0 ? ADC0_RA : ADC1_RA) {
            static_assert(max_capacity%2==0, "RingBufferDMA's max_capacity must be a power of two.");

            uint8_t DMAMUX_SOURCE_ADC = DMAMUX_SOURCE_ADC0;
            #if ADC_NUM_ADCS>1
            if(ADC_num==ADC_NUM::ADC_1){
                DMAMUX_SOURCE_ADC = DMAMUX_SOURCE_ADC1;
            }
            #endif // ADC_NUM_ADCS
            dmaChannel.triggerAtHardwareEvent(DMAMUX_SOURCE_ADC);

            dmaChannel.source((volatile uint16_t &)ADC_RA);
            dmaChannel.destinationCircular(elems, max_capacity*sizeof(elems[0])); // 2 bytes per element

            #if defined(KINETISL)
            // fix bug in DMAChannel.h that doesn't set the right destination size. https://github.com/PaulStoffregen/cores/pull/77
            dmaChannel.CFG->DCR &= ~DMA_DCR_DSIZE(3);
            dmaChannel.CFG->DCR |= DMA_DCR_DSIZE(2);
            // b/c of that bug, the transfer count is also incorrect.
            dmaChannel.transferCount(max_capacity);

            // Interrupt resets the number of bytes to copy to continue
            if(dmaChannel.channel == 0) {
                dmaChannel.attachInterrupt(dma_isr_0);
            } else if(dmaChannel.channel == 1) {
                dmaChannel.attachInterrupt(dma_isr_1);
            } else if(dmaChannel.channel == 2) {
                dmaChannel.attachInterrupt(dma_isr_2);
            } else if(dmaChannel.channel == 3) {
                dmaChannel.attachInterrupt(dma_isr_3);
            }
            dmaChannel.interruptAtCompletion();
            #endif
        }

        //! Destructor
        ~RingBufferDMA() {
            dmaChannel.detachInterrupt();
            dmaChannel.disable();
        }

        //! Clears the dmaChannel interrupt.
        /** In addition, for Teensy LC it also resets the buffer so the transfers continue.
        */
        void clearInterrupt() {
            dmaChannel.clearInterrupt();
            #if defined(KINETISL)
            dmaChannel.CFG->DSR_BCR = max_capacity*sizeof(uint16_t);
            #endif
        }

        //! Add an interrupt at completion of halfway (not for Teensy LC).
        /** For Teensy LC you must call clearInterrupt() at the beginning of your custom interrupt function
        */
        void attachInterrupt(void (*dma_isr)(void), bool at_completion = true) {
            dmaChannel.attachInterrupt(dma_isr);
            #if defined(KINETISK)
            if(at_completion) {
                dmaChannel.interruptAtCompletion();
            } else {
                dmaChannel.interruptAtHalf();
            }
            #elif defined(KINETISL)
            dmaChannel.interruptAtCompletion();
            #endif
        }

        //! Start DMA operation
        void start() {
            // read ADC_RA to clear any DMA trigger
            volatile uint16_t val __attribute__((unused)) = ADC_RA;
            dmaChannel.enable();

        }

        //! Returns true if the buffer is full
        /** This is only true when last element is filled, but not when the buffer starts to be overwritten.
        *   i.e., when numElements == max_capacity
        */
        volatile bool isFull() {
            return dmaChannel.complete();
        }

        //! Number of elements written so far
        volatile uint32_t numElements() {
            return (uint32_t*)dmaChannel.destinationAddress() - (uint32_t*)&elems[0];
        }

        //! Pointer to the data
        volatile uint16_t* const buffer() {return elems;}

        //! DMAChannel to handle all low level DMA code.
        DMAChannel dmaChannel{};


    protected:
    private:

        //! ADC module of the instance
        uint8_t ADC_number;

        volatile uint32_t& ADC_RA;

        //! Buffer, aligned correctly to the number of bytes.
        volatile uint16_t elems[max_capacity] alignas(max_capacity*sizeof(uint16_t));

        // static isr's to reset the right DMA channel for Teensy LC
        static void dma_isr_0() {
            DMA_DSR_BCR0 = DMA_DSR_BCR_DONE;
            DMA_DSR_BCR0 = (max_capacity*sizeof(uint16_t) & 0x00FFFFFF);
        }
        static void dma_isr_1() {
            DMA_DSR_BCR1 = DMA_DSR_BCR_DONE;
            DMA_DSR_BCR1 = (max_capacity*sizeof(uint16_t) & 0x00FFFFFF);
        }
        static void dma_isr_2() {
            DMA_DSR_BCR2 = DMA_DSR_BCR_DONE;
            DMA_DSR_BCR2 = (max_capacity*sizeof(uint16_t) & 0x00FFFFFF);
        }
        static void dma_isr_3() {
            DMA_DSR_BCR3 = DMA_DSR_BCR_DONE;
            DMA_DSR_BCR3 = (max_capacity*sizeof(uint16_t) & 0x00FFFFFF);
        }

};

#endif // RINGBUFFERDMA_H
