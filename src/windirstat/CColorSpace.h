// treemap.h - Declaration of CColorSpace, CTreemap and CTreemapPreview
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

#ifndef _CColorSpace_h_
#define _CColorSpace_h_

#include <inttypes.h>
//
// CColorSpace. Helper class for manipulating colors. Static members only.
//
class CColorSpace
{
public:
	// Returns the brightness of color. Brightness is a value between 0 and 1.0.
	static double GetColorBrightness(uint32_t color);

	// Gives a color a defined brightness.
	static uint32_t MakeBrightColor(uint32_t color, double brightness);

	// Returns true, if the system has <= 256 colors
	static bool Is256Colors();

	// Swaps values above 255 to the other two values
	static void NormalizeColor(int& red, int& green, int& blue);

protected:
	// Helper function for NormalizeColor()
	static void DistributeFirst(int& first, int& second, int& third);
};

#endif // _CColorSpace_h_
