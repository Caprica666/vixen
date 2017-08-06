using System;
using System.Collections.Generic;
using System.Linq;
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
using Vixen;

namespace MagicMirror.Kiosk
{
    public partial class MainWindow : Window
    {
        public MainWindow()
            : base()
        {
            InitializeComponent();
            KeyUp += KeyEventHandler;
        }

        public void FullScreen(bool flag)
        {
            if (flag)
            {
                ShowInTaskbar = false;
                WindowStyle = WindowStyle.None;
                WindowState = WindowState.Maximized;
            }
            else
            {
                ShowInTaskbar = false;
                WindowStyle = WindowStyle.SingleBorderWindow;
                WindowState = WindowState.Normal;
            }
        }

        protected void KeyEventHandler(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.F11:		// full screen
                FullScreen(true);
                break;

                case Key.F12:		// restore normal size
                FullScreen(false);
                break;
            }
        }
    }
}
