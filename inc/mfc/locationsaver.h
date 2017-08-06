#pragma once

namespace Vixen
{
	namespace MFC
	{
		/*!
		 * @class LocationSaver
		 * @brief mMFC class that maintains a file of saved camera positions
		 */
		class LocationSaver : public Engine
		{
		// Construction
		public:
			LocationSaver();
			LocationSaver(const TCHAR* fname);
			~LocationSaver();

			void JumpToSavedLocation(int num);
			bool ReadStoredLocations(const TCHAR *fname);
			void WriteStoredLocations(const TCHAR *fname = NULL);
			void StoreCurrentLocationTo(int num);
			void RecallStoredLocation(int num);
			void GetLocation(int num, Vec3& pos, Quat& rot);

		// Implementation
		protected:
			Core::String	m_strFilename;
			FloatArray		m_Locations;
		};
	}	// end MFC
}		// end  Vixen
