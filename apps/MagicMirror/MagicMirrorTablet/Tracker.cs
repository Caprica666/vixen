using System;
using System.Collections;
using System.Windows;

using Vixen;

namespace MagicMirror.Tablet
{
public class Tracker
{
	public string	Name = "Tracker";
	public int		IsTracking = 0;
	public int		ButtonMask = 0;
	public Box2		ActiveArea = null;
	public Vec2[]	StartPos = new Vec2[4];
	public Vec2[]	CurPos = new Vec2[4];
	public Vec2[]	EndPos = new Vec2[4];

	public Tracker()
	{
	}


	public virtual void OnMouse(float x, float y, int buttons)
	{
		if (ButtonMask == 0)
			return;
		if (IsTracking != 0)
		{
			if ((buttons & ButtonMask) == 0)
				OnTouchUp(0, x, y);
			else
				OnTouchMove(0, x, y);
		}
		else if ((buttons & ButtonMask) == ButtonMask)
 			OnTouchDown(0, x, y);
	}

	public virtual void OnTouchDown(int finger, float x, float y)
	{
		int fingermask = 1 << finger;
		bool inactivearea = InActiveArea(x, y);
		Vec2 curpos = new Vec2(x, y);

		if (!inactivearea)
			return;
		if (IsTracking == 0)					// are we tracking now?
		{
			StartPos[finger] = curpos;
			CurPos[finger] = curpos;
			IsTracking |= fingermask;			// start tracking
			OnStart(finger, x, y);				// declare done
		}
		else
		{
			if ((IsTracking & fingermask) == 0)	// not tracking this finger?
			{
				IsTracking |= fingermask;		// start tracking
				StartPos[finger] = curpos;
			}
			CurPos[finger] = curpos;
			OnMove(finger, x, y);
		}
	}

	public virtual void OnTouchUp(int finger, float x, float y)
	{
		int fingermask = 1 << finger;
		Vec2 curpos = new Vec2(x, y);

		if ((IsTracking & fingermask) == 0)		// are we tracking this finger?
			return;								// no, ignore up event
		IsTracking &= ~fingermask;				// last finger went up?
		CurPos[finger] = curpos;
		EndPos[finger] = curpos;
		if (IsTracking == 0)
			OnStop(finger, x, y);				// declare done
	}

	public virtual void OnTouchMove(int finger, float x, float y)
	{
		int fingermask = 1 << finger;
		bool inactivearea = InActiveArea(x, y);

		if (!inactivearea)
			return;
		if ((IsTracking & fingermask) == 0)		// not tracking this finger?
			OnTouchDown(finger, x, y);			// treat it as touch down
		else
		{
			CurPos[finger] = new Vec2(x, y);
			OnMove(finger, x, y);
		}
	}

	public bool InActiveArea(float x, float y)
	{
		if (ActiveArea == null)
			return true;
		if ((ActiveArea.Width > 0) && ((x < ActiveArea.min.x) || (x > ActiveArea.max.x)))
			return false;
		if ((ActiveArea.Height > 0) && ((y < ActiveArea.min.y) || (y > ActiveArea.max.y)))
			return false;
		return true;
	}

	protected virtual void OnStart(int index, float x, float y)
	{
	}

	protected virtual void OnStop(int index, float x, float y)
	{
	}

	protected virtual void OnMove(int index, float x, float y)
	{
	}
}

public class SwipeEventArgs : RoutedEventArgs
{
	public string SwipeDirection;
	public Vixen.Vec2 SwipeDelta;

	public SwipeEventArgs(RoutedEvent re, string dir, Vec2 delta)
		: base(re)
	{
		SwipeDirection = dir;
		SwipeDelta = delta;
	}
	~SwipeEventArgs() { }
};

public class Swiper : Tracker
{
	public delegate void SwipeEventHandler(object sender, SwipeEventArgs e);
	public event SwipeEventHandler SwipeEvent;
	public static readonly RoutedEvent RoutedSwipeEvent = EventManager.RegisterRoutedEvent("Swipe",
			RoutingStrategy.Bubble, typeof(SwipeEventHandler), typeof(Tracker));

	public Swiper()
		: base()
	{
	}

	protected override void OnStop(int index, float x, float y)
	{
		Vec2	delta = EndPos[0] - StartPos[0];
		string	dir = "diagonal";
		float	f;
		Vec2 diff = delta;

		diff.Normalize();
		f = diff.Dot(new Vec2(1.0f, 0.0f));
		if (f > 0.8f)						// swipe to the right?
			dir = "right";
		else if (f < -0.8f)					// swipe to the left?
			dir = "left";
		f = diff.Dot(new Vec2(0.0f, 1.0f));
		if (f > 0.8f)						// swipe up?
			dir = "down";
		else if (f < -0.8f)					// swipe down?
			dir = "up";
		if (SwipeEvent != null)
		{
			SwipeEventArgs args = new SwipeEventArgs(RoutedSwipeEvent, dir, delta);
			SwipeEvent(this, args);
		}
	}
}

}
