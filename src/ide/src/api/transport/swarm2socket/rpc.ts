export type SequencedTrue = { value: true; seq: number };
export type RpcReturnParam = SequencedTrue | number | string;

export function parseRpcReturnParam(message: string): RpcReturnParam {
  if (message.endsWith("ok"))
    return {
      value: true,
      seq: parseInt(message.split(" ")[0]),
    };

  const intLike = /^[+-]?\d+$/.test(message);
  if (intLike) {
    const n = Number(message);
    if (!Number.isNaN(n)) return n;
  }

  return message;
}
