#include "CFpgaItem.h"

#include <cstdint>
#include <algorithm>
#include <cinttypes>
#include <cstring>

EUtilisationMetric CFpgaItem::_UtilisationMetric = EUtilisationMetric::REG;

CFpgaItem::CFpgaItem(const char* name, const CResourceUtilisation& ru, CFpgaItem* parent) :
		_ru(ru),
		_parent(parent),
		_name(NULL),
		_sizeofChildren(0)
{
	uint32_t nameLength = strlen(name);
	_name = new char[nameLength + 1];
	memcpy(_name, name, nameLength + 1);
}

CFpgaItem::~CFpgaItem()
{
	clear();
	delete[] _name;
}

void CFpgaItem::clear()
{
	for (auto itr = _children.begin(); itr != _children.end(); ++itr)
	{
		delete *itr;
	}
	_children.clear();
}

CFpgaItem* CFpgaItem::getParent() const
{
	return _parent;
}

uint32_t CFpgaItem::getDepth() const
{
	uint32_t depth = 0;
	CFpgaItem* parent = _parent;
	while (parent)
	{
		depth++;
		parent = parent->getParent();
	}
	return depth;
}

CResourceUtilisation& CFpgaItem::getResourceUtilisation()
{
	return _ru;
}

const char* CFpgaItem::getName() const
{
	return _name;
}

void CFpgaItem::printHeirachy() const
{
	puts("---------------------------------------------");
	const CFpgaItem* root = this;
	while(root->getParent())
	{
		root = root->getParent();
	}
	printf("%c %6" PRIu64 " : %-40s\n", root->isAncestorOf(this) ? '+' : '-', root->TmiGetRecursiveSize(), root->getName());
	root->printTreeTo(this);
}

void CFpgaItem::printTreeTo(const CFpgaItem* descendant) const
{
	if (this == descendant)
	{
		return;
	}
	uint32_t depth = getDepth();
	uint32_t numChildren = TmiGetChildrenCount();
	for (uint32_t c = 0; c < numChildren; c++)
	{
		const CFpgaItem* child = dynamic_cast<const CFpgaItem*>(TmiGetChild(c));
		for (uint32_t d = 0; d < depth + 1; d++)
		{
			if(descendant == child)
			{
				fputs("**", stdout);
			}
			else
			{
				fputs("  ", stdout);
			}
		}
		bool isDescendant = descendant->isAncestorOf(child);
		printf("%c %6" PRIu64 " : %-40s\n", child->TmiIsLeaf() ? ' ' : isDescendant ? '+' : '-', child->TmiGetRecursiveSize(), child->getName());
		if(isDescendant)
		{
			child->printTreeTo(descendant);
		}
	}
}

bool cmp(const CFpgaItem* a, const CFpgaItem* b)
{
	// rsort
	return a->TmiGetRecursiveSize() > b->TmiGetRecursiveSize();
}

void CFpgaItem::sort()
{
	std::sort(_children.begin(), _children.end(), &cmp);
	for(auto child : _children)
	{
		child->sort();
	}
}

void CFpgaItem::recursivelyCalculateSize()
{
	uint64_t childSizes = 0;
	for (auto child : _children)
	{
		child->recursivelyCalculateSize();
		uint64_t childSize = child->TmiGetRecursiveSize();
		childSizes += childSize;
	}
	_sizeofChildren = childSizes;
}

void CFpgaItem::addChild(CFpgaItem* child)
{
	_children.push_back(child);
}

bool CFpgaItem::TmiIsLeaf() const
{
	return TmiGetChildrenCount() == 0;
}

CRect CFpgaItem::TmiGetRectangle() const
{
	return _rect;
}

void CFpgaItem::TmiSetRectangle(const CRect& rc)
{
	_rect = rc;
}

uint32_t CFpgaItem::TmiGetGraphColor() const
{
	return 0x00aa00;
}

int CFpgaItem::TmiGetChildrenCount() const
{
	// must return num non zero size children
	uint32_t count = 0;
	for (auto child : _children)
	{
		if(child->TmiGetRecursiveSize() > 0)
		{
			count ++;
		}
	}
	return count;
}

CTreeMap::Item* CFpgaItem::TmiGetChild(int n) const
{
	// must return n'th non zero size child
	uint32_t count = 0;
	for (auto child : _children)
	{
		if(child->TmiGetRecursiveSize() > 0)
		{
			if(count == n)
			{
				return child;
			}
			count ++;
		}
	}
	return NULL;
}

uint64_t CFpgaItem::TmiGetLocalSize() const
{
	return getSelectedMetricSize();
}

uint64_t CFpgaItem::TmiGetRecursiveSize() const
{
	return getSelectedMetricSize() + _sizeofChildren;
}

void CFpgaItem::print(FILE* fh)
{
// indent
	uint32_t depth = getDepth();
	for (uint32_t i = 0; i < depth; i++)
	{
		fputc('+', fh);
	}

	fprintf(fh, "%70s, %3u, , %" PRIu64 ", %6u, %6u, %6u, %4u, %4u\n", _name, TmiGetChildrenCount(), TmiGetRecursiveSize(), _ru.getSlices(), _ru.getRegisters(), _ru.getLuts(), _ru.getRams(), _ru.getDsps());
	for (auto child : _children)
	{
		child->print(fh);
	}
}

void CFpgaItem::SetUtilisationMetric(EUtilisationMetric metric)
{
	_UtilisationMetric = metric;
}

uint32_t CFpgaItem::getSelectedMetricSize() const
{
	switch (_UtilisationMetric)
	{
		case EUtilisationMetric::SLICE:
			return _ru.getSlices();
		case EUtilisationMetric::REG:
			return _ru.getRegisters();
		case EUtilisationMetric::LUT:
			return _ru.getLuts();
		case EUtilisationMetric::RAM:
			return _ru.getRams();
		case EUtilisationMetric::DSP:
			return _ru.getDsps();
		default:
			return 0;
	}
}

bool CFpgaItem::isAncestorOf(const CFpgaItem* other) const
{
	const CFpgaItem* item = this;
	while(item->getParent())
	{
		if(item->getParent() == other)
		{
			return true;
		}
		item = item->getParent();
	}
	return false;
}

