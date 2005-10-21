#include <states\State.h>

State::State()
{
	_bits = 0;
}

bool
State::IsSet(unsigned int state)
{
	unsigned __int64 flag = 1;
	flag <<= state;
	return (_bits&flag) != 0;
}

void
State::Set(unsigned int state)
{
	unsigned __int64 flag = 1;
	flag <<= state;
	_bits |= flag;
}

void
State::UnSet(unsigned int state)
{
	unsigned __int64 flag = 1;
	flag <<= state;
	flag = ~flag;
	_bits &= flag;
}
