import type { Component, ParentComponent } from "solid-js";
import TitleImage from "../assets/ftswarm.svg";
import Telescope from "lucide-solid/icons/telescope";
import Cpu from "lucide-solid/icons/cpu";
import Gamepad2 from "lucide-solid/icons/gamepad-2";
import Plug from "lucide-solid/icons/plug";
import FolderPen from "lucide-solid/icons/folder-pen";
import Workflow from "lucide-solid/icons/workflow";
import Box from "lucide-solid/icons/box";
import Settings from "lucide-solid/icons/settings";
import Rss from "lucide-solid/icons/rss";
import Cable from "lucide-solid/icons/cable";
import Update from "lucide-solid/icons/circle-fading-arrow-up";
import { Dynamic } from "solid-js/web";
import { Surface1 } from "./Surface";

export const Title = () => {
  return <img src={TitleImage} class="min-w-32 px-4" />;
};

const Category: ParentComponent<{ icon: Component; title: string }> = (
  props,
) => {
  return (
    <div class="flex pt-4 gap-1 items-center">
      <Dynamic component={props.icon} />
      <span class="flex-1">{props.title}</span>
      {props.children}
    </div>
  );
};

const Activity: Component<{ active: boolean }> = (props) => {
  return (
    <div class="inline-flex w-3 items-center">
      <div
        class="w-1 rounded-r h-4"
        classList={{ "bg-thm-primary": props.active }}
      ></div>
    </div>
  );
};

const MenuEntry: ParentComponent<{
  icon: Component;
  text: string;
  active: boolean;
}> = (props) => {
  return (
    <button class="flex gap-1 items-center text-left cursor-pointer transition-colors text-thm-font-muted hover:text-thm-font">
      <Activity active={props.active} />
      {/*@ts-ignore*/}
      <Dynamic component={props.icon} class="size-5" />
      <span class="flex-1">{props.text}</span>
      {props.children}
    </button>
  );
};

const StatusCircle: Component<{ class: String }> = (props) => {
  return <div class={"size-3.5 rounded-full " + props.class} />;
};

export const Sidebar = () => {
  return (
    <aside class="flex flex-col gap-3">
      <Title />

      <Category icon={Telescope} title="Monitor Swarm">
        <Surface1 class="px-1 py-px ml-4">5/6</Surface1>
      </Category>
      <MenuEntry icon={FolderPen} text="My Swarm" active={true} />
      <MenuEntry icon={Cpu} text="ftSwarm400" active={false}>
        <StatusCircle class="bg-thm-ok animate-pulse" />
      </MenuEntry>
      <MenuEntry icon={Cpu} text="ftSwarm401" active={false}>
        <StatusCircle class="bg-thm-ok animate-pulse" />
      </MenuEntry>
      <MenuEntry icon={Cpu} text="ftSwarm402" active={false}>
        <StatusCircle class="bg-thm-ok animate-pulse" />
      </MenuEntry>
      <MenuEntry icon={Gamepad2} text="ftSwarm403" active={false}>
        <StatusCircle class="bg-thm-ok animate-pulse" />
      </MenuEntry>
      <MenuEntry icon={Plug} text="ftSwarm404" active={false}>
        <StatusCircle class="bg-thm-ok animate-pulse" />
      </MenuEntry>
      <MenuEntry icon={Plug} text="ftSwarm405" active={false}>
        <StatusCircle class="bg-thm-error" />
      </MenuEntry>

      <Category icon={Workflow} title="Event Configuration" />
      <MenuEntry icon={Box} text="Configuration 1" active={false}>
        <StatusCircle class="bg-thm-primary" />
      </MenuEntry>
      <MenuEntry icon={Box} text="Configuration 2" active={false} />
      <MenuEntry icon={Box} text="Configuration 3" active={false} />
      <MenuEntry icon={Box} text="Configuration 4" active={false} />

      <Category icon={Settings} title="Swarm Settings" />
      <MenuEntry icon={Rss} text="WiFi & Web" active={false} />
      <MenuEntry icon={Cable} text="Swarm Configuration" active={false} />
      <MenuEntry icon={Settings} text="Miscellaneous" active={false} />
      <MenuEntry icon={Update} text="Firmware Update" active={false} />
    </aside>
  );
};
