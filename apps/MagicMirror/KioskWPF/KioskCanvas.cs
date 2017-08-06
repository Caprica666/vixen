using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Interop;
using System.IO;

using Vixen;
using MagicMirror;

namespace MagicMirror.Kiosk
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
	public class KioskCanvas : AvatarCanvas
	{		
		/*!
		 * String with the URL or path to the Kinect configuration file.
		 * If the Kinect config file is found, the Kinect body tracker is
		 * used to control the avatar and select outfits. Otherwise,
		 * the mouse is used to pick.
		 */
		public string KinectConfig
		{
			get { return base.GetValue(KinectConfigProp) as System.String; }
			set { base.SetValue(KinectConfigProp, value); }
		}

		public static readonly DependencyProperty KinectConfigProp = DependencyProperty.Register("KinectConfig", typeof(System.String), typeof(Canvas3D));

		protected override void OnInitialized(System.EventArgs e)
		{
			if (KinectConfig != null)
			{
				MainWindow mainwin = (MainWindow) Parent;
				//mainwin.FullScreen(true);
				try
				{
					scene = new KioskScene(GetMediaPath(KinectConfig));
				}
				catch (System.IO.FileNotFoundException exc)
				{
					SharedWorld.LogError(exc.Message);
					scene = new KioskScene();
				}
			}
			else
				scene = new KioskScene();
			base.OnInitialized(e);
		}

		protected override void OnMouse(float x, float y, int buttons, float t)
		{
			KioskScene kscene = scene as KioskScene;
			if (kscene != null)
				kscene.OnMouse(x, y, buttons, t);
		}
	}
}
