__title__ = "Demo 1"


def ui_demo(oled, oled_raw):
    oled.show_title("Demo App UI")
    oled.show_kv("A", "B", (16 + 7 * 0) + 2 * 0)


def init(app):
    app.menu.append_menu_item(["Demo App", ui_demo])
