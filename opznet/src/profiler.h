/* Copyright (C) Steve Rabin, 2000. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Steve Rabin, 2000"
 */
#pragma once

#define LEAN_AND_MEAN
#include <Windows.h>
#include <string>

#pragma comment( lib, "winmm.lib" )

typedef struct {
	bool bValid;                 //Whether this data is valid
	unsigned int iProfileInstances;      //# of times ProfileBegin called
	int iOpenProfiles;           //# of times ProfileBegin w/o ProfileEnd
	WCHAR szName[256];            //Name of sample
	double fStartTime;            //The current open profile start time
	double fAccumulator;          //All samples this frame added together
	double fChildrenSampleTime;   //Time taken by all children
	unsigned int iNumParents;            //Number of profile parents
} ProfileSample;

typedef struct {
	bool bValid;        //Whether the data is valid
	WCHAR szName[256];   //Name of the sample
	double fAve;         //Average time per frame (percentage)
	double fMin;         //Minimum time per frame (percentage)
	double fMax;         //Maximum time per frame (percentage)
} ProfileSampleHistory;

#define NUM_PROFILE_SAMPLES 50

class opzProfiler
{
public:
	static void S_Create();	//���α׷� �����Ҷ� ȣ���Ѵ�.
	static void S_Delete();
	
	static void S_FrameBegin(){_instance->g_startProfile = _instance->GetExactTime();}	//�� �������� �����ϴ� ��ġ�� ����.
	static void S_FrameEnd();					//�� �������� ������ ��ġ�� ����.

	static void S_SampleBegin(LPCWSTR name);	//�м��� ���ϴ� �������ۿ� ����.
	static void S_SampleEnd(LPCWSTR name);		//�� ������ ������ ����.(���ڴ� ProfileBegin�� ���ƾ���)

	static void S_SaveFile();					//������ profile.txt�� �����ϴ� �Լ�
	static LPCWSTR S_GetBuffer();				//������ LPCWSTR�� ������. ( �̰� ����ϸ� �ǽð����� �ۼ�Ƽ�� Ȯ�ΰ���)

private:
	static opzProfiler *_instance;

	double g_startProfile;
	double g_endProfile;

#ifdef _WIN32
	double g_StartTime;
#else
	struct timeb   g_StartTimeb;
#endif
	double g_CurrentTime;
	double g_TimeLastTick;

	ProfileSample g_samples[NUM_PROFILE_SAMPLES];
	ProfileSampleHistory g_history[NUM_PROFILE_SAMPLES];
	
	std::wstring ProfileBuffer;

	opzProfiler(void);
	~opzProfiler(void);

	void StoreProfileInHistory( LPWSTR name, double percent );
	void GetProfileFromHistory( LPWSTR name, double* ave, double* min, double* max );
	void InitTime( void );
	double GetElapsedTime( void );
	double GetExactTime( void );
	void MarkTimeThisTick( void );

	void ClearTextBuffer();
	void PutTextBuffer(LPWSTR c);

	void SampleBegin(LPCWSTR name);
	void SampleEnd(LPCWSTR name);
	void FrameEnd();

	void SaveFile();
};

inline void opzProfiler::S_SampleBegin(LPCWSTR name)
{
#ifdef _DEBUG
	_instance->SampleBegin(name);
#endif
}

inline void opzProfiler::S_SampleEnd(LPCWSTR name)
{
#ifdef _DEBUG
	_instance->SampleEnd(name);
#endif
}

inline void opzProfiler::S_FrameEnd()
{
#ifdef _DEBUG
	_instance->FrameEnd();
#endif
}

inline void opzProfiler::S_SaveFile()
{
#ifdef _DEBUG
	_instance->SaveFile();
#endif
}

inline LPCWSTR opzProfiler::S_GetBuffer()
{
	return _instance->ProfileBuffer.c_str();
}
