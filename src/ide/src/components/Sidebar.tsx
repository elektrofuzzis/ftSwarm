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
import { Surface1 } from "./Surface";
import { useDebug } from "../contexts/DebugContext";
import { useOMContext } from "../contexts/transport/context";
import { For } from "solid-js/web";
import { SwOSState } from "../api/generated/genApiEnums";
import { useLocation, A } from "../util/router";
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
      class="flex items-center gap-1 pt-4"
      classList={{ "opacity-50": !!props.disabled }}
    >
      {props.icon({})}
      <span class="flex-1">{props.title}</span>
      {props.children}
    </div>
  );
};

const Activity: Component<{ active: boolean }> = (props) => {
  return (
    <div class="inline-flex w-3 items-center">
      <div
        class="h-4 w-1 rounded-r"
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
      class="text-thm-font-muted hover:text-thm-font flex cursor-pointer items-center gap-1 text-left transition-colors"
      classList={{ "opacity-50": !!props.disabled }}
    >
      <Activity active={props.active} />
      {/*@ts-ignore*/}
      {props.icon({ class: "size-5" })}
      <span class="flex-1">{props.text}</span>
      {props.children}
    </button>
  );
};

export const StatusCircle: Component<{ class: string }> = (props) => {
  return <div class={`size-3.5 rounded-full ${props.class}`} />;
};

const BottomRowIndicator: ParentComponent = (props) => {
  return <div class="flex items-center gap-3 p-3">{props.children}</div>;
};

const state2Bg = [
  "bg-thm-error", // 0: OFFLINE
  "bg-thm-primary", // 1: BOOTING
  "bg-thm-primary", // 2: STARTWIFI
  "bg-thm-ok animate-pulse", // 3: RUNNING
  "bg-thm-error animate-pulse", // 4: ERROR
  "bg-thm-primary", // 5: WAITING
  "bg-thm-primary", // 6: IDENTIFY
  "bg-thm-error", // 7: FATAL
  "bg-thm-error", // 8: FACTORY1
  "bg-thm-error", // 9: FACTORY2
];

export const Sidebar: Component<{
  open?: boolean;
  onToggle?: () => void;
}> = (props) => {
  const swarm = useOMContext();
  const controllers = () => Object.values(swarm.controllers);
  const totalControllers = () => controllers().length;
  const onlineControllers = () =>
    controllers().filter((it) => it.state === SwOSState.RUNNING).length;

  const route = useLocation();

  const loginState = useLoginContext();

  return (
    <aside class="flex h-full flex-col gap-3">
      <div class="flex flex-1 flex-col gap-3 overflow-scroll">
        <div class="flex items-center pt-2 pr-2 lg:hidden">
          <Title />
          <button
            class="text-thm-font-muted hover:text-thm-font ml-auto rounded-md p-1 text-2xl"
            onClick={props.onToggle}
          >
            &times;
          </button>
        </div>
        <div class="hidden lg:block">
          <Title />
        </div>

        <Category icon={Telescope} title="Monitor Swarm">
          <Surface1 class="ml-4 px-1 py-px">
            {onlineControllers()}/{totalControllers()}
          </Surface1>
        </Category>

        <div onClick={() => props.onToggle?.()}>
          <A href="/controller/overview" class="flex w-full flex-col">
            <MenuEntry
              icon={FolderPen}
              text="My Swarm"
              active={route() === "/controller/overview"}
            />
          </A>
        </div>
        <For each={controllers()}>
          {(it, _) => (
            <div onClick={() => props.onToggle?.()}>
              <A
                href={`/controller/${it.serialNumber}`}
                class="flex w-full flex-col"
              >
                <MenuEntry
                  icon={getControllerIcon(it.CtrlVersion)}
                  text={it.name}
                  active={route() === `/controller/${it.serialNumber}`}
                >
                  <StatusCircle class={state2Bg[it.state]} />
                </MenuEntry>
              </A>
            </div>
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
