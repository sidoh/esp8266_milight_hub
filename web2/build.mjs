import { tailwindPlugin } from 'esbuild-plugin-tailwindcss';
import esbuild from 'esbuild';


await esbuild.build({
  entryPoints: ['src/index.tsx'],
  bundle: true,
  outfile: 'dist/bundle.js',
  minify: true,
  sourcemap: false,
  define: { 'process.env.NODE_ENV': '"production"' },
  loader: {
    '.tsx': 'tsx',
  },
  plugins: [tailwindPlugin()],
});