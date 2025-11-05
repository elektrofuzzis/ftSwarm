import {
  For,
  type Component,
  Switch,
  Match,
  Show,
  createEffect,
  createRenderEffect,
  on,
  createSignal,
} from "solid-js";
import { useDebug } from "../contexts/DebugContext";
import { Surface1 } from "./Surface";
import X from "lucide-solid/icons/x";
import PanelRight from "lucide-solid/icons/panel-right";
import PanelBottom from "lucide-solid/icons/panel-bottom";
import SquareArrowOutUpRight from "lucide-solid/icons/square-arrow-out-up-right";
import ArrowDown from "lucide-solid/icons/arrow-down";
import {
  LogChannel,
  channelColors,
  type LogMessage,
  DebugMode,
} from "../contexts/logtypes";

const LogEntry = (props: { log: LogMessage }) => {
  const colorInfo = channelColors[props.log.channel];
  return (
    <div class="font-mono text-sm flex items-start">
      <span class="text-gray-500 mr-2">
        {new Date(props.log.timestamp).toLocaleTimeString()}
      </span>
      <span class="w-[10ch]">
        <span
          class={`font-bold rounded px-1.5 py-0.5 text-xs mr-2 ${colorInfo.bg} ${colorInfo.text}`}
        >
          {props.log.channel}
        </span>
      </span>
      <span class="flex-1 whitespace-pre-wrap">{props.log.data}</span>
    </div>
  );
};

export const DebugMenu: Component = () => {
  const {
    isOpen,
    toggleMenu,
    logs,
    visibleChannels,
    toggleChannel,
    isChannelVisible,
    mode,
    setMode,
    forceScroll,
  } = useDebug();

  let logContainerRef: HTMLDivElement | null = null;
  const [isScrolledToBottom, setIsScrolledToBottom] = createSignal(true);
  const [showToLatest, setShowToLatest] = createSignal(false);

  createRenderEffect(() => {
    if (logContainerRef == null) return;
    const ref = logContainerRef as HTMLDivElement;
    const isNearBottom =
      ref.scrollHeight - ref.scrollTop - ref.clientHeight < 20;
    setIsScrolledToBottom(isNearBottom);
  });

  createEffect(
    on(logs, () => {
      // This runs after the DOM is updated with the new log.
      if (isScrolledToBottom() && logContainerRef) {
        const ref = logContainerRef as HTMLDivElement;
        ref.scrollTop = ref.scrollHeight;
      }
    }),
  );

  createEffect(
    on(forceScroll, () => {
      if (logContainerRef) {
        const ref = logContainerRef as HTMLDivElement;
        ref.scrollTop = ref.scrollHeight;
      }
    }),
  );

  const handleScroll = () => {
    if (!logContainerRef) return;
    const ref = logContainerRef as HTMLDivElement;
    const isAtBottom = ref.scrollHeight - ref.scrollTop - ref.clientHeight < 20;
    const isOverlayMode = mode() === DebugMode.SCREEN_RIGHT;
    setShowToLatest(isOverlayMode && !isAtBottom);
    setIsScrolledToBottom(isAtBottom);
  };

  const scrollToBottom = () => {
    if (logContainerRef) {
      const ref = logContainerRef as HTMLDivElement;
      ref.scrollTop = ref.scrollHeight;
    }
  };

  const filteredLogs = () =>
    logs().filter((log) => isChannelVisible(log.channel));

  const menuContent = (
    <Surface1 class="p-4 flex-grow flex flex-col gap-2 h-full">
      <div class="flex justify-between items-center mb-2">
        <h2 class="text-xl font-bold">Debug Communication</h2>

        <div class="flex gap-2 items-center">
          <div class="flex rounded overflow-hidden">
            <For each={Object.values(LogChannel)}>
              {(channel) => {
                const colorInfo = channelColors[channel];
                const isVisible = () => visibleChannels().includes(channel);
                return (
                  <button
                    onClick={() => toggleChannel(channel)}
                    class={`p-2 text-sm transition-all ${
                      isVisible()
                        ? `${colorInfo.bg} ${colorInfo.text}`
                        : "bg-thm-surface-2 text-thm-font-muted"
                    } hover:brightness-125`}
                  >
                    {channel}
                  </button>
                );
              }}
            </For>
          </div>

          <div class="flex items-center gap-1 p-1 bg-thm-surface-2 rounded-md">
            <button
              onClick={() => setMode(DebugMode.SCREEN_RIGHT)}
              class={`p-1.5 rounded ${
                mode() === DebugMode.SCREEN_RIGHT ? "bg-thm-primary" : ""
              }`}
              title={DebugMode.SCREEN_RIGHT}
            >
              <SquareArrowOutUpRight class="w-5 h-5" />
            </button>
            <button
              onClick={() => setMode(DebugMode.ATTACHED_RIGHT)}
              class={`p-1.5 rounded ${
                mode() === DebugMode.ATTACHED_RIGHT ? "bg-thm-primary" : ""
              }`}
              title={DebugMode.ATTACHED_RIGHT}
            >
              <PanelRight class="w-5 h-5" />
            </button>
            <button
              onClick={() => setMode(DebugMode.ATTACHED_BOTTOM)}
              class={`p-1.5 rounded ${
                mode() === DebugMode.ATTACHED_BOTTOM ? "bg-thm-primary" : ""
              }`}
              title={DebugMode.ATTACHED_BOTTOM}
            >
              <PanelBottom class="w-5 h-5" />
            </button>

            <div class="border-l border-thm-surface-border-3 h-5 mx-1"></div>

            <button
              onClick={toggleMenu}
              class="p-1.5 rounded hover:bg-thm-error"
              title="Close Menu"
            >
              <X class="w-5 h-5" />
            </button>
          </div>
        </div>
      </div>

      <div class="flex-1 flex flex-col relative min-h-0">
        <div
          ref={logContainerRef!}
          onScroll={handleScroll}
          class="flex flex-col gap-1 p-2 rounded bg-thm-surface-2 flex-grow overflow-y-scroll"
        >
          <For each={filteredLogs()}>{(log) => <LogEntry log={log} />}</For>
        </div>
        <Show when={showToLatest()}>
          <button
            onClick={scrollToBottom}
            class="absolute bottom-2 right-2 p-2 rounded-full bg-thm-primary hover:bg-thm-accent shadow-lg"
            title="Scroll to latest"
          >
            <ArrowDown class="w-5 h-5" />
          </button>
        </Show>
      </div>
    </Surface1>
  );

  return (
    <Switch>
      <Match when={mode() === DebugMode.SCREEN_RIGHT}>
        <Show when={isOpen()}>
          <div class="fixed inset-0 bg-[#00000080] z-40" />
          <div class="fixed top-0 right-0 h-full w-2/3 z-50 shadow-lg p-4 flex flex-col gap-4">
            {menuContent}
          </div>
        </Show>
      </Match>
      <Match
        when={
          mode() === DebugMode.ATTACHED_RIGHT ||
          mode() === DebugMode.ATTACHED_BOTTOM
        }
      >
        {menuContent}
      </Match>
    </Switch>
  );
};
