import commonjs from "@rollup/plugin-commonjs";
import { nodeResolve } from "@rollup/plugin-node-resolve";
import terser from "@rollup/plugin-terser";
import typescript from "@rollup/plugin-typescript";
import tryCatch from "./tryCatch.js";

const isDevelopment = process.env.NODE_ENV === "development";

/**
 * @type {import("rollup").RollupOptions[]}
 */
const configs = [
  ["./src/game.ts", "./dist/game.js"],
  ["./src/generic.ts", "./dist/generic.js"],
  ["./src/tampermonkey.ts", "./dist/tampermonkey.js"],
].map(([input, file]) => ({
  input,
  output: {
    sourcemap: "inline",
    sourcemapPathTransform: (relativeSourcePath) =>
      new URL(
        "./frontend/" + relativeSourcePath,
        "rollup://chief/frontend/"
      ).toString(),
    file,
    format: "iife",
  },
  plugins: [
    commonjs(),
    nodeResolve(),
    typescript({
      sourceMap: true,
      inlineSources: true,
    }),
    tryCatch(),
    !isDevelopment && terser(),
  ],
}));

export default configs;
