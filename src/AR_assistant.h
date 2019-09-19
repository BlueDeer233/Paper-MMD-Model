#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofVboMesh.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define	STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image.h>

#include <Saba/Base/Path.h>
#include <Saba/Base/File.h>
#include <Saba/Base/UnicodeUtil.h>
#include <Saba/Base/Time.h>
#include <Saba/Model/MMD/PMDModel.h>
#include <Saba/Model/MMD/PMXModel.h>
#include <Saba/Model/MMD/VMDFile.h>
#include <Saba/Model/MMD/VMDAnimation.h>
#include <Saba/Model/MMD/VMDCameraAnimation.h>

class AR_assistant
{
public:
	AR_assistant();
	~AR_assistant();
	bool setup();
	bool road(std::string modelPath, std::string vmdPath1, std::string vmdPath2);
	void update();
	void draw();

private:
	struct AppContext;

	struct Input
	{
		std::string					m_modelPath;
		std::vector<std::string>	m_vmdPaths;
	};

	struct MMDShader
	{
		~MMDShader()
		{
			Clear();
		}

		GLuint	m_prog = 0;

		// attribute
		GLint	m_inPos = -1;
		GLint	m_inNor = -1;
		GLint	m_inUV = -1;

		// uniform
		GLint	m_uWV = -1;
		GLint	m_uWVP = -1;

		GLint	m_uAmbinet = -1;
		GLint	m_uDiffuse = -1;
		GLint	m_uSpecular = -1;
		GLint	m_uSpecularPower = -1;
		GLint	m_uAlpha = -1;

		GLint	m_uTexMode = -1;
		GLint	m_uTex = -1;
		GLint	m_uTexMulFactor = -1;
		GLint	m_uTexAddFactor = -1;

		GLint	m_uSphereTexMode = -1;
		GLint	m_uSphereTex = -1;
		GLint	m_uSphereTexMulFactor = -1;
		GLint	m_uSphereTexAddFactor = -1;

		GLint	m_uToonTexMode = -1;
		GLint	m_uToonTex = -1;
		GLint	m_uToonTexMulFactor = -1;
		GLint	m_uToonTexAddFactor = -1;

		GLint	m_uLightColor = -1;
		GLint	m_uLightDir = -1;

		GLint	m_uLightVP = -1;
		GLint	m_uShadowMapSplitPositions = -1;
		GLint	m_uShadowMap0 = -1;
		GLint	m_uShadowMap1 = -1;
		GLint	m_uShadowMap2 = -1;
		GLint	m_uShadowMap3 = -1;
		GLint	m_uShadowMapEnabled = -1;

		bool Setup(const AppContext& appContext);
		void Clear();
	};

	struct MMDEdgeShader
	{
		~MMDEdgeShader()
		{
			Clear();
		}

		GLuint	m_prog = 0;

		// attribute
		GLint	m_inPos = -1;
		GLint	m_inNor = -1;

		// uniform
		GLint	m_uWV = -1;
		GLint	m_uWVP = -1;
		GLint	m_uScreenSize = -1;
		GLint	m_uEdgeSize = -1;

		GLint	m_uEdgeColor = -1;

		bool Setup(const AppContext& appContext);
		void Clear();
	};

	struct MMDGroundShadowShader
	{
		~MMDGroundShadowShader()
		{
			Clear();
		}

		GLuint	m_prog = 0;

		// attribute
		GLint	m_inPos = -1;

		// uniform
		GLint	m_uWVP = -1;
		GLint	m_uShadowColor = -1;

		bool Setup(const AppContext& appContext);
		void Clear();
	};

	struct Texture
	{
		GLuint	m_texture;
		bool	m_hasAlpha;
	};

	struct AppContext
	{
		~AppContext()
		{
			Clear();
		}

		std::string m_resourceDir;
		std::string	m_shaderDir;
		std::string	m_mmdDir;

		std::unique_ptr<MMDShader>				m_mmdShader;
		std::unique_ptr<MMDEdgeShader>			m_mmdEdgeShader;
		std::unique_ptr<MMDGroundShadowShader>	m_mmdGroundShadowShader;

		glm::mat4	m_viewMat;
		glm::mat4	m_projMat;
		int			m_screenWidth = 0;
		int			m_screenHeight = 0;

		glm::vec3	m_lightColor = glm::vec3(1, 1, 1);
		glm::vec3	m_lightDir = glm::vec3(-0.5f, -1.0f, -0.5f);

		std::map<std::string, Texture>	m_textures;
		GLuint	m_dummyColorTex = 0;
		GLuint	m_dummyShadowDepthTex = 0;

		float	m_elapsed = 0.0f;
		float	m_animTime = 0.0f;
		std::unique_ptr<saba::VMDCameraAnimation>	m_vmdCameraAnim;

		bool Setup();
		void Clear();

		Texture GetTexture(const std::string& texturePath);
	};

	struct Material
	{
		explicit Material(const saba::MMDMaterial& mat)
			: m_mmdMat(mat)
		{}

		const saba::MMDMaterial&	m_mmdMat;
		GLuint	m_texture = 0;
		bool	m_textureHasAlpha = false;
		GLuint	m_spTexture = 0;
		GLuint	m_toonTexture = 0;
	};

	struct Model
	{
		std::shared_ptr<saba::MMDModel>	m_mmdModel;
		std::shared_ptr<saba::VMDAnimation>	m_vmdAnim;

		GLuint	m_posVBO = 0;
		GLuint	m_norVBO = 0;
		GLuint	m_uvVBO = 0;
		GLuint	m_ibo = 0;
		GLenum	m_indexType;

		GLuint	m_mmdVAO = 0;
		GLuint	m_mmdEdgeVAO = 0;
		GLuint	m_mmdGroundShadowVAO = 0;

		std::vector<Material>	m_materials;

		bool Setup(AppContext& appContext);
		void Clear();

		void UpdateAnimation(const AppContext& appContext);
		void Update(const AppContext& appContext);
		void Draw(const AppContext& appContext);
	};

public:
	AppContext m_appContext;
	Model m_model;
	std::shared_ptr<saba::VMDAnimation> m_vmdAnims[2];
};
