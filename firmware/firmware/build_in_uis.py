import ssd1306
import main


def draw_close(oled_class, oled: ssd1306.SSD1306_I2C):
    main.app.teardown_request = True
    main.app.oled.show_title(str(main.app.teardown_counter))


def draw_jstickctrl(oled_class, oled: ssd1306.SSD1306_I2C):
    oled_class.show_title("Joystick CTRL")
    oled_class.show_kv("XA", str(oled_class.ctrl_joystick.cache_x), (16 + 7 * 0) + 2 * 0)
    oled_class.show_kv("YA", str(oled_class.ctrl_joystick.cache_y), (16 + 7 * 1) + 2 * 1)
    oled_class.show_kv("SW", str(oled_class.ctrl_joystick.cache_sw), (16 + 7 * 2) + 2 * 2)


def draw_bluescreen(oled_class, oled: ssd1306.SSD1306_I2C):
    oled_class.show_title("Bluescreen")
    oled.fill_rect(0, 16, 256, 64 - 16, 1)
    oled.text(":(", 10, 20, 0)
