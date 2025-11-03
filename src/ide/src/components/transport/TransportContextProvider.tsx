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
import { LoadingScreen, LoadingStep } from "../LoadingScreen";
import type { SwarmToSocketSubscription } from "../../api/transport/swarm2socket";
import logger from "../../util/logger";
import { TransportContext } from "./context";

class ContextTransportAdapter implements TransportAdapter {
  async onSubscription(message: SwarmToSocketSubscription): Promise<void> {
    logger.info(`Received subscription message: ${JSON.stringify(message)}`);
  }
  async onError(error: CommunicationError): Promise<void> {
    logger.error(`Transport error: ${error}`);
  }
}

export const TransportContextProvider: ParentComponent<{
  sourceIp: string;
  factory: TransportFactory;
}> = (props) => {
  const [step, setStep] = createSignal(LoadingStep.CONNECTING);
  const [transport, { refetch: reloadTransport }] = createResource(() =>
    props.factory(new ContextTransportAdapter()),
  );

  return (
    <Suspense fallback={<LoadingScreen currentStep={step()} />}>
      <Switch>
        <Match when={transport.error}>error happened: {transport.error}</Match>
        <Match when={transport()}>
          <TransportContext.Provider value={transport()}>
            {props.children}
          </TransportContext.Provider>
        </Match>
      </Switch>
    </Suspense>
  );
};
