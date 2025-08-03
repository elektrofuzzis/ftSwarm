export type RpcReturnParam = true | number | string;

export function parseRpcReturnParam(message: string): RpcReturnParam {
  if (message === "ok") {
    return true;
  }

  try {
    return parseInt(message);
  } catch (error) {}

  return message;
}
