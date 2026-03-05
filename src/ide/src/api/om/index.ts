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

export class RootObjectModel {
    public readonly localName: Accessor<String>;
    public readonly isKelda: Accessor<boolean>;

    // Stored by serial number
    public readonly controllers: Store<Record<number, ApiController>>;

    private readonly _update: (toProcess: ApiGetSwarmResponse) => void;

    private readonly optimisticRegistry = new OptimisticRegistry();

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

        this._update = (toProcess: ApiGetSwarmResponse) => {
            setLocalName(toProcess.name);
            setIsKelda(toProcess.kelda == 1);
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

    useBoundStore<T>(key: RegistryKey, source: () => T, mutator: (val: T) => Promise<void>, options?: OptimisticStoreOptions): OptimisticStore<T> {
        const store = this.optimisticRegistry.useBoundStore(key, source(), mutator, options);
        createEffect(() => store[2](source()));
        return store;
    }
}
