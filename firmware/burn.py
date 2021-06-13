import argparse
import logging
import os
import sys
import threading
import tkinter as tk
from zipfile import ZipFile

import venv

from burn import config


class Burn:
    class Application(tk.Frame):
        class Board:
            def __init__(self, port):
                from download.micropython_master.tools import pyboard as pb
                import esptool as et

                self.port = port
                self.driver: pb = pb
                self.toolbox: et = et

            def clear(self):
                self.toolbox.main(argv=["--chip", "esp32", "--port", self.port, "erase_flash"])

        def __init__(self, master=None):
            super().__init__(master)
            self.master = master
            self.grid()
            self.create_widgets()

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
            self.log.grid(row=2)
            self.sendlog("Welcome to burn.py")
            self.sendlog("Connect to Controller via widget above")

        def sendlog(self, text):
            self.log["state"] = "normal"
            self.log.insert(tk.END, text + "\n")
            self.log["state"] = "disabled"

        class ConnectionThread(threading.Thread):
            def __init__(self, port, connection_success, connection_failed):
                super().__init__()
                self.port = port
                self.connection_success = connection_success
                self.connection_failed = connection_failed

            def run(self) -> None:
                if os.system("venv"
                             + os.sep + "Scripts"
                             + os.sep + "python download"
                             + os.sep + "micropython_master"
                             + os.sep + "tools"
                             + os.sep + "pyboard.py -d " + self.port + " -c 'exit()'"
                             ) == 0:
                    self.connection_success()
                else:
                    self.connection_failed()

        def connecttosrv(self):
            self.sendlog("Trying to connect to " + self.connection_possibilities.get("1.0", 'end-1c'))
            self.ConnectionThread(
                self.connection_possibilities.get("1.0", 'end-1c'),
                lambda: self.sendlog("Connected"),
                lambda: self.sendlog("Failed to Connect")
            ).start()

    def __init__(self, logger_configurated: logging.Logger):
        self.log = logger_configurated
        self.log.info("Welcome to burn.py")

        self.ui_root = tk.Tk()
        self.ui_root.wm_title("burn.py")
        self.ui_root.tk.call('wm', 'iconphoto', self.ui_root._w, tk.PhotoImage(file='burn/FlameV1.png'))
        self.ui = self.Application(self.ui_root)
        self.ui.mainloop()

        self.log.info("Bye from burn.py")


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

    def deactivate(self):
        os.system("venv" + os.sep + "Scripts" + os.sep + "deactivate")

    def install_packages(self):
        self.logger.info("Installing Packages")
        os.system("venv" + os.sep + "Scripts" + os.sep + "python -m pip install --upgrade pip")
        os.system("venv" + os.sep + "Scripts" + os.sep + "python -m pip install esptool")
        os.system("venv" + os.sep + "Scripts" + os.sep + "python -m pip install docopt==0.6.2")
        os.system("venv" + os.sep + "Scripts" + os.sep + "python -m pip install adafruit-ampy==1.0.5")
        os.system("venv" + os.sep + "Scripts" + os.sep + "python -m pip install requests")

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
