import type {
  SwarmToSocketRpcResponse,
  SwarmToSocketSubscription,
} from "./swarm2socket";

export enum ErrorResolution {
  FAIL,
  RECONNECT,
  IGNORE,
}

export interface CommunicationError {
  what(): string;
  resolution(): ErrorResolution;
}

export class TransportError implements CommunicationError {
  constructor(
    private readonly message: string,
    private readonly resolve: ErrorResolution,
  ) {}

  what(): string {
    return this.message;
  }

  resolution(): ErrorResolution {
    return this.resolve;
  }
}

export interface Transport {
  applySync<T>(func: () => Promise<T>): Promise<T>;
  send(data: string): Promise<void>;
  receiveResult(): Promise<SwarmToSocketRpcResponse>;
}

export interface TransportAdapter {
  onConnected(): Promise<void>;
  onOutgoing(message: string): Promise<void>;
  onIncoming(message: string): Promise<void>;
  onUpdate(message: any): Promise<void>;
  onSubscription(message: SwarmToSocketSubscription): Promise<void>;
  onError(error: CommunicationError): Promise<void>;
  setMissedTimer(count: number): Promise<void>;
}

export type TransportFactory = (
  adapter: TransportAdapter,
) => Promise<Transport>;

export async function transactMessage(
  transport: Transport,
  message: string,
): Promise<SwarmToSocketRpcResponse> {
  return transport.applySync(async () => {
    await transport.send(message);
    return await transport.receiveResult();
  });
}
