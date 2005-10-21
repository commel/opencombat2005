#pragma once

// This class represents a generic object state
class State
{
public:
	State();

	// Checks whether this state is on or not
	bool IsSet(unsigned int state);
	
	// Sets and unsets specific states 
	void Set(unsigned int state);
	void UnSet(unsigned int state);

private:
	unsigned __int64 _bits;
};
