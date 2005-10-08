#pragma once

class Element
{
public:
	Element(void);
	virtual ~Element(void);

	inline int GetIndex() { return _index; }
	inline int GetHeight() { return _height; }
	inline bool GetBlocksHeight() { return _blocksHeight; }

protected:
	int _index;
	int _height;
	char _name[32];
	bool _blocksHeight;

	friend class ElementManager;
};
