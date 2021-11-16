#ifndef ROBOT_MOTION_H
#define ROBOT_MOTION_H

#include <brclcpp/brclcpp.h>
#include <brclcpp/timer/timer.h>
#include <yaml-cpp/yaml.h>
#include <map>
#include <std_msgs/Int32.pb.h>
#include <nav_msgs/OccupancyGrid.pb.h>

#define M_TASK_CONTROL_RUN_FREQ     (50.0)



class RobotMotion
{
    public: 
        RobotMotion(brclcpp::Node::SharedPtr &nh);
        using MotionMsgType = bros::std_msgs::Int32;
        bros::std_msgs::Int32 motion_msg;
    public:
        void init_motion(brclcpp::Node::SharedPtr &nh);
        bool do_normol();

    public:
       
    private:
        std::shared_ptr<brclcpp::Publisher<MotionMsgType>> down_stop_pub = nullptr;//下降紧急事件停止处理信号
};
#endif