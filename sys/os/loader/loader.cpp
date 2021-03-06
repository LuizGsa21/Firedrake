//
//  loader.cpp
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

#include <vfs/vfs.h>
#include <libc/string.h>
#include <kern/kprintf.h>
#include "elf.h"
#include "loader.h"

namespace OS
{
	IODefineMeta(Executable, IO::Object)

	KernReturn<Executable *> Executable::Init(Sys::VM::Directory *directory, const char *path)
	{
		if(!IO::Object::Init())
			return Error(KERN_FAILURE);

		_directory = directory;
		_entry    = 0;
		_physical = 0;
		_virtual  = 0;
		_pages    = 0;

		KernReturn<void> status;
		KernReturn<int> fd;
		VFS::Context *context = VFS::Context::GetKernelContext();

		if((fd = VFS::Open(context, path, O_RDONLY)).IsValid() == false)
			return fd.GetError();

		KernReturn<void> result;
		KernReturn<off_t> size;
		size_t left;

		uint8_t *temp;
		uint8_t *buffer = nullptr;

		// Get the size of the file and allocate enough storage
		if((size = VFS::Seek(context, fd, 0, SEEK_END)).IsValid() == false)
		{
			status = size.GetError();
			goto exit;
		}

		left   = size;
		buffer = new uint8_t[size];

		if(!buffer)
		{
			status = Error(KERN_NO_MEMORY);
			goto exit;
		}

		if(!VFS::Seek(context, fd, 0, SEEK_SET).IsValid())
		{
			status = Error(KERN_FAILURE);
			goto exit;
		}

		temp = buffer;

		// Read the file
		while(left > 0)
		{
			KernReturn<size_t> read = VFS::Read(context, fd, temp, left);
			
			if(!read.IsValid())
			{
				status = read.GetError();
				goto exit;
			}

			temp += read;
			left -= read;
		}

		result = Load(buffer);
		status = result.IsValid() ? ErrorNone : result.GetError();

	exit:
		VFS::Close(context, fd);
		delete[] buffer;

		if(!status.IsValid())
			return status.GetError();

		return this;
	}

	void Executable::Dealloc()
	{
		if(_physical)
			Sys::PM::Free(_physical, _pages);

		IO::Object::Dealloc();
	}



	KernReturn<void> Executable::Load(void *data)
	{
		char *begin = static_cast<char *>(data);
		elf_header_t *header = static_cast<elf_header_t *>(data);

		if(strncmp((const char *)header->e_ident, ELF_MAGIC, strlen(ELF_MAGIC)) != 0)
			return Error(KERN_FAILURE);

		if(header->e_phnum == 0)
			return Error(KERN_INVALID_ARGUMENT);


		_entry = header->e_entry;

		elf_program_header_t *programHeader = reinterpret_cast<elf_program_header_t *>(begin + header->e_phoff);

		vm_address_t minAddress = -1;
		vm_address_t maxAddress = 0;

		for(size_t i = 0; i < header->e_phnum; i ++)
		{
			elf_program_header_t *program = programHeader + i;

			if(program->p_type == PT_LOAD)
			{
				if(program->p_paddr < minAddress)
					minAddress = program->p_paddr;

				if(program->p_paddr + program->p_memsz > maxAddress)
					maxAddress = program->p_paddr + program->p_memsz;
			}
		}

		_virtual = VM_PAGE_ALIGN_DOWN(minAddress);
		_pages   = VM_PAGE_COUNT(maxAddress - minAddress);

		KernReturn<uintptr_t> physical;

		if((physical = Sys::PM::Alloc(_pages)).IsValid() == false)
		{
			_physical = 0x0;
			return physical.GetError();
		}
		
		if((_directory->MapPageRange(physical, _virtual, _pages, kVMFlagsUserlandRW)).IsValid() == false)
			return Error(KERN_NO_MEMORY);
		
		_physical = physical;

		// Copy the date into the physical memory
		KernReturn<vm_address_t> kernel;

		if((kernel = Sys::VM::Directory::GetKernelDirectory()->Alloc(_physical, _pages, kVMFlagsKernel)).IsValid() == false)
			return kernel.GetError();
		
		uint8_t *buffer = reinterpret_cast<uint8_t *>(kernel.Get());

		for(size_t i = 0; i < header->e_phnum; i ++)
		{
			elf_program_header_t *program = programHeader + i;

			if(program->p_type == PT_LOAD)
				memcpy(&buffer[program->p_vaddr - minAddress], &begin[program->p_offset], program->p_filesz);
		}

		Sys::VM::Directory::GetKernelDirectory()->Free(kernel, _pages);
		return ErrorNone;
	}
}
