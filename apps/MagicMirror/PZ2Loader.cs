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
            Hashtable   poseinfo = new Hashtable();
            int         level = 0;
            int         numbones = 0;
            int         offset = -1;

            try
            {
                /*
                 * Parse the DAZ Studio pose file and extract the rotations
                 * for each bone into a hash table
                 */
                using (StreamReader posefile = new StreamReader("pose/" + name + ".pz2"))
                {
                    System.String   line;
                    System.String   bonename = "";
                    char[]          space = new char[] { ' ' };
                    float			xrot = 0;
                    float			yrot = 0;
                    float			zrot = 0;
 
                    while ((line = posefile.ReadLine().Trim()) != null)
                    {
                        string[]    words = line.Split(space);
                        string      opcode;
 
                        if (line == "")
                            continue;
                        if (line.EndsWith("{"))             // open bracket increases nesting leve4l
                        {
                            ++level;
                            continue;
                        }
                        if (line.EndsWith("}"))             // close bracket decreases nesting level
                        {
                            --level;
                            if (level == 0)
                                break;
                            continue;
                        }
                        if (words.Length == 0)              // nothing parsed?
                            continue;
                        opcode = words[0];
                        if (words.Length < 1)               // has an argument?
                            continue;
                        if (opcode == "actor")              // found a new bone?
                        {
                            bonename = words[1];            // save the bone name
							xrot = 0;
							yrot = 0;
							zrot = 0;
                            ++numbones;
                        }
                        else if (opcode == "rotateX")       // X rotation coming up?
                            offset = 0;
                        else if (opcode == "rotateY")       // Y rotation coming up?
                            offset = 1;
                        else if (opcode == "rotateZ")       // Z rotation coming up?
                            offset = 2;
                        else if (opcode == "k")             // is it a key?
                        {
                            float time = float.Parse(words[1]);     // parse the time
                            float angle = float.Parse(words[2]);    // parse the rotation angle in degrees
                            Quat  q;
 
                            angle *= (float) Math.PI / 180;
                            switch (offset)
                            {
                                case 0: xrot = angle; break;

                                case 1: yrot = angle; break;

                                case 2:
                                zrot = angle;
								if ((xrot == 0) && (yrot == 0) && (zrot == 0))
									break;
                                q = new Quat(Model.XAXIS, xrot);
								if (yrot != 0)
									q *= new Quat(Model.YAXIS, yrot);
 								if (zrot != 0)
									q *= new Quat(Model.ZAXIS, yrot);
								q.Normalize();
                                offset = -1;
								if (q.IsEmpty())
									break;
								if (bonename.Length > 0)            // save rotation for the bone
								{
									poseinfo[bonename] = q;
									SharedWorld.Trace(bonename + " " + q.x + " " + q.y + " " + q.z + " " + q.w + "\n");
								}
                                break;
                           }
                        }
                    }
                 }
                if (numbones != BodyPoser.NumBones)
                     Canvas3D.LogError(System.String.Format("Pose with {0} bones does not match skeleton with {1} bones", numbones, BodyPoser.NumBones));
                /*
                 * Convert hash table with rotations into a Pose.
                 */
                Pose pose = new Pose(BodyPoser);
                foreach (DictionaryEntry entry in poseinfo)
                {
                    System.String key = entry.Key.ToString();
                    System.String bonename = key;
                    Quat q = entry.Value as Quat;
                    int boneindex;

                    boneindex = BodyPoser.GetBoneIndex(bonename);
                    if (boneindex >= 0)
                    {
                        pose.SetLocalRotation(boneindex, q);
                        SharedWorld.Trace(bonename + " " + q.x + " " + q.y + " " + q.z + " " + q.w + "\n");
                    }
                    else
                        Canvas3D.LogError("Cannot find bone " + bonename + " in skeleton ");
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
