using System;
using System.Text;
using System.IO;
using Vixen;

namespace MagicMirror
{
	/*!
	 * Implements a virtual puppet - a skinned character that can be posed,
	 * animated or dressed.
	 * 
	 * The 3D content for the puppet can come from Vixen binary files (.VIX)
	 * or Havok physics files (.HKT). This class connects the avatar to the
	 * current scene - both the scene graph and the simulation tree.
	 *
	 * @see Physics Skeleton
	 */
	public class Puppet
	{
		public bool             IsActive = false;
		public Skeleton         BodyPoser = null;
		public String			PhysicsOptions;
		public bool				IsHavokFile = false;
		public Model			BodyModel		{ get { return bodyModel; } }
		public bool				IsVisible		{ get { return isVisible; } }
		public String			Name			{ get { return baseName; } }
		public Skeleton			AnimSkeleton	{ get { return animSkel; } }
		public Model			RightHand = null;
		public Model			LeftHand = null;
		public Model			Head = null;
 
		protected bool			isVisible = false;
		protected int			physicsOpts = 0;
		protected ExtModel		bodyModel = null;
		protected Model         bodySkeleton = null;
		protected Engine        bodySim = null;
		protected Engine		hairSim = null;
		protected ExtModel		hairModel = null;
		protected Skeleton		hairSimSkel = null;
		protected Skeleton		hairAnimSkel = null;
		protected String		baseName = null;
		protected String		filename = null;
		protected Model		    rightHand = null;
		protected Model			leftHand = null;
		protected string		havokRig = null;
		protected string		animRig = null;
		protected Skeleton		animSkel = null;
		protected bool			usehavok = false;

		public Puppet(String name, String url, dynamic config)
		{
			baseName = name;
			filename = url;
			usehavok = false;
			if (url.ToLower().EndsWith(".hkt"))
				IsHavokFile = true;
			if (config != null)
			{
				if (config.rig != null)
					animRig = config.rig;
					if (config.havokrig != null)
					{
						havokRig = config.havokrig;
						usehavok = true;
					}
				if (config.physics != null)
					PhysicsOptions = config.physics;
			}
			bodyModel = new ExtModel();
			bodyModel.Options |= ExtModel.REMOVE;
			hairModel = new ExtModel();
			hairModel.Options |= ExtModel.REMOVE | ExtModel.AUTOLOAD;
			bodyModel.FileName = url;
		}

        public void MoveTo(Vec3 pos)
        {
            if (BodyPoser != null)
                BodyPoser.Position = pos;
        }

		public void Show()
		{
			isVisible = true;
			IsActive = true;
			if (bodySim != null)
				bodySim.Enable(Engine.ACTIVE);
			if (hairSim != null)
				hairSim.Enable(Engine.ACTIVE);
			if (hairAnimSkel != null)
				hairAnimSkel.Active = true;
			if (bodyModel != null)
				bodyModel.Active = true;
			if (hairModel != null)
				hairModel.Active = true;
		}

		public void Hide()
		{
			isVisible = false;
			if (bodyModel != null)
				bodyModel.Active = false;
			if (hairModel != null)
				hairModel.Active = false;
			if (bodySim != null)
				bodySim.Disable(Engine.ACTIVE);
			if (hairSim != null)
				hairSim.Disable(Engine.ACTIVE);
		}

		public void LoadHair(string url)
		{
			hairModel.FileName = url;
			bodyModel.Append(hairModel);
		}

		public Skeleton ConnectBody(Engine simroot)
		{
			if (simroot == null)
				return null;
			try
			{
				string filebase = Path.GetFileNameWithoutExtension(filename).ToLower() + ".";
				animSkel = (Skeleton)simroot.Find(filebase + animRig + ".skeleton", Group.FIND_DESCEND | Group.FIND_EXACT);

				if (usehavok)
				{
					Physics physroot = Physics.Get();
					Engine skelmapper;

					BodyPoser = (Skeleton)simroot.Find(filebase + havokRig + ".skeleton", Group.FIND_DESCEND | Group.FIND_EXACT);
					if ((PhysicsOptions != null))
					{
						if (PhysicsOptions.Contains("ynamic"))
							physicsOpts |= RagDoll.DYNAMIC;
						if (PhysicsOptions.Contains("ower"))
							physicsOpts |= RagDoll.POWERED;
						BodyPoser.SetBoneOptions(0, Skeleton.BONE_ANIMATE);
					}
					else
					{
						if (BodyPoser.GetBoneName(0).StartsWith("Proxy"))
						{
							BodyPoser.SetBoneOptions(BodyPoser.GetBoneIndex("Proxy_RightWrist"), Skeleton.BONE_LOCK_ROTATION);
							BodyPoser.SetBoneOptions(BodyPoser.GetBoneIndex("Proxy_RightAnkle"), Skeleton.BONE_LOCK_ROTATION);
							BodyPoser.SetBoneOptions(BodyPoser.GetBoneIndex("Proxy_LeftWrist"), Skeleton.BONE_LOCK_ROTATION);
							BodyPoser.SetBoneOptions(BodyPoser.GetBoneIndex("Proxy_LeftAnkle"), Skeleton.BONE_LOCK_ROTATION);
						}
					}
					BodyPoser.Control |= physicsOpts;
					BodyPoser.Active = false;
					skelmapper = (Engine) BodyPoser.Find(".mapper", Group.FIND_END | Group.FIND_CHILD);
					if (skelmapper != null)
						skelmapper.Active = true;
					if (physroot != null)
						bodySim = (Engine) physroot.Find(filebase + "cloth", Group.FIND_DESCEND | Group.FIND_EXACT);
					if (bodySim == null)
						bodySim = (Engine) physroot.Find(".skin", Group.FIND_DESCEND | Group.FIND_END);
					if (physicsOpts != 0)
						physroot.Active = true;
					bodySkeleton = (Model) bodyModel.Find(havokRig, Group.FIND_DESCEND | Group.FIND_END);
					if (bodySkeleton != null)
					{
						bodySkeleton.Active = false;
						RightHand = (Model) bodySkeleton.Find("RightWrist", Group.FIND_DESCEND | Group.FIND_END);
						LeftHand = (Model) bodySkeleton.Find("LeftWrist", Group.FIND_DESCEND | Group.FIND_END);
						Head = (Model) bodySkeleton.Find("Head", Group.FIND_DESCEND | Group.FIND_END);
					}
				}
				else
				{
					bodySim = (Engine) simroot.Find(".skin", Group.FIND_DESCEND | Group.FIND_END);
					BodyPoser = animSkel;
				}
 				Show();
			}
			catch (Exception)
			{
				// do nothing, simulation tree does not have SCAPESkin and BodyPoser
			}
			return BodyPoser;
		}

		public Skeleton ConnectHair(Engine simroot)
		{
			if (simroot == null)
				return null;
			try
			{
		   /*
			* find the hair simulation skeleton. If using Havok,
			* enable the skeleton mapper to map the rigid body motion
			* onto the hair skeleton.
			* 
			* Find the hair simulation root - either Havok cloth or a skin.
			*/
				System.String filebase = Path.GetFileNameWithoutExtension(hairModel.FileName).ToLower();
				String skelname = filebase + ".proxy_" + filebase;
				String ext = Path.GetExtension(hairModel.FileName).ToLower();

				if (usehavok && (ext != ".vix"))
				{
					RagDoll ragdoll;
					Physics physroot = Physics.Get();
					Engine skelmapper;
					Model hairBones = (Model) hairModel.Find(skelname, Group.FIND_DESCEND | Group.FIND_END);
					//if (hairBones != null)
					//	hairBones.Active = false;
					skelname += ".skeleton";
					ragdoll = (RagDoll) Scriptor.Find(skelname);
					hairSimSkel = ragdoll;
					hairSimSkel.Control |= RagDoll.DYNAMIC;
					hairSimSkel.SetBoneOptions(0, Skeleton.BONE_ANIMATE);
					skelmapper = (Engine)ragdoll.Find(".mapper", Group.FIND_END | Group.FIND_CHILD);
					if (skelmapper != null)
					{
						skelmapper.Active = true;
						hairAnimSkel = skelmapper.Target as Skeleton;
					}
					if (physroot != null)
					{
						hairSim = (Engine) physroot.Find(filebase + ".cloth", Group.FIND_DESCEND | Group.FIND_EXACT);
						if (hairSim == null)
							hairSim = (Engine) physroot.Find(".skin", Group.FIND_DESCEND | Group.FIND_END);
						physroot.Active = true;
					}
				}
				else
				{
					skelname = filebase + "." + filebase + ".skeleton";
					hairSimSkel = (Skeleton) Scriptor.Find(skelname);
					hairSim = (Engine)simroot.Find(".skin", Group.FIND_DESCEND | Group.FIND_END);
				}
				/*
				 * Find the head bone in the animation skeleton and connect the hair to it.
				 */
				if (animSkel != null)
				{
					int headindex = animSkel.GetBoneIndex("center_head");
					if (headindex > 0)
					{
						Transformer head = animSkel.GetBone(headindex);
						head.SetOptions(Transformer.WORLD);
						if (hairSimSkel != null)
							head.Target = animSkel.GetBone(0);
						else head.Target = hairModel;
					}
				}
				Show();
			}
			catch (Exception)
			{
				// do nothing
			}
			return hairSimSkel;
		}

		/*
		 * Restore the character's joints to their bind pose
		 */
		public void RestoreBindPose()
		{
			BodyPoser.RestoreBindPose();
		}

		/*
		 * Set the current pose of this character from the input pose.
		 */
		public void SetPose(Pose pose)
		{
			if (BodyPoser != null)
				BodyPoser.Pose = pose;
		}
	}
}
