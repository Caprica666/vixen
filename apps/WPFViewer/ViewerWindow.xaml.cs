using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using Vixen;
using ObjIO;
using Xceed.Wpf.Toolkit.PropertyGrid;

namespace Vixen.Viewer
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class ViewerWindow : Window
	{
		public Viewer3D Viewer { get { return _viewer; } }
		private Viewer3D _viewer = null;

		public ViewerWindow()
		{
			Canvas3D.UsePhysics = true;
			InitializeComponent();
			view3D.SetSceneEvent += OnSetScene;
			view3D.LoadAnimEvent += OnLoadAnim;
			sceneView.Properties = property3d;
			zoomButton.Click += sceneView.OnZoom;
			startButton.Click += sceneView.OnStart;
			stopButton.Click += sceneView.OnStop;
		}

		protected override void OnActivated(EventArgs e)
		{
			view3D.SetSceneEvent += OnSetScene;
		}

		protected void OnSetScene(String scenename)
		{
			sceneView.Root = SharedWorld.MainScene;
		}

		protected void OnLoadAnim(String name, String url)
		{
			sceneView.Refresh();
		}

		/*
		 * Come here when "Open Scene" menu item has been chosen.
		 * Brings up file open dialog for scenes.
		 */
		public void OnOpenSceneClicked(object sender, RoutedEventArgs e)
		{
			view3D.OpenScene("vix,hkt,obj,xml", view3D.MediaDir);
		}

		/*
		 * Come here when "Save Scene" menu item has been chosen.
		 * Brings up file save dialog for scenes.
		 */
		public void OnSaveSceneClicked(object sender, RoutedEventArgs e)
		{
			view3D.SaveScene("vix", view3D.MediaDir);
		}

		/*
		 * Come here when "Open Animation" menu item has been chosen.
		 * Brings up file open dialog for animations.
		 */
		public void OnOpenAnimClicked(object sender, RoutedEventArgs e)
		{
			view3D.OpenAnimation("bvh,vix,scp", view3D.MediaDir);
		}

		public void OnPlayClicked(object sender, RoutedEventArgs e)
		{
			view3D.PlayAnimation(null);
		}

		public void OnPauseClicked(object sender, RoutedEventArgs e)
		{
			view3D.PauseAnimation(null);
		}

		public void OnShowAllClicked(object sender, RoutedEventArgs e)
		{
			Scene scene = SharedWorld.MainScene;

			if (scene != null)
				scene.ShowAll();
		}
		
	}

} // end Vixen
