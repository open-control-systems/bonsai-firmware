import { defineConfig } from "vite";
import preact from "@preact/preset-vite";
import compression from "vite-plugin-compression";

export default defineConfig({
  plugins: [
    preact(),
    compression({
      // Options: gzip, brotliCompress
      algorithm: "gzip",
      // File extension for compressed files
      ext: ".gz",
      // Minimum size in bytes to compress
      threshold: 128,
      deleteOriginFile: true,
    }),
  ],
  server: {
    proxy: {
      "/api/v1/telemetry": {
        target: "http://192.168.4.1",
        changeOrigin: true, // Fixes cross-origin issues
      },
      "/api/v1/registration": {
        target: "http://192.168.4.1",
        changeOrigin: true, // Fix cross-origin issues
      },
    },
  },
  build: {
    rollupOptions: {
      output: {
        entryFileNames: "assets/index.js",
        assetFileNames: "assets/[name].[ext]",
      },
    },
  },
});
