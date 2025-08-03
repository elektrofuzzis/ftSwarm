const styles = {
  errorLabel:
    "background: #F44336; color: #fff; padding: 2px 4px; border-radius: 4px; font-weight: bold;",
  warnLabel:
    "background: #FF9800; color: #000; padding: 2px 4px; border-radius: 4px; font-weight: bold;",
  successLabel:
    "background: #4CAF50; color: #000; padding: 2px 4px; border-radius: 4px; font-weight: bold;",
  infoLabel:
    "background: #2196F3; color: #fff; padding: 2px 4px; border-radius: 4px; font-weight: bold;",
  debugLabel:
    "background: #607D8B; color: #fff; padding: 2px 4px; border-radius: 4px; font-weight: bold;",
  timestamp: "color: #9E9E9E; font-style: italic;",
  message: "",
  errorName: "color: #D32F2F; font-weight: bold;",
  errorMessage: "color: #D32F2F;",
  metadataLabel: "color: #9E9E9E; font-style: italic;",
  stackTrace: "color: #757575;",
  reset: "color: unset;",
};

enum LogLevel {
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

class Logger {
  private logLevel: LogLevel;

  constructor(level: LogLevel = LogLevel.INFO) {
    this.logLevel = level;
  }

  private formatMessage(level: LogLevel, ...args: any[]): void {
    if (level > this.logLevel) return;

    const timestamp = new Date().toISOString();
    let label = "";
    let labelStyle = "";
    const logArgs: any[] = [];

    switch (level) {
      case LogLevel.ERROR:
        label = "%c ERROR %c";
        labelStyle = styles.errorLabel;
        break;
      case LogLevel.WARN:
        label = "%c WARN %c ";
        labelStyle = styles.warnLabel;
        break;
      case LogLevel.SUCCESS:
        label = "%c SUCCESS %c";
        labelStyle = styles.successLabel;
        break;
      case LogLevel.INFO:
        label = "%c INFO %c ";
        labelStyle = styles.infoLabel;
        break;
      case LogLevel.DEBUG:
        label = "%c DEBUG %c";
        labelStyle = styles.debugLabel;
        break;
    }

    let messageString = `%c[${timestamp}] ${label} `;
    logArgs.push(styles.timestamp, labelStyle, styles.reset);

    for (const arg of args) {
      if (arg instanceof RichError) {
        messageString += `\n%c${arg.name}: %c${arg.message}`;
        logArgs.push(styles.errorName, styles.errorMessage);

        if (arg.metadata) {
          messageString += `\n%cMetadata: %o`;
          logArgs.push(styles.metadataLabel);
          logArgs.push(arg.metadata);
        }

        if (arg.stack) {
          messageString += `\n%cStack Trace:\n%c${arg.stack}`;
          logArgs.push(styles.metadataLabel, styles.stackTrace);
        }
      } else if (arg instanceof Error) {
        messageString += `%c${arg.name}: %c${arg.message}`;
        logArgs.push(styles.errorName, styles.errorMessage);
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

    switch (level) {
      case LogLevel.ERROR:
        console.warn(messageString, ...logArgs);
        break;
      case LogLevel.WARN:
        console.warn(messageString, ...logArgs);
        break;
      default:
        console.log(messageString, ...logArgs);
        break;
    }
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

let initialLogLevel = LogLevel.INFO;

if (import.meta.env.DEV) {
  initialLogLevel = LogLevel.DEBUG;
}

// Check for Vite config (import.meta.env)
if (
  typeof import.meta !== "undefined" &&
  import.meta.env &&
  typeof import.meta.env.VITE_LOG_LEVEL !== "undefined"
) {
  const viteLevel = String(import.meta.env.VITE_LOG_LEVEL).toUpperCase();
  switch (viteLevel) {
    case "ERROR":
      initialLogLevel = LogLevel.ERROR;
      break;
    case "WARN":
      initialLogLevel = LogLevel.WARN;
      break;
    case "SUCCESS":
      initialLogLevel = LogLevel.SUCCESS;
      break;
    case "INFO":
      initialLogLevel = LogLevel.INFO;
      break;
    case "DEBUG":
      initialLogLevel = LogLevel.DEBUG;
      break;
  }
}

// Check for localStorage override
if (typeof window !== "undefined" && window.localStorage) {
  const storedLevel = window.localStorage.getItem("logLevel");
  if (storedLevel) {
    switch (storedLevel.toUpperCase()) {
      case "ERROR":
        initialLogLevel = LogLevel.ERROR;
        break;
      case "WARN":
        initialLogLevel = LogLevel.WARN;
        break;
      case "SUCCESS":
        initialLogLevel = LogLevel.SUCCESS;
        break;
      case "INFO":
        initialLogLevel = LogLevel.INFO;
        break;
      case "DEBUG":
        initialLogLevel = LogLevel.DEBUG;
        break;
    }
  }
}

const logger = new Logger(initialLogLevel);

// Global functions to enable/disable debugging in production
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
