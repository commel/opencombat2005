#pragma once

// These are actions that an object can perform
namespace Action
{
	enum Type {
		Defending,
		Cowering,
		Ambushing,
		Hiding,
		Firing,
		Reloading,
		Moving,
		MovingFast,
		Crawling,
		Sneaking,
		NoTarget,
		NumActions // Must be last
	};
};
