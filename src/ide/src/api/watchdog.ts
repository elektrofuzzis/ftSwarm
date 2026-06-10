export class WatchdogTimer {
  private watchdogTimer: NodeJS.Timeout | undefined;
  private watchdogErrorCount = 0;

  constructor(
    private readonly websocket: WebSocket,
    private readonly setMissedCount: (count: number) => void = (_) => {},
    private readonly duration: number = 3000,
    private readonly onClose: () => void = () => {},
  ) {
    this.start();
  }

  reset() {
    this.watchdogErrorCount = 0;
    this.setMissedCount(this.watchdogErrorCount);
  }

  private start() {
    this.watchdogTimer = setInterval(() => {
      this.watchdogErrorCount++;
      this.setMissedCount(this.watchdogErrorCount);
      if (this.watchdogErrorCount >= 5) {
        this.websocket.close(3001, "Watchdog timeout");
        this.onClose();
      }
    }, this.duration);
  }

  close() {
    if (this.watchdogTimer) {
      clearInterval(this.watchdogTimer);
    }
  }
}
