#pragma once

// These are status' that an object can take
namespace Unit
{
	enum Status
	{
		Healthy,
		Incapacitated,
		Wounded,
		Dead,
		NumStatus // Must be last
	};

	enum Action {
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

namespace Team
{
	enum Status
	{
		Healthy,
		Incapacitated,
		Dead,
		NumStatus // Must be last
	};

	enum Action {
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
