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
 * Reads a DAZ Studio DSF file with a pose in it and produces
 * a Vixen Pose object for this avatar's skeleton.
 * @param posename name of pose
 */
        public Pose ReadPose(System.String posename)
        {
            System.String filename = "pose/" + posename + "pose.dsf";
            Hashtable poseinfo = new Hashtable();
            try
            {
                /*
                 * Parse the DAZ Studio pose file and extract the rotations
                 * for each bone into a hash table
                 */
                using (StreamReader posefile = new StreamReader(filename))
                {
                    System.String line;
                    Regex pattern = new Regex("([a-zA-Z0-9]+)" + Regex.Escape(":?") + "rotation/([xyz])");
 
                    while ((line = posefile.ReadLine()) != null)
                    {
                        Match match = pattern.Match(line);
                        System.String name;
                        System.String axis;

                        if (!match.Success)
                            continue;
                        name = match.Groups[1].ToString();
                        axis = match.Groups[2].ToString();
                        line = posefile.ReadLine();
                        int p = line.IndexOf("[ 0, ");
                        if (p > 0)
                        {
                            System.String s = line.Substring(p + 5);
                            float angle;
                            Quat rot;
                            Quat q = null;
 
                            s = s.Substring(0, s.IndexOf("]"));
                            angle = float.Parse(s);
                            if (angle == 0)
                                continue;
                            angle *= (float) Math.PI / 180.0f;
                            if (axis == "x")
                                q = new Quat(Model.XAXIS, angle);
                            else if (axis == "y")
                                q = new Quat(Model.YAXIS, angle);
                            else if (axis == "z")
                                q = new Quat(Model.ZAXIS, angle);
                            if (q == null)
                                continue;
                            if (poseinfo.Contains(name))
                            {
                                rot = poseinfo[name] as Quat;
                                rot.Mul(rot, q);
                            }
                            else
                            {
                                poseinfo.Add(name, q);
                            }
                        }
                    }
                 }
                /*
                 * Convert hash table with rotations into a Pose.
                 * Map DAZ bone names into Vixen bone names.
                 */
                Pose pose = new Pose(BodyPoser);
                foreach (DictionaryEntry entry in poseinfo)
                {
                    System.String key = entry.Key.ToString();
                    System.String bonename = key;
                    Quat q = entry.Value as Quat;
                    int boneindex;

                    if (key == "abdomen") bonename = "Torso";
                    else if (key == "rHand") bonename = "RightWrist";
                    else if (key == "lHand") bonename = "LeftWrist";
                    else if (key == "rShldr") bonename = "RightShoulder";
                    else if (key == "lShldr") bonename = "LeftShoulder";
                    else if (key == "rForeArm") bonename = "RightElbow";
                    else if (key == "lForeArm") bonename = "LeftElbow";
                    else if (key == "rShin") bonename = "RightKnee";
                    else if (key == "lShin") bonename = "LeftShin";
                    else if (key == "rThigh") bonename = "RightHip";
                    else if (key == "lThigh") bonename = "LeftHip";
                    else if (key == "lFoot") bonename = "LeftAnkle";
                    else if (key == "rFoot") bonename = "RightAnkle";
                    boneindex = BodyPoser.GetBoneIndex(bonename);
                    if (boneindex >= 0)
                    {
                        pose.SetLocalRotation(boneindex, q);
                        SharedWorld.Trace(bonename + " " + q.x + " " + q.y + " " + q.z + " " + q.w + "\n");
                    }
                }
                return pose;
             }
            catch (Exception ex)
            {
                Canvas3D.LogError(ex.Message + " Cannot read pose file " + filename);
                return null;
            }
        }
	}
}
