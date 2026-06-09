import {createSignal, type Accessor, createEffect} from "solid-js";
import type {
    ApiController,
    ApiGetSwarmResponse,
} from "../apiTypes";
import logger from "../../util/logger";
import {
    createStore,
    reconcile,
    type SetStoreFunction,
    type Store,
} from "solid-js/store";
import {OptimisticRegistry, type RegistryKey} from "./optimisticRegistry.ts";
import type {OptimisticStore, OptimisticStoreOptions} from "./optimistic.ts";

export type SequencedDatum<T> = {
    seq?: number;
    data: T;
}

export type Sequence = number | undefined;

export class RootObjectModel {
    public readonly localName: Accessor<String>;
    public readonly isKelda: Accessor<boolean>;
    public readonly lastSequence: Accessor<number>;

    // Stored by serial number
    public readonly controllers: Store<Record<number, ApiController>>;

    private readonly _update: (toProcess: ApiGetSwarmResponse) => void;

    private readonly optimisticRegistry = new OptimisticRegistry();

    private readonly needsSaveSignal = createSignal(false);

    public get needsSave() {
        return this.needsSaveSignal[0]();
    }

    public markSaved() {
        this.needsSaveSignal[1](false);
    }

    constructor() {
        logger.info("RootObjectModel created");
        const [controllers, setControllers] = createStore<
            Record<number, ApiController>
        >({});
        this.controllers = controllers;

        const [localName, setLocalName] = createSignal<string>("");
        this.localName = localName;

        const [isKelda, setIsKelda] = createSignal<boolean>(false);
        this.isKelda = isKelda;

        const [lastSequence, setLastSequence] = createSignal<number>(0);
        this.lastSequence = lastSequence;

        this._update = (toProcess: ApiGetSwarmResponse) => {
            setLocalName(toProcess.name);
            setIsKelda(toProcess.kelda == 1);
            setLastSequence(toProcess.sync);
            this.mergeControllers(toProcess.controllers, setControllers);
        };
    }

    public update(toProcess: ApiGetSwarmResponse) {
        this._update(toProcess);
    }

    private mergeControllers(
        newControllers: ApiController[],
        setControllers: SetStoreFunction<Record<number, ApiController>>,
    ) {
        const newControllersRecord = newControllers.reduce(
            (acc, controller) => {
                acc[parseInt(controller.serialNumber)] = controller;
                return acc;
            },
            {} as Record<number, ApiController>,
        );

        setControllers(reconcile(newControllersRecord));
    }

    useController(serial: number) {
        return () => this.controllers[serial];
    }

    useBoundStore<T>(key: RegistryKey, source: () => SequencedDatum<T>, mutator: (val: T) => Promise<Sequence>, options?: OptimisticStoreOptions): OptimisticStore<T> {
        let sourceVal = source();
        const [lastSeenSeq, setLastSeenSeq] = createSignal(sourceVal.seq ?? 0);
        const store = this.optimisticRegistry.useBoundStore(
            key,
            sourceVal.data,
            async (v) => {
                this.needsSaveSignal[1](true);
                let seq = await mutator(v)
                if (!seq) return
                setLastSeenSeq(seq)
            },
            options
        );
        let storeValue = store[2];
        createEffect(() => {
            // sequences are counting up per transaction & mod 255. Assume that when we got a value more than 10 less
            // of our last seen sequence, we just witnessed an overflow.
            // also allow if the sequence is exactly the same as last seen, as we might get multiple updates with the same sequence
            let updatedSourceValue = source();
            if (updatedSourceValue.seq && (updatedSourceValue.seq >= lastSeenSeq() || updatedSourceValue.seq < lastSeenSeq() - 10)) {
                if (updatedSourceValue.seq < lastSeenSeq() - 10) {
                    logger.debug(`Sequence overflow detected or forced update. lastSeenSeq: ${lastSeenSeq()}, updatedSourceValue.seq: ${updatedSourceValue.seq}`)
                }
                setLastSeenSeq(updatedSourceValue.seq)
                storeValue(updatedSourceValue.data)
            } else if (!updatedSourceValue.seq) {
                storeValue(updatedSourceValue.data)
            }
        });
        return store;
    }
}
