#pragma once

struct Action
{
	Action(int index, void *data)
	{
		Index = index;
		Data = data;
	}

	Action() {}

	// Index is the index into the global actions array 
	int Index;

	// This is data that is interpreted differently by each action
	void *Data;
};
