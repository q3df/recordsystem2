#include <stdio.h>
#include "Logger.h"

int main(int argc, char **argv) {
	printf("TET\n");

	for(int i = 0; i < 1000000; i++) {
		LOG(LOG_DEBUG, "%i", i);
		LOG(LOG_DEBUG, "%i", i);
		LOG(LOG_DEBUG, "%i", i);
	}

	return 0;
}
