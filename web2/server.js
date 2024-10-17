const express = require('express');
const { createProxyMiddleware } = require('http-proxy-middleware');
const path = require('path');

const app = express();
const PORT = 3000; // You can change this to any port you prefer
const PROXY_URL = process.env.PROXY_URL || 'http://10.133.8.88'; // Replace with your proxy URL

const proxyMiddleware = createProxyMiddleware({
  target: PROXY_URL,
  changeOrigin: true,
});

// Serve static files from the 'dist' directory
app.use(express.static(path.join(__dirname, 'dist')));

// Proxy requests that aren't files
app.use('/', proxyMiddleware);

app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});