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
#include "CTreeMap.h"
#include "CColorSpace.h"

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

const CTreeMap::Options CTreeMap::_defaultOptions =
{ KDirStatStyle, false, RGB(0, 0, 0), 0.88, 0.38, 0.91, 0.13, -1.0, -1.0 };

const CTreeMap::Options CTreeMap::_defaultOptionsOld =
{ KDirStatStyle, false, RGB(0, 0, 0), 0.85, 0.4, 0.9, 0.15, -1.0, -1.0 };

const uint32_t CTreeMap::_defaultCushionColors[] =
{ RGB(0, 0, 255), RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 255, 255), RGB(255, 0, 255), RGB(255, 255, 0), RGB(150, 150, 255), RGB(255, 150, 150), RGB(150, 255, 150), RGB(150, 255, 255), RGB(255, 150, 255), RGB(255, 255, 150), RGB(255, 255, 255) };

const uint32_t CTreeMap::_defaultCushionColors256[] =
{ RGB(0, 0, 255), RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 255, 255), RGB(255, 0, 255), RGB(255, 255, 0), RGB(100, 100, 100) };

void CTreeMap::GetDefaultPalette(std::vector<uint32_t>& palette)
{
	if (CColorSpace::Is256Colors())
	{
		palette.resize(sizeof(_defaultCushionColors256) / sizeof(_defaultCushionColors256[0]));
		for (int i = 0; i < sizeof(_defaultCushionColors256) / sizeof(_defaultCushionColors256[0]); i++)
		{
			palette[i] = _defaultCushionColors256[i];
		}

		// We don't do
		// EqualizeColors(_defaultCushionColors256, _countof(_defaultCushionColors256), palette);
		// because on 256 color screens, the resulting colors are not distinguishable.
	}
	else
	{
		EqualizeColors(_defaultCushionColors, sizeof(_defaultCushionColors) / sizeof(_defaultCushionColors[0]), palette);
	}
}

void CTreeMap::EqualizeColors(const uint32_t *colors, int count, std::vector<uint32_t>& out)
{
	out.resize(count);

	for (int i = 0; i < count; i++)
	{
		out[i] = CColorSpace::MakeBrightColor(colors[i], PALETTE_BRIGHTNESS);
	}
}

CTreeMap::Options CTreeMap::GetDefaultOptions()
{
	return _defaultOptions;
}

CTreeMap::Options CTreeMap::GetOldDefaultOptions()
{
	return _defaultOptionsOld;
}

CTreeMap::CTreeMap(Callback *callback)
{
	m_callback = callback;
	SetOptions(&_defaultOptions);
	SetBrightnessFor256();
}

void CTreeMap::SetOptions(const Options *options)
{
	ASSERT(options != NULL);
	m_options = *options;

	// Derive normalized vector here for performance
	const double lx = m_options.lightSourceX;   // negative = left
	const double ly = m_options.lightSourceY;   // negative = top
	static const double lz = 10;

	const double len = sqrt(lx * lx + ly * ly + lz * lz);
	m_Lx = lx / len;
	m_Ly = ly / len;
	m_Lz = lz / len;

	SetBrightnessFor256();
}

CTreeMap::Options CTreeMap::GetOptions()
{
	return m_options;
}

void CTreeMap::SetBrightnessFor256()
{
	if (CColorSpace::Is256Colors())
	{
		m_options.brightness = PALETTE_BRIGHTNESS;
	}
}

#ifdef _DEBUG
void CTreeMap::RecurseCheckTree(Item *item)
{
	if(item->TmiIsLeaf())
	{
		ASSERT(item->TmiGetChildrenCount() == 0);
	}
	else
	{
// TODO: check that children are sorted by size.
		uint64_t sum = 0;
		for(int i = 0; i < item->TmiGetChildrenCount(); i++)
		{
			Item *child = item->TmiGetChild(i);
			sum += child->TmiGetSize();
			RecurseCheckTree(child);
		}
		ASSERT(sum == item->TmiGetSize());
	}
}
#endif

void CTreeMap::DrawTreemap(CSdlDisplay* display, CRect rc, Item *root, const Options *options)
{
#ifdef _DEBUG
	RecurseCheckTree(root);
#endif // _DEBUG

	if (options != NULL)
	{
		SetOptions(options);
	}

	if (rc.getWidth() <= 0 || rc.getHeight() <= 0)
	{
		return;
	}

	if (m_options.grid)
	{
		display->fillSolidRect(rc, m_options.gridColor);
	}
	else
	{
		// We shrink the rectangle here, too.
		// If we didn't do this, the layout of the treemap would
		// change, when grid is switched on and off.
		uint32_t left = rc.getLeft();
		uint32_t right = rc.getRight() - 1;
		uint32_t top = rc.getTop();
		uint32_t bottom = rc.getBottom() - 1;
		if(right >= left && bottom >= top)
		{
			display->drawStraightLine(left, top, right, top, 0x00c0c0c0);
			display->drawStraightLine(left, bottom, right, bottom, 0x00c0c0c0);
			display->drawStraightLine(left, top, left, bottom, 0x00c0c0c0);
			display->drawStraightLine(right, top, right, bottom, 0x00c0c0c0);
		}
	}

	rc.getRight()--;
	rc.getBottom()--;

	if (rc.getWidth() <= 0 || rc.getHeight() <= 0)
	{
		return;
	}

	m_renderArea = rc;

	if (root->TmiGetSize() > 0)
	{
		double surface[4];
		for (int i = 0; i < sizeof(surface)/sizeof(*surface); i++)
		{
			surface[i] = 0;
		}

		// Recursively draw the tree graph
		RecurseDrawGraph(display, root, rc, true, surface, m_options.height, 0);

#ifdef STRONGDEBUG  // slow, but finds bugs!
#ifdef _DEBUG
		for(int x = rc.getLeft(); x < rc.getRight() - m_options.grid; x++)
		{
			for(int y = rc.getTop(); y < rc.getBottom() - m_options.grid; y++)
			{
				ASSERT(FindItemByPoint(root, CPoint(x, y)) != NULL);
			}
		}
#endif
#endif
	}
	else
	{
		display->fillSolidRect(rc, RGB(33, 33, 33));
	}
}

CTreeMap::Item *CTreeMap::FindItemByPoint(Item *item, CPoint point)
{
	ASSERT(item != NULL);
	const CRect& rc = item->TmiGetRectangle();

	if (!rc.ptInRect(point))
	{
		// The only case that this function returns NULL is that
		// point is not inside the rectangle of item.
		//
		// Take notice of
		// (a) the very right an bottom lines, which can be "grid" and
		//     are not covered by the root rectangle,
		// (b) the fact, that WM_MOUSEMOVEs can occur after WM_SIZE but
		//     before WM_PAINT.
		//
		return NULL;
	}

	Item *ret = NULL;

	int gridWidth = m_options.grid ? 1 : 0;

	if (rc.getWidth() <= gridWidth || rc.getHeight() <= gridWidth)
	{
		ret = item;
	}
	else if (item->TmiIsLeaf())
	{
		ret = item;
	}
	else
	{
		ASSERT(item->TmiGetSize() > 0);
		ASSERT(item->TmiGetChildrenCount() > 0);

		for (int i = 0; i < item->TmiGetChildrenCount(); i++)
		{
			Item *child = item->TmiGetChild(i);

			ASSERT(child->TmiGetSize() > 0);

#ifdef _DEBUG
			CRect rcChild(child->TmiGetRectangle());
			ASSERT(rcChild.getRight() >= rcChild.getLeft());
			ASSERT(rcChild.getBottom() >= rcChild.getTop());
			ASSERT(rcChild.getLeft() >= rc.getLeft());
			ASSERT(rcChild.getRight() <= rc.getRight());
			ASSERT(rcChild.getTop() >= rc.getTop());
			ASSERT(rcChild.getBottom() <= rc.getBottom());
#endif
			if (child->TmiGetRectangle().ptInRect(point))
			{
				ret = FindItemByPoint(child, point);
				ASSERT(ret != NULL);
#ifdef STRONGDEBUG
#ifdef _DEBUG
				for(i++; i < item->TmiGetChildrenCount(); i++)
				{
					child = item->TmiGetChild(i);

					if(child->TmiGetSize() == 0)
					{
						break;
					}

					rcChild = child->TmiGetRectangle();
					if(rcChild.getLeft() == -1)
					{
						ASSERT(rcChild.getTop() == -1);
						ASSERT(rcChild.getRight() == -1);
						ASSERT(rcChild.getBottom() == -1);
						break;
					}

					ASSERT(rcChild.getRight() >= rcChild.getLeft());
					ASSERT(rcChild.getBottom() >= rcChild.getTop());
					ASSERT(rcChild.getLeft() >= rc.getLeft());
					ASSERT(rcChild.getRight() <= rc.getRight());
					ASSERT(rcChild.getTop() >= rc.getTop());
					ASSERT(rcChild.getBottom() <= rc.getBottom());
				}
#endif
#endif

				break;
			}
		}
	}

	ASSERT(ret != NULL);

	if (ret == NULL)
	{
		ret = item;
	}

	return ret;
}

void CTreeMap::DrawColorPreview(CSdlDisplay* display, const CRect& rc, uint32_t color, const Options *options)
{
	if (options != NULL)
	{
		SetOptions(options);
	}

	double surface[4];
	for (int i = 0; i < sizeof(surface)/sizeof(*surface); i++)
	{
		surface[i] = 0;
	}

	AddRidge(rc, surface, m_options.height * m_options.scaleFactor);

	m_renderArea = rc;

	// Recursively draw the tree graph
	RenderRectangle(display, CRect(0, 0, rc.getWidth(), rc.getHeight()), surface, color);

	if (m_options.grid)
	{
		display->fillSolidRect(rc, m_options.gridColor);
	}
}

void CTreeMap::RecurseDrawGraph(CSdlDisplay* display, Item *item, const CRect& rc, bool asroot, const double *psurface, double h, uint32_t flags)
{
	ASSERT(rc.getWidth() >= 0);
	ASSERT(rc.getHeight() >= 0);

	ASSERT(item->TmiGetSize() > 0);

	if (m_callback != NULL)
	{
		m_callback->TreemapDrawingCallback();
	}

	item->TmiSetRectangle(rc);

	int gridWidth = m_options.grid ? 1 : 0;

	if (rc.getWidth() <= gridWidth || rc.getHeight() <= gridWidth)
	{
		return;
	}

	double surface[4];

	if (IsCushionShading())
	{
		for (int i = 0; i < sizeof(surface)/sizeof(*surface); i++)
		{
			surface[i] = psurface[i];
		}

		if (!asroot)
		{
			AddRidge(rc, surface, h);
		}
	}

	if (item->TmiIsLeaf())
	{
		RenderLeaf(display, item, surface);
	}
	else
	{
		ASSERT(item->TmiGetChildrenCount() > 0);
		ASSERT(item->TmiGetSize() > 0);

		DrawChildren(display, item, surface, h, flags);
	}
}

// My first approach was to make this member pure virtual and have three
// classes derived from CTreemap. The disadvantage is then, that we cannot
// simply have a member variable of type CTreemap but have to deal with
// pointers, factory methods and explicit destruction. It's not worth.

void CTreeMap::DrawChildren(CSdlDisplay* display, Item *parent, const double *surface, double h, uint32_t flags)
{
	switch (m_options.style)
	{
	case KDirStatStyle:
	{
		KDirStat_DrawChildren(display, parent, surface, h, flags);
	}
		break;

	case SequoiaViewStyle:
	{
		SequoiaView_DrawChildren(display, parent, surface, h, flags);
	}
		break;
	}
}

// I learned this squarification style from the KDirStat executable.
// It's the most complex one here but also the clearest, imho.
//
void CTreeMap::KDirStat_DrawChildren(CSdlDisplay* display, Item *parent, const double *surface, double h, uint32_t /*flags*/)
{
	ASSERT(parent->TmiGetChildrenCount() > 0);

	const CRect& rc = parent->TmiGetRectangle();

	std::vector<double> rows;    // Our rectangle is divided into rows, each of which gets this height (fraction of total height).
	std::vector<int> childrenPerRow;    // childrenPerRow[i] = # of children in rows[i]

	std::vector<double> childWidth; // Widths of the children (fraction of row width).
	childWidth.resize(parent->TmiGetChildrenCount());

	bool horizontalRows = KDirStat_ArrangeChildren(parent, childWidth, rows, childrenPerRow);

	const int width = horizontalRows ? rc.getWidth() : rc.getHeight();
	const int height = horizontalRows ? rc.getHeight() : rc.getWidth();
	ASSERT(width >= 0);
	ASSERT(height >= 0);

	int c = 0;
	double top = horizontalRows ? rc.getTop() : rc.getLeft();
	for (int row = 0; row < rows.size(); row++)
	{
		double fBottom = top + rows[row] * height;
		int bottom = (int) fBottom;
		if (row == rows.size() - 1)
		{
			bottom = horizontalRows ? rc.getBottom() : rc.getRight();
		}
		double left = horizontalRows ? rc.getLeft() : rc.getTop();
		for (int i = 0; i < childrenPerRow[row]; i++, c++)
		{
			Item *child = parent->TmiGetChild(c);
			ASSERT(childWidth[c] >= 0);
			double fRight = left + childWidth[c] * width;
			int right = (int) fRight;

			bool lastChild = (i == childrenPerRow[row] - 1 || childWidth[c + 1] == 0);

			if (lastChild)
			{
				right = horizontalRows ? rc.getRight() : rc.getBottom();
			}

			CRect rcChild;
			if (horizontalRows)
			{
				rcChild.getLeft() = (int) left;
				rcChild.getRight() = right;
				rcChild.getTop() = (int) top;
				rcChild.getBottom() = bottom;
			}
			else
			{
				rcChild.getLeft() = (int) top;
				rcChild.getRight() = bottom;
				rcChild.getTop() = (int) left;
				rcChild.getBottom() = right;
			}

#ifdef _DEBUG
			if(rcChild.getWidth() > 0 && rcChild.getHeight()() > 0)
			{
				CRect test;
				test.IntersectRect(parent->TmiGetRectangle(), rcChild);
				ASSERT(test == rcChild);
			}
#endif

			RecurseDrawGraph(display, child, rcChild, false, surface, h * m_options.scaleFactor, 0);

			if (lastChild)
			{
				i++, c++;

				if (i < childrenPerRow[row])
				{
					parent->TmiGetChild(c)->TmiSetRectangle(CRect(-1, -1, -1, -1));
				}

				c += childrenPerRow[row] - i;
				break;
			}

			left = fRight;
		}
		// This asserts due to rounding error: ASSERT(left == (horizontalRows ? rc.getRight() : rc.getBottom()));
		top = fBottom;
	}
	// This asserts due to rounding error: ASSERT(top == (horizontalRows ? rc.getBottom() : rc.getRight()));
}

// return: whether the rows are horizontal.
//
bool CTreeMap::KDirStat_ArrangeChildren(Item *parent, std::vector<double>& childWidth, std::vector<double>& rows, std::vector<int>& childrenPerRow)
{
	ASSERT(!parent->TmiIsLeaf());
	ASSERT(parent->TmiGetChildrenCount() > 0);

	if (parent->TmiGetSize() == 0)
	{
		rows.push_back(1.0);
		childrenPerRow.push_back(parent->TmiGetChildrenCount());
		for (int i = 0; i < parent->TmiGetChildrenCount(); i++)
		{
			childWidth[i] = 1.0 / parent->TmiGetChildrenCount();
		}
		return true;
	}

	bool horizontalRows = (parent->TmiGetRectangle().getWidth() >= parent->TmiGetRectangle().getHeight());

	double width = 1.0;
	if (horizontalRows)
	{
		if (parent->TmiGetRectangle().getHeight() > 0)
		{
			width = (double) parent->TmiGetRectangle().getWidth() / parent->TmiGetRectangle().getHeight();
		}
	}
	else
	{
		if (parent->TmiGetRectangle().getWidth() > 0)
		{
			width = (double) parent->TmiGetRectangle().getHeight() / parent->TmiGetRectangle().getWidth();
		}
	}

	int nextChild = 0;
	while (nextChild < parent->TmiGetChildrenCount())
	{
		int childrenUsed;
		rows.push_back(KDirStat_CalcutateNextRow(parent, nextChild, width, childrenUsed, childWidth));
		childrenPerRow.push_back(childrenUsed);
		nextChild += childrenUsed;
	}

	return horizontalRows;
}

double CTreeMap::KDirStat_CalcutateNextRow(Item *parent, const int nextChild, double width, int& childrenUsed, std::vector<double>& childWidth)
{
	int i = 0;
	static const double _minProportion = 0.4;
	ASSERT(_minProportion < 1);

	ASSERT(nextChild < parent->TmiGetChildrenCount());
	ASSERT(width >= 1.0);

	const double mySize = (double) parent->TmiGetSize();
	ASSERT(mySize > 0);
	uint64_t sizeUsed = 0;
	double rowHeight = 0;

	for (i = nextChild; i < parent->TmiGetChildrenCount(); i++)
	{
		uint64_t childSize = parent->TmiGetChild(i)->TmiGetSize();
		if (childSize == 0)
		{
			ASSERT(i > nextChild);  // first child has size > 0
			break;
		}

		sizeUsed += childSize;
		double virtualRowHeight = sizeUsed / mySize;
		ASSERT(virtualRowHeight > 0);
		ASSERT(virtualRowHeight <= 1);

		// Rectangle(mySize)    = width * 1.0
		// Rectangle(childSize) = childWidth * virtualRowHeight
		// Rectangle(childSize) = childSize / mySize * width

		double childWidth_ = childSize / mySize * width / virtualRowHeight;

		if (childWidth_ / virtualRowHeight < _minProportion)
		{
			ASSERT(i > nextChild); // because width >= 1 and _minProportion < 1.
			// For the first child we have:
			// childWidth / rowHeight
			// = childSize / mySize * width / rowHeight / rowHeight
			// = childSize * width / sizeUsed / sizeUsed * mySize
			// > childSize * mySize / sizeUsed / sizeUsed
			// > childSize * childSize / childSize / childSize
			// = 1 > _minProportion.
			break;
		}
		rowHeight = virtualRowHeight;
	}
	ASSERT(i > nextChild);

	// Now i-1 is the last child used
	// and rowHeight is the height of the row.

	// We add the rest of the children, if their size is 0.
	while (i < parent->TmiGetChildrenCount() && parent->TmiGetChild(i)->TmiGetSize() == 0)
	{
		i++;
	}

	childrenUsed = i - nextChild;

	// Now as we know the rowHeight, we compute the widths of our children.
	for (i = 0; i < childrenUsed; i++)
	{
		// Rectangle(1.0 * 1.0) = mySize
		double rowSize = mySize * rowHeight;
		double childSize = (double) parent->TmiGetChild(nextChild + i)->TmiGetSize();
		double cw = childSize / rowSize;
		ASSERT(cw >= 0);
		childWidth[nextChild + i] = cw;
	}

	return rowHeight;
}

// The classical squarification method.
//
void CTreeMap::SequoiaView_DrawChildren(CSdlDisplay* display, Item *parent, const double *surface, double h, uint32_t /*flags*/)
{
	// Rest rectangle to fill
	CRect remaining(parent->TmiGetRectangle());

	ASSERT(remaining.getWidth() > 0);
	ASSERT(remaining.getHeight() > 0);

	// Size of rest rectangle
	uint64_t remainingSize = parent->TmiGetSize();
	ASSERT(remainingSize > 0);

	// Scale factor
	const double sizePerSquarePixel = (double) parent->TmiGetSize() / remaining.getWidth() / remaining.getHeight();

	// First child for next row
	int head = 0;

	// At least one child left
	while (head < parent->TmiGetChildrenCount())
	{
		ASSERT(remaining.getWidth() > 0);
		ASSERT(remaining.getHeight() > 0);

		// How we divide the remaining rectangle
		bool horizontal = (remaining.getWidth() >= remaining.getHeight());

		// Height of the new row
		const int height = horizontal ? remaining.getHeight() : remaining.getWidth();

		// Square of height in size scale for ratio formula
		const double hh = (height * height) * sizePerSquarePixel;
		ASSERT(hh > 0);

		// Row will be made up of child(rowBegin)...child(rowEnd - 1)
		int rowBegin = head;
		int rowEnd = head;

		// Worst ratio so far
		double worst = std::numeric_limits<double>::max();

		// Maximum size of children in row
		uint64_t rmax = parent->TmiGetChild(rowBegin)->TmiGetSize();

		// Sum of sizes of children in row
		uint64_t sum = 0;

		// This condition will hold at least once.
		while (rowEnd < parent->TmiGetChildrenCount())
		{
			// We check a virtual row made up of child(rowBegin)...child(rowEnd) here.

			// Minimum size of child in virtual row
			uint64_t rmin = parent->TmiGetChild(rowEnd)->TmiGetSize();

			// If sizes of the rest of the children is zero, we add all of them
			if (rmin == 0)
			{
				rowEnd = parent->TmiGetChildrenCount();
				break;
			}

			// Calculate the worst ratio in virtual row.
			// Formula taken from the "Squarified Treemaps" paper.
			// (http://http://www.win.tue.nl/~vanwijk/)

			const double ss = ((double) sum + rmin) * ((double) sum + rmin);
			const double ratio1 = hh * rmax / ss;
			const double ratio2 = ss / hh / rmin;

			const double nextWorst = std::max(ratio1, ratio2);

			// Will the ratio get worse?
			if (nextWorst > worst)
			{
				// Yes. Don't take the virtual row, but the
				// real row (child(rowBegin)..child(rowEnd - 1))
				// made so far.
				break;
			}

			// Here we have decided to add child(rowEnd) to the row.
			sum += rmin;
			rowEnd++;

			worst = nextWorst;
		}

		// Row will be made up of child(rowBegin)...child(rowEnd - 1).
		// sum is the size of the row.

		// As the size of parent is greater than zero, the size of
		// the first child must have been greater than zero, too.
		ASSERT(sum > 0);

		// Width of row
		int width = (horizontal ? remaining.getWidth() : remaining.getHeight());
		ASSERT(width > 0);

		if (sum < remainingSize)
			width = (int) ((double) sum / remainingSize * width);
		// else: use up the whole width
		// width may be 0 here.

		// Build the rectangles of children.
		CRect rc;
		double fBegin;
		if (horizontal)
		{
			rc.getLeft() = remaining.getLeft();
			rc.getRight() = remaining.getLeft() + width;
			fBegin = remaining.getTop();
		}
		else
		{
			rc.getTop() = remaining.getTop();
			rc.getBottom() = remaining.getTop() + width;
			fBegin = remaining.getLeft();
		}

		// Now put the children into their places
		for (int i = rowBegin; i < rowEnd; i++)
		{
			int begin = (int) fBegin;
			double fraction = (double) (parent->TmiGetChild(i)->TmiGetSize()) / sum;
			double fEnd = fBegin + fraction * height;
			int end = (int) fEnd;

			bool lastChild = (i == rowEnd - 1 || parent->TmiGetChild(i + 1)->TmiGetSize() == 0);

			if (lastChild)
			{
				// Use up the whole height
				end = (horizontal ? remaining.getTop() + height : remaining.getLeft() + height);
			}

			if (horizontal)
			{
				rc.getTop() = begin;
				rc.getBottom() = end;
			}
			else
			{
				rc.getLeft() = begin;
				rc.getRight() = end;
			}

			ASSERT(rc.getLeft() <= rc.getRight());
			ASSERT(rc.getTop() <= rc.getBottom());

			ASSERT(rc.getLeft() >= remaining.getLeft());
			ASSERT(rc.getRight() <= remaining.getRight());
			ASSERT(rc.getTop() >= remaining.getTop());
			ASSERT(rc.getBottom() <= remaining.getBottom());

			RecurseDrawGraph(display, parent->TmiGetChild(i), rc, false, surface, h * m_options.scaleFactor, 0);

			if (lastChild)
				break;

			fBegin = fEnd;
		}

		// Put the next row into the rest of the rectangle
		if (horizontal)
		{
			remaining.getLeft() += width;
		}
		else
		{
			remaining.getTop() += width;
		}

		remainingSize -= sum;

		ASSERT(remaining.getLeft() <= remaining.getRight());
		ASSERT(remaining.getTop() <= remaining.getBottom());

		ASSERT(remainingSize >= 0);

		head += (rowEnd - rowBegin);

		if (remaining.getWidth() <= 0 || remaining.getHeight() <= 0)
		{
			if (head < parent->TmiGetChildrenCount())
			{
				parent->TmiGetChild(head)->TmiSetRectangle(CRect(-1, -1, -1, -1));
			}

			break;
		}
	}
	ASSERT(remainingSize == 0);
	ASSERT(remaining.getLeft() == remaining.getRight() || remaining.getTop() == remaining.getBottom());
}

bool CTreeMap::IsCushionShading()
{
	return m_options.ambientLight < 1.0 && m_options.height > 0.0 && m_options.scaleFactor > 0.0;
}

void CTreeMap::RenderLeaf(CSdlDisplay* display, Item *item, const double *surface)
{
	CRect rc = item->TmiGetRectangle();

	if (m_options.grid)
	{
		rc.getTop()++;
		rc.getLeft()++;if
(		rc.getWidth() <= 0 || rc.getHeight() <= 0)
		{
			return;
		}
	}

	RenderRectangle(display, rc, surface, item->TmiGetGraphColor());
}

void CTreeMap::RenderRectangle(CSdlDisplay* display, const CRect& rc, const double *surface, uint32_t color)
{
	double brightness = m_options.brightness;

	if ((color & COLORFLAG_MASK) != 0)
	{
		uint32_t flags = (color & COLORFLAG_MASK);
		color = CColorSpace::MakeBrightColor(color, PALETTE_BRIGHTNESS);

		if ((flags & COLORFLAG_DARKER) != 0)
		{
			brightness *= 0.66;
		}
		else
		{
			brightness *= 1.2;
			if (brightness > 1.0)
			{
				brightness = 1.0;
			}
		}
	}

	if (IsCushionShading())
	{
		DrawCushion(display, rc, surface, color, brightness);
	}
	else
	{
		DrawSolidRect(display, rc, color, brightness);
	}
}

void CTreeMap::DrawSolidRect(CSdlDisplay* display, const CRect& rc, uint32_t col, double brightness)
{
	int red = RGB_GET_RVALUE(col);
	int green = RGB_GET_GVALUE(col);
	int blue = RGB_GET_BVALUE(col);

	const double factor = brightness / PALETTE_BRIGHTNESS;

	red = (int) (red * factor);
	green = (int) (green * factor);
	blue = (int) (blue * factor);

	CColorSpace::NormalizeColor(red, green, blue);

	display->fillSolidRect(rc, BGR(blue, green, red));
}

void CTreeMap::DrawCushion(CSdlDisplay* display, const CRect& rc, const double *surface, uint32_t col, double brightness)
{
	// Cushion parameters
	const double Ia = m_options.ambientLight;

	// Derived parameters
	const double Is = 1 - Ia;   // shading

	const double colR = RGB_GET_RVALUE(col);
	const double colG = RGB_GET_GVALUE(col);
	const double colB = RGB_GET_BVALUE(col);

	uint32_t* screen = display->getScreen();

	for (int iy = rc.getTop(); iy < rc.getBottom(); iy++)
		for (int ix = rc.getLeft(); ix < rc.getRight(); ix++)
		{
			double nx = -(2 * surface[0] * (ix + 0.5) + surface[2]);
			double ny = -(2 * surface[1] * (iy + 0.5) + surface[3]);
			double cosa = (nx * m_Lx + ny * m_Ly + m_Lz) / sqrt(nx * nx + ny * ny + 1.0);
			if (cosa > 1.0)
			{
				cosa = 1.0;
			}

			double pixel = Is * cosa;
			if (pixel < 0)
			{
				pixel = 0;
			}

			pixel += Ia;
			ASSERT(pixel <= 1.0);

			// Now, pixel is the brightness of the pixel, 0...1.0.

			// Apply contrast.
			// Not implemented.
			// Costs performance and nearly the same effect can be
			// made width the m_options->ambientLight parameter.
			// pixel = pow(pixel, m_options->contrast);

			// Apply "brightness"
			pixel *= brightness / PALETTE_BRIGHTNESS;

			// Make color value
			int red = (int) (colR * pixel);
			int green = (int) (colG * pixel);
			int blue = (int) (colB * pixel);

			CColorSpace::NormalizeColor(red, green, blue);

			// ... and set!
			screen[ix + iy * display->getWidth()] = BGR(blue, green, red);
		}
}

void CTreeMap::AddRidge(const CRect& rc, double *surface, double h)
{
	/*
	 Unoptimized:

	 if(rc.getWidth() > 0)
	 {
	 surface[2]+= 4 * h * (rc.getRight() + rc.getLeft()) / (rc.getRight() - rc.getLeft());
	 surface[0]-= 4 * h / (rc.getRight() - rc.getLeft());
	 }

	 if(rc.getHeight() > 0)
	 {
	 surface[3]+= 4 * h * (rc.getBottom() + rc.getTop()) / (rc.getBottom() - rc.getTop());
	 surface[1]-= 4 * h / (rc.getBottom() - rc.getTop());
	 }
	 */

	// Optimized (gained 15 ms of 1030):
	int width = rc.getWidth();
	int height = rc.getHeight();

	ASSERT(width > 0 && height > 0);

	double h4 = 4 * h;

	double wf = h4 / width;
	surface[2] += wf * (rc.getRight() + rc.getLeft());
	surface[0] -= wf;

	double hf = h4 / height;
	surface[3] += hf * (rc.getBottom() + rc.getTop());
	surface[1] -= hf;
}
