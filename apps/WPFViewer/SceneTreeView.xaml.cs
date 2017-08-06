using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Controls;
using System.Windows;
using System.Reflection;
using System.ComponentModel;
using Vixen;
using Xceed.Wpf.Toolkit.PropertyGrid;
using Xceed.Wpf.Toolkit.PropertyGrid.Attributes;

namespace Vixen.Viewer
{
	public partial class SceneTreeView : TreeView
	{
		private SharedObj _root = null;
		private SharedObj _selectedObj = null;
		private Dictionary<FrameworkElement, SharedObj> _objDict = new Dictionary<FrameworkElement, SharedObj>();

		public static readonly RoutedEvent SelectedEvent = EventManager.RegisterRoutedEvent(
			   "Selected", RoutingStrategy.Bubble, typeof(RoutedEventHandler), typeof(SceneTreeView));

		public event RoutedEventHandler Selected
		{
			add { AddHandler(SelectedEvent, value); }
			remove { RemoveHandler(SelectedEvent, value); }
		}

		public SharedObj Root { get { return _root; } set { _root = value; Refresh(); } }

		public PropertyGrid Properties = null;

		public SceneTreeView()
			: base()
		{
			SetupPropertyaAttributes();
		}

		public void Refresh()
		{
			Items.Clear();
			_objDict.Clear();
			Items.Add(AddVixObj(_root));
		}

		public SharedObj FindVixObj(object source)
		{
			try
			{
				if (source.GetType() == typeof(TreeViewItem))
				{
					TreeViewItem item = source as TreeViewItem;
					source = item.Header;
				}
				Panel p = source as Panel;
				return _objDict[p];
			}
			catch (Exception) { }
			return null;
		}

		protected TreeViewItem AddVixObj(SharedObj obj)
		{
			TreeViewItem node = MakeItem(obj);

			if (node == null)
				return null;
			if (obj.IsClass((uint) SerialID.VX_Scene))
				MakeScene(node, (Scene)obj);
			if (obj.IsClass((uint) SerialID.VX_Model))
				MakeModel(node, (Model) obj);
			else if (obj.IsClass((uint) SerialID.VX_Engine))
				MakeEngine(node, (Engine) obj);
			return node;
		}

		protected void MakeScene(TreeViewItem item, Scene scene)
		{
			TreeViewItem child = MakeItem(scene.Models);
			if (child != null)
			{
				MakeModel(child, scene.Models);
				item.Items.Add(child);
			}
			child = MakeItem(scene.Engines);
			if (child != null)
			{
				MakeEngine(child, scene.Engines);
				item.Items.Add(child);
			}
		}

		protected void MakeShape(TreeViewItem item, Shape shape)
		{
			Geometry geo;
			Appearance app;

			if (shape == null)
				return;
			geo = shape.Geometry;
			if ((geo != null) && geo.IsClass((uint)SerialID.VX_TriMesh))
			{
				TreeViewItem child = MakeItem(geo);
				if (child != null)
				{
					MakeMesh(child, (TriMesh) geo.ConvertTo(SerialID.VX_TriMesh));
					item.Items.Add(child);
				}
			}
			app = shape.Appearance;
			if ((app != null) && (app.Name != null))
			{
				TreeViewItem child = MakeItem(app);
				if (child != null)
				{
					MakeAppearance(child, app);
					item.Items.Add(child);
				}
			}
		}

		protected void MakeMesh(TreeViewItem item, TriMesh mesh)
		{
			Label label = GetLabel(item);

			if (label != null)
			{
				string extra = String.Format(" {0} vertices, {1} faces", mesh.VertexCount, mesh.IndexCount / 3);
				label.Content += extra;
			}
			RemoveCheckBox(item);
		}

		protected void MakeAppearance(TreeViewItem item, Appearance appear)
		{
			Label label = GetLabel(item);

			if ((appear == null) || (label == null))
				return;
			RemoveCheckBox(item);
			for (int i = 0; i < appear.NumSamplers; ++i)
			{
				Sampler sampler = appear.GetSampler(i);
				Texture tex;

				if (sampler == null)
					continue;
				tex = sampler.Texture;
				if ((tex == null) || (tex.FileName == null))
					continue;
				label.Content += " " + tex.FileName;
			}
		}

		protected void MakeModel(TreeViewItem item, Model model)
		{
			Model g = model.First();
			model = model.ConvertTo(SerialID.VX_Shape) as Model;
			if (model != null)
				MakeShape(item, model as Shape);
			while (g != null)
			{
				TreeViewItem child = MakeItem(g);

				MakeModel(child, g);
				if (child != null)
					item.Items.Add(child);
				g = g.Next();
			}
		}

		protected void MakeEngine(TreeViewItem item, Engine eng)
		{
			Engine g = eng.First();
			while (g != null)
			{
				TreeViewItem child = MakeItem(g);

				MakeEngine(child, g);
				if (child != null)
					item.Items.Add(child);
				g = g.Next();
			}
		}

		protected TreeViewItem MakeItem(SharedObj obj)
		{
			TreeViewItem node;
			string desc;
			StackPanel panel;
			CheckBox check;
			Label label;

			if (obj == null)
				return null;
			desc = obj.ClassName;
			if (obj.Name != null)
				desc += " " + obj.Name;
			node = new TreeViewItem();
			node.Selected += OnSelect;
			panel = new StackPanel();
			panel.Orientation = System.Windows.Controls.Orientation.Horizontal;
			check = new CheckBox();
			check.Focusable = false;
			check.Checked += OnCheck;
			check.Unchecked += OnUncheck;
			panel.Children.Add(check);
			label = new Label();
			label.Content = desc;
			panel.Children.Add(label);
			label.VerticalAlignment = System.Windows.VerticalAlignment.Top;
			check.VerticalAlignment = System.Windows.VerticalAlignment.Center;
			node.Header = panel;
			_objDict[panel] = obj;
			if (obj.Active)
				check.IsChecked = true;
			else
				check.IsChecked = false;
			return node;
		}

		protected Label GetLabel(TreeViewItem node)
		{
			try
			{
				Panel panel = node.Header as Panel;
				foreach (UIElement e in panel.Children)
				{
					if (e.GetType() == typeof(Label))
						return (Label) e;
				}
			}
			catch (Exception)
			{
			}
			return null;
		}

		protected void RemoveCheckBox(TreeViewItem node)
		{
			try
			{
				Panel panel = node.Header as Panel;
				foreach (UIElement e in panel.Children)
				{
					if (e.GetType() == typeof(CheckBox))
					{
						panel.Children.Remove(e);
						return;
					}
				}
			}
			catch (Exception)
			{
			}
		}

		protected void OnCheck(object sender, RoutedEventArgs args)
		{
			try
			{
				CheckBox checkbox = sender as CheckBox;
				SharedObj vixobj = FindVixObj(checkbox.Parent as Panel);
				if (vixobj != null)
					vixobj.Active = true;
			}
			catch (Exception)
			{
			}
		}

		protected void OnUncheck(object sender, RoutedEventArgs args)
		{
			try
			{
				CheckBox checkbox = sender as CheckBox;
				SharedObj vixobj = FindVixObj(checkbox.Parent as Panel);
				if (vixobj != null)
					vixobj.Active = false;
			}
			catch (Exception)
			{
			}
		}

		public void OnZoom(object sender, RoutedEventArgs args)
		{
			try
			{
				Model mod = _selectedObj as Model;
				Box3 bounds = new Box3();
				Scene scene = SharedWorld.MainScene;
				mod.GetBound(bounds);
				scene.ZoomToBounds(bounds);
			}
			catch (Exception)
			{
			}
		}

		public void OnStart(object sender, RoutedEventArgs args)
		{
			try
			{
				Engine eng = _selectedObj as Engine;
				eng.Start();
			}
			catch (Exception)
			{
			}
		}

		public void OnStop(object sender, RoutedEventArgs args)
		{
			try
			{
				Engine eng = _selectedObj as Engine;
				eng.Stop();
			}
			catch (Exception)
			{
			}
		}

		public void OnSelect(object sender, RoutedEventArgs args)
		{
			try
			{
				TreeViewItem selected = sender as TreeViewItem;
				SharedObj obj = FindVixObj(sender);

				if (!selected.IsSelected)
					return;
				if (obj.IsClass((uint)SerialID.VX_Group))
				{
					_selectedObj = obj;
					if (Properties != null)
					{
						Properties.SelectedObject = obj;
						RaiseEvent(new RoutedEventArgs(PropertyGrid.SelectedObjectChangedEvent, Properties));
					}
					RaiseEvent(new RoutedEventArgs(SelectedEvent, obj));
				}
			}
			catch (Exception)
			{
			}
		}

		protected void SetupPropertyaAttributes()
		{
			Attribute[] tmp = { new ExpandableObjectAttribute() };
			Attribute[] tmp2 = { new EditorAttribute(typeof(NumericCollectionEditor), typeof(NumericCollectionEditor)) };

			TypeDescriptor.AddAttributes(typeof(Vec3), tmp);
			TypeDescriptor.AddAttributes(typeof(Vec2), tmp);
			TypeDescriptor.AddAttributes(typeof(Vec4), tmp);
			TypeDescriptor.AddAttributes(typeof(Quat), tmp);
			TypeDescriptor.AddAttributes(typeof(Col4), tmp);
			TypeDescriptor.AddAttributes(typeof(Box2), tmp);
			TypeDescriptor.AddAttributes(typeof(Box3), tmp);
			TypeDescriptor.AddAttributes(typeof(Model), tmp);
			TypeDescriptor.AddAttributes(typeof(Shape), tmp);
			TypeDescriptor.AddAttributes(typeof(Camera), tmp);
            TypeDescriptor.AddAttributes(typeof(Transformer), tmp);
			TypeDescriptor.AddAttributes(typeof(Interpolator), tmp);
			TypeDescriptor.AddAttributes(typeof(KeyFramer), tmp);
			TypeDescriptor.AddAttributes(typeof(Skin), tmp);
			TypeDescriptor.AddAttributes(typeof(Skeleton), tmp);
			TypeDescriptor.AddAttributes(typeof(Appearance), tmp);
			TypeDescriptor.AddAttributes(typeof(TriMesh), tmp);
			TypeDescriptor.AddAttributes(typeof(Sampler), tmp);
			TypeDescriptor.AddAttributes(typeof(Material), tmp);
			TypeDescriptor.AddAttributes(typeof(PhongMaterial), tmp);
			TypeDescriptor.AddAttributes(typeof(FloatArray), tmp2);
			TypeDescriptor.AddAttributes(typeof(IntArray), tmp);
		}

}

}
