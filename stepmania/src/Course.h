#ifndef COURSE_H
#define COURSE_H
/*
-----------------------------------------------------------------------------
 Class: Course

 Desc: A queue of songs and notes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GameConstantsAndTypes.h"

struct PlayerOptions;
struct SongOptions;
class Song;
struct Notes;

class Course
{
	int			m_iStages;

	struct course_entry {
		CString description;
		CString modifiers; // set player and song options from these
		Song *song;
	};
	vector<course_entry> entries;
	vector<int> order;

public:
	Course();

	CString		m_sName;
	CString		m_sBannerPath;
	CString		m_sCDTitlePath;

	bool		m_bRepeat;	// repeat after last song?
	bool		m_bRandomize;	// play the songs in a random order
	int			m_iLives;	// -1 means use bar life meter
	int			m_iExtra;	// extra stage number...

	Notes *GetNotesForStage( int iStage );
	Song *GetSong( int iStage ) const;
	CString GetDescription( int iStage ) const;
	CString GetModifiers( int iStage ) const;
	void GetPlayerOptions( PlayerOptions* pPO_out );
	void GetSongOptions( SongOptions* pSO_out);
	int GetNumStages() const;

	void LoadFromCRSFile( CString sPath, CArray<Song*,Song*> &apSongs );
	void CreateEndlessCourseFromGroupAndDifficulty( CString sGroupName, Difficulty dc, CArray<Song*,Song*> &apSongsInGroup );
	void AddStage( Song* pSong, CString sDescription, CString sModifiers );

	void GetSongAndNotesForCurrentStyle( CArray<Song*,Song*>& apSongsOut, CArray<Notes*,Notes*>& apNotesOut, CStringArray& asModifiersOut, bool bShuffled );
	RageColor	GetColor();

private:
	void Shuffle();
};


void SortCoursePointerArrayByDifficulty( CArray<Course*,Course*> &apCourses );


#endif
