export {};

let wasmModule: any = null;

self.onmessage = async (ev) => {
    const { canvasIndex, dim, scriptUrl } = ev.data;

    if (!wasmModule) {
        const createModule = (await import(scriptUrl)).default;
        wasmModule = await createModule();
    }

    wasmModule.start_search(canvasIndex, dim, 100);
let lastSend = 0;

    function compute() {
        // ⏱ allow up to 4 ms of compute per chunk (~250 chunks/sec)
        const end = performance.now() + 4;

        while (performance.now() < end) {
            // 🚀 compute as fast as CPU allows
            wasmModule.step_search(canvasIndex);
        }

        // throttle messaging to ~60 FPS
        const now = performance.now();
        if (now - lastSend > 16) {
            const result = wasmModule.step_search(canvasIndex); // or whichever method returns state

            // serialize colors to real JS objects
            const size = result.colors.size();
            const serializedColors = Array.from({ length: size }, (_, i) => {
                const c = result.colors.get(i);
                return {
                    name: c.name,
                    rgbValue: { ...c.rgbValue }
                };
            });

            postMessage({
                canvasIndex,
                result: {
                    score: result.score,
                    iterations: result.iterations,
                    colors: serializedColors
                }
            });

            lastSend = now;
        }

        // voluntarily yield to browser scheduler
        setTimeout(compute, 0);
    }

    compute();
};