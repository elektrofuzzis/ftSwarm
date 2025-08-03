/* @refresh reload */
import { render } from "solid-js/web";
import "./index.css";
import { App } from "./app";
import { HashRouter, Route } from "@solidjs/router";
import { SwarmOverviewRoute } from "./routes/SwarmOverviewRoute";
import logger from "./util/logger";
import { WebSocketTransportFactory } from "./api/transport/wsTransport";

const root = document.getElementById("root");
const factory = new WebSocketTransportFactory("ws://localhost:8080");

logger.info("Welcome to ftSwarm IDE");
logger.debug("Debug logging active");

render(
  () => (
    <HashRouter root={App}>
      <Route path="/" component={SwarmOverviewRoute} />
      <Route path="/bla" component={SwarmOverviewRoute} />
    </HashRouter>
  ),
  root!,
);
