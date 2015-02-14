//
//  IPCSyscall.h
//  Firedrake
//
//  Created by Sidney Just
//  Copyright (c) 2015 by Sidney Just
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
#include "IPC.h"

namespace OS
{
	namespace IPC
	{
		struct IPCPortCallArgs
		{
			ipc_port_t *port;
		} __attribute__((packed));

		struct IPCReadWriteArgs
		{
			ipc_header_t *header;
			ipc_size_t size;
			int mode;
		} __attribute__((packed));

		struct IPCAllocatePortArgs
		{
			ipc_port_t *result;
			ipc_name_t name;
			ipc_bits_t options;
		} __attribute__((packed));


		KernReturn<uint32_t> Syscall_IPCTaskPort(uint32_t &esp, IPCPortCallArgs *args);
		KernReturn<uint32_t> Syscall_IPCThreadPort(uint32_t &esp, IPCPortCallArgs *args);
		KernReturn<uint32_t> Syscall_IPCMessage(uint32_t &esp, IPCReadWriteArgs *args);
		KernReturn<uint32_t> Syscall_IPCAllocatePort(uint32_t &esp, IPCAllocatePortArgs *args);
	}
}