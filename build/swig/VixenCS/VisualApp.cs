using System;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Threading;
using System.Threading.Tasks;

namespace Vixen
{	
	public class VisualApp : Application
	{
		protected	SharedWorld VixWorld = null;

		protected override void OnStartup(StartupEventArgs e)
		{
			if (VixWorld == null)
				VixWorld = new Viewer3D();
			if (e.Args.Length > 0)
				VixWorld.ParseArgs(e.Args[0]);
			DispatcherUnhandledException += Application_DispatcherUnhandledException;
			TaskScheduler.UnobservedTaskException += Scheduler_DispatcherUnhandledException;
			Canvas3D.StartVixen(VixWorld);
			base.OnStartup(e);
		}

        protected override void OnExit(ExitEventArgs e)
        {
            base.OnExit(e);
        }

        private void Application_DispatcherUnhandledException(object sender, DispatcherUnhandledExceptionEventArgs e)
        {
			OnException(e.Exception.InnerException);
		}

        private void Scheduler_DispatcherUnhandledException(object sender, UnobservedTaskExceptionEventArgs e)
        {
            OnException(e.Exception);
        }

		private void OnException(Exception ex)
		{
			System.Windows.Markup.XamlParseException xamlexc = ex as System.Windows.Markup.XamlParseException;
			System.IO.FileFormatException ffexc = ex as System.IO.FileFormatException;
			string message = "ERROR: EXCEPTION " + ex.Message;

			if (xamlexc != null)
				message = string.Format("ERROR: Exiting because configuration file {0} is confusing at line {1}",
										xamlexc.BaseUri.AbsolutePath, xamlexc.LineNumber);
			else if (ffexc != null)
			{
				message = "ERROR: Exiting because configuration file is confusing " + ffexc.SourceUri;
				if (ffexc.InnerException != null)
					message += "\n" + ffexc.InnerException.Message;
			}
			else
				message += ex.StackTrace;
			MessageBoxResult alert = MessageBox.Show(message);
			Application.Current.Shutdown();
		}
	}

}
