//
//  spinlock.h
//  libkernel
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

#ifndef _LIBKERNEL_SPINLOCK_H_
#define _LIBKERNEL_SPINLOCK_H_

#include "base.h"
#include "stdint.h"

typedef uint8_t kern_spinlock_t;

#define KERN_SPINLOCK_INIT 0
#define KERN_SPINLOCK_INIT_LOCKED 1

kern_extern void kern_spinlock_lock(kern_spinlock_t *lock);
kern_extern bool kern_spinlock_tryLock(kern_spinlock_t *lock);
kern_extern void kern_spinlock_unlock(kern_spinlock_t *lock);

#endif /* _LIBKERNEL_SPINLOCK_H_ */
