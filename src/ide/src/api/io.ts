import type { ApiGeneralIoType, ApiOutputIoType } from "./apiTypes";
import { getIoIcon, type IconComponent } from "./icons";

export abstract class Io<T extends ApiGeneralIoType> {
  name: string;
  typeid: number;
  icon: IconComponent;
  active: boolean;

  constructor(object: T) {
    this.name = object.name;
    this.typeid = object.type;
    this.active = object.active;
    this.icon = getIoIcon(object.icon);
  }
}

export abstract class MotorIo extends Io<ApiOutputIoType> {
  constructor(object: ApiOutputIoType) {
    super(object);
  }
}
