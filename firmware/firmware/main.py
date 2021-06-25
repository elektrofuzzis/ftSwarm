from swarm_control import *
from build_in_uis import *
from machine import *
from ser import SerialIn
import json
import time
import uos


class Application:
    def __init__(self):
        self.load_config()

        self.i2c = SoftI2C(scl=Pin(22), sda=Pin(21))
        self.serial = SerialIn(self)

        cfg = self.load_config()
        self.mode = cfg["mode"]
        self.autostart = cfg["autostart"]
        self.peripherals = cfg["peripherals"]

        self.control = self.peripherals == 2 and self.autostart != "" and self.mode == 0

        if self.control:
            self.joystick1 = Joystick(32, 33, 26)
            self.menu = Menu()

            self.menu.append_menu_item(["Bluescreen", draw_bluescreen])
            self.menu.append_menu_item(["Joystick", draw_jstickctrl])
            self.menu.append_menu_item(["Close", draw_close])

            self.oled = Oled(
                self.i2c,
                self.joystick1,
                self.menu
            )

        self.teardown_request = False
        self.should_restart = False

    def start(self):
        while not self.teardown_request:
            time.sleep(0.2)

            self.serial.update()

            if self.control:
                self.oled.update()

            if self.teardown_request:
                self.teardown()
                if self.should_restart:
                    self.teardown_request = False
                    self.should_restart = False
                    self.teardown_counter = 0

    def restart(self):
        self.teardown_request = True
        self.should_restart = True

    def teardown(self):
        if self.control:
            self.oled.teardown()

    def load_config(self):
        return json.load(open("config.json"))


app = Application()
mods = []

if __name__ == '__main__':
    for f in uos.ilistdir("programs"):
        mod = __import__("programs." + f[0].replace(".py", "", 1), globals(), locals(), ["__title__", "init"])

        print("MODULE: " + mod.__title__)
        mod.init(app)

        mods.append(mod)

    app.start()
