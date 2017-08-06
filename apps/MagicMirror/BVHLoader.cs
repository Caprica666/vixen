using System;
using System.IO;
using System.Collections;
using System.Text.RegularExpressions;

using Vixen;

namespace MagicMirror
{
	public partial class Puppet
	{
/*
 * Reads a DAZ Studio PZ2 file with a pose in it and produces
 * a Vixen Pose object for this avatar's skeleton.
 * @param posename file name containing pose
 */
        public Pose ReadPose(System.String name)
        {
			ArrayList	bones = new ArrayList();
			ArrayList	positions = new ArrayList();
			int         level = 0;
            int         numbones = 0;
			Int32		numframes = 0;
			bool		parsing_motion = false;
			System.String bonename = "";
			Pose		bindpose = BodyPoser.BindPose;
            Pose		pose = new Pose(BodyPoser);
			float		x, y, z;

			pose.Copy(bindpose);
			try
            {
                /*
                 * Parse the DAZ Studio pose file and extract the rotations
                 * for each bone into a hash table
                 */
                using (StreamReader posefile = new StreamReader("pose/" + name + ".bvh"))
                {
                    System.String   line;
                    char[]          space = new char[] { ' ' };
 
                    while ((line = posefile.ReadLine().Trim()) != null)
                    {
                        string[]    words = line.Split(space);
                        string      opcode;
 
                        if (line == "")
                            continue;
					/*
					 * Parsing motion for each frame.
					 * Each line in the file contains the root joint position and rotations for all joints.
					 */
						if (parsing_motion)
						{
							int nbones = 0;
							if (words[0].StartsWith("Frame"))
								continue;
							x = float.Parse(words[0]);	// root bone position
							y = float.Parse(words[1]);
							z = float.Parse(words[2]);
							pose.SetPosition(new Vec3(x, y, z));
							for (int i = 3; i < words.Length; i += 3)
							{
								bonename = bones[nbones] as System.String;
								int boneindex = BodyPoser.GetBoneIndex(bonename);

								++nbones;
								if (boneindex >= 0)
								{
									Quat q, b;
									z = float.Parse(words[i]);	// Z, Y, X rotation angles
									y = float.Parse(words[i + 1]);
									x = float.Parse(words[i + 2]);
									if ((x == 0) && (y == 0) && (z == 0))
										continue;
									if (true)
									{
										q = new Quat(Model.ZAXIS, z * (float)Math.PI / 180);
										q *= new Quat(Model.YAXIS, x * (float)Math.PI / 180);
										q *= new Quat(Model.XAXIS, y * (float)Math.PI / 180);
									}
									else
									{
										q = new Quat(Model.ZAXIS, y * (float) Math.PI / 180);
										q *= new Quat(Model.YAXIS, x * (float) Math.PI / 180);
										q *= new Quat(Model.XAXIS, z * (float) Math.PI / 180);
									}
									q.Normalize();
									b = pose.GetLocalRotation(boneindex);
									q *= b;
									pose.SetLocalRotation(boneindex, q);
									SharedWorld.Trace(bonename + " " + q.x + " " + q.y + " " + q.z + " " + q.w + "\n");
								}
								else
									Canvas3D.LogError("Cannot find bone " + bonename + " in skeleton ");
	
							}
							break;
						}
					/*
					 * Parsing skeleton definition with joint names and positions.
					 */
						else
						{
							if (words.Length < 1)               // has an argument?
								continue;
							opcode = words[0];
							if ((opcode == "ROOT") ||			// found root bone?
								(opcode == "JOINT"))			// found any bone?
							{
								bonename = words[1];            // save the bone name
								++numbones;
							}
							else if (opcode == "OFFSET")       // bone position
							{
								float xpos = float.Parse(words[1]);
								float ypos = float.Parse(words[2]);
								float zpos = float.Parse(words[3]);

								if (bonename.Length > 0)		// save position for the bone
								{
									bones.Add(bonename);
									positions.Add(new Vec3(xpos, ypos, zpos));
									SharedWorld.Trace(bonename + " " + xpos + " " + ypos + " " + zpos + "\n");
								}
								bonename = "";
								continue;
							}
							else if (opcode == "MOTION")
							{
								parsing_motion = true;
								if (numbones != BodyPoser.NumBones)
									Canvas3D.LogError(System.String.Format("Pose with {0} bones does not match skeleton with {1} bones", numbones, BodyPoser.NumBones));
							}
						}
                    }
				}
                return pose;
             }
            catch (Exception ex)
            {
                Canvas3D.LogError(ex.Message + " Cannot read pose file " + name);
                return null;
            }
        }
	}
}
