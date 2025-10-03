import { ErrorResolution, type CommunicationError } from "..";

export class SwarmToSocketMessageParseError implements CommunicationError {
  public static UNKNOWN = this.err("Unknown Message");
  public static SUB_PARSE = this.err("Invalid Subscription Format");

  private constructor(private readonly message: string) {}

  resolution(): ErrorResolution {
    return ErrorResolution.FAIL;
  }

  what(): string {
    return this.message;
  }

  private static err(message: string): SwarmToSocketMessageParseError {
    return new SwarmToSocketMessageParseError(message);
  }
}
