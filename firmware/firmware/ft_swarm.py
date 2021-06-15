from machine import *
from error_defs import *


class FtSwarmObject:
    def __init__(self, adress, *ports):
        self.__local = adress == "local"

        self.adress = adress
        self.ports = ports
        self.last_error = FTSWARM_OK

    def is_local(self):
        return self.__local


class FtSwarmMotor(FtSwarmObject):
    MCOMMAND_SLEEP, MCOMMAND_LEFT, MCOMMAND_RIGHT, MCOMMAND_BRAKE, MCOMMAND_COAST = range(5)
    M1, M2 = range(2)

    __PWM_M1, __PWM_M2 = range(2)

    def __init__(self, adress, motor):
        super().__init__(adress, motor)
        if not self.is_local():
            # Remote Init Code
            return

        # Local Init Code
        self.motor = motor

        self.__in1 = None
        self.__in2 = None
        self.__pwmChannel = None

        # Pin modes and pin set
        self.__sleep = Pin(5, mode=Pin.OUT)

        if self.motor == FtSwarmMotor.M1:
            self.__in1 = Pin(23, mode=Pin.OUT)
            self.__in2 = Pin(4, mode=Pin.OUT)
            self.__pwmChannel = FtSwarmMotor.__PWM_M1

        elif self.motor == FtSwarmMotor.M2:
            self.__in1 = Pin(2, mode=Pin.OUT)
            self.__in2 = Pin(0, mode=Pin.OUT)
            self.__pwmChannel = FtSwarmMotor.__PWM_M2

        else:
            self.last_error = FTSWARM_NO_PORT
            return

        # Set pwm
        self.__pwm = PWM(self.__in1, freq=5000)
        self.__pwm.duty(0)

        # Automatically set to sleep mode
        self.sleep()

    def local_cmd(self, mcommand, speed):
        if mcommand == FtSwarmMotor.MCOMMAND_SLEEP:
            self.__sleep.value(0)

        elif mcommand == FtSwarmMotor.MCOMMAND_LEFT:
            self.__sleep.value(1)
            self.__pwm = PWM(self.__in1, freq=5000)
            self.__pwm.duty(speed)
            self.__in2.value(1)

        elif mcommand == FtSwarmMotor.MCOMMAND_RIGHT:
            self.__sleep.value(1)
            self.__pwm = PWM(self.__in2, freq=5000)
            self.__pwm.duty(speed)
            self.__in1.value(1)

        elif mcommand == FtSwarmMotor.MCOMMAND_BRAKE:
            self.__sleep.value(1)
            self.__in1.value(1)
            self.__in2.value(1)

        elif mcommand == FtSwarmMotor.MCOMMAND_COAST:
            self.__sleep.value(1)
            self.__in1.value(0)
            self.__in2.value(0)

        else:
            self.last_error = FTSWARM_CMD_NOT_FOUND

    def remote_cmd(self, mcommand, speed):
        print("CMD: " + mcommand + "/" + speed)

    def cmd(self, mcommand, speed):
        if self.is_local():
            self.local_cmd(mcommand, speed)
        else:
            self.remote_cmd(mcommand, speed)

    def right(self, speed):
        self.cmd(self.MCOMMAND_RIGHT, speed)

    def left(self, speed):
        self.cmd(self.MCOMMAND_LEFT, speed)

    def coast(self):
        self.cmd(self.MCOMMAND_COAST, 0)

    def brake(self):
        self.cmd(self.MCOMMAND_BRAKE, 0)

    def sleep(self):
        self.cmd(self.MCOMMAND_SLEEP, 0)


class FtSwarmDigitalInput(FtSwarmObject):
    A1 = 27
    A2 = 26
    A3 = 25
    A4 = 33

    FTSWARM_LOW = 0
    FTSWARM_HIGH = 1
    FTSWARM_UNKNOWN = 255

    def __init__(self, adress, port):
        super().__init__(adress, port)

        self.last_error = FTSWARM_OK

        if port != 27 or port != 26 or port != 25 or port != 33:
            self.last_error = FTSWARM_NO_PORT
            return

        self.port = port

        if self.is_local():
            self.pin = Pin(port, mode=Pin.IN)
        else:
            self.adress = adress
            print("Port " + port)

        self.last_error = FTSWARM_OK

    def get_state(self):
        if self.is_local():
            # noinspection PyArgumentList
            return self.pin.value()
        else:
            return FtSwarmDigitalInput.FTSWARM_UNKNOWN

    def is_high(self):
        return self.get_state() == 1

    def is_low(self):
        return self.get_state() == 0
