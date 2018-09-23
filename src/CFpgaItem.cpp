#include "CFpgaItem.h"

EUtilisationMetric CFpgaItem::_UtilisationMetric = EUtilisationMetric::REG;

CFpgaItem::CFpgaItem(const char* name, const CResourceUtilisation& ru, CFpgaItem* parent) :
		_ru(ru),
		_parent(parent),
		_name(NULL)
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
	return depth > 0 ? depth - 1 : depth;
}

CResourceUtilisation& CFpgaItem::getResourceUtilisation()
{
	return _ru;
}

const char* CFpgaItem::getName() const
{
	return _name;
}

void CFpgaItem::addChild(CFpgaItem* child)
{
	_children.push_back(child);
}

bool CFpgaItem::TmiIsLeaf() const
{
	return _children.empty();
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
	return _children.size();
}

CTreeMap::Item* CFpgaItem::TmiGetChild(int c) const
{
	return _children[c];
}

uint64_t CFpgaItem::TmiGetSize() const
{
	uint64_t childSizes = 0;
	if (!TmiIsLeaf())
	{
		for (auto child : _children)
		{
			uint64_t childSize = child->TmiGetSize();
			childSizes += childSize;
		}
	}
	//printf("%40s, _ru.getRegisters() = %4u, childSizes = %u, total = %u\n", _name, _ru.getRegisters(), childSizes,  _ru.getRegisters() + childSizes);
	switch (_UtilisationMetric)
	{
		case EUtilisationMetric::SLICE:
			return childSizes + _ru.getSlices();
		case EUtilisationMetric::REG:
			return childSizes + _ru.getRegisters();
		case EUtilisationMetric::LUT:
			return childSizes + _ru.getLuts();
		case EUtilisationMetric::RAM:
			return childSizes + _ru.getRams();
		case EUtilisationMetric::DSP:
			return childSizes + _ru.getDsps();
		default:
			return 1;
	}
}

void CFpgaItem::print(FILE* fh)
{
// indent
uint32_t depth = getDepth();
for (uint32_t i = 0; i < depth; i++)
{
	fputc('+', fh);
}

fprintf(fh, "%40s, %6u, %6u, %6u, %4u, %4u\n", _name, _ru.getSlices(), _ru.getRegisters(), _ru.getLuts(), _ru.getRams(), _ru.getDsps());
for (auto child : _children)
{
	child->print(fh);
}
}

void CFpgaItem::SetUtilisationMetric(EUtilisationMetric metric)
{
_UtilisationMetric = metric;
}

