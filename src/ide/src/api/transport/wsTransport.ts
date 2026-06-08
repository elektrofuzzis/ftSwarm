import {
  ErrorResolution,
  TransportError,
  type Transport,
  type TransportAdapter,
  type TransportFactory,
} from ".";
import { Mutex } from "../../util/lock";
import logger from "../../util/logger";
import { decompressBlob } from "../rawTranslator";
import { WatchdogTimer } from "../watchdog";
import type {SwarmToSocketError, SwarmToSocketRpcResponse} from "./swarm2socket";
import { parseSwarmToSocketMessage } from "./swarm2socket";
import {Result} from "../../util/result.ts";

export class WebSocketTransport implements Transport {
  private readonly webSocket: WebSocket;
  private readonly adapter: TransportAdapter;
  private readonly messageQueue: Result<SwarmToSocketRpcResponse, SwarmToSocketError>[] = [];
  private readonly lock: Mutex = new Mutex();
  private readonly waitLocks: (() => void)[] = [];
  private watchdogTimer: WatchdogTimer;

  constructor(webSocket: WebSocket, adapter: TransportAdapter) {
    this.webSocket = webSocket;
    this.adapter = adapter;

    this.webSocket.addEventListener("message", this.handleMessage);
    this.webSocket.addEventListener("error", this.handleError);
    this.webSocket.addEventListener("close", this.handleClose);

    adapter.onConnected();

    this.watchdogTimer = new WatchdogTimer(
      this.webSocket,
      adapter.setMissedTimer.bind(adapter),
      3000,
      () => {
        this.adapter.onError(
          new TransportError("Connection timed out", ErrorResolution.RECONNECT),
        );
      },
    );
  }

  async applySync<T>(func: () => Promise<T>): Promise<T> {
    return this.lock.runExclusive(func);
  }

  private handleError = (error: Event) => {
    console.error("WebSocket error:", error);
    this.adapter.onError(
      new TransportError("WebSocket error occurred", ErrorResolution.RECONNECT),
    );
  };

  private handleClose = (event: CloseEvent) => {
    console.log(`WebSocket connection closed`, event);
    this.watchdogTimer.close();
    this.adapter.onError(
      new TransportError(
        `WebSocket connection closed: ${event.code} ${event.reason}`,
        ErrorResolution.RECONNECT,
      ),
    );
  };

  private handleMessage = async (event: MessageEvent) => {
    this.watchdogTimer.reset();
    // Binary message received
    const message = await decompressBlob(event.data);

    // Skip empty messages
    if (!message.trim()) {
      return;
    }

    await this.adapter.onIncoming(message);

    const parsedResult = parseSwarmToSocketMessage(message);

    if (parsedResult.isErr()) {
      console.error("Failed to parse message:", parsedResult.unwrapErr());
      return;
    }

    const parsedMessage = parsedResult.unwrap();

    switch (parsedMessage.kind) {
      case "subscription":
        // Handle subscription message by calling the callback
        await this.adapter.onSubscription(parsedMessage);
        break;
      case "rpc-response":
        // Queue the result or resolve a pending promise
        this.messageQueue.push(Result.ok(parsedMessage));
        this.waitLocks.shift()?.();
        break;
      case "error":
        this.messageQueue.push(Result.err(parsedMessage));
        this.waitLocks.shift()?.();
        await this.adapter.onError(
            new TransportError(
                `ftSwarm error: ${parsedMessage.message}`,
                ErrorResolution.IGNORE,
            ),
        );
        break;
      case "state-update":
        await this.adapter.onUpdate(parsedMessage);
        break;
      case "log":
        logger.info("ftSwarm log:", parsedMessage.message);
        break;
      case "start-cli":
        console.log("ftSwarm CLI started");
        break;
    }
  };

  async send(data: string): Promise<void> {
    await this.adapter.onOutgoing(data);
    this.webSocket.send(data);
  }

  async receiveResult(): Promise<Result<SwarmToSocketRpcResponse, SwarmToSocketError>> {
    if (this.messageQueue.length > 0) {
      console.log(this.messageQueue);
      return this.messageQueue.shift()!;
    }

    return new Promise<Result<SwarmToSocketRpcResponse, SwarmToSocketError>>((resolve) => {
      this.waitLocks.push(() => {
        if (this.messageQueue.length > 0) {
          console.log(this.messageQueue);
          resolve(this.messageQueue.shift()!);
        } else {
          this.adapter.onError(
            new TransportError(
              "No message available when expected",
              ErrorResolution.FAIL,
            ),
          );
        }
      });
    });
  }
}

export const websocketTransportFactory: (url: string) => TransportFactory =
  (url: string) => async (adapter: TransportAdapter) => {
    const websocket = new WebSocket(url);

    await new Promise<void>((resolve, reject) => {
      function onOpen() {
        websocket.removeEventListener("open", onOpen);
        websocket.removeEventListener("error", onError);
        resolve();
      }

      function onError(_: Event) {
        websocket.removeEventListener("open", onOpen);
        websocket.removeEventListener("error", onError);
        reject(
          new TransportError(
            "WebSocket connection error",
            ErrorResolution.RECONNECT,
          ),
        );
      }

      websocket.addEventListener("open", onOpen);
      websocket.addEventListener("error", onError);
    });

    return new WebSocketTransport(websocket, adapter);
  };
