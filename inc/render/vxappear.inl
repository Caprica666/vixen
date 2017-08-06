namespace Vixen {

inline const DeviceBuffer* Appearance::GetMaterial() const
	{ return m_Material; }

inline DeviceBuffer* Appearance::GetMaterial()
	{ return m_Material; }

inline const Shader* Appearance::GetPixelShader() const
{	return m_PixelShader; }


inline const Shader* Appearance::GetVertexShader() const
{	return m_VertexShader; }


inline bool	Shader::GetShaderCode(const void** data, size_t* length) const
{
	if (data && length)
	{
		*data = (const void*) m_BinaryCode;
		*length = (size_t) m_BinaryLength;
		return *data != NULL;
	}
	return false;
}

} // end Vixen