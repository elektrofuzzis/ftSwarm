import { createEffect, createSignal, Show, type Component } from "solid-js";
import { LoginState, useLoginContext } from "../contexts/LoginContext";
import { useAuthenticator } from "../api/authenticator.ts";

export const LoginOverlay: Component = () => {
  const loginState = useLoginContext();
  const authenticator = useAuthenticator();
  let dialogRef: HTMLDialogElement | undefined;
  let inputRef: HTMLInputElement | undefined;

  createEffect(() => {
    const isLoggingIn = loginState.status() === LoginState.LOGGING_IN;

    if (isLoggingIn && !dialogRef?.open) {
      dialogRef?.showModal();
      inputRef?.focus();
    } else if (!isLoggingIn && dialogRef?.open) {
      dialogRef?.close();
    }
  });

  let [swarmPin, setSwarmPin] = createSignal("");
  const [networkError, setNetworkError] = createSignal(null as string | null);
  const error = () => {
    const pin = swarmPin();
    if (pin.length !== 4) return "Pin must be 4 digits";
    if (isNaN(Number(pin))) return "Pin must be a number";
    return networkError();
  };

  createEffect(() => {
    swarmPin();
    setNetworkError(null);
  });

  async function performLogin() {
    const result = await authenticator.authenticate(swarmPin());
    result.mapErr(setNetworkError);
  }

  return (
    <dialog
      ref={dialogRef}
      onClose={() => {
        if (loginState.status() === LoginState.LOGGING_IN) loginState.logout();
      }}
      class="bg-thm-surface-1 border-thm-surface-border-1 m-auto rounded-lg border p-6 shadow-xl backdrop:bg-black/60 backdrop:backdrop-blur-sm focus:outline-none"
    >
      <div class="flex flex-col">
        <div class="flex justify-between">
          <h2 class="text-thm-font text-xl font-bold">Logging In</h2>

          <button
            onClick={loginState.logout}
            class="text-thm-font-muted cursor-pointer p-1"
          >
            &times;
          </button>
        </div>

        <p class="text-thm-font-muted my-2">Please enter the swarm pin below</p>

        <input
          ref={inputRef}
          type="text"
          class="border-thm-surface-border-2 text-thm-font w-full min-w-80 rounded border px-3 py-2 transition-colors"
          placeholder="Enter swarm pin"
          onInput={(e) => setSwarmPin(e.target.value)}
          onKeyDown={(e) => {
            if (e.key === "Enter" && error() == null) {
              performLogin();
            }
          }}
        />

        <Show when={error() != null}>
          <p class="text-thm-error">{error()}</p>
        </Show>

        <button
          onClick={performLogin}
          class="bg-thm-surface-2 hover:bg-thm-surface-3 border-thm-surface-border-2 text-thm-font mt-6 rounded border px-4 py-2 transition-colors disabled:opacity-50"
          disabled={error() != null}
        >
          Login
        </button>
      </div>
    </dialog>
  );
};
