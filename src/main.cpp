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

	CSdlDisplay* display = new CSdlDisplay();

	CTreeMap::Options options = CTreeMap::GetDefaultOptions();
	CTreeMap* treemap = new CTreeMap(NULL);

	while(1)
	{
		treemap->DrawTreemap(display, CRect(0,0, display->getWidth(), display->getHeight()), mrpParser.getItems(), &options);
		display->swapBuffers();
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 1000000000/60;
		nanosleep(&ts, NULL);
	}


	return 0;
}
