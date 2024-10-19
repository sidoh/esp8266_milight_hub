const express = require('express');
const { createProxyMiddleware } = require('http-proxy-middleware');
const path = require('path');

const app = express();
const PORT = 3000; // You can change this to any port you prefer
const PROXY_URL = process.env.PROXY_URL || 'http://10.133.8.88'; // Replace with your proxy URL
const WS_PROXY_URL = process.env.WS_PROXY_URL || `ws://10.133.8.88:81`; // WebSocket proxy URL, defaults to PROXY_URL if not set

// HTTP proxy middleware
const httpProxyMiddleware = createProxyMiddleware({
  target: PROXY_URL,
  changeOrigin: true,
});

// WebSocket proxy middleware
const wsProxyMiddleware = createProxyMiddleware({
  target: WS_PROXY_URL,
  changeOrigin: true,
  ws: true, // Enable WebSocket proxying
});


// Serve static files from the 'dist' directory
app.use('/dist', express.static(path.join(__dirname, 'dist/compiled')));
app.use('/index.html', express.static(path.join(__dirname, 'dist/index.html')));

// Proxy HTTP requests that aren't files
app.use('/', httpProxyMiddleware);

const server = app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
  console.log(`HTTP requests proxied to: ${PROXY_URL}`);
  console.log(`WebSocket connections proxied to: ${WS_PROXY_URL}`);
});

// Upgrade HTTP server to also handle WebSocket connections
server.on('upgrade', wsProxyMiddleware.upgrade);
