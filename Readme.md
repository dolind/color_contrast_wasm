# Color Contrast Wasm demo


In this project I examined how to C++ via WASM in web development.

I used an old project of mine. Color Contrast. 
We search for a grid with the best color distribution.

## Algorithmic part
I tried some optimization techniques, but stopped after the allocated time box was finished.

- hill climbing
- simulated annealing

Current issue is that the metric would prefer a 2 color grid. Currently this is prohibited by forcing as many colors as available.
For large grids this means we choose only the color map once and then scale the 2 most contrasting colors.

## Delivery technology
Key Result: How to use Shared buffers in Wasm with web workers. Fun.


## Links
[You can see the solution here](https://delicate-limit-0c06.docachaorg-132.workers.dev/)

[Blog article is here](https://www.storymelange.com/posts/projects/webassembly/c-in-your-browser-is-webassembly-worth-the-effort.html)