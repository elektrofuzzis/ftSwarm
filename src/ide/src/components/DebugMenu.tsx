import { For, type Component } from "solid-js";
import { useDebug } from "../contexts/DebugContext";
import { Surface1 } from "./Surface";
import X from "lucide-solid/icons/x";
import {
  LogChannel,
  channelColors,
  type LogMessage,
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
  } = useDebug();

  const filteredLogs = () =>
    logs().filter((log) => isChannelVisible(log.channel));

  return (
    <>
      <div
        class="fixed inset-0 bg-[#00000080] z-40 transition-opacity duration-300 ease-in-out"
        classList={{
          "opacity-100": isOpen(),
          "opacity-0 pointer-events-none": !isOpen(),
        }}
      />
      <div
        class="fixed top-0 right-0 h-full w-2/3 z-50 shadow-lg p-4 flex flex-col gap-4 transition-transform duration-300 ease-in-out"
        classList={{
          "translate-x-0": isOpen(),
          "translate-x-full": !isOpen(),
        }}
      >
        <Surface1 class="p-4 flex-grow flex flex-col gap-2">
          <div class="flex justify-between items-center mb-4">
            <h2 class="text-xl font-bold">Debug Communication</h2>
            <button
              onClick={toggleMenu}
              class="p-1 rounded-full hover:bg-thm-surface-2"
            >
              <X class="w-6 h-6" />
            </button>
          </div>

          <div>
            <h3 class="font-bold mb-2">Visible Channels</h3>
            <div class="flex flex-wrap gap-2">
              <For each={Object.values(LogChannel)}>
                {(channel) => {
                  const colorInfo = channelColors[channel];
                  const isVisible = () => visibleChannels().includes(channel);
                  return (
                    <button
                      onClick={() => toggleChannel(channel)}
                      class={`px-2 py-1 text-sm rounded transition-all ${
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
          </div>

          <div class="flex-1 flex flex-col mt-4">
            <h3 class="font-bold mb-2">Logs</h3>
            <div class="flex flex-col gap-1 p-2 rounded bg-thm-surface-2 flex-grow overflow-y-scroll">
              <For each={filteredLogs()}>{(log) => <LogEntry log={log} />}</For>
            </div>
          </div>
        </Surface1>
      </div>
    </>
  );
};
