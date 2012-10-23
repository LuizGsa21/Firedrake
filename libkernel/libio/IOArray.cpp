//
//  IOArray.cpp
//  libio
//
//  Created by Sidney Just
//  Copyright (c) 2012 by Sidney Just
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

#include <libc/string.h>

#include "IOArray.h"
#include "IOMemory.h"

#ifdef super
#undef super
#endif
#define super IOObject

// Constructor

IORegisterClass(IOArray, super)

IOArray *IOArray::withCapacity(uint32_t capacity)
{
	IOArray *array = IOArray::alloc();
	if(!array->initWithCapacity(capacity))
	{
		array->release();
		return 0;
	}

	return array;
}


bool IOArray::init()
{
	if(!super::init())
		return false;

	_capacity = _changeSize = 5;
	_count = 0;

	_objects = (IOObject **)IOMalloc(_capacity * sizeof(IOObject));
	if(!_objects)
		return false;

	return true;
}

bool IOArray::initWithCapacity(uint32_t capacity)
{
	if(!super::init())
		return false;

	_capacity  = capacity;
	_count = 0;
	_changeSize = 5;

	_objects = (IOObject **)IOMalloc(_capacity * sizeof(IOObject));
	if(!_objects)
		return false;

	return true;
}

void IOArray::free()
{
	for(uint32_t i=0; i<_count; i++)
	{
		IOObject *object = _objects[i];
		object->release();
	}

	IOFree(_objects);
	super::free();
}


// Public API

void IOArray::addObject(IOObject *object)
{
	if(_capacity >= _count)
	{
		object->retain();
		_objects[_count] = object;

		_count ++;
		return;
	}


	IOObject **temp = (IOObject **)IOMalloc((_capacity + _changeSize) * sizeof(IOObject));
	if(temp)
	{
		memcpy(temp, _objects, _count * sizeof(IOObject));
		IOFree(_objects);

		_objects  = temp;
		_capacity = _capacity + _changeSize;

		object->retain();
		_objects[_count] = object;

		return;
	}
}

void IOArray::removeObject(IOObject *object)
{
	uint32_t index = indexOfObject(object);
	removeObject(index);
}

void IOArray::removeObject(uint32_t index)
{
	if(index == IONotFound || index >= _count)
		return;

	IOObject *object = _objects[index];
	object->release();

	memcpy(&_objects[index], &_objects[index + 1], _count - index);
	_count --;

	if((_capacity - (_changeSize * 2)) > _count)
	{
		IOObject **temp = (IOObject **)IOMalloc((_count + _changeSize) * sizeof(IOObject));
		memcpy(temp, _objects, _count * sizeof(IOObject));
		IOFree(_objects);

		_objects = temp;
		_capacity = _count + _changeSize;
	}
}



uint32_t IOArray::indexOfObject(IOObject *object)
{
	for(uint32_t i=0; i<_count; i++)
	{
		IOObject *tobject = _objects[i];

		if(tobject->isEqual(object))
			return i;
	}

	return IONotFound;
}

bool IOArray::containsObject(IOObject *object)
{
	for(uint32_t i=0; i<_count; i++)
	{
		IOObject *tobject = _objects[i];
		
		if(tobject->isEqual(object))
			return true;
	}

	return false;
}


IOObject *IOArray::objectAtIndex(uint32_t index)
{
	if(index != IONotFound && index < _count)
	{
		return _objects[index];
	}

	return 0;
}

void IOArray::setChangeSize(uint32_t tchangeSize)
{
	if(tchangeSize != _changeSize && tchangeSize >= 1)
	{
		_changeSize = tchangeSize;
	}
}

uint32_t IOArray::changeSize()
{
	return _changeSize;
}

uint32_t IOArray::count()
{
	return _count;
}

uint32_t IOArray::capacity()
{
	return _capacity;
}
