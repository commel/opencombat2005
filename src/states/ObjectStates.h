#pragma once

class ObjectStates
{
public:
	int *States;
	char **StateNames;
	int NumStates;
};

// This enum must correspond exactly to the SoldierStates.txt file!
// XXX/GWS: This needs to be done better. I dont like how they are disconnected
namespace SoldierState
{
	enum State
	{
		Standing=0,
		Prone,
		Stopped,
		Moving,
		Firing,
		Walking,
		WalkingSlow,
		Crawling,
		Running,
		Reloading,
		DyingBlownUp,
		DyingBackward,
		DyingForward,
		Dead,
		Reloaded,
		OutOfAmmo,
		NoTarget,
		FindingCover,
		Following,
		FollowingInFormation,
		Defending,
		Ambushing,
	};
};