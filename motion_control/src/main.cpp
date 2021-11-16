#include <brclcpp/brclcpp.h>
#include <brclcpp/time/rate.h>
#include "robot_motion.h"

int main(int argc, char **argv)
{
    brclcpp::Init(argc,argv);

    auto node = brclcpp::CreateNode("motion_control");

    auto dev_control = new RobotMotion(node);

    sleep(10);
    
    brclcpp::Rate loop_rate(M_TASK_CONTROL_RUN_FREQ);
    while(brclcpp::Ok())
        {
            brclcpp::SpinSome(node);
            dev_control->do_normol();
            loop_rate.Sleep();
        }
}