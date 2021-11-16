#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "modbus.h"

/**
 * Main Constructor of Modbus Connector Object
 * @param host IP Address of Host
 * @param port Port for the TCP Connection
 * @return     A Modbus Connector Object
 */
modbus::modbus(std::string host, uint16_t port=502) {
    HOST = host;
    PORT = port;
    _slaveid = 1;
    _msg_id = 1;
    _connected = false;
    err = false;
    err_no = 0;
    error_msg = "";
    _busy = false;
}



/**
 * Destructor of Modbus Connector Object
 */
modbus::~modbus(void) = default;


/**
 * Modbus Slave ID Setter
 * @param id  ID of the Modbus Server Slave
 */
void modbus::modbus_set_slave_id(int id) {
    _slaveid = id;
}



/**
 * Build up a Modbus/TCP Connection
 * @return   If A Connection Is Successfully Built
 */
bool modbus::modbus_connect() {
    if(HOST.empty() || PORT == 0) {
        std::cout << "Missing Host and Port" << std::endl;
        return false;
    } else {
        std::cout << "Found Proper Host "<< HOST << " and Port " <<PORT <<std::endl;
    }

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == -1) {
        std::cout <<"Error Opening Socket" <<std::endl;
        return false;
    } else {
        std::cout <<"Socket Opened Successfully" << std::endl;
    }

    struct timeval timeout{};
    timeout.tv_sec  = 20;  // after 20 seconds connect() will timeout
    timeout.tv_usec = 0;
    setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    _server.sin_family = AF_INET;
    _server.sin_addr.s_addr = inet_addr(HOST.c_str());
    _server.sin_port = htons(PORT);

    if (connect(_socket, (struct sockaddr*)&_server, sizeof(_server)) < 0) {
        std::cout<< "Connection Error" << std::endl;
        return false;
    }
    sleep(1);
    std::cout<< "Connected" <<std::endl;
    _connected = true;
    err = false;
    return true;
}


/**
 * Close the Modbus/TCP Connection
 */
void modbus::modbus_close() {
    close(_socket);
    _busy = false;
    _connected = false;
    std::cout <<"Socket Closed" <<std::endl;
}


/**
 * Modbus Request Builder
 * @param to_send   Message Buffer to Be Sent
 * @param address   Reference Address
 * @param func      Modbus Functional Code
 */
void modbus::modbus_build_request(uint8_t *to_send, uint address, int func) const {
    to_send[0] = (uint8_t) _msg_id >> 8u;
    to_send[1] = (uint8_t) (_msg_id & 0x00FFu);
    to_send[2] = 0;
    to_send[3] = 0;
    to_send[4] = 0;
    to_send[6] = (uint8_t) _slaveid;
    to_send[7] = (uint8_t) func;
    to_send[8] = (uint8_t) (address >> 8u);
    to_send[9] = (uint8_t) (address & 0x00FFu);
}


/**
 * Write Request Builder and Sender
 * @param address   Reference Address
 * @param amount    Amount of data to be Written
 * @param func      Modbus Functional Code
 * @param value     Data to Be Written
 */
int modbus::modbus_write(int address, uint amount, int func, const uint16_t *value) {
    int status = 0;
    if(func == WRITE_COIL || func == WRITE_REG) {
        uint8_t to_send[12];
        modbus_build_request(to_send, address, func);
        to_send[5] = 6;
        to_send[10] = (uint8_t) (value[0] >> 8u);
        to_send[11] = (uint8_t) (value[0] & 0x00FFu);
        status = modbus_send(to_send, 12);
    } else if(func == WRITE_REGS){
        uint8_t to_send[13 + 2 * amount];
        modbus_build_request(to_send, address, func);
        to_send[5] = (uint8_t) (7 + 2 * amount);
        to_send[10] = (uint8_t) (amount >> 8u);
        to_send[11] = (uint8_t) (amount & 0x00FFu);
        to_send[12] = (uint8_t) (2 * amount);
        for(int i = 0; i < amount; i++) {
            to_send[13 + 2 * i] = (uint8_t) (value[i] >> 8u);
            to_send[14 + 2 * i] = (uint8_t) (value[i] & 0x00FFu);
        }
        status = modbus_send(to_send, 13 + 2 * amount);
    } else if(func == WRITE_COILS) {
        uint8_t to_send[14 + (amount -1) / 8 ];
        modbus_build_request(to_send, address, func);
        to_send[5] = (uint8_t) (7 + (amount + 7) / 8);
        to_send[10] = (uint8_t) (amount >> 8u);
        to_send[11] = (uint8_t) (amount & 0x00FFu);
        to_send[12] = (uint8_t) ((amount + 7) / 8);
        for(int i = 0; i < (amount+7)/8; i++)
            to_send[13 + i] = 0; // init needed before summing!
        for(int i = 0; i < amount; i++) {
            to_send[13 + i/8] += (uint8_t) (value[i] << (i % 8u));
        }
        status = modbus_send(to_send, 14 + (amount - 1) / 8);
    }
    return status;

}


/**
 * Read Request Builder and Sender
 * @param address   Reference Address
 * @param amount    Amount of Data to Read
 * @param func      Modbus Functional Code
 */
int modbus::modbus_read(int address, uint amount, int func){
    uint8_t to_send[12];
    modbus_build_request(to_send, address, func);
    to_send[5] = 6;
    to_send[10] = (uint8_t) (amount >> 8u);
    to_send[11] = (uint8_t) (amount & 0x00FFu);
    return modbus_send(to_send, 12);
}


/**
 * Read Holding Registers
 * MODBUS FUNCTION 0x03
 * @param address    Reference Address
 * @param amount     Amount of Registers to Read
 * @param buffer     Buffer to Store Data Read from Registers
 */
int modbus::modbus_read_holding_registers(int address, int amount, uint16_t *buffer) {
    if(_connected) {
        if(amount > 65535 || address > 65535) {
            set_bad_input();
            return EX_BAD_DATA;
        }
        _busy = true;
        ssize_t n = modbus_read(address, amount, READ_REGS);
        if (n <= 0) {
            set_bad_con();
            std::cout <<"read_holding modbus_read error" <<std::endl;
            return BAD_CON;
        }
        uint8_t to_rec[MAX_MSG_LENGTH];
        ssize_t k = modbus_receive(to_rec);
        if (k <= 0) {
            set_bad_con();
            std::cout <<"read_holding modbus_receive error" <<std::endl;
            return BAD_CON;
        }
        _busy = false;
        //modbuserror_handle(to_rec, READ_REGS);
        //if(err) return err_no;
        for(uint i = 0; i < amount; i++) {
            buffer[i] = ((uint16_t)to_rec[9u + 2u * i]) << 8u;
            buffer[i] += (uint16_t) to_rec[10u + 2u * i];
        }
        return 0;
    } else {
        set_bad_con();
        return BAD_CON;
    }
}


/**
 * Read Input Registers
 * MODBUS FUNCTION 0x04
 * @param address     Reference Address
 * @param amount      Amount of Registers to Read
 * @param buffer      Buffer to Store Data Read from Registers
 */
int modbus::modbus_read_input_registers(int address, int amount, uint16_t *buffer) {
    if(_connected){
        if(amount > 65535 || address > 65535) {
            set_bad_input();
            return EX_BAD_DATA;
        }
        ssize_t n = modbus_read(address, amount, READ_INPUT_REGS);
        //std::cout <<"modbus_read" << n <<std::endl;
        if (n <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        uint8_t to_rec[MAX_MSG_LENGTH];
        ssize_t k = modbus_receive(to_rec);
        //std::cout <<"modbus_receive" << k <<std::endl;
        if (k <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        //modbuserror_handle(to_rec, READ_INPUT_REGS);
        //if(err) return err_no;
        for(uint i = 0; i < amount; i++) {
            buffer[i] = ((uint16_t)to_rec[9u + 2u * i]) << 8u;
            buffer[i] += (uint16_t) to_rec[10u + 2u * i];
        }
        return 0;
    } else {
        set_bad_con();
        return BAD_CON;
    }
}


/**
 * Read Coils
 * MODBUS FUNCTION 0x01
 * @param address     Reference Address
 * @param amount      Amount of Coils to Read
 * @param buffer      Buffer to Store Data Read from Coils
 */
int modbus::modbus_read_coils(int address, int amount, bool *buffer) {
    if(_connected) {
        if(amount > 2040 || address > 65535) {
            set_bad_input();
            return EX_BAD_DATA;
        }
        ssize_t n = modbus_read(address, amount, READ_COILS);
        if (n <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        uint8_t to_rec[MAX_MSG_LENGTH];
        ssize_t k = modbus_receive(to_rec);
        if (k <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        //modbuserror_handle(to_rec, READ_COILS);
        //if(err) return err_no;
        for(uint i = 0; i < amount; i++) {
            buffer[i] = (bool) ((to_rec[9u + i / 8u] >> (i % 8u)) & 1u);
        }
        return 0;
    } else {
        set_bad_con();
        return BAD_CON;
    }
}


/**
 * Read Input Bits(Discrete Data)
 * MODBUS FUNCITON 0x02
 * @param address   Reference Address
 * @param amount    Amount of Bits to Read
 * @param buffer    Buffer to store Data Read from Input Bits
 */
int modbus::modbus_read_input_bits(int address, int amount, bool* buffer) {
    if(_connected) {
        if(amount > 2040 || address > 65535) {
            set_bad_input();
            return EX_BAD_DATA;
        }
        ssize_t n = modbus_read(address, amount, READ_INPUT_BITS);
        if (n <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        uint8_t to_rec[MAX_MSG_LENGTH];
        ssize_t k = modbus_receive(to_rec);
        if (k <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        //modbuserror_handle(to_rec, READ_INPUT_BITS);
        //if(err) return err_no;
        for(uint i = 0; i < amount; i++) {
            buffer[i] = (bool) ((to_rec[9u + i / 8u] >> (i % 8u)) & 1u);
        }
        
        return 0;
    } else {
        return BAD_CON;
    }
}


/**
 * Write Single Coils
 * MODBUS FUNCTION 0x05
 * @param address    Reference Address
 * @param to_write   Value to be Written to Coil
 */
int modbus::modbus_write_coil(int address, const bool& to_write) {
    if(_connected) {
        if(address > 65535) {
            set_bad_input();
            return EX_BAD_DATA;
        }
        int value = to_write * 0xFF00;
        ssize_t n = modbus_write(address, 1, WRITE_COIL, (uint16_t *)&value);
        if (n <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        uint8_t to_rec[MAX_MSG_LENGTH];
        ssize_t k = modbus_receive(to_rec);
        if (k <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        //modbuserror_handle(to_rec, WRITE_COIL);
        //if(err) return err_no;
        return 0;
    } else {
        set_bad_con();
        return BAD_CON;
    }
}


/**
 * Write Single Register
 * FUCTION 0x06
 * @param address   Reference Address
 * @param value     Value to Be Written to Register
 */
int modbus::modbus_write_register(int address, const uint16_t& value) {
    if(_connected) {
        if(address > 65535) {
            set_bad_input();
            return EX_BAD_DATA;
        }
        ssize_t n = modbus_write(address, 1, WRITE_REG, &value);
        if (n <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        uint8_t to_rec[MAX_MSG_LENGTH];
        ssize_t k = modbus_receive(to_rec);
        if (k <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        //modbuserror_handle(to_rec, WRITE_COIL);
        //if(err) return err_no;
        return 0;
    } else {
        set_bad_con();
        return BAD_CON;
    }
}


/**
 * Write Multiple Coils
 * MODBUS FUNCTION 0x0F
 * @param address  Reference Address
 * @param amount   Amount of Coils to Write
 * @param value    Values to Be Written to Coils
 */
int modbus::modbus_write_coils(int address, int amount, const bool *value) {
    if(_connected) {
        if(address > 65535 || amount > 65535) {
            set_bad_input();
            return EX_BAD_DATA;
        }
        uint16_t temp[amount];
        for(int i = 0; i < amount; i++) {
            temp[i] = (uint16_t)value[i];
        }
        ssize_t n = modbus_write(address, amount, WRITE_COILS, temp);
        if (n <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        uint8_t to_rec[MAX_MSG_LENGTH];
        ssize_t k = modbus_receive(to_rec);
        if (k <= 0) {
            set_bad_con();
            return BAD_CON;
        }
        //modbuserror_handle(to_rec, WRITE_COILS);
        //if(err) return err_no;
        return 0;
    } else {
        set_bad_con();
        return BAD_CON;
    }
}


/**
 * Write Multiple Registers
 * MODBUS FUNCION 0x10
 * @param address Reference Address
 * @param amount  Amount of Value to Write
 * @param value   Values to Be Written to the Registers
 */
int modbus::modbus_write_registers(int address, int amount, const uint16_t *value) {
    if(_connected) {
        if(address > 65535 || amount > 65535) {
            set_bad_input();
            return EX_BAD_DATA;
        }
        _busy = true;
        ssize_t n = modbus_write(address, amount, WRITE_REGS, value);
        if (n <= 0) {
            set_bad_con();
            std::cout <<"write_registers modbus_write error" <<std::endl;
            return BAD_CON;
        }
        uint8_t to_rec[MAX_MSG_LENGTH];
        ssize_t k = modbus_receive(to_rec);
        if (k <= 0) {
            set_bad_con();
            std::cout <<"write_registers modbus_receive error" <<std::endl;
            return BAD_CON;
        }
        _busy = false;
        //modbuserror_handle(to_rec, WRITE_REGS);
        //if(err) return err_no;
        return 0;
    } else {
        set_bad_con();
        return BAD_CON;
    }
}


/**
 * Data Sender
 * @param to_send Request to Be Sent to Server
 * @param length  Length of the Request
 * @return        Size of the request
 */
ssize_t modbus::modbus_send(uint8_t *to_send, int length) {
    _msg_id++;
    return send(_socket, to_send, (size_t)length, 0);
}


/**
 * Data Receiver
 * @param buffer Buffer to Store the Data Retrieved
 * @return       Size of Incoming Data
 */
ssize_t modbus::modbus_receive(uint8_t *buffer) const {
    return recv(_socket, (char *) buffer, MAX_MSG_LENGTH, 0);
}

void modbus::set_bad_con() {
    err = true;
    error_msg = "BAD CONNECTION";
}


void modbus::set_bad_input() {
    err = true;
    error_msg = "BAD FUNCTION INPUT";
}

/**
 * Error Code Handler
 * @param msg   Message Received from the Server
 * @param func  Modbus Functional Code
 */
void modbus::modbuserror_handle(const uint8_t *msg, int func) {
    if(msg[7] == func + 0x80) {
        err = true;
        switch(msg[8]){
            case EX_ILLEGAL_FUNCTION:
                error_msg = "1 Illegal Function";
                break;
            case EX_ILLEGAL_ADDRESS:
                error_msg = "2 Illegal Address";
                break;
            case EX_ILLEGAL_VALUE:
                error_msg = "3 Illegal Value";
                break;
            case EX_SERVER_FAILURE:
                error_msg = "4 Server Failure";
                break;
            case EX_ACKNOWLEDGE:
                error_msg = "5 Acknowledge";
                break;
            case EX_SERVER_BUSY:
                error_msg = "6 Server Busy";
                break;
            case EX_NEGATIVE_ACK:
                error_msg = "7 Negative Acknowledge";
                break;
            case EX_MEM_PARITY_PROB:
                error_msg = "8 Memory Parity Problem";
                break;
            case EX_GATEWAY_PROBLEMP:
                error_msg = "10 Gateway Path Unavailable";
                break;
            case EX_GATEWYA_PROBLEMF:
                error_msg = "11 Gateway Target Device Failed to Respond";
                break;
            default:
                error_msg = "UNK";
                break;
        }
        std::cout << error_msg + "nnnnn" <<std::endl;
        
    }
    err = false;
    error_msg = "NO ERR";
}
