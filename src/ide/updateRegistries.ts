import { readFile, writeFile } from "fs/promises";

// Captures the entire enum block.
// Group 1: The content between { ... }
// Group 2: The enum's name (with _t stripped)
const ENUM_ISOLATION_REGEX =
  /typedef\s+enum\s+(?:\w+\s+)?{\s*([\s\S]*?)\s*}\s*(\w+?)(?:_t)?\s*;/gm;

// Captures a single enum member line.
// Group 1: The constant's name (e.g., "MEMBER_A")
// Group 2: The constant's value (e.g., "-1" or "0xFF", optional)
const ENUM_CONSTANT_REGEX =
  /\s*(\w+)\s*(?:\=\s*(-?(?:0x)?[a-fA-F0-9]+|\w+))?\s*,?/g;

// Captures the full SLITERAL block
// Group 1: The content between { ... }
const TRANSLATION_LITERAL_SEPARATOR_REGEX =
  /const\s+char\s+SLITERAL.*{([\s\S]*?)};/gm;

// Captures a single translation literal element.
// Group 1: The literal string (e.g., "Hello", including quotes)
const TRANSLATION_LITERAL_ELEMENT_REGEX = /(".*"),?/gm;

class CEnumeration {
  constructor(
    public readonly name: string,
    public readonly values: Record<string, number>,
  ) {}

  toString(): string {
    return `export const enum ${this.name} {\n${Object.entries(this.values)
        .filter(([_, value]) => !isNaN(value))
      .map(([key, value]) => `  ${key} = ${value}`)
      .join(",\n")}\n}`;
  }
}

async function generateEnums() {
  const includePath =
    process.cwd() + "/ftSwarm/src/ftswarm-core/include/SwOS.h";
  const content = (await readFile(includePath)).toString();

  let enums: CEnumeration[] = [];
  const enumMatches = content.matchAll(ENUM_ISOLATION_REGEX) || [];

  for (const match of enumMatches) {
    const enumName = match[2];
    if (enumName == "SwOSLabel" || enumName == "FtSwarmController") continue;
    
    // Remove comments within the enum block
    const enumValueString = match[1].replace(/\/\/.*$/gm, "").replace(/\/\*[\s\S]*?\*\//g, "");
    const constantMatches = enumValueString.matchAll(ENUM_CONSTANT_REGEX) || [];

    let lastValue = -1;
    let values: Record<string, number> = {};

    for (const constantMatch of constantMatches) {
      const constantName = constantMatch[1];
      if (constantName.endsWith("_MAX") || 
          constantName.endsWith("MAXIOTYPE") || 
          constantName.endsWith("MAXSTATE") || 
          constantName.endsWith("MAXVERSION") || 
          constantName.endsWith("MAXMOTION") || 
          constantName.endsWith("MAXTRIGGER") || 
          constantName.endsWith("MAXOPERATOR") || 
          constantName.endsWith("MAXOPERAND") || 
          constantName.endsWith("MAXSCREEN") ||
          constantName === "FTSWARM_MAXCONTROLLERTYPE") continue;
          
      let constantValue: number;
      if (constantMatch[2]) {
        const valStr = constantMatch[2].trim();
        if (valStr.startsWith("0x")) {
          constantValue = parseInt(valStr, 16);
        } else if (/^-?\d+$/.test(valStr)) {
          constantValue = parseInt(valStr, 10);
        } else {
          // It's a reference to another constant
          constantValue = values[valStr] ?? (lastValue + 1);
        }
      } else {
        constantValue = lastValue + 1;
      }
      
      values[constantName] = constantValue;
      lastValue = constantValue;
    }

    if (Object.keys(values).length > 0) {
      enums.push(new CEnumeration(enumName, values));
    }
  }

  const targetFileContent =
    "// GENERATED ENUMERATIONS FROM ftSwarm/src/ftswarm-core/include/SwOS.h\n" +
    enums.map((enumObj) => enumObj.toString()).join("\n\n");

  await writeFile("src/api/generated/genApiEnums.ts", targetFileContent);
}

async function generateTranslations() {
  const includePath =
    process.cwd() + "/ftSwarm/src/ftswarm-core/src/serialize.cpp";
  const content = (await readFile(includePath)).toString();
  const [literalBlockMatch] = content.matchAll(
    TRANSLATION_LITERAL_SEPARATOR_REGEX,
  );
  const literalBlock = literalBlockMatch ? literalBlockMatch[1] : "";

  let translations: string[] = [];

  const translationMatches =
    literalBlock.matchAll(TRANSLATION_LITERAL_ELEMENT_REGEX) || [];

  let index = 0;
  for (const match of translationMatches) {
    const translationValue = match[1] + ":";
    const id = index++ + 128;
    translations.push(`  ${id}: '${translationValue}'`);
  }

  const targetFileContent =
    "// GENERATED TRANSLATIONS FROM ftSwarm/src/ftswarm-core/src/serialize.cpp\n" +
    `export const ftSwarmReplacements: Record<string, string> = {\n${translations.join(",\n")}\n};`;

  await writeFile("src/api/generated/genApiTranslations.ts", targetFileContent);
}

async function generateIoMappings() {
  const includePath =
    process.cwd() + "/ftSwarm/src/ftswarm-core/include/SwOS.h";
  const content = (await readFile(includePath)).toString();

  const classMatch = content.match(/const\s+SwOSIOClass_t\s+SWOSIOCLASS\[SWOSIO_MAXIOTYPE\s*\]\s*=\s*{([\s\S]*?)}\s*;/);
  const showMatch = content.match(/const\s+bool\s+SHOWIOINAPI\[SWOSIO_MAXIOTYPE\]\s*=\s*{([\s\S]*?)}\s*;/);
  const typeNamesMatch = content.match(/const\s+char\s+SWOSIOTYPE\[SWOSIO_MAXIOTYPE\]\[20\]\s*=\s*{([\s\S]*?)}\s*;/);
  const enumTypeMatch = content.match(/typedef\s+enum\s*{\s*SWOSIO_UNDEF\s*=\s*-1,([\s\S]*?)SWOSIO_MAXIOTYPE\s*}\s*SwOSIOType_t\s*;/);

  if (!classMatch || !showMatch || !typeNamesMatch || !enumTypeMatch) {
    console.error("Could not find IO mapping arrays or enum in SwOS.h");
    return;
  }

  const parseArray = (str: string) => {
    // Remove C++ comments
    const cleanStr = str.replace(/\/\/.*$/gm, "").replace(/\/\*[\s\S]*?\*\//g, "");
    const results = [];
    let current = "";
    let depth = 0;
    let inQuotes = false;
    
    for (let i = 0; i < cleanStr.length; i++) {
      const char = cleanStr[i];
      if (char === '"' && cleanStr[i-1] !== '\\') inQuotes = !inQuotes;
      if (!inQuotes) {
        if (char === "{") depth++;
        else if (char === "}") depth--;
        else if (char === "," && depth === 0) {
          results.push(current.trim());
          current = "";
          continue;
        }
      }
      current += char;
    }
    if (current.trim()) results.push(current.trim());
    return results.filter(r => r.length > 0);
  };

  const classes = parseArray(classMatch[1]);
  const shows = parseArray(showMatch[1]);
  const names = parseArray(typeNamesMatch[1]).map(n => n.replace(/"/g, '').trim());
  const enumMembers = parseArray(enumTypeMatch[1]).map(m => m.split('=')[0].trim());

  // Limit to SWOSIO_MAXIOTYPE (44)
  const maxIOType = 44;
  const ioMappings = [];
  for (let i = 0; i < maxIOType; i++) {
    const rawClass = classes[i] || "SWOSIOCLASS_INPUT";
    const cleanClass = rawClass.replace("SWOSIOCLASS_", "");
    
    ioMappings.push({
      name: names[i],
      enumMember: enumMembers[i],
      ioClass: cleanClass,
      showInApi: shows[i] === 'true'
    });
  }

  const targetFileContent =
    "// GENERATED IO MAPPINGS FROM ftSwarm/src/ftswarm-core/include/SwOS.h\n" +
    "import { SwOSIOClass, SwOSIOType } from \"./genApiEnums\";\n\n" +
    "export interface IoTypeInfo {\n" +
    "  type: SwOSIOType;\n" +
    "  name: string;\n" +
    "  ioClass: SwOSIOClass;\n" +
    "  showInApi: boolean;\n" +
    "}\n\n" +
    "export const ioTypeInfos: Record<number, IoTypeInfo> = {\n" +
    ioMappings.map((m) => `  [SwOSIOType.${m.enumMember}]: { type: SwOSIOType.${m.enumMember}, name: \"${m.name}\", ioClass: SwOSIOClass.SWOSIOCLASS_${m.ioClass}, showInApi: ${m.showInApi} }`).join(",\n") +
    "\n};\n";

  await writeFile("src/api/generated/genIoMappings.ts", targetFileContent);
}

await generateEnums();
await generateTranslations();
await generateIoMappings();
