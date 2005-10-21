#pragma once

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (P)
#endif

#ifndef NULL
#define NULL 0
#endif

#include <objects\SoldierManager.h>
#include <objects\SquadManager.h>
#include <objects\WeaponManager.h>
#include <objects\VehicleManager.h>
#include <sound\SoundManager.h>
#include <graphics\FontManager.h>
#include <graphics\EffectManager.h>
#include <graphics\Mark.h>
#include <graphics\WidgetManager.h>
#include <world\ElementManager.h>
#include <world\World.h>
#include <misc\StatusCallback.h>
#include <misc\Structs.h>
#include <application\CursorInterface.h>
#include <ai\AStar.h>
#include <states\ObjectStates.h>
#include <states\ObjectActions.h>
#include <states\SoldierStateTransitions.h>

/**
 * This file describes a wrapper for all of our global variables.
 * This is useful so that we don't have to pass around pointers to
 * objects everywhere.
 */
struct WorldConstants
{
	const static int PixelsPerMeter = 6;
};

struct ObjectStatesContainer
{
	// State for our soldiers
	ObjectStates Soldiers;

	// State for out vehicles
};

struct ObjectActionsContainer
{
	// Actions for our soldiers
	ObjectActions Soldiers;

	// Actions for our vehicles
};

struct ObjectStateTransitionsContainer
{
	// State transitions for our soldiers
	SoldierStateTransitions Soldiers;

	// State transitions for our vehicles
};

struct WorldGlobals
{
	/**
	 * First all of the object managers we are going to use.
	 */
	SoldierManager *Soldiers;
	SquadManager *Squads;
	WeaponManager *Weapons;
	VehicleManager *Vehicles;
	EffectManager *Effects;
	ElementManager *Elements;

	/**
	 * Widget managers.
	 */
	WidgetManager *Icons;
	WidgetManager *Terrain;

	/**
	 * Sound managers.
	 */
	SoundManager *Voices;
	SoundManager *SoundEffects;

	/**
	 * A font manager for text.
	 */
	FontManager *Fonts;

	/**
	 * The currently loaded world.
	 */
	World *CurrentWorld;

	/**
	 * Marks used for range finding.
	 */
	Mark *Marks;

	/** 
	 * An A* search algorithm useful for pathfinding
	 */
	AStar Pathing;

	/**
	 * Some debugging variables for rendering.
	 */
	bool bRenderElevation;
	bool bRenderElements;
	bool bRenderStats;
	bool bWeaponFan;
	bool bRenderPaths;
	bool bRenderHelpText;
	bool bRenderBuildingOutlines;

	/**
	 * A structure which contains all of our constants.
	 */
	WorldConstants Constants;

	/**
	 * A structure to hold all of our action and state and state transition
	 * information.
	 */
	ObjectStatesContainer States;
	ObjectActionsContainer Actions;
	ObjectStateTransitionsContainer StateTransitions;

	WorldGlobals() { bRenderElevation=false; bRenderElements=true;bWeaponFan=false;bRenderStats=false;bRenderPaths=false;bRenderHelpText=true;bRenderBuildingOutlines=false; }
};

/**
 * Application specific globals.
 */
struct ApplicationGlobals
{
	/**
	 * Relays status information to the app.
	 */
	StatusCallback *Status;

	/**
	 * An interface to the cursor.
	 */
	CursorInterface *Cursor;

	/**
	 * The current working directory.
	 */
	char CurrentDirectory[256];

	/**
	 * The configuration directory.
	 */
	char ConfigDirectory[256];

	/**
	 * The graphics directory.
	 */
	char GraphicsDirectory[256];

	/**
	 * The maps directory.
	 */
	char MapsDirectory[256];

	/**
	 * The sounds directory.
	 */
	char SoundsDirectory[256];
};

struct Globals
{
	WorldGlobals World;
	ApplicationGlobals Application;
};

extern Globals *g_Globals;
