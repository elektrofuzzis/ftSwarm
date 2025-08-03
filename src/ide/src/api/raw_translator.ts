import logger from "../util/logger";

const ftSwarmReplacements: Record<string, string> = {
  [String.fromCharCode(2)]: '"io":',
  [String.fromCharCode(3)]: '"kelda":',
  [String.fromCharCode(4)]: '"sensor":',
  [String.fromCharCode(5)]: '"actor":',
  [String.fromCharCode(6)]: '"trigger":',
  [String.fromCharCode(7)]: '"state":',
  [String.fromCharCode(8)]: '"speed":',
  [String.fromCharCode(9)]: '"highResolution":',
  [String.fromCharCode(10)]: '"offset":',
  [String.fromCharCode(11)]: '"position":',
  [String.fromCharCode(12)]: '"value":',
  [String.fromCharCode(13)]: '"valueLR":',
  [String.fromCharCode(14)]: '"valueFB":',
  [String.fromCharCode(15)]: '"name":',
  [String.fromCharCode(17)]: '"serialNumber":',
  [String.fromCharCode(18)]: '"type":',
  [String.fromCharCode(19)]: '"icon":',
  [String.fromCharCode(20)]: '"active":',
  [String.fromCharCode(21)]: '"brightness":',
  [String.fromCharCode(22)]: '"color":',
  [String.fromCharCode(23)]: '"quarternion":',
  [String.fromCharCode(24)]: '"acceleration":',
  [String.fromCharCode(25)]: '"url":',
  [String.fromCharCode(26)]: '"framesize":',
  [String.fromCharCode(27)]: '"quality":',
  [String.fromCharCode(28)]: '"contrast":',
  [String.fromCharCode(29)]: '"saturation":',
  [String.fromCharCode(30)]: '"h-Mirror":',
  [String.fromCharCode(31)]: '"v-Flip":',
};

function translateStringToObject<T>(
  jsonString: string,
  replacements: Record<string, string>,
): T | null {
  let processedString = jsonString;

  for (const key in replacements) {
    processedString = processedString.replace(key, replacements[key]);
  }

  try {
    return JSON.parse(processedString) as T;
  } catch (error) {
    logger.error("Failed to parse JSON:", error);
    return null;
  }
}

export function translateSwarmJson(jsonString: string) {
  return translateStringToObject(jsonString, ftSwarmReplacements);
}

export function translateRawJson(jsonString: string) {
  return translateStringToObject(jsonString, {});
}
