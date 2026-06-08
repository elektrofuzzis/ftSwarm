import logger from "../util/logger";
import { Result } from "../util/result";
import { ftSwarmReplacements } from "./generated/genApiTranslations";

/*
 * Decompress the blob using the provided decompression algorithm
 *
 * 1. Go over every byte. If it is a key in ftSwarmReplacements, append the corresponding value to strbuf.
 * 2. If it is not a key in ftSwarmReplacements, append the byte as a character to strbuf.
 */
export async function decompressBlob(blob: Blob | string): Promise<string> {
  let strbuf = "";

  if (typeof blob === "string") return blob;

  const arrayBuffer = await blob.arrayBuffer();
  const uint8Array = new Uint8Array(arrayBuffer);

  for (let i = 0; i < uint8Array.length; i++) {
    const byte = uint8Array[i];
    if (byte in ftSwarmReplacements) {
      strbuf += ftSwarmReplacements[byte];
    } else {
      strbuf += String.fromCharCode(byte);
    }
  }

  return strbuf;
}

export function jsonify<T>(jsonString: string): Result<T, Error> {
  try {
    return Result.ok(JSON.parse(jsonString) as T);
  } catch (error) {
    logger.error("Failed to parse JSON:", error, { jsonString });
    return Result.err(error as Error);
  }
}
