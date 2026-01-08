const flasks = [
    {id: 'flask-health', type: 0, color: '#6a0020'},
    {id: 'flask-stamina', type: 1, color: '#2f7d33'},
    {id: 'flask-magick', type: 2, color: '#2f4ba4'},
    {id: 'flask-other', type: 3, color: '#7b4997'}
];

// Store references to SVG elements for quick access
const flaskElements = {};

function initFlask(item) {
    const obj = document.getElementById(item.id);
    if (!obj) return;

    // Set initial glow color
    obj.style.setProperty('--glow-color', item.color);

    const setupSvg = () => {
        // Prevent double initialization
        if (flaskElements[item.type]) return;

        const svgDoc = obj.contentDocument;
        if (!svgDoc) {
            console.warn(`Cannot access content of ${item.id}. Check CORS or loading state.`);
            return;
        }

        const svgRoot = svgDoc.documentElement;

        // 1. Expand viewBox
        // Check if already expanded to avoid repeated expansion on reload
        if (!svgRoot.hasAttribute('data-expanded')) {
            const originalVB = svgRoot.getAttribute('viewBox').split(' ').map(parseFloat);
            const padding = 80;
            const newX = originalVB[0] - padding;
            const newY = originalVB[1] - padding;
            const newW = originalVB[2] + (padding * 2);
            const newH = originalVB[3] + (padding * 2);
            svgRoot.setAttribute('viewBox', `${newX} ${newY} ${newW} ${newH}`);
            svgRoot.setAttribute('data-expanded', 'true');
        }

        // 2. Inject Styles
        if (!svgDoc.getElementById('flask-styles')) {
            const styleElement = svgDoc.createElementNS("http://www.w3.org/2000/svg", "style");
            styleElement.id = 'flask-styles';
            styleElement.textContent = `
                #flask-fill-rect {
                    transform-origin: bottom;
                    transform-box: fill-box;
                    transform: scaleY(0); /* Start empty or controlled by JS */
                    transition: transform 0.5s ease-out;
                }

                .flask-counter {
                    font-family: sans-serif;
                    font-weight: 900;
                    font-size: 82px;
                    fill: white;
                    stroke: black;
                    stroke-width: 10px;
                    paint-order: stroke;
                    pointer-events: none;
                }
            `;
            svgDoc.documentElement.appendChild(styleElement);
        }

        // 3. Inject Text Element
        let textElement = svgDoc.querySelector('.flask-counter');
        if (!textElement) {
            textElement = svgDoc.createElementNS("http://www.w3.org/2000/svg", "text");
            textElement.textContent = "";
            textElement.setAttribute("class", "flask-counter");
            textElement.setAttribute("x", "242");
            textElement.setAttribute("y", "266");
            textElement.setAttribute("dominant-baseline", "central");
            textElement.setAttribute("text-anchor", "middle");
            svgRoot.appendChild(textElement);
        }

        // Store references
        flaskElements[item.type] = {
            object: obj,
            svgRoot: svgRoot,
            fillRect: svgDoc.getElementById('flask-fill-rect'),
            text: textElement,
            wasFull: false
        };

        // Make visible if it was hidden by CSS
        // obj.style.opacity = 1; 
    };

    // Try to setup immediately if loaded, otherwise wait for load
    if (obj.contentDocument && obj.contentDocument.documentElement) {
        setupSvg();
    } else {
        obj.addEventListener('load', setupSvg);
    }
}

window.firstInitDom = () => {
    flasks.forEach(initFlask);
}

// Global functions called from C++

window.setWidgetSettings = (settingsJson) => {
    try {
        const settings = JSON.parse(settingsJson);
        const container = document.getElementById('widget-container');

        if (!container) return;

        if (!settings.enable) {
            container.style.display = 'none';
            return;
        }
        container.style.display = 'block';

        if (settings.anchor_all) {
            // Global positioning
            container.style.left = (settings.x * window.innerWidth) + 'px';
            container.style.top = (settings.y * window.innerHeight) + 'px';
            container.style.transform = `scale(${settings.size})`;
            container.style.opacity = settings.opacity;

            // Reset individual flask styles if they were set
            for (const key in flaskElements) {
                const el = flaskElements[key].object;
                el.style.transform = '';
                el.style.opacity = '';
                el.style.left = '';
                el.style.top = '';
            }

            // Ensure objects are visible (they default to opacity 0 in CSS)
            flasks.forEach(f => {
                const obj = document.getElementById(f.id);
                if (obj) obj.style.opacity = '1';
            });

        } else {
            // Individual positioning
            container.style.left = '0px';
            container.style.top = '0px';
            container.style.transform = 'scale(1)';
            container.style.opacity = '1';

            const applyFlaskSettings = (type, s) => {
                // We need to access the DOM element even if flaskElements is not fully init yet
                // (e.g. SVG loading)
                const item = flasks.find(f => f.type === type);
                if (!item) return;
                const el = document.getElementById(item.id);

                if (el) {
                    el.style.left = (s.x * window.innerWidth) + 'px';
                    el.style.top = (s.y * window.innerHeight) + 'px';
                    el.style.transform = `scale(${s.size})`;
                    el.style.opacity = s.opacity;
                }
            };

            applyFlaskSettings(0, settings.health);
            applyFlaskSettings(1, settings.stamina);
            applyFlaskSettings(2, settings.magick);
            applyFlaskSettings(3, settings.other);
        }

    } catch (e) {
        console.error("Error parsing settings JSON", e);
    }
};

window.setWidgetSettingsInit = (settingsJson) => {
    setTimeout(() => window.setWidgetSettings(settingsJson), 1500)
}

window.updateFlaskData = (args) => {
    // Format: "type,percent,count,glow"
    // Example: "0,0.500,5,0"

    if (!args) return;
    const parts = args.split(',');
    if (parts.length < 4) return;

    const flaskType = parseInt(parts[0]);
    const fillPercent = parseFloat(parts[1]);
    const count = parseInt(parts[2]);
    const shouldGlow = parts[3] === '1';

    const el = flaskElements[flaskType];
    if (!el) return;

    // Update fill
    if (el.fillRect) {
        el.fillRect.style.transform = `scaleY(${fillPercent})`;
    }

    // Update text
    if (el.text) {
        el.text.textContent = count > 0 ? count : "";
    }

    // Glow logic
    const isFull = fillPercent >= 0.99;
    let triggerGlow = shouldGlow;

    if (isFull && !el.wasFull) {
        triggerGlow = true;
    }
    el.wasFull = isFull;

    if (triggerGlow) {
        el.object.style.animationPlayState = 'running';
        // Reset animation after 3 seconds
        setTimeout(() => {
            el.object.style.animationPlayState = 'paused';
        }, 3000);
    }

    // Ensure visibility
    if (el.object.style.opacity === '' || el.object.style.opacity === '0') {
        el.object.style.opacity = '1';
    }
};


document.addEventListener('DOMContentLoaded', () => {
    setTimeout(() => window.firstInitDom(), 1000);
});

// For browser debugging: simulate settings if not in game
/*
if (!window.prisma) {
    setTimeout(() => {
        const mockSettings = {
            enable: true,
            x: 0.25, y: 0.25, size: 1.0, opacity: 1.0, anchor_all: true,
            health: {x:0,y:0,size:1,opacity:1},
            stamina: {x:0,y:0,size:1,opacity:1},
            magick: {x:0,y:0,size:1,opacity:1},
            other: {x:0,y:0,size:1,opacity:1}
        };
        window.setWidgetSettings(JSON.stringify(mockSettings));
        window.firstInitDom();
        
        // Mock data update
        window.updateFlaskData("0,0.5,5,0");
        window.updateFlaskData("1,1.0,3,1");
        window.updateFlaskData("2,0.2,0,0");
        window.updateFlaskData("3,0.8,10,1");
    }, 500);
}
*/
