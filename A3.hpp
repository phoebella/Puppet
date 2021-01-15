// Winter 2019

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"
#include "GeometryNode.hpp"

#include "SceneNode.hpp"
#include "JointNode.hpp"
#include "Command.h"


#include <glm/glm.hpp>
#include <memory>
#include <vector>
using namespace std;

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};


class A3 : public CS488Window {
public:
	A3(const std::string & luaSceneFile);
	virtual ~A3();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(const SceneNode &node);
	void renderArcCircle();

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;
    
    
    
    //changes
    double cur_x,cur_y;
    bool can_render_circle;
    bool z_buffer_on;
    bool back_face;
    bool front_face;
    bool left,middle,right;
    int mode;
    bool picking;
    list<Command*> my_commands;
    Command* cur_command;
    int iterator_counter;
    vector<SceneNode*> all_nodes;
    vector<JointNode *> selected_nodes;
    
    
    //methods
    void recursive_rotate(SceneNode* m_rootNode,glm::mat4,glm::mat4 m_rotate);
    SceneNode* recursive_find_byname(SceneNode* m_rootNode,std::string s);
    SceneNode* recursive_find(SceneNode* m_rootNode, unsigned int node_id);
    void vCalcRotVec(float fNewX, float fNewY,
                float fOldX, float fOldY,
                float fDiameter,
                     float *fVecX, float *fVecY, float *fVecZ);
    void vAxisRotMatrix(float fVecX, float fVecY, float fVecZ, glm::mat4& mNewMat);
    void print_mat4(glm::mat4);
    void print_vector(vector<SceneNode*>);
    void updateShaderUniforms(
                              const ShaderProgram & shader,
                              const GeometryNode & node,
                              const glm::mat4 & viewMatrix
                              );
    
    void undo();
    void redo();
    Command* get_cur_command();
    
};
