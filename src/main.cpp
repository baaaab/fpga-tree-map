#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "CFpgaItem.h"
#include "CMrpParser.h"
#include "CSdlDisplay.h"
#include "windirstat/CRect.h"
#include "windirstat/CTreeMap.h"

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s map_report_file\n", argv[0]);
		exit(1);
	}

	const char* mapReport = argv[1];
	if(access(mapReport, R_OK) != 0)
	{
		fprintf(stderr, "Unable to read file: %s\n", mapReport);
		exit(1);
	}

	CMrpParser mrpParser(mapReport);
	mrpParser.parse();

	CFpgaItem* root = mrpParser.getItems();

	CTreeMap* treemap = new CTreeMap(NULL);

	CSdlDisplay* display = new CSdlDisplay();
	display->run(treemap, mrpParser.getItems());

	return 0;
}
