import { readFile, writeFile } from "fs/promises";

// Captures the entire enum block.
// Group 1: The content between { ... }
// Group 2: The enum's name (with _t stripped)
const ENUM_ISOLATION_REGEX =
  /typedef\s+enum\s*{\s+([\s\S]*?)\s+}\s*(\w+?)(?:_t)?\s*;/gm;

// Captures a single enum member line.
// Group 1: The constant's name (e.g., "MEMBER_A")
// Group 2: The constant's value (e.g., "-1" or "0xFF", optional)
const ENUM_CONSTANT_REGEX =
  /\s*(\w+)\s*(?:\=\s*(-?(?:0x)?[a-fA-F0-9]+))?\s*(?:(?:\/\/[^\n]*|\/\*[\s\S]*?\*))?\s*,?\s*(?:(?:\/\/[^\n]*|\/\*[\s\S]*?\*))?/gm;

class CEnumeration {
  constructor(
    public readonly name: string,
    public readonly values: Record<string, number>,
  ) {}

  toString(): string {
    return `export enum ${this.name} {\n${Object.entries(this.values)
      .map(([key, value]) => `  ${key} = ${value}`)
      .join(",\n")}\n};`;
  }
}

const includePath = process.cwd() + "/ftSwarm/src/ftswarm-core/include/SwOS.h";
const content = (await readFile(includePath)).toString();

let enums: CEnumeration[] = [];
const enumMatches = content.matchAll(ENUM_ISOLATION_REGEX) || [];

for (const match of enumMatches) {
  const enumName = match[2];
  const enumValueString = match[1];
  const constantMatches = enumValueString.matchAll(ENUM_CONSTANT_REGEX) || [];

  let lastValue = -1;
  let values: Record<string, number> = {};

  for (const constantMatch of constantMatches) {
    const constantName = constantMatch[1];
    const constantValue = constantMatch[2]
      ? Number(constantMatch[2])
      : lastValue + 1;
    values[constantName] = constantValue;
    lastValue = constantValue;
  }

  enums.push(new CEnumeration(enumName, values));
}

const targetFileContent =
  "// GENERATED ENUMERATIONS FROM ftSwarm/src/ftswarm-core/include/SwOS.h\n" +
  enums.map((enumObj) => enumObj.toString()).join("\n\n");

await writeFile("src/api/api_enums.ts", targetFileContent);
