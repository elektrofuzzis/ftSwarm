export type RpcReturnParam = true | number | string;

export function parseRpcReturnParam(message: string): RpcReturnParam {
  if (message === "ok") return true;

  const intLike = /^[+-]?\d+$/.test(message);
  if (intLike) {
    const n = Number(message);
    if (!Number.isNaN(n)) return n;
  }

  return message;
}
