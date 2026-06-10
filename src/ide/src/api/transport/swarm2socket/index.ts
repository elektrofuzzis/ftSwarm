import { Result } from "../../../util/result";
import { jsonify } from "../../rawTranslator";
import { SwarmToSocketMessageParseError } from "./error";
import { parseRpcReturnParam, type RpcReturnParam } from "./rpc";
import {
  parseSubscriptionResponse,
  type SubscriptionResponse,
} from "./subscription";

export type SwarmToSocketLog = {
  kind: "log";
  message: string;
};

export type SwarmToSocketRpcResponse = {
  kind: "rpc-response";
  param: RpcReturnParam;
};

export type SwarmToSocketSubscription = {
  kind: "subscription";
  value: SubscriptionResponse;
};

export type SwarmToSocketError = {
  kind: "error";
  message: string;
};

export type SwarmToSocketStartCli = {
  kind: "start-cli";
};

export type SwarmToSocketStateUpdate = {
  kind: "state-update";
  value: any; // TODO
};

export type SwarmToSocketMessage =
  | SwarmToSocketLog
  | SwarmToSocketRpcResponse
  | SwarmToSocketSubscription
  | SwarmToSocketError
  | SwarmToSocketStartCli
  | SwarmToSocketStateUpdate;

function isLogMessage(message: string): boolean {
  return message.startsWith("[");
}

function isRpcResponse(message: string): boolean {
  return message.startsWith("R: ");
}

function isSubscription(message: string): boolean {
  return message.startsWith("S: ");
}

function isError(message: string): boolean {
  return message.trim().startsWith("^");
}

function isStartCli(message: string): boolean {
  return message.includes("@@@ ftSwarmOS CLI started");
}

function isStateUpdate(message: string): boolean {
  return message.startsWith("{");
}

export function parseSwarmToSocketMessage(
  message: string,
): Result<SwarmToSocketMessage, SwarmToSocketMessageParseError> {
  if (isStartCli(message)) {
    return Result.ok({ kind: "start-cli" });
  } else if (isLogMessage(message)) {
    return Result.ok({ kind: "log", message });
  } else if (isRpcResponse(message)) {
    return Result.ok({
      kind: "rpc-response",
      param: parseRpcReturnParam(message.substring(3)),
    });
  } else if (isSubscription(message)) {
    return parseSubscriptionResponse(message.substring(3)).map((r) => ({
      kind: "subscription",
      value: r,
    }));
  } else if (isError(message)) {
    return Result.ok({ kind: "error", message });
  } else if (isStateUpdate(message)) {
    return jsonify<any>(message)
      .map<{ kind: "state-update"; value: any }>((value) => ({
        kind: "state-update",
        value,
      }))
      .mapErr<SwarmToSocketMessageParseError>(
        (_: any) => SwarmToSocketMessageParseError.STATE_UPDATE_ERROR,
      );
  } else {
    return Result.err(SwarmToSocketMessageParseError.UNKNOWN);
  }
}
