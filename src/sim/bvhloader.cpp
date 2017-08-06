#include "vixen.h"


namespace Vixen {

/*!
 * @fn bool BVHLoader::ReadAnim(const TCHAR* filename, Core::Stream* instream, LoadEvent* event)
 * @param filename	name of BVH file to load.
 * @param instream	stream to use for reading.
 * @param event		event to initialize if load is successful.
 *
 * Called by the loader to load a BVH animation file.
 * LoadAnim will issue a blocking read that waits until all
 * of the motion capture data been loaded.
 *
 * @return \b true if load was successful, else \b false
 *
 * @see World::LoadAsync FileLoader::Load
 */
bool BVHLoader::ReadAnim(const TCHAR* filename, Core::Stream* instream, LoadEvent* e)
{
	Ref<Skeleton>	bvhskel;
	Skeleton*		skel = (Skeleton*) (SharedObj*) e->Sender;	
	BVHLoader		loader(*instream, filename);
	Core::String	name;
	LoadSceneEvent*	ev = (LoadSceneEvent*) e;

	ev->Code = Event::LOAD_SCENE;
	if (!loader.LoadSkel())					// make skeleton from BVH hierarchy
		VX_ERROR(("BVHLoader: %s ERROR: cannot load BVH file skeleton", filename), false);
	bvhskel = loader.GetSkeleton();
	if (loader.GetNumFrames() <= 1)			// a single frame - it is a pose then
	{
		if (!skel || !skel->IsClass(VX_Skeleton))
		{
			if (!loader.LoadKeyAsPose(bvhskel))	// load first set of BVH keys
				VX_ERROR(("BVHLoader: %s ERROR: cannot load BVH file motion data", filename), false);
			ev->Object = loader.GetPose();
		}
		else
		{
			Messenger* mess = GetMessenger();

			mess->Observe(skel, Event::LOAD_SCENE, NULL);
			if (!loader.LoadKeyAsPose(skel))	// load first set of BVH keys
				VX_ERROR(("BVHLoader: %s ERROR: cannot load BVH file motion data", filename), false);
			if (skel->GetNumBones() != bvhskel->GetNumBones())
			{
				VX_WARNING(("BVHLoader: %s WARNING pose with %d bones may be incompatible with skeleton %s with %d bones",
						filename, bvhskel->GetNumBones(), skel->GetName(), skel->GetNumBones()));
			}
			ev->Object = loader.GetPose();
		}
	}
	else
	{
		if (!loader.LoadKeyAsPose(bvhskel))		// read BVH bind pose
			VX_ERROR(("BVHLoader: %s ERROR: cannot load BVH file motion data", filename), false);
		bvhskel->SetBindPose(loader.GetPose());	// set BVH skeleton bind pose
		if (!loader.LoadKeys())					// load BVH keys
			VX_ERROR(("BVHLoader: %s ERROR: cannot load BVH file motion data %s", (const TCHAR*) name), false);
		ev->Object = bvhskel;
	}
	VX_TRACE(FileLoader::Debug, ("BVHLoader::LoadPose %s", name));
#ifdef _DEBUG
	if (FileLoader::Debug > 2)
		bvhskel->Print(vixen_debug, SharedObj::PRINT_All);
#endif
	return true;
}

BVHLoader::BVHLoader(Core::Stream& instream, const TCHAR* filename)
 :	m_Stream(instream), m_FileName(filename)
{
	TCHAR	filebase[VX_MaxPath];
	TCHAR	dir[VX_MaxPath];

	instream.ParseDirectory(filename, filebase, dir);
	instream.SetDirectory(dir);			// establish base directory
	m_FileBase = filebase;
}

bool BVHLoader::LoadSkel()
{
	Transformer*	root = NULL;

	Transformer*	bone = NULL;
	Transformer*	parent = NULL;
	Skeleton*		skel = NULL;
	int				level = 0;
	TCHAR			line[1024];
	int				nread;
	int				nlines = 0;
	int				numbones = 0;
	float			x, y, z;
	Core::String	bonename;
	Core::BaseArray<const TCHAR*, Core::BaseObj>	names;
	Core::BaseArray<int, Core::BaseObj>				parentids;

	names.SetElemAllocator(Core::ThreadAllocator::Get());
	parentids.SetElemAllocator(Core::ThreadAllocator::Get());
	while (nread = NextLine(line, 1024))
	{
		TCHAR*	p = line;
		TCHAR*	q;
		int		n = 0;

		++nlines;
		while ((*p == TEXT('\t')) || (*p == TEXT(' ')))
			++p;
		if (*p == TEXT('{'))			// down one level
		{
			if (!bonename.IsEmpty())
				++level;
			continue;
		}
		if (*p == TEXT('}'))			// up one level
		{
			if (!bonename.IsEmpty())
			{
				--level;
				VX_ASSERT(level >= 0);
				parent = (Transformer*) bone->Parent();
				bone = NULL;
			}
			continue;
		}
		if (STRNCASECMP(p, TEXT("offset"), 6) == 0)
		{
			if (SSCANF(p + 6, TEXT("%f %f %f"), &x, &y, &z) != 3)
			{
				VX_WARNING(("BVHLoader: ERROR bad syntax line %d: %s", nlines, p));
				continue;
			}
			VX_ASSERT(bone);
			bone->SetPosition(Vec3(x, y, z));
			bone->CalcMatrix();
			VX_TRACE2(FileLoader::Debug, ("\toffset %0.3f %0.3f %0.3f\n", x, y, z));
			continue;
		}
		if (STRNCASECMP(p, TEXT("end"), 3) == 0)
		{
			bonename.Empty();
			continue;
		}
		if (STRNCASECMP(p, TEXT("frame"), 5) == 0)
		{
			if (SSCANF(p, TEXT("Frames: %d"), &m_NumFrames) == 1)
				continue;
			if (SSCANF(p, TEXT("Frame Time: %f"), &m_TimeStep) == 1)
				break;
		}
		if (STRNCASECMP(p, TEXT("root"), 4) == 0)
		{
			root = bone = new Transformer();
			p += 5;
		}
		else if (STRNCASECMP(p, TEXT("joint"), 5) == 0)
		{
			parent = bone;
			bone = new Transformer();
			VX_ASSERT(parent);
			parent->Append(bone);
			bone->SetControl(Engine::CONTROL_CHILDREN | Engine::CHILDREN_FIRST);
			p += 6;
		}
		else
			continue;
		q = p;
		bonename.Empty();
		while (*q && (*q != TEXT('\t')) && (*q != TEXT(' ')) && (*q != TEXT('\n')) && (*q != TEXT('\r')))
			bonename += *q++;
		VX_TRACE2(FileLoader::Debug, ("Creating bone %d %s\n", numbones, bonename));
		bone->SetName(bonename);
		bone->SetBoneIndex(numbones);
		names.Append(bone->GetName());
		++numbones;
	}
	skel = new Skeleton(numbones);
	skel->SetName(m_FileBase + TEXT('.') + m_FileBase + TEXT(".skeleton"));
	for (int i = 0; i < numbones; ++i)
	{
		skel->SetBoneName(i, names[i]);
	}
	skel->Append(root);
	skel->FindBones();
	/*
	 * Calculate the bind pose of the skeleton. All the rotations are empty.
	 * The translations are converted to world space.
	 */
	Vec3*	positions = (Vec3*) alloca(numbones * sizeof(Vec3));
	Quat*	rotations = (Quat*) alloca(numbones * sizeof(Quat));
	for (int i = 0; i < numbones; ++i)
	{
		Transformer*	bone = skel->GetBone(i);

		rotations[i].Set(0, 0, 0, 1);
		positions[i].Set(0, 0, 0);
		if (bone)
		{
			Core::String	s(m_FileBase);
			const Matrix*	total = bone->GetTotalTransform();

			total->GetTranslation(positions[i]);
			s += TEXT('.');
			s += names[i];
			s += TEXT(".xform");
			bone->SetName(s);
		}
	}
	skel->SetBindPose(rotations, positions);
	m_Skeleton = skel;
	return true;
}

bool BVHLoader::LoadKeyAsPose(Skeleton* skel)
{
	int				numchannels = (skel->GetNumBones() + 1) * 3;
	float*			data = (float*) alloca(numchannels * sizeof(float));
	int				nkeys = ReadKeys(data, numchannels);
	Transformer*	bone;
	Pose*			pose = new Pose(skel);
	float*			fp = data;

	if (nkeys != numchannels)
	{	VX_WARNING(("BVHLoader: Warning %d channels requested, %d keys read", numchannels, nkeys)); }
	bone = skel->GetBone(0);
	VX_ASSERT(bone);
	pose->Copy(skel->GetBindPose());
	pose->SetName(m_FileBase + TEXT('.') + m_FileBase + TEXT(".pose"));
	m_RootPos.Set(fp[0], fp[1], fp[2]);
	fp += 3;
	m_Pose = pose;
	pose->Sync();
	for (int i = 0; i < m_Skeleton->GetNumBones(); ++i)
	{
		float	z = *fp++ * PI / 180.0f;
		float	x = *fp++ * PI / 180.0f;
		float	y = *fp++ * PI / 180.0f;
		Quat	q(Model::ZAXIS, z);
		int		boneindex = i;

		if (m_Skeleton != (const Skeleton*) skel)
			boneindex = skel->GetBoneIndex(m_Skeleton->GetBoneName(i));
		if (boneindex < 0)
		{
			VX_WARNING(("BVHLoader: WARNING bone %s not found in skeleton", m_Skeleton->GetBoneName(i)));
			continue;
		}
		q *= Quat(Model::YAXIS, y);
		q *= Quat(Model::XAXIS, x);
		q.Normalize();
		VX_ASSERT(q.LengthSquared() <= 1.0f + FLT_EPSILON);
		q *= pose->GetLocalRotation(boneindex);
		pose->SetLocalRotation(boneindex, q);
		VX_PRINTF(("%s %0.3f %0.3f %0.3f %0.3f\n", skel->GetBoneName(boneindex), q.x, q.y, q.z, q.w));
	}
	pose->Sync();
	return true;
}

bool BVHLoader::LoadKeys()
{
	int				numbones = m_Skeleton->GetNumBones();
	int				numchannels = (numbones + 1) * 3;
	float*			data = (float*) alloca(numchannels * sizeof(float));
	int				nkeys;
	Transformer*	bone = m_Skeleton->GetBone(0);
	Interpolator*	poskeys = new Interpolator();
	Core::String	name;
	Interpolator**	rotations = (Interpolator**) alloca(numbones * sizeof(Interpolator*));

	memset(rotations, 0, numbones * sizeof(Interpolator*));
	m_Time = 0;
	name = bone->GetName();
	name += ".pos";
	poskeys->SetName(name);
	poskeys->SetInterpType(Interpolator::LINEAR);
	poskeys->SetDestType(Interpolator::POSITION);
	poskeys->SetValSize(3);
	poskeys->SetMaxSize(m_NumFrames);
	bone->Append(poskeys);
	while (nkeys = ReadKeys(data, numchannels))
	{
		float*	fp = data;
		Vec3	p(fp[0], fp[1], fp[2]);

		if (nkeys != numchannels)
		{	VX_WARNING(("BVHLoader: Warning %d channels requested, %d keys read", numchannels, nkeys)); }
		p -= m_RootPos;
		poskeys->AddKey(m_Time, (const float*) &p);
		fp += 3;
		for (int i = 0; i < m_Skeleton->GetNumBones(); ++i)
		{
			bone = m_Skeleton->GetBone(i);
			if (bone == NULL)
				continue;

			Interpolator*	rotkeys = (Interpolator*) rotations[i];
			float	z = *fp++ * PI / 180.0f;
			float	x = *fp++ * PI / 180.0f;
			float	y = *fp++ * PI / 180.0f;
			Interpolator::WQuat	wq;

			Quat q = Quat(Model::ZAXIS, z);
			q *= Quat(Model::YAXIS, y);
			q *= Quat(Model::XAXIS, x);
			q.Normalize();
//			VX_PRINTF(("%s %0.3f %0.3f %0.3f %0.3f\n", m_Skeleton->GetBoneName(i), q.x, q.y, q.z, q.w));

			if (rotkeys == NULL)
			{
				name = bone->GetName();
				name += ".rot";
				rotkeys = new Interpolator();
				rotkeys->SetInterpType(Interpolator::SLERP);
				rotkeys->SetDestType(Interpolator::ROTATION);
				rotkeys->SetName(name);
				rotkeys->SetValSize(sizeof(Quat) / sizeof(float));
				rotkeys->SetMaxSize(m_NumFrames);
				bone->Append(rotkeys);
				rotations[i] = rotkeys;
//				VX_PRINTF(("%s %p %0.3f %0.3f %0.3f %0.3f\n", name, rotkeys, q.x, q.y, q.z, q.w));
			}
			rotkeys->AddKey(m_Time, (const float*) &q);
		}
		m_Time += m_TimeStep;
	}
	return true;
}

int	BVHLoader::ReadKeys(float* data, int numchannels)
{
	TCHAR	line[4096];
	int		nread;
	int		nkeys = 0;
	float*	fp = data;
	TCHAR*	s = line;
	TCHAR*	e;
	double	v;

	nread = NextLine(line, 4096);
	if (nread == 0)
		return 0;
	while (s < (line + nread)) 
	{
		v = STRTOD(s, &e);
		if (s == e)
			break;
		s = e;
		*fp++ = (float) v;
		if (++nkeys >= numchannels)
			break;
	}
	return nkeys;
}


/*
 * Read up to the next newline into the buffer
 */
int	BVHLoader::NextLine(TCHAR* linebuf, int maxlen)
{
	int nread = 0;
	TCHAR*	p = linebuf;

	while (!m_Stream.IsEmpty())
	{
		TCHAR	c;
		if (m_Stream.Read((char*) &c, sizeof(TCHAR)) != sizeof(TCHAR))
			break;
		if (nread < maxlen)
		{
			++nread;
			*p++ = c;
		}
		if ((c == 0) || (c == TEXT('\n')))
			break;
	}
	return nread;
}

}	// end Vixen