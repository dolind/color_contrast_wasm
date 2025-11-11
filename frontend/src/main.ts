type RGB = { r: number; g: number; b: number };
type OilColor = { name: string; rgbValue: RGB };
type WasmVector<T> = {
    get(index: number): T;
    size(): number;
};
type GridResult = { colors: WasmVector<OilColor>; score: number; iterations: number; };


interface GridData {
    colors: WasmVector<OilColor>;
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
            const {r, g, b} = result.colors.get(k).rgbValue;
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

        const color = colors.get(idx);
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


async function animateAllGrids() {
    sizeCanvases();
    const module = await ensureModule();

    canvases.forEach((canvas, idx) => {
        const dim = Number(canvas.dataset.dim);
        stats[dim] = {score: 0, time: 0, iterations: 0, started: performance.now()};
        module.start_search(idx, dim, 2000); // each grid has its own engine instance

    });

    function animate() {
        canvases.forEach((canvas, idx) => {
            const dim = Number(canvas.dataset.dim);
            const t0 = performance.now();

            const result = wasmModule!.step_search(idx) as GridResult;

            const dt = performance.now() - t0;
            const elapsed = performance.now() - stats[dim].started;
            drawGrid(canvas, result, dim);

            stats[dim] = {
                score: result.score,
                time: elapsed,
                iterations: result.iterations,
                started: stats[dim].started
            };
        });

        updateStatsTable();
        requestAnimationFrame(animate);
    }

    animate();
}

ensureModule().then(() => animateAllGrids());