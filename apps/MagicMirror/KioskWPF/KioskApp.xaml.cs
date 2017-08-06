using System;
using System.Collections.Generic;
using System.Configuration;
using System.Windows;
using MagicMirror;

namespace MagicMirror
{
	namespace Kiosk
	{
		public partial class KioskApp  : Vixen.VisualApp 
		{
			protected override void OnStartup(StartupEventArgs e)
			{
				Vixen.Canvas3D.UsePhysics = true;
				VixWorld = new Vixen.SharedWorld();
				base.OnStartup(e);
			}
		}
	}
}

