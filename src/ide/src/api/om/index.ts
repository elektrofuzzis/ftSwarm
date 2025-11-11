import { createSignal, type Accessor } from "solid-js";
import type { ApiGetSwarmResponse } from "../apiTypes";
import logger from "../../util/logger";

export class RootObjectModel {
  public readonly localName: Accessor<String>;
  public readonly isKelda: Accessor<boolean>;
  public readonly controllers: Accessor<any[]>;

  private _update: (toProcess: ApiGetSwarmResponse) => void;

  constructor() {
    logger.info("RootObjectModel created");
    const [controllers, setControllers] = createSignal<any[]>([]);
    this.controllers = controllers;

    const [localName, setLocalName] = createSignal<string>("");
    this.localName = localName;

    const [isKelda, setIsKelda] = createSignal<boolean>(false);
    this.isKelda = isKelda;

    this._update = (toProcess: ApiGetSwarmResponse) => {
      setLocalName(toProcess.name);
      setIsKelda(toProcess.kelda == 1);
      setControllers(toProcess.controllers);
    };
  }

  public update(toProcess: ApiGetSwarmResponse) {
    this._update(toProcess);
  }
}
