import {
  type Component,
  createEffect,
  createMemo,
  createRenderEffect,
  createSignal,
  For,
  on,
  Show,
} from "solid-js";
import { useDebug } from "../contexts/DebugContext";
import { Surface1 } from "./Surface";
import ArrowDown from "lucide-solid/icons/arrow-down";
import Trash2 from "lucide-solid/icons/trash-2";
import ToggleRight from "lucide-solid/icons/toggle-right";
import ToggleLeft from "lucide-solid/icons/toggle-left";
import Compass from "lucide-solid/icons/compass";
import {
  channelColors,
  LogChannel,
  type LogMessage,
} from "../contexts/logtypes";

const LogEntry = (props: { log: LogMessage }) => {
  const colorInfo = channelColors[props.log.channel];
  return (
    <div class="flex items-start font-mono text-sm">
      <span class="mr-2 text-gray-500">
        {new Date(props.log.timestamp).toLocaleTimeString()}
      </span>
      <span class="w-[10ch]">
        <span
          class={`mr-2 rounded px-1.5 py-0.5 text-xs font-bold ${colorInfo.bg} ${colorInfo.text}`}
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
    forceScroll,
    searchTerm,
    setSearchTerm,
    isPaused,
    togglePause,
    clearLogs,
  } = useDebug();

  let logContainerRef: HTMLDivElement | null = null;
  const [isScrolledToBottom, setIsScrolledToBottom] = createSignal(true);
  const [showToLatest, setShowToLatest] = createSignal(false);
  const [isSearchVisible, setIsSearchVisible] = createSignal(false);

  const toggleSearch = () => setIsSearchVisible(!isSearchVisible());

  // --- Scrolling Logic ---
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
      if (isScrolledToBottom() && logContainerRef && !isPaused()) {
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
    setShowToLatest(!isAtBottom);
    setIsScrolledToBottom(isAtBottom);
  };

  const scrollToBottom = () => {
    if (logContainerRef) {
      const ref = logContainerRef as HTMLDivElement;
      ref.scrollTop = ref.scrollHeight;
    }
  };

  const displayedLogs = createMemo(() => {
    return logs().filter(
      (log) =>
        isChannelVisible(log.channel) &&
        log.data.toLowerCase().includes(searchTerm().toLowerCase()),
    );
  });

  const channels: LogChannel[] = [
    LogChannel.RAW_IN,
    LogChannel.RAW_OUT,
    LogChannel.UPDATES,
    LogChannel.RPC,
    LogChannel.APP,
  ];

  const menuContent = (
    <Surface1 class="flex h-full flex-grow flex-col gap-2 p-4">
      <div class="mb-2 flex items-center justify-between">
        <h2 class="text-xl font-bold">Debug Communication</h2>

        <div class="flex items-center gap-2">
          <div class="flex overflow-hidden rounded">
            <For each={channels}>
              {(channel) => {
                const colorInfo = channelColors[channel];
                const isVisible = () => visibleChannels().includes(channel);
                return (
                  <button
                    onClick={() => toggleChannel(channel)}
                    class={`cursor-pointer p-2 text-sm transition-all ${
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

          <div class="bg-thm-surface-2 flex items-center gap-1 rounded-md p-1">
            <button
              onClick={toggleSearch}
              class={`cursor-pointer rounded p-1.5 transition-all ${isSearchVisible() ? "bg-thm-primary hover:brightness-110" : "hover:bg-thm-surface-3"}`}
              title="Filter Logs"
            >
              <Compass class="h-5 w-5" />
            </button>
            <button
              onClick={togglePause}
              class="hover:bg-thm-surface-3 cursor-pointer rounded p-1.5 transition-colors"
              title={isPaused() ? "Resume" : "Pause"}
            >
              <Show when={isPaused()} fallback={<ToggleLeft class="h-5 w-5" />}>
                <ToggleRight class="h-5 w-5" />
              </Show>
            </button>
            <button
              onClick={clearLogs}
              class="hover:bg-thm-error cursor-pointer rounded p-1.5 transition-colors"
              title="Clear Logs"
            >
              <Trash2 class="h-5 w-5" />
            </button>

            <div class="border-thm-surface-border-3 mx-1 h-5 border-l"></div>

            <button
              onClick={toggleMenu}
              class="hover:bg-thm-error cursor-pointer rounded p-1.5 transition-colors"
              title="Close Menu"
            >
              <span class="flex size-5 items-center justify-center">
                &times;
              </span>
            </button>
          </div>
        </div>
      </div>
      <Show when={isSearchVisible()}>
        <input
          type="text"
          placeholder="Filter logs..."
          class="bg-thm-surface-2 border-thm-surface-border-2 w-full rounded-md border p-2 text-sm"
          value={searchTerm()}
          onInput={(e) => setSearchTerm(e.currentTarget.value)}
        />
      </Show>

      <div class="relative flex min-h-0 flex-1 flex-col">
        <div
          ref={logContainerRef!}
          onScroll={handleScroll}
          class="bg-thm-surface-2 flex flex-grow flex-col gap-1 overflow-y-scroll rounded p-2"
        >
          <For each={displayedLogs()}>{(log) => <LogEntry log={log} />}</For>
        </div>
        <Show when={showToLatest()}>
          <button
            onClick={scrollToBottom}
            class="bg-thm-primary hover:bg-thm-accent absolute right-2 bottom-2 rounded-full p-2 shadow-lg"
            title="Scroll to latest"
          >
            <ArrowDown class="h-5 w-5" />
          </button>
        </Show>
      </div>
    </Surface1>
  );

  return (
    <Show when={isOpen()}>
      <div class="fixed inset-0 z-40 bg-[#00000080]" onClick={toggleMenu} />
      <div class="fixed top-0 right-0 z-50 flex h-full w-2/3 flex-col gap-4 p-4 shadow-lg">
        {menuContent}
      </div>
    </Show>
  );
};
