import type {
  SwarmToSocketError,
  SwarmToSocketRpcResponse,
  SwarmToSocketSubscription,
} from "./swarm2socket";

export interface Transport {
  send(data: string): Promise<void>;
  receiveResult(): Promise<SwarmToSocketRpcResponse | SwarmToSocketError>;
}

export interface TransportFactory {
  createTransport(
    onSubscriptionResult: (message: SwarmToSocketSubscription) => void,
  ): Transport;
}
