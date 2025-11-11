
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

    const rows = Object.keys(stats)
        .sort((a, b) => Number(a) - Number(b))
        .map(dimStr => {
            const dim = Number(dimStr);
            const s = stats[dim];

            return `
                <tr>
                  <td>${dim}</td>
                  <td>${s.score.toFixed(2)}</td>
                  <td>${s.time.toFixed(1)}</td>
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
            <th>time (ms)</th>
            <th>iterations</th>   <!-- ✅ new column -->
          </tr>
        </thead>
        <tbody>${rows}</tbody>
      </table>
    `;
}

import workerUrl from "./gridWorker.ts?worker&url";  // 👈 important

const MAX_WORKERS = 5;
const workers: Worker[] = [];
const taskQueue: number[] = []; // store canvas indexes instead of canvases

function assignTask(worker: Worker, canvasIndex: number) {
    const canvas = canvases[canvasIndex];
    const dim = Number(canvas.dataset.dim);

    stats[dim] = { score: 0, time: 0, iterations: 0, started: performance.now() };

    worker.postMessage({
        canvasIndex,  // directly reference canvas
        dim,
        scriptUrl
    });
}

async function animateAllGrids() {
    sizeCanvases();
    await ensureModule();

    // build queue with numeric indexes
    for (let i = 0; i < canvases.length; i++) taskQueue.push(i);

    // create worker pool
    for (let i = 0; i < MAX_WORKERS; i++) {
        const worker = new Worker(workerUrl, { type: "module" });

        worker.onmessage = (ev) => {
            const { canvasIndex, result } = ev.data;
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
}


ensureModule().then(() => animateAllGrids());