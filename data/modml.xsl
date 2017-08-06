<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:msxsl="urn:schemas-microsoft-com:xslt" exclude-result-prefixes="msxsl">
    <xsl:output method="xml" indent="yes"/>
  <xsl:variable name="numframes" >
    <xsl:apply-templates mode="number" select="//RWF_JOINT_ANIMATION/RWF_ANIMATION_FRAMES/@Contents"/>
  </xsl:variable>

  <xsl:template match="/">
      <xsl:apply-templates select="*/RWF_SCENE" >
      </xsl:apply-templates>
     </xsl:template>
  
  <xsl:template match="RWF_SCENE">

    <scene>
       <xsl:attribute name="Name">
        <xsl:value-of select="RWF_NAME/@Contents" />
      </xsl:attribute>
      <xsl:apply-templates select="RWF_VIEW_TRANSFORM|RWF_TRANSFORM" />
      <models>
        <xsl:apply-templates select="RWF_LIGHT/RWF_LIGHT_ITEM" />
        <xsl:apply-templates select="RWF_MESH/RWF_MESH_ITEM" />
      </models>
      <engines>
        <skeleton>
          <xsl:attribute name="Name"  >
            <xsl:value-of select="RWF_JOINT/RWF_JOINT_ITEM/RWF_NAME/@Contents" />
          </xsl:attribute>
          <xsl:apply-templates select="RWF_JOINT" />
          <xsl:apply-templates select="RWF_SKIN" />
        </skeleton>
      </engines>
      <xsl:apply-templates select="RWF_MOD_IMAGES" />
    </scene>
  </xsl:template>

  <xsl:template match="RWF_VIEW_TRANSFORM">
    <Camera>
      <Matrix>
        <xsl:apply-templates select="*" />
      </Matrix>
    </Camera>
  </xsl:template>
  
  <xsl:template match="RWF_TRANSFORM|RWF_TRANSFORM_CURRENT|RWF_JOINT_LOCAL_MAT">
    <Matrix>
      <xsl:apply-templates select="RWF_TRANSFORM_SCALE|RWF_TRANSFORM_ROTATE|RWF_TRANSFORM_TRANSLATION" />
    </Matrix>
  </xsl:template>

  <xsl:template match="RWF_TRANSFORM_SCALE">
    <xsl:attribute name="scale">
      <xsl:apply-templates mode="vec3" select="@Contents"/>
    </xsl:attribute>
  </xsl:template>
  
  <xsl:template match="RWF_TRANSFORM_ROTATE">
    <xsl:attribute name="rotate">
      <xsl:apply-templates mode="vec3" select="@Contents"/>
    </xsl:attribute>
  </xsl:template>
  
  <xsl:template match="RWF_TRANSFORM_TRANSLATION">
    <xsl:attribute name="translate">
      <xsl:apply-templates mode="vec3" select="@Contents"/>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="RWF_MOD_IMAGES">
    <Textures>
      <xsl:apply-templates select="RWF_MOD_IMAGES_ITEM "/>
    </Textures>
  </xsl:template>
  
  <xsl:template match="RWF_MOD_IMAGES_ITEM">
    <Texture>
      <xsl:attribute name="Name">
        <xsl:value-of select="@Name"/>
      </xsl:attribute>
      <xsl:attribute name="FileName">
        <xsl:value-of select="descendant::RWF_RESOURCE_FULL_PATH/@Contents"/>
        <xsl:value-of select="descendant::RWF_RESOURCE_RELATIVE_PATH/@Contents"/>
        <xsl:value-of select="@Name"/>
      </xsl:attribute>
      <xsl:attribute name="ID_3D">
        <xsl:value-of select="RWF_3D_ID/@Contents"/>
      </xsl:attribute>
    </Texture>
  </xsl:template>
 
  
  <xsl:template match="RWF_MESH_ITEM">
    <shape>
      <xsl:attribute name="Name">
        <xsl:value-of select="RWF_NAME/@Contents"/>
      </xsl:attribute>
      <xsl:attribute name="ID">
        <xsl:apply-templates mode="number" select="RWF_ID/@Contents"/>
      </xsl:attribute>
      <xsl:apply-templates select="RWF_TRANSFORM" />
      <mesh>
        <xsl:apply-templates select="RWF_MESH_GEOMETRY_SHORT" />
        <vertices>
          <xsl:attribute name="Count">
            <xsl:apply-templates mode="number" select="RWF_MESH_VERTEX_NUMBER/@Contents"/>
          </xsl:attribute>
          <data type="float">
            <xsl:value-of select="RWF_MESH_VERTEX/@Contents"/>
          </data>
        </vertices>
    </mesh>
      <xsl:apply-templates select="RWF_FACE_SET/RWF_FACE_SET_ITEM/RWF_SHADER"/>
    </shape>
  </xsl:template>

  <xsl:template match="RWF_SHADER">
    <appearance>
      <xsl:attribute name="Name">
        <xsl:value-of select="descendant::RWF_RESOURCE/@Name"/>
      </xsl:attribute>
      <xsl:attribute name="file">
        <xsl:value-of select="descendant::RWF_RESOURCE_FULL_PATH/@Contents"/>
        <xsl:value-of select="descendant::RWF_RESOURCE_RELATIVE_PATH/@Contents"/>
      </xsl:attribute>
      <xsl:attribute name="ID_3D">
        <xsl:apply-templates mode="number" select="RWF_3D_ID/@Contents"/>
      </xsl:attribute>
    </appearance>
  </xsl:template>
  
  <xsl:template match="RWF_MESH_GEOMETRY_SHORT">
  <vertexindices>
    <xsl:attribute name="Count">
      <xsl:apply-templates mode="number" select="RWF_MESH_FACES_NUMBER/@Contents"/>
    </xsl:attribute>
    <data type="int">
      <xsl:value-of select="RWF_MESH_FACES/@Contents"/>
    </data>
  </vertexindices>
    <texcoordindices>
      <xsl:attribute name="Count">
        <xsl:apply-templates mode="number" select="RWF_MESH_FACES_NUMBER/@Contents"/>
      </xsl:attribute>
      <data type="int">
        <xsl:value-of select="RWF_MESH_FACE_TEXTURES/@Contents"/>
      </data>
    </texcoordindices>
    <texcoords>
      <xsl:attribute name="Count">
        <xsl:apply-templates mode="number" select="RWF_MESH_TEXTURES_NUMBER/@Contents"/>
      </xsl:attribute>
      <data type="float">
        <xsl:value-of select="RWF_MESH_TEXTURES/@Contents"/>
      </data>
    </texcoords>
  </xsl:template>

  <xsl:template match="RWF_LIGHT_ITEM">
    <Light>
      <xsl:value-of select="RWF_DATA/@Contents"/>
    </Light>
  </xsl:template>

  <xsl:template match="RWF_SKIN">
    <skin>
      <vertexindices>
        <xsl:attribute name="Count" >
          <xsl:apply-templates mode="number" select="../RWF_SKIN_NUMBER/@Contents"/>
        </xsl:attribute>
        <data type="int">
          <xsl:value-of select="RWF_SKIN_VERTEX_INDEX/@Contents" />
        </data>
      </vertexindices>
      <weights>
        <xsl:value-of select="RWF_SKIN_WEIGHTS/@Contents"/>
      </weights>
      <joints>
        <xsl:value-of select="RWF_SKIN_JOINTS_IDS/@Contents"/>
      </joints>
    </skin>
  </xsl:template>
  
  <xsl:template match="RWF_JOINT">
       <xsl:apply-templates select="RWF_JOINT_ITEM" />
  </xsl:template>

  <xsl:template match="*[starts-with(name(), 'RWF_JOINT_ITEM')]" >
    <xsl:variable name="jointname">
      <xsl:value-of select="RWF_NAME/@Contents"/>
    </xsl:variable>
    <xsl:if test="$jointname!=''">     
    <joint>
      <xsl:attribute name="Name">
        <xsl:value-of select="$jointname"/>
      </xsl:attribute>
      <xsl:attribute name="ID">
        <xsl:apply-templates mode="number" select="RWF_ID/@Contents"/>
      </xsl:attribute>
      <matrix>
        <xsl:value-of select="RWF_JOINT_LOCAL_AXIS_MAT/@Contents"/>
      </matrix>
      <xsl:apply-templates select="//RWF_JOINT_ANIMATION//RWF_NAME[@Contents=$jointname]/following-sibling::RWF_JOINT_ANIMATION_TRANS_MAT" />
      <xsl:apply-templates select="RWF_JOINT" />
      <xsl:apply-templates select="*[starts-with(name(), 'RWF_JOINT_ITEM')]" />
    </joint>
    </xsl:if>
    <xsl:if test="$jointname=''">
      <xsl:apply-templates select="RWF_JOINT" />
      <xsl:apply-templates select="*[starts-with(name(), 'RWF_JOINT_ITEM')]" />
    </xsl:if>
   </xsl:template>

  <xsl:template match="RWF_JOINT_LOCAL_AXIS_MAT" >
    <xsl:attribute name="LocalAxis">
      <matrix>
        <xsl:value-of select="@Contents"/>
      </matrix>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="RWF_JOINT_ANIMATION_TRANS_MAT">
    <animation_data type="float" >
      <xsl:attribute name="Count" >
        <xsl:value-of select="$numframes"/>
      </xsl:attribute>
      <xsl:value-of select="@Contents"/>
    </animation_data>
  </xsl:template>

  <xsl:template mode="number" match="@Contents">
    <xsl:if test="starts-with(., '1: ')" >
      <xsl:value-of select="substring(., 4, string-length(.)-3)"/>         
    </xsl:if>
  </xsl:template>

  <xsl:template mode="vec3" match="@Contents">
    <xsl:if test="starts-with(., '3: ')" >
      <xsl:value-of select="substring(., 4, string-length(.)-3)"/>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
