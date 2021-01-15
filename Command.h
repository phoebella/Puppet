
#ifndef Command_h
#define Command_h

#include "SceneNode.hpp"
#include "JointNode.hpp"
#include <vector>

using namespace std;

class Command {
public:
    Command(vector<JointNode *> a):my_actors(a){};
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual ~Command(){};
    vector<JointNode *> my_actors;
};


class Joint_Rotate : public Command {
public:
    Joint_Rotate(vector<JointNode*> v);
    
    virtual void execute();
    virtual void undo();
    vector<GLfloat> my_angles;
    float angle_common = 0;
};

#endif
