const fs = require("fs");
const zlib = require("zlib");
const crypto = require("crypto");
const path = require("path");

const html = `
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>MiLight Hub</title>
<link href="data:image/x-icon;base64,AAABAAEAEBAAAAEACABoBQAAFgAAACgAAAAQAAAAIAAAAAEACAAAAAAAAAEAAAAAAAAAAAAAAAEAAAAAAAAAAAAA/38NABD+CwD/jQsA9pUOABRu/wD/HQwA+gPrAP4MbAAZ8f8ACxb9AP4GkwCW/AoADYj+AP59CQD7FAkAC//nAJYK/QAQ/w4Acgv/AP71CwCSDP8A+AkYAP7rBAAN/+0A/xEcAP1rCwAM/okADf+HAI/7CgAL/5sADP95AA0U/wD4/gsADXr+ABAO/gDlC/8A+v0NAA3/EwB4Ef8A8wr7AP0IeAAN7P4Abf8LAAuJ/wCVDv8A+hcGACH+DgBz/wsA+g7/ABT+ZQAMCv0Aagn/AJT/DQD+EPsADP7sAOf/DAD/Dx0A/g2AAB4N/gB0/gwADZz/ABLu/wAM/BYA+wqMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAkYAAAAAAAAAAAAAAUsAAA+NwAAGzIAAAAAAAAAIj0AKhAAHh8AAAAAAAoAAAANAAAAABwAAAA/AAAzIAAAAAAAAAAAAAAmAgAAADsjAAAAAAAAAAASLwAAAAAAAAAAAAAAAAAAAAAAABMnNAAAAAAAAAAAAAArMDwVLREAAAAAAAAAAAAADDUdAAAAAAAAAAAAAAAAAAAAAAAAJDEAAAAAAAAAACU4AAAAKDYAAAAAAAAAAAAAFCEAAAcAAAA6AAAAAAEAAAAXAAAAAABACAAWLgAaAwAAAAAAAAALKQAAOQYAAA4EAAAAAAAAAAAAABkPAAAAAAAAAP5/AADmZwAA8k8AALvdAACf+QAAz/MAAP//AAAf+AAAH/gAAP//AADP8wAAn/kAALvdAADyTwAA5mcAAP5/AAA=" rel="icon" type="image/x-icon" />
<style>
  #loading {
    position: fixed;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    font-size: 24px;
    font-family: Arial, sans-serif;
  }
</style>
<script>${fs.readFileSync("src/pageload.js")}</script>
</head>
<body>
<div id="page">
<div id="loading">Loading...</div>
</div>
</body>
</html>
`;

function generateGzippedHeader(inputFile, variableName) {
  const content = fs.readFileSync(inputFile);
  const gzipped = zlib.gzipSync(content, { level: 9 });
  const gzippedLength = gzipped.length;

  // Calculate hash of the content
  const hash = crypto
    .createHash("md5")
    .update(content)
    .digest("hex")
    .slice(0, 8);

  // Append hash to the output filename
  const outputFile = `${inputFile}.gz`;
  const [baseName, extension] = outputFile.split(".");
  const serverFilename = `${baseName}.${hash}.${extension}`;

  fs.writeFileSync(outputFile, gzipped);
  fs.writeFileSync(
    `${outputFile}.h`,
    `#define ${variableName}_gz_len ${gzippedLength}\n` +
      `static const char ${variableName}_filename[] = "/${serverFilename}";\n` +
      `static const char ${variableName}_gz[] PROGMEM = {` +
      gzipped.map((byte) => byte.toString()).join(",") +
      "};\n"
  );

  console.log(`Generated ${outputFile}.h (${gzippedLength / 1024} KB)`);

  const variables = {};
  variables[`${variableName}:filename`] = outputFile;
  variables[`${variableName}:filename_with_hash`] = serverFilename;
  variables[`${variableName}:size`] = gzippedLength;

  return variables;
}

// Generate gzipped files and headers
const variables = {
  ...generateGzippedHeader("dist/bundle.css", "bundle_css"),
  ...generateGzippedHeader("dist/bundle.js", "bundle_js"),
};

let outputHtml = html;
Object.entries(variables).forEach(([key, value]) => {
    outputHtml = outputHtml.replace(`{{${key}}}`, value);
})

fs.writeFileSync(
  "dist/index.html",
  outputHtml
);
generateGzippedHeader("dist/index.html", "index_html");

console.log("Finished compiling all files.");
