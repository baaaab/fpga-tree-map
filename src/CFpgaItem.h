#ifndef SRC_CFPGAITEM_H_
#define SRC_CFPGAITEM_H_

#include "CResourceUtilisation.h"
#include "EUtilisationMetric.h"
#include "windirstat/CTreeMap.h"

class CFpgaItem : public CTreeMap::Item
{
public:
	CFpgaItem(const char* name, const CResourceUtilisation& ru, CFpgaItem* parent);
	virtual ~CFpgaItem();

	void addChild(CFpgaItem* child);
	void clear();
	CFpgaItem* getParent() const;
	uint32_t getDepth() const;
	CResourceUtilisation& getResourceUtilisation();
	const char* getName() const;

	// interface functions
	bool            TmiIsLeaf          () const;
	CRect           TmiGetRectangle    () const;
	void            TmiSetRectangle    (const CRect& rc);
	uint32_t        TmiGetGraphColor   () const;
	int             TmiGetChildrenCount() const;
	CTreeMap::Item* TmiGetChild        (int c) const;
	uint64_t        TmiGetSize         () const;

	void print(FILE* fh);

	static void SetUtilisationMetric(EUtilisationMetric metric);

private:
	CRect _rect;
	std::vector<CFpgaItem*> _children;
	CResourceUtilisation _ru;
	CFpgaItem* _parent;
	char* _name;

	static EUtilisationMetric _UtilisationMetric;


};

#endif /* SRC_CFPGAITEM_H_ */
