# Web UI

This contains the web UI for espMH. The UI is written with the following stack:

* Typescript
* React
* Tailwind
* Shadcn/ui
* esbuild

It compiles everything into C++ header files which are copied to `dist` and included as a part of the platformio build process.

## Development

It _should_ be enough to install node v20+ and run:

```bash
npm run watch
```

This will continuously run the build as files change. When I develop, I typically also use the nodejs server:

```bash
cross-env PROXY_URL=http://hostname-of-your-hub node server.js
```

This does two things:

1. It proxies API requests to an actual ESP8266 running on your local network.
2. It serves the compiled files from `dist/compiled` so you don't have to recompile when you make changes.

### API Client

The API client in `web2/api` is generated from the openapi spec using `openapi-zod-client`. Run it with this command:

```bash
openapi-zod-client ../docs/openapi.yaml -o ./api/api-zod.ts --with-description --export-schemas
```

## CDN

The compiled javascript bundle in particular is pretty large (~200 KB). The ESP8266 is barely capable of serving it (it takes 5-10s) and there are frequently failures. 

I've landed on this solution:

1. Serve the compiled files from a jsdelivr github mirror. The web UI will attempt to load the appropriate compiled artifacts from here by default using the appropraite git tag, e.g.:
   ```
   https://cdn.jsdelivr.net/gh/sidoh/esp8266_milight_hub@latest/web2/dist/versions/<version>/bundle.js
   ```
2. If that fails, some embedded javascript in the web UI will attempt to load the bundle from the ESP8266.
3. These files are cached indefinitely using a `Cache-Control: max-age=31536000` header.
4. Filenames for the ESP8266-local files contain a content hash so new versions will automatically cache-bust.

It'd be nice at some point to have CI build the CDN artifacts, but for now I'm just going to check them in.

## Releasing

To release a new version of the web UI to the CDN, follow these steps:

1. Bump the version in `package.json`.
2. Run `npm run build`.
3. Check in the changes to git.