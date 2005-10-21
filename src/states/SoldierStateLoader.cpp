#include <states\SoldierStateLoader.h>
#include <misc\readline.h>
#include <misc\Array.h>
#include <misc\trim.h>
#include <stdio.h>

void
SoldierStateLoader::Load(char *fileName, ObjectStates *states)
{
	char buffer[1024];
	int nread;
	Array<char> stateNames;

	// We need to read in the file and load the states deal thing
	FILE *fp = fopen(fileName, "r");
	while((nread = readline(fp, buffer, 1024)) > 0) {
		// Let's trim our string
		char *p = _ltrim(buffer);
		p = _ttrim(p);
		if(strlen(p) > 0) {
			stateNames.Add(strdup(p));
		}
	}
	fclose(fp);

	// Now go back through the array and add to our dest
	states->NumStates = stateNames.Count;
	states->StateNames = (char **) calloc(states->NumStates, sizeof(char *));
	states->States = (int *) calloc(states->NumStates, sizeof(int));
	for(int i = 0; i < stateNames.Count; ++i) {
		states->States[i] = i;
		states->StateNames[i] = stateNames.Items[i];
	}
}
