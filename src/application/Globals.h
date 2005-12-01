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
#include <misc\StrictArray.h>
#include <world\Nationality.h>
#include <graphics\Font.h>

/**
 * This file describes a wrapper for all of our global variables.
 * This is useful so that we don't have to pass around pointers to
 * objects everywhere.
 */
struct WorldConstants
{
	const static int PixelsPerMeter = 5;
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

/**
 * This enum let's us know how a team is controlled.
 */
enum TeamController
{
	PlayerControlled,
	ComputerControlled,
	NumControllers
};

/**
 * The following defines an ID for each player and how many players
 * we are allowing in the game.
 */
#define MAX_PLAYERS	32
typedef int PlayerID;

/**
 * This structure keeps track of the attributes for each team in the game.
 */
struct TeamAttributes
{
	// The objects that belong to this team
	Array<Object> Objects;

	// Is this team computer or player controlled?
	TeamController Controller;

	// The nationality of this team. Nationalities are defined in the
	// Nationalites.xml file, and the order in which they appear
	// is this index below
	NationalityID Nationality;

	// Which player is controlling this team? This ID also corresponds
	// to the index in the Teams array that this structure belongs to.
	PlayerID Player;

	// Which teams are allies of this team?
	StrictArray<PlayerID> Allies;

	// Which teams are enemies of this team?
	// We keep track of enemies and allies separately because
	// we want to allow neutral teams as well. A neutral team is
	// someone who is neither an enemy or an ally
	StrictArray<PlayerID> Enemies;
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
	bool bRenderBuildingInteriors;

	/**
	 * A structure which contains all of our constants.
	 */
	WorldConstants Constants;

	/**
	 * This keeps track of all of the information about each
	 * team in the game, which objects he controls, whether
	 * or not it's a computer or player controlled team, etc.
	 */
	TeamAttributes Teams[MAX_PLAYERS];
	int NumTeams;

	/**
	 * This is the current user of this game.
	 */
	PlayerID CurrentPlayer;

	/**
	 * The nationalities that are possible in this world.
	 */
	StrictArray<Nationality *> Nationalities;

	/**
	 * An array of static font's used in this world.
	 */
	StrictArray<StaticFont *> StaticFonts;

	/**
	 * A structure to hold all of our action and state and state transition
	 * information.
	 */
	ObjectStatesContainer States;
	ObjectActionsContainer Actions;

	WorldGlobals() { bRenderElevation=false; bRenderElements=true;bWeaponFan=false;bRenderStats=false;bRenderPaths=false;bRenderHelpText=true;bRenderBuildingOutlines=false;bRenderBuildingInteriors=false; }
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
