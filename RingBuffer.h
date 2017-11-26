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

#ifndef RINGBUFFER_H
#define RINGBUFFER_H


/** Class RingBuffer implements a circular buffer of fixed size, must be power of 2.
*   This is a purely software implementation, so it works for all boards.
*/
template<uint32_t max_capacity>
class RingBuffer
{
    public:
        //! Default constructor
        constexpr RingBuffer() {
            static_assert(max_capacity%2==0, "RingBuffer's max_capacity must be a power of two.");
        }

        /** Default destructor */
        virtual ~RingBuffer() {}

        //! Returns true if the buffer is full
        bool isFull() {
            return size() == b_size;
        }

        //! Returns true if the buffer is empty
        bool isEmpty() {
            return b_read == b_write;
        }

        //! Write a value into the buffer
        void write(int16_t value) {
            if(isFull()) {
                b_read++;
            }
            elems[restrict(b_write++)] = value;
        }

        //! Read a value from the buffer. Make sure it's not empty first.
        int16_t read() {
            return elems[restrict(b_read++)];
        }

        uint32_t size() {
            // take care of overflow
            // get the maximum value for b_read (same as for b_write)
            // this is calculated at compile time
            return (b_write - b_read) & (std::numeric_limits<decltype(b_read)>::max());
        }

        int16_t elems[max_capacity];

    public:

        uint32_t restrict(uint32_t p) {
            return p&(b_size-1);
        }

        const uint32_t b_size = max_capacity;
        // we don't restrict them to b_size, but let them increment.
        // when they reach the maximum number they'll overflow to 0 and continue working (we need to be careful in size() though).
        // This is why the max_capacity must be a power of two,
        // otherwise the after the overflow the positions wouldn't be correct relative to each other.
        // (overflow happens at a power of two, therefore a multiple of the max_capacity.)
        uint32_t b_read = 0;
        uint32_t b_write = 0;
};


#endif // RINGBUFFER_H
