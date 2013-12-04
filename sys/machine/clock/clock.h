//
//  clock.h
//  Firedrake
//
//  Created by Sidney Just
//  Copyright (c) 2013 by Sidney Just
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
//  documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
//  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
//  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <prefix.h>
#include <libc/stdint.h>
#include <machine/interrupts/interrupts.h>

#ifndef _CLOCK_H_
#define _CLOCK_H_

namespace clock
{
	extern ir::apic_timer_divisor_t default_time_divisor;
	extern uint32_t default_time_counter;
	extern uint32_t default_time_resolution;

	uint32_t calculate_apic_frequency(uint32_t resolution, ir::apic_timer_divisor_t apic_divisor);
	uint32_t calculate_apic_frequency_average(uint32_t resolution, ir::apic_timer_divisor_t apic_divisor);

	kern_return_t init();
}

#endif /* _CLOCK_H_ */