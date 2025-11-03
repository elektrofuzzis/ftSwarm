import { createSignal, type ParentComponent } from "solid-js";
import type { TransportFactory } from "../api/transport";
import { LoadingScreen, LoadingStep } from "./LoadingScreen";
import logger from "../util/logger";
import { makeTimer } from "@solid-primitives/timer";

export const TransportContext: ParentComponent<{
  sourceIp: string;
  factory: TransportFactory;
}> = (props) => {
  const [step, setStep] = createSignal(LoadingStep.CONNECTING);

  makeTimer(
    () => {
      if (step() === LoadingStep.CONNECTING) {
        setStep(LoadingStep.LOADING);
      } else {
        setStep(LoadingStep.CONNECTING);
      }
    },
    3000,
    setInterval,
  );

  return (
    <>
      {/*{props.children}*/}
      <LoadingScreen currentStep={step()} />
    </>
  );
};
