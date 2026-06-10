import {
  createContext,
  createSignal,
  useContext,
  type ParentComponent,
  type Accessor,
  onMount,
} from "solid-js";
import { LogChannel, type LogMessage } from "./logtypes";

export type DebugContextType = {
  isOpen: Accessor<boolean>;
  isUnlocked: Accessor<boolean>;
  toggleMenu: () => void;
  logs: Accessor<LogMessage[]>;
  addLog: (channel: LogChannel, data: string) => void;
  visibleChannels: Accessor<LogChannel[]>;
  toggleChannel: (channel: LogChannel) => void;
  isChannelVisible: (channel: LogChannel) => boolean;
  forceScroll: Accessor<number>;
  searchTerm: Accessor<string>;
  setSearchTerm: (term: string) => void;
  isPaused: Accessor<boolean>;
  togglePause: () => void;
  clearLogs: () => void;
};

const DebugContext = createContext<DebugContextType>();

const MAX_LOGS = 1000;

export const DebugContextProvider: ParentComponent = (props) => {
  const [isOpen, setIsOpen] = createSignal(false);
  const [isUnlocked, setIsUnlocked] = createSignal(false);
  const [logs, setLogs] = createSignal<LogMessage[]>([]);
  const [visibleChannels, setVisibleChannels] = createSignal<LogChannel[]>([
    LogChannel.RPC,
    LogChannel.APP,
  ]);
  const [forceScroll, setForceScroll] = createSignal(0);
  const [searchTerm, setSearchTerm] = createSignal("");
  const [isPaused, setIsPaused] = createSignal(false);

  const addLog = (channel: LogChannel, data: string) => {
    // Do not add new logs if paused to prevent memory build-up of hidden logs
    if (isPaused()) return;
    setLogs((prev) => {
      const list = [...prev, { channel, data, timestamp: Date.now() }];
      if (list.length > MAX_LOGS) {
        list.shift();
      }
      return list;
    });
  };

  const clearLogs = () => {
    setLogs([]);
    addLog(LogChannel.APP, "Logs cleared.");
  };

  const togglePause = () => setIsPaused(!isPaused());

  const toggleChannel = (channel: LogChannel) => {
    if (visibleChannels().includes(channel)) {
      setVisibleChannels(visibleChannels().filter((c) => c !== channel));
    } else {
      setVisibleChannels([...visibleChannels(), channel]);
    }
    setForceScroll((p) => p + 1);
  };

  const isChannelVisible = (channel: LogChannel) => {
    return visibleChannels().includes(channel);
  };

  const toggleMenu = () => {
    const newIsOpen = !isOpen();
    if (newIsOpen && !isUnlocked()) {
      setIsUnlocked(true);
    }
    setIsOpen(newIsOpen);
  };

  onMount(() => {
    addLog(LogChannel.APP, "Welcome to the Debug Session!");
  });

  return (
    <DebugContext.Provider
      value={{
        isOpen,
        isUnlocked,
        toggleMenu,
        logs,
        addLog,
        visibleChannels,
        toggleChannel,
        isChannelVisible,
        forceScroll,
        searchTerm,
        setSearchTerm,
        isPaused,
        togglePause,
        clearLogs,
      }}
    >
      {props.children}
    </DebugContext.Provider>
  );
};

export const useDebug = () => {
  const context = useContext(DebugContext);
  if (!context) {
    throw new Error("useDebug must be used within a DebugContextProvider");
  }
  return context;
};
