using System;
using System.Windows;

namespace MagicMirror.Tablet
{
	public partial class TabletApp : Vixen.VisualApp
	{
		protected override void OnStartup(StartupEventArgs e)
		{
			Vixen.Canvas3D.UsePhysics = true;
			VixWorld = new Vixen.SharedWorld();
			base.OnStartup(e);
		}
	}
}
