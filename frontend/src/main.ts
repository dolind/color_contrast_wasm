type RGB = { r: number; g: number; b: number };
type OilColor = { name: string; rgbValue: RGB };
type WasmVector<T> = {
    get(index: number): T;
    size(): number;
};
type GridResult = { colors: OilColor[]; score: number; iterations: number; };


interface GridData {
    colors: OilColor[];
    dim: number;
}

const gridData = new WeakMap<HTMLCanvasElement, GridData>();
const stats: Record<number, {
    score: number;
    time: number;
    iterations: number;
    started: number;
}> = {};
const scriptUrl = new URL('/color_contrast_wasm.js', import.meta.url).href;
const createModule = (await import(scriptUrl)).default;

const constantCellsCheckbox = document.getElementById("constantCells") as HTMLInputElement;
const canvases = Array.from(document.querySelectorAll("canvas.grid")) as HTMLCanvasElement[];

const gridType = Number((document.getElementById("gridType") as HTMLSelectElement)?.value ?? 0);
const algoType = Number((document.getElementById("algoType") as HTMLSelectElement)?.value ?? 0);
const uniformColorDistribution = (document.getElementById("uniformColors") as HTMLInputElement)?.checked ? 1 : 0;

let wasmModule: Awaited<ReturnType<typeof createModule>> | null = null;

async function ensureModule() {
    if (!wasmModule) {
        wasmModule = await createModule();
    }
    return wasmModule;
}


function drawGrid(canvas: HTMLCanvasElement, result: GridResult, dim: number) {
    const ctx = canvas.getContext("2d")!;
    const cellPx = canvas.width / dim;

    gridData.set(canvas, {colors: result.colors, dim});

    let k = 0;
    for (let y = 0; y < dim; y++) {
        for (let x = 0; x < dim; x++) {
            const {r, g, b} = result.colors[k].rgbValue;
            k++;
            ctx.fillStyle = `rgb(${r}, ${g}, ${b})`;
            ctx.fillRect(x * cellPx, y * cellPx, cellPx, cellPx);
        }
    }
}

function sizeCanvases(): void {
    const useConstantCellSize = constantCellsCheckbox.checked;

    canvases.forEach((canvas) => {
        const dim = Number(canvas.dataset.dim);

        if (useConstantCellSize) {
            const screenFraction = 0.02;
            const cell = Math.max(Math.floor((window.innerWidth * screenFraction)), 25);
            canvas.width = canvas.height = cell * dim;
        } else {
            const screenFraction = 0.15;
            const gridSize = Math.max(Math.floor((window.innerWidth * screenFraction)), 200);


            canvas.width = canvas.height = gridSize;
        }

        // ✅ Keeps drawing resolution & visible size aligned
        canvas.style.width = `${canvas.width}px`;
        canvas.style.height = `${canvas.height}px`;
    });
}

// Checkbox toggles cell sizing mode
constantCellsCheckbox?.addEventListener("change", () => {
    sizeCanvases(); // keep animation running, just resize
});

const tooltip = document.getElementById("tooltip") as HTMLDivElement;
canvases.forEach(canvas => {
    canvas.addEventListener("mousemove", (ev) => {
        const data = gridData.get(canvas);
        if (!data) return;

        const {colors, dim} = data;

        const rect = canvas.getBoundingClientRect();
        const x = ev.clientX - rect.left;
        const y = ev.clientY - rect.top;

        const cellPx = canvas.width / dim;
        const col = Math.floor(x / cellPx);
        const row = Math.floor(y / cellPx);
        const idx = row * dim + col;

        const color = colors[idx];
        if (!color) {
            tooltip.style.opacity = "0";
            return;
        }

        tooltip.textContent = color.name;
        tooltip.style.left = `${ev.pageX}px`;
        tooltip.style.top = `${ev.pageY - 10}px`;
        tooltip.style.opacity = "1";
    });

    canvas.addEventListener("mouseleave", () => {
        tooltip.style.opacity = "0";
    });
});

function updateStatsTable() {
    const div = document.getElementById("grid-stats")!;
    const timeDiv = document.getElementById("grid-time")!; // new separate display

    const rows = Object.keys(stats)
        .sort((a, b) => Number(a) - Number(b))
        .map(dimStr => {
            const dim = Number(dimStr);
            const s = stats[dim];

            return `
                <tr>
                  <td>${dim}</td>
                  <td>${s.score.toFixed(2)}</td>
                  <td>${s.iterations}</td>
                </tr>
            `;
        })
        .join("");

    div.innerHTML = `
      <table>
        <thead>
          <tr>
            <th>dim</th>
            <th>score</th>
            <th>iterations</th>
          </tr>
        </thead>
        <tbody>${rows}</tbody>
      </table>
    `;

    // ⏱ show most recent elapsed time separately
    const maxDim = Math.max(...Object.keys(stats).map(Number));
    const last = stats[maxDim];
    if (last) {
        timeDiv.textContent = `Elapsed time: ${last.time.toFixed(1)} ms`;
    }
}


import workerUrl from "./gridWorker.ts?worker&url"; // 👈 important

const MAX_WORKERS = 5;
const workers: Worker[] = [];
const taskQueue: number[] = []; // store canvas indexes instead of canvases
let stopTimeoutId: number | null = null;

function assignTask(worker: Worker, canvasIndex: number) {
    const canvas = canvases[canvasIndex];
    const dim = Number(canvas.dataset.dim);

    stats[dim] = {score: 0, time: 0, iterations: 0, started: performance.now()};

    worker.postMessage({
        canvasIndex,
        dim,
        scriptUrl,
        gridType: getSelectedValue("gridType"),
        algoType: getSelectedValue("algoType"),
        uniformColorDistribution: getUniformFlag()
    });
}

async function animateAllGrids() {
    sizeCanvases();
    await ensureModule();

    // build queue with numeric indexes
    for (let i = 0; i < canvases.length; i++) taskQueue.push(i);

    // create worker pool
    for (let i = 0; i < MAX_WORKERS; i++) {
        const worker = new Worker(workerUrl, {type: "module"});

        worker.onmessage = (ev) => {
            const {canvasIndex, result} = ev.data;
            const canvas = canvases[canvasIndex];
            const dim = Number(canvas.dataset.dim);

            drawGrid(canvas, result, dim);

            stats[dim] = {
                score: result.score,
                time: performance.now() - stats[dim].started,
                iterations: result.iterations,
                started: stats[dim].started
            };

            updateStatsTable();

            const next = taskQueue.shift();
            if (next !== undefined) assignTask(worker, next);
        };

        workers.push(worker);
    }

    // start initial batch
    workers.forEach(worker => {
        const idx = taskQueue.shift();
        if (idx !== undefined) assignTask(worker, idx);
    });


    const maxTimeInput = document.getElementById("iters") as HTMLInputElement;

    const maxTimeSec = Number(maxTimeInput.value);

    stopTimeoutId = window.setTimeout(() => {
        console.log(`⏹ stopping after ${maxTimeSec}s`);

        workers.forEach(w => w.terminate());
        workers.length = 0;

        const timeDiv = document.getElementById("grid-time");
        if (timeDiv) timeDiv.textContent = `Stopped after ${maxTimeSec} seconds`;
    }, maxTimeSec * 1000);

}



function restartAllWorkers() {
    console.log("🔁 Restarting due to control change");

    // Stop current workers
    workers.forEach(w => w.terminate());
    workers.length = 0;
    taskQueue.length = 0;

    if (stopTimeoutId !== null) {
        clearTimeout(stopTimeoutId);
        stopTimeoutId = null;
    }

    animateAllGrids();
}
const maxTimeInput = document.getElementById("iters") as HTMLInputElement;
maxTimeInput.addEventListener("change", restartAllWorkers);

// Re-run when algorithm or grid type changes
document.querySelectorAll('input[name="gridType"]').forEach(el =>
    el.addEventListener("change", restartAllWorkers)
);
document.querySelectorAll('input[name="algoType"]').forEach(el =>
    el.addEventListener("change", restartAllWorkers)
);

// Re-run when uniform checkbox toggled
document.getElementById("uniformColors")?.addEventListener("change", restartAllWorkers);

// Re-run when constant cell sizing toggled
function getSelectedValue(name: string): number {
    const el = document.querySelector<HTMLInputElement>(`input[name="${name}"]:checked`);
    return el ? Number(el.value) : 0;
}

function getUniformFlag(): number {
    return (document.getElementById("uniformColors") as HTMLInputElement)?.checked ? 1 : 0;
}
ensureModule().then(() => animateAllGrids());