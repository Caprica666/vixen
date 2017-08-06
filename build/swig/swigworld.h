class SwigWorld : public World3D
{
	protected:
		Core::Semaphore	m_EventPending;	// signals events waiting
		Core::List		m_EventQueue;	// Vixen -> C# event queue
		Event*			m_LastEvent;	// last event added to queue
		Event*			m_NextEvent;	// next event gotten from queue
		bool			m_Stopped;		// true if event processing stopped

	public:
		SwigWorld() : World3D()
		{
			m_Stopped = false;
			m_EventQueue.Next = NULL;
			m_LastEvent = (Event*) &m_EventQueue;
		}
	
		~SwigWorld()	{ StopEvents(); }

		void StopEvents()
		{
			Lock();
			m_LastEvent = CleanEvents();
			if (m_LastEvent)
				delete m_LastEvent;
			m_EventQueue.Next = NULL;
			m_NextEvent = NULL;
			m_Stopped = true;
			Unlock();
			m_EventPending.Release();
		}

		Event* NextEvent()
		{
			if (m_Stopped)
				return NULL;
			m_EventPending.Wait();
			Lock();
			if (m_NextEvent == NULL)
				m_NextEvent = (Event*) m_EventQueue.Next;
			else
				m_NextEvent = (Event*) m_NextEvent->Next;
			while (m_NextEvent && (m_NextEvent->Code == 0))
				m_NextEvent = (Event*) m_NextEvent->Next;
			Unlock();
			return m_NextEvent;
		}

		void Observe(int eventcode)
		{
			Messenger* mess = GetMessenger();

			if (mess != NULL)
				mess->Observe(NULL, eventcode, NULL);
		}

	protected:
		virtual void Stop()
		{
			StopEvents();
			World3D::Stop();
		}

		virtual bool OnEvent(Event* ev)
		{
			switch (ev->Code)
			{
				default:				// filter out these
				World3D::OnEvent(ev);
				return false;

				case Event::START:
				case Event::STOP:
				case Event::SCENE_CHANGE:
				case Event::LOAD_SCENE:	// pass these to scripting client
				case Event::ENTER:
				case Event::LEAVE:
				case Event::PICK:
				case Event::NOPICK:
				case Event::ERROR:
				World3D::OnEvent(ev);
			}

			if (m_Stopped)
				return true;

			Event* e = MakeEvent(ev->Code);
			*e = *((Event*) ev);	
			e->Next = NULL;
			Lock();
			m_LastEvent = CleanEvents();
			VX_ASSERT(m_LastEvent);
			m_LastEvent->Next = e;				// put new event at the end
			Unlock();
			m_EventPending.Release();			// signal consumer we have events
			return true;
		}

		Event* CleanEvents()
		{
			Event* prev = (Event*) &m_EventQueue;
			Event* cur = (Event*) m_EventQueue.Next;
			while (cur)							// remove already processed events
			{
				if (cur->Code)					// event not handled? leave in queue
					prev = cur;
				else							// event handled? remove from queue			
				{
					prev->Next = cur->Next;		// unlink a middle one
					delete cur;					// garbage collect the event
				}
				cur = (Event*) cur->Next;
			}
			return prev;						// return last event in queue
		}
};
