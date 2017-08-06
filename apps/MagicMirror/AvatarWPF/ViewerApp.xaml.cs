using System;
using System.Configuration;
using System.Windows;

using Vixen;

namespace MagicMirror.Viewer
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class ViewerApp  : VisualApp 
    {
        protected override void OnStartup(StartupEventArgs e)
        {
			Canvas3D.UsePhysics = true;
			Canvas3D.Debug = 1;
			VixWorld = new SharedWorld();
            base.OnStartup(e);
        }
    }

}
