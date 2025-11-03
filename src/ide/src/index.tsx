/* @refresh reload */
import { render } from "solid-js/web";
import "./index.css";
import { App } from "./app";
import { HashRouter, Route } from "@solidjs/router";
import { SwarmOverviewRoute } from "./routes/SwarmOverviewRoute";
import logger from "./util/logger";
import { websocketTransportFactory } from "./api/transport/wsTransport";
import { TransportContextProvider } from "./components/transport/TransportContextProvider";

function getSourceIp() {
  if (import.meta.env.DEV && import.meta.env.VITE_SOURCE_IP) {
    return import.meta.env.VITE_SOURCE_IP;
  } else {
    return window.location.hostname;
  }
}

const root = document.getElementById("root");
const sourceIp = getSourceIp();
const factory = websocketTransportFactory(`ws://${sourceIp}/ws`);

logger.info("Welcome to ftSwarm IDE");
logger.debug("Debug logging active");

render(
  () => (
    <TransportContextProvider sourceIp={sourceIp} factory={factory}>
      <HashRouter root={App}>
        <Route path="/" component={SwarmOverviewRoute} />
        <Route path="/bla" component={SwarmOverviewRoute} />
      </HashRouter>
    </TransportContextProvider>
  ),
  root!,
);
