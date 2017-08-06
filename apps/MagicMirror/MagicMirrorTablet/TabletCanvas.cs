using System;
using System.Dynamic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Interop;
using System.IO;
using System.Web.Helpers;

using Vixen;
using MagicMirror;

namespace MagicMirror.Tablet
{
	/*!
	 * 3D WPF control for virtual dressing room kiosk.
	 * Input properties specify initial configuration -
	 * background, avatar, clothing, animations, UI.
	 * The avatar uses the Kinect to track the users body movements.
	 * The user interface to select clothing is gestural - you move
	 * that avatar's hands and she selects items in the 3D scene
	 * using 3D collision detection. If no Kinect configuration
	 * file is provided, outfits can be selected with the mouse.
	 */
	public class TabletCanvas : AvatarCanvas
	{
		public Tracker MouseTracker = null;

		protected override void OnInitialized(System.EventArgs e)
		{
			scene = new AvatarScene();
			scene.SuppressNavigation = true;
			PreviewTouchDown += OnTouchDown;
			PreviewTouchUp += OnTouchUp;
			PreviewTouchMove += OnTouchMove;
			OnStop += OnStopAnim;

			base.OnInitialized(e);
		}

		protected override void OnMouse(float x, float y, int buttons, float t)
		{
			base.OnMouse(x, y, buttons, t);
			if (MouseTracker != null)
				MouseTracker.OnMouse(x, y, buttons);
		}

		public void OnStopAnim(System.String anim_name)
		{
			if (scene.NowPlaying == anim_name)
				RandomAnimation("idle", 0);
		}

		/*
		 * Called when the user swipes with one finger or the mouse
		 */
		public void OnSwipe(object sender, SwipeEventArgs e)
		{
			if (e.SwipeDirection == "left")			// you swipe left
				RandomAnimation("right", 0.5f);		// she turns to her right (your left)
			else if (e.SwipeDirection == "right")	// you swip right
				RandomAnimation("left", 0.5f);		// she turns to her left (your right)
			else if (e.SwipeDirection == "up")		// you swipe up
				RandomAnimation("front", 0.5f);		// she walks the catwalk
			else
				RandomAnimation("idle", 0.5f);
		}


		protected void OnTouchDown(Object sender, TouchEventArgs e)
		{
			if (MouseTracker != null)
			{
				int finger = e.TouchDevice.Id;
				Point p = e.GetTouchPoint(this).Position;
				MouseTracker.OnTouchDown(finger, (float)p.X, (float)p.Y);
			}
		}

		protected void OnTouchUp(Object sender, TouchEventArgs e)
		{
			if (MouseTracker != null)
			{
				int finger = e.TouchDevice.Id;
				Point p = e.GetTouchPoint(this).Position;
				MouseTracker.OnTouchUp(finger, (float)p.X, (float)p.Y);
			}
		}

		protected void OnTouchMove(Object sender, TouchEventArgs e)
		{
			if (MouseTracker != null)
			{
				int finger = e.TouchDevice.Id;
				Point p = e.GetTouchPoint(this).Position;
				MouseTracker.OnTouchMove(finger, (float)p.X, (float)p.Y);
			}
		}
	}
}
