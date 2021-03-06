//
//  virtual.h
//  Firedrake
//
//  Created by Sidney Just
//  Copyright (c) 2014 by Sidney Just
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

#ifndef _VIRTUAL_H_
#define _VIRTUAL_H_

#include <libc/stdint.h>
#include <libcpp/bitfield.h>
#include <kern/kern_return.h>
#include <libc/sys/spinlock.h>
#include <libc/sys/mman.h>

#define VM_PAGE_SHIFT 12
#define VM_DIRECTORY_SHIFT 22

#define VM_PAGE_SIZE (1 << VM_PAGE_SHIFT)
#define VM_PAGE_MASK (~(VM_PAGE_SIZE - 1))

#define VM_PAGE_COUNT(x)      ((((x) + ~VM_PAGE_MASK) & VM_PAGE_MASK) / VM_PAGE_SIZE)
#define VM_PAGE_ALIGN_DOWN(x) ((x) & VM_PAGE_MASK)
#define VM_PAGE_ALIGN_UP(x)   (VM_PAGE_ALIGN_DOWN((x) + ~VM_PAGE_MASK))

typedef uintptr_t vm_address_t;

#define kVMFlagsKernel Sys::VM::Directory::Flags(Sys::VM::Directory::Flags::Present | Sys::VM::Directory::Flags::Writeable)
#define kVMFlagsKernelNoCache Sys::VM::Directory::Flags(kVMFlagsKernel | Sys::VM::Directory::Flags::NoCache)
#define kVMFlagsUserlandRW Sys::VM::Directory::Flags(Sys::VM::Directory::Flags::Present | Sys::VM::Directory::Flags::Writeable | Sys::VM::Directory::Flags::Userspace)
#define kVMFlagsUserlandR  Sys::VM::Directory::Flags(Sys::VM::Directory::Flags::Present | Sys::VM::Directory::Flags::Userspace)
#define kVMFlagsAll Sys::VM::Directory::Flags(0x7f)

namespace Sys
{
	namespace VM
	{
		constexpr vm_address_t kLowerLimit  = 0x1000;
		constexpr vm_address_t kUpperLimit  = 0xfffff000;
		constexpr vm_address_t kKernelLimit = 0x0ffff000;

		class Directory
		{
		public:
			CPP_BITFIELD(Flags, uint32_t,
				Present      = (1 << 0),
				Writeable    = (1 << 1),
				Userspace    = (1 << 2),
				Writethrough = (1 << 3),
				NoCache      = (1 << 4),
				Accessed     = (1 << 5),
				Dirty        = (1 << 6)
			);

			Directory(uint32_t *directory); // Shouldn't be called directly! Use Create() instead!
			~Directory();

			static KernReturn<Directory *> Create();
			static Directory *GetKernelDirectory();

			KernReturn<void> MapPage(uintptr_t physical, vm_address_t virtAddress, Flags flags);
			KernReturn<void> MapPageRange(uintptr_t physical, vm_address_t virtAddress, size_t pages, Flags flags);

			KernReturn<uintptr_t> ResolveAddress(vm_address_t address);

			KernReturn<vm_address_t> Alloc(uintptr_t physical, size_t pages, Flags flags);
			KernReturn<vm_address_t> AllocLimit(uintptr_t physical, vm_address_t lower, vm_address_t upper, size_t pages, Flags flags);
			KernReturn<vm_address_t> AllocTwoSidedLimit(uintptr_t physical, vm_address_t lower, vm_address_t upper, size_t pages, Flags flags);
			KernReturn<void> Free(vm_address_t address, size_t pages);

			KernReturn<vm_address_t> __Alloc_NoLockPrivate(uintptr_t physical, size_t pages, Flags flags);

			uint32_t *GetPhysicalDirectory() const { return _directory; }

		private:
			KernReturn<uint32_t> GetPageTableEntry(uint32_t *pageDirectory, vm_address_t vaddress);

			uint32_t *_directory;
			spinlock_t _lock;
		};

		static inline Directory::Flags TranslateMmapProtection(int protection)
		{
			Directory::Flags vmflags = Directory::Flags::Present;

			if(!(protection & PROT_NONE))
				vmflags |= Directory::Flags::Userspace;

			if((protection & PROT_WRITE))
				vmflags |= Directory::Flags::Writeable;

			return vmflags;
		}
	}

	KernReturn<void> VMInit();
}

static inline void invlpg(vm_address_t addr)
{
	__asm__ volatile("invlpg (%0)" :: "r" (addr) : "memory");
}

#endif /* _VIRTUAL_H_ */
