using System;
using System.Collections;
using System.IO;
using System.Text;
using Vixen;
using MagicMirror;

namespace MagicMirror.Kiosk
{
	/*!
	 * 3D display for virtual dressing room kiosk.
	 * Input properties specify initial configuration -
	 * background, avatar, clothing, animations, UI.
	 * The avatar uses the Kinect to track the users body movements.
	 * The user interface to select clothing is gestural - you move
	 * that avatar's hands and she selects items in the 3D scene
	 * using 3D collision detection. If no Kinect configuration
	 * file is provided, outfits can be selected with the mouse.
	 */
	public class KioskScene : KinectAvatarScene
	{
		protected Hashtable     poses = new Hashtable();
		protected ExtModel      uiRoot;
		protected Model         rightHandCursor = null;
		protected Model         leftHandCursor = null;
		protected Appearance    selectedAppear = null;
		protected Shape         selectedItem = null;
		protected System.String defaultUIFile = "ui/ui_4items.vix";
		protected Selector		outfitSelector;

		/*!
		 * Creates a KioskScene which uses the Kinect to track
		 * the users movements and apply them to the avatar.
		 * Outfits are selected with the avatars hands.
		 */
		public KioskScene(System.String kinect_file)
			: base(kinect_file)
		{
			outfitSelector = new TriggerSelector(4);
		}

		/*!
		 * Creates a KioskScene which animates the avatar and
		 * selects outfits with the mouse.
		 */
		public KioskScene()
			: base()
		{
		}

		/*
		 * Loads the user interface file if provided.
		 */
		public override void Initialize()
		{
			base.Initialize();
			if (config.ui != null)
				uiRoot.FileName = config.ui;
		}

		/*
		 * Creates initial simulation tree. If the Kinect was not
		 * requested, the PickSelector is used to select
		 * outfits with the mouse.
		 */
		protected override Engine MakeEngines()
		{
			Engine simroot = base.MakeEngines();
			if (outfitSelector == null)
				outfitSelector = new PickSelector(rightHandCursor, leftHandCursor);
			simRoot.Append(outfitSelector.Root);
			return simRoot;
		}

		/*
		 * Creates initial scene graph. Adds a placeholder object for the 3D UI.
		 */
		protected override Model MakeModels()
		{
			Model sceneRoot = base.MakeModels();
			uiRoot = new ExtModel();
			uiRoot.Options |= ExtModel.REMOVE;
			sceneRoot.Append(uiRoot);
			return sceneRoot;
		}

		/*
		 * Vixen event handler. Responds to selection events:
		 *	Event.SELECT	shape picked in 3D scene
		 *	Event.DESELECT	nothing picked in 3D scene
		 *	Event.ENTER		3D object enters trigger area
		 *	Calls the OnPick function to handle the selection.
		 */
		public override void OnVixen(Event ev, SharedWorld world)
		{
			int code = ev.Code;

			if (code == Event.SELECT)
				try
				{
					PickEvent pe = ev as PickEvent;
					Shape picked = pe.Target as Shape;
					OnPick(picked);
				}
				catch (Exception) { }
			else if (code == Event.ENTER)
				try
				{
					TriggerEvent te = ev as TriggerEvent;
					Shape picked = te.Target as Shape;
					OnPick(picked);
				}
				catch (Exception) { }
			else if (code == Event.DESELECT)
				OnPick(null);
			base.OnVixen(ev, world);
		}

		/*
		 * Event.LOADSCENE handler - comes here when 3D scene loaded.
		 * If a user interface scene is loaded (name contains "ui_")
		 * bind the UI scene to the Selector for picking outfits.
		 */
		public override void OnSceneLoad(SharedWorld world, Scene scene, String scenefile)
		{
			System.String name = scene.Name.ToLower();

			if (name.Contains("ui_"))
			{
				Model iconRoot = uiRoot.Find(".selectable", Group.FIND_DESCEND | Group.FIND_END) as Model;
				SharedWorld.Trace("Loaded " + name);
				if (iconRoot != null)
				{
					RaiseSceneLoadEvent(name, scenefile);
					if (outfitSelector != null)
						outfitSelector.SetTarget(iconRoot);
					return;
				}
			}
			base.OnSceneLoad(world, scene, scenefile);
		}

		/*
		 * Load a new closet from the given URL.
		 * After loading, bind the closet contents to the Selector.
		 * This will change the 3D UI to use the thumbnail images
		 * in the closet for each outfit.
		 */
		public override void LoadCloset(String url)
		{
			base.LoadCloset(url);
			if ((closet != null) && (outfitSelector != null))
				outfitSelector.Clothing = closet;
		}

		/*
		 * Come here when a 3D shape is picked or a trigger engine
		 * registers a collision. If the picked item is part of
		 * the UI, it's name will start with "plane_" or "icon_"
		 * followed by the garment name. In this case the 3D item
		 * is hilited and the SelectGarmentEvent is raised.
		 */
		public virtual void OnPick(Shape picked)
		{
			if (picked == null)
				return;
			if (closet == null)
				return;
			string name = picked.Name;
			if (name.Contains(".plane_"))
			{
				picked = (Shape) picked.Parent();
				name = picked.Name;
			}
			if (outfitSelector.Select(picked))
			{
				int i = name.IndexOf(".icon_");
				RaiseSelectGarmentEvent(name.Substring(i + 6));
			}
		}

		protected override Skeleton ConnectAvatar(SharedWorld world, Scene scene)
		{
			Skeleton skeleton = base.ConnectAvatar(world, scene);

			if ((skeleton != null) &&
				(outfitSelector != null) &&
				(typeof(TriggerSelector).IsAssignableFrom(outfitSelector.GetType())))
			{
				TriggerSelector tmp = outfitSelector as TriggerSelector;

				tmp.AddCollider(currentAvatar.RightHand);
				tmp.AddCollider(currentAvatar.LeftHand);
			}
			return skeleton;
		}
	
		/*
		 * Comes here when the user turns their body to the left or right.
		 * Stops body tracking and plays an animation.
		 */
		protected override void OnUserTurn(int userid, Vec3 torsoz)
		{
			if (!currentAvatar.BodyPoser.Active)
				return;
			base.OnUserTurn(userid, torsoz);
			if (!enableAnimation)
				return;
			currentAvatar.BodyPoser.Active = false;
			if (torsoz.x > 0)
				RandomAnimation("left", blendtime);
			else
				RandomAnimation("right", blendtime);
		}

		/*
		 * Come here on any mouse event.
		 * If not using the Kinect, this function hanstdles the mouse events
		 * to animate the avatar and select outfits.
		 *	CONTROL will move the avatars waist to the mouse location (Z = 0)
		 *	SHIFT	will start the "turn left" or "turn right" animation
		 */
		public void OnMouse(float x, float y, int buttons, float time_in_sec)
		{
            if (outfitSelector != null)
            {
                outfitSelector.OnMouse(x, y, buttons, time_in_sec);
				if (typeof(PickSelector).IsAssignableFrom(outfitSelector.GetType()))
				{
					if ((buttons & MouseEvent.CONTROL) == MouseEvent.CONTROL)
					{
						Vec3 worldpos = ((PickSelector)outfitSelector).WorldPos;
						currentAvatar.MoveTo(worldpos);
					}
					if ((buttons & (MouseEvent.SHIFT | MouseEvent.RIGHT)) == (MouseEvent.SHIFT | MouseEvent.RIGHT))
					{
						if (y < 300)
							RandomAnimation("front", 0);
						else if (x < 400)
							RandomAnimation("right", 0);
						else
							RandomAnimation("left", 0);
					}
				}
			}
 		}
	}
}
