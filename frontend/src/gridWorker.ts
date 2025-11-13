export {};

let wasmModule: any = null;

self.onmessage = async (ev) => {
    const {
        canvasIndex,
        dim,
        scriptUrl,
        gridType,
        algoType,
        uniformColorDistribution,
        sab,
        sabIDX
    } = ev.data;

    if (!wasmModule) {
        const createModule = (await import(scriptUrl)).default;
        wasmModule = await createModule();
    }

    const sharedRGB = new Uint8Array(sab);
    const sharedIDX = new Uint8Array(sabIDX);
    const rgbLen = sharedRGB.length;
    const wasmRGBPtr = wasmModule._malloc(rgbLen);

    wasmModule.start_search(canvasIndex, dim, 100, gridType, algoType, uniformColorDistribution);

    let lastSend = 0;

    function compute() {
        const end = performance.now() + 4;
        let lastStepInfo: any = null;

        // do as many steps as possible in this 4ms budget
        while (performance.now() < end) {
            lastStepInfo = wasmModule.step_search_info(canvasIndex);
        }

        // throttle messaging to ~20 FPS
        const now = performance.now();

        if (now - lastSend > 50) {

            wasmModule.export_grid_rgb(canvasIndex, wasmRGBPtr, rgbLen);
            const wasmRGB = wasmModule.HEAPU8.subarray(wasmRGBPtr, wasmRGBPtr + rgbLen);
            sharedRGB.set(wasmRGB);
            for (let i = 0; i < sharedIDX.length; i++) {
                sharedIDX[i] = wasmModule.get_color_index(canvasIndex, i);
            }
            // serialize colors to real JS objects
            self.postMessage({
                canvasIndex,
                score: lastStepInfo.score,
                iterations: lastStepInfo.iterations
            });


            lastSend = now;
        }

        // voluntarily yield to browser scheduler
        setTimeout(compute, 0);
    }

    compute();
};