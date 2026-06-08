import {createContext, createSignal, useContext, type Accessor, type ParentComponent} from "solid-js";

export enum LoginState {
    LOGGED_OUT,
    LOGGING_IN,
    LOGGED_IN,
}

export interface LoginContextType {
    status: Accessor<LoginState>;
    startLogin: () => void;
    login: () => void;
    logout: () => void;
    interactiveDisabled: Accessor<boolean>
}

const LoggedInContext = createContext<LoginContextType>();


export const LoggedInContextProvider: ParentComponent = (props) => {
    const [isLoggedIn, setIsLoggedIn] = createSignal(LoginState.LOGGED_OUT);

    const startLogin = () => {
        setIsLoggedIn(LoginState.LOGGING_IN);
    };

    const login = () => {
        setIsLoggedIn(LoginState.LOGGED_IN);
    };

    const logout = () => {
        setIsLoggedIn(LoginState.LOGGED_OUT);
    };

    return (
        <LoggedInContext.Provider value={{
            status: isLoggedIn,
            startLogin,
            login,
            logout,
            interactiveDisabled: () => isLoggedIn() !== LoginState.LOGGED_IN
        }}>
            {props.children}
        </LoggedInContext.Provider>
    );
};

export const useLoginContext = () => {
    const context = useContext(LoggedInContext);
    if (!context) {
        throw new Error("useLoginContext must be used within a LoggedInContextProvider");
    }
    return context;
};
