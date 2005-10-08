#pragma once

class StatusCallback
{
public:
	virtual void Status(char *msg) = 0;
};