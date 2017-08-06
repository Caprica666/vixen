using System;
using System.Collections;

namespace Vixen
{
public class HandTracker
{
	public System.String Name = "HandTracker";
	public int		ButtonMask = MouseEvent.SHIFT;
	public float	ZOffset = 0.0f;
	public Box3		ActiveArea = null;
	public Box3		ViewArea = null;
	public bool		IsTracking = false;
	public Model	LeftHandCursor = null;
	public Model	RightHandCursor = null;
	public SpotLight RightLight = null;
	public SpotLight LeftLight = null;
	public Vec3		LeftHandPos = new Vec3(0, 0, 0);
	public Vec3		RightHandPos = new Vec3(0, 0, 0);
    public Vec3     WorldPos;
	public enum Hand : int { NONE = 0, LEFT = 1, RIGHT = 2 };

	protected ArrayList children = new ArrayList();

	public HandTracker() : base()
	{
	}

	public void Append(HandTracker child)
	{
		children.Add(child);
	}

	public virtual void OnMouse(float x, float y, int buttons, float time_in_sec)
	{
		WorldPos = WorldFromScreen(new Vec3(x, y, 0));
		if (IsTracking)
		{
			if ((buttons & ButtonMask) == 0)
			{
				IsTracking = false;
				OnStop(Hand.LEFT);
				OnStop(Hand.RIGHT);
			}
			else
			{
				if (InActiveArea(WorldPos))
				{
					RightHandPos = WorldPos;
					OnMove(Hand.RIGHT, WorldPos, null);
				}
				else
				{
					IsTracking = false;
					OnStop(Hand.LEFT);
					OnStop(Hand.RIGHT);
				}
			}
		}
		else if ((buttons & ButtonMask) == ButtonMask)
		{
			IsTracking = true;
			LeftHandPos = WorldPos;
			 RightHandPos = WorldPos;
			OnStart(Hand.RIGHT, RightHandPos);
		}
		foreach (HandTracker child in children)
			child.OnMouse(x, y, buttons, time_in_sec);
	}

	public virtual void OnEvent(Event ev)
	{
		if (ev.Code != Event.TRACK)
			return;
		try
		{
			Hand hand = Hand.NONE;
			TrackEvent tev = ev as TrackEvent;
			Vec3 p = WorldFromTrack2D(tev.Position);
 
			if (tev.PartIndex == Skeleton.LEFT_HAND)
			{
				hand = Hand.LEFT;
				LeftHandPos = p;
			}
			else if (tev.PartIndex == Skeleton.RIGHT_HAND)
			{
				hand = Hand.RIGHT;
				RightHandPos = p;
			}
			else if (tev.Type == TrackEvent.USER_MOVE)
			{
				if (InActiveArea(p))
					OnMove(hand, p, null);
				else
					OnStop(hand);
			}
		}
		catch (Exception) { }
		foreach (HandTracker child in children)
			child.OnEvent(ev);
	}

	public Vec3 Normalize(Vec3 p)
	{
		Scene scene = SharedWorld.MainScene;
		Camera cam = scene.Camera;
		Box3 viewarea = ViewArea;
		Vec3 n = new Vec3();

		if (viewarea != null)
			n.z = (p.z - viewarea.min.z) / viewarea.Depth;
		else
		{
			viewarea = new Box3(0, 0, cam.Hither, scene.Viewport.Width, scene.Viewport.Height, cam.Yon);
			n.z = cam.Translation.z / viewarea.Depth;
		}
		n.x = (p.x - viewarea.min.x) / viewarea.Width;
		n.y = (p.y - viewarea.min.y) / viewarea.Height;
		//SharedWorld.Trace(System.String.Format("HandTracker:Normalize ({0}, {1}, {2}) -> ({3}, {4}, {5})\n", p.x, p.y, p.z, n.x, n.y, n.z));
		return n;
	}

	public Vec3 WorldFromScreen(Vec3 pos)
	{
		Scene	scene = SharedWorld.MainScene;
		Camera	cam = scene.Camera;
		Box3	vv = cam.ViewVol;
		Vec3	p = Normalize(pos);
		Matrix	mtx = cam.GetViewTrans();
		Vec3	w = new Vec3(vv.min.x + p.x * vv.Width,
							 vv.max.y - p.y * vv.Height,
							 vv.min.z - p.z * vv.Depth + ZOffset);
		float	tmp = -0.5f * w.z / cam.Hither;
		
		tmp = -w.z / cam.Hither;
		w.x *= tmp;
		w.y *= tmp;
		cam.TotalTransform(mtx);
		mtx.Transform(w, w);
		SharedWorld.Trace(System.String.Format(Name + " Screen({0}, {1}, {2}) -> World({3}, {4}, {5})\n", pos.x, pos.y, pos.z, w.x, w.y, w.z));
		return w;
	}

	public Vec3 WorldFromTrack2D(Vec3 p)
	{
		Scene scene = SharedWorld.MainScene;
		Camera cam = scene.Camera;
		Box3 vvol = cam.ViewVol;
		float camdist = cam.Translation.Length;
		float zdist = camdist - ZOffset;
		Vec3 w = new Vec3();
		Vec3 v = new Vec3();
		Matrix mtx = new Matrix();
		float tmp = 0.5f * vvol.Height / vvol.min.z;

		v.x = p.x * tmp * zdist;
		v.y = p.y * tmp * zdist;
		v.z = -zdist;
		cam.TotalTransform(mtx);
		mtx.Transform(v, w);
		SharedWorld.Trace(System.String.Format(Name + " Track({0}, {1}, {2}) -> World({3}, {4}, {5})\n", p.x, p.y, p.z, w.x, w.y, w.z));
		return w;
	}

		public Vec3 WorldFromTrack3D(Vec3 p)
		{
			Scene scene = SharedWorld.MainScene;
			Camera cam = scene.Camera;
			Box3 vvol = cam.ViewVol;
			float camdist = cam.Translation.Length;
			Vec3 w = new Vec3();
			Vec3 v = new Vec3();
			Matrix mtx = new Matrix();
			float tmp;

			v.z = camdist + vvol.Depth * p.z;
			tmp = 0.5f * vvol.Height * v.z / vvol.min.z;
			v.x = p.x * tmp;
			v.y = p.y * tmp;
			v.z = -v.z;
			cam.TotalTransform(mtx);
			mtx.Transform(v, w);
			w.z = -w.z;
			SharedWorld.Trace(System.String.Format(Name + " Track({0}, {1}, {2}) -> World({3}, {4}, {5})\n", p.x, p.y, p.z, w.x, w.y, w.z));
			return w;
		}

	public bool InActiveArea(Vec3 p)
	{
		if (ActiveArea == null)
			return true;
		if ((ActiveArea.Width > 0) && ((p.x < ActiveArea.min.x) || (p.x > ActiveArea.max.x)))
			return false;
		if ((ActiveArea.Height > 0) && ((p.y < ActiveArea.min.y) || (p.y > ActiveArea.max.y)))
			return false;
		if ((ActiveArea.Depth > 0) && ((p.z < ActiveArea.min.z) || (p.z > ActiveArea.max.z)))
			return false;
		return true;
	}

	protected virtual void OnStart(Hand partindex, Vec3 p)
	{
		if ((partindex == Hand.RIGHT) && (RightHandCursor != null))
		{
			RightHandPos = p;
			if (RightHandCursor != null)
				RightHandCursor.Translation = p;
			RightHandCursor.Active = true;
		}
		else if ((partindex == Hand.LEFT) && (LeftHandCursor != null))
		{
			LeftHandPos = p;
			if (LeftHandCursor != null)
				LeftHandCursor.Translation = p;
			LeftHandCursor.Active = true;
		}
	}

	protected virtual void OnStop(Hand partindex)
	{
		if ((partindex == Hand.RIGHT) && (RightHandCursor != null))
			RightHandCursor.Active = false;
		else if ((partindex == Hand.LEFT) && (LeftHandCursor != null))
			LeftHandCursor.Active = false;
	}

	protected virtual void OnMove(Hand partindex, Vec3 p, Quat r)
	{
		Vec3 campos = SharedWorld.MainScene.Camera.Translation;
		if ((partindex == Hand.RIGHT) && (RightHandCursor != null))
		{
			RightHandPos = p;
			if (RightHandCursor != null)
			{
				Vec3 axis = new Vec3(0.0f, 0.0f, 1.0f);
				Vec3 dir = p - campos;
				Quat rot = new Quat(axis, dir);
				RightHandCursor.Translation = p;
				RightHandCursor.Rotation = rot;
				axis.z = -1.0f;
				if (RightLight != null)
					RightLight.Rotation = new Quat(axis, dir);
			}
		}
		else if ((partindex == Hand.LEFT) && (LeftHandCursor != null))
		{
			LeftHandPos = p;
			if (LeftHandCursor != null)
			{
				Vec3 axis = new Vec3(0.0f, 0.0f, 1.0f);
				Vec3 dir = p - campos;
				Quat rot = new Quat(axis, dir);
				LeftHandCursor.Translation = p;
				LeftHandCursor.Rotation = rot;
				axis.z = -1.0f;
				if (LeftLight != null)
					LeftLight.Rotation = new Quat(axis, dir);
			}
		}
	}
}

}
