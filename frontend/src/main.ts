type RGB = { r: number; g: number; b: number };
type OilColor = { name: string; rgbValue: RGB };
type WasmVector<T> = {
    get(index: number): T;
    size(): number;
};
type GridResult = { colors: WasmVector<OilColor>; score: number };



interface GridData {
    colors: WasmVector<OilColor>;
    dim: number;
}

const gridData = new WeakMap<HTMLCanvasElement, GridData>();

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

async function renderGrid(canvas: HTMLCanvasElement, dim: number): Promise<number> {
    const module = await ensureModule();        // ✅ reuse module
    const result = module.compute_grid(dim, 1000) as GridResult;

    const ctx = canvas.getContext("2d")!;
    const cellPx = canvas.width / dim;

    gridData.set(canvas, {
        colors: result.colors,
        dim
    });

    let k = 0;
    for (let y = 0; y < dim; y++) {
        for (let x = 0; x < dim; x++) {

            const {r, g, b} = result.colors.get(k).rgbValue;
            k++;
            ctx.fillStyle = `rgb(${r}, ${g}, ${b})`;
            ctx.fillRect(x * cellPx, y * cellPx, cellPx, cellPx);
        }
    }
    return result.score;
}

async function renderAll(): Promise<void> {
    sizeCanvases();
    const stats: { dim: number; score: number; time: number }[] = [];

    await Promise.all(
        canvases.map(async (canvas) => {
            const dim = Number(canvas.dataset.dim);
            const start = performance.now();

            const score = await renderGrid(canvas, dim);
            const time = performance.now() - start;
            stats.push({dim, score, time});
        })
    );
    updateStatsTable(stats);
}

// Checkbox toggles cell sizing mode
constantCellsCheckbox.addEventListener("change", renderAll);

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

function updateStatsTable(rows: { dim: number; score: number; time: number }[]) {
    const div = document.getElementById("grid-stats")!;
    div.innerHTML = `
      <table>
        <thead>
          <tr><th>dim</th><th>score</th><th>time (ms)</th></tr>
        </thead>
        <tbody>
          ${rows
              .map(row => `
              <tr>
                <td>${row.dim}</td>
                <td>${row.score.toFixed(2)}</td>
                <td>${row.time.toFixed(1)}</td>
              </tr>`
              )
              .join("")}
        </tbody>
      </table>
    `;
}

// Initial render
renderAll();