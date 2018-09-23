// treemap.cpp - Implementation of CColorSpace, CTreemap and CTreemapPreview
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2005 Bernhard Seifert
// Copyright (C) 2004-2017 WinDirStat Team (windirstat.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <assert.h>
#include "CColorSpace.h"
#include "CTreeMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BGR(b,g,r)          ((uint32_t)(((uint8_t)(b)|((uint32_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(r))<<16)))
#define RGB(r,g,b)          ((uint32_t)(((uint8_t)(r)|((uint32_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16)))
#define RGB_GET_BVALUE(c)   (((uint32_t)c & 0xff0000) >> 16)
#define RGB_GET_GVALUE(c)   (((uint32_t)c & 0x00ff00) >> 8)
#define RGB_GET_RVALUE(c)   (((uint32_t)c & 0x0000ff) >> 0)
#define ASSERT assert

// I define the "brightness" of an rgb value as (r+b+g)/3/255.
// The EqualizeColors() method creates a palette with colors
// all having the same brightness of 0.6
// Later in RenderCushion() this number is used again to
// scale the colors.

static const double PALETTE_BRIGHTNESS = 0.6;

/////////////////////////////////////////////////////////////////////////////

double CColorSpace::GetColorBrightness(uint32_t color)
{
	uint32_t const crIndividualIntensitySum = RGB_GET_RVALUE(color) + RGB_GET_GVALUE(color) + RGB_GET_BVALUE(color);
	return crIndividualIntensitySum / 255.0 / 3.0;
}

uint32_t CColorSpace::MakeBrightColor(uint32_t color, double brightness)
{
	ASSERT(brightness >= 0.0);
	ASSERT(brightness <= 1.0);

	double dred = (RGB_GET_RVALUE(color) & 0xFF) / 255.0;
	double dgreen = (RGB_GET_GVALUE(color) & 0xFF) / 255.0;
	double dblue = (RGB_GET_BVALUE(color) & 0xFF) / 255.0;

	double f = 3.0 * brightness / (dred + dgreen + dblue);
	dred *= f;
	dgreen *= f;
	dblue *= f;

	int red = (int) (dred * 255);
	int green = (int) (dgreen * 255);
	int blue = (int) (dblue * 255);

	NormalizeColor(red, green, blue);

	return RGB(red, green, blue);
}

// Returns true, if the System has 256 Colors or less.
// In this case options.brightness is ignored (and the
// slider should be disabled).
//
bool CColorSpace::Is256Colors()
{
	return false;
}

void CColorSpace::NormalizeColor(int& red, int& green, int& blue)
{
	ASSERT(red + green + blue <= 3 * 255);

	if (red > 255)
	{
		DistributeFirst(red, green, blue);
	}
	else if (green > 255)
	{
		DistributeFirst(green, red, blue);
	}
	else if (blue > 255)
	{
		DistributeFirst(blue, red, green);
	}
}

void CColorSpace::DistributeFirst(int& first, int& second, int& third)
{
	const int h = (first - 255) / 2;
	first = 255;
	second += h;
	third += h;

	if (second > 255)
	{
		const int j = second - 255;
		second = 255;
		third += j;
		ASSERT(third <= 255);
	}
	else if (third > 255)
	{
		const int j = third - 255;
		third = 255;
		second += j;
		ASSERT(second <= 255);
	}
}

/////////////////////////////////////////////////////////////////////////////
