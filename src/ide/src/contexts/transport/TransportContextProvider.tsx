import {
  createMemo,
  createResource,
  createSignal,
  Match,
  Switch,
  type ParentComponent,
} from "solid-js";
import type {
  CommunicationError,
  TransportAdapter,
  TransportFactory,
} from "../../api/transport";
import { LoadingScreen, LoadingStep } from "../../components/LoadingScreen";
import type {
  SwarmToSocketStateUpdate,
  SwarmToSocketSubscription,
} from "../../api/transport/swarm2socket";
import logger from "../../util/logger";
import { TransportContext, OMContext } from "./context";
import { ErrorScreen } from "../../components/ErrorScreen";
import { useDebug, type DebugContextType } from "../DebugContext";
import { LogChannel } from "../logtypes";
import { MissedHeartbeatCounter } from "../../components/MissedHeartbeatCounter";
import { RootObjectModel } from "../../api/om";

class ContextTransportAdapter implements TransportAdapter {
  constructor(
    private readonly debug: DebugContextType,
    private readonly setMissedHeartbeatCount: (count: number) => void,
    private readonly setCommunicationError: (error: CommunicationError) => void,
    private readonly om: RootObjectModel,
  ) {}

  async onOutgoing(message: string): Promise<void> {
    this.debug.addLog(LogChannel.RAW_OUT, message);
  }

  async onIncoming(message: string): Promise<void> {
    this.debug.addLog(LogChannel.RAW_IN, message);
  }

  async onUpdate(message: SwarmToSocketStateUpdate): Promise<void> {
    this.debug.addLog(LogChannel.UPDATES, JSON.stringify(message));
    this.om.update(message.value);
  }

  async onSubscription(message: SwarmToSocketSubscription): Promise<void> {
    logger.info(`Received subscription message: ${JSON.stringify(message)}`);
  }

  async onError(error: CommunicationError): Promise<void> {
    logger.error(`Transport error: ${error.what()}: ${error.resolution()}`);
    this.setCommunicationError(error);
  }

  async setMissedTimer(count: number): Promise<void> {
    this.setMissedHeartbeatCount(count);
  }
}

export const TransportContextProvider: ParentComponent<{
  sourceIp: string;
  factory: TransportFactory;
}> = (props) => {
  const debug = useDebug();
  const [step, setStep] = createSignal(LoadingStep.CONNECTING);
  const [missedHeartbeatCount, setMissedHeartbeatCount] = createSignal(0);
  const [communicationError, setCommunicationError] =
    createSignal<CommunicationError | null>(null);

  const om = createMemo(() => new RootObjectModel());

  const [transport] = createResource(() => {
    setStep(LoadingStep.CONNECTING);
    return props.factory(
      new ContextTransportAdapter(
        debug,
        setMissedHeartbeatCount,
        setCommunicationError,
        om(),
      ),
    );
  });

  const loading = <LoadingScreen currentStep={step()} />;

  return (
    <OMContext.Provider value={om()}>
      <MissedHeartbeatCounter count={missedHeartbeatCount} />
      <Switch>
        <Match when={communicationError() != null}>
          <ErrorScreen
            message={communicationError()!.what()}
            onRestart={() => location.reload()}
          />
        </Match>
        <Match when={transport.loading}>{loading}</Match>
        <Match when={transport.error}>
          <ErrorScreen
            message={transport.error.message}
            onRestart={() => location.reload()}
          />
        </Match>
        <Match when={transport()}>
          <TransportContext.Provider value={transport()}>
            {props.children}
          </TransportContext.Provider>
        </Match>
      </Switch>
    </OMContext.Provider>
  );
};
