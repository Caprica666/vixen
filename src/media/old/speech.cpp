#include "win32/psmwin.h"
#include "psmedia.h"

#ifdef PSM_SPEECH

PS_IMPLEMENT_CLASSID(PSSpeaker, PSEngine, PSM_Speaker);
PS_IMPLEMENT_CLASSID(PSSpeechTranslator, PSEngine, PSM_SpeechTranslator);


class SpeechThread : public VThread
{
public:
	SpeechThread(PSSpeechTranslator* s) : VThread(0) { DoExit = false; m_Speech = s; }
	void Start(PSThreadFunc* func = NULL)
		{ VThread::Run(func ? func : ThreadFunc); }

	bool	DoExit;

protected:
	static VThreadFunc ThreadFunc;

	PSSpeechTranslator*	m_Speech;
};

PSSpeechTranslator::PSSpeechTranslator() : PSEngine()
{
	m_SpeechThread = NULL;
	m_RecoCtxt = NULL;
	m_Grammar = NULL;
}

PSSpeechTranslator::~PSSpeechTranslator()
{
	m_SpeechThread->DoExit = true;
	m_SpeechThread = NULL;
	m_RecoCtxt = NULL;
	m_Grammar = NULL;
}

bool PSSpeechTranslator::Listen(PSWindow win)
{
	if (!Init(win))
		return false;
	if (win == NULL)
	   {
		SpeechThread* thread = new SpeechThread(this);
		if (thread == NULL)
		   { PSM_ERROR(("SpeechTranslator_Listen: ERROR - cannot start speech\n"), false); }
		thread->Start();
	   }
	return true;
}

void SpeechThread::ThreadFunc(void* arg)
{
	USES_CONVERSION;

	SpeechThread*			spthread = (SpeechThread*) arg;
	VTlsData*				tls = VTlsData::Get();

	while (!spthread->DoExit)
	   {
		PSSpeechTranslator* speech = spthread->m_Speech;
		if (SUCCEEDED(speech->WaitForResult()))
			speech->ProcessResults();
	   }
	delete tls;
	delete spthread;
}

bool PSSpeechTranslator::Init(PSWindow win)
{
	HRESULT hr;

    if (FAILED(hr = ::CoInitialize(NULL)))
		return false;
	hr = m_RecoCtxt.CoCreateInstance(CLSID_SpSharedRecoContext);
	if (!SUCCEEDED(hr))
		return false;
	if (win)
		hr = m_RecoCtxt->SetNotifyWindowMessage((HWND) win, WM_SPEECH, (DWORD) this, 0);
	else
		hr = m_RecoCtxt->SetNotifyWin32Event();

	if (SUCCEEDED(hr) &&
		SUCCEEDED(hr = m_RecoCtxt->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION))) &&
		SUCCEEDED(hr = m_RecoCtxt->SetAudioOptions(SPAO_RETAIN_AUDIO, NULL, NULL)) &&
		SUCCEEDED(hr = m_RecoCtxt->CreateGrammar(0, &m_Grammar)) &&
		SUCCEEDED(hr = m_Grammar->LoadDictation(NULL, SPLO_STATIC)) &&
		SUCCEEDED(hr = m_Grammar->SetDictationState(SPRS_ACTIVE)))
		return true;
	PSM_ERROR(("SpeechTranslator:: ERROR cannot initialize speech"), false);
}

PSEvent* PSSpeechTranslator::LogEvent()
{ 
	CSpDynamicString dstrText;

	if (SUCCEEDED(m_Result->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, 
                                    TRUE, &dstrText, NULL)))
	   {
		PSSpeechEvent* spe = new PSSpeechEvent;
		char			speechbuf[PSM_MaxString];
		int				n = wcstombs(speechbuf, dstrText, PSM_MaxString); 

		spe->Sender = this;
		spe->Speech = speechbuf;
		spe->Log();	
		return spe;
	   }
	return NULL;
}		

HRESULT PSSpeechTranslator::WaitForResult()
{
    HRESULT hr = S_OK;
	CSpEvent event;

    while (SUCCEEDED(hr) &&
		   SUCCEEDED(hr = event.GetFrom(m_RecoCtxt)) &&
           (hr == S_FALSE))
        hr = m_RecoCtxt->WaitForNotifyEvent(INFINITE);
    return hr;
}

void PSSpeechTranslator::ProcessResults()
{
    USES_CONVERSION;
    CSpEvent event;

    while (event.GetFrom(m_RecoCtxt) == S_OK)
       {
        switch (event.eEventId)
           {
            case SPEI_SOUND_START:
            case SPEI_SOUND_END:
			break;

            case SPEI_RECOGNITION:
			m_Result = event.RecoResult();
			if (m_Result)
			   {
				m_Grammar->SetDictationState( SPRS_INACTIVE );
				LogEvent();
				m_Result.Release();
				m_Grammar->SetDictationState( SPRS_ACTIVE );
			   }
			break;
           }
       }
} 
#endif