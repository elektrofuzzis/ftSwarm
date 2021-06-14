import argparse
import subprocess
import logging
import os
import sys
import threading
import tkinter as tk
from zipfile import ZipFile

import venv

from burn import config


# noinspection PyUnresolvedReferences
class Board:
    class EsptoolThread(threading.Thread):
        def __init__(self, action, success, failure):
            super().__init__()
            self.action = action
            self.success = success
            self.failure = failure

        def run(self) -> None:
            # noinspection PyBroadException
            try:
                self.action()
                self.success()
            except Exception as e:
                print(e)
                self.failure()

    def __init__(self, port, master):
        from download.micropython_master.tools import pyboard as pb
        import esptool as et

        self.port = port
        self.master = master
        self.driver: pb = pb
        self.toolbox: et = et

    def clear(self):
        self.EsptoolThread(
            action=lambda: self.toolbox.main(argv=["--chip", "esp32", "--port", self.port, "erase_flash"]),
            success=lambda: self.master.sendlog("Ereased Chip"),
            failure=lambda: self.master.sendlog("Could not erease Chip")
        ).start()

    def write_micropython(self):
        self.EsptoolThread(
            action=lambda: self.toolbox.main(argv=["--chip", "esp32", "--port", self.port,
                                                   "write_flash", "-z", "0x1000",
                                                   "stub/esp32-idf4-20210202-v1.14.bin"]),
            success=lambda: self.master.sendlog("Wrote Micropython"),
            failure=lambda: self.master.sendlog("Could not write Micropython")
        ).start()

    def sigkill(self):
        pyb = self.driver.Pyboard(device=self.port)
        pyb.enter_raw_repl()
        pyb.exit_raw_repl()
        pyb.close()

        self.master.sendlog("Killed Program")

    def command(self, cmd) -> str:
        try:
            pyb = self.driver.Pyboard(device=self.port)
            pyb.enter_raw_repl()

            out = pyb.exec(cmd).decode('UTF-8').replace('\r', '')

            pyb.exit_raw_repl()
            pyb.close()
            return out
        except self.driver.PyboardError as error:
            self.master.sendlog("A Pyboard Error occured: " + str(error))

        return ""

    def list_files(self):
        ls = "import uos\nfor f in uos.ilistdir():  print(f[0])"

        self.master.sendlog("Files:")

        for file in self.command(ls).split("\n"):
            if file == "":
                continue

            self.master.sendlog(" - " + file)

    def remove_files(self):
        ls = "import uos\nfor f in uos.ilistdir():  print(f[0])"

        cmd = self.command(ls)

        pyb = self.driver.Pyboard(device=self.port)
        pyb.enter_raw_repl()

        for file in cmd.split("\n"):
            if file == "" or "." not in file:
                continue

            pyb.fs_rm(file)

        pyb.exit_raw_repl()
        pyb.close()


class Burn:
    class Application(tk.Frame):
        class Controls(tk.Frame):
            def __init__(self, master=None):
                super().__init__(master)
                self.master: Burn.Application = master
                self.create_btns()

            def clear_board(self):
                try:
                    self.master.connected.clear()
                except AttributeError:
                    self.master.sendlog("Connection not given")

            def write_micropy(self):
                try:
                    self.master.connected.write_micropython()
                except AttributeError:
                    self.master.sendlog("Connection not given")

            def list_fw(self):
                try:
                    self.master.connected.list_files()
                except AttributeError:
                    self.master.sendlog("Connection not given")

            def remove_fw(self):
                try:
                    self.master.connected.remove_files()
                except AttributeError:
                    self.master.sendlog("Connection not given")

            def kill(self):
                try:
                    self.master.connected.sigkill()
                except AttributeError:
                    self.master.sendlog("Connection not given")

            def openputty(self):
                try:
                    if hasattr(self, "putty"):
                        self.putty.kill()

                    self.putty = subprocess.Popen(
                        [
                            "putty",
                            "-serial", self.master.connected.port,
                            "-sercfg", "115200,8,n,1,N"
                        ]
                    )
                except AttributeError:
                    self.master.sendlog("Connection not given")

            def get_button(self, name, action):
                btn = tk.Button(self)
                btn["text"] = name
                btn["command"] = action
                btn.pack()

                return btn

            def create_btns(self):
                self.get_button(
                    "Clear",
                    lambda: self.clear_board()
                )

                self.get_button(
                    "Write Python",
                    lambda: self.write_micropy()
                )

                self.get_button(
                    "Open Putty",
                    lambda: self.openputty()
                )

                self.get_button(
                    "Stop running Program",
                    lambda: self.kill()
                )

                self.get_button(
                    "List files",
                    lambda: self.list_fw()
                )

                self.get_button(
                    "Remove Files",
                    lambda: self.remove_fw()
                )

        def __init__(self, master=None, logger=None):
            super().__init__(master)
            self.master = master
            self.logger = logger
            self.grid()
            self.create_widgets()

            self.connected: Board

        def create_widgets(self):
            self.connection_possibilities = tk.Text(self)
            self.connection_possibilities["height"] = 1
            self.connection_possibilities.grid(row=1, column=0)

            self.connect = tk.Button(self)
            self.connect["text"] = "Connect"
            self.connect["command"] = self.connecttosrv
            self.connect.grid(row=1, column=1)

            self.log = tk.Text(self)
            self.log["state"] = "disabled"
            self.log.grid(row=2, column=0)

            self.controls = self.Controls(self)
            self.controls.grid(row=2, column=1)

            self.sendlog("Welcome to burn.py")
            self.sendlog("Connect to Controller via widget above")

        def sendlog(self, text):
            self.logger.info("SwarmDevTool: " + text)
            self.log["state"] = "normal"
            self.log.insert(tk.END, text + "\n")
            self.log["state"] = "disabled"

        def connecttosrv(self):
            self.sendlog("Set Port to " + self.connection_possibilities.get("1.0", 'end-1c'))
            self.connected = Board(self.connection_possibilities.get("1.0", 'end-1c'), self)

    def __init__(self, logger_configurated: logging.Logger):
        self.logger = logger_configurated
        self.logger.info("Welcome to burn.py")

        self.ui_root = tk.Tk()
        self.ui_root.wm_title("burn.py")
        # noinspection PyProtectedMember
        self.ui_root.tk.call('wm', 'iconphoto', self.ui_root._w, tk.PhotoImage(file='burn/FlameV1.png'))
        self.ui = self.Application(self.ui_root, self.logger)
        self.ui.mainloop()

        self.logger.info("Bye from burn.py")


class Bootstrap:
    def __init__(self, logger: logging.Logger):
        self.logger = logger
        self.logger.info("(Re)Loading Bootstrap")

        parser = argparse.ArgumentParser(description='Burn.py / Burn the FTSwarm', prog="burn.py")
        parser.add_argument("--use-active-env", "-a", action="count", default=0)

        args = parser.parse_args()

        if args.use_active_env > 0:
            Burn(configure_logger("burn.py"))
        else:
            self.activate()
            self.install_packages()
            self.download_all()
            os.system("venv" + os.sep + "Scripts" + os.sep + "python burn.py -a")
            self.deactivate()

    def activate(self):
        if not os.path.isdir("venv"):
            self.logger.info("Creating VENV")
            venv.EnvBuilder(
                clear=True,
                with_pip=True
            ).create("venv")

        os.system("venv" + os.sep + "Scripts" + os.sep + "activate")
        os.system("venv" + os.sep + "bin" + os.sep + "activate")

    def deactivate(self):
        os.system("venv" + os.sep + "Scripts" + os.sep + "deactivate")
        os.system("venv" + os.sep + "bin" + os.sep + "deactivate")

    def install_packages(self):
        self.logger.info("Installing Packages")
        os.system("venv" + os.sep + "Scripts" + os.sep + "python -m pip install --upgrade pip")
        os.system("venv" + os.sep + "Scripts" + os.sep + "python -m pip install esptool")
        os.system("venv" + os.sep + "Scripts" + os.sep + "python -m pip install docopt==0.6.2")
        os.system("venv" + os.sep + "Scripts" + os.sep + "python -m pip install adafruit-ampy==1.0.5")
        os.system("venv" + os.sep + "Scripts" + os.sep + "python -m pip install requests")

        os.system("venv" + os.sep + "bin" + os.sep + "python -m pip install --upgrade pip")
        os.system("venv" + os.sep + "bin" + os.sep + "python -m pip install esptool")
        os.system("venv" + os.sep + "bin" + os.sep + "python -m pip install docopt==0.6.2")
        os.system("venv" + os.sep + "bin" + os.sep + "python -m pip install adafruit-ampy==1.0.5")
        os.system("venv" + os.sep + "bin" + os.sep + "python -m pip install requests")

    def download_all(self):
        try:
            os.mkdir("download")
            self.logger.debug("Download Directory created")
        except FileExistsError:
            self.logger.debug("Download Directory exists")

        if not os.path.isdir("download" + os.sep + "direct_downloads"):
            os.mkdir("download" + os.sep + "direct_downloads")
            self.logger.debug("Created Direct Downloads Folder")

        import requests

        if not os.path.isfile("download" + os.sep + "direct_downloads" + os.sep + "mipy-master.zip"):
            self.logger.info("Downloading missing Dependency 'micropython'")
            file = requests.get(config.DOWNLOAD_URL_MIPY, allow_redirects=True)
            open("download" + os.sep + "direct_downloads" + os.sep + "mipy-master.zip", 'wb').write(file.content)

            self.logger.info("Extracting")
            with ZipFile("download" + os.sep + "direct_downloads" + os.sep + "mipy-master.zip") as zipfile:
                zipfile.extractall(path="download")

            os.rename("download" + os.sep + "micropython-master", "download" + os.sep + "micropython_master")


def configure_logger(name):
    logger = logging.getLogger(name)

    stdout_handler = logging.StreamHandler(stream=sys.stdout)
    stdout_handler.setLevel(config.LOG_LEVEL)
    stdout_format = logging.Formatter('%(name)s - %(levelname)s - %(message)s')
    stdout_handler.setFormatter(stdout_format)

    file_handler = logging.FileHandler('burn.log', mode="a")
    file_handler.setLevel(config.FILE_LOG_LEVEL)
    file_format = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    file_handler.setFormatter(file_format)

    logger.addHandler(stdout_handler)
    logger.addHandler(file_handler)

    logger.setLevel(logging.DEBUG)
    return logger


if __name__ == '__main__':
    Bootstrap(configure_logger("bootstrap"))
