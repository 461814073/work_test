/*
 * @Author: ltxmlsn
 * @Date: 2020-04-02 11:01:18
 * @LastEditors: ltxmlsn
 * @LastEditTime: 2020-04-07 09:50:33
 * @Description: file content
 */
/**
 * @file config.h
 * @author zaqaler
 * @brief  task control motion
 * @version 0.1
 * @date 2019-05-10
 *
 * @copyright Copyright (c) 2019
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define VERSION "version: 1.0.0.a.0  date: 20200407"

#define SUBSCRIBE_QUEUE_BUF_LEN         (10)
#define PUBLISH_QUEUE_BUF_LEN         (100)

#define SUB_COMMON_MESSAGE "/robot_sys_state"
#define PUB_COMMON_MESSAGE "/work_sys_state"
#define CLI_JSONRPC "jsonrpc_client"
#define SRV_JSONRPC "jsonrpc_service"
#define READY_ACTION "ready_action"
#define TASK_ACTION "task_action"


//
//#define ACTION_WAIT_FOR_SERVER_TIMEOUT  (20)   /* action wait for server timoute unit: second */
//#define TASK_CONTROL_TIMEOUT_THRESHOLD  (600)  /* task control timeout threshold uint: second */
#define M_TASK_CONTROL_RUN_FREQ         (50.0)  /* agv task control shedule frequency unit:Hz */
#define M_TASK_STATE_SEND_FREQ          (1)    /* agv task state send frequency unit:Hz */
#define WORK_TIMEOUT                    (600)  // ��װ������ʱʱ�� unit:s
//#define LINEAR_VEL_MAX                  (5000) /* linear speed maxinum mm/s */
//#define ANGULAR_VEL_MAX                 (100)  /* angular speed maxinum rad/s */
//#define LINE_TRAJ                       (1)    /* traj is line */
//
//#define ROS_PUB_TOPIC_SPEAKER_CMD  	   "/speaker_cmd"
//#define ROS_PUB_TOPIC_TASK_FINISHED    "/task_finished"
//#define ROS_PUB_TOPIC_MOTORS_RESET     "/motors_reset"
//#define ROS_PUB_TOPIC_JOY_CTRL_ENABLE  "/joy_ctrl_state_set"
//#define SUBSCRIBE_LASER_SCAN_STATUS    "/laser_scan_status"
//
#define SUBSCRIBE_CURRENT_POST_TOPIC   "/current_pose"
//#define ROS_TOPIC_INITIALPOSE          "/initialpose"
//
#define SUBSCRIBE_BRACKET_TOPIC         "HoistOutput"
#define PUBLISH_BRACKET_TOPIC           "HoistInput"
#define PUBLISH_BRACKET_PARM_TOPIC      "HoisterPar"
//
#define TOPIC_DEVICE_INPUT              "/device_control_request"
#define TOPIC_DEVICE_OUTPUT             "/device_control_response"

#define TOPIC_FROM_PAD                  "/ros_communication_request"
#define TOPIC_TO_PAD                    "/ros_communication_response"

#define TOPIC_CATCH_FAULT              "/fault_level"
//
//#define ACTION_MOTION_CONTROL          "/motion_control"
//
//#define SUBSCRIBE_MQTT_TOPIC           "/task_data"
//#define PUBLISH_MQTT_TOPIC             "/task_state"
//
//#define SUB_TOPIC_TASK_ACTION 		   "/task_instruction_action"
//
//#define ACTION_LOGIC_TOPS              "/logic_task_cmd"
//#define SUBSCRIBE_LOGIC_TASK_STATUS_TOPIC   "/logic_task_status"
//#define PUBLISH_MOVEMENT_TASK_STATUS_TOPIC  "/movement_task_status"
//#define SUBSCRIBE_SPI_DIN_TOPIC             "/agv_din"
//#define PUBLISH_SPI_DOUT_TOPIC              "/agv_dout"
//#define SUBSCRIBE_BATTERY_TOPIC             "/agv_bms"
//#define SUBSCRIBE_MILEAGE_TOPIC             "/mileage_data"
//
//
//#define SUBSCRIBE_DEVICE_ERROR_TOPIC        "/device_error_status"
//
//#define ROS_SUB_TOPIC_IOT_DEV2CLOUD "/iot/dev2cloud"
//#define ROS_PUT_TOPIC_IOT_CLOUD2DEV "/iot/cloud2dev"
//
////#define PUB_TOPIC_DEVICE_INFO_BASIC         "device_info_basic"
//#define PUB_TOPIC_DEVICE_INFO_BASIC         "/deviceinfo"
//#define PUB_TOPIC_DEVICE_INFO_EXCEPTION     "/device_info_exception"
//#define PUB_TOPIC_DEVICE_INFO_SWITCH_STATE  "/device_info_switch_state"
//#define PUB_TOPIC_DEVICE_INFO_TASK          "/device_info_task"
//
//#define PUB_TOPIC_NOTIFY                    "/deviceinfo/notify"
//#define PUB_TOPIC_CONNECT_ACK               "/connect-ack"
//#define PUB_TOPIC_ALARM                     "/alarm"

#define RUN_PER_SEC(xxx) \
    [](){\
        static int times = 0;\
        if(times ++ % M_TASK_CONTROL_RUN_FREQ == M_TASK_CONTROL_RUN_FREQ/2)\
        {\
            xxx;\
        }\
    }();

#define TO_ZERO(num) (fabs(num) < 0.00001 ? 0. : num)

#endif
