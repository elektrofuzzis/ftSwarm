import { createSignal, onCleanup } from "solid-js";

type AsyncMutator<T> = (newValue: T) => Promise<void>;

type OptimisticStoreOptions = {
  gracePeriod?: number; // default 300ms
  editingGracePeriod?: number; // default 0ms (suppresses after editing ends)
  mutationIntervalMs?: number; // default 500ms
  onError?: (error: unknown) => void;
};

type OptimisticStore<T> = [
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
 * - Updates UI immediately on mutate (optimisticValue).
 * - Throttles server mutation calls (min interval configurable).
 * - Ensures the last value is always sent (queues latest during throttle).
 * - Suppresses socket updates while mutating + grace period after last mutation.
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
    gracePeriod = 300,
    editingGracePeriod = 0,
    mutationIntervalMs = 500,
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
            // Send immediately
            dispatchMutation(qv); // Recursive dispatch for newest value
            return; // Defer grace scheduling until final dispatch chain ends
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

        // Schedule grace period only if this is the last finished mutation with no further sends pending
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
      // Safe to dispatch immediately (not throttled & not mutating OR we allow overlapping — here we prioritize interval)
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
