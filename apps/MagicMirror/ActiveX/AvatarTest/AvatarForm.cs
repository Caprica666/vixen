using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace AvatarTest
{
	public partial class AvatarForm : Form
	{
		public AvatarForm()
		{
			InitializeComponent();
			avatarControl = new AvatarAX.AvatarControl();
			avatarControl.Name = "avatarControl";
			avatarControl.OnLoadAvatar += OnLoadAvatar;
			avatarControl.OnLoadAnim += AddAnimation;
			loadAnimDialog.InitialDirectory = avatarControl.MediaDir + "\\anim\\amy";
			loadAnimDialog.Filter = "Motion files|*.bvh";
			twoPanes.Panel2.Controls.Add(avatarControl);
			dynamic config = avatarControl.LoadConfig("c:\\projects\\vixen\\data\\magicmirror\\amyconfig.json");
			if ((config != null) && (config.mediadir != null))
				avatarControl.MediaDir = config.mediadir;
		}

		/*
		 * Called every time an avatar is loaded.
		 * We load the green dress and put it on her.
		 */
		private void OnLoadAvatar(String name)
		{
			avatarControl.LoadGarment(avatarControl.ConfigOpts.defaultoutfit, null);
		}

		/*
		 * Called every time an animation is loaded.
		 * We add the name of the animation to a list box.
		 */
		private void AddAnimation(String name, String url)
		{
			if (!animationsLoaded.Items.Contains(name))
				animationsLoaded.Items.Add(name);
		}

		/*
		 * Called when "pause" button is clicked.
		 * Stops the currently running animation.
		 */
		private void pauseButton_Click(object sender, EventArgs e)
		{
			try
			{
				string selected = animationsLoaded.SelectedItem as string;

				if (selected != null)
					avatarControl.PauseAnimation(selected);
			}
			catch (Exception ex)
			{
				Vixen.SharedWorld.LogError("OnPauseAnimation EXCEPTION: " + ex.Message);
			}
		}

		/*
		 * Called when an animation is selected.
		 * Plays the selected animation.
		 */
		private void OnAnimSelected(object sender, EventArgs e)
		{
			try
			{
				string selected = animationsLoaded.SelectedItem as string;

				if (selected != null)
					avatarControl.PlayAnimation(selected, 0);
			}
			catch (Exception ex)
			{
				Vixen.SharedWorld.LogError("OnAnimSelected EXCEPTION: " + ex.Message); 
			}
		}

		/*
		 * Called when the "load" button is pressed.
		 * Brings up a file open dialog to select the animation file to open.
		 */
		private void OnOpenAnimFile(object sender, EventArgs e)
		{
			DialogResult result = loadAnimDialog.ShowDialog();
			if (result == DialogResult.OK)
				avatarControl.LoadAnimation(loadAnimDialog.FileName, null);
		}

	}
}
