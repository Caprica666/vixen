using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows.Media.Imaging;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.IO;
using System.Globalization;
using MagicMirror;

namespace MagicMirror.Tablet
{
	public partial class MainWindow : Window
	{
		protected ArrayList closet = null;
		protected Swiper swipeDetector = new Swiper();

		public MainWindow()
		{
			InitializeComponent();
		}

		protected override void OnContentRendered(EventArgs e)
		{
			base.OnContentRendered(e);
			swipeDetector.ButtonMask = Vixen.MouseEvent.LEFT;
			swipeDetector.SwipeEvent += avatarCanvas.OnSwipe;
			avatarCanvas.MouseTracker = swipeDetector;
			avatarCanvas.OnLoadCloset += OnLoadCloset;
			avatarCanvas.TransferDelegates();
			garmentSelector.SelectionChanged += new SelectionChangedEventHandler(OnImageSelected);
		}

		/*
		 * Called when an closet configuration file has finished loading.
		 */
		private void OnLoadCloset(String url, ArrayList closetitems)
		{
			ObservableCollection<string> icons = new ObservableCollection<string>();
			int i = 0;
			int selected = 0;

			closet = closetitems;
			foreach (dynamic d in closet)
			{
				if (d.iconfile != null)
				{
					String uri = d.iconfile.Replace("\\", "/");

					uri = avatarCanvas.GetMediaPath(uri);
					icons.Add(uri);
					if (avatarCanvas.ConfigOpts.defaultoutfit == d.name)
						selected = i;
					++i;
				}
			}
			garmentSelector.ItemsSource = icons;
			garmentSelector.SelectedIndex = selected;
		}

		/*
		 * Called when the user selects an image from the image selection bar
		 */
		protected void OnImageSelected(object sender, EventArgs e)
		{
			string selected = garmentSelector.SelectedItem as string;
			foreach (dynamic d in closet)
			{
				if (selected.EndsWith(d.iconfile))
					avatarCanvas.LoadGarment(d.name, d.filename);
			}
		}
	}

	public class PathToImageConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
		{
			string path = value as string;
			if (path != null)
			{
				BitmapImage image = new BitmapImage();
				image.BeginInit();
				image.UriSource = new Uri("file://" + path);
				image.EndInit();
				return image;
			}
			else
				return null;
		}

		public object ConvertBack(object value, Type targetType,
									object parameter, CultureInfo culture)
		{
			throw new NotImplementedException();
		}
	}

}

