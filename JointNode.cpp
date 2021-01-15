// Winter 2019

#include "JointNode.hpp"
#include "cs488-framework/MathUtils.hpp"
#include <glm/gtx/transform.hpp>

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name)
{
	m_nodeType = NodeType::JointNode;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}
 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;
}


void JointNode::rotate_joint(float angle_to_be){
    if(m_joint_x.init+angle_to_be<m_joint_x.min){
        m_joint_x.init = m_joint_x.min;
    }else if(m_joint_x.init+angle_to_be>m_joint_x.max){
        m_joint_x.init = m_joint_x.max;
    }else{
        m_joint_x.init = m_joint_x.init + angle_to_be;
    }
    float cur_angle = m_joint_x.init;
    glm::mat4 rotate_matrix = glm::rotate(cur_angle,glm::vec3(1,0,0));
    trans = rotate_matrix*trans;
    std::cout << "after trans" << std::endl;
    invtrans = glm::inverse(trans);
}
