import ToggleLeft from "lucide-solid/icons/toggle-left";
import SwitchCameraIcon from "lucide-solid/icons/switch-camera";
import Magnet from "lucide-solid/icons/magnet";
import Gauge from "lucide-solid/icons/gauge";
import MousePointerClick from "lucide-solid/icons/mouse-pointer-click";
import LineChart from "lucide-solid/icons/line-chart";
import Zap from "lucide-solid/icons/zap";
import CircuitBoard from "lucide-solid/icons/circuit-board";
import Thermometer from "lucide-solid/icons/thermometer";
import Eye from "lucide-solid/icons/eye";
import Gamepad from "lucide-solid/icons/gamepad";
import Cog from "lucide-solid/icons/cog";
import Tractor from "lucide-solid/icons/tractor";
import Lightbulb from "lucide-solid/icons/lightbulb";
import Droplet from "lucide-solid/icons/droplet";
import AirVent from "lucide-solid/icons/air-vent";
import Speaker from "lucide-solid/icons/speaker";
import Clock from "lucide-solid/icons/clock";
import RotateCw from "lucide-solid/icons/rotate-cw";
import Activity from "lucide-solid/icons/activity";
import Radar from "lucide-solid/icons/radar";
import Camera from "lucide-solid/icons/camera";
import Sparkles from "lucide-solid/icons/sparkles";
import Monitor from "lucide-solid/icons/monitor";
import Network from "lucide-solid/icons/network";
import Orbit from "lucide-solid/icons/orbit";
import Power from "lucide-solid/icons/power";
import Palette from "lucide-solid/icons/palette";
import Footprints from "lucide-solid/icons/footprints";
import FileQuestionMark from "lucide-solid/icons/file-question-mark";
import Cpu from "lucide-solid/icons/cpu";
import type {Component} from "solid-js";
import {FtSwarmController, FtSwarmVersion, SwOSIOType} from "./generated/genApiEnums.ts";
import {Move3D} from "lucide-solid";

export type IconComponent = Component<{ class?: string }>;

const ioIconMap: Record<number, IconComponent> = {
    0: ToggleLeft,
    1: SwitchCameraIcon,
    2: Magnet,
    3: Gauge,
    4: MousePointerClick,
    5: LineChart,
    6: Zap,
    7: CircuitBoard,
    8: Thermometer,
    9: Eye,
    10: Gamepad,
    11: Cog,
    12: Cog,
    13: Cog,
    14: Tractor,
    15: Cog,
    16: Lightbulb,
    17: Droplet,
    18: AirVent,
    19: Speaker,
    20: Cog,
    21: Clock,
    22: RotateCw,
    23: Activity,
    24: Radar,
    25: Camera,
    26: Gauge,
    27: Sparkles,
    28: Monitor,
    29: Network,
    30: Orbit,
    31: CircuitBoard,
    32: Power,
    33: Palette,
    34: Footprints,
    35: Radar,
    36: Gamepad,
};

const controllerIconMap: Record<FtSwarmController, IconComponent> = {
    [FtSwarmController.FTSWARM_NOCTRL]: Cpu,
    [FtSwarmController.FTSWARM]: Cpu,
    [FtSwarmController.FTSWARMCONTROL]: Gamepad,
    [FtSwarmController.FTSWARMCAM]: Camera,
    [FtSwarmController.FTSWARMPWRDRIVE]: Move3D,
    [FtSwarmController.FTSWARMDUINO]: CircuitBoard,
    [FtSwarmController.FTSWARM_MAXCONTROLLERTYPE]: Cpu
};

const versionToControllerMap: Record<FtSwarmVersion, FtSwarmController> = {
    [FtSwarmVersion.FTSWARM_NOVERSION]: FtSwarmController.FTSWARM_NOCTRL,
    [FtSwarmVersion.FTSWARMJST_1V0]: FtSwarmController.FTSWARM,
    [FtSwarmVersion.FTSWARMCONTROL_1V3]: FtSwarmController.FTSWARMCONTROL,
    [FtSwarmVersion.FTSWARMJST_1V15]: FtSwarmController.FTSWARM,
    [FtSwarmVersion.FTSWARMRS_2V0]: FtSwarmController.FTSWARM,
    [FtSwarmVersion.FTSWARMRS_2V1]: FtSwarmController.FTSWARM,
    [FtSwarmVersion.FTSWARMCAM_3V12]: FtSwarmController.FTSWARMCAM,
    [FtSwarmVersion.FTSWARMDUINO_1V141]: FtSwarmController.FTSWARMDUINO,
    [FtSwarmVersion.FTSWARMPWRDRIVE_1V141]: FtSwarmController.FTSWARMPWRDRIVE,
    [FtSwarmVersion.FTSWARMXL_1V00]: FtSwarmController.FTSWARM,
    [FtSwarmVersion.FTSWARMRC_1V140]: FtSwarmController.FTSWARM,
    [FtSwarmVersion.FTSWARMMAXVERSION]: FtSwarmController.FTSWARM_MAXCONTROLLERTYPE,
}

export function getIoIcon(iconName: string): IconComponent {
    const iconId = parseInt(iconName.split(/[\._-]/)[0]) as SwOSIOType;
    const icon = ioIconMap[iconId];
    if (!icon) return FileQuestionMark;
    return icon;
}

export function getControllerIcon(controller: FtSwarmVersion) {
    if (Object.keys(versionToControllerMap).includes(controller.toString()))
        return controllerIconMap[versionToControllerMap[controller]];

    return controllerIconMap[FtSwarmController.FTSWARM_NOCTRL];
}
