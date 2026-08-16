import {defineConfig} from "vite";
import solid from "vite-plugin-solid";
import tailwindcss from "@tailwindcss/vite";
import { visualizer } from "rollup-plugin-visualizer";
import {compression, defineAlgorithm} from 'vite-plugin-compression2';
import {viteSingleFile} from 'vite-plugin-singlefile';
import { constants } from 'zlib';

export default defineConfig({
    plugins: [
        solid({
            solid: {
                hydratable: false,
                generate: 'dom'
            }
        }),
        tailwindcss(),
        compression({
            algorithms: [
                defineAlgorithm('gzip', {
                    level: constants.Z_BEST_COMPRESSION,
                    windowBits: 15,
                    memLevel: 9
                }),
            ],
            exclude: [/\.(gz)$/],
        }),
        viteSingleFile(),
        visualizer(),
    ],
    build: {
        target: ["es2022", "chrome100"],
        minify: "terser",
        terserOptions: {
            compress: {
                drop_console: true,
                drop_debugger: true,
                global_defs: {
                    'DEV': false,
                    'import.meta.env.DEV': false,
                    'process.env.NODE_ENV': '"production"'
                },
                pure_funcs: ['Function.prototype'],
                passes: 3,
                unsafe: true,
                unsafe_arrows: true,
                unsafe_comps: true,
                unsafe_math: true,
                unsafe_proto: true,
                toplevel: true
            },
            mangle: {
                toplevel: true,
            },
            format: {
                comments: false,
            },
        },
        cssMinify: "lightningcss",
        sourcemap: false,
        cssCodeSplit: false,
        modulePreload: false,
        assetsInlineLimit: () => true,
    },
    resolve: {
        conditions: ['production', 'browser', 'solid']
    },
});
