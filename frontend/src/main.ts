// 1-based index to match COLOR_MAP keys (1..24)
const COLOR_NAMES: (string | null)[] = [
    null, // 0 unused
    "White",
    "Pale Blue",
    "Azure Blue",
    "Yellow Ocre",
    "Mandarin",
    "Intense Red",
    "Ultramarine Blue",
    "Ruby Red",
    "Delft Blue",
    "Geranium Lake Light",
    "Celestial Blue",
    "Red Brown",
    "Celadon Green",
    "Brown Madder",
    "Cinnabar Green",
    "Raw Umber",
    "Green Medium",
    "Burnt Umber",
    "Pine Green",
    "Reddish Brown Grey",
    "Lemon Yellow",
    "Grey Green",
    "Yellow Deep",
    "Black"
];


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

const canvasRGBBuffers: SharedArrayBuffer[] = [];
const canvasRGBViews: Uint8Array[] = [];

const canvasIndexBuffers: SharedArrayBuffer[] = [];
const canvasIndexViews: Uint8Array[] = [];

async function ensureModule() {
    if (!wasmModule) {
        wasmModule = await createModule();
    }
    return wasmModule;
}

function drawGridFromRGB(canvas: HTMLCanvasElement, rgb: Uint8Array, dim: number) {
    const ctx = canvas.getContext("2d")!;
    const cellPx = canvas.width / dim;

    let k = 0;
    for (let y = 0; y < dim; y++) {
        for (let x = 0; x < dim; x++) {
            const r = rgb[k++];
            const g = rgb[k++];
            const b = rgb[k++];

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
const tooltipTextNode = document.createTextNode("");
tooltip.innerHTML = "";
tooltip.appendChild(tooltipTextNode);

// Pre-calc styles (avoids layout trashing)
tooltip.style.position = "absolute";
tooltip.style.pointerEvents = "none";
tooltip.style.opacity = "0";
canvases.forEach((canvas, canvasIndex) => {
    canvas.addEventListener("mousemove", (ev) => {
        const dim = Number(canvas.dataset.dim);
        const idxView = canvasIndexViews[canvasIndex];

        if (!idxView) return;

        const rect = canvas.getBoundingClientRect();
        const x = ev.clientX - rect.left;
        const y = ev.clientY - rect.top;

        const cellPx = canvas.width / dim;
        const col = Math.floor(x / cellPx);
        const row = Math.floor(y / cellPx);
        const idx = row * dim + col;

        const colorID = idxView[idx];
        const name = COLOR_NAMES[colorID] ?? "Unknown";

        tooltipTextNode.nodeValue = name;
        tooltip.style.left = `${ev.clientX + 10}px`;
        tooltip.style.top = `${ev.clientY - 10}px`;
        tooltip.style.opacity = "1";
    });

    canvas.addEventListener("mouseleave", () => {
        tooltip.style.opacity = "0";
    });
});


const tableDiv = document.getElementById("grid-stats")!;
const timeDiv = document.getElementById("grid-time")!;

const statsTable = document.createElement("table");
const thead = document.createElement("thead");
const headerRow = document.createElement("tr");

["dim", "score", "iterations"].forEach(h => {
    const th = document.createElement("th");
    th.textContent = h;
    headerRow.appendChild(th);
});

thead.appendChild(headerRow);
statsTable.appendChild(thead);

const tbody = document.createElement("tbody");
statsTable.appendChild(tbody);

tableDiv.appendChild(statsTable);

// keep rows reusable:
const statsRows = new Map<number, HTMLTableRowElement>();

function updateStatsTable() {
    for (const dimStr of Object.keys(stats)) {
        const dim = Number(dimStr);
        const s = stats[dim];

        let row = statsRows.get(dim);
        if (!row) {
            row = document.createElement("tr");
            row.innerHTML = `
                <td class="dim"></td>
                <td class="score"></td>
                <td class="iterations"></td>
            `;
            statsRows.set(dim, row);
            tbody.appendChild(row);
        }

        row.querySelector(".dim")!.textContent = String(dim);
        row.querySelector(".score")!.textContent = s.score.toFixed(2);
        row.querySelector(".iterations")!.textContent = String(s.iterations);
    }

    // update time (simple text update → no leak)
    const maxDim = Math.max(...Object.keys(stats).map(Number));
    const last = stats[maxDim];
    if (last) {
        timeDiv.textContent = `Elapsed time: ${last.time.toFixed(1)} ms`;
    }
}

function initSharedBuffers() {
    canvases.forEach((canvas, canvasIndex) => {
        const dim = Number(canvas.dataset.dim);
        const cells = dim * dim;

        // RGB buffer
        const rgbBytes = cells * 3;
        const rgbSAB = new SharedArrayBuffer(rgbBytes);
        canvasRGBBuffers[canvasIndex] = rgbSAB;
        canvasRGBViews[canvasIndex] = new Uint8Array(rgbSAB);

        // NEW: index buffer
        const idxSAB = new SharedArrayBuffer(cells);  // 1 byte per cell
        canvasIndexBuffers[canvasIndex] = idxSAB;
        canvasIndexViews[canvasIndex] = new Uint8Array(idxSAB);
    });
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
        uniformColorDistribution: getUniformFlag(),
        sab: canvasRGBBuffers[canvasIndex],
        sabIDX: canvasIndexBuffers[canvasIndex]
    });
}

async function animateAllGrids() {
    sizeCanvases();
    initSharedBuffers();
    await ensureModule();

    // build queue with numeric indexes
    for (let i = 0; i < canvases.length; i++) taskQueue.push(i);

    // create worker pool
    for (let i = 0; i < MAX_WORKERS; i++) {
        const worker = new Worker(workerUrl, {type: "module"});

        worker.onmessage = (ev) => {
            const { canvasIndex, score, iterations } = ev.data;
            const canvas = canvases[canvasIndex];
            const dim = Number(canvas.dataset.dim);

            const rgbView = canvasRGBViews[canvasIndex];
            drawGridFromRGB(canvas, rgbView, dim);

            stats[dim] = {
                score,
                time: performance.now() - stats[dim].started,
                iterations,
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