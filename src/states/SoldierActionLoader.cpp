#include <states\SoldierActionLoader.h>
#include <application\Globals.h>
#include <misc\readline.h>
#include <misc\trim.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static ObjectActions::StateIdx find_state(char *stateName);

void
SoldierActionLoader::Load(char *fileName, ObjectActions *actions)
{
	char buffer[1024];
	int nread;
	Array<char> reqs;
	Array<char> changes;

	// We cannot load our actions if we do not already have a bunch of states
	assert(g_Globals->World.States.Soldiers.NumStates > 0);

	// Let's loop through our input file
	FILE *fp = fopen(fileName, "r");
	
	// Let's pick off the first line, which is our column headers
	assert(readline(fp, buffer, 1024) > 0);

	// Now let's go through each line
	actions->NumActions = 0;
	while((nread = readline(fp, buffer, 1024)) > 0) 
	{
		// Let's tokenize this buffer based on the '\t' character
		char *values[5];
		int i = 0;	
		char *token = strtok(buffer, "\t");
		while(token != NULL)
		{
			char *p = _ltrim(token);
			p = _ttrim(p);
			values[i++] = p;
			token = strtok(NULL, "\t");
		}

		// If we didnt get enough then stop looping
		if(i < 5) 
		{
			break;
		}

		// Now we need to create a new action based on our values
		ObjectActions::Action *action;
		if(actions->NumActions == 0)
		{
			actions->NumActions++;
			actions->Actions = (ObjectActions::Action *)calloc(1, sizeof(ObjectActions::Action));
			action = &(actions->Actions[0]);
		}
		else
		{
			actions->NumActions++;
			actions->Actions = (ObjectActions::Action *)realloc(actions->Actions, actions->NumActions*sizeof(ObjectActions::Action));
			action = &(actions->Actions[actions->NumActions-1]);
		}

		action->Name = strdup(values[0]);
		action->Group = strdup(values[1]);
		action->Time = atol(values[2]);
		action->NumRequirements = 0;
		action->NumAdds = 0;
		action->NumSubtracts = 0;

		// Let's get our requirements
		reqs.Clear();
		token = strtok(values[3], ",");
		while(token != NULL) 
		{
			char *p = _ltrim(token);
			p = _ttrim(p);
			if(strcmp("nil",p) != 0) {
				reqs.Add(strdup(p));
			}
			token = strtok(NULL, ",");
		}
		
		action->NumRequirements = reqs.Count;
		action->Requirements = (ObjectActions::StateIdx *) calloc(action->NumRequirements, sizeof(ObjectActions::StateIdx));
		for(i = 0; i < reqs.Count; ++i)
		{
			action->Requirements[i] = find_state(reqs.Items[i]);
			free(reqs.Items[i]);
		}

		// Let's get our changes
		int nAdds=0,nSubtracts=0;
		changes.Clear();
		token = strtok(values[4], ",");
		while(token != NULL)
		{
			char *p = _ltrim(token);
			p = _ttrim(p);
			
			if(*p == '+')
			{
				++nAdds;
				changes.Add(strdup(p));
				token = strtok(NULL, ",");
			} 
			else if(*p == '-')
			{
				++nSubtracts;
				changes.Add(strdup(p));
				token = strtok(NULL, ",");
			}
			else
			{
				// Ignore it
				token = strtok(NULL, ",");
			}
		}

		action->NumAdds = 0;
		action->Adds = (ObjectActions::StateIdx *)calloc(nAdds, sizeof(ObjectActions::StateIdx));
		action->NumSubtracts = 0;
		action->Subtracts = (ObjectActions::StateIdx *)calloc(nSubtracts, sizeof(ObjectActions::StateIdx));
		for(i = 0; i < changes.Count; ++i)
		{
			char *p = changes.Items[i];
			if(*p == '+')
			{
				action->Adds[action->NumAdds++] = find_state(p+1);
			}
			else if(*p == '-')
			{
				action->Subtracts[action->NumSubtracts++] = find_state(p+1);
			}
			else
			{
				assert(0);
			}
			free(p);
		}
	}
	fclose(fp);
}

ObjectActions::StateIdx 
find_state(char *stateName)
{
	for(int i = 0; i < g_Globals->World.States.Soldiers.NumStates; ++i)
	{
		if(strcmp(g_Globals->World.States.Soldiers.StateNames[i], stateName) == 0)
		{
			return i;
		}
	}
	assert(0);
	return 0;
}

