//
//  interrupts.h
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

#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include <prefix.h>
#include <libc/stdint.h>
#include <kern/kern_return.h>
#include <machine/cpu.h>
#include "idt.h"
#include "trampoline.h"
#include "apic.h"

static inline void cli()
{
	__asm__ volatile("cli");
}

static inline void sti()
{
	__asm__ volatile("sti");
}

namespace ir
{
	typedef void (*interrupt_handler_t)(uint8_t, cpu_t *);

	kern_return_t set_interrupt_handler(uint8_t vector, interrupt_handler_t handler);
	kern_return_t init();
}

#endif /* _INTERRUPTS_H_ */
