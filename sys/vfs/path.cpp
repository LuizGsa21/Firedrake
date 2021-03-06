//
//  path.cpp
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

#include <kern/kalloc.h>
#include <kern/kprintf.h>
#include "path.h"
#include "instance.h"

namespace VFS
{
	Path::Path(const char *path, Context *context) :
		_context(context),
		_node(nullptr),
		_totalElements(0),
		_elementsLeft(0)
	{
		_path = static_cast<char *>(kalloc(strlen(path + 1)));
		_element = _path;

		if(!_path)
			return;

		strcpy(_path, path);

		// Count the number of elements
		char *temp = _path;
		while(*temp != '\0')
		{
			if(*temp == '/')
			{
				do { temp ++; } while(*temp == '/'); 

				if(*temp != '\0') // Ignore trailing / delimiters
					_totalElements ++;

				continue;
			}

			temp ++;
		}

		_elementsLeft = _totalElements;

		// Determine the starting node
		_node = (*_path == '/') ? _context->GetRootDir() : _context->GetCurrentDir();
	}

	Path::~Path()
	{
		if(_path)
			kfree(_path);
	}


	IO::StrongRef<IO::String> Path::GetNextName()
	{
		while(*_element == '/')
			_element ++;

		// Get the name of the element
		char *element = strpbrk(_element, "/");
		size_t offset = 0;

		if(element)
		{
			offset = element - _element;
			_element[offset] = '\0';
		}

		IO::StrongRef<IO::String> name = IOTransferRef(IO::String::Alloc()->InitWithCString(_element));

		if(element)
			_element[offset] = '/';

		return name;
	}

	KernReturn<Node *> Path::ResolveElement()
	{
		if(!_path)
			return Error(KERN_NO_MEMORY);

		if(_elementsLeft == 0)
			return Error(KERN_RESOURCE_EXHAUSTED, ENOENT);

		// Seek to the next element
		while(*_element == '/')
			_element ++;

		// Get the name of the element
		char *element = strpbrk(_element, "/");
		size_t offset = 0;

		if(element)
		{
			offset = element - _element;
			_element[offset] = '\0';
		}
		
		_name = IOTransferRef(IO::String::Alloc()->InitWithCString(_element));
		
		// Look up the next node
		Instance *instance = _node->GetInstance();

		KernReturn<IO::StrongRef<Node>> result = instance->LookUpNode(_context, _node, _name->GetCString());
		if(!result.IsValid())
		{
			// Revert to the old state and bail
			if(element)
				_element[offset] = '/';

			return result.GetError();
		}

		_node = result;

		if(_node->IsLink())
			_node = _node->Downcast<Link>()->GetTarget();
		if(_node->IsMountpoint())
			_node = _node->Downcast<Mountpoint>()->GetLinkedNode();

		// Advance to the next element
		_element += offset + 1;
		_elementsLeft --;

		return _node.Load();
	}
}
