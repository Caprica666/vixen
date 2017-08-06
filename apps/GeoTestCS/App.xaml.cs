using System;
using System.Collections.Generic;
using System.Configuration;
using System.Windows;
using System.Windows.Interop;
using System.Reflection;


namespace GeoTest
{
	public partial class App : Vixen.VisualApp
	{
		protected override void OnStartup(StartupEventArgs args)
		{
			Vixen.Scene.RenderOptions = "hlsl";
			Vixen.World.DoAsyncLoad = false;
			base.OnStartup(args);
 		}
	}
}
