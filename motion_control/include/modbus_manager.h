#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <string>

#include <stdint.h>

#include <jsoncpp/json/json.h>

typedef struct
{
    uint8_t cmd;
    uint16_t addr;
    uint16_t cmd_size;
    uint16_t *cmd_param;
}M_CMD;

class ModbusManager
{
public:

    void add_cmd(uint8_t cmd, uint16_t addr, uint16_t cmd_size, uint16_t *cmd_param)//int cmd_id, int cmd_addr, uint32_t cmd_param
    {
        M_CMD cmd_temp;
        cmd_temp.cmd = cmd;
        cmd_temp.addr = addr;
        cmd_temp.cmd_size = cmd_size;
        cmd_temp.cmd_param = cmd_param;

        pthread_mutex_lock(&m_cmd_lock);
        m_cmd_todo_list.push_back(cmd_temp);
        pthread_mutex_unlock(&m_cmd_lock);
    }

    void add_cmd_next(uint8_t cmd, uint16_t addr, uint16_t cmd_size, uint16_t *cmd_param)//int cmd_id, int cmd_addr, uint32_t cmd_param
    {
        M_CMD cmd_temp;
        cmd_temp.cmd = cmd;
        cmd_temp.addr = addr;
        cmd_temp.cmd_size = cmd_size;
        cmd_temp.cmd_param = cmd_param;

        pthread_mutex_lock(&m_cmd_lock);
        m_cmd_todo_list.insert(m_cmd_todo_list.begin(),cmd_temp);
        pthread_mutex_unlock(&m_cmd_lock);
    }

    int get_cmd_num()
    {
        return m_cmd_todo_list.size();
    }

    void clear_cmd()
    {
        pthread_mutex_lock(&m_cmd_lock);
        m_cmd_exec_list.clear();
        m_cmd_todo_list.clear();
        pthread_mutex_unlock(&m_cmd_lock);
    }

    M_CMD fetch_lask_cmd(bool pop = true)
    {
        if(m_cmd_todo_list.size() == 0)
        {
            static M_CMD invalid_cmd = {0xFF, 0, 0, 0};
            return invalid_cmd;
        }

        pthread_mutex_lock(&m_cmd_lock);
        M_CMD cmd = m_cmd_todo_list.back();
        if (pop)
        {
            if(m_record_exec)
            {
                m_cmd_exec_list.push_back(cmd);
            }
            m_cmd_todo_list.erase(m_cmd_todo_list.end());
        }
        pthread_mutex_unlock(&m_cmd_lock);

        return cmd;
    }

    M_CMD fetch_first_cmd(bool pop = true)
    {
        if(m_cmd_todo_list.size() == 0)
        {
            static M_CMD invalid_cmd = {0xFF, 0, 0, 0};
            return invalid_cmd;
        }

        pthread_mutex_lock(&m_cmd_lock);
        M_CMD cmd = m_cmd_todo_list.front();
        if (pop)
        {
            if(m_record_exec)
            {
                m_cmd_exec_list.push_back(cmd);
            }
            m_cmd_todo_list.erase(m_cmd_todo_list.begin());
        }
        pthread_mutex_unlock(&m_cmd_lock);

        return cmd;
    }

protected:

    // is_record_exec 记录已经执行的cmd
    ModbusManager(bool is_record_exec)
    {
        m_record_exec = is_record_exec;
        pthread_mutex_init(&m_cmd_lock, NULL);
    }

    std::vector<M_CMD> m_cmd_todo_list; // 待执行命令列表
    std::vector<M_CMD> m_cmd_exec_list; // 已执行命令列表

    pthread_mutex_t m_cmd_lock;

    bool m_record_exec;

};

class ModbusCmdManager : public ModbusManager
{
public:
    ModbusCmdManager():ModbusManager(false)
    {
    }
};


#endif // MODBUS_MANAGER_H
