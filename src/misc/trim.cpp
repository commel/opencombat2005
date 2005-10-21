#include <ctype.h>
#include <string.h>
#include <misc\trim.h>

char * 
_ltrim(char *src)
{
	char *cp = src;

	if(cp && *cp)
	{
		// find first non-whitespace character
		while(isspace(*cp)) {
			++cp;
		}

		if(cp != src) {
			memcpy(src, cp, (strlen(cp)+1)*sizeof(char));
		}
	}

	return src;
}

char *
_ttrim(char *src)
{
	char *cp = src;

	if(cp && *cp)
	{
		bool bNonSpaceSeen = false;

		// check if string is blank
		while(*cp)
		{
			if(!isspace(*cp)) {
				bNonSpaceSeen = true;
			}
			++cp;
		}

		if(bNonSpaceSeen)
		{
			--cp;

			// find last non-whitespace character
			while((cp >= src) && (isspace(*cp))) {
				*cp-- = '\0';
			}
		}
		else
		{
			// string contains only whitespace characters
			*src = '\0';
		}
	}

	return src;
}
