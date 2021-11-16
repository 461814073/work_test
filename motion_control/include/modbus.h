//
// Created by Fanzhe on 5/28/2017.
//
//#include <sys/socket.h>
#include <netinet/in.h>

#ifndef MODBUSPP_MODBUS_H
#define MODBUSPP_MODBUS_H

#define MAX_MSG_LENGTH 260

///Function Code
#define     READ_COILS        0x01
#define     READ_INPUT_BITS   0x02
#define     READ_REGS         0x03
#define     READ_INPUT_REGS   0x04
#define     WRITE_COIL        0x05
#define     WRITE_REG         0x06
#define     WRITE_COILS       0x0F
#define     WRITE_REGS        0x10

///Exception Codes

#define    EX_ILLEGAL_FUNCTION  0x01 // Function Code not Supported
#define    EX_ILLEGAL_ADDRESS   0x02 // Output Address not exists
#define    EX_ILLEGAL_VALUE     0x03 // Output Value not in Range
#define    EX_SERVER_FAILURE    0x04 // Slave Deive Fails to process request
#define    EX_ACKNOWLEDGE       0x05 // Service Need Long Time to Execute
#define    EX_SERVER_BUSY       0x06 // Server Was Unable to Accept MB Request PDU
#define    EX_NEGATIVE_ACK      0x07
#define    EX_MEM_PARITY_PROB   0x08
#define    EX_GATEWAY_PROBLEMP  0x0A // Gateway Path not Available
#define    EX_GATEWYA_PROBLEMF  0x0B // Target Device Failed to Response
#define    EX_BAD_DATA          0XFF // Bad Data lenght or Address

#define    BAD_CON              -1

/// Modbus Operator Class
/**
 * Modbus Operator Class
 * Providing networking support and mobus operation support.
 */
class modbus {
private:
    
    uint16_t PORT{};
    int _socket{};
    uint _msg_id{};
    int _slaveid{};
    std::string HOST;

    struct sockaddr_in _server{};


    inline void modbus_build_request(uint8_t *to_send, uint address, int func) const;

    int modbus_read(int address, uint amount, int func);
    int modbus_write(int address, uint amount, int func, const uint16_t *value);

    inline ssize_t modbus_send(uint8_t *to_send, int length);
    inline ssize_t modbus_receive(uint8_t *buffer) const;

    void modbuserror_handle(const uint8_t *msg, int func);

    inline void set_bad_con();
    inline void set_bad_input();


public:
    bool _connected{};
    bool _busy;
    bool err{};
    int err_no{};
    std::string error_msg;




    modbus(std::string host, uint16_t port);
    ~modbus();

    bool modbus_connect();
    void modbus_close();

    void modbus_set_slave_id(int id);

    int  modbus_read_coils(int address, int amount, bool* buffer);
    int  modbus_read_input_bits(int address, int amount, bool* buffer);
    int  modbus_read_holding_registers(int address, int amount, uint16_t *buffer);
    int  modbus_read_input_registers(int address, int amount, uint16_t *buffer);

    int  modbus_write_coil(int address, const bool& to_write);
    int  modbus_write_register(int address, const uint16_t& value);
    int  modbus_write_coils(int address, int amount, const bool *value);
    int  modbus_write_registers(int address, int amount, const uint16_t *value);


};



#endif //MODBUSPP_MODBUS_H
