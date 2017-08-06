#pragma once

namespace Vixen
{
	class GLRenderer;

	/*!
	 * @class CullList
	 *
	 * @brief Geometry sorter which performs occlusion culling to further
	 *	reduce number of primitives drawn.
	 *
	 * This method of occclusion culling relies on an extension
	 * to OpenGL which checks whether or not pixels were drawn
	 * by the display hardware.
	 *
	 * @internal
	 * @ingroup vixen
	 *
	 * @see GeoSorter CullScene
	 */
	class CullList : public GLRenderer
	{
	public:
		CullList();

		virtual	int		AddShape(Shape*, const Matrix*, int32 lightmask);
		virtual	void	Reset();
		virtual	void	Render(int32 opts = GeoSorter::All);

	protected:
		void		RenderBox(Box3&) const;
		bool		Cull(const Geometry*) const;

		bool		HasOcclusion;
	};

}	// end Vixen

