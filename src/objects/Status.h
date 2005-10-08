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
};
