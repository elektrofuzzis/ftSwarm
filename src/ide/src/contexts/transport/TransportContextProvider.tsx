import {
  createResource,
  createSignal,
  Match,
  Suspense,
  Switch,
  type ParentComponent,
} from "solid-js";
import type {
  CommunicationError,
  TransportAdapter,
  TransportFactory,
} from "../../api/transport";
import { LoadingScreen, LoadingStep } from "../../components/LoadingScreen";
import type { SwarmToSocketSubscription } from "../../api/transport/swarm2socket";
import logger from "../../util/logger";
import { TransportContext } from "./context";
import { ErrorScreen } from "../../components/ErrorScreen";
import { useDebug, type DebugContextType } from "../DebugContext";
import { LogChannel } from "../logtypes";

class ContextTransportAdapter implements TransportAdapter {
  constructor(private readonly debug: DebugContextType) {}

  async onOutgoing(message: string): Promise<void> {
    this.debug.addLog(LogChannel.RAW_OUT, message);
  }

  async onIncoming(message: string): Promise<void> {
    this.debug.addLog(LogChannel.RAW_IN, message);
  }

  async onUpdate(message: any): Promise<void> {
    this.debug.addLog(LogChannel.UPDATES, JSON.stringify(message));
  }

  async onSubscription(message: SwarmToSocketSubscription): Promise<void> {
    logger.info(`Received subscription message: ${JSON.stringify(message)}`);
  }
  async onError(error: CommunicationError): Promise<void> {
    logger.error(`Transport error: ${error.what()}: ${error.resolution()}`);
  }
}

export const TransportContextProvider: ParentComponent<{
  sourceIp: string;
  factory: TransportFactory;
}> = (props) => {
  const debug = useDebug();
  const [step, setStep] = createSignal(LoadingStep.CONNECTING);
  const [transport] = createResource(() => {
    setStep(LoadingStep.CONNECTING);
    return props.factory(new ContextTransportAdapter(debug));
  });

  const loading = <LoadingScreen currentStep={step()} />;

  return (
    <Suspense fallback={loading}>
      <Switch>
        <Match when={transport.loading}>{loading}</Match>
        <Match when={transport.error}>
          <ErrorScreen
            message={transport.error.message}
            onRestart={location.reload}
          />
        </Match>
        <Match when={transport()}>
          <TransportContext.Provider value={transport()}>
            {props.children}
          </TransportContext.Provider>
        </Match>
      </Switch>
    </Suspense>
  );
};
