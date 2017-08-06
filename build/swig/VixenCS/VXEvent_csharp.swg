
namespace Vixen
{
	%csmethodmodifiers Event::Log	"protected";

	%typemap(csout, excode=SWIGEXCODE) Event {
		IntPtr cPtr = $imcall;
		$csclassname ret = ($csclassname)  Event.MakeEvent(cptr, true);$excode
		return ret;
	}

	%typemap(csout, excode=SWIGEXCODE) Event* {
		IntPtr cPtr = $imcall;$excode
		return ($csclassname) Event.MakeEvent(cPtr, $owner);
	}

	%typemap(csout, excode=SWIGEXCODE) Event& {
		IntPtr cPtr = $imcall;$excode
		return ($csclassname) Event.MakeEvent(cPtr, $owner);
	}

	%typemap(csvarout, excode=SWIGEXCODE2) Event& %{
		get
		{
		  IntPtr cPtr = $imcall;$excode
		  return ($csclassname) Event.MakeEvent(cPtr, false);
		}
	%}

	%typemap(csvarout, excode=SWIGEXCODE2) Event* %{
		get
		{
		  IntPtr cPtr = $imcall;$excode
		  return ($csclassname) Event.MakeEvent(cPtr, false);
		}
	%}
		
	%typemap(cscode) Event %{
		public void Log()
		{
			swigCMemOwn = false;
			LogInternal();
		}

		static public Event MakeEvent(IntPtr cptr, bool memown)
		{
			if (cptr == IntPtr.Zero)
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
