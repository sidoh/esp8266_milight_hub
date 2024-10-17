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
PROXY_URL=http://hostname-of-your-hub node server.js
```

I do this only to proxy API requests to an actual ESP8266 running on my local network.

### API Client

The API client in `web2/api` is generated from the openapi spec using `openapi-zod-client`. Run it with this command:

```bash
openapi-zod-client ../docs/openapi.yaml -o ./api/api-zod.ts --with-description
```