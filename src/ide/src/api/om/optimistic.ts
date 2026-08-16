import { createSignal, onCleanup } from "solid-js";
import logger from "../../util/logger";

export type AsyncMutator<T> = (newValue: T) => Promise<void>;

export type OptimisticStoreOptions = {
  /**
   * Time in milliseconds to suppress source updates after a mutation completes.
   * Prevents UI rubber-banding from delayed server echoes.
   * Default: 1200
   */
  gracePeriod?: number;

  /**
   * Time in milliseconds to suppress source updates after explicit editing ends.
   * Default: 1800
   */
  editingGracePeriod?: number;

  /**
   * Minimum time in milliseconds between dispatched mutations to the server.
   * Default: 750
   */
  mutationIntervalMs?: number;

  /**
   * Callback invoked if the async mutator throws an error.
   */
  onError?: (error: unknown) => void;
};

export type OptimisticStore<T> = [
  value: () => T,
  mutate: (newValue: T) => void,
  setSource: (sourceValue: T) => void,
  accessories: {
    isMutating: () => boolean;
    isEditing: () => boolean;
    sourceValue: () => T;
    beginEditing: () => void;
    endEditing: () => void;
    setEditing: (editing: boolean) => void;
    isThrottled: () => boolean;
    queuedValue: () => T | undefined;
  },
];

/**
 * Optimistic store that:
 * - Updates UI immediately on mutating (optimisticValue).
 * - Throttles server mutation calls (min interval configurable).
 * - Ensures the last value is always sent (queues latest during throttle).
 * - Suppresses socket updates while mutating + a grace period after last mutation.
 * - Supports explicit editing suppression (pointer down/up) even without mutation.
 *
 * Concurrency / correctness features:
 * - Overlapping mutation completions handled by pending counter + monotonic ids.
 * - Separate grace timers for mutation completion and editing end.
 * - Throttling layer guarantees at most one server send per mutationIntervalMs.
 */
export function createOptimisticStore<T>(
  initialValue: T,
  asyncMutator: AsyncMutator<T>,
  options: OptimisticStoreOptions = {},
): OptimisticStore<T> {
  const {
    gracePeriod = 1200,
    editingGracePeriod = 1800,
    mutationIntervalMs = 750,
    onError,
  } = options;

  // Signals
  const [sourceOfTruth, setSourceOfTruth] = createSignal(initialValue);
  const [optimisticValue, setOptimisticValue] = createSignal(initialValue);
  const [isMutating, setIsMutating] = createSignal(false);
  const [isEditing, setIsEditing] = createSignal(false);
  const [queuedValueSig, setQueuedValueSig] = createSignal<T | undefined>(
    undefined,
  );

  // Internal timers / counters
  let mutationGraceTimer: number | undefined;
  let editingGraceTimer: number | undefined;
  let dispatchTimer: number | undefined;
  let pendingMutations = 0;
  let currentMutationId = 0;
  let lastDispatchTime = 0;

  onCleanup(() => {
    if (mutationGraceTimer) clearTimeout(mutationGraceTimer);
    if (editingGraceTimer) clearTimeout(editingGraceTimer);
    if (dispatchTimer) clearTimeout(dispatchTimer);
  });

  // Editing controls
  const beginEditing = () => {
    if (editingGraceTimer) {
      clearTimeout(editingGraceTimer);
      editingGraceTimer = undefined;
    }
    if (!isEditing()) setIsEditing(true);
  };

  const endEditing = () => {
    if (editingGraceTimer) {
      clearTimeout(editingGraceTimer);
      editingGraceTimer = undefined;
    }
    if (editingGracePeriod > 0) {
      editingGraceTimer = window.setTimeout(() => {
        setIsEditing(false);
        if (!isMutating() && sourceOfTruth() !== optimisticValue()) {
          setOptimisticValue(() => sourceOfTruth());
        }
        editingGraceTimer = undefined;
      }, editingGracePeriod);
    } else {
      setIsEditing(false);
      if (!isMutating() && sourceOfTruth() !== optimisticValue()) {
        setOptimisticValue(() => sourceOfTruth());
      }
    }
  };

  const setEditing = (editing: boolean) => {
    if (editing) beginEditing();
    else endEditing();
  };

  // Throttling helpers
  const isThrottled = () => dispatchTimer !== undefined;

  const dispatchMutation = (value: T) => {
    // Begin mutation accounting
    pendingMutations++;
    const thisMutationId = ++currentMutationId;
    if (!isMutating()) setIsMutating(true);

    lastDispatchTime = performance.now();
    setQueuedValueSig(undefined); // Consumed queued value

    asyncMutator(value)
      .catch((err) => {
        onError?.(err);
        setOptimisticValue(() => sourceOfTruth());
      })
      .finally(() => {
        pendingMutations--;
        // If there is still a queued value (user changed during in-flight), decide whether to send it now or schedule
        const qv = queuedValueSig();
        if (qv !== undefined) {
          const elapsed = performance.now() - lastDispatchTime;
          if (elapsed >= mutationIntervalMs) {
            // Send it immediately
            dispatchMutation(qv); // Recursive dispatch for the newest value
            return; // Defer grace scheduling until the final dispatch chain ends
          }
          // Schedule send after remaining interval
          if (!dispatchTimer) {
            dispatchTimer = window.setTimeout(() => {
              dispatchTimer = undefined;
              const latest = queuedValueSig();
              if (latest !== undefined) dispatchMutation(latest);
            }, mutationIntervalMs - elapsed);
          }
        }

        // Schedule grace period only if this is the last finished mutation with no further sending pending
        if (
          pendingMutations === 0 &&
          thisMutationId === currentMutationId &&
          queuedValueSig() === undefined &&
          dispatchTimer === undefined
        ) {
          mutationGraceTimer = window.setTimeout(() => {
            setIsMutating(false);
            if (!isEditing() && sourceOfTruth() !== optimisticValue()) {
              setOptimisticValue(() => sourceOfTruth());
            }
            mutationGraceTimer = undefined;
          }, gracePeriod);
        }
      });
  };

  const mutate = (newValue: T) => {
    // Optimistic UI update immediately
    setOptimisticValue(() => newValue);

    // Update queued value
    setQueuedValueSig(() => newValue);

    // If a dispatch is already scheduled, just update the queued value
    if (dispatchTimer) return;

    const now = performance.now();
    const elapsed = now - lastDispatchTime;

    if (elapsed >= mutationIntervalMs && !isMutating()) {
      // Safe to dispatch immediately (not throttled and not mutating OR we allow overlapping — here we prioritize the interval)
      const qv = queuedValueSig();
      if (qv !== undefined) {
        dispatchMutation(qv);
      }
    } else if (elapsed >= mutationIntervalMs && isMutating()) {
      // We have an in-flight mutation; let its completion logic handle dispatching queued value.
      return;
    } else {
      // Need to wait remaining time
      dispatchTimer = window.setTimeout(
        () => {
          dispatchTimer = undefined;
          const qv = queuedValueSig();
          if (qv !== undefined) dispatchMutation(qv);
        },
        Math.max(0, mutationIntervalMs - elapsed),
      );
    }
  };

  const setSource = (newValueFromSocket: T) => {
    setSourceOfTruth(() => newValueFromSocket);
    if (!isMutating() && !isEditing()) {
      setOptimisticValue(() => newValueFromSocket);
    } else {
      logger.debug(
        `Optimistic update discarded. isMutating: ${isMutating()}, isEditing: ${isEditing()}. New value:`,
        newValueFromSocket,
      );
    }
  };

  return [
    optimisticValue,
    mutate,
    setSource,
    {
      isMutating,
      isEditing,
      sourceValue: sourceOfTruth,
      beginEditing,
      endEditing,
      setEditing,
      isThrottled,
      queuedValue: queuedValueSig,
    },
  ];
}
