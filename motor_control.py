import time
from ctypes import LittleEndianStructure, c_float, c_uint8, c_uint16
from dataclasses import dataclass
from enum import Enum

from serial import Serial
from serial.tools import list_ports


class command(Enum):
    SET_MOTOR   = 0
    SET_FREQ    = 1
    SET_DUTY    = 2
    SET_DELAY   = 3
    ACTIVATE_MOTORS = 4
    STOP_MOTORS = 5
    
class cmd_struct(LittleEndianStructure):
    _pack_ = 1
    _fields_=[
        ("header",  c_uint16),
        ("length",  c_uint8),
        ("command", c_uint8),
        ]

@dataclass
class motor:
    num: int
    freq: float = 0
    duty: float = 0
    delay: float = 0

class motor_control:
    __header = 0xFAFF

    def __init__(self, n_motors: int, port: str) -> None:
        self.port = port
        self.connected = False
        self.serial = None
        self.motors = []
        if self.port:
            self.connect(port)

        for i in range(n_motors):
            self.motors.append(motor(num=i))

    def connect(self, port: str):
        try:
            self.port = port
            self.ser = Serial(port=self.port, baudrate=115200)
            self.connected = True
        except Exception as e:
            print(e)
    
    def disconnect(self):
        if self.connected:
            self.ser.close()
            self.connected = False

    def set_motor_freq(self, num: int, freq: float):
        self.motors[num].freq = freq
        cmd = cmd_struct(self.__header, 5, command.SET_FREQ.value)
        cmd_bytes = bytearray(cmd)
        cmd_bytes.extend(bytearray(c_uint8(num)))
        cmd_bytes.extend(bytearray(c_float(freq)))
        self.send_cmd(cmd_bytes)

    def set_motor_duty(self, num: int, duty: float):
        self.motors[num].duty = duty
        cmd = cmd_struct(self.__header, 5, command.SET_DUTY.value)
        cmd_bytes = bytearray(cmd)
        cmd_bytes.extend(bytearray(c_uint8(num)))
        cmd_bytes.extend(bytearray(c_float(duty)))
        self.send_cmd(cmd_bytes)
    
    def set_motor_delay(self, num: int, delay: float):
        self.motors[num].delay = delay
        cmd = cmd_struct(self.__header, 5, command.SET_DELAY.value)
        cmd_bytes = bytearray(cmd)
        cmd_bytes.extend(bytearray(c_uint8(num)))
        cmd_bytes.extend(bytearray(c_float(delay)))
        self.send_cmd(cmd_bytes)
    
    def config_motor(self, num: int, freq: float, duty: float, delay: float):
        print(freq, duty, delay)
        self.set_motor_freq(num, freq)
        self.set_motor_duty(num, duty)
        self.set_motor_delay(num, delay)

    def start_motors(self):
        cmd_start = cmd_struct(self.__header, 0, command.ACTIVATE_MOTORS.value)
        self.send_cmd(bytearray(cmd_start))
        print('STARTED!!!')

    def stop_motors(self):
        cmd_stop = cmd_struct(self.__header, 0, command.STOP_MOTORS.value)
        self.send_cmd(bytearray(cmd_stop))
        print('STOP!!!')

    def send_cmd(self, cmd: bytearray):
        if self.connected:
            self.ser.write(cmd)
        # print(cmd.hex(), len(cmd))

    def __repr__(self) -> str:
        return "\n".join([str(m) for m in self.motors])

if __name__ == '__main__':
    NUM_MOTORS = 6
    freqs = [1, 30, 100, 200, 10, 1]
    dutys = [0.0, 0.0, 0.0, 0.19, 0.0, 0.0]
    ports = list_ports.comports()
    print(ports[0].device, ports[0].description)
    control = motor_control(NUM_MOTORS, None)
    control.connect(ports[0].device)
    control.stop_motors()
    for i in range(NUM_MOTORS):
        control.config_motor(i, freqs[i], 0., 0)

    control.start_motors()
    time.sleep(0.1)
    # control.stop_motors()
    
    print(control)
    