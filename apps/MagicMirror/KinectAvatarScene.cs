using System;
using System.Collections;
using System.IO;
using System.Text;
using Vixen;

namespace MagicMirror
{
	/*!
	 * @class KinectAvatarScene 
	 * Implements an animatable 3D avatar that you can dress in different outfits
	 * which tracks your body movements using the Kinect.
	 * Content files for the background, avatar, clothing and animations can be separately loaded.
	 * Events are produced when the user enters or leaves the area and when the user turns.
	 *
	 * @see AvatarScene KioskScene Avatar
	 */
	public class KinectAvatarScene : AvatarScene
	{
		protected float		blendtime = 1.0f;
		protected KinectTracker masterKinect = null;
		protected int		kinectUserID =-1;
		protected PoseMapper poseMapper = null;
		protected Shape		cameraOutput = null;
		protected bool		enableAnimation = false;
		protected Trigger	frontAnimTrigger = new Trigger();
		public KinectTracker Kinect { get { return masterKinect; } }

		#region BodyTrackEvents interface
		public delegate void OnSensorInitHandler(String s);
		public event OnSensorInitHandler SensorInitEvent;
		public delegate void OnNewUserHandler(String s, int userid);
		public event OnNewUserHandler NewUserEvent;
		public delegate void OnStartTrackHandler(String s, int userid);
		public event OnStartTrackHandler StartTrackEvent;
		public delegate void OnStopTrackHandler(String s, int userid);
		public event OnStopTrackHandler StopTrackEvent;
		public delegate void OnPauseTrackHandler(String s, int userid);
		public event OnPauseTrackHandler PauseTrackEvent;
		public delegate void OnUserTurnHandler(String s, int userid, Vec3 pos);
		public event OnUserTurnHandler UserTurnEvent;
		#endregion

		public KinectAvatarScene(System.String kinect_file)
			: base()
		{
			masterKinect = new KinectTracker();
			masterKinect.Active = false;
			if (!masterKinect.Configure(kinect_file))
				throw new FileNotFoundException(kinect_file + " cannot open Kinect data file");
		}

		public KinectAvatarScene()
			: base()
		{
		}

		protected override Engine MakeEngines()
		{
			Engine simroot = base.MakeEngines();
			if (masterKinect != null)
				simroot.Append(masterKinect);
			frontAnimTrigger.Name = "magicmirror.frontanim.trigger";
			frontAnimTrigger.Active = false;
			simroot.Append(frontAnimTrigger);
			return simRoot;
		}

		/*!
		 * @param ev	Vixen event to handle.
		 * @param world	current world
		 * Handles vixen events which come from the body tracker.
		 * This function also calls event handlers for BodyTrackEvents.
		 */
		public override void OnVixen(Event ev, SharedWorld world)
		{
			int code = ev.Code;

			if (code == Event.TRACK)
			{
				TrackEvent kev = (TrackEvent)ev;

				if (kev.Type == TrackEvent.NEW_USER)
					OnNewUser(kev.UserID);
				else if (kev.Type == TrackEvent.START_BODY_TRACK)
					OnStartTrack(kev.UserID);
				else if (kev.Type == TrackEvent.PAUSE_BODY_TRACK)
					OnPauseTrack(kev.UserID);
				else if (kev.Type == TrackEvent.STOP_BODY_TRACK)
					OnStopTrack(kev.UserID);
				else if (kev.Type == TrackEvent.INIT_DONE)
					OnSensorInit();
				else if (kev.Type == TrackEvent.USER_TURN)
					OnUserTurn(kev.UserID, kev.Position);
			}
			else if (code == Event.ENTER && enableAnimation)
				try
				{
					TriggerEvent te = ev as TriggerEvent;
					if (te.Sender .SameAs(frontAnimTrigger))
						RandomAnimation("front", blendtime);
				}
				catch (Exception) { }

			base.OnVixen(ev, world);
		}

		/*!
		 * Called when when the body tracking sensor has initialized.
		 * Invokes the SensorInitEvent handler.
		 */
		protected virtual void OnSensorInit()
		{
			Console.WriteLine("Sensor Initialized");
			if ((SensorInitEvent != null) && (masterKinect != null))
				SensorInitEvent(masterKinect.Name);
		}

		/*!
		 * @param userid	sensor ID of new user
		 * Called when the sensor detects a new user walking into the scene.
		 * Invokes the NewUserEvent handler.
		 */
		protected virtual void OnNewUser(int userid)
		{
			Console.WriteLine("New User " + userid);
			if ((NewUserEvent != null) && (masterKinect != null))
				NewUserEvent(masterKinect.Name, userid);
		}

		/*!
		 * @param userid	sensor ID of user
		 * Called when the sensor starts tracking a user.
		 * Invokes the StartTrackEvent handler.
		 */
		protected virtual void OnStartTrack(int userid)
		{
			Console.WriteLine("Start Track " + userid);
			kinectUserID = userid;
			currentAvatar.BodyPoser.Active = true;
			currentAvatar.Show();
			PauseAnimation(null);
			//cameraOutput.Active = false;
			if ((config.physics != null) &&
				(config.physics != "none") &&
				(config.physics != ""))
				physics.Active = true;
			if ((StartTrackEvent != null) && (masterKinect != null))
				StartTrackEvent(masterKinect.Name, userid);
		}

		/*!
		 * @param userid	sensor ID of user
		 * Called when the sensor pauses tracking the user.
		 * Invokes the PauseTrackEvent handler.
		 */
		protected virtual void OnPauseTrack(int userid)
		{
			Console.WriteLine("Pause Track " + userid);
			if (userid == kinectUserID)
			{
				kinectUserID = -1;
				currentAvatar.BodyPoser.Active = false;
				if (enableAnimation)
					RandomAnimation("idle", blendtime);
			}
			if ((PauseTrackEvent != null) && (masterKinect != null))
				PauseTrackEvent(masterKinect.Name, userid);
		}

		/*!
		 * @param userid	sensor ID of user
		 * Called when the sensor stops tracking a user.
		 * Invokes the StopTrackEvent handler.
		 */
		protected virtual void OnStopTrack(int userid)
		{
			Console.WriteLine("Stop Track " + userid);
			if (userid == kinectUserID)
			{
				kinectUserID = -1;
				currentAvatar.RestoreBindPose();
				if (poseMapper != null)
					poseMapper.Source.RestoreBindPose();
				if (enableAnimation)
					RandomAnimation("idle", blendtime);
			}
			if ((StopTrackEvent != null) && (masterKinect != null))
				StopTrackEvent(masterKinect.Name, userid);
		}

		/*!
		 * @param name		name of Vixen engine for sensor
		 * @param userid	sensor ID of user
		 * @param pos		position of users chest
		 * Called when the user turns away from the sensor.\
		 * Invokes the UserTurnEvent handler.
		 */
		protected virtual void OnUserTurn(int userid, Vec3 pos)
		{
			Console.WriteLine("User Turn " + userid);
			if ((UserTurnEvent != null) && (masterKinect != null) && enableAnimation)
				UserTurnEvent(masterKinect.Name, userid, pos);
		}

		protected override void OnStopAnim(SharedObj sender)
		{
			bool restart = (nowPlaying != null) && nowPlaying.SameAs(sender);
			base.OnStopAnim(sender);
			if (!restart)
				return;
			if (kinectUserID >= 0)
			{
				currentAvatar.BodyPoser.Active = true;
				return;
			}
			if (enableAnimation)
				RandomAnimation("idle", 0);
		}

		protected override void OnSceneChange(SharedWorld world, Scene scene)
		{
			bool firstTime = !sceneLoaded;
			base.OnSceneChange(world, scene);
			if (firstTime && (masterKinect != null))
			{
				masterKinect.SetFlags((uint)SharedObj.DOEVENTS);
				masterKinect.Options = (int) BodyTracker.TRACK_FRONT_ONLY | (int) BodyTracker.TRACK_SKELETON;
				world.Observe(Event.TRACK);
			}
		}

		protected override Skeleton ConnectAvatar(SharedWorld world, Scene scene)
		{
			Skeleton skeleton = base.ConnectAvatar(world, scene);
			dynamic anims = null;

			if (config != null)
				anims = config.animation;
			if (frontAnimTrigger != null)
				frontAnimTrigger.AddCollider(currentAvatar.Head);
			if (masterKinect == null)
				return skeleton;

			try
			{
				Box3 bodysize = new Box3();
				Camera cam = scene.Camera;

				poseMapper = masterKinect.PoseMapper;
				if (poseMapper != null)
				{
					Skeleton source = poseMapper.Source;
					string havokrig = "Proxy_Waist";
					string rig = "genesis";

					if ((config != null) && (config.havokrig != null))
						havokrig = config.havokrig;
					if ((config != null) && (config.rig != null))
						rig = config.rig;
					poseMapper.Target = skeleton;
					if (!skeleton.GetBoneName(0).Contains(havokrig))
						poseMapper.SelectBoneMap(rig);
					else
						poseMapper.SelectBoneMap("default");
					if (source != null)
					{
						Vec3 waistpos = source.Pose.GetWorldPosition(0);
						poseMapper.MapWorldToSource();
						poseMapper.Source.BindPose = source.Pose;
						masterKinect.WaistOffset = waistpos;
					}
					else
						SharedWorld.LogError("MagicMirror: PoseMapper does not have source skeleton");
				}
				masterKinect.Active = true;
			}
			catch (Exception ex)
			{
				SharedWorld.LogError("MagicMirror: problem connecting Kinect " + ex.Message);
			}
			return skeleton;
		}

		public override void OnSceneLoad(SharedWorld world, Scene scene, String scenefile)
		{
			if (!sceneLoaded &&
				(cameraOutput == null) &&
				(scenefile == interiorRoot.FileName))
			{
				if (masterKinect != null)
				{
					cameraOutput = MakeCameraOutputShape(scene.Camera, true);
					sceneRoot.Append(cameraOutput);
					cameraOutput.Active = true;
					SharedWorld.Trace("Scene: " + sceneRoot.Name + " -> child " + cameraOutput.Name + "\n");
				}
				if (frontAnimTrigger != null)
				{
					Box3 b = new Box3(-2.0f, 0.0f, -0.05f, 2.0f, 3.0f, 2.0f);

					frontAnimTrigger.SetGeoBox(b);
					frontAnimTrigger.Options = Trigger.BOX;
					frontAnimTrigger.SetFlags((uint)SharedObj.DOEVENTS);
					frontAnimTrigger.Active = true;
				}
			}
			base.OnSceneLoad(world, scene, scenefile);
		}

		/*
		 * Make the shape for the Kinect depth camera output
		 */
		protected Shape MakeCameraOutputShape(Camera cam, bool iscolor)
		{
			Shape shape = new Shape();
			Appearance app = new Appearance();
			PhongMaterial mtl = new PhongMaterial(new Col4(1.0f, 1.0f, 1.0f, 0.7f));
			Sampler sampler = new Sampler();
			TriMesh mesh = new TriMesh("position float 3, normal float 3, texcoord float 2");
			Vec2 videosize = new Vec2(640, 480);
			Vec2 texsize = new Vec2(1024, 512);
			Box3 vvol = cam.ViewVol;
			Vec3 campos = cam.Translation;
			float camdist = campos.Length;		// distance of camera from body
			float backdist = camdist + vvol.max.z / 6.0f;
			float h = backdist * vvol.Height / vvol.min.z;
			float w = h;

			backdist -= camdist;
			if (Kinect != null)
			{
				Texture tex = new Texture();
				sampler.Texture = tex;
				if (iscolor)
				{
					tex.Name = "magicmirror.kinect.colorimage";
					Kinect.ColorTexture = tex;
				}
				else
				{
					tex.Name = "magicmirror.kinect.depthimage";
					Kinect.DepthTexture = tex;
				}
			}
			sampler.Name = "diffuse";
			sampler.Set(Sampler.TEXTUREOP, Sampler.DIFFUSE);
			sampler.Set(Sampler.TEXCOORD, 0);
			app.Set(Appearance.CULLING, 1);
			app.Set(Appearance.LIGHTING, 1);
			app.Set(Appearance.TRANSPARENCY, 1);
			app.SetSampler(0, sampler);
			app.Material = mtl;
			app.Name = "cameraoutput";
			shape.Appearance = app;
			GeoUtil.Rect(mesh, h * videosize.x / videosize.y, h, videosize.x / texsize.x, videosize.y / texsize.y);
			shape.Geometry = mesh;
			shape.Name = "magicmirror.kinect.camerashape";
			shape.Translate(0, h / 2, -backdist);
			return shape;
		}

	}

}
