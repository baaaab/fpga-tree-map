#ifndef SRC_CFPGAITEM_H_
#define SRC_CFPGAITEM_H_

#define __STDC_FORMAT_MACROS

#include <cstdint>
#include <cstdio>
#include <vector>

#include "CResourceUtilisation.h"
#include "EUtilisationMetric.h"
#include "windirstat/CRect.h"
#include "windirstat/CTreeMap.h"

class CFpgaItem : public CTreeMap::Item
{
public:
	CFpgaItem(const char* name, const CResourceUtilisation& ru, CFpgaItem* parent);
	virtual ~CFpgaItem();

	void addChild(CFpgaItem* child);
	void clear();
	CFpgaItem* getChild(int c) const;
	CFpgaItem* getParent() const;
	CFpgaItem* getPreviousSibling() const;
	CFpgaItem* getNextSibling() const;
	uint32_t getDepth() const;
	CResourceUtilisation& getResourceUtilisation();
	const char* getName() const;
	void printHeirachy() const;
	void printTreeTo(const CFpgaItem* descendant) const;
	void sort();
	void recursivelyCalculateSize();
	void setColour(uint32_t colour);

	// interface functions
	bool            TmiIsLeaf          () const;
	CRect           TmiGetRectangle    () const;
	void            TmiSetRectangle    (const CRect& rc);
	uint32_t        TmiGetGraphColor   () const;
	int             TmiGetChildrenCount() const;
	CTreeMap::Item* TmiGetChild        (int c) const;
	uint64_t        TmiGetLocalSize    () const;
	uint64_t        TmiGetRecursiveSize() const;

	void print(FILE* fh);

	static void SetUtilisationMetric(EUtilisationMetric metric);

private:
	uint32_t        getSelectedMetricSize() const;
	bool            isAncestorOf(const CFpgaItem* other) const;

	CRect _rect;
	std::vector<CFpgaItem*> _children;
	CResourceUtilisation _ru;
	CFpgaItem* _parent;
	char* _name;
	uint64_t _sizeofChildren;;
	uint32_t _colour;

	static EUtilisationMetric _UtilisationMetric;


};

#endif /* SRC_CFPGAITEM_H_ */
