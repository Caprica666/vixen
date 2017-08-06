#include "vxmfc.h"
#include "mfc/locationsaver.h"

#define DEFAULT_VIEWPOINTS 6
#define	LOC_EntrySize	((sizeof(Vec3) + sizeof(Quat)) / sizeof(float))

namespace Vixen {
namespace MFC {

LocationSaver::LocationSaver()
{
	m_Locations.SetMaxSize(DEFAULT_VIEWPOINTS);
}

LocationSaver::LocationSaver(const TCHAR* fname)
{
	ReadStoredLocations(fname);
}

LocationSaver::~LocationSaver()
{
}

void LocationSaver::StoreCurrentLocationTo(int num)
{
	SharedObj*	target = GetTarget();
	Vec3	pos;
	Quat	 rot;

	if (target == NULL)
		return;
	if (target->IsClass(VX_Camera))
	{
		Camera* cam = (Camera*) target;
		pos = cam->GetTranslation();
		rot = cam->GetRotation();
	}
	else if (target->IsClass(VX_Transformer))
	{
		Transformer* xform = (Transformer*) target;
		pos = xform->GetPosition();
		rot = xform->GetRotation();
	}
	else return;

	num *= LOC_EntrySize;
	m_Locations.SetAt(num, pos.x);
	m_Locations.SetAt(num + 1, pos.y);
	m_Locations.SetAt(num + 2, pos.z);
	m_Locations.SetAt(num + 3, rot.x);
	m_Locations.SetAt(num + 4, rot.y);
	m_Locations.SetAt(num + 5, rot.z);
	m_Locations.SetAt(num + 6, rot.w);
	WriteStoredLocations();
}

void LocationSaver::GetLocation(int num, Vec3& pos, Quat& rot)
{
	if (num >= m_Locations.GetSize())
		return;
	num *= LOC_EntrySize;
	pos.x = m_Locations.GetAt(num);
	pos.y = m_Locations.GetAt(num + 1);
	pos.z = m_Locations.GetAt(num + 2);
	rot.x = m_Locations.GetAt(num + 3);
	rot.y = m_Locations.GetAt(num + 4);
	rot.z = m_Locations.GetAt(num + 5);
	rot.w = m_Locations.GetAt(num + 6);
}

void LocationSaver::RecallStoredLocation(int num)
{
	// safety check
	SharedObj*	target = GetTarget();
	Vec3	pos;
	Quat	rot;

	if (target == NULL)
		return;
	if (num >= m_Locations.GetSize())
		return;
	GetLocation(num, pos, rot);	
	num *= LOC_EntrySize;
	if (target->IsClass(VX_Camera))
	{
		Camera* cam = (Camera*) target;
		cam->Reset();
		cam->Rotate(rot);
		cam->SetTranslation(pos);
	}
	else if (target->IsClass(VX_Transformer))
	{
		Transformer* xform = (Transformer*) target;
		xform->SetPosition(pos);
		xform->SetRotation(rot);
	}
}

bool LocationSaver::ReadStoredLocations(const TCHAR *fname)
{
	float tmp;

	// remember filename for later
	m_strFilename = fname;

	FILE *fp = FOPEN(m_strFilename, TEXT("r"));
	if (!fp) return false;

	int num_viewpoints;
	m_Locations.SetSize(0);
	fscanf(fp, "%d\n", &num_viewpoints);
	for (int i = 0; i < num_viewpoints; i++)
	{
		for (int j = 0; j < LOC_EntrySize; ++j)
		{
			fscanf(fp, "%f", &tmp);
			m_Locations.Append(tmp);
		}
		fscanf(fp, "\n");
	}
	fclose(fp);
	return true;
}

void LocationSaver::WriteStoredLocations(const TCHAR *fname)
{
	float tmp;

	// if they don't provide a name, default to the name used to read
	if (fname != NULL)
		m_strFilename = fname;

	FILE *fp = FOPEN(m_strFilename, TEXT("w"));
	if (!fp) return;

	int num_viewpoints = (int) m_Locations.GetSize() / LOC_EntrySize;
	fprintf(fp, "%d\n", num_viewpoints);
	for (int i = 0; i < num_viewpoints; i++)
	{
		for (int j = 0; j < LOC_EntrySize; ++j)
		{
			tmp = m_Locations.GetAt(i * LOC_EntrySize + j);
			fprintf(fp, "%f ", tmp);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void LocationSaver::JumpToSavedLocation(int num)
{
	RecallStoredLocation(num-1);
}

}	// end MFC
}	// end Vixen