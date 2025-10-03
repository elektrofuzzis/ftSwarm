import { Result } from "../../../util/result";
import { SwarmToSocketMessageParseError } from "./error";
import { parseRpcReturnParam, type RpcReturnParam } from "./rpc";

export type SubscriptionResponse = {
  portName: string;
  value: RpcReturnParam;
};

export function parseSubscriptionResponse(
  response: string,
): Result<SubscriptionResponse, SwarmToSocketMessageParseError> {
  return Result.try(() => {
    const [portName, value] = response.split(" ");

    return {
      portName,
      value: parseRpcReturnParam(value),
    };
  }, SwarmToSocketMessageParseError.SUB_PARSE);
}
