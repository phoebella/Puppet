myroot = gr.node('root')

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
pink = gr.material({0.85, 0.75, 1.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
orange = gr.material({0.7, 0.25, 0.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.5, 0.8, 0.1}, {0.1, 0.1, 0.1}, 10)
lightgreen = gr.material({0.2, 0.87, 0.66}, {0.1, 0.1, 0.1}, 10)

torso = gr.mesh('sphere', 'torso')
myroot:add_child(torso)
torso:scale(0.6, 0.85, 0.5)
torso:set_material(pink)

leftshoulder = gr.mesh('sphere', 'leftshoulder')
torso:add_child(leftshoulder)
leftshoulder:scale(0.5, 0.25, 0.4)
leftshoulder:translate(-0.45, 0.7, 0.0)
leftshoulder:set_material(orange)

leftshouler_joint = gr.joint('leftshouler_joint', {-3.14, 0, 0.78},{-1.57, 0,0.78})
leftshoulder:add_child(leftshouler_joint)
leftshouler_joint:translate(-0.8, 0.7, 0.0)

rightshoulder = gr.mesh('sphere', 'rightshoulder')
torso:add_child(rightshoulder)
rightshoulder:scale(0.5, 0.25, 0.4)
rightshoulder:translate(0.45, 0.7, 0.0)
rightshoulder:set_material(orange)

rightshoulder_joint = gr.joint('rightshoulder_joint',{-3.14, 0, 0.78},{-1.57, 0,0.78})
rightshoulder:add_child(rightshoulder_joint)
rightshoulder_joint:translate(0.45, 0.7, 0.0)

neck = gr.mesh('sphere', 'neck')
neck:scale(0.2, 0.4, 0.2)
neck:translate(0.0, 0.4, 0.0)
neck:set_material(black)

neckjoint = gr.joint('neckjoint', {-3.14, 0, 3.14},{-1.57, 0,1.57})
neckjoint:add_child(neck)
torso:add_child(neckjoint)
neckjoint:translate(0.0, 0.5, 0.0)

head = gr.mesh('sphere', 'head')
head:scale(0.45, 0.45, 0.45)
head:translate(0.0, 0.05, 0.0)
head:set_material(red)

left_eye = gr.mesh('sphere', 'left_eye')
head:add_child(left_eye)
left_eye:scale(0.1, 0.1, 0.1)
left_eye:translate(-0.18, 0.05, 0.32)
left_eye:set_material(black)

right_eye = gr.mesh('sphere', 'right_eye')
head:add_child(right_eye)
right_eye:scale(0.1, 0.1, 0.1)
right_eye:translate(0.18, 0.05, 0.32)
right_eye:set_material(black)




headjoint = gr.joint('headjoint', {-0.78, 0, 0.78},{-1.5, 0,1.5})
headjoint:add_child(head)
neck:add_child(headjoint)
headjoint:translate(0.0, 1.4, 0.0)

left_upperarm = gr.mesh('sphere', 'left_upperarm')
leftshouler_joint:add_child(left_upperarm)
left_upperarm:scale(0.16, 0.4, 0.16)
left_upperarm:translate(-0.8, 0.2, 0.0)
left_upperarm:set_material(green)

left_upperarm_joint = gr.joint('left_upperarm_joint',{-1.57, 0, 0.175},{-1.57, 0,1.57})
left_upperarm:add_child(left_upperarm_joint)
left_upperarm_joint:translate(-0.8, -0.15, 0.0)

left_lowerarm = gr.mesh('sphere', 'left_lowerarm')
left_upperarm_joint:add_child(left_lowerarm)
left_lowerarm:scale(0.12, 0.4, 0.12)
left_lowerarm:translate(-0.8, -0.4, 0.0)
left_lowerarm:set_material(green)

left_lowerarm_joint = gr.joint('left_lowerarm_joint',{-0.78, 0, 0.78},{-1.57, 0,1.57})
left_lowerarm:add_child(left_lowerarm_joint)
left_lowerarm_joint:translate(-0.8, -0.65, 0.0)

lefthand = gr.mesh('cube', 'lefthand')
left_lowerarm_joint:add_child(lefthand)
lefthand:scale(0.15, 0.15, 0.15)
lefthand:translate(-0.8, -0.8, 0.0)
lefthand:set_material(blue)

right_upperarm = gr.mesh('sphere', 'right_upperarm')
rightshoulder_joint:add_child(right_upperarm)
right_upperarm :scale(0.16, 0.4, 0.16)
right_upperarm :translate(0.8, 0.2, 0.0)
right_upperarm :set_material(green)

right_upperarm_joint = gr.joint('right_upperarm_joint',{-1.57, 0, 0.175},{-1.57, 0,1.57})
right_upperarm:add_child(right_upperarm_joint)
right_upperarm_joint:translate(0.8, -0.15, 0.0)

right_lowerarm = gr.mesh('sphere', 'right_lowerarm')
right_upperarm_joint:add_child(right_lowerarm)
right_lowerarm:scale(0.12, 0.4, 0.12)
right_lowerarm:translate(0.8, -0.4, 0.0)
right_lowerarm:set_material(green)

right_lowerarm_joint = gr.joint('right_lowerarm_joint',{-0.78, 0, 0.78},{-1.57, 0,1.57})
right_lowerarm:add_child(right_lowerarm_joint)
right_lowerarm_joint:translate(0.8, -0.65, 0.0)

righthand =gr.mesh('cube', 'lefthand')
right_lowerarm_joint:add_child(righthand)
righthand:scale(0.15, 0.15, 0.15)
righthand:translate(0.8, -0.8, 0.0)
righthand:set_material(blue)

hip = gr.mesh('sphere', 'hip')
torso:add_child(hip)
hip:scale(0.5,0.25,0.2)
hip:translate(0.0, -0.8, 0.0)
hip:set_material(orange)

hip_left_joint = gr.joint('hip_left_joint',{-1.57, 0, 0.78},{-1.57, 0,1.57})
hip:add_child(hip_left_joint)
hip_left_joint:translate(-0.25,-0.9,0.0)

hip_right_joint = gr.joint('hip_right_joint',{-1.57, 0, 0.78},{-1.57, 0,1.57})
hip:add_child(hip_right_joint)
hip_right_joint:translate(0.25,-0.9,0.0)

left_thigh = gr.mesh('sphere', 'left_thigh')
hip_left_joint:add_child(left_thigh)
left_thigh :scale(0.2, 0.7, 0.2)
left_thigh :translate(-0.25, -1.3, 0.0)
left_thigh :set_material(green)

right_thigh = gr.mesh('sphere', 'right_thigh')
hip_right_joint:add_child(right_thigh)
right_thigh:scale(0.2, 0.7, 0.2)
right_thigh :translate(0.25, -1.3, 0.0)
right_thigh :set_material(green)

left_thigh_joint = gr.joint('left_thigh_joint',{-1.57, 0,1.57},{-1.57, 0,1.57})
left_thigh:add_child(left_thigh_joint)
left_thigh_joint:translate(-0.25, -1.8, 0.0)

right_thigh_joint = gr.joint('right_thigh_joint',{-1.57, 0,1.57},{-1.57, 0,1.57})
right_thigh:add_child(right_thigh_joint)
right_thigh_joint:translate(0.25, -1.8, 0.0)

leftcalf = gr.mesh('sphere', 'leftcalf')
left_thigh_joint:add_child(leftcalf)
leftcalf:scale(0.15, 0.6, 0.15)
leftcalf:translate(-0.25, -2.1, 0.0)
leftcalf :set_material(green)

rightcalf = gr.mesh('sphere', 'rightcalf')
right_thigh_joint:add_child(rightcalf)
rightcalf:scale(0.15, 0.6, 0.15)
rightcalf:translate(0.25, -2.1, 0.0)
rightcalf:set_material(green)

left_calf_joint = gr.joint('left_calf_joint', {-0.78, 0, 0.78}, {-0.78, 0, 0.78})
leftcalf:add_child(left_calf_joint)
left_calf_joint:translate(-0.25, -2.5, 0.0)

right_calf_joint = gr.joint('right_calf_joint', {-0.78, 0, 0.78}, {-0.78, 0, 0.78})
rightcalf:add_child(right_calf_joint)
right_calf_joint:translate(0.25, -2.5, 0.0)

left_foot = gr.mesh('sphere', 'left_foot')
left_calf_joint:add_child(left_foot);
left_foot:scale(0.15, 0.15, 0.3)
left_foot:translate(-0.25, -2.6, 0.0)
left_foot:set_material(lightgreen)

right_foot = gr.mesh('sphere', 'right_foot')
right_calf_joint:add_child(right_foot);
right_foot:scale(0.15, 0.15, 0.3)
right_foot:translate(0.25, -2.6, 0.0)
right_foot:set_material(lightgreen)




myroot:translate(0.0, 0.3, -5.5)

return myroot




