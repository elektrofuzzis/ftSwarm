import Car from "lucide-solid/icons/car";
import ToggleLeft from "lucide-solid/icons/toggle-left";
import ToggleRight from "lucide-solid/icons/toggle-right";
import Magnet from "lucide-solid/icons/magnet";
import Gauge from "lucide-solid/icons/gauge";
import CircuitBoard from "lucide-solid/icons/circuit-board";
import Thermometer from "lucide-solid/icons/thermometer";
import Gamepad from "lucide-solid/icons/gamepad";
import Cog from "lucide-solid/icons/cog";
import RotateCw from "lucide-solid/icons/rotate-cw";
import Activity from "lucide-solid/icons/activity";
import Camera from "lucide-solid/icons/camera";
import Monitor from "lucide-solid/icons/monitor";
import LineSquiggle from "lucide-solid/icons/line-squiggle";
import Power from "lucide-solid/icons/power";
import Compass from "lucide-solid/icons/compass";
import Palette from "lucide-solid/icons/palette";
import FileQuestionMark from "lucide-solid/icons/file-question-mark";
import LampCeiling from "lucide-solid/icons/lamp-ceiling";
import AudioWaveform from "lucide-solid/icons/audio-waveform";
import WindArrowDown from "lucide-solid/icons/wind-arrow-down";
import Bell from "lucide-solid/icons/bell";
import Cpu from "lucide-solid/icons/cpu";
import Move3D from "lucide-solid/icons/move-3d";
import Joystick from "lucide-solid/icons/joystick";
import RefreshCCWDot from "lucide-solid/icons/refresh-ccw-dot";
import Sun from "lucide-solid/icons/sun";
import Ruler from "lucide-solid/icons/ruler";
import Settings2 from "lucide-solid/icons/settings-2";
import type {Component} from "solid-js";
import {FtSwarmVersion, SwOSIOType} from "./generated/genApiEnums.ts";

export type IconComponent = Component<{ class?: string }>;

const ioTypeIconMap: Record<SwOSIOType, IconComponent> = {
    [SwOSIOType.SWOSIO_UNDEF]: Cog,
    [SwOSIOType.SWOSIO_DIGITAL]: ToggleLeft,
    [SwOSIOType.SWOSIO_SWITCH]: ToggleRight,
    [SwOSIOType.SWOSIO_REEDSWITCH]: Magnet,
    [SwOSIOType.SWOSIO_LIGHTBARRIER]: Sun,
    [SwOSIOType.SWOSIO_BUTTON]: ToggleRight,
    [SwOSIOType.SWOSIO_ANALOG]: Gauge,
    [SwOSIOType.SWOSIO_VOLTMETER]: Gauge,
    [SwOSIOType.SWOSIO_OHMMETER]: Gauge,
    [SwOSIOType.SWOSIO_THERMOMETER]: Thermometer,
    [SwOSIOType.SWOSIO_LDR]: Gauge,
    [SwOSIOType.SWOSIO_JOYSTICK]: Joystick,
    [SwOSIOType.SWOSIO_MOTOR]: RotateCw,
    [SwOSIOType.SWOSIO_XSMOTOR]: RotateCw,
    [SwOSIOType.SWOSIO_XMMOTOR]: RotateCw,
    [SwOSIOType.SWOSIO_TRACTOR]: RotateCw,
    [SwOSIOType.SWOSIO_ENCODER]: RefreshCCWDot,
    [SwOSIOType.SWOSIO_LAMP]: LampCeiling,
    [SwOSIOType.SWOSIO_VALVE]: WindArrowDown,
    [SwOSIOType.SWOSIO_COMPRESSOR]: WindArrowDown,
    [SwOSIOType.SWOSIO_BUZZER]: Bell,
    [SwOSIOType.SWOSIO_STEPPER]: Move3D,
    [SwOSIOType.SWOSIO_COUNTER]: Activity,
    [SwOSIOType.SWOSIO_ROTARYENCODER]: Activity,
    [SwOSIOType.SWOSIO_FREQUENCYMETER]: AudioWaveform,
    [SwOSIOType.SWOSIO_LIDAR]: Ruler,
    [SwOSIOType.SWOSIO_CAM]: Camera,
    [SwOSIOType.SWOSIO_SERVO]: Settings2,
    [SwOSIOType.SWOSIO_PIXEL]: Palette,
    [SwOSIOType.SWOSIO_OLED]: Monitor,
    [SwOSIOType.SWOSIO_I2C]: Cpu,
    [SwOSIOType.SWOSIO_GYRO]: Compass,
    [SwOSIOType.SWOSIO_POWER]: Power,
    [SwOSIOType.SWOSIO_COLORSENSOR]: Palette,
    [SwOSIOType.SWOSIO_TRAILSENSOR]: LineSquiggle,
    [SwOSIOType.SWOSIO_ULTRASONIC]: Ruler,
    [SwOSIOType.SWOSIO_JOYSTICK_POTI]: Joystick,
    [SwOSIOType.SWOSIO_WHEELDRIVE]: RotateCw,
    [SwOSIOType.SWOSIO_MINIMOTOR]: RotateCw,
    [SwOSIOType.SWOSIO_SMOTOR]: RotateCw,
    [SwOSIOType.SWOSIO_POWERMOTOR]: RotateCw,
    [SwOSIOType.SWOSIO_MMOTOR]: RotateCw,
    [SwOSIOType.SWOSIO_RCMOTOR]: RotateCw,
    [SwOSIOType.SWOSIO_RCSERVO]: Settings2,
    [SwOSIOType.SWOSIO_RCPOTI]: RefreshCCWDot,
    [SwOSIOType.SWOSIO_MAXIOTYPE]: Cog
};

enum FtSwarmController {
    FTSWARM_NOCTRL,
    FTSWARM,
    FTSWARMCONTROL,
    FTSWARMCAM,
    FTSWARMPWRDRIVE,
    FTSWARMDUINO,
    FTSWARMRC,
    FTSWARM_MAXCONTROLLERTYPE,
}

const controllerIconMap: Record<FtSwarmController, IconComponent> = {
    [FtSwarmController.FTSWARM_NOCTRL]: Cpu,
    [FtSwarmController.FTSWARM]: Cpu,
    [FtSwarmController.FTSWARMCONTROL]: Gamepad,
    [FtSwarmController.FTSWARMCAM]: Camera,
    [FtSwarmController.FTSWARMPWRDRIVE]: Move3D,
    [FtSwarmController.FTSWARMDUINO]: CircuitBoard,
    [FtSwarmController.FTSWARMRC]: Car,
    [FtSwarmController.FTSWARM_MAXCONTROLLERTYPE]: Cpu,
};

const versionToControllerMap: Record<FtSwarmVersion, FtSwarmController> = {
    [FtSwarmVersion.FTSWARM_NOVERSION]: FtSwarmController.FTSWARM_NOCTRL,
    [FtSwarmVersion.FTSWARMCONTROL_1V3]: FtSwarmController.FTSWARMCONTROL,
    [FtSwarmVersion.FTSWARMJST_1V15]: FtSwarmController.FTSWARM,
    [FtSwarmVersion.FTSWARMRS_2V1]: FtSwarmController.FTSWARM,
    [FtSwarmVersion.FTSWARMCAM_3V12]: FtSwarmController.FTSWARMCAM,
    [FtSwarmVersion.FTSWARMDUINO_1V141]: FtSwarmController.FTSWARMDUINO,
    [FtSwarmVersion.FTSWARMPWRDRIVE_1V141]: FtSwarmController.FTSWARMPWRDRIVE,
    [FtSwarmVersion.FTSWARMXL_1V00]: FtSwarmController.FTSWARM,
    [FtSwarmVersion.FTSWARMCONTROL_1V3UC]: FtSwarmController.FTSWARMCONTROL,
    [FtSwarmVersion.FTSWARMRC_1V141]: FtSwarmController.FTSWARMRC,
    [FtSwarmVersion.FTSWARMMAXVERSION]: FtSwarmController.FTSWARM_MAXCONTROLLERTYPE,
};

export function getIoIcon(ioType: SwOSIOType): IconComponent {
    const icon = ioTypeIconMap[ioType];
    if (!icon) return FileQuestionMark;
    return icon;
}

export function getControllerIcon(controller: FtSwarmVersion | null) {
    if (!controller) return controllerIconMap[FtSwarmController.FTSWARM_NOCTRL];
    if (Object.keys(versionToControllerMap).includes(controller.toString()))
        return controllerIconMap[versionToControllerMap[controller]];

    return controllerIconMap[FtSwarmController.FTSWARM_NOCTRL];
}
