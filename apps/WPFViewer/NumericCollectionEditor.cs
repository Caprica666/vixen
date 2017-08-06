using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Collections.Specialized;
using Xceed.Wpf.Toolkit.PropertyGrid;
using Xceed.Wpf.Toolkit.PropertyGrid.Editors;

namespace Vixen.Viewer
{
	public class NumericCollectionEditor : TypeEditor<NumberList>
	{
	    protected override void SetControlProperties()
		{
		}

		protected override void SetValueDependencyProperty()
		{
			ValueProperty = NumberList.ItemsSourceProperty;
		}

		protected override void ResolveValueBinding( PropertyItem propertyItem )
		{
			System.Type pt = propertyItem.PropertyType;
			System.Type[] types = pt.GetGenericArguments();
			Editor.ItemsSourceType = pt;
			if (pt.IsGenericType)
				Editor.ItemType = types[0];
			else
			{
				Editor.ItemType = pt.GetElementType();
				if (Editor.ItemType == null)
				{
					Type[] ifaces = pt.GetInterfaces();
					Type itemType = ifaces.FirstOrDefault(
							(i) => i.IsGenericType && i.GetGenericTypeDefinition() == typeof(IEnumerable<>));
					if (itemType != null)
						types = itemType.GetGenericArguments();
					Editor.ItemType = types[0];
				}
			}
			base.ResolveValueBinding(propertyItem);
		}
	};

}
