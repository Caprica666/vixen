using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;
using System.Windows.Navigation;
using System.Windows.Input;
using System.Windows.Interop;
using System.Reflection;

using Vixen;

namespace VideoTest
{
	/// <summary>
	/// Interaction logic for App.xaml
	/// </summary>
    /// 
	public partial class App : VixenWPFApp
	{
		public System.String VideoFile;
		public System.String AudioFile;

		[STAThread]
		public static void Main()
		{
			VideoTest.App app = new VideoTest.App();
			SharedWorld.DoAsyncLoad = false;
			app.InitializeComponent();
			app.Run();
		}

	}
}
