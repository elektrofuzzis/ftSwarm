import { createContext } from "solid-js";
import type { Transport } from "../../api/transport";
import type { RootObjectModel } from "../../api/om";
import { useContext } from "solid-js";

export const TransportContext = createContext<Transport>();
export const OMContext = createContext<RootObjectModel>();

export function useTransportContext() {
  const context = useContext(TransportContext);
  if (!context) {
    throw new Error(
      "useTransportContext must be used within a TransportContext.Provider",
    );
  }
  return context;
}

export function useOMContext() {
  const context = useContext(OMContext);
  if (!context) {
    throw new Error("useOMContext must be used within a OMContext.Provider");
  }
  return context;
}
