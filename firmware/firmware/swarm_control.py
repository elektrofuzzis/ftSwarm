import time
from machine import Pin, ADC
import ssd1306


class Oled:
    def __init__(self, i2c, ctrl_joystick, menu):
        self.ctrl_joystick = ctrl_joystick
        self.menu = menu
        self.oled = ssd1306.SSD1306_I2C(128, 64, i2c, 0x3c)
        self.oled.fill(0)
        self.oled.poweron()
        self.oled.show()

    def update(self):
        self.oled.fill(0)

        self.ctrl_joystick.digital_value_x_change()
        self.ctrl_joystick.digital_value_y_change()
        self.ctrl_joystick.digital_value_sw_change()

        if self.menu.is_menu():
            self.draw_menu()
        else:
            self.draw_current()

        self.oled.show()

    def draw_menu(self):
        self.oled.fill(0)
        self.show_title("Menu")

        if self.ctrl_joystick.cache_sw:
            self.menu.close_menu()
            return

        if self.ctrl_joystick.cache_x == 1 and self.menu.get_current() != 0:
            self.menu.up()

        elif self.ctrl_joystick.cache_x == -1 and self.menu.get_current() != len(self.menu.get_all()) - 1:
            self.menu.down()

        if self.menu.get_current() != 0:
            p0 = self.menu.get_all()[self.menu.get_current() - 1]
        else:
            p0 = ["", None]

        p1 = self.menu.get_all()[self.menu.get_current()]

        if self.menu.get_current() != len(self.menu.get_all()) - 1:
            p2 = self.menu.get_all()[self.menu.get_current() + 1]
        else:
            p2 = ["", None]

        self.oled.fill_rect(2, (15 * 2), 100, 13, 1)

        self.oled.text(p0[0], 3, (15 * 1) + 3, 1)
        self.oled.text(p1[0], 3, (15 * 2) + 3, 0)
        self.oled.text(p2[0], 3, (15 * 3) + 3, 1)
        self.oled.show()

    def draw_current(self):
        self.menu.get_all()[self.menu.get_current()][1](self, self.oled)

        if self.ctrl_joystick.cache_y == -1:
            self.menu.open_menu()

    def show_title(self, title):
        self.oled.text(title, 3, 0, 2)
        self.oled.line(0, 15, 255, 15, 1)

    def show_kv(self, key, value, y):
        self.oled.text(key + ": " + value, 15, y, 2)

    def teardown(self):
        self.oled.fill(0)
        self.oled.text("Goodbye", 15, 0, 2)
        self.oled.show()
        time.sleep(1)
        self.oled.fill(0)
        self.oled.show()


class Menu:
    def __init__(self):
        self._menu = False
        self._menu_items = []
        self._local = 2

    def append_menu_item(self, item):
        self._menu_items.append(item)

    def get_all(self):
        return self._menu_items

    def is_menu(self):
        return self._menu

    def open_menu(self):
        self._menu = True
        print("Open")

    def close_menu(self):
        self._menu = False

    def up(self):
        self._local -= 1

    def down(self):
        self._local += 1

    def get_current(self):
        return self._local


class Joystick:
    state_x = 0
    state_y = 0
    state_sw = False

    cache_x = 0
    cache_y = 0
    cache_sw = False

    def __init__(self, px, py, psw):
        self.xa = ADC(Pin(px, Pin.IN))
        self.ya = ADC(Pin(py, Pin.IN))
        self.sw = Pin(psw, Pin.IN)

        self.ya.atten(ADC.ATTN_11DB)
        self.xa.atten(ADC.ATTN_11DB)

    def digital_value_x(self):
        val = self.xa.read()
        if val == 0:
            return -1
        elif val == 4095:
            return 1
        else:
            return 0

    def digital_value_y(self):
        val = self.ya.read()
        if val == 0:
            return -1
        elif val == 4095:
            return 1
        else:
            return 0

    def digital_value_sw(self):
        # noinspection PyArgumentList
        return self.sw.value() != 1

    def digital_value_sw_change(self):
        actual_value = self.digital_value_sw()

        if self.state_sw != actual_value:
            self.state_sw = actual_value
            self.cache_sw = actual_value
            return actual_value
        else:
            self.cache_sw = False
            return False

    def digital_value_x_change(self):
        actual_value = self.digital_value_x()

        if self.state_x != actual_value:
            self.state_x = actual_value
            self.cache_x = actual_value
            return actual_value
        else:
            self.cache_x = 0
            return 0

    def digital_value_y_change(self):
        actual_value = self.digital_value_y()

        if self.state_y != actual_value:
            self.state_y = actual_value
            self.cache_y = actual_value
            return actual_value
        else:
            self.cache_y = 0
            return 0
