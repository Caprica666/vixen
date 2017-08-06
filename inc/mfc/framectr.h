
#pragma once

namespace Vixen
{
	namespace MFC
	{
	/*!
	 * @class FrameCounter
	 * @brief Engine that updates the status text of the main window
	 * with the frame count, total polygons and polygons culled.
	 *
	 * This engine must be used with an application that is derived
	 * from Demo because it relies upon a member function to
	 * permit access to the status text.
	 *
	 * @ingroup vixenmfc
	 * @see Engine::SetTarget VXStatusLog Engine
	 */
		class FrameCounter : public Engine
		{
		public:
			FrameCounter();
			virtual bool	Eval(float t);
			void			SetCameraInfo(bool bOnOff);

			//! Get frame number of last frame displayed.
			int		GetFrameNum() const		{ return m_Frame; }

			//! Get frame rate in frames per second.
			float	GetFrameRate() const	{ return m_FramesPerSec; }

			//! Get total vertices submitted last frame.
			int		GetTotalVerts() const	{ return m_VertsTotal; }

			//! Get vertices culled last frame.
			int		GetCulledVerts() const	{ return m_VertsCulled; }

			//! Get vertices drawn (not visibility culled) last frame.
			int		GetDrawnVerts() const	{ return m_VertsTotal - m_VertsCulled; }

			//! Getcamera position last frame.
			Vec3	GetCameraPos() const	{ return m_CameraPos; }

			//! Get camera direction last frame (XYZ rotation angles).
			Vec3	GetCameraDir() const	{ return m_CameraDir; }

		protected:
			uint32		m_Frame;			// frame number
			float		m_FramesPerSec;		// frames / sec last calculated
			uint32		m_VertsTotal;		// total verts shown last frame
			uint32		m_VertsCulled;		// verts culled last frame
			uint32		m_PrimsRendered;	// primitives rendered last frame
			Vec3		m_CameraPos;		// camera position last frame
			Vec3		m_CameraDir;		// camera direction last frame
			bool		m_bShowCameraInfo;	// true to show camera info on status line

			float	m_time[120];
			int		m_i;			// counts frames, modulo 30
		};
	}	// end MFC
} // end Vixen


