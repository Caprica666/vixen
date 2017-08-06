using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using MagicMirror;

	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		private Microsoft.Win32.OpenFileDialog loadAnimDialog;
		private MagicMirror.Viewer.ClothRecorder _clothAnim = null;
		private Garment _currentGarment = null;
		private float _timeinc = 1.0f / 24.0f;

		public MainWindow()
		{
			loadAnimDialog = new Microsoft.Win32.OpenFileDialog();
			InitializeComponent();
			Vixen.SharedWorld.SetDebugLevel(1);
		}

		protected override void OnContentRendered(EventArgs e)
		{
			base.OnContentRendered(e);
			avatarCanvas.OnLoadAnim += OnLoadAnim;
			avatarCanvas.OnLoadAvatar += OnLoadAvatar;
			avatarCanvas.OnLoadGarment += OnLoadGarment;
			avatarCanvas.OnStop += OnStopAnim;
			avatarCanvas.TransferDelegates();
			loadAnimDialog.InitialDirectory = avatarCanvas.MediaDir + "/anim/" + avatarCanvas.ConfigOpts.avatar.name;
			loadAnimDialog.InitialDirectory = loadAnimDialog.InitialDirectory.Replace('/', '\\');
			loadAnimDialog.Filter = "Motion files|*.bvh|Vixen files|*.vix|Maya cache files|*.xml|Havok files|*.hkt";
			loadButton.Click += new RoutedEventHandler(OnLoadClicked);
			playButton.Click += new RoutedEventHandler(OnPlayClicked);
			saveAnimButton.Click += new RoutedEventHandler(OnSaveAnimClicked);
			saveClothButton.Click += new RoutedEventHandler(OnSaveClothClicked);
			recordButton.Click += new RoutedEventHandler(OnRecordClicked);
		}

		/*
		 * Come here when "load" button has been pressed.
		 * Bring up file open dialog
		 */
		protected void OnLoadClicked(object sender, EventArgs e)
		{
			Nullable<bool> result = loadAnimDialog.ShowDialog();
			if (result.Value)
				avatarCanvas.LoadAnimation(loadAnimDialog.FileName, null);
		}


		/*
		 * Come here when "save cloth" button has been pressed.
		 * Bring up file save dialog
		 */
		protected void OnSaveClothClicked(object sender, EventArgs e)
		{
			if (_clothAnim != null)
				_clothAnim.SaveCloth();
		}

		/*
		 * Come here when "save animation" button has been pressed.
		 * Bring up file save dialog
		 */
		protected void OnSaveAnimClicked(object sender, EventArgs e)
		{
			string selected = animationsLoaded.SelectedItem as string;
			dynamic g = _currentGarment;

			if ((selected == null) || (g == null))
				return;
			if (_clothAnim == null)
				_clothAnim = new MagicMirror.Viewer.ClothRecorder(_timeinc);
			else if (recordButton.IsChecked == true)
			{
				_clothAnim.SaveAnimation();
				recordButton.IsChecked = false;
				return;
			}
			Vixen.Engine saveme;
			string name = selected + "_" + g.name;

			_clothAnim.AnimName = selected;
			_clothAnim.BaseName = g.name;
			saveme = avatarCanvas.FindAnimation(name);
			if ((saveme != null) && (typeof(Vixen.Animator).IsAssignableFrom(saveme.GetType())))
			{
				Vixen.Animator anim = (Vixen.Animator) saveme;
				Vixen.Engine root = anim.GetRootEngine();
				Vixen.MeshAnimator meshanim;

				if ((root != null) && typeof(Vixen.MeshAnimator).IsAssignableFrom(root.GetType()))
				{
					meshanim = (Vixen.MeshAnimator)root;
					_clothAnim.SaveAnimation(meshanim);
				}
			}
			recordButton.IsChecked = false;
		}

		/*
		 * Come here when animation file has been loaded.
		 * Add it to the list of animations.
		 */
		public void OnLoadAnim(String name, String url)
		{
			if (!animationsLoaded.Items.Contains(name))
				animationsLoaded.Items.Add(name);
		}

		/*
		 * Come here when record button is pressed.
		 * Add it to the list of animations.
		 */
		protected void OnRecordClicked(object sender, EventArgs e)
		{
			try
			{
				string selected = animationsLoaded.SelectedItem as string;
				CheckBox button = sender as CheckBox;
				bool ischecked = (bool) button.IsChecked;

				if (_currentGarment == null)
					return;
				if (_clothAnim == null)
					ConnectClothAnim(_currentGarment);
				_clothAnim.Record(ischecked);
			}
			catch (Exception ex)
			{
				Vixen.SharedWorld.LogError("OnAnimSelected EXCEPTION: " + ex.Message);
			}
		}

		protected void OnPlayClicked(object sender, EventArgs e)
		{
			try
			{
				string selected = animationsLoaded.SelectedItem as string;

				if (selected != null)
				{
					if (_clothAnim != null)
					{
						_clothAnim.AnimName = selected;
						_clothAnim.Start();
					}
					avatarCanvas.PlayAnimation(selected, 0);
				}
			}
			catch (Exception ex)
			{
				Vixen.SharedWorld.LogError("OnAnimSelected EXCEPTION: " + ex.Message);
			}
		}

		/*
		 * Called when an animation stops.
		 */
		private void OnStopAnim(string name)
		{
			string selected = animationsLoaded.SelectedItem as string;

			if ((selected != null) && name.StartsWith(selected) && (_clothAnim != null))
				_clothAnim.Pause();
		}

		/*
		 * Called when an avatar is loaded.
		 */
		private void OnLoadAvatar(String name)
		{
			dynamic d = avatarCanvas.ConfigOpts;

			if (d.timeinc != null)
			{
				_timeinc = (float) Double.Parse(d.timeinc);
				Vixen.SharedWorld.MainScene.TimeInc = _timeinc;
			}
			if (avatarCanvas.ConfigOpts.defaultoutfit != null)
				avatarCanvas.LoadGarment(avatarCanvas.ConfigOpts.defaultoutfit, null);
		}

		/*
		 * Called when a garment is loaded.
		 */
		private void OnLoadGarment(String name, String filename)
		{
			Garment garment = avatarCanvas.FindGarment(name);

			_currentGarment = garment;
		}

		private bool ConnectClothAnim(Garment garment)
		{
			try
			{
				dynamic g = garment;
				Vixen.Model clothmesh = garment.ClothMesh;
				Vixen.SharedWorld world = Vixen.SharedWorld.Get();
				dynamic d = avatarCanvas.ConfigOpts;

				if (d.timeinc != null)
					_timeinc = (float) Double.Parse(d.timeinc);
				if (clothmesh == null)
					return false;
				if (_clothAnim == null)
					_clothAnim = new MagicMirror.Viewer.ClothRecorder(_timeinc);
				else
					_clothAnim.Clear();
				_clothAnim.BaseName = g.Name;
				world.SuspendScene();
				_clothAnim.AnimRoot = null;
				_clothAnim.MeshRoot = clothmesh;
				_clothAnim.AnimRoot.PutAfter(Vixen.Physics.Get());
				world.ResumeScene();
				return true;
			}
			catch (Exception ex)
			{
				Vixen.SharedWorld.LogError(ex.Message);
			}
			return false;
		}

	}
