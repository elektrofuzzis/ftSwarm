export const enum LogChannel {
  RAW_IN = "Raw In",
  RAW_OUT = "Raw Out",
  UPDATES = "Updates",
  RPC = "RPC",
  APP = "App",
}

export type LogMessage = {
  channel: LogChannel;
  timestamp: number;
  data: string;
};

export const channelColors: Record<LogChannel, { text: string; bg: string }> = {
  [LogChannel.RAW_IN]: { text: "text-green-400", bg: "bg-green-900" },
  [LogChannel.RAW_OUT]: { text: "text-blue-400", bg: "bg-blue-900" },
  [LogChannel.UPDATES]: { text: "text-yellow-400", bg: "bg-yellow-900" },
  [LogChannel.RPC]: { text: "text-cyan-400", bg: "bg-cyan-900" },
  [LogChannel.APP]: { text: "text-purple-400", bg: "bg-purple-900" },
};
