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
    <div class="flex w-full h-full items-center justify-center">
      <div>
        <WifiOff class="w-16 h-16 mb-4 text-thm-error mx-auto" />
        <Surface1 class="p-4">
          <h1 class="text-2xl mb text-thm-font">An Error Occurred</h1>
          <p class="text-lg mb-4 text-thm-font-muted max-w-[30ch]">
            The connection failed. Please check ftSwarm and network. If the
            problem persists, try restarting the device.
          </p>
          <p class="text-lg mb-4 text-thm-error font-mono">{props.message}</p>
          <Show when={props.onRestart}>
            <Button variant="primary" onClick={props.onRestart}>
              Try again
            </Button>
          </Show>
        </Surface1>
      </div>
      <div class="fixed bottom-0 left-0 right-0">
        <ReadTheDocs />
      </div>
      <PatternedBackground />
    </div>
  );
};
