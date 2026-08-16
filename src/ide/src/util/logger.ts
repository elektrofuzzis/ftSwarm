const styles = {
  errorLabel:
    "background:#F44336;color:#fff;padding:2px 4px;border-radius:4px;font-weight:bold;",
  warnLabel:
    "background:#FF9800;color:#000;padding:2px 4px;border-radius:4px;font-weight:bold;",
  successLabel:
    "background:#4CAF50;color:#000;padding:2px 4px;border-radius:4px;font-weight:bold;",
  infoLabel:
    "background:#2196F3;color:#fff;padding:2px 4px;border-radius:4px;font-weight:bold;",
  debugLabel:
    "background:#607D8B;color:#fff;padding:2px 4px;border-radius:4px;font-weight:bold;",
  timestamp: "color:#9E9E9E;font-style:italic;",
  errorName: "color:#D32F2F;font-weight:bold;",
  errorMessage: "color:#D32F2F;",
  metadataLabel: "color:#9E9E9E;font-style:italic;",
  stackTrace: "color:#757575;",
  reset: "color:unset;",
};

export const enum LogLevel {
  ERROR = 0,
  WARN,
  SUCCESS,
  INFO,
  DEBUG,
}

interface RichErrorOptions {
  metadata?: Record<string, any>;
}

export class RichError extends Error {
  public readonly metadata: Record<string, any> | undefined;
  constructor(message: string, options?: RichErrorOptions) {
    super(message);
    this.name = "RichError";
    this.metadata = options?.metadata;
  }
}

const LABELS = [
  ["%c ERROR %c", styles.errorLabel],
  ["%c WARN %c ", styles.warnLabel],
  ["%c SUCCESS %c", styles.successLabel],
  ["%c INFO %c ", styles.infoLabel],
  ["%c DEBUG %c", styles.debugLabel],
];

const consoleMethods: Record<LogLevel, typeof console.log> = {
  [LogLevel.ERROR]: console.error,
  [LogLevel.WARN]: console.warn,
  [LogLevel.SUCCESS]: console.log,
  [LogLevel.INFO]: console.log,
  [LogLevel.DEBUG]: console.debug,
};

class Logger {
  private logLevel: LogLevel;

  constructor(level: LogLevel = LogLevel.INFO) {
    this.logLevel = level;
  }

  private formatMessage(level: LogLevel, ...args: any[]): void {
    if (level > this.logLevel) return;

    const [label, labelStyle] = LABELS[level];
    let messageString = `%c[${new Date().toISOString()}] ${label} `;
    const logArgs: any[] = [styles.timestamp, labelStyle, styles.reset];

    for (const arg of args) {
      if (arg instanceof Error) {
        messageString +=
          (arg instanceof RichError ? "" : " ") +
          `%c${arg.name}: %c${arg.message}`;
        logArgs.push(styles.errorName, styles.errorMessage);

        if (arg instanceof RichError && arg.metadata) {
          messageString += `\n%cMetadata: %o`;
          logArgs.push(styles.metadataLabel, arg.metadata);
        }

        if (arg.stack) {
          messageString += `\n%cStack Trace:\n%c${arg.stack}`;
          logArgs.push(styles.metadataLabel, styles.stackTrace);
        }
      } else if (typeof arg === "object") {
        messageString += ` %o`;
        logArgs.push(arg);
      } else {
        messageString += ` %s`;
        logArgs.push(String(arg));
      }
    }

    const consoleMethod = consoleMethods[level];
    consoleMethod(messageString, ...logArgs);
  }

  public info(...args: any[]): void {
    this.formatMessage(LogLevel.INFO, ...args);
  }
  public success(...args: any[]): void {
    this.formatMessage(LogLevel.SUCCESS, ...args);
  }
  public warn(...args: any[]): void {
    this.formatMessage(LogLevel.WARN, ...args);
  }
  public error(...args: any[]): void {
    this.formatMessage(LogLevel.ERROR, ...args);
  }
  public debug(...args: any[]): void {
    this.formatMessage(LogLevel.DEBUG, ...args);
  }
}

const MAP: Record<string, LogLevel> = {
  ERROR: 0,
  WARN: 1,
  SUCCESS: 2,
  INFO: 3,
  DEBUG: 4,
};
let initialLogLevel = import.meta.env.DEV ? LogLevel.DEBUG : LogLevel.INFO;

const envLevel = import.meta.env?.VITE_LOG_LEVEL;
if (envLevel && MAP[String(envLevel).toUpperCase()] !== undefined) {
  initialLogLevel = MAP[String(envLevel).toUpperCase()];
}

if (typeof window !== "undefined" && window.localStorage) {
  const storedLevel = window.localStorage.getItem("logLevel");
  if (storedLevel && MAP[storedLevel.toUpperCase()] !== undefined) {
    initialLogLevel = MAP[storedLevel.toUpperCase()];
  }
}

const logger = new Logger(initialLogLevel);

if (typeof window !== "undefined") {
  (window as any).enableDebugLogging = () => {
    window.localStorage.setItem("logLevel", "DEBUG");
    logger["logLevel"] = LogLevel.DEBUG;
  };
  (window as any).disableDebugLogging = () => {
    window.localStorage.setItem("logLevel", "INFO");
    logger["logLevel"] = LogLevel.INFO;
  };
}

export default logger;
