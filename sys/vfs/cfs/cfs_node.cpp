//
//  cfs_node.cpp
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

#include <machine/memory/memory.h>
#include <kern/kprintf.h>
#include <libcpp/algorithm.h>
#include <vfs/context.h>
#include "cfs_node.h"

namespace CFS
{
	IODefineMeta(Node, VFS::Node)

	Node *Node::Init(const char *name, VFS::Instance *instance, uint64_t id, void *memo)
	{
		if(!VFS::Node::Init(name, instance, VFS::Node::Type::File, id))
			return nullptr;

		_openProc = nullptr;
		_closeProc = nullptr;
		_readProc = nullptr;
		_writeProc = nullptr;
		_sizeProc = nullptr;
		_mmapProc = nullptr;
		_msyncProc = nullptr;
		_supportsSeek = false;
		_memo = memo;

		return this;
	}


	void Node::SetSizeProc(SizeProc proc)
	{
		_sizeProc = proc;
	}
	void Node::SetIoctlProc(IoctlProc proc)
	{
		_ioctlProc = proc;
	}
	void Node::SetOpenProc(OpenCloseProc proc)
	{
		_openProc = proc;
	}
	void Node::SetCloseProc(OpenCloseProc proc)
	{
		_closeProc = proc;
	}
	void Node::SetMmapProc(MmapProc proc)
	{
		_mmapProc = proc;
	}
	void Node::SetMsyncProc(MsyncProc proc)
	{
		_msyncProc = proc;
	}


	void Node::SetSupportsSeek(bool seek)
	{
		_supportsSeek = seek;
	}

	uint64_t Node::GetSize() const
	{
		if(!_sizeProc)
			return 0;

		return _sizeProc(_memo);
	}

	void Node::Open()
	{
		if(_openProc)
			_openProc(_memo);
	}
	void Node::Close()
	{
		if(_closeProc)
			_closeProc(_memo);
	}

	KernReturn<size_t> Node::WriteData(VFS::Context *context, off_t offset, const void *data, size_t size)
	{
		if(_writeProc)
		{
			size_t result = _writeProc(_memo, context, offset, data, size);
			if(result == (size_t)-1)
				return Error(KERN_FAILURE);

			return result;
		}

		return Error(KERN_UNSUPPORTED);
	}
	KernReturn<size_t> Node::ReadData(VFS::Context *context, off_t offset, void *data, size_t size)
	{
		if(_readProc)
		{
			size_t result = _readProc(_memo, context, offset, data, size);
			if(result == (size_t)-1)
				return Error(KERN_FAILURE);

			return result;
		}

		return Error(KERN_UNSUPPORTED);
	}
	KernReturn<void> Node::Ioctl(VFS::Context *context, uint32_t request, void *data)
	{
		if(!_ioctlProc)
			return Error(KERN_UNSUPPORTED);

		return _ioctlProc(_memo, context, request, data);
	}

	KernReturn<OS::MmapTaskEntry *> Node::Mmap(VFS::Context *context, OS::MmapArgs *args)
	{
		if(!_mmapProc)
			return Error(KERN_UNSUPPORTED);

		return _mmapProc(_memo, context, this, args);
	}
	KernReturn<size_t> Node::Msync(VFS::Context *context, OS::MmapTaskEntry *entry, OS::MsyncArgs *args)
	{
		if(!_msyncProc)
			return Error(KERN_UNSUPPORTED);

		return _msyncProc(_memo, context, entry, args);
	}
}
