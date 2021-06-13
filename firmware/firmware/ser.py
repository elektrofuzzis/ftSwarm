import uselect
import sys


class SerialIn:
    def __init__(self, app):
        self.message = ""
        self.app = app

    def update(self):
        if uselect.select([sys.stdin], [], [], 0.01)[0]:
            char = sys.stdin.read(1)

            print(char, end="")

            if char != '\n':
                self.message += char
                return

            self.workwith(self.message)
            self.message = ""

    def workwith(self, message):
        if message == "reload" or message == "rl":
            # Perform Hot Reload
            self.app.restart()
        else:
            print("ERR id=0 msg=notfound")
