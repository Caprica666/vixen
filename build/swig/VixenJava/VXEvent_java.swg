/*
 * SWIG interface file for Event
 */
namespace Vixen
{
	%javamethodmodifiers Event::Log	"protected";

	%typemap(javaout) Event {
		jlong ptr = $jnicall;
		if (ptr == 0)
			return null;
		return ($javaclassname)  Event.MakeEvent(ptr, true);
	}

	%typemap(javaout, excode=SWIGEXCODE) Event* {
		jlong ptr = $jnicall;
		if (ptr == 0)
			return null;
		return ($javaclassname) Event.MakeEvent(ptr, $owner);
	}

	%typemap(javaout, excode=SWIGEXCODE) Event& {
		jlong ptr = $jnicall;
		return ($javaclassname) Event.MakeEvent(ptr, $owner);
	}
		
	%typemap(javacode) Event %{
		public void Log()
		{
			swigCMemOwn = false;
			LogInternal();
		}

		static public Event MakeEvent(jlong cptr, bool memown)
		{
			if (cptr == 0)
				return null;
			Event	src = new Event(cptr, false);
			Event	dst;
			int		code = src.Code;

			if (code == Event.TRACK)
				dst = new TrackEvent(cptr, memown);
			else if ((code == Event.PICK) ||
					 (code == Event.NOPICK) ||		
					 (code == Event.DESELECT) ||		
					 (code == Event.SELECT))
				dst = new PickEvent(cptr, memown);
			else if ((code == Event.LOAD_IMAGE) ||
					 (code == Event.LOAD_SCENE))
				dst = new LoadSceneEvent(cptr, memown);
			else if (code == Event.LOAD_TEXT)
				dst = new LoadTextEvent(cptr, memown);
			else if (code == Event.LOAD_DATA)
				dst = new LoadDataEvent(cptr, memown);
			else if ((code == Event.ATTACH_SCENE) ||
					 (code == Event.SCENE_CHANGE))
				dst = new SceneEvent(cptr, memown);
			else if ((code == Event.ENTER) ||
					 (code == Event.LEAVE) ||		
					 (code == Event.COLLIDE))
				dst = new TriggerEvent(cptr, memown);
			else if (code == Event.ERROR)
				dst = new ErrorEvent(cptr, memown);
			else if (code == Event.MOUSE)
				dst = new MouseEvent(cptr, memown);
			else if (code == Event.KEY)
				dst = new KeyEvent(cptr, memown);
			else if (code == Event.NAVIGATE)
				dst = new NavigateEvent(cptr, memown);
			else if (code == Event.NAVINPUT)
				dst = new NavInputEvent(cptr, memown);
			else if (code == Event.TRI_HIT)
				dst = new TriHitEvent(cptr, memown);
			else
				return src;
			return dst;
		}
	%}
}
