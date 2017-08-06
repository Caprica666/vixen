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
	public partial class NumberList : ListBox
	{

	#region Properties

	#region IsOpen

	public static readonly DependencyProperty IsOpenProperty = DependencyProperty.Register( "IsOpen", typeof( bool ), typeof( NumberList ), new UIPropertyMetadata( false, OnIsOpenChanged ) );
	public bool IsOpen
	{
		get { return ( bool )GetValue( IsOpenProperty ); }
		set { SetValue( IsOpenProperty, value );	}
	}

	private static void OnIsOpenChanged( DependencyObject o, DependencyPropertyChangedEventArgs e )
	{
		NumberList editor = o as NumberList;
		if( editor != null )
		editor.OnIsOpenChanged( ( bool )e.OldValue, ( bool )e.NewValue );
	}

	protected virtual void OnIsOpenChanged( bool oldValue, bool newValue )
	{ }

	#endregion //IsOpen

	#region ItemsSource

	private static void OnItemsSourceChanged( DependencyObject o, DependencyPropertyChangedEventArgs e )
	{
		NumberList editor = o as NumberList;
		if( editor != null )
			editor.OnItemsSourceChanged((IEnumerable) e.OldValue, (IEnumerable) e.NewValue);
	}

	protected override void OnItemsSourceChanged( IEnumerable oldValue, IEnumerable newValue )
	{
		if( newValue == null )
			return;
		if( ItemsSourceType == null )
			ItemsSourceType = newValue.GetType();
		if( ItemType == null )
			ItemType = newValue.GetType().GetGenericArguments()[ 0 ];
	}

	protected override void OnItemsChanged(NotifyCollectionChangedEventArgs e)
	{

	}

	#endregion //ItemsSource

	public static readonly DependencyProperty ItemsSourceTypeProperty = DependencyProperty.Register("ItemsSourceType", typeof(Type), typeof(NumberList), new UIPropertyMetadata(null));
	public Type ItemsSourceType
	{
		get { return ( Type )GetValue( ItemsSourceTypeProperty ); }
		set { SetValue( ItemsSourceTypeProperty, value ); }
	}

	public static readonly DependencyProperty ItemTypeProperty = DependencyProperty.Register("ItemType", typeof(Type), typeof(NumberList), new UIPropertyMetadata(null));
	public Type ItemType
	{
		get { return ( Type )GetValue( ItemTypeProperty ); }
		set { SetValue( ItemTypeProperty, value ); }
	}

	#endregion //Properties

	#region Constructors

	static NumberList()
	{
		DefaultStyleKeyProperty.OverrideMetadata(typeof(NumberList), new FrameworkPropertyMetadata(typeof(NumberList)));
	}

	public NumberList()
	{

	}

	#endregion //Constructors

	#region Methods

    #endregion //Methods
  }
}
