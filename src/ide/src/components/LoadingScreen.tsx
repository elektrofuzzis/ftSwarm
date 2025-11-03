import type { Component } from "solid-js";
import { Surface1 } from "./Surface";
import { ReadTheDocs } from "./ReadTheDocs";
import PatternedBackground from "./PatternedBackground";

export enum LoadingStep {
  CONNECTING,
  LOADING,
}

const style = `
@keyframes loading-bar-move {
  0% {
    background-position: 0 0;
  }
  100% {
    background-position: 64px 0;
  }
}
.loading-bar-animated {
  animation: loading-bar-move 2s linear infinite;
  transition: width 0.25s cubic-bezier(0.175, 0.885, 0.32, 1.275) !important;
}
`;

export const LoadingScreen: Component<{ currentStep: LoadingStep }> = (
  props,
) => {
  return (
    <div class="flex w-full h-full items-center justify-center">
      <style>{style}</style>
      <Surface1 class="p-4">
        <h1 class="text-2xl mb text-thm-font">Loading ftSwarm Dashboard</h1>
        <p class="text-lg mb-4 text-thm-font-muted">
          {props.currentStep === LoadingStep.CONNECTING
            ? "Connecting to ftSwarm..."
            : "Loading data..."}
        </p>
        <div class="w-full h-4 rounded bg-thm-surface-2 border border-thm-surface-border-2 overflow-hidden relative">
          <div
            class={`h-full transition-all duration-500 loading-bar-animated`}
            style={{
              width:
                props.currentStep === LoadingStep.CONNECTING
                  ? "33.33%"
                  : "66.66%",
              "background-color": "var(--thm-primary)",
              "background-image":
                "repeating-linear-gradient(135deg, var(--thm-accent), var(--thm-accent) 8px, var(--thm-primary) 8px, var(--thm-primary) 16px)",
              "box-shadow": "0 0 4px 0 var(--thm-primary)",
              "background-size": "64px 100%",
            }}
          />
          <div
            class="absolute inset-0 rounded pointer-events-none"
            style={{
              border: "1px solid var(--thm-surface-border-2)",
              "box-sizing": "border-box",
            }}
          />
        </div>
      </Surface1>
      <div class="fixed bottom-0 left-0 right-0">
        <ReadTheDocs />
      </div>
      <PatternedBackground />
    </div>
  );
};
