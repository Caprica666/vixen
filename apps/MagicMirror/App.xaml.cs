using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;

using Vixen;

namespace MagicMirror
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        protected SharedWorld VixWorld = null;

        protected override void OnStartup(StartupEventArgs e)
        {
            VixWorld = new SharedWorld();

            Canvas3D.UsePhysics = true;
            Canvas3D.StartVixen(VixWorld);
            base.OnStartup(e);
        }

    }

}
