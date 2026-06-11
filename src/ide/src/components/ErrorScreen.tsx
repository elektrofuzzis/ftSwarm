import { Show, type Component } from "solid-js";
import { Surface1 } from "./Surface";
import { ReadTheDocs } from "./ReadTheDocs";
import PatternedBackground from "./PatternedBackground";
import WifiOff from "lucide-solid/icons/wifi-off";
import { Button } from "./Button";

export const ErrorScreen: Component<{
  message: string;
  onRestart?: () => void;
}> = (props) => {
  return (
    <div class="flex h-full w-full items-center justify-center">
      <div>
        <WifiOff class="text-thm-error mx-auto mb-4 h-16 w-16" />
        <Surface1 class="p-4">
          <h1 class="mb text-thm-font text-2xl">An Error Occurred</h1>
          <p class="text-thm-font-muted mb-4 max-w-[30ch] text-lg">
            The connection failed. Please check ftSwarm and network. If the
            problem persists, try restarting the device.
          </p>
          <p class="text-thm-error mb-4 font-mono text-lg">{props.message}</p>
          <Show when={props.onRestart}>
            <Button variant="primary" onClick={props.onRestart}>
              Try again
            </Button>
          </Show>
        </Surface1>
      </div>
      <div class="fixed right-0 bottom-0 left-0">
        <ReadTheDocs />
      </div>
      <PatternedBackground />
    </div>
  );
};
