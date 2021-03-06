//
//  scheduler_syscall.h
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
#include <libc/stdint.h>
#include "scheduler.h"

namespace OS
{
	struct SchedThreadCreateArgs
	{
		uintptr_t entry;
		uintptr_t argument1;
		uintptr_t argument2;
		size_t stack;
	};

	struct SchedThreadExitArgs
	{
		int32_t exitCode;
	};

	struct SchedThreadJoinArgs
	{
		tid_t tid;
	};

	struct SchedExecArgs
	{
		const char *path;
		const char **args;
		const char **envp;
	};

	KernReturn<uint32_t> Syscall_SchedThreadCreate(Thread *thread, SchedThreadCreateArgs *arguments);
	KernReturn<uint32_t> Syscall_SchedThreadExit(Thread *thread, SchedThreadExitArgs *arguments);
	KernReturn<uint32_t> Syscall_SchedThreadJoin(Thread *thread, SchedThreadJoinArgs *arguments);
	KernReturn<uint32_t> Syscall_SchedThreadYield(Thread *thread, void *arguments);

	KernReturn<uint32_t> Syscall_Fork(Thread *thread, void *arguments);
	KernReturn<uint32_t> Syscall_Exec(Thread *thread, SchedExecArgs *arguments);
	KernReturn<uint32_t> Syscall_Spawn(Thread *thread, SchedExecArgs *arguments);
}
