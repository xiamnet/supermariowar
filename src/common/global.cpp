#include "global.h"
#include <math.h>

#include "FileIO.h"
#include "GameValues.h"
#include "gfx.h"

#include <cassert>

//1.8.0.0 == Release to staff
//1.8.0.1 == Second release to staff
//1.8.0.2 == beta1
//1.8.0.3 == beta2
//1.8.0.4 == final
//1.9.0.0 == neagix work-in-progress, not released
//2.0.0.0 == fluffypillow netplay code
int g_iVersion[] = {2, 0, 0, 0};

// main game directory, read from command line argument
char *RootDataDirectory = "data";

CGame *smw;
CResourceManager *rm;

bool VersionIsEqual(int iVersion[], short iMajor, short iMinor, short iMicro, short iBuild)
{
    return iVersion[0] == iMajor && iVersion[1] == iMinor && iVersion[2] == iMicro && iVersion[3] == iBuild;
}

bool VersionIsEqualOrBefore(int iVersion[], short iMajor, short iMinor, short iMicro, short iBuild)
{
    if (iVersion[0] < iMajor)
        return true;

    if (iVersion[0] == iMajor) {
        if (iVersion[1] < iMinor)
            return true;

        if (iVersion[1] == iMinor) {
            if (iVersion[2] < iMicro)
                return true;

            if (iVersion[2] == iMicro) {
                return iVersion[3] <= iBuild;
            }
        }
    }

    return false;
}

bool VersionIsEqualOrAfter(int iVersion[], short iMajor, short iMinor, short iMicro, short iBuild)
{
    if (iVersion[0] > iMajor)
        return true;

    if (iVersion[0] == iMajor) {
        if (iVersion[1] > iMinor)
            return true;

        if (iVersion[1] == iMinor) {
            if (iVersion[2] > iMicro)
                return true;

            if (iVersion[2] == iMicro) {
                return iVersion[3] >= iBuild;
            }
        }
    }

    return false;
}

gv game_values;

FiltersList *filterslist;  //Filters list must be initiallized before maps list because it is used in maplist constructor
MapList *maplist;
SkinList *skinlist;
AnnouncerList *announcerlist;
MusicList *musiclist;
WorldMusicList *worldmusiclist;
GraphicsList *menugraphicspacklist;
GraphicsList *worldgraphicspacklist;
GraphicsList *gamegraphicspacklist;
SoundsList *soundpacklist;
TourList *tourlist;
WorldList *worldlist;

std::vector<MapMusicOverride*> mapmusicoverrides;
std::vector<WorldMusicOverride*> worldmusicoverrides;

CMap      *g_map;
CTilesetManager *g_tilesetmanager;

CObjectContainer noncolcontainer;
CObjectContainer objectcontainer[3];

bool g_fLoadMessages = true;

bool  fResumeMusic = true;

//Network stuff
int g_iNextNetworkID = 0;
int g_iNextMessageID = 0;
char szIPString[32] = "";

//Joystick-Init
SDL_Joystick **joysticks = NULL;
short joystickcount = 0;


const char * GameInputNames[NUM_KEYS] = {"Left", "Right", "Jump", "Down", "Turbo", "Use Item", "Pause", "Exit"};
const char * MenuInputNames[NUM_KEYS] = {"Up", "Down", "Left", "Right", "Select", "Cancel", "Random", "Fast Map"};

/*
0 == poison mushroom
1 == 1up
2 == 2up
3 == 3up
4 == 5up
5 == flower
6 == star
7 == clock
8 == bobomb
9 == pow
10 == bulletbill
11 == hammer
12 == green shell
13 == red shell
14 == spike shell
15 == buzzy shell
16 == mod
17 == feather
18 == mystery mushroom
19 == boomerang
20 == tanooki
21 == ice wand
22 == podoboo
23 == bombs
24 == leaf
25 == pwings
*/


short g_iDefaultPowerupPresets[NUM_POWERUP_PRESETS][NUM_POWERUPS] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
    {5,10, 4, 2, 1,10, 8, 4, 4, 2, 2, 4, 8, 4, 2, 4, 2, 4, 5, 6, 6, 3, 4, 4, 5, 3}, //Custom 1
    {5,10, 4, 2, 1,10, 8, 4, 4, 2, 2, 4, 8, 4, 2, 4, 2, 4, 5, 6, 6, 3, 4, 4, 5, 3}, //Custom 2
    {5,10, 4, 2, 1,10, 8, 4, 4, 2, 2, 4, 8, 4, 2, 4, 2, 4, 5, 6, 6, 3, 4, 4, 5, 3}, //Custom 3
    {5,10, 4, 2, 1,10, 8, 4, 4, 2, 2, 4, 8, 4, 2, 4, 2, 4, 5, 6, 6, 3, 4, 4, 5, 3}, //Custom 4
    {5,10, 4, 2, 1,10, 8, 4, 4, 2, 2, 4, 8, 4, 2, 4, 2, 4, 5, 6, 6, 3, 4, 4, 5, 3}, //Custom 5
    {5,10, 5, 3, 1,10, 2, 3, 4, 3, 3, 4, 9, 6, 2, 4, 4, 7, 5, 6, 6, 3, 2, 2, 5, 5}, //Balanced
    {5, 0, 0, 0, 0,10, 0, 0, 0, 0, 0, 7, 9, 6, 3, 4, 0, 0, 0, 4, 0, 2, 0, 2, 0, 0}, //Weapons Only
    {0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0}, //Koopa Bros Weapons
    {5,10, 7, 5, 2, 0, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 5, 0, 3, 0, 0, 0, 8, 6}, //Support Items
    {3, 3, 1, 0, 0, 0, 0, 0, 4, 2, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 3, 3, 0, 0}, //Booms and Shakes
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 2, 0, 0, 0, 8, 3}, //Fly and Glide
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 4, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //Shells
    {5, 8, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0}, //Mushrooms Only
    {3, 5, 0, 0, 0, 5, 2, 0, 0, 0, 3, 0, 6, 4, 1, 3, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, //Super Mario Bros 1
    {0, 5, 0, 0, 0, 0, 2, 4, 3, 2, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0}, //Super Mario Bros 2
    {0, 3, 0, 0, 0, 8, 4, 0, 0, 0, 5, 2,10, 9, 4, 5, 0, 0, 0, 4, 3, 0, 4, 0, 8, 5}, //Super Mario Bros 3
    {0,10, 0, 0, 0,10, 6, 0, 0, 0, 2, 0, 8, 4, 2, 4, 0, 4, 0, 0, 0, 0, 5, 0, 0, 0}, //Super Mario World
};

//Koopa Bros Weapons
//Mushrooms Only
//Super Mario Bros (All mushrooms ((poison was in lost levels, that counts as SMB1)), fireflower, star, and maybe the shells)
//Super Mario Bros 2 (All mushrooms but poison, star, clock, pow, green/red shell, bombs)
//One I'd like to see is SMWorld (esc{sp}) only stuff...
//0,10,0,1,0,10,6,0,0,0,2,0,8,4,2,4,0,10,0,5,2,0,0

short g_iCurrentPowerupPresets[NUM_POWERUP_PRESETS][NUM_POWERUPS];


//Conversion to switch 1.6 tiles into 1.7 tiles
short g_iTileConversion[] =    {0,1,2,3,4,5,6,7,8,9,
                                10,11,12,13,14,15,575,670,702,703,
                                32,33,34,35,36,37,38,39,40,41,
                                42,43,44,45,46,47,332,331,330,637,
                                64,65,66,67,68,69,70,71,72,73,
                                74,75,76,77,78,79,365,299,366,853,
                                537,595,505,658,659,656,657,774,775,776,
                                540,96,97,98,643,644,645,26,27,506,
                                122,123,124,690,691,688,689,745,746,747,
                                569,128,129,130,704,677,907,90,91,572,
                                931,602,539,885,728,729,730,731,186,187,
                                188,160,161,162,736,192,194,30,31,898,
                                737,738,739,800,760,761,762,763,218,219,
                                220,864,865,509,768,224,226,62,63,930,
                                769,770,771,508,598,599,600,507,601,510,
                                603,896,897,192,193,260,259,193,194,541,
                                627,699,697,940,941,942,860,861,862,250,
                                252,543,158,498,499,500,922,924,854,886,
                                605,125,126,127,720,721,752,754,753,722,
                                723,928,929,563,531,532,923,571,882,851,
                                309,310,311,343,278,341,99,100,101,489,
                                490,491,384,385,386,147,113,148,914,664,
                                373,374,375,376,310,377,131,132,133,553,
                                554,555,416,417,418,179,145,180,946,570,
                                867,868,869,213,214,215,163,164,165,566,
                                567,568,448,449,450,863,530,504,892,883,
                                899,900,901,245,246,247,777,778,779,250,
                                251,252,856,857,858,859,562,712,893,905,
                                908,909,910,624,625,626,592,593,594,282,
                                283,284,888,889,890,891,710,711,894,937
                               };

void _load_drawmsg(const std::string& f)
{
    if (g_fLoadMessages) {
        /*
        static SDL_Rect r;
        r.x = 0;
        r.y = 0;
        r.w = 500;
        r.h = (Uint16)menu_font_small.getHeight();
        Uint32 col = SDL_MapRGB(screen->format, 189, 251, 255);
        SDL_FillRect(screen, &r, col);      //fill empty area
        */

        rm->menu_font_small.draw(0, 0, f.c_str());
    }
}

void _load_waitforkey()
{
#ifndef __EMSCRIPTEN__
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN)
                return;
            if (event.type == SDL_JOYBUTTONDOWN)
                return;
        }

        SDL_Delay(10);
    }
#endif
}

//Takes a path to a file and gives you back the file name (with or without author) as a char *
void GetNameFromFileName(char * szName, const char * szFileName, bool fStripAuthor)
{
#ifdef _XBOX
    const char * p = strrchr(szFileName, '\\');
#else
    const char * p = strrchr(szFileName, '/');
#endif

    if (!p)
        p = szFileName;
    else
        p++;

    strcpy(szName, p);

    if (fStripAuthor) {
        char * pUnderscore = strchr(szName, '_');
        if (pUnderscore)
            strcpy(szName, ++pUnderscore);
    }

    char * pLastPeriod = strrchr(szName, '.');

    if (pLastPeriod)
        *pLastPeriod = 0;
}

//Takes a file name and gives you back just the name of the file with no author or file extention
//and the first letter of the name will come back capitalized
std::string stripCreatorAndDotMap(const std::string &filename)
{
    size_t firstUnderscore = filename.find("_");    //find first _
    if (firstUnderscore == std::string::npos)   //if not found start with first character
        firstUnderscore = 0;
    else
        firstUnderscore++;                      //we don't want the _

    std::string withoutPrefix = filename.substr(firstUnderscore);   //substring without bla_ and .map (length-4)
    withoutPrefix = withoutPrefix.substr(0, withoutPrefix.length() - 4);        //i have no idea why this doesn't work if i do it like this: (leaves .map if the map starts with an underscore)
    //                                                              return filename.substr(firstUnderscore, filename.length()-4);

    //Capitalize the first letter so the hash table sorting works correctly
    if (withoutPrefix[0] >= 97 && withoutPrefix[0] <= 122)
        withoutPrefix[0] -= 32;

    return withoutPrefix;
}

//Takes a path to a file and gives you back just the name of the file with no author or file extention
std::string stripPathAndExtension(const std::string &path)
{
    size_t chopHere = path.find("_");   //find first _
    if (chopHere == std::string::npos) {    //if not found, then find the beginning of the filename
        chopHere = path.find_last_of(getDirectorySeperator());  //find last /
        if (chopHere == std::string::npos)  //if not found, start with first character
            chopHere = 0;
        else
            chopHere++;                     //we don't want the /
    } else {
        chopHere++;                     //we don't want the _
    }

    std::string withoutPath = path.substr(chopHere);    //substring without bla_
    withoutPath = withoutPath.substr(0, withoutPath.length() - 4); //and without extension like .map (length-4)

    return withoutPath;
}

short iScoreboardPlayerOffsetsX[3][3] = {{40, 0, 0}, {19, 59, 0}, {6, 40, 74}};
short iKingOfTheHillZoneLimits[4][4] = {{0, 0, 1, 2}, {0, 1, 2, 4}, {0, 2, 4, 7}, {0, 2, 5, 12}};

const char * g_szBackgroundConversion[26] = {"Land_Classic.png",
        "Castle_Dungeon.png",
        "Desert_Pyramids.png",
        "Ghost_GhostHouse.png",
        "Underground_Cave.png",
        "Clouds_AboveTheClouds.png",
        "Castle_GoombaHall.png",
        "Platforms_GreenSpottedHills.png",
        "Snow_SnowTrees.png",
        "Desert_Desert.png",
        "Underground_BrownRockWall.png",
        "Land_CastleWall.png",
        "Clouds_Clouds.png",
        "Land_GreenMountains.png",
        "Land_InTheTrees.png",
        "Battle_Manor.png",
        "Platforms_JaggedGreenStones.png",
        "Underground_RockWallAndPlants.png",
        "Underground_DarkPipes.png",
        "Bonus_StarryNight.png",
        "Platforms_CloudsAndWaterfalls.png",
        "Battle_GoombaPillars.png",
        "Bonus_HillsAtNight.png",
        "Castle_CastlePillars.png",
        "Land_GreenHillsAndClouds.png",
        "Platforms_BlueSpottedHills.png"
                                            };

//Conversion from backgrounds to music categories created by NMcCoy
short g_iMusicCategoryConversion[26] = {0, 3, 8, 5, 1, 9, 3, 4, 10, 8, 1, 0, 9, 0, 0, 7, 4, 1, 1, 6, 4, 7, 6, 3, 0, 4};
const char * g_szMusicCategoryNames[MAXMUSICCATEGORY] = {"Land", "Underground", "Underwater", "Castle", "Platforms", "Ghost", "Bonus", "Battle", "Desert", "Clouds", "Snow"};
short g_iDefaultMusicCategory[MAXMUSICCATEGORY] = {0, 0, 0, 0, 0, 1, 2, 3, 0, 0, 0};

const char * g_szWorldMusicCategoryNames[MAXWORLDMUSICCATEGORY] = {"Grass", "Desert", "Water", "Giant", "Sky", "Ice", "Pipe", "Dark", "Space"};

const char * g_szAutoFilterNames[NUM_AUTO_FILTERS] = {"Death Tiles", "Warps", "Ice", "Item Boxes", "Breakable Blocks", "Throwable Blocks", "On/Off Blocks", "Platforms", "Hazards", "Item Destroyable Blocks", "Hidden Blocks", "Map Items"};
short g_iAutoFilterIcons[NUM_AUTO_FILTERS] = {37, 29, 33, 1, 0, 6, 40, 73, 19, 87, 17, 118};


void LoadCurrentMapBackground()
{
    char filename[128];
    sprintf(filename, "gfx/packs/backgrounds/%s", g_map->szBackgroundFile);
    std::string path = convertPath(filename, gamegraphicspacklist->current_name());

    //if the background file doesn't exist, use the classic background
    if (!File_Exists(path))
        path = convertPath("gfx/packs/backgrounds/Land_Classic.png", gamegraphicspacklist->current_name());

    gfx_loadimagenocolorkey(&rm->spr_background, path);
}

//TODO  - Review what is colliding with what and remove duplicates (i.e. shell vs. throwblock and throwblock vs. shell should only detect one way)
short g_iCollisionMap[MOVINGOBJECT_LAST][MOVINGOBJECT_LAST] = {
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_none = 0
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_powerup = 1
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1}, //movingobject_fireball = 2
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_goomba = 3
    {0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0}, //movingobject_bulletbill = 4
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1}, //movingobject_hammer = 5
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_poisonpowerup = 6
    {0,0,0,1,1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0}, //movingobject_shell = 7
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0}, //movingobject_throwblock = 8
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0}, //movingobject_egg = 9
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_star = 10
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0}, //movingobject_flag = 11
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_cheepcheep = 12
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_koopa = 13
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1}, //movingobject_boomerang = 14
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_carried = 15
    {0,0,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1}, //movingobject_iceblast = 16
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_bomb = 17
    {0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0}, //movingobject_podobo = 18
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_treasurechest = 19
    {0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,0}, //movingobject_attackzone = 20
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_pirhanaplant = 21
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1}, //movingobject_explosion = 22
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_buzzybeetle = 23
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_spiny = 24
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_phantokey = 25
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_flagbase = 26
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_yoshi = 27
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_coin = 28
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_collectioncard = 29
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //movingobject_sledgebrother = 30
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,1}, //movingobject_sledgehammer = 31
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,1}, //movingobject_superfireball = 32
    {0,0,0,1,1,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,1}, //movingobject_throwbox = 33
};

short iPlatformPathDotSize[3] = {12, 6, 4};
short iPlatformPathDotOffset[3] = {0, 12, 18};

void DrawPlatform(short pathtype, TilesetTile ** tiles, short startX, short startY, short endX, short endY, float angle, float radiusX, float radiusY, short iSize, short iPlatformWidth, short iPlatformHeight, bool fDrawPlatform, bool fDrawShadow)
{
    short iStartX = startX >> iSize;
    short iStartY = startY >> iSize;
    short iEndX = endX >> iSize;
    short iEndY = endY >> iSize;

    float fRadiusX = radiusX / (float)(1 << iSize);
    float fRadiusY = radiusY / (float)(1 << iSize);

    short iSizeShift = 5 - iSize;
    int iTileSize = 1 << iSizeShift;

    if (fDrawPlatform) {
        for (short iPlatformX = 0; iPlatformX < iPlatformWidth; iPlatformX++) {
            for (short iPlatformY = 0; iPlatformY < iPlatformHeight; iPlatformY++) {
                TilesetTile * tile = &tiles[iPlatformX][iPlatformY];

                int iDstX = 0;
                int iDstY = 0;

                if (pathtype == 2) {
                    iDstX = iStartX + (iPlatformX << iSizeShift) + (short)(fRadiusX * cos(angle)) - (iPlatformWidth << (iSizeShift - 1));
                    iDstY = iStartY + (iPlatformY << iSizeShift) + (short)(fRadiusY * sin(angle)) - (iPlatformHeight << (iSizeShift - 1));
                } else {
                    iDstX = iStartX + (iPlatformX << iSizeShift) - (iPlatformWidth << (iSizeShift - 1));
                    iDstY = iStartY + (iPlatformY << iSizeShift) - (iPlatformHeight << (iSizeShift - 1));
                }

                SDL_Rect bltrect = {iDstX, iDstY, iTileSize, iTileSize};
                if (tile->iID >= 0) {
                    SDL_BlitSurface(g_tilesetmanager->GetTileset(tile->iID)->GetSurface(iSize), &g_tilesetmanager->rRects[iSize][tile->iCol][tile->iRow], blitdest, &bltrect);
                } else if (tile->iID == TILESETANIMATED) {
                    SDL_BlitSurface(rm->spr_tileanimation[iSize].getSurface(), &g_tilesetmanager->rRects[iSize][tile->iCol << 2][tile->iRow], blitdest, &bltrect);
                } else if (tile->iID == TILESETUNKNOWN) {
                    //Draw unknown tile
                    SDL_BlitSurface(rm->spr_unknowntile[iSize].getSurface(), &g_tilesetmanager->rRects[iSize][0][0], blitdest, &bltrect);
                }

                bool fNeedWrap = false;
                if (iDstX + iTileSize >= smw->GetScreenWidth(iSize)) {
                    iDstX -= smw->GetScreenWidth(iSize);
                    fNeedWrap = true;
                } else if (iDstX < 0) {
                    iDstX += smw->GetScreenWidth(iSize);
                    fNeedWrap = true;
                }

                if (fNeedWrap) {
                    bltrect.x = iDstX;
                    bltrect.y = iDstY;
                    bltrect.w = iTileSize;
                    bltrect.h = iTileSize;

                    if (tile->iID >= 0)
                        SDL_BlitSurface(g_tilesetmanager->GetTileset(tile->iID)->GetSurface(iSize), &g_tilesetmanager->rRects[iSize][tile->iCol][tile->iRow], blitdest, &bltrect);
                    else if (tile->iID == TILESETANIMATED)
                        SDL_BlitSurface(rm->spr_tileanimation[iSize].getSurface(), &g_tilesetmanager->rRects[iSize][tile->iCol << 2][tile->iRow], blitdest, &bltrect);
                    else if (tile->iID == TILESETUNKNOWN)
                        SDL_BlitSurface(rm->spr_unknowntile[iSize].getSurface(), &g_tilesetmanager->rRects[iSize][0][0], blitdest, &bltrect);
                }
            }
        }
    }

    SDL_Rect rPathSrc = {iPlatformPathDotOffset[iSize], 0, iPlatformPathDotSize[iSize], iPlatformPathDotSize[iSize]}, rPathDst;

    if (pathtype == 0) { //line segment
        if (fDrawShadow) {
            for (short iCol = 0; iCol < iPlatformWidth; iCol++) {
                for (short iRow = 0; iRow < iPlatformHeight; iRow++) {
                    if (tiles[iCol][iRow].iID != -2)
                        rm->spr_platformstarttile.draw(iStartX - (iPlatformWidth << (iSizeShift - 1)) + (iCol << iSizeShift), iStartY - (iPlatformHeight << (iSizeShift - 1)) + (iRow << iSizeShift), 0, 0, iTileSize, iTileSize);
                }
            }

            for (short iCol = 0; iCol < iPlatformWidth; iCol++) {
                for (short iRow = 0; iRow < iPlatformHeight; iRow++) {
                    if (tiles[iCol][iRow].iID != -2)
                        rm->spr_platformendtile.draw(iEndX - (iPlatformWidth << (iSizeShift - 1)) + (iCol << iSizeShift), iEndY - (iPlatformHeight << (iSizeShift - 1)) + (iRow << iSizeShift), 0, 0, iTileSize, iTileSize);
                }
            }
        }

        //Draw connecting dots
        float dDiffX = (float)(iEndX - iStartX);
        float dDiffY = (float)(iEndY - iStartY);

        short iDistance = (short)sqrt(dDiffX * dDiffX + dDiffY * dDiffY);

        short iNumSpots = (iDistance >> iSizeShift);
        float dIncrementX = dDiffX / (float)iNumSpots;
        float dIncrementY = dDiffY / (float)iNumSpots;

        float dX = (float)(iStartX) - (float)(iPlatformPathDotSize[iSize] >> 1);
        float dY = (float)(iStartY) - (float)(iPlatformPathDotSize[iSize] >> 1);

        for (short iSpot = 0; iSpot < iNumSpots + 1; iSpot++) {
            gfx_setrect(&rPathDst, (short)dX, (short)dY, iPlatformPathDotSize[iSize], iPlatformPathDotSize[iSize]);
            SDL_BlitSurface(rm->spr_platformpath.getSurface(), &rPathSrc, blitdest, &rPathDst);

            dX += dIncrementX;
            dY += dIncrementY;
        }
    } else if (pathtype == 1) { //continuous straight path
        if (fDrawShadow) {
            for (short iCol = 0; iCol < iPlatformWidth; iCol++) {
                for (short iRow = 0; iRow < iPlatformHeight; iRow++) {
                    if (tiles[iCol][iRow].iID != -2)
                        rm->spr_platformstarttile.draw(iStartX - (iPlatformWidth << (iSizeShift - 1)) + (iCol << iSizeShift), iStartY - (iPlatformHeight << (iSizeShift - 1)) + (iRow << iSizeShift), 0, 0, iTileSize, iTileSize);
                }
            }
        }

        float dIncrementX = (float)iTileSize * cos(angle);
        float dIncrementY = (float)iTileSize * sin(angle);

        float dX = (float)(iStartX) - (float)(iPlatformPathDotSize[iSize] >> 1);
        float dY = (float)(iStartY) - (float)(iPlatformPathDotSize[iSize] >> 1);

        for (short iSpot = 0; iSpot < 50; iSpot++) {
            gfx_setrect(&rPathDst, (short)dX, (short)dY, iPlatformPathDotSize[iSize], iPlatformPathDotSize[iSize]);
            SDL_BlitSurface(rm->spr_platformpath.getSurface(), &rPathSrc, blitdest, &rPathDst);

            short iWrapX = (short)dX;
            short iWrapY = (short)dY;
            bool fNeedWrap = false;
            if (dX + iPlatformPathDotSize[iSize] >= smw->GetScreenWidth(iSize)) {
                iWrapX = (short)(dX - smw->GetScreenWidth(iSize));
                fNeedWrap = true;
            } else if (dX < 0.0f) {
                iWrapX = (short)(dX + smw->GetScreenWidth(iSize));
                fNeedWrap = true;
            }

            if (dY + iPlatformPathDotSize[iSize] >= smw->GetScreenHeight(iSize)) {
                iWrapY = (short)(dY - smw->GetScreenHeight(iSize));
                fNeedWrap = true;
            } else if (dY < 0.0f) {
                iWrapY = (short)(dY + smw->GetScreenHeight(iSize));
                fNeedWrap = true;
            }

            if (fNeedWrap) {
                gfx_setrect(&rPathDst, iWrapX, iWrapY, iPlatformPathDotSize[iSize], iPlatformPathDotSize[iSize]);
                SDL_BlitSurface(rm->spr_platformpath.getSurface(), &rPathSrc, blitdest, &rPathDst);
            }

            dX += dIncrementX;
            dY += dIncrementY;
        }
    } else if (pathtype == 2) { //ellipse
        //Calculate the starting position
        if (fDrawShadow) {
            short iEllipseStartX = (short)(fRadiusX * cos(angle)) - (iPlatformWidth << (iSizeShift - 1)) + iStartX;
            short iEllipseStartY = (short)(fRadiusY * sin(angle)) - (iPlatformHeight << (iSizeShift - 1)) + iStartY;

            for (short iCol = 0; iCol < iPlatformWidth; iCol++) {
                for (short iRow = 0; iRow < iPlatformHeight; iRow++) {
                    if (tiles[iCol][iRow].iID != -2)
                        rm->spr_platformstarttile.draw(iEllipseStartX + (iCol << iSizeShift), iEllipseStartY + (iRow << iSizeShift), 0, 0, iTileSize, iTileSize);
                }
            }
        }

        float fAngle = angle;
        for (short iSpot = 0; iSpot < 32; iSpot++) {
            short iX = (short)(fRadiusX * cos(fAngle)) - (iPlatformPathDotSize[iSize] >> 1) + iStartX;
            short iY = (short)(fRadiusY * sin(fAngle)) - (iPlatformPathDotSize[iSize] >> 1) + iStartY;

            gfx_setrect(&rPathDst, iX, iY, iPlatformPathDotSize[iSize], iPlatformPathDotSize[iSize]);
            SDL_BlitSurface(rm->spr_platformpath.getSurface(), &rPathSrc, blitdest, &rPathDst);

            if (iX + iPlatformPathDotSize[iSize] >= smw->GetScreenWidth(iSize)) {
                gfx_setrect(&rPathDst, iX - smw->GetScreenWidth(iSize), iY, iPlatformPathDotSize[iSize], iPlatformPathDotSize[iSize]);
                SDL_BlitSurface(rm->spr_platformpath.getSurface(), &rPathSrc, blitdest, &rPathDst);
            } else if (iX < 0) {
                gfx_setrect(&rPathDst, iX + smw->GetScreenWidth(iSize), iY, iPlatformPathDotSize[iSize], iPlatformPathDotSize[iSize]);
                SDL_BlitSurface(rm->spr_platformpath.getSurface(), &rPathSrc, blitdest, &rPathDst);
            }

            fAngle += TWO_PI / 32.0f;
        }
    }
}

//[Direction][Frame]
SDL_Rect g_rFlameRects[4][4] = { { {0, 0, 96, 32}, {0, 32, 96, 32}, {0, 64, 96, 32}, {0, 96, 96, 32} },
    { {96, 0, 96, 32}, {96, 32, 96, 32}, {96, 64, 96, 32}, {96, 96, 96, 32} },
    { {0, 128, 32, 96}, {32, 128, 32, 96}, {64, 128, 32, 96}, {96, 128, 32, 96} },
    { {128, 128, 32, 96}, {160, 128, 32, 96}, {192, 128, 32, 96}, {224, 128, 32, 96} }
};

//[Type][Direction][Frame]
SDL_Rect g_rPirhanaRects[4][4][4] = { { { {0, 0, 32, 48}, {32, 0, 32, 48}, {64, 0, 32, 48}, {96, 0, 32, 48} },
        { {128, 0, 32, 48}, {160, 0, 32, 48}, {192, 0, 32, 48}, {224, 0, 32, 48} },
        { {304, 0, 48, 32}, {304, 32, 48, 32}, {304, 64, 48, 32}, {304, 96, 48, 32} },
        { {304, 128, 48, 32}, {304, 160, 48, 32}, {304, 192, 48, 32}, {304, 224, 48, 32} }
    },

    {   { {0, 48, 32, 48}, {32, 48, 32, 48}, {64, 48, 32, 48}, {96, 48, 32, 48} },
        { {128, 48, 32, 48}, {160, 48, 32, 48}, {192, 48, 32, 48}, {224, 48, 32, 48} },
        { {256, 0, 48, 32}, {256, 32, 48, 32}, {256, 64, 48, 32}, {256, 96, 48, 32} },
        { {256, 128, 48, 32}, {256, 160, 48, 32}, {256, 192, 48, 32}, {256, 224, 48, 32} }
    },

    {   { {0, 96, 32, 64}, {32, 96, 32, 64}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {64, 96, 32, 64}, {96, 96, 32, 64}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {192, 128, 64, 32}, {192, 160, 64, 32}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {192, 192, 64, 32}, {192, 224, 64, 32}, {0, 0, 0, 0}, {0, 0, 0, 0} }
    },

    {   { {0, 160, 32, 48}, {32, 160, 32, 48}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {64, 160, 32, 48}, {96, 160, 32, 48}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {144, 128, 48, 32}, {144, 160, 48, 32}, {0, 0, 0, 0}, {0, 0, 0, 0} },
        { {144, 192, 48, 32}, {144, 224, 48, 32}, {0, 0, 0, 0}, {0, 0, 0, 0} }
    }
};


short iFireballHazardSize[3] = {18, 9, 5};

short iStandardOffset[3] = {0, 32, 48};
float dBulletBillFrequency[3] = {10.0f, 5.0f, 2.5f};

short iPirhanaPlantOffsetY[4][3] = {{0, 0, 0}, {48, 24, 12}, {96, 48, 24}, {160, 80, 40}};


void DrawMapHazard(MapHazard * hazard, short iSize, bool fDrawCenter)
{
    short iSizeShift = 5 - iSize;
    short iTileSize = 1 << iSizeShift;

    SDL_Rect rDotSrc = {iPlatformPathDotOffset[iSize] + 22, 0, iPlatformPathDotSize[iSize], iPlatformPathDotSize[iSize]}, rDotDst;
    SDL_Rect rPathSrc = {iStandardOffset[iSize], 12, iTileSize, iTileSize}, rPathDst;

    gfx_setrect(&rPathDst, hazard->ix << (iSizeShift - 1), hazard->iy << (iSizeShift - 1), iTileSize, iTileSize);

    if (fDrawCenter) {
        if (hazard->itype <= 1) {
            SDL_BlitSurface(rm->spr_platformpath.getSurface(), &rPathSrc, blitdest, &rPathDst);
        }
    }

    if (hazard->itype == 0) { //fireball string
        short iNumDots = 16;
        float dRadius = (float)((hazard->iparam[0] - 1) * 24) / (float)(1 << iSize) + (iPlatformPathDotSize[iSize] >> 1);
        float dAngle = hazard->dparam[1];
        for (short iDot = 0; iDot < iNumDots; iDot++) {
            rDotDst.x = (short)(dRadius * cos(dAngle)) + rPathDst.x + (iTileSize >> 1) - (iPlatformPathDotSize[iSize] >> 1);
            rDotDst.y = (short)(dRadius * sin(dAngle)) + rPathDst.y + (iTileSize >> 1) - (iPlatformPathDotSize[iSize] >> 1);
            rDotDst.h = rDotDst.w = iPlatformPathDotSize[iSize];

            rm->spr_platformpath.draw(rDotDst.x, rDotDst.y, rDotSrc.x, rDotSrc.y, rDotDst.w, rDotDst.h);
            dAngle += TWO_PI / iNumDots;
        }

        //Draw the fireball string
        for (short iFireball = 0; iFireball < hazard->iparam[0]; iFireball++) {
            short x = (hazard->ix << (iSizeShift - 1)) + (short)((float)(iFireball * (24 >> iSize)) * cos(hazard->dparam[1])) + (iTileSize >> 1) - (iFireballHazardSize[iSize] >> 1);
            short y = (hazard->iy << (iSizeShift - 1)) + (short)((float)(iFireball * (24 >> iSize)) * sin(hazard->dparam[1])) + (iTileSize >> 1) - (iFireballHazardSize[iSize] >> 1);

            rm->spr_hazard_fireball[iSize].draw(x, y, 0, 0, iFireballHazardSize[iSize], iFireballHazardSize[iSize]);
        }
    } else if (hazard->itype == 1) { //rotodisc
        short iNumDots = 16;
        float dRadius = (hazard->dparam[2] + (iTileSize >> 1) - (iPlatformPathDotSize[iSize] >> 1)) / (float)(1 << iSize);
        float dAngle = hazard->dparam[1];
        for (short iDot = 0; iDot < iNumDots; iDot++) {
            rDotDst.x = (short)(dRadius * cos(dAngle)) + rPathDst.x + (iTileSize >> 1) - (iPlatformPathDotSize[iSize] >> 1);
            rDotDst.y = (short)(dRadius * sin(dAngle)) + rPathDst.y + (iTileSize >> 1) - (iPlatformPathDotSize[iSize] >> 1);
            rDotDst.h = rDotDst.w = iPlatformPathDotSize[iSize];

            rm->spr_platformpath.draw(rDotDst.x, rDotDst.y, rDotSrc.x, rDotSrc.y, rDotDst.w, rDotDst.h);
            dAngle += TWO_PI / iNumDots;
        }

        //Draw the rotodiscs
        float dSector = TWO_PI / hazard->iparam[0];
        dAngle = hazard->dparam[1];
        dRadius = hazard->dparam[2] / (float)(1 << iSize);
        for (short iRotodisc = 0; iRotodisc < hazard->iparam[0]; iRotodisc++) {
            short x = rPathDst.x + (short)(dRadius * cos(dAngle));
            short y = rPathDst.y + (short)(dRadius * sin(dAngle));

            rm->spr_hazard_rotodisc[iSize].draw(x, y, 0, 0, iTileSize, iTileSize);

            dAngle += dSector;
        }
    } else if (hazard->itype == 2) { //bullet bill
        rm->spr_hazard_bulletbill[iSize].draw(rPathDst.x, rPathDst.y, 0, hazard->dparam[0] < 0.0f ? 0 : iTileSize, iTileSize, iTileSize);

        short iBulletPathX = rPathDst.x - iPlatformPathDotSize[iSize];
        if (hazard->dparam[0] > 0.0f)
            iBulletPathX = rPathDst.x + iTileSize;

        short iBulletPathSpacing = (short)(hazard->dparam[0] * dBulletBillFrequency[iSize]);
        while (iBulletPathX >= 0 && iBulletPathX < smw->GetScreenWidth(iSize)) {
            gfx_setrect(&rDotDst, iBulletPathX, rPathDst.y + ((iTileSize - iPlatformPathDotSize[iSize]) >> 1), iPlatformPathDotSize[iSize], iPlatformPathDotSize[iSize]);
            SDL_BlitSurface(rm->spr_platformpath.getSurface(), &rDotSrc, blitdest, &rDotDst);

            iBulletPathX += hazard->iparam[0] < 0.0f ? -iBulletPathSpacing : iBulletPathSpacing;
        }
    } else if (hazard->itype == 3) { //flame cannon
        SDL_Rect * rect = &g_rFlameRects[hazard->iparam[1]][2];

        short iOffsetX = 0;
        short iOffsetY = 0;

        if (hazard->iparam[1] == 1) {
            iOffsetX = -(iTileSize << 1);
        } else if (hazard->iparam[1] == 2) {
            iOffsetY = -(iTileSize << 1);
        }

        rm->spr_hazard_flame[iSize].draw(rPathDst.x + iOffsetX, rPathDst.y + iOffsetY, rect->x >> iSize, rect->y >> iSize, rect->w >> iSize, rect->h >> iSize);
    } else if (hazard->itype >= 4 && hazard->itype <= 7) { //pirhana plants
        SDL_Rect * rect = &g_rPirhanaRects[hazard->itype - 4][hazard->iparam[1]][0];
        short iOffsetX = 0;
        short iOffsetY = 0;

        if (hazard->iparam[1] == 0) {
            if (hazard->itype == 6)
                iOffsetY = -iTileSize;
            else
                iOffsetY = -(iTileSize >> 1);
        } else if (hazard->iparam[1] == 2) {
            if (hazard->itype == 6)
                iOffsetX = -iTileSize;
            else
                iOffsetX = -(iTileSize >> 1);
        }

        rm->spr_hazard_pirhanaplant[iSize].draw(rPathDst.x + iOffsetX, rPathDst.y + iOffsetY, rect->x >> iSize, rect->y >> iSize, rect->w >> iSize, rect->h >> iSize);
    }
}

SDL_Rect iCountDownNumbers[4][4][2] =
    {   {{{0, 0, 64, 64},{288, 208, 64, 64}},
        {{0, 64, 48, 48},{296, 216, 48, 48}},
        {{192, 64, 32, 32},{304, 224, 32, 32}},
        {{0, 112, 16, 16},{312, 232, 16, 16}}
    },

    {   {{64, 0, 64, 64},{288, 208, 64, 64}},
        {{48, 64, 48, 48},{296, 216, 48, 48}},
        {{224, 64, 32, 32},{304, 224, 32, 32}},
        {{16, 112, 16, 16},{312, 232, 16, 16}}
    },

    {   {{128, 0, 64, 64},{288, 208, 64, 64}},
        {{96, 64, 48, 48},{296, 216, 48, 48}},
        {{192, 96, 32, 32},{304, 224, 32, 32}},
        {{32, 112, 16, 16},{312, 232, 16, 16}}
    },

    {   {{192, 0, 64, 64},{288, 208, 64, 64}},
        {{144, 64, 48, 48},{296, 216, 48, 48}},
        {{224, 96, 32, 32},{304, 224, 32, 32}},
        {{48, 112, 16, 16},{312, 232, 16, 16}}
    }
};

short iCountDownTimes[28] = {3, 3, 3, 15, 3, 3, 3, 3, 3, 3, 15, 3, 3, 3, 3, 3, 3, 15, 3, 3, 3, 3, 3, 3, 45, 3, 3, 3};
short iCountDownRectSize[28] = {3, 2, 1, 0, 1, 2, 3, 3, 2, 1, 0, 1, 2, 3, 3, 2, 1, 0, 1, 2, 3, 3, 2, 1, 0, 1, 2, 3};
short iCountDownRectGroup[28] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3};
short iCountDownAnnounce[28] = {-1, -1, -1, 12, -1, -1, -1, -1, -1, -1, 13, -1, -1, -1, -1, -1, -1, 14, -1, -1, -1, -1, -1, -1, 15, -1, -1, -1};

TileType GetIncrementedTileType(TileType type)
{
    switch (type) {
    case tile_nonsolid:
        return tile_solid;
    case tile_solid:
        return tile_solid_on_top;
    case tile_solid_on_top:
        return tile_ice;
    case tile_ice:
        return tile_death;
    case tile_death:
        return tile_death_on_top;
    case tile_death_on_top:
        return tile_death_on_bottom;
    case tile_death_on_bottom:
        return tile_death_on_left;
    case tile_death_on_left:
        return tile_death_on_right;
    case tile_death_on_right:
        return tile_ice_on_top;
    case tile_ice_on_top:
        return tile_ice_death_on_bottom;
    case tile_ice_death_on_bottom:
        return tile_ice_death_on_left;
    case tile_ice_death_on_left:
        return tile_ice_death_on_right;
    case tile_ice_death_on_right:
        return tile_super_death;
    case tile_super_death:
        return tile_super_death_top;
    case tile_super_death_top:
        return tile_super_death_bottom;
    case tile_super_death_bottom:
        return tile_super_death_left;
    case tile_super_death_left:
        return tile_super_death_right;
    case tile_super_death_right:
        return tile_player_death;
    case tile_player_death:
        return tile_nonsolid;
    }

    return tile_nonsolid;
}
