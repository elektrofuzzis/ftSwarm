import {useTransportContext} from "../contexts/transport/context.ts";
import {useLoginContext} from "../contexts/LoginContext.tsx";
import {transactMessage} from "./transport";
import {Result, Unit} from "../util/result.ts";

export type Authenticator = {
    authenticate: (code: String) => Promise<Result<Unit, AuthenticatorError>>;
    logout: () => void;
}

export enum AuthenticatorError {
    INVALID_CODE = "Invalid Code",
    UNKNOWN_ERROR = "Unknown Error",
}

export const useAuthenticator = () => {
    let transport = useTransportContext()
    let state = useLoginContext()

    return {
        async authenticate(code: String): Promise<Result<Unit, AuthenticatorError>> {
            const result = await transactMessage(transport, `swarm.login(${code})`)
            console.log(result)
            return result
                .mapErr(({message}) => message.includes("wrong pin") ? AuthenticatorError.INVALID_CODE : AuthenticatorError.UNKNOWN_ERROR)
                .map(_ => {
                    // State transition: we're logged in
                    state.login()
                    return Unit
                })
        },
        logout: function (): void {
            // Just transition to logged-out state in the frontend.
            state.logout()
        }
    } satisfies Authenticator
}