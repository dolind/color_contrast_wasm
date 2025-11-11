type RGB = { r: number; g: number; b: number };
type OilColor = { name: string; rgb: RGB };
type GridResult = { colors: OilColor[]; score: number };

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

async function renderGrid(canvas: HTMLCanvasElement, dim: number): Promise<void> {
    const module = await ensureModule();        // ✅ reuse module
    const result = module.compute_grid(dim, 1000) as GridResult;

    const ctx = canvas.getContext("2d")!;
    const cellPx = canvas.width / dim;

    let k = 0;
    for (let y = 0; y < dim; y++) {
        for (let x = 0; x < dim; x++) {

            const [r, g, b] = result.colors[k++].rgb;
            ctx.fillStyle = `rgb(${r}, ${g}, ${b})`;
            ctx.fillRect(x * cellPx, y * cellPx, cellPx, cellPx);
        }
    }
}

async function renderAll(): Promise<void> {
    sizeCanvases();
    await Promise.all(
        canvases.map(async (canvas) => {
            const dim = Number(canvas.dataset.dim);
            await renderGrid(canvas, dim);
        })
    );
}

// Checkbox toggles cell sizing mode
constantCellsCheckbox.addEventListener("change", renderAll);

// Initial render
renderAll();