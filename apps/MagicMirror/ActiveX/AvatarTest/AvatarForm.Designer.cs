namespace AvatarTest
{
	partial class AvatarForm
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.twoPanes = new System.Windows.Forms.SplitContainer();
			this.loadButton = new System.Windows.Forms.Button();
			this.animListLabel = new System.Windows.Forms.Label();
			this.animationsLoaded = new System.Windows.Forms.ListBox();
			this.stopButton = new System.Windows.Forms.Button();
			this.playButton = new System.Windows.Forms.Button();
			this.loadAnimDialog = new System.Windows.Forms.OpenFileDialog();
			((System.ComponentModel.ISupportInitialize)(this.twoPanes)).BeginInit();
			this.twoPanes.Panel1.SuspendLayout();
			this.twoPanes.SuspendLayout();
			this.SuspendLayout();
			// 
			// twoPanes
			// 
			this.twoPanes.Dock = System.Windows.Forms.DockStyle.Fill;
			this.twoPanes.Location = new System.Drawing.Point(0, 0);
			this.twoPanes.Name = "twoPanes";
			// 
			// twoPanes.Panel1
			// 
			this.twoPanes.Panel1.Controls.Add(this.loadButton);
			this.twoPanes.Panel1.Controls.Add(this.animListLabel);
			this.twoPanes.Panel1.Controls.Add(this.animationsLoaded);
			this.twoPanes.Panel1.Controls.Add(this.stopButton);
			this.twoPanes.Panel1.Controls.Add(this.playButton);
			this.twoPanes.Size = new System.Drawing.Size(775, 507);
			this.twoPanes.SplitterDistance = 219;
			this.twoPanes.TabIndex = 0;
			// 
			// loadButton
			// 
			this.loadButton.Location = new System.Drawing.Point(24, 223);
			this.loadButton.Name = "loadButton";
			this.loadButton.Size = new System.Drawing.Size(46, 23);
			this.loadButton.TabIndex = 5;
			this.loadButton.Text = "load";
			this.loadButton.UseVisualStyleBackColor = true;
			this.loadButton.Click += new System.EventHandler(this.OnOpenAnimFile);
			// 
			// animListLabel
			// 
			this.animListLabel.AutoSize = true;
			this.animListLabel.Location = new System.Drawing.Point(72, 4);
			this.animListLabel.Name = "animListLabel";
			this.animListLabel.Size = new System.Drawing.Size(74, 16);
			this.animListLabel.TabIndex = 4;
			this.animListLabel.Text = "Animations";
			// 
			// animationsLoaded
			// 
			this.animationsLoaded.FormattingEnabled = true;
			this.animationsLoaded.ItemHeight = 16;
			this.animationsLoaded.Location = new System.Drawing.Point(23, 36);
			this.animationsLoaded.Name = "animationsLoaded";
			this.animationsLoaded.Size = new System.Drawing.Size(173, 164);
			this.animationsLoaded.TabIndex = 3;
			this.animationsLoaded.SelectedIndexChanged += new System.EventHandler(this.OnAnimSelected);
			// 
			// stopButton
			// 
			this.stopButton.Location = new System.Drawing.Point(151, 223);
			this.stopButton.Name = "stopButton";
			this.stopButton.Size = new System.Drawing.Size(46, 23);
			this.stopButton.TabIndex = 2;
			this.stopButton.Text = "stop";
			this.stopButton.UseVisualStyleBackColor = true;
			this.stopButton.Click += new System.EventHandler(this.pauseButton_Click);
			// 
			// playButton
			// 
			this.playButton.Location = new System.Drawing.Point(86, 223);
			this.playButton.Name = "playButton";
			this.playButton.Size = new System.Drawing.Size(46, 23);
			this.playButton.TabIndex = 1;
			this.playButton.Text = "play";
			this.playButton.UseVisualStyleBackColor = true;
			this.playButton.Click += new System.EventHandler(this.OnAnimSelected);
			// 
			// loadAnimDialog
			// 
			this.loadAnimDialog.FileName = "loadAnimDialog";
			this.loadAnimDialog.Title = "Load Animation";
			// 
			// AvatarForm
			// 
			this.ClientSize = new System.Drawing.Size(775, 507);
			this.Controls.Add(this.twoPanes);
			this.Name = "AvatarForm";
			this.Text = "AvatarForm";
			this.twoPanes.Panel1.ResumeLayout(false);
			this.twoPanes.Panel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.twoPanes)).EndInit();
			this.twoPanes.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private AvatarAX.AvatarControl avatarControl;
		private System.Windows.Forms.SplitContainer twoPanes;
		private System.Windows.Forms.Button playButton;
		private System.Windows.Forms.Button stopButton;
		private System.Windows.Forms.ListBox animationsLoaded;
		private System.Windows.Forms.Label animListLabel;
		private System.Windows.Forms.OpenFileDialog loadAnimDialog;
		private System.Windows.Forms.Button loadButton;

	}
}

