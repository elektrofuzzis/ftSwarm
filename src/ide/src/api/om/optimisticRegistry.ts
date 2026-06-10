import type {ApiController, ApiGeneralIoType} from "../apiTypes.ts";
import {createRoot, onCleanup} from "solid-js";
import {createOptimisticStore, type OptimisticStore, type OptimisticStoreOptions} from "./optimistic.ts";

export type RegistryKey = string;

export type OptimisticRegistryEntry = {
    store: OptimisticStore<any>,
    dispose: () => void,
    refs: number
}

export function registryKeyOf<T>(controller: ApiController, io: ApiGeneralIoType, identifier: T): RegistryKey {
    return `${controller.serialNumber}:${io.name}:${String(identifier)}`;
}

export class OptimisticRegistry {
    private stores = new Map<RegistryKey, OptimisticRegistryEntry>();

    private useStore<T>(key: RegistryKey, initial: T, mutator: (val: T) => Promise<void>, options?: OptimisticStoreOptions) {
        if (!this.stores.has(key)) {
            createRoot((dispose) => {
                const store = createOptimisticStore(initial, mutator, options);
                this.stores.set(key, {store, dispose, refs: 0});
            })
        }

        const entry = this.stores.get(key)!;
        entry.refs++;
        return entry.store;
    }

    private disposeStore(key: RegistryKey) {
        const entry = this.stores.get(key)!;
        entry.refs--;
        if (entry.refs === 0) {
            entry.dispose();
            this.stores.delete(key);
        }
    }

    useBoundStore<T>(key: RegistryKey, initial: T, mutator: (val: T) => Promise<void>, options?: OptimisticStoreOptions): OptimisticStore<T> {
        const store = this.useStore(key, initial, mutator, options);
        onCleanup(() => this.disposeStore(key));
        return store;
    }
}