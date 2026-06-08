import { Show, type Component, type ParentComponent } from "solid-js";
import TitleImage from "../assets/ftswarm.svg";
import Telescope from "lucide-solid/icons/telescope";
import FolderPen from "lucide-solid/icons/folder-pen";
import Workflow from "lucide-solid/icons/workflow";
import Box from "lucide-solid/icons/box";
import Settings from "lucide-solid/icons/settings";
import Rss from "lucide-solid/icons/rss";
import Cable from "lucide-solid/icons/cable";
import Update from "lucide-solid/icons/circle-fading-arrow-up";
import { Dynamic } from "solid-js/web";
import { Surface1 } from "./Surface";
import { useDebug } from "../contexts/DebugContext";
import { useOMContext } from "../contexts/transport/context";
import { For } from "solid-js/web";
import { SwOSState } from "../api/generated/genApiEnums";
import { useLocation } from "@solidjs/router";
import { getControllerIcon } from "../api/icons.ts";
import { Button } from "./Button.tsx";
import { LoginState, useLoginContext } from "../contexts/LoginContext.tsx";

export const Title = () => {
  const { toggleMenu } = useDebug();
  let clickTimestamps: number[] = [];

  const handleClick = () => {
    const now = Date.now();
    clickTimestamps.push(now);
    // Filter timestamps to keep only those within the last second
    clickTimestamps = clickTimestamps.filter((t) => now - t < 1000);
    if (clickTimestamps.length >= 5) {
      toggleMenu();
      clickTimestamps = []; // Reset after activation
    }
  };

  return <img src={TitleImage} class="min-w-32 px-6" onClick={handleClick} />;
};

const Category: ParentComponent<{
  icon: Component;
  title: string;
  disabled?: boolean;
}> = (props) => {
  return (
    <div
      class="flex pt-4 gap-1 items-center"
      classList={{ "opacity-50": !!props.disabled }}
    >
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
  icon: Component<{ class?: string }>;
  text: string;
  active: boolean;
  disabled?: boolean;
}> = (props) => {
  return (
    <button
      class="flex gap-1 items-center text-left cursor-pointer transition-colors text-thm-font-muted hover:text-thm-font"
      classList={{ "opacity-50": !!props.disabled }}
    >
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

const BottomRowIndicator: ParentComponent = (props) => {
  return <div class="flex items-center p-3 gap-3">{props.children}</div>;
};

const state2Bg: Record<SwOSState, string> = {
  [SwOSState.OFFLINE]: "bg-thm-error",
  [SwOSState.BOOTING]: "bg-thm-primary",
  [SwOSState.STARTWIFI]: "bg-thm-primary",
  [SwOSState.RUNNING]: "bg-thm-ok animate-pulse",
  [SwOSState.ERROR]: "bg-thm-error animate-pulse",
  [SwOSState.WAITING]: "bg-thm-primary",
  [SwOSState.IDENTIFY]: "bg-thm-primary",
  [SwOSState.FATAL]: "bg-thm-error",
  [SwOSState.MAXSTATE]: "bg-thm-error",
  [SwOSState.FACTORY1]: "bg-thm-error",
  [SwOSState.FACTORY2]: "bg-thm-error",
};

export const Sidebar: Component = () => {
  const swarm = useOMContext();
  const controllers = () => Object.values(swarm.controllers);
  const totalControllers = () => controllers().length;
  const onlineControllers = () =>
    controllers().filter((it) => it.state === SwOSState.RUNNING).length;

  const location = useLocation();
  const route = () => location.pathname;

  const loginState = useLoginContext();

  return (
    <aside class="flex flex-col gap-3 h-full">
      <div class="flex flex-col gap-3 flex-1 overflow-scroll">
        <Title />

        <Category icon={Telescope} title="Monitor Swarm">
          <Surface1 class="px-1 py-px ml-4">
            {onlineControllers()}/{totalControllers()}
          </Surface1>
        </Category>

        <a href="/controller/overview" class="w-full flex flex-col">
          <MenuEntry
            icon={FolderPen}
            text="My Swarm"
            active={route() === "/controller/overview"}
          />
        </a>
        <For each={controllers()}>
          {(it, _) => (
            <a
              href={`/controller/${it.serialNumber}`}
              class="w-full flex flex-col"
            >
              <MenuEntry
                icon={getControllerIcon(it.CtrlVersion)}
                text={it.name}
                active={route() === `/controller/${it.serialNumber}`}
              >
                <StatusCircle class={state2Bg[it.state]} />
              </MenuEntry>
            </a>
          )}
        </For>

        <Category icon={Workflow} title="Event Configuration" disabled={true} />
        <MenuEntry
          icon={Box}
          text="Configuration 1"
          active={false}
          disabled={true}
        >
          <StatusCircle class="bg-thm-primary" />
        </MenuEntry>
        <MenuEntry
          icon={Box}
          text="Configuration 2"
          active={false}
          disabled={true}
        />
        <MenuEntry
          icon={Box}
          text="Configuration 3"
          active={false}
          disabled={true}
        />
        <MenuEntry
          icon={Box}
          text="Configuration 4"
          active={false}
          disabled={true}
        />

        <Category icon={Settings} title="Swarm Settings" disabled={true} />
        <MenuEntry
          icon={Rss}
          text="WiFi & Web"
          active={false}
          disabled={true}
        />
        <MenuEntry
          icon={Cable}
          text="Swarm Configuration"
          active={false}
          disabled={true}
        />
        <MenuEntry
          icon={Settings}
          text="Miscellaneous"
          active={false}
          disabled={true}
        />
        <MenuEntry
          icon={Update}
          text="Firmware Update"
          active={false}
          disabled={true}
        />
      </div>
      <div class="flex flex-col gap-3">
        <Show when={loginState.status() == LoginState.LOGGED_OUT}>
          <Button class="w-full" on:click={loginState.startLogin}>
            Log In
          </Button>
        </Show>
        <Show when={loginState.status() == LoginState.LOGGED_IN}>
          <Button class="w-full" variant="outline" on:click={loginState.logout}>
            Log out
          </Button>
        </Show>
        <BottomRowIndicator>
          <StatusCircle class="bg-thm-ok" /> Online &bull; Up to date
        </BottomRowIndicator>
      </div>
    </aside>
  );
};
