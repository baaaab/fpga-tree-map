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

#ifndef __CTREEMAP_H__
#define __CTREEMAP_H__

#include <inttypes.h>
#include <vector>
#include <math.h>

#include "../CSdlDisplay.h"
#include "../windirstat/CPoint.h"
#include "../windirstat/CRect.h"

//
// CTreemap. Can create a treemap. Knows 3 squarification methods:
// KDirStat-like, SequoiaView-like and Simple.
//
// This class is fairly reusable.
//
class CTreeMap
{
public:
	// One of these flags can be added to the uint32_t returned
	// by TmiGetGraphColor(). Used for <Free space> (darker)
	// and <Unknown> (brighter).
	//
	static const uint32_t COLORFLAG_DARKER = 0x01000000;
	static const uint32_t COLORFLAG_LIGHTER = 0x02000000;
	static const uint32_t COLORFLAG_MASK = 0x03000000;

	//
	// Item. Interface which must be supported by the tree items.
	// If you prefer to use the getHead()/getNext() pattern rather
	// than using an array for the children, you will have to
	// rewrite CTreemap.
	//
	class Item
	{
	public:
		virtual bool TmiIsLeaf() const = 0;
		virtual CRect TmiGetRectangle() const = 0;
		virtual void TmiSetRectangle(const CRect& rc) = 0;
		virtual uint32_t TmiGetGraphColor() const = 0;
		virtual int TmiGetChildrenCount() const = 0;
		virtual Item *TmiGetChild(int c) const = 0;
		virtual uint64_t TmiGetSize() const = 0;
	};

	//
	// Callback. Interface with 1 "callback" method. Can be given
	// to the CTreemap-constructor. The CTreemap will call the
	// method very frequently during building the treemap.
	// It's because, if the tree has been paged out by the system,
	// building the treemap can last long (> 30 seconds).
	// TreemapDrawingCallback() gives the chance to provide at
	// least a little visual feedback (Update of RAM usage
	// indicator, for instance).
	//
	class Callback
	{
	public:
		virtual void TreemapDrawingCallback() = 0;
	};

	//
	// Treemap squarification style.
	//
	enum STYLE
	{
		KDirStatStyle,      // Children are layed out in rows. Similar to the style used by KDirStat.
		SequoiaViewStyle    // The 'classical' squarification as described in at http://www.win.tue.nl/~vanwijk/.
	};

	//
	// Collection of all treemap options.
	//
	struct Options
	{
		STYLE style;            // Squarification method
		bool grid;              // Whether or not to draw grid lines
		uint32_t gridColor;     // Color of grid lines
		double brightness;      // 0..1.0   (default = 0.84)
		double height;          // 0..oo    (default = 0.40)    Factor "H"
		double scaleFactor;     // 0..1.0   (default = 0.90)    Factor "F"
		double ambientLight;    // 0..1.0   (default = 0.15)    Factor "Ia"
		double lightSourceX;    // -4.0..+4.0 (default = -1.0), negative = left
		double lightSourceY;    // -4.0..+4.0 (default = -1.0), negative = top

		int GetBrightnessPercent()
		{
			return RoundDouble(brightness * 100);
		}
		int GetHeightPercent()
		{
			return RoundDouble(height * 100);
		}
		int GetScaleFactorPercent()
		{
			return RoundDouble(scaleFactor * 100);
		}
		int GetAmbientLightPercent()
		{
			return RoundDouble(ambientLight * 100);
		}
		int GetLightSourceXPercent()
		{
			return RoundDouble(lightSourceX * 100);
		}
		int GetLightSourceYPercent()
		{
			return RoundDouble(lightSourceY * 100);
		}
		CPoint GetLightSourcePoint()
		{
			return CPoint(GetLightSourceXPercent(), GetLightSourceYPercent());
		}

		void SetBrightnessPercent(int n)
		{
			brightness = n / 100.0;
		}
		void SetHeightPercent(int n)
		{
			height = n / 100.0;
		}
		void SetScaleFactorPercent(int n)
		{
			scaleFactor = n / 100.0;
		}
		void SetAmbientLightPercent(int n)
		{
			ambientLight = n / 100.0;
		}
		void SetLightSourceXPercent(int n)
		{
			lightSourceX = n / 100.0;
		}
		void SetLightSourceYPercent(int n)
		{
			lightSourceY = n / 100.0;
		}
		void SetLightSourcePoint(CPoint pt)
		{
			SetLightSourceXPercent(pt.x);
			SetLightSourceYPercent(pt.y);
		}

		int RoundDouble(double d)
		{
			return round(d);
		}
	};

public:
	// Get a good palette of 13 colors (7 if system has 256 colors)
	static void GetDefaultPalette(std::vector<uint32_t>& palette);

	// Create a equally-bright palette from a set of arbitrary colors
	static void EqualizeColors(const uint32_t *colors, int count, std::vector<uint32_t>& out);

	// Good values
	static Options GetDefaultOptions();

	// WinDirStat <= 1.0.1 default options
	static Options GetOldDefaultOptions();

public:
	// Construct the treemap generator and register the callback interface.
	CTreeMap(Callback *callback = NULL);

	// Alter the options
	void SetOptions(const Options *options);
	Options GetOptions();

#ifdef _DEBUG
	// DEBUG function
	void RecurseCheckTree(Item *item);
#endif // _DEBUG

	// Create and draw a treemap
	void DrawTreemap(CSdlDisplay* display, CRect rc, Item *root, const Options *options = NULL);

	// In the resulting treemap, find the item below a given coordinate.
	// Return value can be NULL, iff point is outside root rect.
	Item *FindItemByPoint(Item *root, CPoint point);

	// Draws a sample rectangle in the given style (for color legend)
	void DrawColorPreview(CSdlDisplay* display, const CRect& rc, uint32_t color, const Options *options = NULL);

protected:
	// The recursive drawing function
	void RecurseDrawGraph(CSdlDisplay* display, Item *item, const CRect& rc, bool asroot, const double *psurface, double h, uint32_t flags);

	// This function switches to KDirStat-, SequoiaView- or Simple_DrawChildren
	void DrawChildren(CSdlDisplay* display, Item *parent, const double *surface, double h, uint32_t flags);

	// KDirStat-like squarification
	void KDirStat_DrawChildren(CSdlDisplay* display, Item *parent, const double *surface, double h, uint32_t flags);
	bool KDirStat_ArrangeChildren(Item *parent, std::vector<double>& childWidth, std::vector<double>& rows, std::vector<int>& childrenPerRow);
	double KDirStat_CalcutateNextRow(Item *parent, const int nextChild, double width, int& childrenUsed, std::vector<double>& childWidth);

	// Classical SequoiaView-like squarification
	void SequoiaView_DrawChildren(CSdlDisplay* display, Item *parent, const double *surface, double h, uint32_t flags);

	// Sets brightness to a good value, if system has only 256 colors
	void SetBrightnessFor256();

	// Returns true, if height and scaleFactor are > 0 and ambientLight is < 1.0
	bool IsCushionShading();

	// Leaves space for grid and then calls RenderRectangle()
	void RenderLeaf(CSdlDisplay* display, Item *item, const double *surface);

	// Either calls DrawCushion() or DrawSolidRect()
	void RenderRectangle(CSdlDisplay* display, const CRect& rc, const double *surface, uint32_t color);
	// void RenderRectangle(CSdlDisplay* display, const CRect& rc, const double *surface, uint32_t color);

	// Draws the surface using SetPixel()
	void DrawCushion(CSdlDisplay* display, const CRect& rc, const double *surface, uint32_t col, double brightness);

	// Draws the surface using FillSolidRect()
	void DrawSolidRect(CSdlDisplay* display, const CRect& rc, uint32_t col, double brightness);

	// Adds a new ridge to surface
	static void AddRidge(const CRect& rc, double *surface, double h);

	static const Options _defaultOptions;              // Good values. Default for WinDirStat 1.0.2
	static const Options _defaultOptionsOld;           // WinDirStat 1.0.1 default options
	static const uint32_t _defaultCushionColors[];      // Standard palette for WinDirStat
	static const uint32_t _defaultCushionColors256[];   // Palette for 256-colors mode

	CRect m_renderArea;

	Options m_options;      // Current options
	double m_Lx;            // Derived parameters
	double m_Ly;
	double m_Lz;

	Callback *m_callback;   // Current callback
};

template <typename T> int signum(T val) {
    return (T(0) < val) - (val < T(0));
}



#endif
