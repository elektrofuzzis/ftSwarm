export type Unit = null;
export const Unit: Unit = null;

export class Result<T, E> {
  private value: T | E;
  private isError: boolean;

  constructor(value: T | E, isError: boolean) {
    this.value = value;
    this.isError = isError;
  }

  static ok<T, E>(value: T): Result<T, E> {
    return new Result<T, E>(value, false);
  }

  static err<T, E>(error: E): Result<T, E> {
    return new Result<T, E>(error, true);
  }

  static try<T, E>(fn: () => T, error: E): Result<T, E> {
    try {
      return Result.ok(fn());
    } catch (_) {
      return Result.err(error);
    }
  }

  isOk(): boolean {
    return !this.isError;
  }

  isErr(): boolean {
    return this.isError;
  }

  unwrap(): T {
    if (this.isError) {
      throw new Error(`Unwrapped error: ${this.value}`);
    }
    return this.value as T;
  }

  unwrapOr<V>(defaultValue: V): T | V {
    if (this.isError) {
      return defaultValue;
    }

    return this.value as T;
  }

  unwrapErr(): E {
    if (!this.isError) {
      throw new Error(`Unwrapped value: ${this.value}`);
    }
    return this.value as E;
  }

  unwrapErrOr<V>(defaultValue: V): E | V {
    if (!this.isError) {
      return defaultValue;
    }

    return this.value as E;
  }

  map<U>(fn: (value: T) => U): Result<U, E> {
    if (this.isError) {
      return Result.err(this.value as E);
    }
    return Result.ok(fn(this.value as T));
  }

  mapErr<F>(fn: (error: E) => F): Result<T, F> {
    if (!this.isError) {
      return Result.ok(this.value as T);
    }
    return Result.err(fn(this.value as E));
  }

  match<U, F>(okFn: (value: T) => U, errFn: (error: E) => F): U | F {
    if (this.isError) {
      return errFn(this.value as E);
    }
    return okFn(this.value as T);
  }
}
