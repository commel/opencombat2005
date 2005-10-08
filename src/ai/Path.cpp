#include ".\path.h"
#include <stdlib.h>

// XXX/GWS: Both of these need to use a memory pool allocator!!!
Path *
AllocatePath()
{
	return (Path *) calloc(1, sizeof(Path));
}

void 
FreePath(Path *path, bool recurse)
{
	if(NULL == path) return;

	if(recurse) {
		Path *t = NULL;
		while(path != NULL) {
			t = path->Next;
			free(path);
			path = t;
		}
	} else {
		free(path);
	}
}
