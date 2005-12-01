#pragma once

class Element
{
public:
	Element(void);
	virtual ~Element(void);
	
	enum Level { Prone=0, Low, Medium, High, Top };

	int Index;
	int Height;
	char Name[32];
	bool BlocksHeight;
	bool Passable;
	unsigned char Cover[4];
	unsigned char Hindrance[4];
	unsigned short Protection[5];
	float Movement[3];

	friend class ElementManager;
};
