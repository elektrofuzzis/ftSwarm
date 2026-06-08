import {createEffect, createSignal, Show, type Component} from "solid-js";
import {LoginState, useLoginContext} from "../contexts/LoginContext";
import {useAuthenticator} from "../api/authenticator.ts";

export const LoginOverlay: Component = () => {
    const loginState = useLoginContext();
    const authenticator = useAuthenticator();
    let dialogRef: HTMLDialogElement | undefined;

    createEffect(() => {
        const isLoggingIn = loginState.status() === LoginState.LOGGING_IN;

        if (isLoggingIn && !dialogRef?.open) {
            dialogRef?.showModal();
        } else if (!isLoggingIn && dialogRef?.open) {
            dialogRef?.close();
        }
    });

    let [swarmPin, setSwarmPin] = createSignal("")
    const [networkError, setNetworkError] = createSignal(null as string | null)
    const error = () => {
        const pin = swarmPin();
        if (pin.length !== 4)
            return "Pin must be 4 digits";
        if (isNaN(Number(pin)))
            return "Pin must be a number";
        return networkError();
    };

    createEffect(() => {
        swarmPin();
        setNetworkError(null)
    })

    async function performLogin() {
        const result = await authenticator.authenticate(swarmPin())
        result.mapErr(setNetworkError)
    }

    return (
        <dialog
            ref={dialogRef}
            onClose={() => {
                if (loginState.status() === LoginState.LOGGING_IN) loginState.logout()
            }}
            class="bg-thm-surface-1 border m-auto border-thm-surface-border-1 p-6 rounded-lg shadow-xl backdrop:bg-black/60 backdrop:backdrop-blur-sm focus:outline-none"
        >
            <div class="flex flex-col">
                <div class="flex justify-between">
                    <h2 class="text-xl font-bold text-thm-font">Logging In</h2>

                    <button
                        onClick={loginState.logout}
                        class="text-thm-font-muted cursor-pointer p-1"
                    >
                        &times;
                    </button>
                </div>

                <p class="text-thm-font-muted my-2">Please enter the swarm pin below</p>

                <input
                    type="text"
                    class="w-full px-3 py-2 border border-thm-surface-border-2 rounded transition-colors text-thm-font min-w-80"
                    placeholder="Enter swarm pin"
                    onInput={(e) => setSwarmPin(e.target.value)}
                />

                <Show when={error() != null}>
                    <p class="text-thm-error">{error()}</p>
                </Show>

                <button
                    onClick={performLogin}
                    class="mt-6 px-4 py-2 bg-thm-surface-2 hover:bg-thm-surface-3 border border-thm-surface-border-2 rounded transition-colors text-thm-font disabled:opacity-50"
                    disabled={error() != null}
                >
                    Login
                </button>
            </div>
        </dialog>
    );
};
