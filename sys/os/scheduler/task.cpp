//
//  task.cpp
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

#include <kern/kprintf.h>
#include <kern/panic.h>
#include <libc/string.h>
#include <vfs/file.h>
#include "scheduler.h"
#include "task.h"

namespace OS
{
	static std::atomic<pid_t> _taskPidCounter;
	IODefineMeta(Task, IO::Object)

	namespace Scheduler
	{
		void AddThread(Thread *thread);
	}

	KernReturn<Task *> Task::Init()
	{
		if(!IO::Object::Init())
			return Error(KERN_FAILURE);

		_lock = SPINLOCK_INIT;
		_state = State::Waiting;
		_pid = _taskPidCounter.fetch_add(1);
		_ring3 = false;
		_tidCounter = 1;
		_mainThread = nullptr;
		_directory = Sys::VM::Directory::GetKernelDirectory();
		_context = nullptr;
		_executable = nullptr;
		_threads = IO::Array::Alloc()->Init();

		memset(_files, 0, CONFIG_MAX_FILES * sizeof(VFS::File *));
		return this;
	}

	KernReturn<Task *> Task::InitWithFile(const char *path)
	{
		KernReturn<Task *> result = Init();
		if(!result.IsValid())
			return result.GetError();

		_ring3 = true;
		_directory = nullptr;

		KernReturn<Sys::VM::Directory *> directory;
		if((directory = Sys::VM::Directory::Create()).IsValid() == false)
			return directory.GetError();

		KernReturn<Executable *> executable;
		if((executable = Executable::Alloc()->Init(directory, path)).IsValid() == false)
			return executable.GetError();

		_directory  = directory;
		_executable = executable;

		KernReturn<Thread *> thread = AttachThread(static_cast<Thread::Entry>(_executable->GetEntry()), 0);
		if(!thread.IsValid())
			return thread.GetError();

		return this;
	}


	void Task::Dealloc()
	{
		_executable->Release();

		if(_directory != Sys::VM::Directory::GetKernelDirectory())
			delete _directory;

		IO::Object::Dealloc();
	}



	KernReturn<Thread *> Task::AttachThread(Thread::Entry entry, size_t stack)
	{
		spinlock_lock(&_lock);

		KernReturn<Thread *> thread = Thread::Alloc()->Init(this, entry, stack);

		if(!thread.IsValid())
		{
			spinlock_unlock(&_lock);
			return thread;
		}

		if(!_mainThread)
			_mainThread = thread;

		_threads->AddObject(thread);
		spinlock_unlock(&_lock);
		
		Scheduler::AddThread(thread);
		return thread;
	}

	void Task::RemoveThread(Thread *thread)
	{
		spinlock_lock(&_lock);
		_threads->RemoveObject(thread);
		spinlock_unlock(&_lock);
	}


	void Task::Lock()
	{
		spinlock_lock(&_lock);
	}
	void Task::Unlock()
	{
		spinlock_unlock(&_lock);
	}


	VFS::File *Task::GetFileForDescriptor(int fd)
	{
		if(fd >= CONFIG_MAX_FILES)
			panic("File descriptor >= CONFIG_MAX_FILES!");

		Lock();
		VFS::File *file = _files[fd];
		Unlock();

		return (file != VFS::InvalidFile) ? file : nullptr;
	}
	void Task::SetFileForDescriptor(VFS::File *file, int fd)
	{
		if(fd >= CONFIG_MAX_FILES)
			panic("File descriptor >= CONFIG_MAX_FILES!");

		if(file == nullptr)
			file = VFS::InvalidFile;

		Lock();
		_files[fd] = file;
		Unlock();
	}

	int Task::AllocateFileDescriptor()
	{
		Lock();
		
		int fd = -1;

		for(int i = 0; i < CONFIG_MAX_FILES; i ++)
		{
			if(_files[i] == nullptr)
			{
				_files[i] = VFS::InvalidFile;
				fd = i;

				break;
			}
		}

		Unlock();

		return fd;
	}
	void Task::FreeFileDescriptor(int fd)
	{
		if(fd >= CONFIG_MAX_FILES)
			panic("File descriptor >= CONFIG_MAX_FILES!");

		Lock();
		_files[fd] = nullptr;
		Unlock();
	}
}