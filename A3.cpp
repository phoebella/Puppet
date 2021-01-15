// Winter 2019

#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"
#include "Command.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <list>
#include <vector>

//#include "trackball.h"

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */





void A3::init()
{
	// Set the background colour.
	glClearColor(0.85, 0.85, 0.85, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();
    
    can_render_circle = false;
    z_buffer_on = true;
    back_face = false;
    front_face = false;
    
    cur_x = 0;
    cur_y = 0;
    picking = false;
    iterator_counter = 0;
    cur_command = NULL;


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
    
    for(int i = 0;i<m_rootNode->totalSceneNodes();++i){
        all_nodes.push_back(recursive_find(m_rootNode.get(),i));
    }
    /*for(int i = 0;i<m_rootNode->totalSceneNodes();++i){
        cout << "my name is " << all_nodes[i]->m_name << endl;
    }
    cout << endl;*/

}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(0.0f, 0.0f, 0.0f);
	m_light.rgbIntensity = vec3(1.0f); // light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;
        
        location = m_shader.getUniformLocation("picking");
        glUniform1i( location, picking ? 1 : 0 );

       //picking example
       if( !picking ) {

		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.05f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
        }
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);
    if (ImGui::BeginMenu("Application")){
        if(ImGui::MenuItem("Reset Position (I)")) {
        }
        if(ImGui::MenuItem("Reset Orientation (O)")){
        }
        if(ImGui::MenuItem("Reset Joints (J)")){
        }
        if(ImGui::MenuItem("Reset All (A)")){
            
        }
        if(ImGui::MenuItem("Quit (Q)")){
            glfwSetWindowShouldClose(m_window, GL_TRUE);
        }
        ImGui::EndMenu();
        
    }
    if (ImGui::BeginMenu("Edit")){
        if(ImGui::MenuItem("Undo (u)")){
            undo();
        }
        if(ImGui::MenuItem("Redo (R)")){
            redo();
        }
        ImGui::EndMenu();
    }
    if(ImGui::BeginMenu("Options")){
        if(ImGui::Checkbox("Circle (C)", &can_render_circle)) {}
        if(ImGui::Checkbox("Z-buffer (Z)", &z_buffer_on)) {}
        if(ImGui::Checkbox("Backface culling (B)", &back_face)) {}
        if(ImGui::Checkbox("Frontface culling (F)", &front_face)) {}
        ImGui::EndMenu();
    }
    if(ImGui::RadioButton("Position/Orientation (P)",&mode,0)){
    }
    if(ImGui::RadioButton("Joints (J)",&mode,1)){
    }


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		/*if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void A3::updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
    {
	
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;
        unsigned int idx = node.m_nodeId;
    
    if( picking ) {
        float r = float(idx&0xff) / 255.0f;
        float g = float((idx>>8)&0xff) / 255.0f;
        float b = float((idx>>16)&0xff) / 255.0f;
        
        location = m_shader.getUniformLocation("material.kd");
        glUniform3f( location, r, g, b );
        CHECK_GL_ERRORS;
    }else{
		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;


		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd = node.material.kd;
		glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.ks");
		vec3 ks = node.material.ks;
		glUniform3fv(location, 1, value_ptr(ks));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.shininess");
		glUniform1f(location, node.material.shininess);
		CHECK_GL_ERRORS;

    }
    }
	shader.disable();
}



//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {
    if(z_buffer_on){
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    if(front_face&&back_face){
        glEnable(GL_CULL_FACE);
        glCullFace( GL_FRONT_AND_BACK);
    } else if(front_face){
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    } else if(back_face){
        glEnable( GL_CULL_FACE);
        glCullFace( GL_BACK );
    } else {
        glDisable(GL_CULL_FACE);
    }
    

	//glEnable( GL_DEPTH_TEST );
	renderSceneGraph(*m_rootNode);


	glDisable( GL_DEPTH_TEST );
    if(can_render_circle){
	   renderArcCircle();
    }
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	for (const SceneNode * node : root.children) {

		if (node->m_nodeType != NodeType::GeometryNode)
			continue;

        //cout << "after ctu" << endl;
		const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);

		updateShaderUniforms(m_shader, *geometryNode, m_view);


		// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		//-- Now render the mesh:
		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}
 

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
    for (const SceneNode * node : root.children) {
        renderSceneGraph(*node);
    }
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}


void A3::recursive_rotate(SceneNode* m_rootNode,mat4 old_trans,mat4 m_rotate){
    mat4 inverse_trans = glm::inverse(old_trans);
    m_rootNode->trans = old_trans*m_rotate*inverse_trans*m_rootNode->trans;
    //print_mat4(m_rootNode->trans);
    for(SceneNode*node : m_rootNode->children){
        recursive_rotate(node,old_trans,m_rotate);
    }
}




SceneNode* A3::recursive_find(SceneNode* m_rootNode, unsigned int node_id){
    if(m_rootNode->m_nodeId==node_id||m_rootNode==NULL){
        return m_rootNode;
    }
    SceneNode* temp;
    for(SceneNode*node : m_rootNode->children){
        temp = recursive_find(node,node_id);
        if(temp!=NULL){
            return temp;
        }
    }
    return NULL;
}


Command* A3::get_cur_command(){
    Command* actor = NULL;
    list<Command*>::iterator it= my_commands.begin();
    if(iterator_counter<my_commands.size()&&iterator_counter>0){
        advance(it,iterator_counter);
        actor = *it;
    }
    return actor;
}



SceneNode* A3::recursive_find_byname(SceneNode* m_rootNode,string s){
    if(m_rootNode->m_name==s||m_rootNode==NULL){
        return m_rootNode;
    }
    SceneNode* temp;
    for(SceneNode*node : m_rootNode->children){
        temp = recursive_find_byname(node,s);
        if(temp!=NULL){
            return temp;
        }
    }
    return NULL;
}


void A3::print_mat4(mat4 m){
    for(int i = 0;i<4;++i){
        for(int j = 0;j<4;++j){
            cout << m[i][j] << ' ';
        }
        cout << endl;
    }
    cout << endl;
}

void A3::print_vector(vector<SceneNode*> v){
    for(int i = 0;i<v.size();++i){
        cout << "my name is " << v[i]->m_name << endl;
    }
    cout << endl;
}


void A3::undo(){
    if(!my_commands.empty()){
        cout << "iterator_counter is " << iterator_counter<< endl;
    
        list<Command*>::iterator it= my_commands.begin();
        advance(it,iterator_counter);
        Command* actor = *it;
        if(actor==NULL){
            cout << "actor is null" << endl;
        }
        //Joint_Rotate* temp_act = dynamic_cast<Joint_Rotate*>(actor);
         cout << "before undo " <<endl;
        
        SceneNode* temp_root;
        mat4 temp_trans;
        mat4 matrix_to_be;
        
        actor->undo();
        
        
        iterator_counter--;
        
    }
}


void A3::redo(){
    if(!my_commands.empty()){
        iterator_counter++;
        if(iterator_counter+1>=my_commands.size()){
            return;
        }
        list<Command*>::iterator it = my_commands.begin();
        advance(it,iterator_counter);
        Command* actor = *it;
        actor->execute();
    }
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
    double diff_x = xPos-cur_x;
    double diff_y = yPos-cur_y;
    GLfloat my_x_angle = diff_x*0.01;
    GLfloat my_y_angle = diff_y*0.01;
    
    if(mode==0){
        if(left){
            m_rootNode->translate(vec3(my_x_angle, -my_y_angle, 0.0f));
        }
        if(middle){
            m_rootNode->translate(vec3(0.0f, 0.0f, my_y_angle));
        }
        if(right){
            
            float x_cen = m_windowWidth/2;
            float y_cen = m_windowHeight/2;
            float fnewx = xPos-x_cen;
            float fnewy = yPos-y_cen;
            float foldx = cur_x-x_cen;
            float foldy = cur_y-y_cen;
            float f_diameter;
            float fvecx,fvecy,fvecz;
            if(m_windowWidth<m_windowHeight){
                f_diameter = m_windowWidth*0.5;
            }else{
                f_diameter = m_windowHeight*0.5;
            }
            vCalcRotVec(fnewx,fnewy,foldx,foldy,f_diameter,&fvecx,&fvecy,&fvecz);
            mat4 new_matrix;
            vAxisRotMatrix(-fvecx,fvecy,-fvecz,new_matrix);
            //print_mat4(new_matrix);
            recursive_rotate(m_rootNode.get(),m_rootNode->trans,new_matrix);
            
            
        }
    } else if(mode==1){
        if(middle){
           // SceneNode* temp_node;
            for(int i = 0;i<selected_nodes.size();++i){
                SceneNode* temp_node = selected_nodes[i];
                list<SceneNode*>::iterator it = temp_node->children.begin();
                SceneNode* temp_root = *it;
                mat4 matrix = rotate(mat4(),my_y_angle,vec3(1,0,0));
                JointNode *temp_joint = static_cast<JointNode*>(temp_node);
                GLfloat cur_range = my_y_angle+temp_joint->m_joint_x.init;
                
                if(cur_range>=temp_joint->m_joint_x.min&&cur_range<=temp_joint->m_joint_x.max){
                    temp_joint->m_joint_x.init = cur_range;
                    cout << "cur range is " << cur_range << " x min is " << temp_joint->m_joint_x.min << " x max is " << temp_joint->m_joint_x.max << endl;
                    recursive_rotate(temp_root,temp_node->trans,matrix);
                }
            }
            
            Command* temp_cmd = get_cur_command();
            if(temp_cmd!=NULL){
                Joint_Rotate* temp_rotate = dynamic_cast<Joint_Rotate*>(temp_cmd);
                temp_rotate->angle_common += my_y_angle;
            }
            
        }
        if(right){
            //cout << "right button" << endl;
            //SceneNode* head_node = recursive_find_byname(m_rootNode.get(),"head");
            SceneNode* head_node = recursive_find_byname(m_rootNode.get(),"headjoint");
            JointNode *temp_joint = static_cast<JointNode*>(head_node);
            if(head_node!=NULL&&head_node->isSelected){
                vec3 transform_vec = vec3(0,1,0);
                cout << "x angle is " << my_x_angle << endl;
                mat4 rotate_m = rotate(mat4(),my_x_angle,transform_vec);
                GLfloat cur_range = my_x_angle+temp_joint->m_joint_x.init;
                if(cur_range>=temp_joint->m_joint_x.min&&cur_range<=temp_joint->m_joint_x.max){
                    temp_joint->m_joint_x.init = cur_range;
                    recursive_rotate(head_node,head_node->trans,rotate_m);
                }
                
            }
        }
    }

    cur_x = xPos;
    cur_y = yPos;
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
    if (!ImGui::IsMouseHoveringAnyWindow()) {
        if(button==GLFW_MOUSE_BUTTON_LEFT){
            switch (actions) {
                case GLFW_PRESS:
                    left = true;
                    if(mode==1){
                        double xpos, ypos;
                        glfwGetCursorPos( m_window, &xpos, &ypos );
                        
                        picking = true;
                        
                        uploadCommonSceneUniforms();
                        glClearColor(1.0, 1.0, 1.0, 1.0 );
                        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                        glClearColor(0.35, 0.35, 0.35, 1.0);
                        
                        draw();
                        
                        // I don't know if these are really necessary anymore.
                        // glFlush();
                        // glFinish();
                        
                        CHECK_GL_ERRORS;
                        
                        // Ugly -- FB coordinates might be different than Window coordinates
                        // (e.g., on a retina display).  Must compensate.
                        xpos *= double(m_framebufferWidth) / double(m_windowWidth);
                        // WTF, don't know why I have to measure y relative to the bottom of
                        // the window in this case.
                        ypos = m_windowHeight - ypos;
                        ypos *= double(m_framebufferHeight) / double(m_windowHeight);
                        
                        GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
                        // A bit ugly -- don't want to swap the just-drawn false colours
                        // to the screen, so read from the back buffer.
                        glReadBuffer( GL_BACK );
                        // Actually read the pixel at the mouse location.
                        glReadPixels( int(xpos), int(ypos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
                        CHECK_GL_ERRORS;
                        
                        unsigned int first = buffer[0];
                        unsigned int second = buffer[1];
                        unsigned int third = buffer[2];
                        cout << "buffer item are " << first  << ' ' << second << ' ' << third << endl;
                        
                        // Reassemble the object ID.
                        unsigned int what = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);
                    /*double getx, gety;
                        glfwGetCursorPos(m_window,&getx,&gety);
                       // struct{ GLubyte first, second, third,forth; } pixel;
                        GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
                        glReadPixels(getx, gety, 1, 1, GL_RGBA, GL_UNSIGNED_INT, &buffer);
                        unsigned int what = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);
                        cout << "obj is  is " << what << endl;*/
                        
                        int nodes_num = m_rootNode->totalSceneNodes();
                        if(what<nodes_num){
                            SceneNode *cur_node = recursive_find(m_rootNode.get(),what);
                            if(cur_node!=NULL){
                                //parent is not torso
                                cout << "cur node not null !" << endl;
                                cout << "cur node name is " << endl;
                                if(cur_node->parent->m_nodeType==NodeType::JointNode){
                                    cur_node->isSelected = !cur_node->isSelected;
                                    cur_node->parent->isSelected = !cur_node->parent->isSelected;
                                    if(cur_node->parent->isSelected){
                                        JointNode *temp_parent;
                                        temp_parent = static_cast<JointNode*>(cur_node->parent);
                                        selected_nodes.push_back(temp_parent);
                                    }else{
                                        for(int i = 0;i<selected_nodes.size();++i){
                                            if(selected_nodes[i]->m_name==cur_node->parent->m_name){
                                                selected_nodes.erase(selected_nodes.begin()+i);
                                            }
                                        }
                                    }
                                    cout << "cur_node name is " << cur_node->m_name << endl;
                                    GeometryNode *cur_node_geo;
                                    if(cur_node->m_nodeType==NodeType::GeometryNode){
                                        cur_node_geo = static_cast<GeometryNode *>(cur_node);
                                    }
                                    
                                    vec3 cur_node_kd = vec3(1,1,1);
                                    vec3 cur_node_ks = vec3(0.1,0.1,0.1);
                                    
                                    if(cur_node_geo->isSelected){
                                        cur_node_geo->true_material = cur_node_geo->material;
                                        cur_node_geo->material.kd = cur_node_kd;
                                        cur_node_geo->material.ks = cur_node_ks;
                                    }else{
                                        cur_node_geo->material.kd = cur_node_geo->true_material.kd;
                                        cur_node_geo->material.ks = cur_node_geo->true_material.ks;
                                    }
                                }
                            }
                        }
                        picking = false;
                        
                        
        
                        
                    }
                    break;
                case GLFW_RELEASE:
                    left = false;
                    break;
                default:
                    break;
            }
        }
        if(button==GLFW_MOUSE_BUTTON_MIDDLE){
            Joint_Rotate* cmd;
            if(actions==GLFW_PRESS){
                middle = true;
                //print_vector(selected_nodes);
                cmd = new Joint_Rotate(selected_nodes);
                my_commands.push_back(cmd);
                iterator_counter = my_commands.size()-1;
                cur_command = cmd;
            } else if(actions==GLFW_RELEASE){
                middle = false;
                /*Command* temp_c = get_cur_command();
                my_commands.push_back(temp_c);
                iterator_counter = my_commands.size()-1;*/
            }
            /*switch (actions==GLFW_PRESS) {
                case GLFW_PRESS:{
                    middle = true;
                    print_vector(selected_nodes);
                    cmd= new Joint_Rotate(selected_nodes);
                    my_commands.push_back(cmd);
                    iterator_counter = my_commands.size()-1;
                    break;
                }
                case GLFW_RELEASE:
                    middle = false;
                    break;
                default:
                    break;*/
        }
        if(button==GLFW_MOUSE_BUTTON_RIGHT){
            switch (actions) {
                case GLFW_PRESS:
                    right = true;
                    break;
                case GLFW_RELEASE:
                    right = false;
                    break;
                default:
                    break;
            }
        }
    }

	return eventHandled;
}
    

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
        switch(key){
            case GLFW_KEY_P:
                mode = 0;
                break;
            case GLFW_KEY_J:
                mode = 1;
                break;
            case GLFW_KEY_Q:
                glfwSetWindowShouldClose(m_window, GL_TRUE);
                break;
            case GLFW_KEY_C:
                can_render_circle=!can_render_circle;
                break;
            case GLFW_KEY_Z:
                z_buffer_on = !z_buffer_on;
                break;
            case GLFW_KEY_B:
                back_face = !back_face;
                break;
            case GLFW_KEY_F:
                front_face = !front_face;
                break;
            default:
                break;
                
        }
	}
	// Fill in with event handling code...

	return eventHandled;
}


//piazza demo
void A3::vCalcRotVec(float fNewX, float fNewY,
                 float fOldX, float fOldY,
                 float fDiameter,
                 float *fVecX, float *fVecY, float *fVecZ) {
    long  nXOrigin, nYOrigin;
    float fNewVecX, fNewVecY, fNewVecZ,        /* Vector corresponding to new mouse location */
    fOldVecX, fOldVecY, fOldVecZ,        /* Vector corresponding to old mouse location */
    fLength;
    
    /* Vector pointing from center of virtual trackball to
     * new mouse position
     */
    fNewVecX    = fNewX * 2.0 / fDiameter;
    fNewVecY    = fNewY * 2.0 / fDiameter;
    fNewVecZ    = (1.0 - fNewVecX * fNewVecX - fNewVecY * fNewVecY);
    
    /* If the Z component is less than 0, the mouse point
     * falls outside of the trackball which is interpreted
     * as rotation about the Z axis.
     */
    if (fNewVecZ < 0.0) {
        fLength = sqrt(1.0 - fNewVecZ);
        fNewVecZ  = 0.0;
        fNewVecX /= fLength;
        fNewVecY /= fLength;
    } else {
        fNewVecZ = sqrt(fNewVecZ);
    }
    
    /* Vector pointing from center of virtual trackball to
     * old mouse position
     */
    fOldVecX    = fOldX * 2.0 / fDiameter;
    fOldVecY    = fOldY * 2.0 / fDiameter;
    fOldVecZ    = (1.0 - fOldVecX * fOldVecX - fOldVecY * fOldVecY);
    
    /* If the Z component is less than 0, the mouse point
     * falls outside of the trackball which is interpreted
     * as rotation about the Z axis.
     */
    if (fOldVecZ < 0.0) {
        fLength = sqrt(1.0 - fOldVecZ);
        fOldVecZ  = 0.0;
        fOldVecX /= fLength;
        fOldVecY /= fLength;
    } else {
        fOldVecZ = sqrt(fOldVecZ);
    }
    
    /* Generate rotation vector by calculating cross product:
     *
     * fOldVec x fNewVec.
     *
     * The rotation vector is the axis of rotation
     * and is non-unit length since the length of a crossproduct
     * is related to the angle between fOldVec and fNewVec which we need
     * in order to perform the rotation.
     */
    *fVecX = fOldVecY * fNewVecZ - fNewVecY * fOldVecZ;
    *fVecY = fOldVecZ * fNewVecX - fNewVecZ * fOldVecX;
    *fVecZ = fOldVecX * fNewVecY - fNewVecX * fOldVecY;
}

/*******************************************************
 * void vAxisRotMatrix(float fVecX, float fVecY, float fVecZ, Matrix mNewMat)
 *
 *    Calculate the rotation matrix for rotation about an arbitrary axis.
 *
 *    The axis of rotation is specified by (fVecX,fVecY,fVecZ). The length
 *    of the vector is the amount to rotate by.
 *
 * Parameters: fVecX,fVecY,fVecZ - Axis of rotation
 *             mNewMat - Output matrix of rotation in column-major format
 *                       (ie, translation components are in column 3 on rows
 *                       0,1, and 2).
 *
 *******************************************************/
void A3::vAxisRotMatrix(float fVecX, float fVecY, float fVecZ, mat4& mNewMat) {
    float fRadians, fInvLength, fNewVecX, fNewVecY, fNewVecZ;
    
    /* Find the length of the vector which is the angle of rotation
     * (in radians)
     */
    fRadians = sqrt(fVecX * fVecX + fVecY * fVecY + fVecZ * fVecZ);
    
    /* If the vector has zero length - return the identity matrix */
    if (fRadians > -0.000001 && fRadians < 0.000001) {
        mNewMat = mat4(1,0,0,0,
                       0,1,0,0,
                       0,0,1,0,
                       0,0,0,1);
        return;
    }
    
    /* Normalize the rotation vector now in preparation for making
     * rotation matrix.
     */
    fInvLength = 1 / fRadians;
    fNewVecX   = fVecX * fInvLength;
    fNewVecY   = fVecY * fInvLength;
    fNewVecZ   = fVecZ * fInvLength;
    
    /* Create the arbitrary axis rotation matrix */
    double dSinAlpha = sin(fRadians);
    double dCosAlpha = cos(fRadians);
    double dT = 1 - dCosAlpha;
    
    mNewMat[0][0] = dCosAlpha + fNewVecX*fNewVecX*dT;
    mNewMat[0][1] = fNewVecX*fNewVecY*dT + fNewVecZ*dSinAlpha;
    mNewMat[0][2] = fNewVecX*fNewVecZ*dT - fNewVecY*dSinAlpha;
    mNewMat[0][3] = 0;
    
    mNewMat[1][0] = fNewVecX*fNewVecY*dT - dSinAlpha*fNewVecZ;
    mNewMat[1][1] = dCosAlpha + fNewVecY*fNewVecY*dT;
    mNewMat[1][2] = fNewVecY*fNewVecZ*dT + dSinAlpha*fNewVecX;
    mNewMat[1][3] = 0;
    
    mNewMat[2][0] = fNewVecZ*fNewVecX*dT + dSinAlpha*fNewVecY;
    mNewMat[2][1] = fNewVecZ*fNewVecY*dT - dSinAlpha*fNewVecX;
    mNewMat[2][2] = dCosAlpha + fNewVecZ*fNewVecZ*dT;
    mNewMat[2][3] = 0;
    
    mNewMat[3][0] = 0;
    mNewMat[3][1] = 0;
    mNewMat[3][2] = 0;
    mNewMat[3][3] = 1;
}


Joint_Rotate::Joint_Rotate(vector<JointNode*> v):Command(v){
    for(int i = 0;i<v.size();++i){
        JointNode * temp = v[i];
       // JointNode *temp_joint = static_cast<JointNode*>(temp);
        my_angles.push_back(temp->m_joint_x.init);
    }
    
}

void Joint_Rotate::execute(){
    for(int i = 0;i<my_actors.size();++i){
        JointNode* temp = my_actors[i];
        /*SceneNode* temp_scene = static_cast<SceneNode*>(temp);
        list<SceneNode*>::iterator it = temp_scene->children.begin();
        SceneNode* temp_root = *it;
        
        mat4 matrix = rotate(mat4(),angle_common,vec3(1,0,0));
        
        GLfloat cur_range = angle_common+temp->m_joint_x.init;
        
        if(cur_range>=temp->m_joint_x.min&&cur_range<=temp->m_joint_x.max){
            temp_scene->recursive_rotate(temp_root,temp_scene->trans,matrix);
        }*/
        //temp->rotate_joint(angle_common);
    }
}
        


void Joint_Rotate::undo(){
    cout << "before undo here " << endl;
    for(int i = 0;i<my_actors.size();++i){
        JointNode* temp = my_actors[i];
        temp->m_joint_x.init = my_angles[i];
        
        SceneNode* temp_scene = static_cast<SceneNode*>(temp);
        list<SceneNode*>::iterator it = temp_scene->children.begin();
        SceneNode* tr = *it;
        
        //mat4 matrix = rotate(mat4(),temp->m_joint_x.init ,vec3(1,0,0));
        
        //temp_scene->recursive_rotate(temp_root,temp_scene->trans,matrix);*/
        //cout << "before roatte" << endl;
        //temp->rotate_joint(temp->m_joint_x.init);
        
    }
}

