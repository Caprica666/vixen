using System;
using System.Collections;
using System.IO;
using System.Text;
using Vixen;
using MagicMirror;

namespace MagicMirror.Kiosk
{
	/*!
	 * @class Selector
	 * Handles selection of garments using a 3D UI.
	 * The 3D UI scene contains a root object named "selectable" with
	 * a child for each item that can be selected. The children are
	 * named "icon_item1", "icon_item2", etc.
	 * These generic names are changed to match the names of the
	 * outfits in the current closet. This way it is easy to determine
	 * which clothing item to select based on the name of the 3D item picked.
	 */
	public class Selector
	{
		public Model		UIRoot;			//! root of 3D UI scene
		public Engine		Root;			//! root of simulation tree for collision engines
		public Appearance	Hilite;			//! apperance of hilited object
		public Closet Clothing				//! current closet used for selection
		{
			get { return closet; }
			set { closet = value; if ((UIRoot != null) && (closet != null)) AttachCloset(); }
		}
		protected Shape selectedItem = null;
		protected Appearance selectedAppear = null;
		protected Closet closet = null;


		public Selector()
		{
			PhongMaterial mtl = new PhongMaterial();

			Hilite = new Appearance();
			mtl.Diffuse = new Col4(0.6f, 0.8f, 0.1f);
			Hilite.Material = mtl;
			Hilite.Name = "selector.hilite";
		}

		/*!
		 * Customizes the 3D user interface to the current closet.
		 * The 3D UI has a 3D shape corresponding to each item that can
		 * be selected. The name and texture of these shapes are
		 * changed to correspond to individual garments in the closet.
		 * If there are more clothes in the closet than items in the UI,
		 * some clothing won't be selectable.
		 *
		 * The 3D art must adhere to the following structure:
		 * Model			"uifile.selectable"
		 *		Shape		"uifile.icon_item1"		(background)
		 *			Shape	"uifile.plane_item1"	(thumbnail with texture)
		 *		Shape		"uifile.icon_item2"		(background)
		 *			Shape	"uifile.plane_item2"	(thumbnail with texture)
		 *	...
		 */
		public virtual void AttachCloset()
		{
			Model cur = UIRoot.First();
			foreach (dynamic g in Clothing)
			{
				if (cur == null)
					return;
				try
				{
					string name = cur.Name;
					int i = name.IndexOf(".icon_");
					if (i > 0)
					{
						Shape plane = cur.First() as Shape;
						name = name.Substring(0, i + 6) + g.Name;
						cur.Name = name;
						name = plane.Name;
						i = name.IndexOf(".plane_");
						if (i > 0)
						{
							plane.Name = name.Substring(0, i + 7) + g.Name;
							Sampler sampler = plane.Appearance.GetSampler(0);
							sampler.Texture.Load(g.IconFile);
							SharedWorld.Trace(plane.Name + " replacing texture " + g.IconFile);
						}
					}
				}
				catch (Exception ex)
				{
					SharedWorld.LogError("AttachCloset " + ex.Message);
				}
				cur = cur.Next();
			}
		}

		/*!
		 * Establishes the root of the 3D UI.
		 * The 3D art must adhere to the following structure:
		 * Model			"uifile.selectable"
		 *		Shape		"uifile.icon_item1"		(background)
		 *			Shape	"uifile.plane_item1"	(thumbnail with texture)
		 *		Shape		"uifile.icon_item2"		(background)
		 *			Shape	"uifile.plane_item2"	(thumbnail with texture)
		 *	...
		 */
		public virtual void SetTarget(Model root)
		{
			UIRoot = root;
			if ((closet != null) && (root != null))
				AttachCloset();
		}

		/*!
		 * Performs selection highlighting for the 3D shape picked.
		 */
		public virtual bool Select(Shape picked)
		{
			System.String name = picked.Name;
			if (selectedItem != null)	// deselect the current item
			{							// by puttings it's former appearance back
				if ((picked != null) && (selectedItem.Name == name))
					return false;
				SharedWorld.Trace(selectedItem.Name + " unselected\n");
				selectedItem.Appearance = selectedAppear;
				selectedItem = null;	// now nothing is selected
				selectedAppear = null;
			}
			selectedItem = picked;
			if (picked != null)
			{
				SharedWorld.Trace(name + " picked\n");
				selectedAppear = selectedItem.Appearance;
				selectedItem.Appearance = Hilite;
				return true;
			}
			return false;
		}

		public virtual void OnMouse(float x, float y, int buttons, float time)
		{
		}

		public virtual void OnEvent(Event e) { }
	}

	/*!
	 * @class TriggerSelector
	 * Handles selection of garments using simple collision detection.
	 * The 3D UI scene contains a root object named "selectable" with
	 * a child for each item that can be selected. The children are
	 * named "icon_item1", "icon_item2", etc.
	 * For each UI item, a Trigger engine is generated which will
	 * raise an ENTER event when any object enters the bounds
	 * of the UI item.
	 */
	public class TriggerSelector : Selector
	{
		public TriggerSelector(int maxitems)
			: base()
		{
			Root = new Engine();
			Root.Name = "magicmirror.triggers";
			for (int i = 0; i < maxitems; ++i)
			{
				Trigger trigger = new Trigger();
				trigger.Name = "magicmirror.trigger." + i;
				trigger.Options = Trigger.BOX;
				Root.Append(trigger);
			}
		}

		/*!
		 * Establishes the root of the 3D UI
		 * A Trigger engine is created for each UI item.
		 */
		public override void SetTarget(Model root)
		{
			base.SetTarget(root);
			Model pickme = root.First();
			Engine trigger = Root.First();
			while ((pickme != null) && (trigger != null))
			{
				if (trigger.IsClass((uint)SerialID.VX_Trigger))
					trigger.Target = pickme;
				pickme = pickme.Next();
				trigger = trigger.Next();
			}
		}

		/*!
		 * Clears the collider list.
		 */
		public void ClearColliders()
		{
			try
			{
				Trigger trigger = (Trigger)Root.First();
				while (trigger != null)
				{
					trigger.Colliders = null;
					trigger = trigger.Next() as Trigger;
				}
			}
			catch (Exception) { };
		}

		/*!
		 * Adds an object to the collider list for the UI items.
		 * Whenever this object enters the bounds of a UI item,
		 * the ENTER event will be raised.
		 */
		public void AddCollider(Model collider)
		{
			try
			{
				Trigger trigger = (Trigger)Root.First();
				while (trigger != null)
				{
					trigger.AddCollider(collider);
					trigger = trigger.Next() as Trigger;
				}
			}
			catch (Exception) { };
		}
	}

	/*!
	 * @class PickSelector
	 * Handles selection of garments using the mouse and ray casting.
	 * The 3D UI scene contains a root object named "selectable" with
	 * a child for each item that can be selected. The children are
	 * named "icon_item1", "icon_item2", etc.
	 * When the mouse enters one of the 3D UI items, a PICK event is raised.
	 */
	public class PickSelector : Selector
	{
		protected RayPicker leftPicker;
		protected RayPicker rightPicker;
		protected HandTracker handTracker;

		public Vec3 WorldPos { get { return handTracker.WorldPos; } }

		public PickSelector(Model leftHandCursor, Model rightHandCursor)
			: base()
		{

			Root = new Engine();
			rightPicker = new RayPicker();
			rightPicker.Name = "magicmirror.rightpicker";
			rightPicker.Options = Picker.MESH;
			rightPicker.Buttons = MouseEvent.SHIFT;
			rightPicker.SetFlags((uint)SharedObj.DOEVENTS);
			rightPicker.PickShape = rightHandCursor;
			leftPicker = rightPicker.Clone() as RayPicker;
			leftPicker.Name = "magicmirror.leftpicker";
			leftPicker.PickShape = leftHandCursor;
			Root.Append(rightPicker);
			Root.Append(leftPicker);
			if (rightHandCursor != null)
			{
				handTracker = new HandTracker();
				handTracker.ButtonMask = MouseEvent.SHIFT;
				handTracker.ZOffset = 0.5f;
				handTracker.RightHandCursor = rightHandCursor;
				handTracker.LeftHandCursor = leftHandCursor;
			}
		}

		public override void SetTarget(Model root)
		{
			rightPicker.Target = root;
			leftPicker.Target = root;
			base.SetTarget(root);
		}

		public override void OnMouse(float x, float y, int buttons, float time)
		{
			if (handTracker != null)
				handTracker.OnMouse(x, y, buttons, time);
		}

		public override void OnEvent(Event e)
		{
			if (handTracker != null)
				handTracker.OnEvent(e);
		}

	}
}
