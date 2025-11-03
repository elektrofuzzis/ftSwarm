import { createContext } from "solid-js";
import type { Transport } from "../../api/transport";

export const TransportContext = createContext<Transport>();
