#ifndef Profile_H
#define Profile_H

#include "Etterna/Models/Misc/DateTime.h"
#include "Etterna/Models/Misc/GameConstantsAndTypes.h"
#include "Etterna/Models/Misc/Grade.h"
#include "Etterna/Models/Misc/HighScore.h"
#include "Etterna/Models/Lua/LuaReference.h"
#include "Etterna/Models/Songs/SongUtil.h"			 // for SongID
#include "Etterna/Models/StepsAndStyles/StepsUtil.h" // for StepsID
#include "Etterna/Models/StepsAndStyles/StyleUtil.h" // for StyleID
#include "Etterna/Models/Misc/XMLProfile.h"
#include "Etterna/Models/Misc/DBProfile.h"
#include "arch/LoadingWindow/LoadingWindow.h"
#include <map>
#include <set>

#include <unordered_map>
using std::string;

class XNode;
struct lua_State;
struct Playlist;

// Current file versions
extern const string ETT_XML;

/**
 * @brief The filename where one can edit their personal profile data.
 *
 * Editable data is an INI because the default INI file association on Windows
 * systems will open the ini file in an editor.  The default association for
 * XML will open in IE.  Users have a much better chance of discovering how to
 * edit this data if they don't have to fight against the file associations. */
extern const std::string EDITABLE_INI;

/**
 * @brief The filename containing the signature for STATS_XML's signature.
 *
 *
 * The "don't share" file is something that the user should always keep private.
 * They can safely share STATS_XML with STATS_XML's signature so that others
 * can authenticate the STATS_XML data.  However, others can't copy that data
 * to their own profile for use in the game unless they also have the "don't
 * share" file.  DontShare contains a piece of information that we can
 * construct using STATS_XML but the user can't construct using STATS_XML. */
extern const string DONT_SHARE_SIG;

extern const std::string PUBLIC_KEY_FILE;
extern const std::string SCREENSHOTS_SUBDIR;
extern const std::string REPLAY_SUBDIR;
extern const std::string EDIT_STEPS_SUBDIR;
extern const std::string LASTGOOD_SUBDIR;
// extern const std::string RIVAL_SUBDIR;

/** @brief The max number of characters that can be used in a profile. */
const unsigned int PROFILE_MAX_DISPLAY_NAME_LENGTH = 64;

class Style;

class Song;
class Steps;
struct Game;

// future goalman stuff - Mina
class ScoreGoal
{
  public:
	float rate = 1.f;
	float percent = .93f;
	int priority = 1;
	bool achieved = false;
	DateTime timeassigned;
	DateTime timeachieved;
	std::string comment = "";
	std::string chartkey = "";

	// which specific score was this goal achieved by, reminder to consider
	// what happens when individual score deletion is possibly added -mina
	std::string scorekey = "";

	XNode* CreateNode() const;
	void LoadFromNode(const XNode* pNode);

	HighScore* GetPBUpTo() const;

	// If the scoregoal has already been completed prior to being assigned, flag
	// it as a vacuous goal
	void CheckVacuity();

	void UploadIfNotVacuous();

	// Vacuous goals will remain in memory for the session but not be written
	// during save -mina
	bool vacuous = false;

	void PushSelf(lua_State* L);
};

struct GoalsForChart
{
  public:
	void Add(ScoreGoal& sg) { goals.emplace_back(sg); }
	vector<ScoreGoal>& Get() { return goals; }
	vector<ScoreGoal> goals;

	XNode* CreateNode() const;
	void LoadFromNode(const XNode* pNode);
};

/**
 * @brief Player data that persists between sessions.
 *
 * This can be stored on a local disk or on a memory card. */
class Profile
{
  public:
	/**
	 * @brief Set up the Profile with default values.
	 *
	 * Note: there are probably a lot of variables. */
	// When adding new score related data, add logic for handling it to
	// MergeScoresFromOtherProfile. -Kyz
	// When adding any new fields, add them to SwapExceptPriority.  Anything not
	// added to SwapExceptPriority won't be swapped correctly when the user
	// changes the list priority of a profile. -Kyz
	Profile()
	  :

	  m_sDisplayName("")
	  , m_sLastUsedHighScoreName("")
	  , m_sGuid(MakeGuid())
	  , m_sDefaultModifiers()
	  , m_lastSong()
	  , m_sLastPlayedMachineGuid("")
	  , m_LastPlayedDate()
	  , m_iNumSongsPlayedByStyle()
	  , m_UserTable()
	  , m_vScreenshots()
	  , profiledir("")
	{
		m_lastSong.Unset();
		m_fPlayerRating = 0.f;
		FOREACH_ENUM(Skillset, ss)
		m_fPlayerSkillsets[ss] = 0.f;

		m_LastPlayedDate.Init();

		FOREACH_ENUM(PlayMode, i)
		m_iNumSongsPlayedByPlayMode[i] = 0;
		FOREACH_ENUM(Difficulty, i)
		m_iNumSongsPlayedByDifficulty[i] = 0;
		for (int& i : m_iNumSongsPlayedByMeter)
			i = 0;

		ZERO(m_iNumStagesPassedByPlayMode);
		ZERO(m_iNumStagesPassedByGrade);
		m_UserTable.Unset();
	}

	// smart accessors
	std::string GetDisplayNameOrHighScoreName() const;
	bool GetDefaultModifiers(const Game* pGameType,
							 std::string& sModifiersOut) const;
	void SetDefaultModifiers(const Game* pGameType, const std::string& sModifiers);

	void AddStepTotals(int iNumTapsAndHolds,
					   int iNumJumps,
					   int iNumHolds,
					   int iNumRolls,
					   int iNumMines,
					   int iNumHands,
					   int iNumLifts);

	// Profiles of the same type and priority are sorted by dir name.
	int m_ListPriority{ 0 };
	// Profile Playlists
	map<string, Playlist> allplaylists;

	// Editable data
	std::string m_sDisplayName;
	// Dont edit this. Should be unique (Is it?)
	std::string m_sProfileID;
	/**
	 * @brief The last used name for high scoring purposes.
	 *
	 * This really shouldn't be in "editable", but it's needed in the smaller
	 * editable file so that it can be ready quickly. */
	std::string m_sLastUsedHighScoreName;

	// General data
	static std::string MakeGuid();
	std::string* GetGuid() { return &m_sGuid; }
	std::string m_sGuid;
	map<std::string, std::string> m_sDefaultModifiers;
	SortOrder m_SortOrder{ SortOrder_Invalid };
	Difficulty m_LastDifficulty{ Difficulty_Invalid };
	StepsType m_LastStepsType{ StepsType_Invalid };
	SongID m_lastSong;
	int m_iCurrentCombo{ 0 };
	int m_iTotalSessions{ 0 };
	int m_iTotalSessionSeconds{ 0 };
	int m_iTotalGameplaySeconds{ 0 };
	int m_iTotalDancePoints{ 0 };
	int m_iNumExtraStagesPassed{ 0 };
	int m_iNumExtraStagesFailed{ 0 };
	int m_iNumToasties{ 0 };
	int m_iTotalTapsAndHolds{ 0 };
	int m_iTotalJumps{ 0 };
	int m_iTotalHolds{ 0 };
	int m_iTotalRolls{ 0 };
	int m_iTotalMines{ 0 };
	int m_iTotalHands{ 0 };
	int m_iTotalLifts{ 0 };
	float m_fPlayerRating;
	float m_fPlayerSkillsets[NUM_Skillset];
	/** @brief Is this a brand new profile? */
	bool m_bNewProfile{ false };

	// seriously why is this not a thing -mina
	std::string profiledir;
	bool IsEtternaProfile{ false };
	/**
	 * @brief Which machine did we play on last, based on the Guid?
	 *
	 * This is mutable because it's overwritten on save, but is usually
	 * const everywhere else. It was decided to keep const on the whole
	 * save chain and keep this mutable. -Chris */
	mutable std::string m_sLastPlayedMachineGuid;
	mutable DateTime m_LastPlayedDate;
	/* These stats count twice in the machine profile if two players are
	 * playing; that's the only approach that makes sense for ByDifficulty and
	 * ByMeter. */
	int m_iNumSongsPlayedByPlayMode[NUM_PlayMode];
	map<StyleID, int> m_iNumSongsPlayedByStyle;
	int m_iNumSongsPlayedByDifficulty[NUM_Difficulty];
	int m_iNumSongsPlayedByMeter[MAX_METER + 1];
	/**
	 * @brief Count the total number of songs played.
	 *
	 * This stat counts once per song, even if two players are active. */
	int m_iNumTotalSongsPlayed{ 0 };
	int m_iNumStagesPassedByPlayMode[NUM_PlayMode];
	int m_iNumStagesPassedByGrade[NUM_Grade];

	// if anymore of these are added they should be enum'd to reduce copy pasta
	// -mina and also should be sets
	void AddToFavorites(const string& ck) { FavoritedCharts.emplace(ck); }
	void AddToPermaMirror(const string& ck) { PermaMirrorCharts.emplace(ck); }
	void RemoveFromFavorites(const string& ck);
	void RemoveFromPermaMirror(const string& ck);
	set<string> FavoritedCharts;
	set<string> PermaMirrorCharts;

	// more future goalman stuff -mina
	void AddGoal(const string& ck);
	void RemoveGoal(const string& ck, DateTime assigned);
	unordered_map<string, GoalsForChart> goalmap;
	void FillGoalTable();
	vector<ScoreGoal*> goaltable;
	int sortmode = 1;	// 1=date 2=rate 3=name 4=priority 5=diff, init to name
						// because that's the default- mina
	int filtermode = 1; // 1=all, 2=completed, 3=uncompleted
	bool asc = false;

	bool HasGoal(const string& ck) { return goalmap.count(ck) == 1; }
	ScoreGoal& GetLowestGoalForRate(const string& ck, float rate);
	void SetAnyAchievedGoals(const string& ck,
							 float& rate,
							 const HighScore& pscore);

	/* store arbitrary data for the theme within a profile */
	LuaTable m_UserTable;

	// this actually does use scoreman atm
	Grade GetBestGrade(const Song* pSong, StepsType st) const;

	// Screenshot Data
	vector<Screenshot> m_vScreenshots;
	void AddScreenshot(const Screenshot& screenshot);
	int GetNextScreenshotIndex() { return m_vScreenshots.size(); }

	// Init'ing
	void InitAll()
	{
		InitEditableData();
		InitGeneralData();
		InitScreenshotData();
	}
	void InitEditableData();
	void InitGeneralData();
	void InitScreenshotData();
	void ClearStats();

	void swap(Profile& other);

	// Loading and saving
	void HandleStatsPrefixChange(std::string dir, bool require_signature);
	ProfileLoadResult LoadAllFromDir(const std::string& sDir,
									 bool bRequireSignature,
									 LoadingWindow* ld);
	ProfileLoadResult LoadStatsFromDir(std::string dir, bool require_signature);
	void LoadTypeFromDir(const std::string& dir);
	void LoadCustomFunction(const std::string& sDir);
	bool SaveAllToDir(const std::string& sDir, bool bSignData) const;

	ProfileLoadResult LoadEditableDataFromDir(const std::string& sDir);

	void SaveTypeToDir(const std::string& dir) const;
	void SaveEditableDataToDir(const std::string& sDir) const;

	void CalculateStatsFromScores(LoadingWindow* ld);
	void CalculateStatsFromScores();

	void SaveStatsWebPageToDir(const std::string& sDir) const;
	void SaveMachinePublicKeyToDir(const std::string& sDir) const;

	static void MoveBackupToDir(const std::string& sFromDir, const std::string& sToDir);
	static std::string MakeUniqueFileNameNoExtension(
	  const std::string& sDir,
	  const std::string& sFileNameBeginning);
	static std::string MakeFileNameNoExtension(const std::string& sFileNameBeginning,
										   int iIndex);

	// Lua
	void PushSelf(lua_State* L);

  private:
	XMLProfile XMLProf;
	DBProfile DBProf;
};

#endif
