const fs = require('fs');
const zlib = require('zlib');

const html = `
<!DOCTYPE html>
<html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>React with esbuild</title>
<style>${fs.readFileSync('dist/bundle.css', 'utf8')}</style>
</head>
<body>
</body>
<script>${fs.readFileSync('dist/bundle.js', 'utf8')}</script>
</html>
`;

const gzip = zlib.gzipSync(html, { level: 9 });
fs.writeFileSync('dist/index.html', html);
fs.writeFileSync('dist/index.html.gz', gzip);


const indexhtmlgz = fs.readFileSync('dist/index.html.gz');
const indexhtmlgzlen = indexhtmlgz.length;

fs.writeFileSync('dist/index.html.gz.h', `#define index_html_gz_len ${indexhtmlgzlen}\n`);
fs.appendFileSync('dist/index.html.gz.h', 'static const char index_html_gz[] PROGMEM = {\n');
fs.appendFileSync('dist/index.html.gz.h', indexhtmlgz.map(byte => byte.toString()).join(','));
fs.appendFileSync('dist/index.html.gz.h', '\n};');

console.log(`Finished compiling. Final output is ${indexhtmlgzlen/1024} KB`);