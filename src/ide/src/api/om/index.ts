import { createSignal, type Accessor } from "solid-js";
import type { ApiController, ApiGetSwarmResponse } from "../apiTypes";
import logger from "../../util/logger";

export class RootObjectModel {
  public readonly localName: Accessor<String>;
  public readonly isKelda: Accessor<boolean>;
  public readonly controllers: Accessor<ApiController[]>;

  private _update: (toProcess: ApiGetSwarmResponse) => void;

  constructor() {
    logger.info("RootObjectModel created");
    const [controllers, setControllers] = createSignal<ApiController[]>([]);
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
    setControllers: (controllers: ApiController[]) => void,
  ) {
    const currentControllers = this.controllers();
    const toAdd = newControllers.filter(
      (controller) =>
        !currentControllers.some(
          (c) => c.serialNumber === controller.serialNumber,
        ),
    );
    const toRemove = currentControllers.filter(
      (controller) =>
        !newControllers.some((c) => c.serialNumber === controller.serialNumber),
    );
    const controllers = [...currentControllers, ...toAdd].filter(
      (controller) =>
        !toRemove.some((c) => c.serialNumber === controller.serialNumber),
    );
    setControllers(controllers);
  }
}
