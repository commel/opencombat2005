#pragma once

// A path to follow
struct Path
{
	int X, Y;
	struct Path *Next;
};

// Allocator and De-allocators for Path's
extern Path *AllocatePath();
extern void FreePath(Path *path, bool recurse);