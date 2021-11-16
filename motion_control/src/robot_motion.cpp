#include "robot_motion.h"

RobotMotion::RobotMotion(brclcpp::Node::SharedPtr &nh)
{
    init_motion(nh);
}

void RobotMotion::init_motion(brclcpp::Node::SharedPtr &nh)
{
    // static brclcpp::Timer check_data(2000*1000000, [this](){
    // 	radar_pub(1);
    // }, false);
    //check_data.Start();
}

bool RobotMotion::do_normol()
{
    return true;
}





















