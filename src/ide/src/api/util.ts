import type {ApiController, ApiGeneralIoType} from "./apiTypes.ts";
import type {SwarmToSocketRpcResponse} from "./transport/swarm2socket";
import type {Sequence} from "./om";

export function apiNameOf(io: ApiGeneralIoType, controller: ApiController): string {
    if (io.alias) return io.alias;
    return `${controller.name}.${io.name}`;
}

export function rpcResponseToSeq(resp: SwarmToSocketRpcResponse | null | undefined): Sequence | undefined {
    if (!resp) return undefined;
    if (Object.keys(resp.param).includes("seq")) {
        const seqParam = resp.param as { seq: Sequence };
        return seqParam.seq;
    }
    return undefined;
}