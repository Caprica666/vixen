using System;
using System.Collections;
using System.IO;
using System.Text;
using Vixen;

namespace MagicMirrorWPF
{
	public class Selector
	{
		public Engine Root;
		public Appearance Hilite;
		protected Shape selectedItem = null;
		protected Appearance selectedAppear = null;
		public Selector()
		{
			PhongMaterial mtl = new PhongMaterial();

			Hilite = new Appearance();
			mtl.Diffuse = new Col4(0.6f, 0.8f, 0.1f);
			Hilite.Material = mtl;
			Hilite.Name = "selector.hilite";
		}
		public virtual void SetTarget(Model root) { }

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

		public override void SetTarget(Model root)
		{
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
			rightPicker.Buttons = MouseEvent.RIGHT;
			rightPicker.SetFlags((uint)SharedObj.DOEVENTS);
			rightPicker.PickShape = rightHandCursor;
			leftPicker = rightPicker.Clone() as RayPicker;
			leftPicker.Name = "magicmirror.leftpicker";
			leftPicker.PickShape = leftHandCursor;
			Root.Append(rightPicker);
			Root.Append(leftPicker);
			handTracker = new HandTracker();
			handTracker.ButtonMask = MouseEvent.RIGHT;
			handTracker.ZOffset = 0.5f;
			handTracker.RightHandCursor = rightHandCursor;
			handTracker.LeftHandCursor = leftHandCursor;
		}

		public override void SetTarget(Model root)
		{
			rightPicker.Target = root;
			leftPicker.Target = root;
		}

		public override void OnMouse(float x, float y, int buttons, float time)
		{
			handTracker.OnMouse(x, y, buttons, time);
		}

		public override void OnEvent(Event e)
		{
			handTracker.OnEvent(e);
		}

	}
}
