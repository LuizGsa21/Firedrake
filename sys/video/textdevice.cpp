//
//  textdevice.cpp
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

#include <machine/port.h>
#include <libc/string.h>
#include <libcpp/algorithm.h>
#include "textdevice.h"

TextVideoDevice::TextVideoDevice()
{
	_base = (uint8_t *)0xb8000;
	_width  = 80;
	_height = 25;

	Clear();
}

void TextVideoDevice::SetColors(Color foreground, Color background)
{
	foregroundColor = foreground;
	backgroundColor = background;
}

void TextVideoDevice::SetCursor(size_t x, size_t y)
{
	uint16_t pos = y * _width + x;

	cursorX = x;
	cursorY = y;

	outb(0x3d4, 0x0f);
	outb(0x3d5, (uint8_t)(pos & 0xff));

	outb(0x3d4, 0x0e);
	outb(0x3d5, (uint8_t)((pos >> 8) & 0xff));

	ColorPoint(x, y, Color::LightGray, backgroundColor);
}

void TextVideoDevice::ColorPoint(size_t x, size_t y, Color foreground, Color background)
{
	uint32_t index = (y * _width + x) * 2;

	uint8_t tForeground = (uint8_t)foreground;
	uint8_t tBackground = (uint8_t)background;

	uint8_t color = (tBackground << 4) | (tForeground & 0x0f);
	_base[index + 1] = color;
}

void TextVideoDevice::Putc(char character)
{
	uint32_t index = (cursorY * _width + cursorX) * 2;
	_base[index] = character;
}


void TextVideoDevice::WriteString(const char *string)
{
	while(*string)
	{
		char character = *(string ++);

		if(character == '\n')
		{
			cursorX = 0;
			cursorY ++;

			if(cursorY >= _height)
				ScrollLines(1);

			continue;
		}

		Putc(character);
		ColorPoint(cursorX, cursorY, foregroundColor, backgroundColor);

		cursorX ++;

		if(cursorX >= _width)
		{
			cursorX = 0;
			cursorY ++;

			if(cursorY >= _height)
				ScrollLines(1);
		}
	}

	SetCursor(cursorX, cursorY);
}

void TextVideoDevice::ScrollLines(size_t lines)
{
	for(size_t i = 0; i < lines; i ++)
	{
		for(size_t y = 0; y < _height; y ++)
		{
			memcpy(&_base[(y * _width) * 2], &_base[((y + 1) * _width) * 2], _width * 2);
		}

		uint8_t color  = ((uint8_t)backgroundColor << 4) | ((uint8_t)foregroundColor & 0x0f);
		uint32_t index = ((_height - 1) * _width) * 2;

		for(size_t x = 0; x < _width; x ++)
		{
			_base[index ++] = '\0';
			_base[index ++] = color;
		}

		if(cursorY > 0)
			cursorY --;
	}

	SetCursor(cursorX, cursorY);
}

void TextVideoDevice::Clear()
{
	uint8_t color = ((uint8_t)backgroundColor << 4) | ((uint8_t)foregroundColor & 0x0f);
	size_t length = _width * _height * 2;

	for(size_t i = 0; i < length;)
	{
		_base[i ++] = '\0';
		_base[i ++] = color;
	}

	SetCursor(0, 0);
}