import { ErrorResolution, type CommunicationError } from "..";

export class SwarmToSocketMessageParseError implements CommunicationError {
  public static UNKNOWN = this.err("Unknown Message", ErrorResolution.FAIL);
  public static SUB_PARSE = this.err(
    "Invalid Subscription Format",
    ErrorResolution.FAIL,
  );
  public static STATE_UPDATE_ERROR = this.err(
    "Invalid State Update Format",
    ErrorResolution.FAIL,
  );

  private constructor(
    private readonly message: string,
    private readonly res: ErrorResolution,
  ) {}

  resolution(): ErrorResolution {
    return this.res;
  }

  what(): string {
    return this.message;
  }

  private static err(
    message: string,
    resolution: ErrorResolution,
  ): SwarmToSocketMessageParseError {
    return new SwarmToSocketMessageParseError(message, resolution);
  }
}
