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
                defineAlgorithm('brotliCompress', {
                    params: {
                        [constants.BROTLI_PARAM_QUALITY]: 11,
                    },
                }),
            ],
            exclude: [/\.(br)$/],
            // deleteOriginalAssets: true,
        }),
        viteSingleFile(),
        visualizer(),
    ],
    build: {
        target: ["es2022", "edge100", "firefox100", "chrome100", "safari15"],
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
                unsafe_proto: true
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
