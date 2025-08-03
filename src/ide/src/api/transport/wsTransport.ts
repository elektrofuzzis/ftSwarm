import type { Transport, TransportFactory } from ".";
import type {
  SwarmToSocketRpcResponse,
  SwarmToSocketError,
  SwarmToSocketSubscription,
} from "./swarm2socket";

export class WebSocketTransport implements Transport {
  private readonly onSubscriptionResult: (
    message: SwarmToSocketSubscription,
  ) => void;

  constructor(
    onSubscriptionResult: (message: SwarmToSocketSubscription) => void,
  ) {
    this.onSubscriptionResult = onSubscriptionResult;
  }

  send(data: string): Promise<void> {
    throw new Error("Method not implemented.");
  }
  receiveResult(): Promise<SwarmToSocketRpcResponse | SwarmToSocketError> {
    throw new Error("Method not implemented.");
  }
}

export class WebSocketTransportFactory implements TransportFactory {
  constructor(url: string) {
    // Initialize WebSocket connection using the provided URL
  }

  createTransport(
    onSubscriptionResult: (message: SwarmToSocketSubscription) => void,
  ): Transport {
    return new WebSocketTransport(onSubscriptionResult);
  }
}
