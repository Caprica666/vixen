using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;

using Vixen;
using MagicMirror;

namespace MagicMirrorWPF
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class MagicMirrorApp  : VixenWPFApp 
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            Canvas3D.UsePhysics = true;
			VixWorld = new SharedWorld();
            base.OnStartup(e);
        }
    }

}
