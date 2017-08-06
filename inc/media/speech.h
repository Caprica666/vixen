//
// If sapi.h is not found by the compiler, be sure the Microsoft Speech SDK
// include files are on your search path
// To get rid of references to the speech SDK in the scene manager,
// define PSM_NOSPEECH at compile time
//
#include <sapi.h>
#include <string.h>
#include <atlbase.h>
#include <sphelper.h>
#include <wchar.h>

#ifndef _PS_SPEECH
#define _PS_SPEECH

#define	WM_SPEECH	WM_APP

class PSSpeechEvent : public PSEvent
{
public:
	PSSpeechEvent() : PSEvent(EVENT_Speech, sizeof(VString)) { }
	PSSpeechEvent(const PSSpeechEvent& src) { *this = src; }

	PSEvent&	operator=(const PSEvent& src)
	{
		PSEvent::operator=(src);
		if (src.Code == EVENT_Speech)
			Speech = ((const PSSpeechEvent&) src).Speech;
		return *this;
	}

	VString	Speech;
};

class SpeechThread;

class PSSpeechTranslator : public PSEngine
{
public:
	PS_DECLARE_CLASS(PSSpeechTranslator);

	PSSpeechTranslator();
	~PSSpeechTranslator();

	bool			Listen(PSWindow = NULL);
	HRESULT			WaitForResult();
	virtual void	ProcessResults();

protected:
	bool			Init(PSWindow = NULL);
	virtual PSEvent*	LogEvent();

	CComPtr<ISpRecoContext> m_RecoCtxt;
	CComPtr<ISpRecoGrammar> m_Grammar;
	CComPtr<ISpRecoResult>	m_Result;
	SpeechThread*			m_SpeechThread;
};

class PSSpeaker : public PSEngine
{
public:
	PS_DECLARE_CLASS(PSSpeaker);

	PSSpeaker()		{ Init(); }
	~PSSpeaker()	{ m_Voice = NULL; }
	bool	Say(const char* saythis);

protected:
	bool Init();

	CComPtr<ISpVoice>		m_Voice;
};

inline bool PSSpeaker::Say(const char* saythis)
{
	if (m_Voice == NULL)
		return false;
	wchar_t buf[PSM_MaxString];
	const char* p = saythis;
	int n = mbstowcs(buf, p, PSM_MaxString); 
	m_Voice->Speak(buf, SPF_ASYNC, NULL);
	return true;
}

inline bool PSSpeaker::Init()
{
	HRESULT hr;

    if (FAILED(hr = ::CoInitialize(NULL)))
		return false;
	hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&m_Voice);
	if (SUCCEEDED(hr))
		return true;
	return false;
}

#endif