#pragma once

#include <set>
#include "brclcpp/brclcpp.h"
#include "brclcpp/timer/timer.h"
//#include <Dev_control.pb.h>
//#include <Hoister.pb.h>
#include "config.h"
#include "modbus.h"
#include "modbus_manager.h"
#include "singleton.h"
#include <yaml-cpp/yaml.h>
#include <lib_demo/dev_log.h>

#define MODBUS_COMMON_REG_ADDRESS					0x0000
#define MODBUS_GUAWANG_REG_ADDRESS					0x0400
#define MODBUS_DIZHUAN1_REG_ADDRESS					0x0800
#define MODBUS_PAINT_REG_ADDRESS					0x0C00
#define MODBUS_WALLHANDING_REG_ADDRESS				0x1000

#define MODBUS_QZPT_REG_ADDRESS                     0x1800
#define MODBUS_LZT_REG_ADDRESS                      0x2000

#define MODBUS_INPUT_REG_ADDRESS_OFFSET				0x4000

enum AGVModbusRosFuncCode {
    ROS_TO_MODBUS_CMD_WRITE = 0x05,
    ROS_TO_MODBUS_CMD_READ = 0x06,
};

// modbus帧消息转化结构体
typedef struct
{
    uint8_t  cmd;
    uint16_t addr;
    uint16_t num;
    std::vector<uint16_t> data;
} ModbusMessage;

class modbus;
class ModbusBase
{
public:

    //virtual void recived_top_callback(void* mem_addr){}

protected:

    void modbus_reconnect()
    {
        YAML::Node modbus_config = YAML::LoadFile("/usr/local/bzl_robot/ewsr_ms/config/basket_control.yaml");

        mb.reset(new modbus(modbus_config["ip"].as<string>(), modbus_config["port"].as<int>()));
        // set slave id
        mb->modbus_set_slave_id(1);
        // connect with the server
        mb->modbus_connect();
    }

    void init_modbus_base(brclcpp::Node::SharedPtr &nh)
    {
        modbus_ctrl_manager = Singleton<ModbusCmdManager>::GetInstance();

        YAML::Node modbus_config = YAML::LoadFile("/usr/local/bzl_robot/ewsr_ms/config/basket_control.yaml");

        //std::cout << "ip:" << modbus_config["ip"].as<string>() << std::endl;
        //std::cout << "port:" << modbus_config["port"].as<int>() << std::endl;

        mb = std::shared_ptr<modbus>(new modbus(modbus_config["ip"].as<string>(), modbus_config["port"].as<int>()));
        // set slave id
        mb->modbus_set_slave_id(1);
        // connect with the server
        mb->modbus_connect();
        sleep(1);

        send_read_all_reg_cmd();
        
        std::thread([this]() {
		brclcpp::Rate rate(1.0);

            while (brclcpp::Ok())
            {
                connect_to_plc();
                rate.Sleep();
            }
        }).detach();

        DevStatus_Info("modbus_mode init mb->_connected {}", mb->_connected);
        
        // 寄存器定时读写
        static std::shared_ptr<brclcpp::Timer> sWriteRegTimer;    
        sWriteRegTimer = std::make_shared<decltype(sWriteRegTimer)::element_type>(15*1000000, [this]() {
            //connect_to_plc();
            //send_read_all_reg_cmd();
            send_modbus_command();
        }, false);//   period(), callback(), oneshot()     period unit :ms
        sWriteRegTimer->Start();
        
        DevStatus_Info("modbus_mode init ok");

        m_last_recv_time = time(0);
    }

    void add_write_reg_map(uint16_t reg_addr, uint16_t reg_num, void* mem_addr)
    {
        S_REG_MEM_MAP map;
        map.reg_addr = reg_addr;
        map.reg_num = reg_num;
        map.mem_addr = mem_addr;

        m_write_reg_map.push_back(map);

        memset(mem_addr, 0, reg_num*2);
    }

    void add_read_reg_map(uint16_t reg_addr, uint16_t reg_num, void* mem_addr)
    {
        S_REG_MEM_MAP map;
        map.reg_addr = reg_addr;
        map.reg_num = reg_num;
        map.mem_addr = mem_addr;

        m_read_reg_map.push_back(map);
        memset(mem_addr, 0, reg_num*2);
    }

    template <typename T1, typename T2>
    void write_modbus_reg_data(T1& data1, T2&data2)
    {
        if((unsigned long)&data1 > (unsigned long)&data2)
        {
            write_modbus_reg_data(data2, data1);
            return;
        }

        for(auto it = m_write_reg_map.begin(); it != m_write_reg_map.end(); ++it)
        {
            int16_t offset = (unsigned long)&data1 - (unsigned long)it->mem_addr;
            //std::cout << "offset " << offset << std::endl;
            if(offset >= it->reg_num*2 || offset < 0)
            {
                continue;
            }
            //std::cout << "it->reg_addr " << it->reg_addr << std::endl;
            modbus_ctrl_manager->add_cmd_next(ROS_TO_MODBUS_CMD_WRITE,
                it->reg_addr + offset/2,
                sizeof(T2) + (unsigned long)&data2 - (unsigned long)&data1,
                (uint16_t *)&data1);
        }
    }

    template <typename T>
    void write_modbus_reg_data(T& data)
    {
        write_modbus_reg_data(data, data);
    }

    template <typename T1, typename T2>
    void read_modbus_reg_data(T1& data1, T2&data2)
    {
        if((unsigned long)&data1 > (unsigned long)&data2)
        {
            read_modbus_reg_data(data2, data1);
            return;
        }

        #define MODBUS_MAX_READ 240
        int len = 0,total_len = 0;
        for(len = 0,total_len = 0;total_len < sizeof(T2) + (unsigned long)&data2 - (unsigned long)&data1; )
        {
            len = MODBUS_MAX_READ<(sizeof(T2) + (unsigned long)&data2 - (unsigned long)&data1 - total_len)?MODBUS_MAX_READ:(sizeof(T2) + (unsigned long)&data2 - (unsigned long)&data1 - total_len);

            for(auto it = m_read_reg_map.begin(); it != m_read_reg_map.end(); ++it)
            {
                int16_t offset = (unsigned long)&data1 - (unsigned long)it->mem_addr + total_len;

                if(offset >= it->reg_num*2 || offset < 0)
                {
                    continue;
                }
                
                modbus_ctrl_manager->add_cmd(ROS_TO_MODBUS_CMD_READ,
                    it->reg_addr + offset/2,
                    len,
                    (uint16_t *)&data1 + total_len/2 );

            }

		    total_len += len;	
        }
    }

    template <typename T>
    void read_modbus_reg_data(T& data)
    {
        read_modbus_reg_data(data, data);
    }

    void send_read_taget_reg_cmd(uint16_t addr)
    {
        for(auto it = m_read_reg_map.begin(); it != m_read_reg_map.end(); ++it)
        {   
            if(it->reg_addr == addr)
            {
                #define MODBUS_MAX_READ 240
                int len = 0,total_len = 0;
                
                for(len = 0,total_len = 0;total_len < 2*it->reg_num; )
                {
                    len = MODBUS_MAX_READ<(2*it->reg_num - total_len)?MODBUS_MAX_READ:(2*it->reg_num - total_len);

                    int16_t offset = total_len;

                    if(offset >= it->reg_num*2 || offset < 0)
                    {
                        continue;
                    }
                    //std::cout << "reg_addr " << it->reg_addr << "len " << len << std::endl;
                    modbus_ctrl_manager->add_cmd(ROS_TO_MODBUS_CMD_READ,
                        it->reg_addr + offset/2,
                        len,
                        (uint16_t *)it->mem_addr + total_len/2 );

                    total_len += len;	
                }
            }
        }
    }

    void send_read_all_reg_cmd()
    {
        for(auto it = m_read_reg_map.begin(); it != m_read_reg_map.end(); ++it)
        {            
            #define MODBUS_MAX_READ 240
            int len = 0,total_len = 0;
            
            for(len = 0,total_len = 0;total_len < 2*it->reg_num; )
            {
                len = MODBUS_MAX_READ<(2*it->reg_num - total_len)?MODBUS_MAX_READ:(2*it->reg_num - total_len);

                int16_t offset = total_len;

                if(offset >= it->reg_num*2 || offset < 0)
                {
                    continue;
                }
                //std::cout << "reg_addr " << it->reg_addr << "len " << len << std::endl;
                modbus_ctrl_manager->add_cmd(ROS_TO_MODBUS_CMD_READ,
                    it->reg_addr + offset/2,
                    len,
                    (uint16_t *)it->mem_addr + total_len/2 );

                total_len += len;	
            }
        }
    }

    void send_write_all_reg_cmd()
    {
        for(auto it = m_write_reg_map.begin(); it != m_write_reg_map.end(); ++it)
        {
            //Rotating_Info("addr={} num={}", it->reg_addr, it->reg_num);
            modbus_ctrl_manager->add_cmd_next(ROS_TO_MODBUS_CMD_WRITE,
                                it->reg_addr, 2*it->reg_num, (uint16_t*)it->mem_addr);
        }
    }

    void connect_to_plc()
    {
        if(!mb->_connected)
        {
            modbus_reconnect();
            DevStatus_Info("modbus_connect _connected:{}",mb->_connected);
        }
        else
        {
            m_last_recv_time = time(0);
        }
    }

    bool communicate_timeout()
    {
        return ((time(0) - m_last_recv_time) > 3);
    }

    bool check_plc_connect()
    {
        /****cyh****默认plc连接正常 start*****/
        return true;
        /****cyh****默认plc连接正常 end*****/
        // return mb->_connected;
    }

private:


    void send_modbus_command()
    {
        int ret=0;
        current_modbus_cmd = modbus_ctrl_manager->fetch_first_cmd();

        if(current_modbus_cmd.cmd == 0xFF)
		return ;
        
        //std::cout << "cmd size " << modbus_ctrl_manager->get_cmd_num() << std::endl;
        //std::cout << "reg_addr " << current_modbus_cmd.addr << "cmd_param " << *current_modbus_cmd.cmd_param << std::endl;

        /*
        static struct timeval start, end;
        double t1, t2;
        gettimeofday(&end, NULL);
        t1 = (double)start.tv_sec * 1000 + (double)start.tv_usec / 1000;
        t2 = (double)end.tv_sec * 1000 + (double)end.tv_usec / 1000;
        printf("time =%fms\n",t2-t1);
        gettimeofday(&start, NULL);
        */

        if(!mb->_connected)
        {
            modbus_ctrl_manager->clear_cmd();
            return ;
        }
        else
        {
            m_last_recv_time = time(0);
        }

        while(mb->_busy)
            usleep(1000);
        

        if(!mb->err && mb->_connected)
        {
            if(current_modbus_cmd.cmd==ROS_TO_MODBUS_CMD_READ)
            {
                ret = mb->modbus_read_holding_registers(current_modbus_cmd.addr, current_modbus_cmd.cmd_size/2, current_modbus_cmd.cmd_param);//modbus_read_input_registers
                if(ret != 0)
                {
                    std::cout << "current_modbus_cmd.addr = " << current_modbus_cmd.addr << std::endl;
                    DevStatus_Info("modbus_read_registers error {}",ret);
                    //for(int i=0;i<5;i++)
                    //{
                    //    usleep(15000);
                    //    ret = mb->modbus_read_holding_registers(current_modbus_cmd.addr, current_modbus_cmd.cmd_size/2, current_modbus_cmd.cmd_param);
                    //    DevStatus_Info("modbus_read_registers cycle {}",ret);
                    //    if(ret == 0)
                    //       i=3;
                    //}
                    //std::cout << "ret2 = " << ret << std::endl;
                }
            }
            else if(current_modbus_cmd.cmd==ROS_TO_MODBUS_CMD_WRITE)
            {
                //std::cout << "reg_addr " << current_modbus_cmd.addr << "cmd_param " << *current_modbus_cmd.cmd_param << std::endl;
                ret = mb->modbus_write_registers(current_modbus_cmd.addr, current_modbus_cmd.cmd_size/2, current_modbus_cmd.cmd_param);
                if(ret != 0)
                {
                    std::cout << "current_modbus_cmd.addr = " << current_modbus_cmd.addr << std::endl;
                    DevStatus_Info("modbus_write_registers error {}",ret);
                    //for(int i=0;i<5;i++)
                    //{
                    //    usleep(15000);
                    //    ret = mb->modbus_write_registers(current_modbus_cmd.addr, current_modbus_cmd.cmd_size/2, current_modbus_cmd.cmd_param);
                    //    DevStatus_Info("modbus_write_registers cycle {}",ret);
                    //    if(ret == 0)
                    //       i=3;
                    //}
                    //std::cout << "ret2 = " << ret << std::endl;
                }
                if(current_modbus_cmd.addr != 8 && current_modbus_cmd.addr != 5001)
                    DevStatus_Info("modbus_write_registers addr:{} cmd_size/2:{} cmd_param:{} " ,current_modbus_cmd.addr , current_modbus_cmd.cmd_size/2 , *current_modbus_cmd.cmd_param);
            }
            if(mb->err)
            {
                mb->modbus_close();
                DevStatus_Info("modbus_write_registers modbus_close");
                sleep(1);
            }
        }
        
    }

    std::shared_ptr<modbus> mb = nullptr;

    typedef struct
    {
        uint16_t reg_addr;
        uint16_t reg_num;
        void* mem_addr;
    }S_REG_MEM_MAP;

    std::list<S_REG_MEM_MAP> m_write_reg_map;
    std::list<S_REG_MEM_MAP> m_read_reg_map;

    ModbusCmdManager* modbus_ctrl_manager;
    M_CMD current_modbus_cmd;

    time_t m_last_recv_time;

};
